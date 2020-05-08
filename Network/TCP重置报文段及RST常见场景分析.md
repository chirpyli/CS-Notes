`RST`表示连接重置，用于关闭那些已经没有必要继续存在的连接。

**产生`RST`的三个条件是：**
1. 目的地为某端口的`SYN`到达，然而在该端口上并没有正在监听的服务器；
2. TCP想取消一个已有连接；
3. TCP接收到一个根本不存在的连接上的分节。

下面的几种场景，都会产生`RST`，以此来说明重置报文段的用途。

### 一、针对不存在端口的连接请求
客户端向服务端某端口发起连接请求`SYN`，但是目的服务端主机不存在该端口，此时向客户端回应`RST`，中断连接请求。

下面通过程序和抓包进行分析。程序源码如下：
```rust
use std::io::prelude::*;
use std::net::TcpStream;
use std::thread;

fn main() {
    let mut stream = TcpStream::connect("192.168.2.229:33333").unwrap();
    let n = stream.write(&[1,2,3,4,5,6,7,8,9]).unwrap();
    println!("send {} bytes to remote node, waiting for end.", n);

    loop{
        thread::sleep_ms(1000);
    }
}
```
上面程序向目的主机`192.168.2.229`发起TCP连接，而目的主机并没有启动端口为`33333`的监听服务。所以当本地主机向目的主机发起TCP连接后，会收到来自目的主机的`RST`，并断开连接。（当然也不是所有的都会回复`RST`，有的主机可能不会进行回复）。抓包如下：

