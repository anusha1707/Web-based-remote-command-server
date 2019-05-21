#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include "server.h"
#include "logger.h"

#define MAXCONN  1000

struct sockaddr_in haddr;
static int server_fd, client_fd;
static int clients[MAXCONN];

char *res_fmt =
    "<!doctype html>"
    "<html>"
    "   <head>"
    "       <style>"
    "           body {width:700px;margin:40px auto 0 auto;}"
    "           #command {width:100%;}"
    "           section {margin:20px 0;}"
    "           textarea {font-family:\'Consolas\';font-size:14px;font-size:14px;width:100%;border:none;}"
    "       </style>"
    "   </head>"
    "   <body>"
    "       <form action=\"/\" method=\"get\">"
    "           Command:<br>"
    "           <input type=\"text\" name=\"command\" id=\"command\"><br>"
    "           <input type=\"submit\" value=\"Run\">"
    "       </form>"
    "       <section>"
    "           <span>Command that was run:</span>"
    "           <textarea>%s</textarea>"
    "       </section>"
    "       <section>"
    "           <span>Standard Output:</span>"
    "           <textarea>%s</textarea>"
    "       </section>"
    "       <section>"
    "           <span>Standard Error:</span>"
    "           <textarea>%s</textarea>"
    "       </section>"
    "       <script>document.querySelectorAll('textarea').forEach(function(ta){ta.style.height=0;ta.style.height=ta.scrollHeight+\"px\";});</script>"
    "   </body>"
    "</html>";

void handle_command();


int ishex(int x) {
	return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F');
}

/// Decode url string
int decode_string(const char *src, char *dest) {
	int c;
	char *o;
	const char *end = src + strlen(src);
 
	for (o = dest; src <= end; o++) {
		c = *src++;
		if (c == '+') c = ' ';
		else if (c == '%' && (!ishex(*src++) ||
					!ishex(*src++) ||
					!sscanf(src - 2, "%2x", &c)))
			return -1;
 
		if (dest) *o = c;
	}

	return o - dest;
}

/// Parse the request
request_t parse_request(int slot) {
    request_t req;
    int rec;

    // Raw request data
    char* req_raw;
    req_raw = malloc(65535);

    rec = recv(clients[slot], req_raw, 65535, 0);

    if (rec < 0) {
        ERROR("recv() error");
    } else if (rec == 0) {
        WARN("Client [%d] disconnected", slot);
    } else {
        req_raw[rec] = '\0';

        req.method = strtok(req_raw, " \t\r\n");
        req.path = strtok(NULL, " \t");
        req.proto = strtok(NULL, " \t\r\n");

        char *encoded_args;
        // Split path to path and args
        if (encoded_args = strchr(req.path, '?')) {
            *encoded_args++ = '\0';
        } else {
            encoded_args = req.path - 1;
        }

        req.args = malloc(strlen(encoded_args) + 1);
        
        if(decode_string(encoded_args, req.args) < 0) {
            WARN("Could not decode the args");
        }

        // Parse headers
        header_t *h = req.headers;
        char *t, *t2;

        while (h < req.headers + 19) {
            char *k, *v, *t;

            k = strtok(NULL, "\r\n: \t");
            if (!k) break;

            v = strtok(NULL, "\r\n");
            while (*v && *v == ' ') v++;

            h->name = k;
            h->value = v;

            h++;
            t = v + 1 + strlen(v);
            if (t[1] == '\r' && t[2] == '\n')
                break;
        }
    }
    return req;
}

/// Return arg with specified key
char* get_arg(char* arg, request_t req) {
    char *arg_tok, *kv_tok;
    char *tmp = strtok_r(req.args, "&", &arg_tok);
    char *key;
    char *value;

    while (tmp != NULL) {
        key = strtok_r(tmp, "=", &kv_tok);
        value = strtok_r(NULL, "=", &kv_tok);
        
        if(!strcmp(key, arg)) {
            return value;
        }

        tmp = strtok_r(NULL, "&", &arg_tok);
    }

    return NULL;
}

/// Handle the request for the specific client slot
void handle_request(int slot) {
    request_t req = parse_request(slot);

    char* command = get_arg("command", req);

    DEBUG("Command request: %s", command);

    handle_command(command, slot);
}

