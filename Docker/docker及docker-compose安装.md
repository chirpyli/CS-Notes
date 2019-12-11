
### 安装docker步骤

#### 一、SET UP THE REPOSITORY
1、Update the apt package index:
```
sudo apt-get update
```
2、Install packages to allow apt to use a repository over HTTPS:
```
sudo apt-get install \
    apt-transport-https \
    ca-certificates \
    curl \
    software-properties-common
```
3、 Add Docker’s official GPG key:
```
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
```
4、Use the following command to set up the stable repository. 
```
sudo add-apt-repository \
   "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
   $(lsb_release -cs) \
   stable"
```

#### 二、INSTALL DOCKER CE
1、Update the apt package index.
```
sudo apt-get update
```
2、 Install the latest version of Docker CE, or go to the next step to install a specific version:
```
sudo apt-get install docker-ce
```
3、Verify that Docker CE is installed correctly by running the hello-world image.
```
sudo docker run hello-world
```

#### 三、To create the docker group and add your user（本步骤非必须，不过不加的话，需要sudo docker命令）:
1、Create the docker group.
```
sudo groupadd docker
```
2、Add your user to the docker group.
```
sudo usermod -aG docker $USER
```

### 安装docker-compose步骤
1、Run this command to download the latest version of Docker Compose:
```
sudo curl -L https://github.com/docker/compose/releases/download/1.20.1/docker-compose-$(uname -s)-$(uname -m) -o /usr/local/bin/docker-compose
```
2、Apply executable permissions to the binary:
```
sudo chmod +x /usr/local/bin/docker-compose
```
3、Test the installation.
```
docker-compose --version
```

#### 安装docker-compose可能出现的异常及解决办法
如果安装过程中遇到这个问题：curl: (7) Failed to connect to github-production-release-asset-2e65be.s3.amazonaws.com port 443: Connection timed out.

问题原因：该地址(github-production-release-asset-2e65be.s3.amazonaws.com) 需要的一些下载的访问被国内屏蔽了，需要修改hosts文件,使用香港的服务器。

解决办法如下：
1. 修改hosts，```sudo gedit /etc/hosts```，添加219.76.4.4 github-cloud.s3.amazonaws.com
2. 保存并重启网络```sudo /etc/init.d/networking restart```
3.  重新按照上面Install Compose安装步骤重新安装即可。

#### 关于hosts
hosts是一个没有扩展名的系统文件，其作用就是将一些常用的网址域名与其对应的IP地址建立一个关联“数据库”，当用户在浏览器中输入一个需要登录的网址时，系统会首先自动从hosts文件中寻找对应的IP地址，一旦找到，系统会立即打开对应网页，如果没有找到，则系统会再将网址提交DNS域名解析服务器进行IP地址的解析。

操作系统规定，在进行DNS请求以前，先检查系自己的hosts文件中是否有这个域名和IP的映射关系。如果有，则直接访问这个IP地址指定的网络位置，如果没有，再向已知的DNS服务器提出域名解析请求。也就是说hosts的IP解析优先级比DNS要高。更准确一些是：dns缓存 > hosts > dns服务。

#### hosts文件的工作方式以及它在具体使用中起哪些作用
1. **加快域名解析**
对于要经常访问的网站，我们可以通过在hosts中配置域名和IP的映射关系，提高域名解析速度。由于有了映射关系，当我们输入域名计算机就能很快解析出IP，而不用请求网络上的DNS服务器。
2. **方便局域网用户**
在很多单位的局域网中，会有服务器提供给用户使用。但由于局域网中一般很少架设DNS服务器，访问这些服务器时，要输入难记的IP地址。这对不少人来说相当麻烦。可以分别给这些服务器取个容易记住的名字，然后在hosts中建立IP映射，这样以后访问的时候，只要输入这个服务器的名字就行了。
3. **屏蔽网站（域名重定向）**
有很多网站不经过用户同意就将各种各样的插件安装到你的计算机中，其中有些说不定就是木马或病毒。对于这些网站我们可以利用Hosts把该网站的域名映射到错误的IP或本地计算机的IP，这样就不用访问了。在WINDOWS系统中，约定 127.0.0.1 为本地计算机的IP地址, 0.0.0.0是错误的IP地址。
如果，我们在Hosts中，写入以下内容：
127.0.0.1 # 要屏蔽的网站 A
0.0.0.0 # 要屏蔽的网站 B
这样，计算机解析域名A和 B时，就解析到本机IP或错误的IP，达到了屏蔽网站A 和B的目的。
4. **顺利连接系统**
对于Lotus的服务器和一些数据库服务器，在访问时如果直接输入IP地址那是不能访问的，只能输入服务器名才能访问。那么我们配置好hosts文件，这样输入服务器名就能顺利连接了。
5. **虚拟域名**
很多时候，网站建设者需要把”软环境“搭建好，再进行上传调试。但类似于邮件服务，则需要使用域名来辅助调试，这时就可以将本地 IP 地址与一个”虚拟域名“做地址指向，就可以达到要求的效果，且无需花费。如：
　　127.0.0.1 网站域名
　　之后在浏览器地址栏中输入对应的网站域名即可。


>参考文档：     
[Get Docker CE for Ubuntu](https://docs.docker.com/install/linux/docker-ce/ubuntu/)     
[Install Docker Compose](https://docs.docker.com/compose/install/)      
[安装docker-compose异常： Failed to connect to github-production-release-XX-](https://www.aliyun.com/jiaocheng/125539.html)     
[Linux hosts文件详解及配置](https://www.linuxidc.com/Linux/2016-10/135886.htm)          