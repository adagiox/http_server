//
// Adapted from Beej's Guide to Network Programming
//

#include "server.h"

#define PORT "8080"
#define BACKLOG 10

#define SITE_ROOT "./site_content"
#define TEST_RESPONSE "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 125\r\n\r\n<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>My test page</title></head><body><p>This is my page</p></body></html>"

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

int ch_before_sp(const char *buf, int index) {
    int i = 0;
    while (buf[index] != ' ') {
        i++;
        index++;
    }
    return i;
}

char **split_request_line(const char *buf) {
    int i = 0;
    int s = 0;
    char ch = buf[i];
    int method_size;
    int uri_size;
    int version_size = 8;
    method_size = ch_before_sp(buf, 0);
    i = method_size + 1;
    uri_size = ch_before_sp(buf, i);
    char **split = malloc(sizeof(char *) * 3);
    char *method = malloc(sizeof(char) * method_size + 1);
    char *uri = malloc(sizeof(char) * uri_size + 1);
    char *version = malloc(sizeof(char) * version_size + 1);
    split[0] = method;
    split[1] = uri;
    split[2] = version;
    i = 0;
    for (int a = 0; a < method_size; a++){
        method[a] = buf[i];
        i++;
    }
    i++;
    method[method_size] = '\0';
    for (int a = 0; a < uri_size; a++){
        uri[a] = buf[i];
        i++;
    }
    i++;
    uri[uri_size] = '\0';
    for (int a = 0; a < version_size; a++){
        version[a] = buf[i];
        i++;
    }
    version[version_size] = '\0';
    return split;
}

int parse_html_request(s_http_request *hr, const char *buf, int buf_len) {
    // just read the first line for now
    char **request_line = split_request_line(buf);
    if (!strcmp(request_line[0], "GET")) {
        hr->method = HTTP_GET;
    }
    else if (!strcmp(request_line[0], "POST")) {
        hr->method = HTTP_POST;
    }
    hr->request_uri = malloc(sizeof(strlen(request_line[1])) + 1);
    strcpy(hr->request_uri, request_line[1]);
    hr->http_version = malloc(sizeof(strlen(request_line[2])) + 1);
    strcpy(hr->http_version, request_line[2]);
    return 1;
}

// assume a mostly valid request, just parse it and send a response
void handle_request(int newfd) {
    char buf[BUFSIZ];
    int bytes_recv;
    s_http_request hr;
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
    parse_html_request(&hr, buf, bytes_recv);
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
            handle_request(newfd);
            
            // ----- send -----
            puts("server: sending....");
            close(sockfd);
            if (send(newfd, TEST_RESPONSE, sizeof(TEST_RESPONSE), 0) == -1)
                perror("send");
            else
                puts(TEST_RESPONSE);
            close(newfd);
            exit(0);
        }
        else {
            wait(NULL);
        }
    }
    return (EXIT_SUCCESS);
}