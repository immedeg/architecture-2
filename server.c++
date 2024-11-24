#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <map>


const char *FIFO_PATH = "/tmp/chat_fifo";
const char *HISTORY_FILE = "chat_history.txt";
size_t BUFFER_SIZE = 256;

void log_message(const std::string &message)
{
    std::ofstream history(HISTORY_FILE, std::ios::app);
    if (history.is_open())
    {
        history << message << std::endl;
    }
    else if(!history.is_open()){
        perror("Error opening file fifo");
        
    }
    if (history.fail()) {
        perror("Error writing to file fifo");
    }
}

void send_message(int fd, const std::string &message)
{
    ssize_t bytes = write(fd, message.c_str(), message.size());
    if(bytes == -1){ //обработка ошибок при записи 
        perror("Error writing to file fifo");
    }
}

std::string get_chat_history()
{
    std::ifstream history(HISTORY_FILE);
    if (!history.is_open()){                    
        return "No history available.";
    }
    
    std::string line, full_history;
    while (std::getline(history, line)){
        full_history += line +'\n';
    }
    return full_history;
}

std::string strip(std::string &message)
{   
    while (message.length() > 0 && (message [0] == ' ' or message[message.length() - 1] == ' ')){
        if (message.length() > 0 && message [0] == ' ')
            message.erase (0, 1);
        if (message [message.length() - 1] == ' ')
            message.erase(message.length() - 1, 1);
    }
    return message;
}

int main(){  
    char buffer[BUFFER_SIZE];
    int fifo_fd;
    // Словарь запросов и ответов
    std::map<std::string, std::string> responses = {
        {"Hello", "Hello"},
        {"How are you?", "Good! How are you?"},
        {"Bye", "Bye"},
        {"ping", "pong"},
        {"Good", "I'm happy for you"},
        {"good", "I'm happy for you"},
        {"Ok", "I'm happy for you"},
        {"ok", "I'm happy for you"},
        {"Bad", "I'm sorry to hear that"},
        {"bad", "I'm sorry to hear that"}
    };

    if(mkfifo(FIFO_PATH,0666) == -1){ // обработка ошибок при создании
        if(errno != EEXIST){
             perror("Error creating fifo file");
             return 1;
        }
    }

    

    std::cout << "Server is running. Waiting for messages..." << std::endl;

    while (true){
        
        fifo_fd = open(FIFO_PATH, O_RDONLY);                        // fcntl.h

        if (fifo_fd < 0){
            perror("Error opening fifo file");
            close(fifo_fd);  
            break;
        }
        

        memset(buffer, 0, BUFFER_SIZE);                             // мб неправильно очистка буфера работает, с запросом длины 0 какие то проблемы
        ssize_t bytes_read = read(fifo_fd, buffer, BUFFER_SIZE);    // unistd.h
        if(bytes_read == -1){ // Обработка ошибок записи 
            perror("Error reading fifo file");
            close(fifo_fd);  
            break;
        }
        else if(bytes_read == 0){//клиент вышел
            remove(HISTORY_FILE); 
            close(fifo_fd); 
            break;

        }
        close(fifo_fd);                                             // unistd.h
        
        if (bytes_read > 0){                                        // исправить для 0
            
            if(bytes_read < BUFFER_SIZE){ //преобразование из массива в строку может добавить лишние символы без символа конца строки 
                buffer[bytes_read] = '\0';

            }
            else{
                buffer[bytes_read-1] = '\0';
            }
            std::string message(buffer);
 
            message = strip(message);                               // убираем лишние пробелы справа и слева

            // Логика обработки сообщений
            bool flag = false;
            fifo_fd = open(FIFO_PATH, O_WRONLY);
            if (fifo_fd < 0) {
                perror("Error opening fifo file");
                close(fifo_fd);  
                break;
            }
            for (const auto& pair : responses) {
                if (message.find(pair.first) != std::string::npos) {
                    std::cout << "Received: " << message << std::endl;
                    send_message(fifo_fd, pair.second);
                    log_message("Client: " + message);
                    log_message("Server: " + pair.second);
                    flag = true;
                    break; 
                }
            }
            if(flag!=1){
                if (message == "history"){
                    std::cout << "Received: history request" << std::endl;
                    std::string history = get_chat_history();
                    send_message(fifo_fd, '\n'+history);
                    log_message("Client: history");
                }
                else{
                    std::cout << "Received: " << message << std::endl;
                    log_message("Client: " + message);
                    send_message(fifo_fd, "I don't understand you");
                    log_message("Server: I don't understand you");

                }

            }
        }
        close(fifo_fd);
        
    }
    
    unlink(FIFO_PATH);                  // Удаляем FIFO             // unistd.h
    return 0;
}
