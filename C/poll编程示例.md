#### 相关定义
```c++
struct pollfd {
    int fd;        /* The descriptor. */
    short events;  /* The event(s) is/are specified here. */
    short revents; /* Events found are returned here. */
};
```

```c++
#include <poll.h> 

/* allows the process to wait for an event to occur and to wake up the process when the event occurs. */
int poll(struct pollfd *fds, nfds_t nfds, int timeout); 
```



#### poll实现的echo服务端代码
```c++
/* echo server with poll */
#include <poll.h>
#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<pthread.h>

#define OPEN_MAX 1024
#define LISTEN_PORT 33333
#define MAX_BUF 1024

int handle_conn(struct pollfd *nfds, char* buf);
void run();

int main(int _argc, char* _argv[]) {
    run();

    return 0;
}

void run() {
    // bind socket
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

    int ret, i;
    struct pollfd nfds[OPEN_MAX];
    for (i=0;i<OPEN_MAX;++i){
        nfds[i].fd = -1;
    }

    nfds[0].fd = listen_sock;
    nfds[0].events = POLLIN;

    char* buf = (char*)malloc(MAX_BUF);   
    while (1) {
        ret = poll(nfds, OPEN_MAX, NULL);
        if (-1 == ret) {
            perror("poll failure.");
            exit(EXIT_FAILURE);
        }

        /* An event on one of the fds has occurred. */
        if (nfds[0].revents & POLLIN) {
            int conn_sock = accept(listen_sock, (struct sockaddr *)&cliaddr, &cliaddr_len);
            if (-1 == conn_sock) {
                perror("accept failure.");
                exit(EXIT_FAILURE);
            }
            printf("accept from %s:%d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));

            for (int k=0;k<OPEN_MAX;++k){
                if (nfds[k].fd < 0){
                    nfds[k].fd = conn_sock;
                    nfds[k].events = POLLIN;
                    break;
                }
                if (k == OPEN_MAX-1){
                    perror("too many clients, nfds size is not enough.");
                    exit(EXIT_FAILURE);
                }
            }
        }

        handle_conn(nfds, buf);
    }

    close(listen_sock);
}

int handle_conn(struct pollfd *nfds, char* buf) {
    int n = 0;
    for (int i=1;i<OPEN_MAX;++i) {
        if (nfds[i].fd<0) {
            continue;
        }

        if (nfds[i].revents & POLLIN) {
            bzero(buf, MAX_BUF);
            n = read(nfds[i].fd, buf, MAX_BUF);
            if (0 == n) {
                close(nfds[i].fd);
                nfds[i].fd = -1;
                continue;
            } 
            if (n>0){
                printf("recv from client: %s\n", buf);
                nfds[i].events = POLLOUT;
            } else {
                perror("read failure.");
                exit(EXIT_FAILURE);
            }
        } else if (nfds[i].revents & POLLOUT) {
            printf("write data to client: %s\n", buf);
            write(nfds[i].fd, buf, sizeof(buf));
            bzero(buf, MAX_BUF);          

            nfds[i].events = POLLIN;
        }
    }

    return 0;
}
```


>参考文档：  
[poll(3) - Linux man page](https://linux.die.net/man/3/poll)     
[Using poll() instead of select()](https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_71/rzab6/poll.htm)
