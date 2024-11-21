#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

/*
1. Добавить проверку ошибок (в частности, когда неправильный ввод)
2. Добавить словарик для запроса-ответа
3. Написать CMake
*/



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
}

void send_message(int fd, const std::string &message)
{
    write(fd, message.c_str(), message.size());
}

std::string get_chat_history()
{
    std::ifstream history(HISTORY_FILE);
    if (!history.is_open()){                    // мб проверку удалить
        return "No history available.";
    }

    std::string line, full_history;
    while (std::getline(history, line)){
        full_history += line + "\n";
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
    mkfifo(FIFO_PATH, 0666);                                        // sys/stat.h

    char buffer[BUFFER_SIZE];
    int fifo_fd;

    std::cout << "Server is running. Waiting for messages..." << std::endl;

    while (true){
        fifo_fd = open(FIFO_PATH, O_RDONLY);                        // fcntl.h

        if (fifo_fd < 0){
            perror("Error opening FIFO");
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE);                             // мб неправильно очистка буфера работает, с запросом длины 0 какие то проблемы
        ssize_t bytes_read = read(fifo_fd, buffer, BUFFER_SIZE);    // unistd.h
        close(fifo_fd);                                             // unistd.h

        if (bytes_read > 0){                                        // исправить для 0
            std::string message(buffer);

            message = strip(message);                               // убираем лишние пробелы справа и слева

            // Логика обработки сообщений
            if (message == "ping"){
                std::cout << "Received: ping" << std::endl;
                log_message("Client: ping");

                fifo_fd = open(FIFO_PATH, O_WRONLY);
                send_message(fifo_fd, "pong");
                close(fifo_fd);

                log_message("Server: pong");
            }

            else if (message == "history"){
                std::cout << "Received: history request" << std::endl;

                std::string history = get_chat_history();
                fifo_fd = open(FIFO_PATH, O_WRONLY);
                send_message(fifo_fd, history);
                close(fifo_fd);

                log_message("Client: history");
            }

            else if (message == "exit"){
                std::cout << "Client disconnected. Exiting..." << std::endl;
                remove(HISTORY_FILE);                               // либо удалять, либо очищать. Я решила удалять
                break;
            }

            else{                                                   // общий случай
                std::cout << "Received: " << message << std::endl;
                log_message("Client: " + message);

                fifo_fd = open(FIFO_PATH, O_WRONLY);
                send_message(fifo_fd, "hi");
                close(fifo_fd);

                log_message("Server: hi");
            }
        }
        // else if (bytes_read == 0) {
        //     fifo_fd = open(FIFO_PATH, O_WRONLY);
        //     send_message(fifo_fd, "   ");
        //     close(fifo_fd);
        // }
    }

    unlink(FIFO_PATH);                  // Удаляем FIFO             // unistd.h
    return 0;
}
