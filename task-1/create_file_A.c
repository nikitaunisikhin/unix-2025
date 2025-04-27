#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define FILE_SIZE 4*1024*1024+1

// DEFINE ERRORS
#define ERROR_CAN_NOT_OPEN_FILE "ERROR: Can't open file\n"
#define ERROR_CAN_NOT_READ_FILE "ERROR: Can't open file\n"
#define ERROR_MEMORY_ALLOCATION_ERROR "ERROR: Memory allocation error\n"

int main() {
    const int size = FILE_SIZE;

    void *buffer = calloc(size, sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, ERROR_MEMORY_ALLOCATION_ERROR);
        return 1;
    }

    *(char *)buffer = 1;
    *((char *)buffer + 9999) = 1;
    *((char *)buffer + size - 1) = 1;
    
    int fd = open("A", O_TRUNC | O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, ERROR_CAN_NOT_OPEN_FILE);
        return 1;
    }
    if (write(fd, buffer, size) == -1) {
        fprintf(stderr, ERROR_CAN_NOT_READ_FILE);
        return 1;
    }

    close(fd);
    free(buffer);

    return 0;
}