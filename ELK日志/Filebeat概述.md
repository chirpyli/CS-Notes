>本文翻译自：[Filebeat Reference [6.5] » Filebeat overview](https://www.elastic.co/guide/en/beats/filebeat/current/filebeat-overview.html#filebeat-overview)。

Filebeat是一个轻量级的日志数据收集传输工具(shipper)。Filebeat安装在用户服务器上，监测追踪用户指定的日志文件或目录，收集日志，然后将之传输到Elasticsearch或Logstash以便进行后面的索引工作。

接下来让我们看一下Filebeat是如何工作的：当启动Filebeat时，会开启一个或多个负责监测追踪指定目录的日志数据输入嗅探器(inputs)。对嗅探器定位到的每个日志数据文件，会各启动一个收集器(harvester)。收集器会逐次读取新的日志数据并将数据发送到数据处理中心(spooler)，数据处理中心会聚合收集器发送来的数据并将数据发送到你指定的输出。
![image](https://user-gold-cdn.xitu.io/2019/6/28/16b9bf463c93114b?w=940&h=735&f=png&s=81346)

更多关于输入和收集器的内容，参考[Filebeat工作原理](https://www.elastic.co/guide/en/beats/filebeat/current/how-filebeat-works.html)。

Filebeat基于libbeat框架。有关libbeat以及搭建Elasticsearch，Logstash，Kibana的更多信息请参阅[Beats手册](https://www.elastic.co/guide/en/beats/libbeat/6.5/index.html)。