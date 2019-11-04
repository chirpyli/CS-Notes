/* echo server*/

#include<sys/epoll.h>
#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>

#define MAX_EVENTS 1024
#define LISTEN_PORT 33333
#define MAX_BUF 1024

// #define LEVEL_TRIGGER

int setnonblocking(int sockfd);
int events_handle_level(int epfd, struct epoll_event ev);
int events_handle_edge(int epfd, struct epoll_event ev);
void run();

int main(int _argc, char* _argv[]) {
    run();

    return 0;
}

void run() {
    int epfd = epoll_create1(0);
    if (-1 == epfd) {
        perror("epoll_create1 failure.");
        exit(EXIT_FAILURE);
    }

    char str[INET_ADDRSTRLEN];
    struct sockaddr_in seraddr, cliaddr;
    socklen_t cliaddr_len = sizeof(cliaddr);
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(LISTEN_PORT);

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (-1 == bind(listen_sock, (struct sockaddr*)&seraddr, sizeof(seraddr))) {
        perror("bind server addr failure.");
        exit(EXIT_FAILURE);
    }
    listen(listen_sock, 5);

    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev)) {
        perror("epoll_ctl add listen_sock failure.");
        exit(EXIT_FAILURE);
    }

    int nfds = 0;
    while (1) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (-1 == nfds) {
            perror("epoll_wait failure.");
            exit(EXIT_FAILURE);
        }

        for ( int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) {
                int conn_sock = accept(listen_sock, (struct sockaddr *)&cliaddr, &cliaddr_len);
                if (-1 == conn_sock) {
                    perror("accept failure.");
                    exit(EXIT_FAILURE);
                }
                printf("accept from %s:%d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));

                setnonblocking(conn_sock);
#ifdef LEVEL_TRIGGER
                ev.events = EPOLLIN;
#else
                ev.events = EPOLLIN | EPOLLET;
#endif
                ev.data.fd = conn_sock;
                if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, conn_sock, &ev)) {
                    perror("epoll_ctl add conn_sock failure.");
                    exit(EXIT_FAILURE);
                }
            } else {
#ifdef LEVEL_TRIGGER
                events_handle_level(epfd, events[n]);
#else
                events_handle_edge(epfd, events[n]);
#endif
            }
        }
    }

    close(listen_sock);
    close(epfd);
}

int setnonblocking(int sockfd){
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}

int events_handle_level(int epfd, struct epoll_event ev) {
    printf("events_handle, ev.events = %d\n", ev.events);
    int fd = ev.data.fd;
    if (ev.events == EPOLLIN) {
        char buf[MAX_BUF];
        bzero(buf, MAX_BUF);
        int n = 0;
        n = read(fd, buf, 5);
        printf("step in level_trigger, read bytes:%d\n", n);

        if (n < 0) {
            perror("read fd failure.");
            if (-1 == epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev)) {
                perror("epoll_ctl del fd failure.");
                exit(EXIT_FAILURE);
            }

            return -1;
        }

        if (0 == n) {
            if (-1 == epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev)) {
                perror("epoll_ctl del fd failure.");
                exit(EXIT_FAILURE);
            }
            close(fd);

            return 0;
        }

        printf("recv from client: %s\n", buf);
        write(fd, buf, n);

        return 0;
    }

    return 0;
}


int events_handle_edge(int epfd, struct epoll_event ev) {
    printf("events_handle, ev.events = %d\n", ev.events);
    int fd = ev.data.fd;
    if (ev.events == EPOLLIN) {
        char* buf = (char*)malloc(MAX_BUF);
        bzero(buf, MAX_BUF);
        int count = 0;
        int n = 0;
        while (1) {
            n = read(fd, (buf + n), 5);
            printf("step in edge_trigger, read bytes:%d\n", n);
            if (n > 0) {
                count += n;
            } else if (0 == n) {
                break;
            } else if (n < 0 && EAGAIN == errno) {
                printf("errno == EAGAIN, break.\n");
                break;
            } else {
                perror("read failure.");
                break;
            }

        }

        if (0 == count) {
            if (-1 == epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev)) {
                perror("epoll_ctl del fd failure.");
                exit(EXIT_FAILURE);
            }
            close(fd);

            return 0;
        }

        printf("recv from client: %s\n", buf);
        write(fd, buf, count);

        free(buf);
        return 0;
    }

    return 0;
}
