#ifndef WRC_SERVER_H
#define WRC_SERVER_H

/// HTTP Header
struct header {
    char* name;             //!< Header identifier
    char* value;            //!< Header value
};

typedef struct header header_t;


/// HTTP Request
struct request {
    char* method;           //!< Method which defines the operation the client wants to perform
    char* path;             //!< Path to the resource which is requested
    char* args;             //!< Path to the resource which is requested
    char* proto;            //!< Version of the protocol
    header_t headers[20];   //!< Optional headers which convey additional information about the request
};

typedef struct request request_t;


/// HTTP Response
struct response {
    char* proto;            //!< Version of the protocol
    int status_code;        //!< Code which indicates if request was successful
    char* status_msg;       //!< Short description about status code
    char* body;             //!< A body containing the fetched response
    header_t headers[20];   //!< Optional headers which convey additional information about the response
};

typedef struct response response_t;



/// Handle the request for the specific client slot
void handle_request(
    int client              //!< Client slot index
);

/// Creates a new server on a given port
void server_init(
    int port                //!< Port on which server will be run
);

/// Listens on a given port for requests
void server_listen(
    int port                //!< Port on which server will be run
);

#endif
