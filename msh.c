#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
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

void reap_background(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("[bg] PID %d finished\n", pid);
    }
}

int main(void) {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int last_status = 0;
    char *argv[MAX_ARGS];

    while (1) {
        reap_background();

        printf("msh[%d]> ", last_status);
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

        int background = 0;
        if (strcmp(argv[argc - 1], "&") == 0) {
            background = 1;
            argv[--argc] = NULL;
        } else {
            size_t last_len = strlen(argv[argc - 1]);
            if (last_len > 1 && argv[argc - 1][last_len - 1] == '&') {
                background = 1;
                argv[argc - 1][last_len - 1] = '\0';
            }
        }
        if (argc == 0) {
            continue;
        }

        if (handle_builtin(argv, line, &last_status)) {
            continue;
        }

        char *outfile = NULL;
        int redir_error = 0;
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], ">") == 0) {
                if (argv[i + 1] == NULL) {
                    fprintf(stderr, "msh: syntax error near unexpected token `newline'\n");
                    last_status = 2;
                    redir_error = 1;
                } else {
                    outfile = argv[i + 1];
                    argv[i] = NULL;
                }
                break;
            }
        }
        if (redir_error) {
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            last_status = 1;
        } else if (pid == 0) {
            if (outfile != NULL) {
                int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open");
                    _exit(1);
                }
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("dup2");
                    close(fd);
                    _exit(1);
                }
                close(fd);
            }

            execvp(argv[0], argv);
            perror(argv[0]);
            _exit(127);
        } else {
            if (background) {
                printf("[bg] PID %d\n", pid);
                last_status = 0;
            } else {
                int status;
                if (waitpid(pid, &status, 0) < 0) {
                    perror("waitpid");
                    last_status = 1;
                } else if (WIFEXITED(status)) {
                    last_status = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    last_status = 128 + WTERMSIG(status);
                }
            }
        }
    }

    reap_background();
    free(line);
    return last_status;
}
