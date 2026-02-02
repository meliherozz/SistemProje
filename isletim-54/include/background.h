#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <sys/types.h>

typedef struct {
    pid_t pid;
    char command[1024];
} Job;
void handle_sigchld(int sig);
extern int job_count;
void wait_for_all_background_jobs(); 
void check_background_jobs();
void run_in_background(const char *command);

#endif
