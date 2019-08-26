>本文翻译自：[Logstash Reference [6.5] » Getting Started with Logstash » Stashing Your First Event](https://www.elastic.co/guide/en/logstash/6.5/first-event.html)。

首先，我们通过一个最简单的Logstash管道来测试Logstash是否已经安装成功。

Logstash管道有两个必需的元素：输入和输出，以及一个可选元素：过滤器。输入插件从源消耗数据，过滤器插件根据您指定的内容修改数据，输出插件将数据写入目标。
![image](https://www.elastic.co/guide/en/logstash/6.5/static/images/basic_logstash_pipeline.png)

让我们运行以下最基础管道来测试Logstash安装成功与否，如下所示：
```
cd logstash-6.5.1
bin/logstash -e 'input { stdin { } } output { stdout {} }'
```
>```bin```目录的位置因平台而异，请参阅[目录布局](https://www.elastic.co/guide/en/logstash/6.5/dir-layout.html)以查找系统上```bin\logstash```所在位置。

```-e```选项允许你可以直接在命令行中指定一个配置。通过命令行进行配置可以避免反复的编辑修改配置文件从而快速的测试你的配置。上面的例子中，以标准输入```stdin```作为输入，结构化的数据直接从输入输出到标准输出```stdout```。

启动Logstash后，看到Logstash打印出信息"Pipeline main started"后，在命令行中输入```hello world```:
```
hello world
2013-11-21T01:22:14.405+0000 0.0.0.0 hello world
```
Logstash为消息添加了时戳和IP信息。退出Logstash可在命令行中输入```CTRL-D```。

恭喜你！你已经创建并运行了一个简单的Logstash管道。下一步，你将学习如何创建更加实用的管道。

