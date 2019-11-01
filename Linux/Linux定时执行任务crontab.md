#### crontab
`crontab`可以定时执行任务，比如可以配置为每天几点执行什么任务，其实还可以配置为开机后执行一个任务。

命令行输入`crontab -e`可进入编辑，每一行是一个定时任务，每个定时任务表示如下：
```
# ┌───────────── minute (0 - 59)
# │ ┌───────────── hour (0 - 23)
# │ │ ┌───────────── day of the month (1 - 31)
# │ │ │ ┌───────────── month (1 - 12)
# │ │ │ │ ┌───────────── day of the week (0 - 6) (Sunday to Saturday;
# │ │ │ │ │                                   7 is also Sunday on some systems)
# │ │ │ │ │
# │ │ │ │ │
# * * * * * command to execute
```
很好理解，就是 `定时时间 + 任务命令`，另外有几个较为特殊的时间表示如下：
|Entry|	Description | Equivalent to |
|--|--|--|
@yearly (or @annually)	|Run once a year at midnight of 1 January|	0 0 1 1 *
@monthly	|Run once a month at midnight of the first day of the month	|0 0 1 * *
@weekly	|Run once a week at midnight on Sunday morning	|0 0 * * 0
@daily	|Run once a day at midnight|	0 0 * * *
@hourly	|Run once an hour at the beginning of the hour	|0 * * * *
<font color=red>@reboot	|<font color=red>Run at startup	|N/A|

这里用到`@reboot`，用法参考下面的示例。

#### 示例
命令行输入`crontab -e`进入编辑：
```Shell
sl@Li:~$ crontab -e
# Edit this file to introduce tasks to be run by cron.
# 
# Each task to run has to be defined through a single line
# indicating with different fields when the task will be run
# and what command to run for the task
# 
# To define the time you can provide concrete values for
# minute (m), hour (h), day of month (dom), month (mon),
# and day of week (dow) or use '*' in these fields (for 'any').# 
# Notice that tasks will be started based on the cron's system
# daemon's notion of time and timezones.
# 
# Output of the crontab jobs (including errors) is sent through
# email to the user the crontab file belongs to (unless redirected).
# 
# For example, you can run a backup of all your user accounts
# at 5 a.m every week with:
# 0 5 * * 1 tar -zcf /var/backups/home.tgz /home/
# 
# For more information see the manual pages of crontab(5) and cron(8)
# 
# m h  dom mon dow   command
@reboot sh /home/sl/System/mount_share.sh

```
其中脚本，换成其他任务即可：
```Shell
#!/bin/sh
echo asdf | sudo -S mount -t vboxsf Share /home/sl/share
```
编辑好后按`CTRL+x`，按提示保存退出即可。这样当机器重启时，就会执行挂载虚拟机共享文件夹任务。



>参考文档：     
[Cron](https://en.wikipedia.org/wiki/Cron)      
[Ubuntu document cron](https://help.ubuntu.com/community/CronHowto)     
[Linux定时执行任务Crontab](https://www.jellythink.com/archives/155)

