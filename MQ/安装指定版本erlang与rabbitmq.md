安装部署Rabbitmq的时候要注意erlang与rabbitmq的版本问题，可参考
[RabbitMQ Erlang Version Requirements](http://www.rabbitmq.com/which-erlang.html)选择对应的版本进行安装部署。

### 一、安装Erlang

1. Adding repository entry
```shell
wget https://packages.erlang-solutions.com/erlang-solutions_1.0_all.deb
sudo dpkg -i erlang-solutions_1.0_all.deb

```

2. Installing Erlang
```shell
sudo apt-get update
sudo apt-cache madison esl-erlang #列出版本后，选择一个版本安装
sudo apt install esl-erlang=1:20.3
```

### 二、安装rabbitmq
 
1. Add apt repositories
```shell
echo "deb https://dl.bintray.com/rabbitmq/debian xenial main" | sudo tee /etc/apt/sources.list.d/bintray.rabbitmq.list
```
2. Next add our public key to your trusted key list using apt-key(8):
```shell
wget -O- https://www.rabbitmq.com/rabbitmq-release-signing-key.asc | sudo apt-key add -
```
3. Run the following command to update the package list:
```shell
sudo apt-get update
```
4. Install rabbitmq-server package:
```shell
sudo apt install rabbitmq-server=3.7.6-1
```



### 三、启动rabbitmq
启动rabbitmq:

```shell
sudo service rabbitmq-server start
```

### 四、 Rabbitmq常用命令
查看rabbimtq服务状态：`service rabbitmq-server status`:
```shell
ubuntu@localhost:~$ sudo service rabbitmq-server status
● rabbitmq-server.service - RabbitMQ broker
   Loaded: loaded (/lib/systemd/system/rabbitmq-server.service; disabled; vendor preset: enabled)
   Active: active (running) since Fri 2019-08-23 11:14:07 CST; 8min ago
 Main PID: 57802 (beam.smp)
   Status: "Initialized"
    Tasks: 90
   Memory: 79.0M
      CPU: 5.419s
   CGroup: /system.slice/rabbitmq-server.service
           ├─57802 /usr/local/lib/erlang/erts-9.3/bin/beam.smp -W w -A 64 -MBas ageffcbf -MHas ageffcb
           ├─57888 /usr/local/lib/erlang/erts-9.3/bin/epmd -daemon
           ├─58074 erl_child_setup 1024
           ├─58096 inet_gethost 4
           └─58097 inet_gethost 4

Aug 23 11:14:05 localhost rabbitmq-server[57802]:   ##  ##
Aug 23 11:14:05 localhost rabbitmq-server[57802]:   ##  ##      RabbitMQ 3.7.6. Copyright (C) 2007-201
Aug 23 11:14:05 localhost rabbitmq-server[57802]:   ##########  Licensed under the MPL.  See http://ww
Aug 23 11:14:05 localhost rabbitmq-server[57802]:   ######  ##
Aug 23 11:14:05 localhost rabbitmq-server[57802]:   ##########  Logs: /var/log/rabbitmq/rabbit@localho
Aug 23 11:14:05 localhost rabbitmq-server[57802]:                     /var/log/rabbitmq/rabbit@localho
Aug 23 11:14:05 localhost rabbitmq-server[57802]:               Starting broker...
Aug 23 11:14:07 localhost rabbitmq-server[57802]: systemd unit for activation check: "rabbitmq-server.
Aug 23 11:14:07 localhost systemd[1]: Started RabbitMQ broker.
Aug 23 11:14:08 localhost rabbitmq-server[57802]:  completed with 0 plugins.
```
下面列出一些常用的命令，更多可参考[rabbitmqctl](https://www.rabbitmq.com/man/rabbitmqctl.8.html)和[官网](https://www.rabbitmq.com/man/rabbitmqctl.8.html)：
```shell
sudo chkconfig rabbitmq-server on  #添加开机启动（chkconfig一般只有redhat系统有）RabbitMQ服务
sudo service rabbitmq-server start  # 启动服务
sudo service rabbitmq-server status  # 查看服务状态
sudo service rabbitmq-server stop   # 停止服务

sudo rabbitmqctl stop   # 停止服务
sudo rabbitmqctl status  # 查看服务状态
sudo rabbitmqctl list_users # 查看当前所有用户
sudo rabbitmqctl list_user_permissions guest # 查看默认guest用户的权限
sudo rabbitmqctl delete_user guest# 删掉默认用户(由于RabbitMQ默认的账号用户名和密码都是guest。为了安全起见, 可以删掉默认用户）
sudo rabbitmqctl add_user username password # 添加新用户
sudo rabbitmqctl set_user_tags username administrator# 设置用户tag
sudo rabbitmqctl set_permissions -p / username ".*" ".*" ".*" # 赋予用户默认vhost的全部操作权限
sudo rabbitmqctl list_user_permissions username # 查看用户的权限
```

### 五、开启web管理接口
rabbitmq既可以命令行操作，也可以用rabbitmq自带的web管理界面，只需要启动插件便可以使用。
```shell
sudo rabbitmq-plugins enable rabbitmq_management
```
然后通过浏览器访问，如果是本机则可以输入<http://127.0.0.1:15672>打开登录界面，输入用户名和密码访问web管理界面了。默认用户名guest密码guest。
>更多可参考[Management Plugin](http://www.rabbitmq.com/management.html)。

### 六、Rabbitmq配置
一般使用默认配置就好了，具体配置可参考[RabbitMQ Configuration](http://www.rabbitmq.com/configure.html)。

#### rabbitmq端口
以下为rabbitmq的默认端口，如果这些端口被其他程序占用，rabbitmq就会出错。
- 4369: epmd, a peer discovery service used by RabbitMQ nodes and CLI tools
- 5672, 5671: used by AMQP 0-9-1 and 1.0 clients without and with TLS
- 25672: used for inter-node and CLI tools communication (Erlang distribution server port) and is allocated from a dynamic range (limited to a single port by default, computed as AMQP port + 20000). See networking guide for details.
- 35672-35682: used by CLI tools (Erlang distribution client ports) for communication with nodes and is allocated from a dynamic range (computed as Erlang dist port + 10000 through dist port + 10010). See networking guide for details.
- 15672: HTTP API clients and rabbitmqadmin (only if the management plugin is enabled)
- 61613, 61614: STOMP clients without and with TLS (only if the STOMP plugin is enabled)
- 1883, 8883: (MQTT clients without and with TLS, if the MQTT plugin is enabled
- 15674: STOMP-over-WebSockets clients (only if the Web STOMP plugin is enabled)
- 15675: MQTT-over-WebSockets clients (only if the Web MQTT plugin is enabled)

如果有相冲突的端口的话，可以参考[Networking and RabbitMQ](http://www.rabbitmq.com/networking.html)更改rabbitmq配置或者去down掉占用端口的那个应用程序或进程吧。