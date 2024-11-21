#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

const char* FIFO_PATH = "/tmp/chat_fifo";
const int BUFFER_SIZE = 256;

void send_message(int fd, const std::string& message) {
    write(fd, message.c_str(), message.size());
}

std::string receive_message(int fd) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytesRead = read(fd, buffer, BUFFER_SIZE);
    if (bytesRead > 0) {
        return std::string(buffer);
    }
    return " ";                 // почему то где то не возвращает
}

int main() {
    char buffer[BUFFER_SIZE];
    std::cout << "Client is running. Type 'exit' to quit." << std::endl;

    while (true) {
        std::cout << "Enter message (ping/history/exit): "; // мб добавить запросов (?)
        std::cin.getline(buffer, BUFFER_SIZE);              // читаем команду                       // удалить лишние пробелы

        std::string message(buffer);
        int fifo_fd = open(FIFO_PATH, O_WRONLY);            // открываем файл для записи
        if (fifo_fd < 0) {                                  // проверка ошибок
            perror("Error opening FIFO");
            continue;
        }

        send_message(fifo_fd, message);                      // отправляем сообщение на сервер
        close(fifo_fd);

        if (message == "exit") {                        
            std::cout << "Exiting..." << std::endl;
            break;
        }

        fifo_fd = open(FIFO_PATH, O_RDONLY);                // открываем файл для чтения
        if (fifo_fd < 0) {                                  // проверка ошибок
            perror("Error opening FIFO");
            continue;
        }

        std::string response = receive_message(fifo_fd);     // получаем запрос
        close(fifo_fd);

        std::cout << "Server: " << response << std::endl;
    }

    return 0;
}
