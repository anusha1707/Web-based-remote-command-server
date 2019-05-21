#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "server.h"
#include "subprocess.h"

void respond(int slot, char* cmd, char* out, char* err);

void handle_command(char* command, int client) {
	INFO("Executing: sh -c \"%s\"", command);

	char* args[] = { "sh", "-c", command, NULL };
    struct subprocess proc;
    execute(args, &proc);

    char buff_out[10240];
    int recout = read(proc.stdout, buff_out, sizeof(buff_out));

    char buff_err[10240];
    int recerr = read(proc.stderr, buff_err, sizeof(buff_err));

	respond(client, command, buff_out, buff_err);
}

int main(int argc, char const *argv[])
{
    server_listen(3838);

    return 0;
}
