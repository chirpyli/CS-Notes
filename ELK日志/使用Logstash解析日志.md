>本文翻译自：[Logstash Reference [6.5] » Getting Started with Logstash » Parsing Logs with Logstash](https://www.elastic.co/guide/en/logstash/6.5/advanced-pipeline.html)

在[存储你的第一个事件](https://www.elastic.co/guide/en/logstash/6.5/first-event.html)一节中，我们创建了一个简单的Logstash管道示例来测试你已安装运行的Logstash。在生产环境中，Logstash管道会非常复杂：一般包含一个或多个输入，过滤器，输出插件。

在本章节，你将创建一个Logstash管道：Filebeat采集Apache web日志数据作为输入，解析输入的日志，创建特定的字段，然后将解析后的数据写入Elasticsearch集群中。您将在配置文件中进行Logstash管道的配置而不是在命令行中配置。

接下来，点击[这里](https://download.elastic.co/demos/logstash/gettingstarted/logstash-tutorial.log.gz)下载示例数据，解压文件。

### 配置Filebeat发送日志到Logstash
在创建Logstash管道之前，你需要配置Filebeat以将日志发送到Logstash中。Filebeat是轻量级，资源占用较低的日志收集工具，从各服务器上收集日志并将这些日志发送到Logstash实例中处理。Filebeat专为可靠性和低延时而设计。Filebeat只占用很少的主机资源，从而Beat输入插件有效的降低了Logstash实例的资源需求。
>在典型应用中，Filebeat与Logstash是部署在不同的主机上的。这里出于教程简便的目的，将它们部署在同一台主机上。

Logstash默认安装已含有[Beat input](http://www.elastic.co/guide/en/logstash/6.5/plugins-inputs-beats.html)插件。```Beat```输入插件使Logstash可以从```Elastic Beat```框架接收事件，这意味者任何编写为与```Beat```框架一起工作的Beat(如Packetbeat和Metricbeat)都可以将事件数据发送到Logstash。

要在数据源主机上安装Filebeat，可以在Filebeat[产品页](https://www.elastic.co/downloads/beats/filebeat)中下载对应的安装包。也可以参阅[Filebeat入门](https://www.elastic.co/guide/en/beats/filebeat/6.5/filebeat-getting-started.html)获取其他安装说明。

安装好Filebeat后，需要对其进行配置。在Filebeat安装目录中找到并打开```filebeat.yml```文件，替换如下行中的配置项。确保```paths```项为先前下载的Apache示例日志文件（logstash-tutorial.log）路径。
```yaml
filebeat.prospectors:
- type: log
  paths:
    - /path/to/file/logstash-tutorial.log  # 须填写绝对路径
output.logstash:
  hosts: ["localhost:5044"]
```
保存配置。为了简化配置，无需配置生成环境中常用TLS/SSL配置。

在数据源主机上，输入以下命令运行Filebeat：
```shell
sudo ./filebeat -e -c filebeat.yml -d "publish"
```
>如果以root身份运行Filebeat，则需要更改配置文件的所有权（请参阅[Config File Ownership and Permissions](https://www.elastic.co/guide/en/beats/libbeat/6.5/config-file-permissions.html)）。

Filebeat将尝试在端口5044上建立连接。在Logstash启动一个激活状态的Beats插件前，该端口上没有任何响应，所以在输入插件处于未激活状态前你看到的在5044端口上建立连接失败的信息都是正常的。

### 为Logstash配置Filebeat输入
接下来，你要创建一个以Beats输入插件作为输入接收来自Beats的事件的Logstash管道配置。

以下的一段内容为一个管道配置的框架：
```yaml
# The # character at the beginning of a line indicates a comment. Use
# comments to describe your configuration.
input {
}
# The filter part of this file is commented out to indicate that it is
# optional.
# filter {
#
# }
output {
}
```
上面这个框架是不起任何作用的，因为输入和输出部分都没有有效的定义。

好，继续，在Logstash目录下创建一个```first-pipeline.conf```的文件并复制粘贴上面的管道配置框架内容。

下一步，通过将以下行添加到```first-pipeline.conf```文件的输入部分，将Logstash实例配置为使用Beats作为输入插件：
```yaml
    beats {
        port => "5044"
    }
```

后面你会配置Logstash输出到Elasticsearch中。目前，你可以先在输出部分添加下面几行配置使Logstash运行时输出到标准输出```stdout```：
```yaml
    stdout { codec => rubydebug }
```
上面的步骤完成后，```first-pipeline.conf```文件配置应该如下：
```yaml
input {
    beats {
        port => "5044"
    }
}
# The filter part of this file is commented out to indicate that it is
# optional.
# filter {
#
# }
output {
    stdout { codec => rubydebug }
}

```
可以运行以下命令验证上面的配置：
```
bin/logstash -f first-pipeline.conf --config.test_and_exit
```

```--config.test_and_exit```选项会检查配置文件并报告错误。

如果配置文件通过了验证测试，运行以下命令启动Logstash：
```
bin/logstash -f first-pipeline.conf --config.reload.automatic
```
```--config.reload.automatic```项会定期自动重载配置，可以不停止重启Logstash就可以修改配置。

在Logstash启动时，你会看到一或多行的关于Logstash忽略```pipelines.yml```文件的警告。你可以放心的忽略这条警告。```pipelines.yml```配置文件作用是一个Logstash实例中运行多个管道。这里的示例中，只运行了一个管道。

如果你的管道正常运行，会在控制台输出大量的事件如下所示：
```yaml
{
    "@timestamp" => 2017-11-09T01:44:20.071Z,
        "offset" => 325,
      "@version" => "1",
          "beat" => {
            "name" => "My-MacBook-Pro.local",
        "hostname" => "My-MacBook-Pro.local",
         "version" => "6.0.0"
    },
          "host" => "My-MacBook-Pro.local",
    "prospector" => {
        "type" => "log"
    },
        "source" => "/path/to/file/logstash-tutorial.log",
       "message" => "83.149.9.216 - - [04/Jan/2015:05:13:42 +0000] \"GET /presentations/logstash-monitorama-2013/images/kibana-search.png HTTP/1.1\" 200 203023 \"http://semicomplete.com/presentations/logstash-monitorama-2013/\" \"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.77 Safari/537.36\"",
          "tags" => [
        [0] "beats_input_codec_plain_applied"
    ]
}
```
### 通过Grok过滤插件解析Web日志
现在你有了一个可以从Filebeat读取日志行的工作管道，但是你会发现日志的格式不是十分理想。由此，您希望解析消息以创建特定字段。为此，你需要使用```grok```过滤插件。

```grok```过滤插件是Logstash默认可用的几个插件之一。有关如何管理Logstash插件的详细信息，可参阅[参考文档](https://www.elastic.co/guide/en/logstash/6.5/working-with-plugins.html)中插件管理一节。

```grok```过滤插件使您能够将非结构化日志数据解析为结构化易查询的形式。

```grok```过滤插件是在输入的日志数据中查找对应模式，因此需要您根据你自己的用例需求去配置插件如何识别对应的模式。 Web服务器日志示例中的代表行如下所示：
```shell
83.149.9.216 - - [04/Jan/2015:05:13:42 +0000] "GET /presentations/logstash-monitorama-2013/images/kibana-search.png
HTTP/1.1" 200 203023 "http://semicomplete.com/presentations/logstash-monitorama-2013/" "Mozilla/5.0 (Macintosh; Intel
Mac OS X 10_9_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.77 Safari/537.36"
```
在日志行启始位置的IP地址是非常好识别的，在括号中的时间戳也同样好识别。要解析这些数据，可以使用```%{COMBINEDAPACHELOG}```模式，用下表的形式结构化Apache日志行：

|Information|Field Name|
|--|--|
IP Address|clientip
User ID|ident
User Authentication|auth
timestamp|timestamp
HTTP Verb|verb
Request body|request
HTTP Version|httpversion
HTTP Status Code | response
Bytes served|bytes
Referrer URL |referrer
User agent|agent

>如果你在构建```grok```模式时需要帮助，尝试使用[Grok调试器](https://www.elastic.co/guide/en/kibana/6.5/xpack-grokdebugger.html)。Grok调试器是的X-Pack的一个功能，可以免费使用。

编辑```first-pipeline.conf```配置文件并将```filter```部分的内容替换为以下内容：
```yaml
filter {
    grok {
        match => { "message" => "%{COMBINEDAPACHELOG}"}
    }
}
```
上面工作完成后，```first-pipeline.conf```配置文件的内容如下：
```yaml
input {
    beats {
        port => "5044"
    }
}
filter {
    grok {
        match => { "message" => "%{COMBINEDAPACHELOG}"}
    }
}
output {
    stdout { codec => rubydebug }
}
```
保存更改项。因为之前在配置中设置了配置自动重载，当你再次更改配置时不必重新启动Logstash使配置生效。但是，您需要强制Filebeat从头开始读取日志文件。 为此，请转到运行Filebeat的终端窗口，然后按```Ctrl + C```关闭Filebeat。 然后删除Filebeat注册表文件```registry```。 例如，运行：
```
sudo rm data/registry
```
由于Filebeat在注册表文件中存储了每个文件被读取后的状态，删除注册表文件将强制Filebea从头开始读取文件。下一步，用下面的命令重启Filebeat:
```
sudo ./filebeat -e -c filebeat.yml -d "publish"
```
如果Filebeat在开始处理事件之前需要等待Logstash重新加载配置文件，则可能会有点延时。在Logstash应用了```grok```模式后，事件将具有以下JSON表示：
```json
{
        "request" => "/presentations/logstash-monitorama-2013/images/kibana-search.png",
          "agent" => "\"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.77 Safari/537.36\"",
         "offset" => 325,
           "auth" => "-",
          "ident" => "-",
           "verb" => "GET",
     "prospector" => {
        "type" => "log"
    },
         "source" => "/path/to/file/logstash-tutorial.log",
        "message" => "83.149.9.216 - - [04/Jan/2015:05:13:42 +0000] \"GET /presentations/logstash-monitorama-2013/images/kibana-search.png HTTP/1.1\" 200 203023 \"http://semicomplete.com/presentations/logstash-monitorama-2013/\" \"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.77 Safari/537.36\"",
           "tags" => [
        [0] "beats_input_codec_plain_applied"
    ],
       "referrer" => "\"http://semicomplete.com/presentations/logstash-monitorama-2013/\"",
     "@timestamp" => 2017-11-09T02:51:12.416Z,
       "response" => "200",
          "bytes" => "203023",
       "clientip" => "83.149.9.216",
       "@version" => "1",
           "beat" => {
            "name" => "My-MacBook-Pro.local",
        "hostname" => "My-MacBook-Pro.local",
         "version" => "6.0.0"
    },
           "host" => "My-MacBook-Pro.local",
    "httpversion" => "1.1",
      "timestamp" => "04/Jan/2015:05:13:42 +0000"
}
```
请注意，事件包含原始消息，但日志消息也会被解析为各个特定字段。

### 通过Geoip过滤插件增强你的数据
除了解析日志数据以获得更好的搜索之外，过滤插件还可以从现有数据中获取补充信息。例如，```geoip```插件查找IP地址，从地址中获取地理位置信息，并将该位置信息添加到日志中。

使用```geoip```插件配置你的Logstash实例，在```first-pipeline.conf```配置文件的filter部分添加如下行：
```
    geoip {
        source => "clientip"
    }
```
```geoip```插件配置要求你指定包含要查找的IP地址信息的源字段的名称。在此示例中，clientip字段包含IP地址信息。

由于过滤器是按序处理的，在配置文件中请确保```geoip```部分在```grok```部分之后，并且都在```filter```内部。

当你完成这步后，```first-pipeline.conf```内容应该如下：
```yaml
input {
    beats {
        port => "5044"
    }
}
 filter {
    grok {
        match => { "message" => "%{COMBINEDAPACHELOG}"}
    }
    geoip {
        source => "clientip"
    }
}
output {
    stdout { codec => rubydebug }
}
```
保存更改。如之前所做的那样，为了强制Filebeat从头开始读取日志文件，关闭Filebeat(```Ctrl+C```)，删除register注册表文件，运行以下命令重启Filebeat:
```
sudo ./filebeat -e -c filebeat.yml -d "publish"
```
请注意，现在的事件已包含了地理位置信息了：
```json
{
        "request" => "/presentations/logstash-monitorama-2013/images/kibana-search.png",
          "agent" => "\"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.77 Safari/537.36\"",
          "geoip" => {
              "timezone" => "Europe/Moscow",
                    "ip" => "83.149.9.216",
              "latitude" => 55.7485,
        "continent_code" => "EU",
             "city_name" => "Moscow",
          "country_name" => "Russia",
         "country_code2" => "RU",
         "country_code3" => "RU",
           "region_name" => "Moscow",
              "location" => {
            "lon" => 37.6184,
            "lat" => 55.7485
        },
           "postal_code" => "101194",
           "region_code" => "MOW",
             "longitude" => 37.6184
    },
    ...
```

### 创建Elasticsearch索引
到现在，你已经将web日志数据解析为各个字段，可以将数据输出到Elasticsearch中了。
>你可以在自己的主机上运行Elasticsearch，也可以在Elastic Cloud上使用我们托管的Elasticsearch Service。 AWS和GCP均提供Elasticsearch服务。 [免费试用Elasticsearch服务](https://www.elastic.co/cloud/elasticsearch-service/signup)。

Logstash管道可以将数据索引到Elasticsearch集群中。编辑```first-pipeline.conf```配置文件，将输出部分替换为如下内容：
```yaml
output {
    elasticsearch {
        hosts => [ "localhost:9200" ]
    }
}
```
在此配置中，Logstash使用http协议与Elasticsearch建立连接。上面的示例假定Logstash与Elasticsearch运行在同一台主机中。你可以通过```hosts```配置项进行类似```hosts => [ "es-machine:9092" ]```的配置去连接一个远端运行的Elasticsearch实例。

到这里，```first-pipeline.conf```配置文件中input、filter、output都有了适当的配置，配置内容如下：
```yaml
input {
    beats {
        port => "5044"
    }
}
 filter {
    grok {
        match => { "message" => "%{COMBINEDAPACHELOG}"}
    }
    geoip {
        source => "clientip"
    }
}
output {
    elasticsearch {
        hosts => [ "localhost:9200" ]
    }
}
```
保存更改。如之前所做的那样，为了强制Filebeat从头开始读取日志文件，关闭Filebeat(```Ctrl+C```)，删除register注册表文件，运行以下命令重启Filebeat:
```
sudo ./filebeat -e -c filebeat.yml -d "publish"
```

### 管道测试
到现在，Logstash管道已经配置为输出数据到Elasticsearch集群中，所有你可以在Elasticsearch中查询到相关的数据。

基于```grok```过滤插件创建的字段，尝试对Elasticsearch进行查询测试。 将```$ DATE```替换为当前日期，格式为```YYYY.MM.DD```：
```
curl -XGET 'localhost:9200/logstash-$DATE/_search?pretty&q=response=200'
```
>索引名中的日期是基于UTC的，并不是Logstash运行所在的时区。如果查询时返回```index_not_found_exception```，请确保```logstash-$DATA```是正确的索引名。查看当前可用索引列表，可用此进行查询：```curl 'localhost:9200/_cat/indices?v'```。

你会得到多个返回的结果。类似：
```json
{
  "took": 50,
  "timed_out": false,
  "_shards": {
    "total": 5,
    "successful": 5,
    "skipped": 0,
    "failed": 0
  },
  "hits": {
    "total": 98,
    "max_score": 2.793642,
    "hits": [
      {
        "_index": "logstash-2017.11.09",
        "_type": "doc",
        "_id": "3IzDnl8BW52sR0fx5wdV",
        "_score": 2.793642,
        "_source": {
          "request": "/presentations/logstash-monitorama-2013/images/frontend-response-codes.png",
          "agent": """"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.77 Safari/537.36"""",
          "geoip": {
            "timezone": "Europe/Moscow",
            "ip": "83.149.9.216",
            "latitude": 55.7485,
            "continent_code": "EU",
            "city_name": "Moscow",
            "country_name": "Russia",
            "country_code2": "RU",
            "country_code3": "RU",
            "region_name": "Moscow",
            "location": {
              "lon": 37.6184,
              "lat": 55.7485
            },
            "postal_code": "101194",
            "region_code": "MOW",
            "longitude": 37.6184
          },
          "offset": 2932,
          "auth": "-",
          "ident": "-",
          "verb": "GET",
          "prospector": {
            "type": "log"
          },
          "source": "/path/to/file/logstash-tutorial.log",
          "message": """83.149.9.216 - - [04/Jan/2015:05:13:45 +0000] "GET /presentations/logstash-monitorama-2013/images/frontend-response-codes.png HTTP/1.1" 200 52878 "http://semicomplete.com/presentations/logstash-monitorama-2013/" "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1700.77 Safari/537.36"""",
          "tags": [
            "beats_input_codec_plain_applied"
          ],
          "referrer": """"http://semicomplete.com/presentations/logstash-monitorama-2013/"""",
          "@timestamp": "2017-11-09T03:11:35.304Z",
          "response": "200",
          "bytes": "52878",
          "clientip": "83.149.9.216",
          "@version": "1",
          "beat": {
            "name": "My-MacBook-Pro.local",
            "hostname": "My-MacBook-Pro.local",
            "version": "6.0.0"
          },
          "host": "My-MacBook-Pro.local",
          "httpversion": "1.1",
          "timestamp": "04/Jan/2015:05:13:45 +0000"
        }
      },
    ...
```
用从IP地址中导出的地理信息尝试进行另一个查询。 将```$ DATE```替换为当前日期，格式为```YYYY.MM.DD```：
```
curl -XGET 'localhost:9200/logstash-$DATE/_search?pretty&q=geoip.city_name=Buffalo'
```
数据中有少量日志条目来自Buffalo，查询结果如下：
```json
{
  "took": 9,
  "timed_out": false,
  "_shards": {
    "total": 5,
    "successful": 5,
    "skipped": 0,
    "failed": 0
  },
  "hits": {
    "total": 2,
    "max_score": 2.6390574,
    "hits": [
      {
        "_index": "logstash-2017.11.09",
        "_type": "doc",
        "_id": "L4zDnl8BW52sR0fx5whY",
        "_score": 2.6390574,
        "_source": {
          "request": "/blog/geekery/disabling-battery-in-ubuntu-vms.html?utm_source=feedburner&utm_medium=feed&utm_campaign=Feed%3A+semicomplete%2Fmain+%28semicomplete.com+-+Jordan+Sissel%29",
          "agent": """"Tiny Tiny RSS/1.11 (http://tt-rss.org/)"""",
          "geoip": {
            "timezone": "America/New_York",
            "ip": "198.46.149.143",
            "latitude": 42.8864,
            "continent_code": "NA",
            "city_name": "Buffalo",
            "country_name": "United States",
            "country_code2": "US",
            "dma_code": 514,
            "country_code3": "US",
            "region_name": "New York",
            "location": {
              "lon": -78.8781,
              "lat": 42.8864
            },
            "postal_code": "14202",
            "region_code": "NY",
            "longitude": -78.8781
          },
          "offset": 22795,
          "auth": "-",
          "ident": "-",
          "verb": "GET",
          "prospector": {
            "type": "log"
          },
          "source": "/path/to/file/logstash-tutorial.log",
          "message": """198.46.149.143 - - [04/Jan/2015:05:29:13 +0000] "GET /blog/geekery/disabling-battery-in-ubuntu-vms.html?utm_source=feedburner&utm_medium=feed&utm_campaign=Feed%3A+semicomplete%2Fmain+%28semicomplete.com+-+Jordan+Sissel%29 HTTP/1.1" 200 9316 "-" "Tiny Tiny RSS/1.11 (http://tt-rss.org/)"""",
          "tags": [
            "beats_input_codec_plain_applied"
          ],
          "referrer": """"-"""",
          "@timestamp": "2017-11-09T03:11:35.321Z",
          "response": "200",
          "bytes": "9316",
          "clientip": "198.46.149.143",
          "@version": "1",
          "beat": {
            "name": "My-MacBook-Pro.local",
            "hostname": "My-MacBook-Pro.local",
            "version": "6.0.0"
          },
          "host": "My-MacBook-Pro.local",
          "httpversion": "1.1",
          "timestamp": "04/Jan/2015:05:29:13 +0000"
        }
      },
     ...
```
如果你正在使用Kibana可视化日志数据，你也可以在Kibana中查看Filebeat读取的数据：

![image](https://www.elastic.co/guide/en/logstash/6.5/static/images/kibana-filebeat-data.png)

可阅读[Filebeat使用文档](https://www.elastic.co/guide/en/beats/filebeat/6.5/filebeat-getting-started.html)获取有关Kibana加载Filebeat索引的信息。

你已经成功的创建了一个管道，使用Filebeat读取Apache web日志信息作为输入，解析日志创建指定字段，并将解析后的日志数据输出到Elasticsearch集群中。下一步，你将学习如何创建一个使用多个输入输出插件的管道。
