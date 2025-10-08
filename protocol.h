#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_PATH 256
#define MAX_DATA 1024

// Define operation types
typedef enum {
    OP_READ,
    OP_WRITE,
    OP_LIST,
    OP_ERROR
} operation_t;

//Client Request Structure
typedef struct {
    uint32_t request_id; // Unique request ID
    operation_t operation; // Operation type
    union{
        struct{
            char path[MAX_PATH];
            size_t offset;
            size_t length; //for READ: number of bytes to read, for WRITE: number of bytes to write
        } file_op;
        struct{
            char path[MAX_PATH];
        } list_op;
    } payload;
} client_request_t;

#endif // PROTOCOL_H
