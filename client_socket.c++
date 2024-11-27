#include <iostream> 
#include <string>  
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <unistd.h>     
#include <cstring>      
#include <cerrno>

int main() {

    
    char buffer[256];  
    
    

    // создаем сокет и обрабатываем ошибки 
    int client_sd = socket(AF_INET, SOCK_STREAM,0);
    if (client_sd == -1) { 
        perror("Socket creation error");
        return 1; 
    }
    sockaddr_in serv_addr;   
    serv_addr.sin_family = AF_INET;   
    serv_addr.sin_port = htons(8080); 
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
   

    
    // соединение с сервером 
    if (connect(client_sd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) { 
        perror("Socket connection error");
        return 1; 
    }

    while(true){
        std::cout << "Enter message: "; 
        std::cin.getline(buffer, 256); 
        buffer[256] = '\0';      
        std::string message(buffer);  
        if(message == "exit") {
            break;
        }      
        send(client_sd, message.c_str(), message.length(), 0);  
        memset(buffer, 0, 256);                
        int bytes_read = read(client_sd, buffer,256 ); 
        if ( bytes_read == -1) {
             perror("Socket reading error");
        } else if (bytes_read == 0) {
              std::cout << "Server disconnected.\n"; 
              break;
        } else {
              buffer[256] = '\0'; 
              std::string response(buffer);
              std::cout << "Server: " << response << std::endl; 
        }


    }

    
    close(client_sd); 
    return 0;   
}
