#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <thread>
#include <cerrno>

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

int main() {
    
    char buffer[256];

    // создаем сокет и проверяем на ошибку создания 
    int server_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sd == -1) { //SOCk_STREAM-указывает на тип сокета TCP 0-ос сама выбирает тип протокола
        perror("Socket creation error");
        return 1;
    }

    
    sockaddr_in address; // sin_family - домен(семейство адресов) - IPv4 - 32 бит; s_addr - адрес без знаков, sin_port - номер порта 
    address.sin_family = AF_INET; //указываем семейство адресов
    address.sin_port = htons(8080);//htons преобразует из машинного порядка байтов в сетевой 
    address.sin_addr.s_addr = inet_addr("127.0.0.1");

    //привязываем сокет к адресу и порту, который станет точкой прослушивания сооббщений и отправки сообщений 
    if (bind(server_sd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("Socket binding error");
        return 1;
    }

    // переводим сокет в состояние прослушивания 
    if (listen(server_sd, 1) < 0) {
        perror("Socket listening error");
        return 1;
    }

    std::cout << "Server listening on port 8080...\n";

    // принимаем запросы на подключение и проверяем на ошибку 
    
    socklen_t addrlen = sizeof(address);
    
    int client_sd = accept(server_sd, (struct sockaddr *)&address, &addrlen);
    if (client_sd == -1) {
        perror("Socket accepting error");
        return 1;
    }

    std::cout << "Connection accepted.\n";

    while (true) {
        memset(buffer, 0, 256);
        // читаем сообщение и обрабатываем ошибки 
        int bytes = read(client_sd, buffer, 256);
        if (bytes == -1) {
            perror("Socket reading error");
            break; 
        } else if (bytes == 0) {
         
            break; //клиент отключился 
        }
        if(bytes < 256){ //преобразование из массива в строку может добавить лишние символы без символа конца строки 
                buffer[bytes] = '\0';

        }
        else{
                buffer[bytes-1] = '\0';
        }

        std::string message(buffer);
        message = strip(message);  
        if (message == "ping") {
            std::string response = "pong";
            send(client_sd, response.c_str(), response.length(), 0);
            std::cout << "Received: " << message << std::endl;
        } else {
             std::string response = "I don't understand you";
            send(client_sd, response.c_str(), response.length(), 0);
            std::cout << "Received: " << message << std::endl;
            
        }
    }

    close(client_sd);
    close(server_sd);
    return 0;
}
