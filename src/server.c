//
// Adapted from Beej's Guide to Network Programming
//

#include "server.h"
#include "files.h"
#include "util.h"

#define PORT "8080"
#define BACKLOG 10

#define SITE_ROOT "/site_content"
#define REL_SITE_ROOT "./site_content"
#define DEFAULT_PAGE "/index.html"
#define TEST_RESPONSE "HTTP/1.1 200 OK\r\n"

void *get_in_addr(s_sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((s_sockaddr_in *)sa)->sin_addr);
    }
    return &(((s_sockaddr_in6 *)sa)->sin6_addr);
}

int init_listener() {
    int yes = 1;
    int status, sockfd, listen_status;
    s_addrinfo hints;
    s_addrinfo *servinfo, *addrs;

    memset(&hints, 0, sizeof(s_addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // get addrinfo
    if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }
    
    //  get sockfd, bind socket, and listen
    for(addrs = servinfo; addrs != NULL; addrs = addrs->ai_next) {
        if((sockfd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol)) == -1)
            continue;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("server: setsockopt");
            return -1;
        }
        if (bind(sockfd, addrs->ai_addr, addrs->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if (addrs == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return -1;
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("server: listen");
        return -1;
    }

    return sockfd;
}

int parse_http_request(s_http_request *hr, const char *buf, int buf_len) {
    // just read the first line for now
    char **request_line = split_request_line(buf);
    if (!strcmp(request_line[0], "GET")) {
        hr->method = HTTP_GET;
    }
    else if (!strcmp(request_line[0], "POST")) {
        hr->method = HTTP_POST;
    }
    else {
        return 0;
    }
    hr->request_uri = malloc(sizeof(strlen(request_line[1])) + 1);
    strcpy(hr->request_uri, request_line[1]);
    hr->http_version = malloc(sizeof(strlen(request_line[2])) + 1);
    strcpy(hr->http_version, request_line[2]);
    return 1;
}

char *parse_uri(const char *uri) {
    char *rel_path;
    if (!strcmp(uri, "/"))
        rel_path = from_string(REL_SITE_ROOT DEFAULT_PAGE, strlen(REL_SITE_ROOT DEFAULT_PAGE));
    else if (!strncmp(SITE_ROOT, uri, strlen(SITE_ROOT)))
        rel_path = from_string(REL_SITE_ROOT DEFAULT_PAGE, strlen(REL_SITE_ROOT DEFAULT_PAGE));
    else
        rel_path = concat_root_and_uri(REL_SITE_ROOT, uri);
    return rel_path;
}

// this really sucks
void http_to_print(s_http_response *res, char *p) {
    int res_len = BUFSIZ;
    int res_p = 0;
    p = malloc(sizeof(char) * res_len);
    strncpy(p, TEST_RESPONSE, strlen(TEST_RESPONSE));
    res_p += strlen(TEST_RESPONSE);
    char *content_type = "Content-Type: text/html\r\n";
    strncpy(p+res_p, content_type, strlen(content_type));
    res_p += strlen(content_type);
    char *content_length = "Content-Length: %i\r\n\r\n";
    char buffer[36];
    int l;
    l = sprintf(buffer, content_length, res->content->length);
    strncpy(p+res_p, buffer, l);
    res_p += l;
    strncpy(p+res_p, res->content->data, res->content->length + 1);
}

int get_resource(s_http_response *res, const char *path) {
    s_http_content *content;
    content = malloc(sizeof(content));
    content->content_type = TEXT_HTML;
    puts(path);
    puts(path);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror("opening resource path");
        return -1;
    }
    void *data = malloc(sizeof(BUFSIZ));
    int bytes_read = 0;
    if (!((bytes_read = fread(data, 1, BUFSIZ, fp)) < BUFSIZ)) {
        perror("get resource");
        return -1;
    }
    content->data = data;
    content->length = bytes_read;
    res->content = content;
    res->status_code = 200;
    res->reason = from_string("OK", 2);
    return 1;
}

char *http_get_response(s_http_request *request) {
    char *resource_path = NULL;
    s_http_response res;
    char *response;
    puts("HTTP GET RESPONSE");
    if ((resource_path = realpath(parse_uri(request->request_uri), NULL)) == NULL)
        perror("parsing uri");
    if (get_resource(&res, resource_path) == -1)
        return "HTTP/1.0 404 Not Found\r\n\r\n";
    http_to_print(&res, response);
    return response;
}

int http_post_response(s_http_request *request) {
    return 1;
}

// assume a mostly valid request. parse it and return a response
void handle_request(int newfd, char *response) {
    char buf[BUFSIZ];
    int bytes_recv;
    s_http_request request;
    switch (bytes_recv = recv(newfd, buf, sizeof(buf), 0)) {
        case 0:
            puts("server: connection closed by remote.");
            break;
        case -1:
            perror("recv");
            exit(EXIT_FAILURE);
        default:
            printf("server: recieved %i bytes.\n", bytes_recv);
            puts(buf);
    }
    if (!parse_http_request(&request, buf, bytes_recv)) {
        perror("handle request");
        return;
    }
    switch(request.method) {
        case HTTP_GET:
            response = http_get_response(&request);
            break;
        case HTTP_POST:
            http_post_response(&request);
            break;
        default:
            break;
    }
    return;
} 

// TODO: create process pool, then set up listener
// listener should poll or similar
// on accepting a new connection, send it a process to be handled
int main() {
    int sockfd, newfd, parent_pid, current_pid;
    s_sa_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    parent_pid = getpid();
    printf("Parent PID: %i\n", parent_pid);

    if ((sockfd = init_listener()) == -1) {
        printf("Failed to start listening on port %s.\n", PORT);
        return (EXIT_FAILURE);
    }
    else
        printf("server: listening on port: %s\n", PORT);
    puts("server: waiting for connections...");
    while(1) {
        sin_size = sizeof(their_addr);
        newfd = accept(sockfd, (s_sockaddr *)&their_addr, &sin_size);
        if (newfd == -1) {
            perror("server: accept");
            continue;
        }
        inet_ntop(their_addr.ss_family,
            get_in_addr((s_sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
        puts("server: forking...");
        current_pid = fork();
        if (current_pid == 0) {
            // ----- recv -----
            char *response = NULL;
            handle_request(newfd, response);
            
            // ----- send -----
            puts("server: sending....");
            close(sockfd);
            if (send(newfd, response, sizeof(response), 0) == -1)
                perror("send");
            else
                puts(response);
            close(newfd);
            free(response);
            exit(0);
        }
        else {
            wait(NULL);
        }
    }
    return (EXIT_SUCCESS);
}