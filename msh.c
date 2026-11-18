#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int last_status = 0;

    while (1) {
        printf("msh> ");
        fflush(stdout);

        nread = getline(&line, &len, stdin);
        if (nread == -1) {
            printf("\n");
            break;
        }
    }

    free(line);
    return last_status;
}
