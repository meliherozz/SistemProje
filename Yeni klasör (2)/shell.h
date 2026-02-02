#ifndef SHELL_H
#define SHELL_H

// Kullanıcı komutlarını çalıştırır
void execute_command(const char *command);

// Kullanıcı girdisini temizler
void sanitize_input(char *input);

// Komutları ayrıştırır
void parse_input(const char *input, char **args);

#endif 
