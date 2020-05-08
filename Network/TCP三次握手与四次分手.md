### TCP首部
TCP工作在传输层，提供应用程序到应用程序之间的可靠传输。学习TCP协议，首先从TCP协议头部开始：
![这里写图片描述](https://imgconvert.csdnimg.cn/aHR0cDovL2ltZy5ibG9nLmNzZG4ubmV0LzIwMTcwODEwMTg1OTEyMzcz)
TCP协议头部每个字段说明一下如下：
   - Source Port和Destination Port:分别占用16位，表示<font color=red>源端口号和目的端口号；用于区别主机中的不同进程，而IP地址是用来区分不同的主机的</font>，源端口号和目的端口号配合上IP首部中的源IP地址和目的IP地址就能唯一的确定一个TCP连接；
   - Sequence Number:用来标识从TCP发端向TCP收端发送的数据字节流，它表示在这个报文段中的的第一个数据字节在数据流中的序号；***主要用来解决网络报乱序的问题***；
   - Acknowledgment Number:32位确认序列号包含发送确认的一端所期望收到的下一个序号，因此，确认序号应当是上次已成功收到数据字节序号加1。不过，只有当标志位中的ACK标志（下面介绍）为1时该确认序列号的字段才有效。***主要用来解决不丢包的问题***；
   - Offset:给出首部中32 bit字的数目，需要这个值是因为任选字段的长度是可变的。这个字段占4bit（最多能表示15个32bit的的字，即4*15=60个字节的首部长度），因此TCP最多有60字节的首部。然而，没有任选字段，正常的长度是20字节；
   - TCP Flags:TCP首部中有6个标志比特，它们中的多个可同时被设置为1，主要是用于操控TCP的状态机的，依次为URG，ACK，PSH，RST，SYN，FIN。每个标志位的意思如下：
       - URG：此标志表示TCP包的紧急指针域（后面马上就要说到）有效，用来保证TCP连接不被中断，并且督促中间层设备要尽快处理这些数据；
       -  ACK：此标志表示应答域有效，就是说前面所说的TCP应答号将会包含在TCP数据包中；有两个取值：0和1，为1的时候表示应答域有效，反之为0；
       - PSH：这个标志位表示Push操作。所谓Push操作就是指在数据包到达接收端以后，立即传送给应用程序，而不是在缓冲区中排队；
       - RST：这个标志表示连接复位请求。用来复位那些产生错误的连接，也被用来拒绝错误和非法的数据包；
       -  SYN：表示同步序号，用来建立连接。SYN标志位和ACK标志位搭配使用，当连接请求的时候，SYN=1，ACK=0；连接被响应的时候，SYN=1，ACK=1；这个标志的数据包经常被用来进行端口扫描。扫描者发送一个只有SYN的数据包，如果对方主机响应了一个数据包回来 ，就表明这台主机存在这个端口；但是由于这种扫描方式只是进行TCP三次握手的第一次握手，因此这种扫描的成功表示被扫描的机器不很安全，一台安全的主机将会强制要求一个连接严格的进行TCP的三次握手；
       - FIN： 表示发送端已经达到数据末尾，也就是说双方的数据传送完成，没有数据可以传送了，发送FIN标志位的TCP数据包后，连接将被断开。这个标志的数据包也经常被用于进行端口扫描。
   - Window:窗口大小，也就是有名的滑动窗口，用来进行流量控制；


### TCP三次握手与四次分手
握手与分手流程如下：
![这里写图片描述](https://imgconvert.csdnimg.cn/aHR0cDovL2ltZy5ibG9nLmNzZG4ubmV0LzIwMTcwODEwMTkwNjQ4NDY3)

运行下面的程序，对应的抓包结果如下：


下面是用于分析TCP三次握手与四次分手过程用的程序代码：

**服务端代码：**
Rust编写的服务端程序代码：
```rust
use std::net::{TcpListener, TcpStream};
use std::io::prelude::*;
use std::thread;

fn main() {
	{
	    let listener = TcpListener::bind("127.0.0.1:33333").unwrap();
	    let (mut stream, addr) = listener.accept().unwrap();
	    println!("tcp accept from {:?}", addr);
	    let mut buf = [0; 1024];
	    let size = stream.read(&mut buf).unwrap();
	    println!("receive from remote {} bytes data.", size);
	    thread::sleep_ms(1000);
	}
	thread::sleep_ms(6*1000);
}
```
或者c++编写的服务端（windows）程序代码：
```c++
#include "stdafx.h"
#include<WinSock2.h>
#include<stdlib.h>
#include<WS2tcpip.h>
#include<string>
#include<iostream>
using namespace std;

#pragma comment(lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "Failed to load Winsock" << endl;
		return -1;
	}

	SOCKET sockServer = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(33333);
	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(sockServer, (LPSOCKADDR)&addrServer, sizeof(SOCKADDR_IN))) {
		cout << "Failed bind:" << WSAGetLastError() << endl;
		return -1;
	}

	if (SOCKET_ERROR == listen(sockServer, 10)) {
		cout<<"Listen failed:"<< WSAGetLastError() << endl;
		return -1;
	}

	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);

	SOCKET sockConn = accept(sockServer, (SOCKADDR*)&addrClient, &len);
	if (SOCKET_ERROR == sockConn) {
		cout << "Accept failed:" << WSAGetLastError() << endl;
		return -1;
	}
	char addrBuf[20] = { '\0' };
	inet_ntop(AF_INET, (void*)&addrClient.sin_addr, addrBuf, 16);
	cout << "Accept from " << addrBuf << endl;

	char recvBuf[1024];
	memset(recvBuf, 0, sizeof(recvBuf));
	int size = recv(sockConn, recvBuf, sizeof(recvBuf), 0);
	cout << "received " << size << " from remote" << endl;

	Sleep(1000);
	closesocket(sockConn);
	closesocket(sockServer);

	WSACleanup();
	system("pause");

    return 0;
}
```
**客户端代码：**
Rust实现如下：
```rust
use std::io::prelude::*;
use std::net::TcpStream;
use std::thread;

fn main() {
    {
        let mut stream = TcpStream::connect("192.168.2.210:33333").unwrap();
        let n = stream.write(&[1,2,3,4,5,6,7,8,9,10]).unwrap();
        println!("send {} bytes to remote node, waiting for end.", n);
        thread::sleep_ms(1000);
    }
    thread::sleep_ms(10*60*1000);
}
```


#### TCP建立连接的过程——三次握手

TCP是面向连接的，无论哪一方向另一方发送数据之前，都必须先在双方之间建立一条连接。在TCP/IP协议中，TCP协议提供可靠的连接服务，连接是通过三次握手进行初始化的。三次握手的目的是同步连接双方的序列号和确认号并交换 TCP窗口大小信息。下面通过上面给出的程序和wireshark抓包工具对TCP连接过程进行分析。

![在这里插入图片描述](https://img-blog.csdn.net/20181018184613506?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

运行服务端程序，运行后服务端程序进入监听状态```LISTEN```。
![在这里插入图片描述](https://img-blog.csdn.net/20181018161301919?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
启动客户端，开始TCP握手。

**第一次：**  客户端向服务端发送SYN（建立连接请求），客户端进入```SYN_SENT```状态（握手中的中间状态都非常短，很难看到，大部分看到的是```LISTEN```和```ESTABLISH```）。如下图所示：
![在这里插入图片描述](https://img-blog.csdn.net/201810181613435?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
抓包（SYN）：
![在这里插入图片描述](https://img-blog.csdn.net/20181018161359609?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

**第二次：** 服务端接收到```SYN```，回应```SYN+ACK```进入```SYN+RCVD```状态（这个状态非常短很难看到）
抓包（SYN+ACK）：
![在这里插入图片描述](https://img-blog.csdn.net/20181018161529271?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)


**第三次：** 客户端收到```SYN+ACK```后，回应```ACK```进入```ESTABLISH```状态。
抓包（ACK）：
![在这里插入图片描述](https://img-blog.csdn.net/20181018161601229?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

服务端收到```ACK```后，进入```ESTABLISH```状态，握手完成，连接建立。

#### TCP断开连接的过程——四次分手

当客户端和服务器通过三次握手建立了TCP连接以后，当数据传送完毕，肯定是要断开TCP连接的啊。那对于TCP的断开连接，这里就有了神秘的“四次分手”。
![在这里插入图片描述](https://img-blog.csdn.net/20181018184645385?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

**第一次：** 主机A（可以是客户端也可以是服务端，这里主机A是客户端首先发起断开连接）发送连接释放报文```FIN```，此时，主机A进入```FIN_WAIT_1```状态，表示主机A没有数据要发送给主机B了。
抓包```FIN```：
![在这里插入图片描述](https://img-blog.csdn.net/20181018190947659?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

**第二次：** 主机B收到了主机A发送的```FIN```报文，向主机A回一个```ACK```报文。主机A收到```ACK```后进入```FIN_WAIT_2```状态，主机B进入```CLOSE_WAIT```状态。
抓包```ACK```：
![在这里插入图片描述](https://img-blog.csdn.net/20181018191105382?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
**第三次：** 当主机B不再需要连接时，向主机A发送连接释放报文```FIN```，请求关闭连接，同时主机B进入```LAST_ACK```状态。
抓包```FIN```：
![在这里插入图片描述](https://img-blog.csdn.net/20181018191227401?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

**第四次：** 主机A收到主机B发送的```FIN```报文，向主机B发送```ACK```报文，然后主机A进入```TIME_WAIT```状态；主机B收到主机A的```ACK```报文以后，就关闭连接。此时，主机A 等待```2MSL```（最大报文存活时间）后依然没有收到回复，则证明Server端已正常关闭，此时，主机A关闭连接。
抓包```ACK```:
![在这里插入图片描述](https://img-blog.csdn.net/20181018191237593?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

至此，TCP的四次分手完成，断开连接。

最后，从代码看抓包结果：
![在这里插入图片描述](https://img-blog.csdn.net/20181018181921200?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
可以明确的看到，首先是进行了3次握手，连接建立后进行了1次发送数据过程，最后是4次分手，结束。


### 三次握手，为什么？
TCP建立连接的三次握手，为什么非要三次呢？两次不行吗？在谢希仁的《计算机网络》中是这样说的：
>为了防止已失效的连接请求报文段突然又传送到了服务端，因而产生错误。

在书中同时举了一个例子，如下：

>“已失效的连接请求报文段”的产生在这样一种情况下：client发出的第一个连接请求报文段并没有丢失，而是在某个网络结点长时间的滞留了，以致延误到连接释放以后的某个时间才到达server。本来这是一个早已失效的报文段。但server收到此失效的连接请求报文段后，就误认为是client再次发出的一个新的连接请求。于是就向client发出确认报文段，同意建立连接。假设不采用“三次握手”，那么只要server发出确认，新的连接就建立了。由于现在client并没有发出建立连接的请求，因此不会理睬server的确认，也不会向server发送数据。但server却以为新的运输连接已经建立，并一直等待client发来数据。这样，server的很多资源就白白浪费掉了。采用“三次握手”的办法可以防止上述现象发生。例如刚才那种情况，client不会向server的确认发出确认。server由于收不到确认，就知道client并没有要求建立连接。”

这就很明白了，防止了服务器端的一直等待而浪费资源。



### 四次分手，为什么？
<font color=red>TCP是全双工模式，这是理解4次分手的关键</font>，这就意味着，当A发出```FIN```报文时，只是表示A已经没有数据要发送了，并不意味着B不需要发送数据给Ａ了，这个时候A还是可以接收来自B的数据；当B返回```ACK```报文时，表示它已经知道A没有数据发送了，但是B还是可以发送数据到A的。所以2次分手是不可以的。当B不再需要向A发送数据时，向Ａ发送```FIN```报文，告诉A，我也没有数据要发送了，之后彼此就会中断这次TCP连接。

**四次分手过程中的状态：**
|状态|解释|
|--|--|
|FIN_WAIT_1|这个状态要好好解释一下，其实FIN_WAIT_1和FIN_WAIT_2状态的真正含义都是表示等待对方的FIN报文。而这两种状态的区别是：FIN_WAIT_1状态实际上是当SOCKET在ESTABLISHED状态时，它想主动关闭连接，向对方发送了FIN报文，此时该SOCKET即进入到FIN_WAIT_1状态。而当对方回应ACK报文后，则进入到FIN_WAIT_2状态，当然在实际的正常情况下，无论对方何种情况下，都应该马上回应ACK报文，所以FIN_WAIT_1状态一般是比较难见到的，而FIN_WAIT_2状态还有时常常可以用netstat看到。（主动方）
|FIN_WAIT_2|上面已经详细解释了这种状态，实际上FIN_WAIT_2状态下的SOCKET，表示半连接，也即有一方要求close连接，但另外还告诉对方，我暂时还有点数据需要传送给你(ACK信息)，稍后再关闭连接。（主动方）
|CLOSE_WAIT|这种状态的含义其实是表示在等待关闭。怎么理解呢？当对方close一个SOCKET后发送FIN报文给自己，你系统毫无疑问地会回应一个ACK报文给对方，此时则进入到CLOSE_WAIT状态。接下来呢，实际上你真正需要考虑的事情是察看你是否还有数据发送给对方，如果没有的话，那么你也就可以 close这个SOCKET，发送FIN报文给对方，也即关闭连接。所以你在CLOSE_WAIT状态下，需要完成的事情是等待你去关闭连接。（被动方）
|LAST_ACK|这个状态还是比较容易好理解的，它是被动关闭一方在发送FIN报文后，最后等待对方的ACK报文。当收到ACK报文后，也即可以进入到CLOSED可用状态了。（被动方）
|TIME_WAIT|表示收到了对方的FIN报文，并发送出了ACK报文，就等2MSL后即可回到CLOSED可用状态了。如果FIN_WAIT_1状态下，收到了对方同时带FIN标志和ACK标志的报文时，可以直接进入到TIME_WAIT状态，而无须经过FIN_WAIT_2状态。（主动方）
|CLOSED| 表示连接中断。

**为什么TIME_WAIT状态要等待2MSL？**
客户端接收到服务器端的 ```FIN```报文后进入此状态，此时并不是直接进入 ```CLOSED```状态，还需要等待一个时间计时器设置的时间 ```2MSL```。这么做有两个理由：
- 确保最后一个确认报文能够到达。如果 B 没收到 A 发送来的确认报文，那么就会重新发送连接释放请求报文，A 等待一段时间就是为了处理这种情况的发生。
- 等待一段时间是为了让本连接持续时间内所产生的所有报文都从网络中消失，使得下一个新的连接不会出现旧的连接请求报文。