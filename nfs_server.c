#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "protocol.h"

#define SERVER_PORT 8080
#define BACKLOG 10

void process_client_req(int client_fd){
    client_request_t req;
    ssize_t bytes_received;

    bytes_received = recv(client_fd, &req, sizeof(client_request_t), 0);
    if(bytes_received <0){
        perror("Receive failed.");
        return;
    }
    if(bytes_received ==0){
        printf("Client disconnected.\n");
        return;
    }

    if(bytes_received != sizeof(client_request_t)){
        fprintf(stderr, "Warning: Incomplete request received.\n");
        return;
    }

    printf("Received request ID: %u, Operation: %d\n", req.request_id, req.operation);
    
    //Process based on operation type
    switch(req.operation){
        case OP_READ:
            printf("READ operation on path: %s, offset: %zu, length: %zu\n",
                req.payload.file_op.path,
                req.payload.file_op.offset,
                req.payload.file_op.length);
            //handle_read(client_fd, &req);
            break;

        case OP_WRITE:
            printf("WRITE operation on path: %s, offset: %zu, length: %zu\n",
                req.payload.file_op.path,
                req.payload.file_op.offset,
                req.payload.file_op.length);
            //handle_write(client_fd, &req);
            break;
        
        case OP_LIST:
            printf("LIST operation on path: %s\n",
                req.payload.list_op.path);
            //handle_list(client_fd, &req);
            break;

        default:
            fprintf(stderr, "Unknown operation type: %d\n", req.operation);
            //send_error_response(client_fd, req.request_id, STATUS_ERROR_UNKNOWN_OP);
            break;
    }
}

int main(){
    int listen_fd, client_fd;
    //create listening socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    //allow address reuse
    int opt = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("Set socket options failed");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    //configure server address struct
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; //bind to all interfaces
    server_addr.sin_port = htons(SERVER_PORT);

    //bind socket to address
    if(bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Bind failed");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    //start listening for connections
    if(listen(listen_fd, BACKLOG) < 0){
        perror("Listen failed");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", SERVER_PORT);
    //accept a client connection
    struct sockaddr_in client_addr;
    socklen_t client_len;
    
    while(1){
        client_len = sizeof(client_addr);
        client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
        if(client_fd < 0){
            perror("Accept failed");
            continue; //continue to accept next connection
        }
        printf("Client connected. Client FD: %d\n", client_fd);
        
        
        //Handle client requests



        close(client_fd); //close client connection after handling
        printf("Client disconnected\n");

    }


    return 0;
}