#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "pipe.h"
#include "shell.h"

void handle_redirection(char *command, int *out_fd) {
    char *redirect = strchr(command, '>');
    if (redirect) {
        *redirect = '\0'; // Komut ve dosya adını ayır
        char *file_name = strtok(redirect + 1, " \t\n");
        if (file_name) {
            *out_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (*out_fd == -1) {
                perror("Failed to open output file");
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "Syntax error: missing file name after '>'\n");
            exit(EXIT_FAILURE);
        }
    }
}


void handle_pipe(const char *command) {
    char *cmds[64];
    int pipefd[2], in_fd = STDIN_FILENO, out_fd = STDOUT_FILENO;
    int i = 0;

    // Komutları boru sembolüne göre ayır
    char *input_copy = strdup(command);
    char *token = strtok(input_copy, "|");
    while (token != NULL) {
        cmds[i++] = strdup(token);
        token = strtok(NULL, "|");
    }
    cmds[i] = NULL;

    for (int j = 0; cmds[j] != NULL; j++) {
        char *args[64];
        if (j == i - 1) { // Son komut için yönlendirme kontrolü
            handle_redirection(cmds[j], &out_fd);
        }
        parse_input(cmds[j], args);

        if (cmds[j + 1] != NULL) { // Ara komutlar için pipe oluştur
            if (pipe(pipefd) == -1) {
                perror("Pipe failed");
                exit(EXIT_FAILURE);
            }
        }

        pid_t pid = fork();
        if (pid == 0) { // Çocuk süreç
            if (in_fd != STDIN_FILENO) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            if (cmds[j + 1] != NULL) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            } else if (out_fd != STDOUT_FILENO) {
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }

            if (execvp(args[0], args) == -1) {
                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
        } else if (pid > 0) { // Ebeveyn süreç
            wait(NULL);
            if (in_fd != STDIN_FILENO) {
                close(in_fd);
            }
            if (cmds[j + 1] != NULL) {
                close(pipefd[1]);
                in_fd = pipefd[0];
            }
        } else {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int j = 0; cmds[j] != NULL; j++) {
        free(cmds[j]);
    }
    free(input_copy);
}
