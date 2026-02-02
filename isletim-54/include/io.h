#ifndef IO_H
#define IO_H

void handle_io_redirection(const char *command);
void parse_redirection(const char *command, char **args, char **file, const char *symbol);
void increment() ;
#endif // IO_H