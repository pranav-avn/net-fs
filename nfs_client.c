#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "protocol.h"

#define SERVER_IP "127.0.0.1" //localhost for testing
#define SERVER_PORT 8080

int send_receive_request(int client_fd, const client_request_t *request, server_response_t *response) {
    ssize_t bytes_sent;
    ssize_t bytes_received;

    //send the request structure
    bytes_sent = send(client_fd, request, sizeof(client_request_t), 0);
    if (bytes_sent < 0) {
        perror("Client: Error sending request");
        return -1;
    }
    if (bytes_sent != sizeof(client_request_t)) {
        fprintf(stderr, "Client: Warning - Sent partial request header.\n");
    }

    // wait for the response structure
    bytes_received = recv(client_fd, response, sizeof(server_response_t), 0);
    
    if (bytes_received < 0) {
        perror("Client: Error receiving response");
        return -1;
    }
    if (bytes_received == 0) {
        fprintf(stderr, "Client: Server closed connection unexpectedly.\n");
        return -1;
    }
    if (bytes_received != sizeof(server_response_t)) {
        fprintf(stderr, "Client: Warning - Received partial response header (%zd bytes).\n", bytes_received);
    }
    return 0;
}

void test_read(int client_fd) {
    client_request_t request;
    server_response_t response;
    
    printf("\n--- Testing OP_READ ---\n");

    memset(&request, 0, sizeof(request));
    request.request_id = 101;
    request.operation = OP_READ;
    
    strncpy(request.payload.file_op.path, "testfile.txt", MAX_PATH - 1);
    request.payload.file_op.offset = 0; 
    request.payload.file_op.length = 100; // Request up to 100 bytes
    
    if (send_receive_request(client_fd, &request, &response) != 0) {
        fprintf(stderr, "READ test failed during communication.\n");
        return;
    }

    if (response.status == STATUS_OK) {
        printf("Server Response: SUCCESS (ID: %u)\n", response.request_id);
        printf("Bytes Read: %zu\n", response.data_length);
        
        size_t print_len = (response.data_length < MAX_DATA) ? response.data_length : MAX_DATA;
        response.data[print_len] = '\0'; 
        
        printf("File Content:\n--- START ---\n%s\n--- END ---\n", response.data);
    } else {
        fprintf(stderr, "Server Response: FAILURE (Status: %d)\n", response.status);
    }
}

void test_write(int client_fd) {
    client_request_t request;
    server_response_t response;
    const char *data_to_write = "New data written by the client!";
    size_t data_len = strlen(data_to_write);
    
    printf("\n--- Testing OP_WRITE (Writing to 'newfile.txt') ---\n");
    
    memset(&request, 0, sizeof(request));
    request.request_id = 202;
    request.operation = OP_WRITE;
    
    strncpy(request.payload.file_op.path, "newfile.txt", MAX_PATH - 1);
    request.payload.file_op.offset = 0; 
    request.payload.file_op.length = data_len;

    if (send(client_fd, &request, sizeof(client_request_t), 0) < 0) {
        perror("Client: Error sending WRITE request header");
        return;
    }
    
    if (send(client_fd, data_to_write, data_len, 0) < 0) {
        perror("Client: Error sending WRITE data payload");
        return;
    }
    
    if (recv(client_fd, &response, sizeof(server_response_t), 0) <= 0) {
        perror("Client: Error receiving WRITE response");
        return;
    }

    if (response.status == STATUS_OK) {
        printf("Server Response: SUCCESS (ID: %u)\n", response.request_id);
        printf("Bytes Confirmed Written: %zu\n", response.data_length);
    } else {
        fprintf(stderr, "Server Response: FAILURE (Status: %d)\n", response.status);
    }
}

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
    if(inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0){
        perror("Invalid address/ Address not supported");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    //connect to server
    if(connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Connection to server failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);
    test_read(client_fd);
    
    // Second, test writing new content to a file on the server
    //test_write(client_fd);
    
    // 3. Clean up
    // Step 6: Close the connection
    printf("\nClient: All tests complete. Closing connection.\n");
    close(client_fd); 
    return 0;
}