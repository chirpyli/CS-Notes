supervisor是Linux下常用的一个进程管理工具。最常见的一个应用场景是监控服务器上的某个应用服务，如果该应用服务异常退出，则重启该服务，保证服务不被长时间中断。

#### 安装（Ubuntu-16.04LTS）
```shell
sudo apt-get install supervisor
```
安装好后可运行```sudo service supervisor status```查看安装后是否正在运行。

#### 使用
主要是配置```/etc/supervisord.conf```文件及```supervisorctl```、```supervisord```命令的使用。

##### 配置文件如下：
```shell
sl@Li:/etc/supervisor$ cat supervisord.conf 
; supervisor config file

[unix_http_server]
file=/var/run/supervisor.sock   ; (the path to the socket file)
chmod=0700                       ; sockef file mode (default 0700)

[supervisord]
logfile=/var/log/supervisor/supervisord.log ; (main log file;default $CWD/supervisord.log)
pidfile=/var/run/supervisord.pid ; (supervisord pidfile;default supervisord.pid)
childlogdir=/var/log/supervisor            ; ('AUTO' child log dir, default $TEMP)

; the below section must remain in the config file for RPC
; (supervisorctl/web interface) to work, additional interfaces may be
; added by defining them in separate rpcinterface: sections
[rpcinterface:supervisor]
supervisor.rpcinterface_factory = supervisor.rpcinterface:make_main_rpcinterface

[supervisorctl]
serverurl=unix:///var/run/supervisor.sock ; use a unix:// URL  for a unix socket

; The [include] section can just contain the "files" setting.  This
; setting can list multiple files (separated by whitespace or
; newlines).  It can also contain wildcards.  The filenames are
; interpreted as relative to this file.  Included files *cannot*
; include files themselves.

[include]
files = /etc/supervisor/conf.d/*.conf   
```
可以直接在```/etc/supervisor/supervisord.conf```写进程管理的配置，不过更推荐将需要管理的进程的配置信息写到```/etc/supervisor/conf.d/```目录下。

>对如何配置及每一配置项的含义可以参考文档：[Configuration File](http://supervisord.org/configuration.html#configuration-file)，这里不再细述。

##### 常用相关命令
```
supervisorctl start app    #启动某个进程
supervisorctl restart app   #重启某个进程
supervisorctl stop app   #关闭某个进程(显式用stop停止掉的进程，用reload或者update都不会自动重启)
supervisorctl reload    #载入最新的配置文件，停止原有进程并按新的配置启动、管理所有进程。
supervisorctl update    #根据最新的配置文件，启动新配置或有改动的进程，配置没有改动的进程不会受影响而重启。
```


>```supervisord```命令参考：[Running supervisord](http://supervisord.org/running.html#running-supervisord)
>```supervisorctl```命令参考：[Running supervisorctl](http://supervisord.org/running.html#running-supervisorctl)


#### 示例
在日志系统中，常使用Filebeat收集服务端产生的日志信息，如果Filebeat异常退出，则有可能会造成日志的遗漏，所有需要当Filebeat异常退出时短时间内再拉起来。相关配置如下：
在```/etc/supervisor/conf.d/```目录下创建```filebeat.conf```配置文件，具体配置如下：
```shell
[program:filebeat]
command=/home/sl/Works/elk/filebeat/filebeat 
directory=/home/sl/Works/elk/filebeat
autostart=true
autorestart=true
user=sl

```
>各项配置的含义可参考：[[program:x] Section Settings](http://supervisord.org/configuration.html#program-x-section-settings)

配置好后运行```sudo supervisorctl reload```即可。

再去查看进程信息```sudo supervisorctl status```，会发现filebeat进程已经启动：
```shell
sl@Li:/etc/supervisor/conf.d$ sudo supervisorctl status
[sudo] password for sl: 
filebeat                         RUNNING   pid 29859, uptime 0:02:18
```


>参考文档：     
[Supervisor: A Process Control System](http://supervisord.org/)     
[supervisor-github](https://github.com/Supervisor/supervisor)