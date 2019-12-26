
### 查看系统详细时间
#### 命令date

date  ——按指定格式显示时间，或者设置系统时间。


#### 基础用法

在命令行下输入date，显示当前系统时间。

```s
sl@Li:~$ date
2018年 07月 03日 星期二 09:34:50 CST
```

#### 其他主要用法

- -d 用来用来规定时间的显示格式

- -s 用来设置时间


格式化输出，可通过```date --h```查看如下：
```
  %%   a literal %
  %a   locale's abbreviated weekday name (e.g., Sun)
  %A   locale's full weekday name (e.g., Sunday)
  %b   locale's abbreviated month name (e.g., Jan)
  %B   locale's full month name (e.g., January)
  %c   locale's date and time (e.g., Thu Mar  3 23:05:25 2005)
  %C   century; like %Y, except omit last two digits (e.g., 20)
  %d   day of month (e.g., 01)
  %D   date; same as %m/%d/%y
  %e   day of month, space padded; same as %_d
  %F   full date; same as %Y-%m-%d
  %g   last two digits of year of ISO week number (see %G)
  %G   year of ISO week number (see %V); normally useful only with %V
  %h   same as %b
  %H   hour (00..23)
  %I   hour (01..12)
  %j   day of year (001..366)
  %k   hour, space padded ( 0..23); same as %_H
  %l   hour, space padded ( 1..12); same as %_I
  %m   month (01..12)
  %M   minute (00..59)
  %n   a newline
  %N   nanoseconds (000000000..999999999)
  %p   locale's equivalent of either AM or PM; blank if not known
  %P   like %p, but lower case
  %r   locale's 12-hour clock time (e.g., 11:11:04 PM)
  %R   24-hour hour and minute; same as %H:%M
  %s   seconds since 1970-01-01 00:00:00 UTC
  %S   second (00..60)
  %t   a tab
  %T   time; same as %H:%M:%S
  %u   day of week (1..7); 1 is Monday
  %U   week number of year, with Sunday as first day of week (00..53)
  %V   ISO week number, with Monday as first day of week (01..53)
  %w   day of week (0..6); 0 is Sunday
  %W   week number of year, with Monday as first day of week (00..53)
  %x   locale's date representation (e.g., 12/31/99)
  %X   locale's time representation (e.g., 23:13:48)
  %y   last two digits of year (00..99)
  %Y   year
  %z   +hhmm numeric time zone (e.g., -0400)
  %:z  +hh:mm numeric time zone (e.g., -04:00)
  %::z  +hh:mm:ss numeric time zone (e.g., -04:00:00)
  %:::z  numeric time zone with : to necessary precision (e.g., -04, +05:30)
  %Z   alphabetic time zone abbreviation (e.g., EDT)
```

例如： 
```
sl@Li:~$ date +"%Y-%m-%d %T"   #显示当前时间
2018-07-03 09:55:33
sl@Li:~$ date -d "+1 day" +"%Y%m%d"     #显示后一天日期
20180704
sl@Li:~$ date -d "-1 day" +"%Y%m%d"     #显示前一天日期
20180702
```

#### Linux查看更高精度的时间
命令如下：
```
sl@Li:~$ date +"%T.%N"  #以纳秒返回当前时间。
10:05:54.316345697
sl@Li:~$ date +"%T.%3N"     #返回当前时间，纳秒四舍五入到前3位数，即毫秒。
10:05:40.182
sl@Li:~$ date +"%T.%6N"     #返回当前时间，纳秒四舍五入到前6位，即微秒
10:06:16.348636

```

---

### 同步网络时间

当当前Linux主机的时间与标准时间不同步时，可使用```ntpdate```进行时间同步。命令如下：```sudo ntpdate <ip address>```。

例如：
```
sudo ntpdate ntp1.aliyun.com #阿里云NTP服务
```

也可使用其他NTP服务器。

>NTP服务器: 在Linux系统中，为了避免主机时间因为在长时间运行下所导致的时间偏差，进行时间同步(synchronize)的工作是非常必要的。Linux系统下，一般使用ntp服务来同步不同机器的时间。NTP 是网络时间协议（Network Time Protocol）的简称，通过网络协议使计算机之间的时间同步化。更多请参考[Linux的NTP配置总结](https://www.cnblogs.com/kerrycode/archive/2015/08/20/4744804.html)。