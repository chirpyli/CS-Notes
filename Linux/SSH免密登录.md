为了工作方便，我们经常需要在两台机器之间进行免密登录设置，比如A机器免密登录B机器。设置SSH免密登录步骤如下：
1. `ssh-keygen -t rsa`生成`.ssh`文件
```
[sl@slpc ~]$ ssh-keygen -t rsa
Generating public/private rsa key pair.
Enter file in which to save the key (/home/sl/.ssh/id_rsa): 
Enter passphrase (empty for no passphrase): 
Enter same passphrase again: 
Your identification has been saved in /home/sl/.ssh/id_rsa.
Your public key has been saved in /home/sl/.ssh/id_rsa.pub.
The key fingerprint is:
SHA256:x1rxar/4kYvIkJ/hyuKs5EJ/bW2VyN1xfc/5LIWPuQA sl@slpc
The key's randomart image is:
+---[RSA 2048]----+
|                 |
|                 |
|          .     .|
|         . o . .o|
|        S E + oo+|
| .     . * * o..+|
|. o   + + + +  *.|
|.o o.o B B + ++ +|
| .oo+.+.B o.=..o |
+----[SHA256]-----+

```
2. 生成`.ssh`文件后，新建`authorized_keys`文件，并将文件权限修改为`600`
```
[sl@slpc .ssh]$ touch authorized_keys
[sl@slpc .ssh]$ chmod 600 authorized_keys 
[sl@slpc .ssh]$ ll
total 12
-rw-------. 1 sl sl    0 Jun 16 14:46 authorized_keys   // 存放远程免密登录的公钥,主要通过这个文件记录多台机器的公钥
-rw-------. 1 sl sl 1679 Jun 16 14:41 id_rsa
-rw-r--r--. 1 sl sl  389 Jun 16 14:41 id_rsa.pub     // 生成的公钥文件
-rw-r--r--. 1 sl sl 1956 Jun 10 19:22 known_hosts
```
3. 在需要免密登录的另一台机子，做同样的设置。

4. 然后将本机的公钥添加到远程机器的`authorized_keys`文件中，可以用`scp -p`传输到远程主机：
```
[sl@slpc .ssh]$ scp -p id_rsa.pub postgres@10.19.36.10:~/.ssh/authorized_keys

Authorized users only. All activities may be monitored and reported.
postgres@10.19.36.10's password: 
id_rsa.pub                            100%  389    83.7KB/s   00:00
```
之后就可以免密登录了。


>参考文档：[Linux下ssh高级使用–免密登录](https://www.linuxprobe.com/linux-ssh-withon-passwd.html)

