##### 可能原因及解决办法
在新安装ubuntu16.04后发现网卡驱动加载了，但是连接不上网，原因之一可能是DNS服务器地址没有配置。

解决办法：
1. 执行 ```sudo vim /etc/network/interfaces```，向文件中添加dns-nameserver 114.114.114.114
2. 执行```sudo vim /etc/resolvconf/resolv.conf.d/base```，在这个文件后面追加nameserver 114.114.114.114
3. 执行```sudo vim /etc/resolv.conf```， 添加 nameserver 114.114.114.114
4. 执行```sudo service networking restart```，重启网络即可



##### 网络服务的启动、关闭和重启
Linux(Ubuntu，不同Linux可能会不一样)网络服务的启动、关闭和重启：
```
sudo service networking stop
sudo service networking start
sudo service networking restart
```
##### 关于DNS
DNS（Domain Name System，域名系统），万维网上作为域名和IP地址相互映射的一个分布式数据库，能够使用户更方便的访问互联网，而不用去记住能够被机器直接读取的IP数串。起一个转换作用：域名-->IP地址。

常用DNS：

- 8.8.8.8——Google提供的免费DNS服务器的IP地址
- 114.114.114.114——国内用户上网常用的DNS
- 223.5.5.5——阿里 AliDNS。阿里公共DNS是阿里巴巴集团推出的DNS递归解析系统，目标是成为国内互联网基础设施的组成部分，面向互联网用户提供“快速”、“稳定”、“智能”的免费DNS递归解析服务。