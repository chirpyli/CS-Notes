
#### 问题描述：
当其他主机向本地虚拟机中运行的服务程序发送数据时，服务端无法收到数据，wireshark抓包分析后，发现错误提示信息：`Destination unreachable (Host administratively prohibited)`。意思是：目的主机被强制禁止。基本可以断定是防火墙的问题。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20181218094334661.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==,size_16,color_FFFFFF,t_70)


#### 解决办法：
关闭防火墙。

在Ubuntu环境中可执行如下命令(其实关闭防火墙一定程度上就等于允许所有包通过)：
```
sudo iptables -P INPUT ACCEPT
sudo iptables -P FORWARD ACCEPT
sudo iptables -P OUTPUT ACCEPT
sudo iptables -F
```

问题解决。