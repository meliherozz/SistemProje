#ifndef PIPE_H
#define PIPE_H

void handle_pipe(const char *command);
void handle_redirection(char *command, int *out_fd);
#endif // PIPE_H