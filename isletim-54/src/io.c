#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "io.h"
#include "shell.h"
#include <sys/wait.h>


void handle_io_redirection(const char *command) {
    char *args[64];
    char *input_file = NULL;
    char *output_file = NULL;

    // Komutu ayrıştır
    parse_input(command, args);

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) { // Çıkış yönlendirme
            output_file = args[i + 1];
            args[i] = NULL; // `>` ve sonrasını kaldır
        } else if (strcmp(args[i], "<") == 0) { // Giriş yönlendirme
            input_file = args[i + 1];
            args[i] = NULL; // `<` ve sonrasını kaldır
        }
    }

    pid_t pid = fork();
    if (pid == 0) { // Çocuk süreç
        // Giriş yönlendirme
        if (input_file != NULL) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Çıkış yönlendirme
        if (output_file != NULL) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Komut çalıştır
        if (execvp(args[0], args) == -1) {
            perror("Error executing command");
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) { // Ebeveyn süreç
        wait(NULL);
    } else {
        perror("Fork failed");
    }
}


void parse_redirection(const char *command, char **args, char **file, const char *symbol) {
    char *input_copy = strdup(command);
    char *token = strtok(input_copy, symbol);
    char *cmd = token;
    *file = strtok(NULL, " ");

    parse_input(cmd, args);
    free(input_copy);
}

void increment() {
    int num;

    // Standart girişten bir tamsayı oku
    if (scanf("%d", &num) == 1) {
        printf("%d\n", num + 1); // 1 artır ve yazdır
    } else {
        fprintf(stderr, "Hata: Giriş bir tamsayı olmalıdır.\n");
        exit(EXIT_FAILURE);
    }
}
