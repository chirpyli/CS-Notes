在Elasticsearch中，我们常常需要按时间来建立索引，以便我们今后的使用与管理，同时我们也经常按时间去删除一些老的数据。比如只保留最近3天的数据，只需将超多3天的索引数据删除就好了。删除索引有很多种方法，你可以自己编写脚本删除索引，也可以手动删除索引```curl -XDELETE http://127.0.0.1:9200/index-name```，这里使用Curator工具删除索引。

#### Curator
[Curator](https://github.com/elastic/curator) 是elasticsearch 官方的一个索引管理工具，可以通过配置文件的方式帮助我们对指定的一批索引进行创建/删除、打开/关闭、快照/恢复等管理操作。

>安装参考文档： [Curator Reference [5.6] » Installation](https://www.elastic.co/guide/en/elasticsearch/client/curator/current/installation.html)

#### 用法
```
Usage: curator [OPTIONS] ACTION_FILE

  Curator for Elasticsearch indices.

  See http://elastic.co/guide/en/elasticsearch/client/curator/current

Options:
  --config PATH  Path to configuration file. Default: ~/.curator/curator.yml
  --dry-run      Do not perform any changes.
  --version      Show the version and exit.
  --help         Show this message and exit.
```

#### 配置
github上给出了配置示例：https://github.com/elastic/curator/tree/master/examples

这里，列出最常用的2个配置文件

curator.yml配置
```yaml
# Remember, leave a key empty if there is no value.  None will be a string,
# not a Python "NoneType"
client:
  hosts:
    - 127.0.0.1
  port: 9200
  url_prefix:
  use_ssl: False
  certificate:
  client_cert:
  client_key:
  aws_key:
  aws_secret_key:
  aws_region:
  ssl_no_validate: False
  http_auth:
  timeout: 30
  master_only: False

logging:
  loglevel: INFO
  logfile:
  logformat: default
  blacklist: ['elasticsearch', 'urllib3']
```

delete_indices.yml配置(删除3天以上的索引配置)
```yaml
# Remember, leave a key empty if there is no value.  None will be a string,
# not a Python "NoneType"
#
# Also remember that all examples have 'disable_action' set to True.  If you
# want to use this action as a template, be sure to set this to False after
# copying it.
actions:
  1:
    action: delete_indices
    description: >-
      Delete indices older than 3 days (based on index name), for test-
      prefixed indices. Ignore the error if the filter does not result in an
      actionable list of indices (ignore_empty_list) and exit cleanly.
    options:
      ignore_empty_list: True
      timeout_override:
      continue_if_exception: False
      disable_action: False
    filters:
    - filtertype: pattern
      kind: prefix
      value: test-
      exclude:
    - filtertype: age
      source: name
      direction: older
      timestring: '%Y.%m.%d'
      unit: days
      unit_count: 3
      exclude:
```

#### 使用示例
执行`
```
curator --config /etc/elasticsearch_curator/curator.yml /etc/elasticsearch_curator/delete_indices.yml
```

运行结果：
```
ubuntu@localhost:/etc/elasticsearch_curator$ curator --config /etc/elasticsearch_curator/curator.yml /etc/elasticsearch_curator/delete_indices.yml 
2018-12-20 14:22:06,854 INFO      Preparing Action ID: 1, "delete_indices"
2018-12-20 14:22:06,863 INFO      Trying Action ID: 1, "delete_indices": Delete indices older than 3 days (based on index name)
2018-12-20 14:22:06,965 INFO      Deleting selected indices: ['test-2018.12.10']
2018-12-20 14:22:06,965 INFO      ---deleting index test-2018.12.10
2018-12-20 14:22:07,262 INFO      Action ID: 1, "delete_indices" completed.
2018-12-20 14:22:07,262 INFO      Job completed.
```
可以看到删除了超过3天的索引。


#### 定时执行任务
配置好Curator后，为了完成定时删除索引的功能还要配置定时任务。
执行```crontab -e```，添加以下一行：
```
0 23 * * * curator --config /etc/elasticsearch_curator/curator.yml /etc/elasticsearch_curator/delete_indices.yml >> /home/ubuntu/elk/data/curator/curator.log 2>&1
```
每天23:00定时执行删除索引的任务。



>参考文档：     
>[Curator Reference](https://www.elastic.co/guide/en/elasticsearch/client/curator/current/index.html)   
>[Curator examples](https://github.com/elastic/curator/tree/master/examples)        
>[Curator从入门到实战](https://elasticsearch.cn/article/779)