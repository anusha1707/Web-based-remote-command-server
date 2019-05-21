#ifndef WRC_SUBPROCESS_H
#define WRC_SUBPROCESS_H

#include <unistd.h>


/// Structure for subprocess control
struct subprocess {
    pid_t pid;              //!< Process ID, generated by fork()
    int stdin;              //!< File descriptor for standard input
    int stdout;             //!< File descriptor for standard output
    int stderr;             //!< File descriptor for standard error
};

typedef struct subprocess subprocess_t;


/// Close pipe for file descriptor and handle errors
void safe_close(
    int fd                  //!< File descriptor
);

/// Open read and write pipes for file descriptor and handle errors
void safe_pipe(
    int fds[2]              //!< File descriptors
);

/// Duplicate descriptors and handle errors
void safe_dup2(
    int fd1,                //!< Source file descriptor
    int fd2                 //!< Destination file descriptor
);

/// Start program as subprocess
void execute(
    char *argv[],           //!< Program arguments, first argument is the name of the program
    struct subprocess *p    //!< Reference of the subprocess instance
);

#endif
