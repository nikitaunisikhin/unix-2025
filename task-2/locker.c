#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RESULT_FILE_NAME "result.txt"

// DEFINE ERRORS
#define ERROR_UNKNOWN_PARAM "INVALID INPUT: Unknown parameter\n"
#define ERROR_FILE_NOT_DEFINE "INVALID INPUT: Not defined file name\n"
#define ERROR_MEMORY_ALLOCATION "ERROR: Memory allocation error\n"

volatile int locks_count = 0;

static void sigint_handler() {
    int fd_stat = open(RESULT_FILE_NAME, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    char buffer[50];
    sprintf(buffer, "PID[%d] Locks count: %d\n", getpid(), locks_count);
    write(fd_stat, buffer, strlen(buffer));

    close(fd_stat);
    exit(0);
}

int main(int argc, char *argv[]) {
    char *file_name = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
        case 'f':
            file_name = optarg;
            break;
        default:
            fprintf(stderr, ERROR_UNKNOWN_PARAM);
            exit(EXIT_FAILURE);
        }
    }
    if (!file_name) {
        fprintf(stderr, ERROR_FILE_NOT_DEFINE);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sigint_handler);

    char *filename_lock = malloc((sizeof(char) * strlen(file_name)) + 5);
    if (!filename_lock) {
        fprintf(stderr, ERROR_MEMORY_ALLOCATION);
        exit(EXIT_FAILURE);
    }
    strcpy(filename_lock, file_name);
    strcat(filename_lock, ".lck");

    while (1) {
        usleep(rand() % 1000 * 1000); // 0 to 999 milliseconds

        pid_t pid = getpid();
        int fd_lck = open(filename_lock, O_CREAT | O_EXCL | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd_lck == -1) {
            sleep(1);
            continue;
        }
        write(fd_lck, &pid, sizeof(pid_t));
        close(fd_lck);

        FILE *fd = fopen(file_name, "r+");
        sleep(1);
        fclose(fd);

        fd_lck = open(filename_lock, O_RDONLY);
        if (fd_lck == -1) {
            exit(errno);
        } 
        
        pid_t current_pid = 0;
        read(fd_lck, &current_pid, sizeof(pid_t));
        if (current_pid != pid) {
            close(fd_lck);
            exit(errno);
        }
        
        locks_count++;

        close(fd_lck);         
        remove(filename_lock);
    }

    free(filename_lock);
}