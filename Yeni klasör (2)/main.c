#include "shell.h"
#include "pipe.h"
#include "background.h"
#include "io.h" // Increment fonksiyonunu ekliyoruz
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> // Dosya işlemleri için
#include <sys/types.h>
#include <sys/wait.h>

extern int job_count; // job_count başka bir dosyada tanımlı

// Komutları noktalı virgüle göre ayrıştırır
void parse_commands(const char *input, char **commands) {
    char *token;
    char *input_copy = strdup(input); // Girdinin bir kopyasını oluştur
    int i = 0;

    token = strtok(input_copy, ";"); // Noktalı virgüle göre ayrıştır
    while (token != NULL) {
        commands[i++] = strdup(token); // Komutları diziye ekle
        token = strtok(NULL, ";");
    }
    commands[i] = NULL; // Diziyi NULL ile sonlandır
    free(input_copy); // Belleği serbest bırak
}

int main() {
    char command[1024];
    char *commands[64]; // Noktalı virgülle ayrılmış komutları saklar

    printf("> ");
    fflush(stdout);

    while (fgets(command, sizeof(command), stdin)) {
        command[strcspn(command, "\n")] = '\0'; // Komuttaki yeni satır karakterini kaldır

        if (strcmp(command, "quit") == 0) {
            printf("Waiting for background jobs to complete...\n");
            while (job_count > 0) {
                check_background_jobs();
                sleep(1);
            }
            printf("All background jobs completed. Exiting.\n");
            break;
        }

        if (strlen(command) == 0) {
            printf("> ");
            fflush(stdout);
            continue;
        }

        check_background_jobs();

        // Komutları noktalı virgüle göre ayrıştır
        parse_commands(command, commands);

        // Ayrıştırılmış her komutu sırayla çalıştır
        for (int i = 0; commands[i] != NULL; i++) {
            char *single_command = commands[i];
            if (strstr(single_command, "increment")) {
                // Increment işlemi
                if (strchr(single_command, '<')) {
                    // Dosya giriş yönlendirmesi var
                    char *args[64];
                    char *input_file = NULL;

                    parse_input(single_command, args); // Komutu ayrıştır

                    // Giriş dosyasını bul
                    for (int i = 0; args[i] != NULL; i++) {
                        if (strcmp(args[i], "<") == 0) {
                            if (args[i + 1] != NULL) {
                                input_file = args[i + 1];
                                args[i] = NULL; // `<` ve sonrasını kaldır
                            } else {
                                printf("Hata: Giriş dosyası belirtilmedi.\n");
                                break;
                            }
                        }
                    }

                    // Giriş dosyasını aç ve yönlendir
                    if (input_file != NULL) {
                        int fd = open(input_file, O_RDONLY);
                        if (fd < 0) {
                            perror("Giriş dosyası açılamadı");
                            continue;
                        }
                        dup2(fd, STDIN_FILENO); // Giriş dosyasını stdin olarak yönlendir
                        close(fd);

                        // Increment fonksiyonunu çağır
                        increment();

                        // Standart giriş akışını sıfırla
                        freopen("/dev/tty", "r", stdin);
                    } else {
                        printf("Hata: Giriş dosyası bulunamadı.\n");
                    }
                } else {
                    // Dosya yönlendirmesi yok, doğrudan çalıştır
                    increment();
                }
                
            } else if (strchr(single_command, '|')) {
                handle_pipe(single_command); // Pipe işle
            } else if (strchr(single_command, '>') || strchr(single_command, '<')) {
                execute_command(single_command); // Giriş/Çıkış yönlendirme
            } else {
                execute_command(single_command); // Normal komut
            }
        }

        printf("> ");
        fflush(stdout);

        // Belleği temizle
        for (int i = 0; commands[i] != NULL; i++) {
            free(commands[i]);
        }
    }

    return 0;
}
