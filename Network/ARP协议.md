#### ARP地址解析协议（IP地址--->MAC地址）
地址解析协议，即ARP（Address Resolution Protocol），是根据IP地址获取物理地址的一个TCP/IP协议。主机与主机之间的通信在物理上实质是网卡与网卡之间的通信，而网卡只认识MAC地址，所以要想实现主机与主机之间的通信，需要知道对方IP地址所对应的MAC地址，完成这一过程的协议就是ARP协议。在具体的网络传输过程中，使用地址解析协议，可根据网络层IP数据包包头中的IP地址信息解析出目标硬件地址（MAC地址）信息，以保证通信的顺利进行。

#### ARP工作原理
每台主机或路由器都有一个ARP缓存表，用来保存IP地址与MAC地址的对应关系。

以主机A（192.168.1.5）向主机B（192.168.1.1）发送数据为例。当发送数据时，主机A会在自己的ARP缓存表中寻找是否有目标IP地址。如果找到了，也就知道了目标MAC地址，直接把目标MAC地址写入帧里面发送就可以了；如果在ARP缓存表中没有找到目标IP地址，主机A就会在网络上发送一个广播arp request，请求包中包含了A主机的ip地址和mac地址。网络上其他主机并不响应ARP询问，直接丢弃，只有主机B接收到这个帧时，才以单播方式向主机A做出回应arp reply，并带上自己的ip和mac地址，而B主机收到A的请求包时也会将A主机的IP与MAC对应关系保存在自己的缓存区。A收到B的回应包后便可得知B的MAC地址，将其存入ARP缓存。此后A再向B发送数据时，就可以直接从缓存表中查找B的地址了，然后直接把数据发送给B。由于B在接收A的请求时也保存了A的地址信息，因此B要向A发送数据也可以直接从缓存表中查找。

ARP缓存表设置了生存时间TTL，在一段时间内（一般15到20分钟，跟操作系统有关）如果表中的某一行没有使用，就会被删除，这样可以大大减少ARP缓存表的长度，加快查询速度。 

> 如果不是一个网段的，比如广域网，这种情况下怎么办呢？其实在这种情况下源主机不需要知道目的主机的MAC地址，它只需要知道网络中下一跳节点的MAC地址就可以了，后面的交给下一跳去解决，依次类推。

#### ARP封装
可以对ARP报文有个直观的认识，深入的话可以结合WireShark抓包学习。
![在这里插入图片描述](https://img-blog.csdn.net/20181012132414915?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

广播请求：
![在这里插入图片描述](https://img-blog.csdn.net/20181012171800717?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

回应：
![在这里插入图片描述](https://img-blog.csdn.net/20181012172409361?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

补充一个以太网帧封装：
![在这里插入图片描述](https://img-blog.csdn.net/2018101217245417?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

再补充一个ARP协议在TCP/IP协议中的位置：
![在这里插入图片描述](https://img-blog.csdn.net/20181012172819695?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

#### arp命令
可通过如下命令查看高速缓存中的所有项目:
```
arp -a
```
其他选项：
```
-a 显示所有接口的当前ARP缓存表；

-v 显示详细信息； 

-n 以数字地址形式显示； 
```

#### RARP协议（MAC地址--->IP地址）
反向地址转换协议，RARP（Reverse Address Resolution Protocol），就是将局域网中某个主机的物理地址转换为IP地址。

#### MAC地址
MAC（Medium/Media Access Control）地址，用来表示互联网上每一个站点的标识符，采用十六进制数表示，共六个字节（48位）。其中，前三个字节是由IEEE的注册管理机构RA负责给不同厂家分配的代码(高位24位），也称为“编制上唯一的标识符”（Organizationally Unique Identifier），后三个字节(低位24位)由各厂家自行指派给生产的适配器接口，称为扩展标识符（唯一性）。一个地址块可以生成224个不同的地址。MAC地址实际上就是适配器地址或适配器标识符EUI-48。
>因为这个MAC地址是全球唯一的，所以在分布式系统中，生成UUID的时候，经常会用MAC地址来标识一台主机ID。