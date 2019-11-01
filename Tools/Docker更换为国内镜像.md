#### 更换原因
在国内访问 Docker 官方的镜像，一直以来速度都慢如蜗牛。为了快速访问 Docker 官方镜像都会配置三方加速器，目前常用三方加速器有：网易、USTC、DaoCloud、阿里云。

现在 Docker 官方针对中国区推出了镜像加速服务。通过 Docker 官方镜像加速，国内用户能够以更快的下载速度和更强的稳定性访问最流行的 Docker 镜像。

#### 如何使用官方镜像
Docker 中国官方镜像加速可通过 registry.docker-cn.com 访问。目前该镜像库只包含流行的公有镜像，而私有镜像仍需要从美国镜像库中拉取。


可以使用以下命令直接从该镜像加速地址进行拉取。
```
docker pull registry.docker-cn.com/myname/myrepo:mytag
```
示例如下：
```
docker pull registry.docker-cn.com/library/ubuntu:16.04
```

>注:除非您修改了Docker守护进程的–registry-mirror参数,否则您将需要完整地指定官方镜像的名称。例如，library/ubuntu、library/redis、library/nginx。


#### 给Docker守护进程配置加速器
通过配置文件启动Docker,修改 `/etc/docker/daemon.json` 文件并添加上 `registry-mirrors` 键值。
```
sudo vim /etc/docker/daemon.json
```

```
{
 "registry-mirrors": ["https://registry.docker-cn.com"]
}
```

>也可选用网易的镜像地址：http://hub-mirror.c.163.com
>
{
"registry-mirrors": ["http://hub-mirror.c.163.com"]
}


修改保存后，重启 Docker 以使配置生效。
```
sudo service docker restart
```

然后速度上了一个台阶。


>参考文档：[Docker Documentation](https://docs.docker.com/)