/// Bundle response
void bundle_response(response_t res, int slot) {
    int i;
    
    char* tmp = malloc((
            strlen(res.proto) + 
            strlen(res.status_msg) + 7
            ) * sizeof(char));

    sprintf(tmp, "%s %d %s\n", res.proto, res.status_code, res.status_msg);
    write(clients[slot], tmp, strlen(tmp));

    for (i=0; i < 20; i++) {
        if(res.headers[i].name == NULL && res.headers[i].value == NULL) 
            break;

        sprintf(tmp, "%s: %s\n", res.headers[i].name, res.headers[i].value);
        write(clients[slot], tmp, strlen(tmp));
    }

    write(clients[slot], "\n", 1);
    write(clients[slot], res.body, strlen(res.body));

}

#define PREV_COMMAND "Your server will include the command that was run here."
#define PREV_STDOUT  "Your server will include the stdout results here."
#define PREV_STDERR  "Your server will include the stderr results here."

/// Return to client
void respond(int slot, char* cmd, char* out, char* err) {
    response_t res;
    
    if (cmd != NULL) {
        res.body = malloc((
            strlen(res_fmt) + 
            strlen(cmd) + 
            strlen(out) + 
            strlen(err) - 5 // 3 * '%s' + 1  = 5
            ) * sizeof(char));

        sprintf(res.body, res_fmt, cmd, out, err);
    } else {
        res.body = malloc((
            strlen(res_fmt) + 
            strlen(PREV_COMMAND) + 
            strlen(PREV_STDOUT) + 
            strlen(PREV_STDERR) - 5 // 3 * '%s' + 1  = 5
            ) * sizeof(char));

        sprintf(res.body, res_fmt, PREV_COMMAND, PREV_STDOUT, PREV_STDERR);
    }
    long n = strlen(res.body), nn;
    while(n!=0) { n /= 10; ++nn; }
    res.headers[2].value = malloc((nn+1) * sizeof(char));

    res.proto = "HTTP/1.1";
    res.status_code = 200;
    res.status_msg = "OK";
    res.headers[0].name = "Server";
    res.headers[0].value = "WRC Server";
    res.headers[1].name = "Content-Type";
    res.headers[1].value = "text/html";
    res.headers[2].name = "Content-Length";
    sprintf(res.headers[2].value, "%ld", strlen(res.body));

    bundle_response(res, slot);

    close(clients[slot]);
    clients[slot] = -1;
}


/// Initialize a new server on a given port
void server_init(int port) {
    DEBUG("Initializing a new server instance on port: %d", port);

    int addrlen = sizeof(haddr);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        ERROR("socket() could not be opened");
    }

    DEBUG("Socket created successfully");

    haddr.sin_family = AF_INET;
    haddr.sin_addr.s_addr = INADDR_ANY;
    haddr.sin_port = htons(port);

    memset(haddr.sin_zero, '\0', sizeof haddr.sin_zero);

    DEBUG("Binding the server");
    if (bind(server_fd, (struct sockaddr *)&haddr, sizeof(haddr)) != 0) {
        ERROR("bind() returned an invalid code");
    }

    DEBUG("Listening for incoming connections");
    if (listen(server_fd, MAXCONN) != 0) {
        ERROR("listen() returned an invalid code");
    }

    DEBUG("Server initialization succeeded");
}


/// Listens on a given port for requests
void server_listen(int port) {
    struct sockaddr_in clientaddr;
    int addrlen = sizeof(clientaddr);

    int client_slot = 0;

    // Setting all client slots to -1, which indicates that they are not filled
    int i;
    for (i=0; i<MAXCONN; i++)
        clients[i] = -1;

    server_init(port);

    INFO("Server listening on: http://localhost:%d", port);

    while(1) {
        if ((clients[client_slot] = accept(server_fd, (struct sockaddr *)&clientaddr, (socklen_t*)&addrlen)) < 0) {
            ERROR("accept() returned an invalid code");
        }

        // Fork the client response
        if (fork() == 0) {
            handle_request(client_slot);
            exit(0);
        }

        // Iterate through client slots until empty is found
        while (clients[client_slot] != -1) 
            client_slot = (client_slot+1) % MAXCONN;
    }
}
