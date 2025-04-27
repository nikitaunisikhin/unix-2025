#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define DEFAULT_BLOCK_SIZE 4096

// DEFINE ERRORS
#define ERROR_UNKNOWN_PATAM "INVALID INPUT: Unknown parameter\n"
#define ERROR_BLOCK_SIZE_MUST_BE_POSITIVE "INVALID INPUT: Block size must be positive\n"
#define ERROR_CAN_NOT_OPEN_OUTPUT_FILE "ERROR: Can't open output file\n"
#define ERROR_CAN_NOT_OPEN_INPUT_FILE "ERROR: Can't open input file\n"
#define ERROR_NOT_CORRECTLY_ARGS_COUNT "INVALID INPUT: There must be 1 or 2 arguments\n"
#define ERROR_MEMORY_ALLOCATION_ERROR "ERROR: Memory allocation error\n"
#define ERROR_LSEEK "ERROR: lseek error\n"
#define ERROR_CAN_NOT_WRITE_IN_FILE "ERROR: Can't write in file\n"
#define ERROR_CAN_NOT_READ_IN_FILE "ERROR: Can't read in file\n"
#define ERROR_FTRUNCATE "ERROR: ftruncate error\n"


int main(int argc, char *argv[]) {
    int block_size = DEFAULT_BLOCK_SIZE;
    
    int opt;
    while ((opt = getopt(argc, argv, "b:")) != -1) {
        switch (opt) {
            case 'b':
                block_size = atoi(optarg);
                if (block_size <= 0) {
                    fprintf(stderr, ERROR_BLOCK_SIZE_MUST_BE_POSITIVE);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, ERROR_UNKNOWN_PATAM);
                exit(EXIT_FAILURE);
        }
    }
    
    int input_fd, output_fd;
    const char *input_file, *output_file;
    int args_count = argc - optind;
    
    switch (args_count) {
        case 1:
            input_fd = STDIN_FILENO;
            output_file = argv[optind];
            break;
        case 2:
            input_file = argv[optind];
            output_file = argv[optind + 1];
            input_fd = open(input_file, O_RDONLY);
            if (input_fd == -1) {
                fprintf(stderr, ERROR_CAN_NOT_OPEN_INPUT_FILE);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            fprintf(stderr, ERROR_NOT_CORRECTLY_ARGS_COUNT);
            exit(EXIT_FAILURE);
    }

    output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (output_fd == -1) {
        fprintf(stderr, ERROR_CAN_NOT_OPEN_OUTPUT_FILE);
        exit(EXIT_FAILURE);
    }

    unsigned char *buffer = malloc(block_size);
    if (!buffer) {
        fprintf(stderr, ERROR_MEMORY_ALLOCATION_ERROR);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read;
    off_t size = 0;

    while ((bytes_read = read(input_fd, buffer, block_size)) > 0) {
        int not_zero_block = 0;
        for (size_t i = 0; i < bytes_read; i++) {
            if (buffer[i] != 0) {
                not_zero_block = 1;
                break;
            }
        }
        if (not_zero_block == 0) {
            if (lseek(output_fd, bytes_read, SEEK_CUR) == -1) {
                free(buffer);
                fprintf(stderr, ERROR_LSEEK);
                exit(EXIT_FAILURE);
            }
        } else {
            ssize_t bytes_write = write(output_fd, buffer, bytes_read);
            if (bytes_write != bytes_read) {
                free(buffer);
                fprintf(stderr, ERROR_CAN_NOT_WRITE_IN_FILE);
                exit(EXIT_FAILURE);
            }
        }
        size += bytes_read;
    }
    
    free(buffer);

    if (bytes_read == -1) {
        fprintf(stderr, ERROR_CAN_NOT_READ_IN_FILE);
        exit(EXIT_FAILURE);
    }
    
    if (ftruncate(output_fd, size) == -1) {
        fprintf(stderr, ERROR_FTRUNCATE);
        exit(EXIT_FAILURE);
    }
    
    if (input_fd != STDIN_FILENO) {
        close(input_fd);
    }
    close(output_fd);
    
    return 0;
}
