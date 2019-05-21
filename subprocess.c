#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "logger.h"
#include "subprocess.h"

/// Close pipe for file descriptor and handle errors
void safe_close(int fd) {
    if (close(fd) == -1) {
        ERROR("Could not close the pipe");
    }
}

/// Open read and write pipes for file descriptor and handle errors
void safe_pipe(int fds[2]) {
    if (pipe(fds) == -1) { 
        ERROR("Could not create the pipe");
    }
}

/// Duplicate descriptors and handle errors
void safe_dup2(int fd1, int fd2) {
    if (dup2(fd1,  fd2) == -1) { 
        ERROR("Could not duplicate the pipe");
    }
    close(fd1);
}

/// Start program as subprocess
void execute(char *argv[], struct subprocess *p) {
    int child_in[2], child_out[2], child_err[2];

    safe_pipe(child_in);
    safe_pipe(child_out);
    safe_pipe(child_err);

    pid_t tmp_pid, pid = fork();

    int retcode;

    if (pid == 0) {
        // If fork is sucessful

        // Close parent's pipes
        safe_close(0);
        safe_close(1);
        safe_close(2);

        // Close unused child's pipes
        safe_close(child_in[1]);
        safe_close(child_out[0]);
        safe_close(child_err[0]);

        // Take control of child's pipes
        safe_dup2(child_in[0], 0);
        safe_dup2(child_out[1], 1);
        safe_dup2(child_err[1], 2);

        // Execute program
        char* envp = { NULL };
        execvpe(argv[0], argv, &envp);
    } else {
        // Else we are in parent process

        // Close unused child's pipes
        safe_close(child_in[0]);
        safe_close(child_out[1]);
        safe_close(child_err[1]);

        // Take control of child's pipes
        p->pid = pid;
        p->stdin = child_in[1];
        p->stdout = child_out[0];
        p->stderr = child_err[0];

        // Wait for program to finish
        tmp_pid = wait(&retcode);
    }
}
