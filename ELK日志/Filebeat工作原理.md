>本文翻译自：[Filebeat Reference [6.5] » How Filebeat works](https://www.elastic.co/guide/en/beats/filebeat/current/how-filebeat-works.html#input)。

在本章节，你将会学到Filebeat的关键组成部分以及它们是如何组织工作的。理解这些概念将有助于你对自己的Filebeat用例进行更优的配置。

Filebeat主要由2部分构成：输入嗅探(inputs)和收集器(harvester)。这些组件组合在一起，追踪数据文件并且将至发送到你指定的目的地。
>原文中inputs这里译为输入嗅探，harvester译为收集器仅是个人理解，可能不太准确。

### 什么是收集器？
收集器负责读取数据文件的内容。收集器会逐行的读取文件并将读到的数据发送到输出。每个文件都会启动一个收集器。文件的打开和关闭由收集器完成，这意味着在收集器运行期间，对应的文件描述符将会保持在打开状态。如果正在被采集的文件被移动或者重命名，Filebeat会继续读取文件。这会产生一定的副作用，会一直占用磁盘空间直到收集器关闭。默认情况下，Filebeat保持文件打开状态直到[close_inactive](https://www.elastic.co/guide/en/beats/filebeat/current/filebeat-input-log.html#filebeat-input-log-close-inactive)状态到达。

关闭收集器后会产生如下结果：
- 关闭文件句柄，如果收集器正在读取文件时文件被删除，则释放占用的资源。
- 直到[scan_frequency](https://www.elastic.co/guide/en/beats/filebeat/current/filebeat-input-log.html#filebeat-input-log-scan-frequency)状态到达，收集器才会再次读取文件。
- 如果在收集器关闭后移动或者删除文件，则不会继续读取文件。

对什么时候关闭收集器，可对[close_*](https://www.elastic.co/guide/en/beats/filebeat/current/filebeat-input-log.html#filebeat-input-log-close-options)配置项进行配置。

### 什么是输入嗅探？
输入嗅探管理所有的收集器并查找所有的数据源。如果输入类型是```log```，输入嗅探会查找磁盘上所有与指定路径通配符匹配的文件并对探测到的每个文件启动一个收集器。每个输入嗅探运行在自己的```Go routine```中。

下面的Filebeat配置示例指定了收集所有匹配指定通配符的所有日志文件数据：
```yaml
filebeat.inputs:
- type: log
  paths:
    - /var/log/*.log
    - /var/path2/*.log
```
Filebeat当前支持[多种输入类型](https://www.elastic.co/guide/en/beats/filebeat/current/configuration-filebeat-options.html#filebeat-input-types)。每种输入类型都可以被多次设置。对```log```型输入，输入嗅探会决定是否启动收集器，收集器是否正在运行，或者文件是否应该被忽略(查看[ignore_older](https://www.elastic.co/guide/en/beats/filebeat/current/filebeat-input-log.html#filebeat-input-log-ignore-older))。当收集器再次启动文件大小发生变化时只会收集自上次关闭后新增的数据行。

### Filebeat如何保持文件状态？
Filebeat会持续保留每个文件的状态并且定期将状态刷新到磁盘上的注册表文件中。这个文件状态用于记录收集器读取文件数据的最新偏移量并保证所有的日志行都被读取发送出去。如果输出目的端（例如：Elasticsearch或者Logstash）不可达，Filebeat会跟踪最后发送的数据行，一旦输出可用会继续从最后停留的数据行开始读取文件。Filebeat正在运行时，每个输入嗅探的文件状态信息也会被保存在内存中。当Filebeat重启时，会从注册表文件中重建状态信息，收集器会根据重建的状态信息继续从最后已知的位置开始读取数据。

对每个输入嗅探，Filebeat会记录它探测到的每个文件的状态。因为文件有可能被移动或者重命名，仅靠文件名或者路径就不足以标识一个文件。对此，Filebeat为每个输入嗅探到的文件保存一个唯一的标识符以检测文件之前是否被收集过。
> 补充：在Linux上，这个唯一标识符是inode，可参考[理解Linux文件系统之inode](https://www.cnblogs.com/xiexj/p/7214502.html)。

如果你的用例中每天会创建大量新的文件，你会发现注册表文件将会增长的很快。可查看[注册表文件太大？](https://www.elastic.co/guide/en/beats/filebeat/current/faq.html#reduce-registry-size)获取可解决此文件的配置选项的详细信息。

### Filebeat如何确保至少一次交付？
Filebeat能够确保日志事件至少一次交付到指定配置输出并且没有数据丢失。Filebeat之所以能够确保不丢失是因为它在注册表文件中存储了每个日志事件投递的状态。

在输出中断无法确认所有事件被收到的情况下，Filebeat将会持续发送事件直到输出确认收到事件。

如果Filebeat在发送事件时被关闭，它不会在关闭前等待输出确认所有事件到达，即它会立即关闭。当Filebeat重启时，在关闭退出前已发送到输出端但是还没有被确认的事件将会被再次发送。这确保了每个事件至少会被交付一次，但是这会造成一部分事件的重复发送。你可以通过配置[shutdown_timeout](https://www.elastic.co/guide/en/beats/filebeat/current/configuration-general-options.html#shutdown-timeout)项设置Filebeat在关闭退出前等待一段时间。

>在涉及日志滚动和删除旧文件时，Filebeat的至少一次交付有一个限制。如果日志文件写入磁盘过快并在Filebeat处理前发生了日志滚动，或文件在输出不可用时被删除，日志数据可能会丢失。在Linux上，Filebeat也可能因inode重用而跳过行。 有关inode重用问题的更多详细信息，请参阅[常见问题解答](https://www.elastic.co/guide/en/beats/filebeat/current/faq.html#faq)。
