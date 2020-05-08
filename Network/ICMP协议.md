#### ICMP协议
在互联网传输过程中，IP数据报难免会出现差错，通常出现差错，处理方法就是丢弃，但是一般，出现差错后，会发送ICMP报文给主机，告诉它一些差错信息，以及对当前的网络状态进行一个探寻。可以说，ICMP的主要目的是用于在TCP/IP网络中发送出错和控制消息。

ICMP报文封装如下：
![在这里插入图片描述](https://img-blog.csdn.net/20181015142759108?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)


#### 主要ICMP报文
ICMP报文主要分三类，即差错报告报文、控制报文、请求/应答报文，如下图所示：
![在这里插入图片描述](https://img-blog.csdn.net/20181015143957134?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)


**目的不可达(Destination Unreachable Message)**
```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             unused                            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Internet Header + 64 bits of Original Data Datagram      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
当路由器收到一个无法传递下去的IP报文时，会发送ICMP目的不可达报文（Type为3）给IP报文的源发送方。报文中的Code就表示发送失败的原因。

**超时(Time Exceeded Message)**
```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             unused                            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Internet Header + 64 bits of Original Data Datagram      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
网络传输IP数据报的过程中，如果IP数据包的TTL值逐渐递减为0时，需要丢弃数据报。这时，路由器需要向源发送方发送ICMP超时报文(Type为11)，Code为0，表示传输过程中超时了。

一个IP数据报可能会因为过大而被分片，然后在目的主机侧把所有的分片重组。如果主机迟迟没有等到所有的分片报文，就会向源发送方发送一个ICMP超时报文，Code为1，表示分片重组超时了。

**参数错误报文(Parameter Problem Message)**
```
   0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    Pointer    |                   unused                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Internet Header + 64 bits of Original Data Datagram      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
当路由器或主机处理数据报时，发现因为报文头的参数错误而不得不丢弃报文时，需要向源发送方发送参数错误报文(Type为12)。当Code为0时，报文中的Pointer表示错误的字节位置。

**源抑制(Source Quench Message)**
```
   0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             unused                            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Internet Header + 64 bits of Original Data Datagram      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
路由器在处理报文时会有一个缓存队列。如果超过最大缓存队列，将无法处理，从而丢弃报文。并向源发送方发一个ICMP源抑制报文(Type为4)，告诉对方降低发送速率。

**重定向(Redirect Message)**
```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                 Gateway Internet Address                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Internet Header + 64 bits of Original Data Datagram      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
当路由收到IP数据报，发现数据报的目的地址在路由表上没有，它就会发ICMP重定向报文(Type为5)给源发送方，提醒它想要发送的地址不在，去其他地方找找吧。

**请求回显或回显应答(Echo or Echo Reply Message)**
```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Identifier          |        Sequence Number        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Data ...
   +-+-+-+-+-
```
Type(8)是请求回显报文(Echo)；Type(0)是回显应答报文(Echo Reply)。

请求回显或回显应答报文属于查询报文。Ping就是用这种报文进行查询和回应。

**信息请求或信息响应(Information Request or Information Reply Message)**
```
 0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |      Code     |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Identifier          |        Sequence Number        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
这种报文是用来找出一个主机所在的网络个数（一个主机可能会在多个网络中）。报文的IP消息头的目的地址会填为全0，表示this，源地址会填为源IP所在的网络IP。


最后，补充一张ICMP报文类型码表：
![在这里插入图片描述](https://img-blog.csdn.net/20181015150052903?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

更多可参考[协议文档](https://www.rfc-editor.org/rfc/rfc792.txt)


#### 几种不发送 ICMP差错报告报文的特殊情况
在有些情况下，为了防止在网络中产生大量的ICMP差错报文（广播风暴），影响网络的正常工作，即使发生差错，也不会产生ICMP差错报文，这些情况包括：
- 对ICMP差错报告报文不再发送 ICMP差错报告报文
- 除第1个IP数据报分片外， 对所有后续分片均不发送ICMP差错报告报文
- 对所有多播IP数据报均不发送 ICMP差错报告报文
- 对具有特殊地址（ 如127.0.0.0 或 0.0.0.0） 的IP数据报不发送ICMP 差错报告报文


#### ICMP应用实例——Ping
Ping 是 ICMP 的一个重要应用，主要用来测试两台主机之间的连通性。

Ping 的原理是通过向目的主机发送 ICMP Echo 请求报文，目的主机收到之后会发送 Echo 回答报文。Ping 会根据时间和成功响应的次数估算出数据包往返时间以及丢包率。

**Ping的完整工作流程：**
Ping本质上是ICMP数据包，所以其工作流程就是ICMP数据包的发送与解析流程。
大致流程如下：
构造ICMP数据包-->构造IP数据包-->构造以太网数据帧----物理传输到目标主机---->获取以太网数据帧-->解析出IP数据包-->解析出ICMP数据包-->发送回送应答报文

本地主机处理流程：

1. 在本地，ping命令会构建一个ICMP数据包（构造回送请求报文）
2. 将该ICMP数据包和目标IP地址给IP协议，IP协议将本地地址作为源地址，与目的地址等构造IP数据包
3. 根据本地ARP缓存查找目的地址IP对应的MAC地址，如果缓存中没有则通过ARP协议找到对应IP的MAC地址。将MAC地址交给数据链路层以构造数据帧
4. 经物理层发送给目的主机

目的主机处理流程：
0. 目的主机接收到数据包
1. 物理层接收到二进制数据流经数据链路层，按照以太网协议解析出数据帧，如果数据帧中的目标MAC地址与本机MAC地址相同，则接收该数据包，否则丢弃该数据包。
2. 接收到该数据包之后，经网络层解析出IP数据包，通过IP包头中的协议字段判断出是ICMP数据包。之后解析出ICMP数据包，发现是回送请求报文（ping request）后马上构建一个ICMP回送应答报文（ping reply）
3. 将封装好的ICMP数据包经网络层、数据链路层、物理层发送回源主机


所以，Ping的本质其实就是ICMP回送请求报文和回送应答报文。我们可通过Wireshark抓包工具对Ping进行分析。使用ping命令：
![在这里插入图片描述](https://img-blog.csdn.net/20181015141800343?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
本机会向远端主机发送ICMP回送请求报文。
![在这里插入图片描述](https://img-blog.csdn.net/20181015141745748?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

远端主机收到后会发送回送应答报文：
![在这里插入图片描述](https://img-blog.csdn.net/20181015142357418?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

在这里补充一张IP数据报格式图：
![在这里插入图片描述](https://img-blog.csdn.net/20181015142630516?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)


#### ICMP应用实例——Traceroute
Traceroute 是 ICMP 的另一个应用，用来跟踪一个分组从源点到终点的路径。有2种实现方案：基于UDP实现和基于ICMP实现。

**基于UDP实现traceroute工作原理：**
在基于UDP的实现中，客户端发送的数据包是通过UDP协议来传输的，使用了一个大于30000的端口号，服务器在收到这个数据包的时候会返回一个端口不可达的ICMP错误信息，客户端通过判断收到的错误信息是TTL超时还是端口不可达来判断数据包是否到达目标主机。流程如下：

1. 源主机向目的主机发送一连串的 IP 数据报（UDP报文）。第一个数据报 P1 的生存时间 TTL 设置为 1，当 P1 到达路径上的第一个路由器 R1 时，R1 收下它并把 TTL 减 1，此时 TTL 等于 0，R1 就把 P1 丢弃，并向源主机发送一个 ICMP 时间超过差错报告报文；
2. 源主机接着发送第二个数据报 P2，并把 TTL 设置为 2。P2 先到达 R1，R1 收下后把 TTL 减 1 再转发给 R2，R2 收下后也把 TTL 减 1，由于此时 TTL 等于 0，R2 就丢弃 P2，并向源主机发送一个 ICMP 时间超过差错报文。
3. 不断执行这样的步骤，直到最后一个数据报刚刚到达目的主机，主机不转发数据报，也不把 TTL 值减 1。但是因为数据报封装的是无法交付的 UDP，因此目的主机要向源主机发送 ICMP 终点不可达差错报告报文。
4. 之后源主机知道了到达目的主机所经过的路由器 IP 地址以及到达每个路由器的往返时间。

Linux中的traceroute就是这种实现方式。

**基于ICMP实现的tracert工作原理：**
在这一种实现中我们不使用UDP协议，而是直接发送一个ICMP回显请求（echo request）数据包，服务器在收到回显请求的时候会向客户端发送一个ICMP回显应答（echo reply）数据包。流程与上面相似，只是最后判断结束上为目标主机（而不是中间经过的主机或路由器）返回一个ICMP回显应答，则结束。

Windows中的tracert就是这种实现方式。利用Wireshark分析如下：
在cmd下输入```tracert www.baidu.com```：
![在这里插入图片描述](https://img-blog.csdn.net/20181015170631206?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

每一跳默认发送三个数据包，我们会看到下面这样的输出：
![在这里插入图片描述](https://img-blog.csdn.net/20181015170948672?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
![在这里插入图片描述](https://img-blog.csdn.net/20181015170959802?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

可以看到TTL逐个递增，并且最终到达目的主机180.97.33.107，到达目的主机，目的主机回复，终止。