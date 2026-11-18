#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

int handle_builtin(char **argv, char *line, int *last_status) {
    if (strcmp(argv[0], "exit") == 0) {
        free(line);
        exit(*last_status);
    }
    if (strcmp(argv[0], "help") == 0) {
        printf("msh - minimal Unix shell\n");
        printf("Built-in commands:\n");
        printf("  cd [dir]   Change current working directory (defaults to $HOME)\n");
        printf("  help       Display this help message\n");
        printf("  exit       Exit the shell\n");
        *last_status = 0;
        return 1;
    }
    if (strcmp(argv[0], "cd") == 0) {
        const char *target = argv[1] ? argv[1] : getenv("HOME");
        if (target == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            *last_status = 1;
        } else if (chdir(target) != 0) {
            perror("cd");
            *last_status = 1;
        } else {
            *last_status = 0;
        }
        return 1;
    }
    return 0;
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

        if (handle_builtin(argv, line, &last_status)) {
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            last_status = 1;
        } else if (pid == 0) {
            execvp(argv[0], argv);
            perror(argv[0]);
            _exit(127);
        } else {
            int status;
            if (waitpid(pid, &status, 0) < 0) {
                perror("waitpid");
                last_status = 1;
            }
        }
    }

    free(line);
    return last_status;
}
