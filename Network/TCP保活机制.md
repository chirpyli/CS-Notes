在需要长连接的网络通信程序中，经常需要心跳检测机制，来实现检测对方是否在线或者维持网络连接的需要。这一机制是在应用层实现的，对应的，在TCP协议中，也有类似的机制，就是TCP保活机制。


### 一、为什么需要保活机制？
设想这种情况，TCP连接建立后，在一段时间范围内双发没有互相发送任何数据。思考以下两个问题：
1. 怎么判断对方是否还在线。这是因为，TCP对于非正常断开的连接系统并不能侦测到（比如网线断掉）。
2. 长时间没有任何数据发送，连接可能会被中断。这是因为，网络连接中间可能会经过路由器、防火墙等设备，而这些有可能会对长时间没有活动的连接断掉。

基于上面两点考虑，需要保活机制。

### 二、TCP保活机制的实现（Linux）
保活机制是由一个保活计时器实现的。当计时器被激发，连接一段将发送一个保活探测报文，另一端接收报文的同时会发送一个`ACK`作为响应。

#### 1、相关配置
具体实现上有以下几个相关的配置：
- 保活时间：默认7200秒（2小时）
- 保活时间间隔：默认75秒
- 保活探测数：默认9次


查看Linux系统中TCP保活机制对应的系统配置如下（不同系统实现可能不同）：
```shell
sl@Li:/proc/sys/net/ipv4$ cat tcp_keepalive_time 
7200
sl@Li:/proc/sys/net/ipv4$ cat tcp_keepalive_intvl 
75
sl@Li:/proc/sys/net/ipv4$ cat tcp_keepalive_probes 
9
```

#### 2、过程描述
连接中启动保活功能的一端，在保活时间内连接处于非活动状态，则向对方发送一个保活探测报文，如果收到响应，则重置保活计时器，如果没有收到响应报文，则经过一个保活时间间隔后再次向对方发送一个保活探测报文，如果还没有收到响应报文，则继续，直到发送次数到达保活探测数，此时，对方主机将被确认为不可到达，连接被中断。

TCP保活功能工作过程中，开启该功能的一端会发现对方处于以下四种状态之一：
1. 对方主机仍在工作，并且可以到达。此时请求端将保活计时器重置。如果在计时器超时之前应用程序通过该连接传输数据，计时器再次被设定为保活时间值。
2. 对方主机已经崩溃，包括已经关闭或者正在重新启动。这时对方的TCP将不会响应。请求端不会接收到响应报文，并在经过保活时间间隔指定的时间后超时。超时前，请求端会持续发送探测报文，一共发送保活探测数指定次数的探测报文，如果请求端没有收到任何探测报文的响应，那么它将认为对方主机已经关闭，连接也将被断开。
3. 客户主机崩溃并且已重启。在这种情况下，请求端会收到一个对其保活探测报文的响应，但这个响应是一个重置报文段`RST`，请求端将会断开连接。
4. 对方主机仍在工作，但是由于某些原因不能到达请求端（例如网络无法传输，而且可能使用ICMP通知也可能不通知对方这一事实）。这种情况与状态2相同，因为TCP不能区分状态2与状态4，结果是都没有收到探测报文的响应。

### 三、保活机制的弊端
理解了上面的实现，就可以发现其存在以下两点主要弊端：
1. 在出现短暂的网络错误的时候，保活机制会使一个好的连接断开；
2. 保活机制会占用不必要的带宽；

所以，保活机制是存在争议的，主要争议之处在于是否应在TCP协议层实现，有两种主要观点：其一，保活机制不必在TCP协议中提供，而应该有应用层实现；其二，认为大多数应用都需要保活机制，应该在TCP协议层实现。

> 保活功能在默认情况下是关闭的。没有经过应用层的请求，Linux系统不会提供保活功能。

