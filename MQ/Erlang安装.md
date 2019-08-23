

### Erlang编译安装。     

#### 安装依赖项
1. gcc/g++、make等开发工具
```
 sudo apt-get install build-essential
```
2. 其它Erlang用到的关键库
```
sudo apt-get install libncurses5-dev m4 libssl-dev
```
3. 其他库
```
sudo apt-get install unixodbc unixodbc-dev libc6
```
4. wxWidge是非必须安装的，可以不安装。如安装可参考[Installing and configuring under Ubuntu](https://wiki.wxwidgets.org/Installing_and_configuring_under_Ubuntu)。另jdk不安装的话需要在configure中加--without-javac项。


#### 获取源码，编译安装
```
wget http://erlang.org/download/otp_src_20.3.tar.gz
tar -zxvf otp_src_20.3.tar.gz
cd otp_src_20.3
./configure --without-javac
make
sudo make install
```
---

### Installation using repository

```
wget https://packages.erlang-solutions.com/erlang-solutions_1.0_all.deb
sudo dpkg -i erlang-solutions_1.0_all.deb
sudo apt-get update
sudo apt-cache madison esl-erlang #列出一个软件的版本的命令是 sudo apt-cache madison soft_name
sudo apt install esl-erlang=1:20.3
```