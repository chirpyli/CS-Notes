### 关于消息中间件
消息传递是分布式系统必然要面对的一个问题，消息中间件解决的就是分布式系统之间消息传递的问题。消息中间件的应用场景大致如下：
- 业务解耦：交易系统不需要知道短信通知服务的存在，只需要发布消息
- 削峰填谷：比如上游系统的吞吐能力高于下游系统，在流量洪峰时可能会冲垮下游系统，消息中间件可以在峰值时堆积消息，而在峰值过去后下游系统慢慢消费消息解决流量洪峰的问题
- 事件驱动：系统与系统之间可以通过消息传递的形式驱动业务，以流式的模型处理

当前主流的消息中间件：RabbitMQ、ActiveMQ和ZeroMQ。

消息中间件可以类比为现实世界中的物流系统，商家将商品发布到物流系统，物流系统将商品送到客户手中。商家和客户可以不用关心东西是怎么运输的，你只需将地址告诉它，剩下的就不用你担心了。

### 关于AMQP
AMQP（Advanced Message Queuing Protocol） 是一个提供统一消息服务的应用层标准协议。它的主要特征是面向消息、队列、路由（包括点对点和发布/订阅），且可靠性强、安全。AMQP协议是一种二进制协议，为客户端应用与消息中间件之间提供异步、安全、高效的交互。 基于此协议的客户端与消息中间件可传递消息，并不受客户端和中间件不同产品、不同开发语言等条件的限制。下面是从网上截取的图片，能更直观的去理解AMQP协议：
![这里写图片描述](https://img-blog.csdn.net/20180426193841252?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)       
可以看到，基本流程就是发布者发布消息--->经过Exchange判断消息路由到那个队列--->具体队列--->订阅者从队列中取走消息。后面会详解。

### rabbitmq基础概念
**RabbitMQ** 是一个由erlang开发的基于AMQP（Advanced Message Queue）协议的开源实现。用于在分布式系统中存储转发消息，在易用性、扩展性、高可用性等方面都非常的优秀。是当前最主流的消息中间件之一。

  **Brocker：** 消息队列服务器实体。主要由两个组件Exchange和Queue构成。而Producer和Consumer是客户端，由用户可根据不同的需要可以使用不同的语言实现。
  ![这里写图片描述](https://imgconvert.csdnimg.cn/aHR0cHM6Ly9ibG9nLnRoYW5rYmFiZS5jb20vaW1ncy8zMDY5NzYtMjAxNjA3MjAxMDQwMzcwNDQtMTA3MTA2MzgwNS5wbmc?x-oss-process=image/format,png)

**Producer：** 消息生产者，就是发布消息的客户端程序。

**Consumer：** 消息消费者，就是订阅消息的客户端程序。  

**Vhost：** 虚拟主机，一个broker里可以开设多个vhost，用作不用用户的权限分离。每个virtual host本质上都是一个RabbitMQ Server，拥有它自己的queue，exchagne，和bindings rule等等（这样方便多个不同应用程序或进程使用同一消息队列服务器）。
 ![这里写图片描述](https://img-blog.csdn.net/20180426193726854?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)      
  **Connection：** 连接，就是一个TCP的连接。Producer和Consumer客户端通过TCP连接到rabbitmq服务器，一般客户端程序的起始处就是建立这个TCP连接。

 **Channels：** 消息通道，在客户端的每个连接里，可建立多个channel(虚拟连接)，每个channel代表一个会话任务。它建立在上述的TCP连接中。数据流动都是在Channel中进行的。客户端程序一般情况是起始建立TCP连接，之后建立Channel。

>那么，为什么使用Channel，而不是直接使用TCP连接？
>对于OS来说，建立和关闭TCP连接是有代价的，频繁的建立关闭TCP连接对于系统的性能有很大的影响，而且TCP的连接数也有限制，这也限制了系统处理高并发的能力。但是，在TCP连接中建立Channel是没有上述代价的。对于Producer或者Consumer来说，可以并发的使用多个Channel进行Publish或者Receive。
    

**Exchange：** 消息交换机，指定消息按什么规则，路由到哪个队列。通常分为四种：

 -   fanout：该类型路由规则非常简单，会把所有发送到该Exchange的消息路由到所有与它绑定的Queue中，相当于广播功能
 -   direct：该类型路由规则会将消息路由到binding key与routing key完全匹配的Queue中
 -   topic：与direct类型相似，只是规则没有那么严格，可以模糊匹配和多条件匹配
 -   headers：该类型不依赖于routing key与binding key的匹配规则来路由消息，而是根据发送的消息内容中的headers属性进行匹配
![这里写图片描述](https://img-blog.csdn.net/20180426193628857?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
  
**Queue：** 消息队列，每个消息都会被投入到一个或者多个队列里。
 
**Binding：** 绑定，它的作用是把exchange和queue按照路由规则binding起来。

**Routing Key：** 路由关键字，exchange根据这个关键字进行消息投递。

### rabbitmq客户端基本流程
![这里写图片描述](https://img-blog.csdn.net/20140220173559828)

**消息接收:**

   -   客户端连接到消息队列服务器，打开一个channel。
   -   客户端声明一个exchange，并设置相关属性。
   -  客户端声明一个queue，并设置相关属性。
   -  客户端使用routing key，在exchange和queue之间建立好绑定关系。
     
**消息发布:**

  - 客户端投递消息到exchange。
  - exchange接收到消息后，就根据消息的key和已经设置的binding，进行消息路由，将消息投递到一个或多个队列里。（这一步其实是rabbitmq服务端实现的过程，不在客户端中）

### rabbitmq若干细节


**谁创建队列queue**

Consumer和Procuder都可以通过 queue.declare 创建queue。对于某个Channel来说，Consumer不能declare一个queue，却订阅其他的queue。当然也可以创建私有的queue。这样只有app本身才可以使用这个queue。queue也可以自动删除，被标为auto-delete的queue在最后一个Consumer unsubscribe后就会被自动删除。那么如果是创建一个已经存在的queue呢？那么不会有任何的影响。需要注意的是没有任何的影响，也就是说第二次创建如果参数和第一次不一样，那么该操作虽然成功，但是queue的属性并不会被修改。

那么谁该负责创建这个queue呢？是Consumer，还是Producer？如果queue不存在，当然Consumer不会得到任何的Message。但是如果queue不存在，那么Producer Publish的Message会被丢弃。所以，为了数据不丢失，一般的做法是Consumer和Producer都try to create the queue！这样能够确保这个接口两者都不会出问题。

**使用ack确认Message的正确传递**

   默认情况下，如果Message 已经被某个Consumer正确的接收到了，那么该Message就会被从queue中移除。当然也可以让同一个Message发送到很多的Consumer。如果一个queue没被任何的Consumer Subscribe（订阅），那么，如果这个queue有数据到达，那么这个数据会被cache，不会被丢弃。当有Consumer时，这个数据会被立即发送到这个Consumer，这个数据被Consumer正确收到时，这个数据就被从queue中删除。

那么什么是正确收到呢？通过ack。每个Message都要被acknowledged（确认，ack）。我们可以显示的在程序中去ack，也可以自动的ack。如果有数据没有被ack，那么RabbitMQ Server会把这个信息发送到下一个Consumer。如果这个app有bug，忘记了ack，那么RabbitMQ Server不会再发送数据给它，因为Server认为这个Consumer处理能力有限。

ack的机制还可以起到限流的作用：在Consumer处理完成数据后发送ack，甚至在额外的延时后发送ack，将有效的balance Consumer的load。当然对于实际的例子，比如我们可能会对某些数据进行merge，比如merge 4s内的数据，然后sleep 4s后再获取数据。特别是在监听系统的state，我们不希望所有的state实时的传递上去，而是希望有一定的延时。这样可以减少某些IO，而且终端用户也不会感觉到。


最后，使用示例可以参考[官网](http://www.rabbitmq.com/tutorials/tutorial-one-python.html)给的python代码示例。
