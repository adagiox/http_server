#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

typedef struct addrinfo s_addrinfo;
typedef struct sockaddr s_sockaddr;
typedef struct sockaddr_in s_sockaddr_in;
typedef struct sockaddr_in6 s_sockaddr_in6;
typedef struct sockaddr_storage s_sa_storage;

enum http_method {
    HTTP_GET,
    HTTP_POST,
};

// no header or body yet
typedef struct http_request {
    enum http_method method;
    char* request_uri;
    char* http_version;
} s_http_request;

typedef struct http_response {
    int http_version;
    int status_code;
    char* reason;
} s_http_response;

int init_listener();

#endif