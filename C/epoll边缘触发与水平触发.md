在网络编程中，会涉及到水平触发与边缘触发的概念，工程中以边缘触发较为常见，本文讲述了边缘触发与水平触发的概念，并给出代码示例，通过代码可以很清楚的看到它们之间的区别。

### 水平触发与边缘触发
水平触发(level-trggered)
 
  - 只要文件描述符关联的读内核缓冲区非空，有数据可以读取，就一直发出可读信号进行通知，
  - 当文件描述符关联的内核写缓冲区不满，有空间可以写入，就一直发出可写信号进行通知

边缘触发(edge-triggered)

- 当文件描述符关联的读内核缓冲区由空转化为非空的时候，则发出可读信号进行通知，
- 当文件描述符关联的内核写缓冲区由满转化为不满的时候，则发出可写信号进行通知

### 两者的区别？

水平触发是只要读缓冲区有数据，就会一直触发可读信号，而边缘触发仅仅在空变为非空的时候通知一次，举个例子：
1. 读缓冲区刚开始是空的
2. 读缓冲区写入2KB数据
3. 水平触发和边缘触发模式此时都会发出可读信号
4. 收到信号通知后，读取了1kb的数据，读缓冲区还剩余1KB数据
5. 水平触发会再次进行通知，而边缘触发不会再进行通知

所以边缘触发需要一次性的把缓冲区的数据读完为止，也就是一直读，直到读到`EGAIN`(`EGAIN`说明缓冲区已经空了)为止，因为这一点，边缘触发需要设置文件句柄为非阻塞。
>ET模式在很大程度上减少了epoll事件被重复触发的次数，因此效率要比LT模式高。epoll工作在ET模式的时候，必须使用非阻塞套接口，以避免由于一个文件句柄的阻塞读/阻塞写操作把处理多个文件描述符的任务饿死。


这里只简单的给出了水平触发与边缘触发的处理方式的不同，边缘触发相对水平触发处理的细节更多一些，
```c
//水平触发
ret = read(fd, buf, sizeof(buf));

//边缘触发（代码不完整，仅为简单区别与水平触发方式的代码）
while(true) {
    ret = read(fd, buf, sizeof(buf);
    if (ret == EAGAIN) break;
}
```


### 代码示例
通过下面的代码示例，能够看到水平触发与边缘触发代码的不同以及触发次数的不同。通过这个示例能够加深你对边缘触发与水平触发的理解。

##### [echo server代码](./echo_server.c)：
```c
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
```

[客户端代码](./echo_client.c)


客户端连接服务端，分别向服务端发送`123`和`123456`，服务端运行结果如下：
##### 边缘触发运行结果：

可以看到边缘触发只触发一次
```
sl@Li:~/Works/study/epoll$ ./server 
accept from 127.0.0.1:38170
events_handle, ev.events = 1
step in edge_trigger, read bytes:4
step in edge_trigger, read bytes:-1
errno == EAGAIN, break.
recv from client: 123

events_handle, ev.events = 1
step in edge_trigger, read bytes:5
step in edge_trigger, read bytes:2
step in edge_trigger, read bytes:-1
errno == EAGAIN, break.
recv from client: 123456
```
重新编译服务端程序，再次发送`123`和`123456`，服务端运行结果如下：
##### 水平触发运行结果：
可以看到，在接收`123456`时，触发了两次，而边缘触发只触发一次。
```
sl@Li:~/Works/study/epoll$ ./server 
accept from 127.0.0.1:38364
events_handle, ev.events = 1
step in level_trigger, read bytes:4
recv from client: 123

events_handle, ev.events = 1
step in level_trigger, read bytes:5
recv from client: 12345
events_handle, ev.events = 1
step in level_trigger, read bytes:2
recv from client: 6
```


---
>还可以参考github上这个项目[handy](https://github.com/yedf/handy)，简洁易用的C++11网络库，支持单机千万并发连接，代码量只有几千行，核心代码大概2k左右。 里面有裸用epoll的实例代码[handy/raw-examples/epoll.cc](https://github.com/yedf/handy/blob/master/raw-examples/epoll.cc)。 