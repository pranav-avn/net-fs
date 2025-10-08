#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "protocol.h"

#define SERVER_IP "127.0.0.1" //localhost for testing
#define SERVER_PORT 8080

int main(){
    //Create client file descriptor
    int client_fd;
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    //configure server address struct
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    return 0;
}