//
// Adapted from Beej's Guide to Network Programming
//

#include "server.h"

#define PORT "8080"
#define BACKLOG 10

#define SITE "./site_content"

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

// create process pool, then set up listener
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
    printf("server: waiting for connections...\n");
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
        printf("server: forking...\n");
        current_pid = fork();
        if (current_pid == 0) {
            printf("server: sending....\n");
            close(sockfd);
            if (send(newfd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(newfd);
            printf("server: sent!\n");
            exit(0);
        }
        else {
            wait(NULL);
        }
    }
    return (EXIT_SUCCESS);
}