#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

const char* FIFO_PATH = "/tmp/chat_fifo";
const int BUFFER_SIZE = 1024;

void send_message(int fd, const std::string& message) {
    ssize_t bytes_w = write(fd, message.c_str(), message.size());
    if(bytes_w == -1){ //обработка ошибок при записи 
        perror("Error writing to file fifo");
    }
}

std::string receive_message(int fd) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytesRead = read(fd, buffer, BUFFER_SIZE);
    if(bytesRead == -1){
        perror("Error reading fifo file");
        exit(1);
    }
    if (bytesRead > 0) {
        return std::string(buffer);
    }
    
    exit(0);          // сервер вышел
}

int main() {
    char buffer[BUFFER_SIZE];
    
    std::cout << "Client is running. Type 'exit' to quit." << std::endl;
    
    while (true) {
        std::cout << "Enter message: "; 
        std::cin.getline(buffer, BUFFER_SIZE);              // читаем команду         
        std::streamsize bytes_read = std::cin.gcount();

        if (bytes_read < BUFFER_SIZE)
        { 
            buffer[bytes_read - 1] = '\0';
        }
        else
        {
            buffer[bytes_read] = '\0';
        }
        
        std::string message(buffer);
        int fifo_fd = open(FIFO_PATH, O_WRONLY);            // открываем файл для записи
        if (fifo_fd < 0) {                                  // проверка ошибок
            perror("Error opening file fifo");
            break;
        }

        

        if (message == "exit") {  
            
            break;
            
        }
        send_message(fifo_fd, message);                      // отправляем сообщение на сервер
        close(fifo_fd);

        fifo_fd = open(FIFO_PATH, O_RDONLY);                // открываем файл для чтения
        if (fifo_fd < 0) {                                  // проверка ошибок
            perror("Error opening file fifo");
            break;
        }

        std::string response = receive_message(fifo_fd);     // получаем запрос
        close(fifo_fd);

        std::cout << "Server: " << response << std::endl;
    }

    return 0;
}