### 四、保活机制应用代码示例
#### 1、 服务端代码
```c++
/* server */
#include<sys/epoll.h>
#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<pthread.h>
#include <sys/time.h>

#define MAX_EVENTS 1024
#define LISTEN_PORT 33333
#define MAX_BUF 65536

struct echo_data;
int setnonblocking(int sockfd);
int events_handle(int epfd, struct epoll_event ev);
void run();

// 应用TCP保活机制的相关代码
int set_keepalive(int sockfd, int keepalive_time, int keepalive_intvl, int keepalive_probes) {
    int optval;
    socklen_t optlen = sizeof(optval);
    optval = 1;
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen)) {
        perror("setsockopt failure.");
        return -1;
    }

    optval = keepalive_probes;
    if (-1 == setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, &optval, optlen)) {
        perror("setsockopt failure.");
        return -1;
    }

    optval = keepalive_intvl;
    if (-1 == setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, &optval, optlen)) {
        perror("setsockopt failure.");
        return -1;
    }

    optval = keepalive_time;
    if (-1 == setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, &optval, optlen)) {
        perror("setsockopt failure.");
        return -1;
    }
}

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
                set_keepalive(conn_sock, 120, 20, 3);
                setnonblocking(conn_sock);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, conn_sock, &ev)) {
                    perror("epoll_ctl add conn_sock failure.");
                    exit(EXIT_FAILURE);
                }
            } else {
                events_handle(epfd, events[n]);
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

struct echo_data {
    char* data;
    int fd;
};

int events_handle(int epfd, struct epoll_event ev) {
    printf("events_handle, ev.events = %d\n", ev.events);
    int fd = ev.data.fd;
    if (ev.events & EPOLLIN) {
        char* buf = (char*)malloc(MAX_BUF);
        bzero(buf, MAX_BUF);
        int count = 0;
        int n = 0;
        while (1) {
            n = read(fd, (buf + count), 1024);
            printf("step in edge_trigger, read bytes:%d\n", n);
            if (n > 0) {
                count += n;
            } else if (0 == n) {
                break;
            } else if (n < 0 && EAGAIN == errno) {
                perror("errno == EAGAIN, break.");
                break;
            } else {
                perror("read failure.");
                if (ETIMEDOUT == errno) {
                    if (-1 == epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev)) {
                    perror("epoll_ctl del fd failure.");
                    exit(EXIT_FAILURE);
                    }
                    close(fd);
                    return 0;
                }
                exit(EXIT_FAILURE);
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
        struct echo_data* ed = (struct echo_data*)malloc(sizeof(struct echo_data));
        ed->data = buf;
        ed->fd = fd;
        ev.data.ptr = ed;
        ev.events = EPOLLOUT | EPOLLET;
        if (-1 == epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev)) {
            perror("epoll_ctl modify fd failure.");
            exit(EXIT_FAILURE);
        }

        return 0;
    } else if (ev.events & EPOLLOUT) {
        struct echo_data* data = (struct echo_data*)ev.data.ptr;
        printf("write data to client: %s", data->data);
        int ret = 0;
        int send_pos = 0;
        const int total = strlen(data->data);
        char* send_buf = data->data;
        while(1) {
            ret = write(data->fd, (send_buf + send_pos), total - send_pos);
            if (ret < 0) {
                if (EAGAIN == errno) {
                    sched_yield();
                    continue;
                }
                perror("write failure.");
                exit(EXIT_FAILURE);
            }
            send_pos += ret;
            if (total == send_pos) {
                break;
            }
        }

        ev.data.fd = data->fd;
        ev.events = EPOLLIN | EPOLLET;
        if (-1 == epoll_ctl(epfd, EPOLL_CTL_MOD, data->fd, &ev)) {
            perror("epoll_ctl modify fd failure.");
            exit(EXIT_FAILURE);
        }

        free(data->data);
        free(data);
    }

    return 0;
}
```

#### 2、客户端代码
```c++
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include <time.h>
#include <sys/time.h>
#include<stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define SERVER_PORT 33333
#define MAXLEN 65535

void client_handle(int sock);


int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        printf("input args %d: %s\n", i, argv[i]);
    }
    struct sockaddr_in seraddr;
    int server_port = SERVER_PORT;
    if (2 == argc) {
        server_port = atoi(argv[1]);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &seraddr.sin_addr);
    seraddr.sin_port = htons(server_port);

    connect(sock, (struct sockaddr *)&seraddr, sizeof(seraddr));
    client_handle(sock);

    return 0;
}

void client_handle(int sock) {
    char sendbuf[MAXLEN], recvbuf[MAXLEN];
    bzero(sendbuf, MAXLEN);
    bzero(recvbuf, MAXLEN);
    int n = 0;

    while (1) {
        if (NULL == fgets(sendbuf, MAXLEN, stdin)) {
            break;
        }
        // 按`#`号退出
        if ('#' == sendbuf[0]) {
            break;
        }
        struct timeval start, end;
        gettimeofday(&start, NULL);
        write(sock, sendbuf, strlen(sendbuf));
        n = read(sock, recvbuf, MAXLEN);
        if (0 == n) {
            break;
        }
        write(STDOUT_FILENO, recvbuf, n);
        gettimeofday(&end, NULL);
        printf("time diff=%ld microseconds\n", ((end.tv_sec * 1000000 + end.tv_usec)- (start.tv_sec * 1000000 + start.tv_usec)));
    }

    close(sock);
}
```

只要连接后，不输入数据，连接就没有数据发送，通过抓包可以发现每120秒就会有保活探测报文发出：
![保活机制](https://img-blog.csdnimg.cn/2019021418131334.png)
如果一直没有收到回复，会中断连接（`RST`）。
![在这里插入图片描述](https://img-blog.csdnimg.cn/201902141812190.png)

>参考文档：[Requirements for Internet Hosts -- Communication Layers](https://tools.ietf.org/html/rfc1122#section-4.2.3.6)