本地主机向目的主机发送TCP连接`SYN`：
![在这里插入图片描述](https://img-blog.csdn.net/20181016181322408?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

目的主机向本地主机回复`ACK、RST`：
![在这里插入图片描述](https://img-blog.csdn.net/20181016181339862?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

### 二、终止一条连接
终止一条连接的正常方法是由通信一方发送一个`FIN`。这种方法也被称为有序释放。因为`FIN`是在之前所有排队数据都已发送后才被发送出去，通常不会出现丢失数据的情况。然而在任何时刻，我们都可以通过发送一个重置报文段`RST`替代`FIN`来终止一条连接。这种方式也被称为终止释放。

终止一条连接可以为应用程序提供两大特性：（1）任何排队的数据都将被抛弃，一个重置报文段会被立即发送出去；（2）重置报文段的接收方会说明通信的另一端采用了终止的方式而不是一次正常关闭。API必须提供一种实现上述终止行为的方式来取代正常的关闭操作。

套接字API可通过套接字选项`SO_LINGER`的数值设置为`0`来实现上述功能。
```c++
/* Structure used to manipulate the SO_LINGER option.  */
struct linger {
    int l_onoff;		/* Nonzero to linger on close.  */
    int l_linger;		/* Time to linger.  */
  };
```
`SO_LINGER`的不同值的含义如下：
1. `l_onoff`为`0`，`l_linger`的值被忽略，内核缺省情况，`close()`调用会立即返回给调用者，TCP模块负责尝试发送残留的缓存区数据。
2. `l_onoff`为非零值，`l_linger`为`0`，则连接立即终止，TCP将丢弃残留在发送缓冲区中的数据并发送`RST`给对方，而不是发送`FIN`，这样避免了`TIME_WAIT`状态，对方`read（）`时将收到`Connection reset by peer`的错误。
3. `l_onoff`为非零值，`l_linger`大于零：如果`l_linger`时间范围，TCP模块成功发送完残留的缓冲区数据，则正常关闭，如果超时，则向对方发送`RST`，丢弃残留在发送缓冲区的数据。


客户端代码间附录代码1，服务端代码如下：
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

int set_linger(int sock, int l_onoff, int l_linger);
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

            set_linger(conn_sock, 1, 0);    //设置SO_LINGER option值为0
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
                nfds[i].events = POLLIN;

                close(nfds[i].fd);  //接收数据后就主动关闭连接，用于RST测试          
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

int set_linger(int sock, int l_onoff, int l_linger) {
    struct linger so_linger;
    so_linger.l_onoff = l_onoff;
    so_linger.l_linger = l_linger;
    int r = setsockopt(sock, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));

    return r;
}
```
抓包结果如下：
![在这里插入图片描述](https://img-blog.csdnimg.cn/2019050810184675.png)
先3次握手，后客户端向服务度发送了5个字节数据，服务端在接收完5字节数据向客户端`ACK`后，表示想中断连接，此时因设置了`SO_LINGER`选项值为`0`，`close()`时，直接向对方发送`RST`而不是正常的发送`FIN`，连接立即终止，并且不会有`TIME_WAIT`状态，TCP将丢弃残留在发送缓冲区中的数据，对方`read（）`时将收到`Connection reset by peer`的错误。



#### 三、半开连接
如果在未告知另一端的情况下通信的一端关闭或终止连接，那么就认为该条TCP连接处于半开状态。

举个例子，服务器主机被切断电源后重启（切断电源前可将网线断开，重启后再接上），此时留个客户端的是一个半开的连接。当客户端再次向服务端发送数据时，服务端对此连接一无所知，会回复一个重置报文段`RST`后，中断连接。

再或者如果程序开启了TCP保活机制，则当监测到对方主机不可达时，发送`RST`中断连接。详细可参考我的另一篇博文[TCP保活机制](https://blog.csdn.net/s_lisheng/article/details/87288445)。

TCP连接如果长时间没有数据收发，会使TCP发送保活探测报文，以维持连接或者探测连接是否存在。
![在这里插入图片描述](https://img-blog.csdnimg.cn/201902141812190.png)
可以看到如果认为连接不存在了，就会发送`RST`中断连接。

#### 四、提前关闭连接
TCP应用程序接收数据是从操作系统中接收的TCP数据，如果数据到达了操作系统但是我应用数据不想继续接收数据了，此时`RST`中断连接。

服务端代码：
```c
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

#define RST_TEST 1

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
#if RST_TEST == 0
            n = read(nfds[i].fd, buf, MAX_BUF);
#else
            n = read(nfds[i].fd, buf, 5);      //只接收部分数据就主动关闭连接，用于RST测试         
#endif
            if (0 == n) {
                close(nfds[i].fd);
                nfds[i].fd = -1;
                continue;
            } 
            if (n>0){
                printf("recv from client: %s\n", buf);
                nfds[i].events = POLLOUT;
#if RST_TEST != 0  
                close(nfds[i].fd);  //只接收部分数据就主动关闭连接，用于RST测试
#endif            
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
客户端发起连接后发送超过5字节的数据后，因为服务端只接收5个字节数据后不再接收数据（此时服务端操作系统已经收到10个字节数据，但上层`read`系统调用只接收5个字节），服务端向客户端发送`RST`中断连接。抓包结果如下：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190507165100743.png)
先3次握手，握手后客户端发送了10字节长度的数据，服务端在回应客户端`ACK`接收到数据后，发送`RST`中断连接。

#### 五、在一个已关闭的TCP连接上收到数据
如果一个已关闭的TCP连接又收到数据，显然是异常的，此时应`RST`中断连接。

服务端其他代码与上个代码相同，下面函数替换一下
```c
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

                close(nfds[i].fd);  //接收数据后就主动关闭连接，用于RST测试          
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

客户端代码与上个相同，只有下面函数不同，替换一下即可：
```c
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
        sleep(2);
        write(sock, sendbuf, strlen(sendbuf));		//这里是测试RST用的代码
        sleep(60);
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
抓包如下：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190507183150469.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==,size_16,color_FFFFFF,t_70)
先3次握手；后客户端向服务端发送了5字节数据，服务端接收到5字节数据回复`ACK`；之后向客户端发送`FIN`，关闭连接，但此时客户端还有数据要发送，没有向服务端发起`FIN`，此时只进行了2次挥手；之后客户端又向服务端发送了5个字节数据，但此时服务端该连接已经调用`close()`关闭，此时再次收到该连接的数据属于异常，回复`RST`中断连接。



#### 六、附录
测试用的客户端代码
```c
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

    if (-1 == connect(sock, (struct sockaddr *)&seraddr, sizeof(seraddr))) {
        perror("connect failure");
        exit(EXIT_FAILURE);
    }
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
        if (n < 0) {
            perror("read failure.");
            exit(EXIT_FAILURE);
        }
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


>参考文档：
[Linux Socket SO_LINGER选项](https://blog.51cto.com/xwandrew/2046615)
[Socket之so_linger与rst](https://segmentfault.com/a/1190000012345710)