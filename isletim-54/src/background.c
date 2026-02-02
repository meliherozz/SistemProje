#include "background.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>


#define MAX_JOBS 128



Job jobs[MAX_JOBS];
int job_count = 0; // Arka plan işlemlerini takip eden sayaç



    void handle_sigchld(int sig) {
    int status;
    pid_t pid;

    // Tamamlanan tüm işlemleri kontrol et
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < job_count; i++) {
            if (jobs[i].pid == pid) {
                printf("[%d] retval: %d\n", pid, WEXITSTATUS(status));
                fflush(stdout);
                // Tamamlanan işi kaldır
                jobs[i] = jobs[job_count - 1];
                job_count--;
                break;
            }
        }
    }
}

void check_background_jobs() {
    signal(SIGCHLD, handle_sigchld); // SIGCHLD sinyalini yakala
}

void run_in_background(const char *command) {
    char *cmd = strdup(command);
    cmd[strlen(cmd) - 1] = '\0'; // '&' işaretini kaldır
    char *args[64];
    parse_input(cmd, args);

    pid_t pid = fork();
    if (pid == 0) { // Çocuk süreç
        if (execvp(args[0], args) == -1) {
            perror("Error executing command");
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) { // Ebeveyn süreç
        printf("[Process running in background] PID: %d\n", pid);
        fflush(stdout);
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].command, cmd, sizeof(jobs[job_count].command));
        job_count++;
    } else {
        perror("Fork failed");
    }

    free(cmd);
}

void wait_for_all_background_jobs() {
    int status;
    while (job_count > 0) {
        pid_t result = wait(&status); // Tamamlanan herhangi bir süreci bekle
        if (result > 0) {
            printf("[%d] retval: %d\n", result, WEXITSTATUS(status));
            fflush(stdout);
            for (int i = 0; i < job_count; i++) {
                if (jobs[i].pid == result) {
                    jobs[i] = jobs[job_count - 1];
                    job_count--;
                    break;
                }
            }
        }
    }
}

