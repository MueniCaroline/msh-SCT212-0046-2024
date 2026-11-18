#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARGS 128

int parse_line(char *line, char **argv) {
    int argc = 0;
    char *token = strtok(line, " \t\r\n");
    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    argv[argc] = NULL;
    return argc;
}

int main(void) {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int last_status = 0;
    char *argv[MAX_ARGS];

    while (1) {
        printf("msh> ");
        fflush(stdout);

        nread = getline(&line, &len, stdin);
        if (nread == -1) {
            printf("\n");
            break;
        }

        int argc = parse_line(line, argv);
        if (argc == 0) {
            continue;
        }
    }

    free(line);
    return last_status;
}
