#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "shell.h"
#include "background.h"
#include "io.h"
#include "pipe.h"

// Kullanıcı girdisini ayrıştırma
void parse_input(const char *input, char **args) {
    char *token;
    char *input_copy = strdup(input); // Girdinin bir kopyasını oluştur
    int i = 0;

    token = strtok(input_copy, " \t"); // Boşluk ve tablara göre ayrıştır
    while (token != NULL) {
        args[i] = strdup(token); // Belleği doğru şekilde kopyala
        i++;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL; // Diziyi NULL ile sonlandır
    free(input_copy); // Belleği serbest bırak
}

// Komutları çalıştırma
void execute_command(const char *command) {
    if (strchr(command, '|')) {
        handle_pipe(command); // Pipe komutlarını ele al
    } else if (strchr(command, '>') || strchr(command, '<')) {
        handle_io_redirection(command); // Giriş-çıkış yönlendirme
    } else if (command[strlen(command) - 1] == '&') {
        run_in_background(command); // Arka plan işlemi
    } else {
        char *args[64];
        char *input_file = NULL, *output_file = NULL;

        // Komut girdisini ayrıştır
        parse_input(command, args);

    
        // Komut boşsa bir şey yapma
        if (args[0] == NULL) {
            printf("No command entered.\n");
            return;
        }

        // Giriş ve çıkış yönlendirme kontrolü
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "<") == 0) { // Giriş yönlendirme
                if (args[i + 1] != NULL) {
                    input_file = args[i + 1];
                    args[i] = NULL; // `<` ve sonrasını kaldır
                } else {
                    printf("Syntax error: no input file specified.\n");
                    return;
                }
            } else if (strcmp(args[i], ">") == 0) { // Çıkış yönlendirme
                if (args[i + 1] != NULL) {
                    output_file = args[i + 1];
                    args[i] = NULL; // `>` ve sonrasını kaldır
                } else {
                    printf("Syntax error: no output file specified.\n");
                    return;
                }
            }
        }

        // Yeni bir süreç oluştur
        pid_t pid = fork();
        if (pid == 0) { // Çocuk süreç
            // Giriş dosyasını yönlendir
            if (input_file != NULL) {
                int fd = open(input_file, O_RDONLY);
                if (fd < 0) {
                    perror("Dosya bulunamadı");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO); // Standart giriş
                close(fd);
            }

            // Çıkış dosyasını yönlendir
            if (output_file != NULL) {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("Error opening output file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO); // Standart çıkış
                close(fd);
            }

            // Komutu çalıştır
            if (execvp(args[0], args) == -1) {
                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
        } else if (pid > 0) { // Ebeveyn süreç
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("Fork failed");
        }

        // Argümanların belleğini serbest bırak
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
    }
}
