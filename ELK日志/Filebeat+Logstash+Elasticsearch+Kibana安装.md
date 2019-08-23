

### Filebeat安装
Filebeat使用Go语言，无须安装java运行环境。
#### 安装Filebeat
从官网下载安装包，解压后放到你想要安装的目录就可以了
```shell
curl -L -O https://artifacts.elastic.co/downloads/beats/filebeat/filebeat-6.5.1-linux-x86_64.tar.gz
tar xzvf filebeat-6.5.1-linux-x86_64.tar.gz
```

#### 运行
修改配置文件```filebeat.yml```

启动```./filebeat -c filebeat.yml```
（*测试学习阶段可以这样启动*）

---

### Logstash安装

#### 前期准备
确保JDK版本为1.8以上

To check your Java version, run the following command:
```shell
java -version
```

安装apt-transport-https
```shell
sudo apt-get install apt-transport-https
```
#### 安装oracle Java JDK
```shell
sudo apt-get install python-software-properties

sudo add-apt-repository ppa:webupd8team/java

sudo apt-get update

sudo apt-get install oracle-java8-installer
```

查看版本
```shell
java -version
```
运行结果：
```shell
cai@xin-x790:~$ java -version
java version "1.8.0_191"
Java(TM) SE Runtime Environment (build 1.8.0_191-b12)
Java HotSpot(TM) 64-Bit Server VM (build 25.191-b12, mixed mode)

```

#### 安装logstash
从官网下载安装包，解压后放到你想要安装的目录就可以了
```shell
wget https://artifacts.elastic.co/downloads/logstash/logstash-6.5.1.tar.gz
tar -zxvf logstash-6.5.1.tar.gz
```
#### 检测是否安装成功
测试安装是否成功，进入安装目录，运行以下命令：
```shell
bin/logstash -e 'input { stdin { } } output { stdout {} }'
```
>-e 参数表示执行后边的语句，标志使您能够直接从命令行指定配置。在命令行中指定配置允许您快速测试配置，而无需在迭代之间编辑文件。示例中的管道从标准输入stdin获取输入，并以结构化格式将输入移动到标准输出stdout。


输入helloworld，运行结果如下：
```shell
sl@Li:/usr/local/logstash$ bin/logstash -e 'input { stdin { } } output { stdout {} }'
helloworldSending Logstash logs to /usr/local/logstash/logs which is now configured via log4j2.properties
[2018-11-30T10:32:27,261][WARN ][logstash.config.source.multilocal] Ignoring the 'pipelines.yml' file because modules or command line options are specified
[2018-11-30T10:32:27,273][INFO ][logstash.runner          ] Starting Logstash {"logstash.version"=>"6.5.0"}
[2018-11-30T10:32:29,257][INFO ][logstash.pipeline        ] Starting pipeline {:pipeline_id=>"main", "pipeline.workers"=>4, "pipeline.batch.size"=>125, "pipeline.batch.delay"=>50}
[2018-11-30T10:32:29,394][INFO ][logstash.pipeline        ] Pipeline started successfully {:pipeline_id=>"main", :thread=>"#<Thread:0x5c08e158 run>"}
The stdin plugin is now waiting for input:
[2018-11-30T10:32:29,525][INFO ][logstash.agent           ] Pipelines running {:count=>1, :running_pipelines=>[:main], :non_running_pipelines=>[]}
[2018-11-30T10:32:29,755][INFO ][logstash.agent           ] Successfully started Logstash API endpoint {:port=>9600}

{
       "message" => "helloworld",
      "@version" => "1",
    "@timestamp" => 2018-11-30T02:32:47.739Z,
          "host" => "Li"
}
```


#### Logstash使用配置文件的方式运行
上述，是通过命令行运行的，下边就是通过配置文件运行的一个完整的例子：

配置文件logstash.conf 
```shell
input {
        stdin {
        }
}

output {
        stdout {
        }
}
~    
```
等同于上面的命令,运行结果相同：
```shell
bin/logstash -f logstash.conf
```
>f 表示指定使用哪一个配置文件进行执行。

---
### Elasticsearch安装

#### 前期准备
与logstash相同，Elasticsearch需要确保JDK版本为1.8以上

To check your Java version, run the following command:
```shell
java -version
```

#### 安装Elasticsearch
获取Elasticsearch
```shell
curl -L -O https://artifacts.elastic.co/downloads/elasticsearch/elasticsearch-6.5.1.tar.gz
```
解压
```shell
tar -xvf elasticsearch-6.5.1.tar.gz
```
将目录拷贝到安装目录即完成安装。

#### 运行
进入elasticsearch bin目录
```shell
cd elasticsearch-6.5.1/bin
```
运行```./elasticsearch```，如果没错的话，运行结果如下：
```shell
sl@Li:/usr/local/elasticsearch/bin$ ./elasticsearch
[2018-12-03T14:13:45,169][INFO ][o.e.e.NodeEnvironment    ] [fDPpOQe] using [1] data paths, mounts [[/ (/dev/sda1)]], net usable_space [259.5gb], net total_space [310.9gb], types [ext4]
[2018-12-03T14:13:45,172][INFO ][o.e.e.NodeEnvironment    ] [fDPpOQe] heap size [990.7mb], compressed ordinary object pointers [true]
[2018-12-03T14:13:45,174][INFO ][o.e.n.Node               ] [fDPpOQe] node name derived from node ID [fDPpOQeySbqIdXW3AJL6fQ]; set [node.name] to override
[2018-12-03T14:13:45,174][INFO ][o.e.n.Node               ] [fDPpOQe] version[6.5.1], pid[29569], build[default/tar/8c58350/2018-11-16T02:22:42.182257Z], OS[Linux/4.15.0-39-generic/amd64], JVM[Oracle Corporation/OpenJDK 64-Bit Server VM/1.8.0_191/25.191-b12]
[2018-12-03T14:13:45,174][INFO ][o.e.n.Node               ] [fDPpOQe] JVM arguments [-Xms1g, -Xmx1g, -XX:+UseConcMarkSweepGC, -XX:CMSInitiatingOccupancyFraction=75, -XX:+UseCMSInitiatingOccupancyOnly, -XX:+AlwaysPreTouch, -Xss1m, -Djava.awt.headless=true, -Dfile.encoding=UTF-8, -Djna.nosys=true, -XX:-OmitStackTraceInFastThrow, -Dio.netty.noUnsafe=true, -Dio.netty.noKeySetOptimization=true, -Dio.netty.recycler.maxCapacityPerThread=0, -Dlog4j.shutdownHookEnabled=false, -Dlog4j2.disable.jmx=true, -Djava.io.tmpdir=/tmp/elasticsearch.ZpLFrZb5, -XX:+HeapDumpOnOutOfMemoryError, -XX:HeapDumpPath=data, -XX:ErrorFile=logs/hs_err_pid%p.log, -XX:+PrintGCDetails, -XX:+PrintGCDateStamps, -XX:+PrintTenuringDistribution, -XX:+PrintGCApplicationStoppedTime, -Xloggc:logs/gc.log, -XX:+UseGCLogFileRotation, -XX:NumberOfGCLogFiles=32, -XX:GCLogFileSize=64m, -Des.path.home=/usr/local/elasticsearch, -Des.path.conf=/usr/local/elasticsearch/config, -Des.distribution.flavor=default, -Des.distribution.type=tar]
[2018-12-03T14:13:46,637][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [aggs-matrix-stats]
[2018-12-03T14:13:46,637][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [analysis-common]
[2018-12-03T14:13:46,637][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [ingest-common]
[2018-12-03T14:13:46,637][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [lang-expression]
[2018-12-03T14:13:46,637][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [lang-mustache]
[2018-12-03T14:13:46,637][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [lang-painless]
[2018-12-03T14:13:46,638][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [mapper-extras]
[2018-12-03T14:13:46,638][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [parent-join]
[2018-12-03T14:13:46,638][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [percolator]
[2018-12-03T14:13:46,638][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [rank-eval]
[2018-12-03T14:13:46,638][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [reindex]
[2018-12-03T14:13:46,638][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [repository-url]
[2018-12-03T14:13:46,638][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [transport-netty4]
[2018-12-03T14:13:46,638][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [tribe]
[2018-12-03T14:13:46,638][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-ccr]
[2018-12-03T14:13:46,638][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-core]
[2018-12-03T14:13:46,638][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-deprecation]
[2018-12-03T14:13:46,639][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-graph]
[2018-12-03T14:13:46,639][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-logstash]
[2018-12-03T14:13:46,639][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-ml]
[2018-12-03T14:13:46,639][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-monitoring]
[2018-12-03T14:13:46,639][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-rollup]
[2018-12-03T14:13:46,639][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-security]
[2018-12-03T14:13:46,639][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-sql]
[2018-12-03T14:13:46,639][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-upgrade]
[2018-12-03T14:13:46,639][INFO ][o.e.p.PluginsService     ] [fDPpOQe] loaded module [x-pack-watcher]
[2018-12-03T14:13:46,640][INFO ][o.e.p.PluginsService     ] [fDPpOQe] no plugins loaded
[2018-12-03T14:13:49,306][INFO ][o.e.x.s.a.s.FileRolesStore] [fDPpOQe] parsed [0] roles from file [/usr/local/elasticsearch/config/roles.yml]
[2018-12-03T14:13:49,791][INFO ][o.e.x.m.j.p.l.CppLogMessageHandler] [fDPpOQe] [controller/29677] [Main.cc@109] controller (64 bit): Version 6.5.1 (Build 1c5fe241dd9aea) Copyright (c) 2018 Elasticsearch BV
[2018-12-03T14:13:50,249][DEBUG][o.e.a.ActionModule       ] [fDPpOQe] Using REST wrapper from plugin org.elasticsearch.xpack.security.Security
[2018-12-03T14:13:50,441][INFO ][o.e.d.DiscoveryModule    ] [fDPpOQe] using discovery type [zen] and host providers [settings]
[2018-12-03T14:13:51,143][INFO ][o.e.n.Node               ] [fDPpOQe] initialized
[2018-12-03T14:13:51,143][INFO ][o.e.n.Node               ] [fDPpOQe] starting ...
[2018-12-03T14:13:51,323][INFO ][o.e.t.TransportService   ] [fDPpOQe] publish_address {127.0.0.1:9300}, bound_addresses {[::1]:9300}, {127.0.0.1:9300}
[2018-12-03T14:13:51,342][WARN ][o.e.b.BootstrapChecks    ] [fDPpOQe] max virtual memory areas vm.max_map_count [65530] is too low, increase to at least [262144]
[2018-12-03T14:13:54,404][INFO ][o.e.c.s.MasterService    ] [fDPpOQe] zen-disco-elected-as-master ([0] nodes joined), reason: new_master {fDPpOQe}{fDPpOQeySbqIdXW3AJL6fQ}{1pXLTM5-SfOGx5EVsx1v8g}{127.0.0.1}{127.0.0.1:9300}{ml.machine_memory=9420918784, xpack.installed=true, ml.max_open_jobs=20, ml.enabled=true}
[2018-12-03T14:13:54,407][INFO ][o.e.c.s.ClusterApplierService] [fDPpOQe] new_master {fDPpOQe}{fDPpOQeySbqIdXW3AJL6fQ}{1pXLTM5-SfOGx5EVsx1v8g}{127.0.0.1}{127.0.0.1:9300}{ml.machine_memory=9420918784, xpack.installed=true, ml.max_open_jobs=20, ml.enabled=true}, reason: apply cluster state (from master [master {fDPpOQe}{fDPpOQeySbqIdXW3AJL6fQ}{1pXLTM5-SfOGx5EVsx1v8g}{127.0.0.1}{127.0.0.1:9300}{ml.machine_memory=9420918784, xpack.installed=true, ml.max_open_jobs=20, ml.enabled=true} committed version [1] source [zen-disco-elected-as-master ([0] nodes joined)]])
[2018-12-03T14:13:54,463][INFO ][o.e.x.s.t.n.SecurityNetty4HttpServerTransport] [fDPpOQe] publish_address {127.0.0.1:9200}, bound_addresses {[::1]:9200}, {127.0.0.1:9200}
[2018-12-03T14:13:54,475][INFO ][o.e.n.Node               ] [fDPpOQe] started

```
安装成功。

---

### Kibana安装
#### 获取Kibana
```shell
wget https://artifacts.elastic.co/downloads/kibana/kibana-6.5.1-linux-x86_64.tar.gz
```
#### 直接解压到安装目录即完成安装
```shell
tar -xzf kibana-6.5.1-linux-x86_64.tar.gz
```

#### 修改配置
这里只修改```server.host```为```0.0.0.0```，因为elasticsearch因为与kibana部署在同一台机子上，所以```elasticsearch.url```可以不修改配置。其他配置项使用默认值。
```shell
# Kibana is served by a back end server. This setting specifies the port to use.
#server.port: 5601

# Specifies the address to which the Kibana server will bind. IP addresses and host names are both valid values.
# The default is 'localhost', which usually means remote machines will not be able to connect.
# To allow connections from remote users, set this parameter to a non-loopback address.
#server.host: "localhost"

# The URL of the Elasticsearch instance to use for all your queries.
#elasticsearch.url: "http://localhost:9200"
```

#### 运行Kibana:
```shell
sl@Li:/usr/local/kibana/bin$ ./kibana
  log   [05:36:39.933] [info][status][plugin:kibana@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:39.969] [info][status][plugin:elasticsearch@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:39.972] [info][status][plugin:xpack_main@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:39.977] [info][status][plugin:searchprofiler@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:39.980] [info][status][plugin:ml@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.024] [info][status][plugin:tilemap@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.025] [info][status][plugin:watcher@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.038] [info][status][plugin:license_management@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:40.041] [info][status][plugin:index_management@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.049] [info][status][plugin:rollup@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.157] [info][status][plugin:timelion@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:40.158] [info][status][plugin:graph@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.165] [info][status][plugin:monitoring@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:40.168] [info][status][plugin:spaces@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.175] [warning][security] Generating a random key for xpack.security.encryptionKey. To prevent sessions from being invalidated on restart, please set xpack.security.encryptionKey in kibana.yml
  log   [05:36:40.178] [warning][security] Session cookies will be transmitted over insecure connections. This is not recommended.
  log   [05:36:40.183] [info][status][plugin:security@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.205] [info][status][plugin:grokdebugger@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.208] [info][status][plugin:dashboard_mode@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:40.211] [info][status][plugin:logstash@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.225] [info][status][plugin:beats_management@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.247] [info][status][plugin:apm@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:40.436] [info][status][plugin:canvas@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:40.442] [info][status][plugin:console@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:40.443] [info][status][plugin:console_extensions@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:40.446] [info][status][plugin:notifications@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:40.477] [info][status][plugin:infra@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:40.481] [info][status][plugin:metrics@6.5.1] Status changed from uninitialized to green - Ready
  log   [05:36:40.492] [info][status][plugin:elasticsearch@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.713] [warning][reporting] Generating a random key for xpack.reporting.encryptionKey. To prevent pending reports from failing on restart, please set xpack.reporting.encryptionKey in kibana.yml
  log   [05:36:40.716] [info][status][plugin:reporting@6.5.1] Status changed from uninitialized to yellow - Waiting for Elasticsearch
  log   [05:36:40.850] [info][license][xpack] Imported license information from Elasticsearch for the [data] cluster: mode: basic | status: active
  log   [05:36:40.853] [info][status][plugin:xpack_main@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.855] [info][status][plugin:searchprofiler@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.855] [info][status][plugin:ml@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.856] [info][status][plugin:tilemap@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.857] [info][status][plugin:watcher@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.857] [info][status][plugin:index_management@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.858] [info][status][plugin:rollup@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.858] [info][status][plugin:graph@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.859] [info][status][plugin:grokdebugger@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.860] [info][status][plugin:logstash@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.860] [info][status][plugin:beats_management@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.860] [info][status][plugin:reporting@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.862] [info][kibana-monitoring][monitoring-ui] Starting monitoring stats collection
  log   [05:36:40.870] [info][status][plugin:security@6.5.1] Status changed from yellow to green - Ready
  log   [05:36:40.923] [info][license][xpack] Imported license information from Elasticsearch for the [monitoring] cluster: mode: basic | status: active
  log   [05:36:41.426] [info][listening] Server running at http://0.0.0.0:5601
  log   [05:36:41.434] [info][status][plugin:spaces@6.5.1] Status changed from yellow to green - Ready


```

#### 访问Kibana
可登录```http://localhost:5601```进行访问。


>参考文档：      
[Filebeat Reference [6.5] » Getting Started With Filebeat » Step 1: Install Filebeat](https://www.elastic.co/guide/en/beats/filebeat/6.5/filebeat-installation.html)
[Elasticsearch Reference [6.5]](https://www.elastic.co/guide/en/elasticsearch/reference/6.5/_installation.html)