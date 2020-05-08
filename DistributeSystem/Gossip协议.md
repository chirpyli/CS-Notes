Gossip算法又被称为反熵(Anti-Entropy)，熵是物理学上的概念，表示杂乱无章，而反熵就是在杂乱无章中寻找一致。这充分说明了Gossip算法的特点：在一个有界网络中，每个节点都随机的与其他节点通信，通过这些杂乱无章的通信，最终所有的节点都会达到状态一致。并且这些节点只需要知道相邻的节点就可以在最终达到一致状态。同时即使某些节点的宕机或者新加入节点，该网络最终依然能够达到一致状态。也就是说，Gossip天然具有分布式容错的优点。

### 一、Gossip基础
#### Gossip本质
Gossip是一个带冗余的容错算法，更进一步，<font color=red>Gossip是一个最终一致性算法</font>。虽然无法保证在某个时刻所有节点状态一致，但可以保证在”最终“所有节点一致，”最终“是一个现实中存在，但理论上无法证明的时间点。

因为Gossip不要求节点知道所有其他节点，因此又具有去中心化的特点，节点之间完全对等，不需要任何的中心节点。实际上Gossip可以用于众多能接受“最终一致性”的领域：失败检测、路由同步、Pub/Sub、动态负载均衡。

但Gossip的缺点也很明显，冗余通信会对网路带宽、CUP资源造成很大的负载，而这些负载又受限于通信频率，该频率又影响着算法收敛的速度。


#### Gossip算法衡量标准
研究者设计不同的Gossip算法，主要是为了提高效率及减少完成传播过程需要的通信开销。所以一般衡量Gossip算法的好坏有两个标准：效率、通信代价。其中效率是指完成Gossip传播所需要的时间。通信代价指完成Gossip传播过程所需要付出的通信开销。通信代价与效率是相互影响的，要综合考虑，这也是算法工程实现的难点。



### 二、交互模式
交互模式主要回答以下两个问题：节点在什么条件下发起信息交换？交换什么样的信息内容？参与交互的两个节点采用何种信息交换方式？

关于第一个问题，即Gossip节点的工作方式，有两种：Anti-entropy（反熵），Rumor mongering（谣言传播）。关于第二个问题，即Gossip节点间的信息交换方法，主要有3种：Push，Pull，Push&Pull。

#### Gossip节点的工作方式：


- **Anti-entropy**：每个节点周期性地随机选择其他节点，然后通过互相交换自己的所有数据来消除两者之间的差异。Anti-entropy方法非常可靠，但是每次节点两两交换自己的所有数据会带来非常大的通信负担，因此不能频繁使用。

- **Rumor mongering**：主要思想是，当一个节点有了新信息后，该节点变成活跃状态，并周期性地联系其他节点向其发行新信息，直到所有节点都知道该新信息。因为节点之间只是交换新信息，所以大大减低了通信负担。

>Anti-Entropy有完全的容错性，但有较大的网络和CPU负载；而Rumor-Mongering有较小的负载，但又必须定义“最新”的边界，难以保证完全容错，无法保证最终一致性，要引入额外的处理机制。所以，一般来讲，为了在通信代价和可靠性两者之间取得折中，需要将两种方法结合使用，一般情况采用Rumor mongering，每隔一段时间使用一次Anti-entropy，以保证信息交换的可靠性。



#### Gossip节点间的信息交换方法（通信方式）：

- **Push**：发起信息交换的节点随机选择联系节点并向其发送自己的信息，一般拥有新信息的节点才会作为发起节点。采用Push方式，才信息传播的初期，已感染节点的数目呈指数增长；当已有一半节点被感染时，每个周期感染节点的数目会迅速减少。其流程图如下所示：
![这里写图片描述](https://img-blog.csdn.net/2018070915114560?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

- **Pull**：发起信息交换的节点随机选择联系节点并从对方获取信息。一般无新信息的节点才会作为发起节点。Pull方式与Push方式相反，在信息传播的初期，感染节点的数目增长；当已有一半节点被感染时，每个周期未感染节点的数目呈乘性减少。其流程图如下所示：
![这里写图片描述](https://img-blog.csdn.net/20180709151156575?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

- **Push&Pull**：将两者结合起来，发起信息交换的节点向选择的节点发送信息，同时从对方获取数据，流程图如下所示：
![这里写图片描述](https://img-blog.csdn.net/20180709151206552?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)


>假设每个节点通信周期都能选择（感染）一个新节点，则Gossip算法退化为一个二分查找过程，每个周期构成一个平衡二叉树，收敛速度为$O(2^n )$，对应的时间开销则为$O(logn )$。这也是Gossip理论上最优的收敛速度。但在实际情况中最优收敛速度是很难达到的。详细分析参见论文[Gossip and Epidemic Protocols](http://disi.unitn.it/~montreso/ds/papers/montresor17.pdf)。

#### 协调机制
协调机制是讨论在每次2个节点通信时，如何交换数据能达到最快的一致性，也即消除两个节点的不一致性。上面所讲的push、pull等是通信方式，协调是在通信方式下的数据交换机制。协调所面临的最大问题是，因为受限于网络负载，不可能每次都把一个节点上的数据发送给另外一个节点，也即每个Gossip的消息大小都有上限。在有限的空间上有效率地交换所有的消息是协调要解决的主要问题。论文原文[Efficient Reconciliation and Flow Control for Anti-Entropy Protocols](https://www.cs.cornell.edu/home/rvr/papers/flowgossip.pdf)中描述了如下两种同步机制：

##### 精确协调（Precise Reconciliation）
精确协调希望在每次通信周期内都非常准确地消除双方的不一致性，具体表现为相互发送对方需要更新的数据，因为每个节点都在并发与多个节点通信，理论上精确协调很难做到。精确协调需要给每个数据项独立地维护自己的`version`，在每次交互是把所有的`(key,value,version)`发送到目标进行比对，从而找出双方不同之处从而更新。但因为Gossip消息存在大小限制，因此每次选择发送哪些数据就成了问题。当然可以随机选择一部分数据，也可确定性的选择数据。对确定性的选择而言，可以有最老优先（根据版本）和最新优先两种，最老优先会优先更新版本最新的数据，而最新更新正好相反，这样会造成老数据始终得不到机会更新，也即饥饿。当然，开发这也可根据业务场景构造自己的选择算法，但始终都无法避免消息量过多的问题。

##### 整体协调（Scuttlebutt Reconciliation）
整体协调与精确协调不同之处是，整体协调不是为每个数据都维护单独的版本号，而是为每个节点上的宿主数据维护统一的`version`。比如节点`P`会为`(p1,p2,...)`维护一个一致的全局`version`，相当于把所有的宿主数据看作一个整体，当与其他节点进行比较时，只需必须这些宿主数据的最高`version`，如果最高`version`相同说明这部分数据全部一致，否则再进行精确协调。

整体协调对数据的选择也有两种方法：

- 广度优先：根据整体`version`大小排序，也称为公平选择。
- 深度优先：根据包含数据多少的排序，也称为非公平选择。因为后者更有实用价值，所以原论文更鼓励后者。



>参考文档：     
[Efficient Reconciliation and Flow Control for Anti-Entropy Protocols](https://www.cs.cornell.edu/home/rvr/papers/flowgossip.pdf)       
[Gossip and Epidemic Protocols](http://disi.unitn.it/~montreso/ds/papers/montresor17.pdf)       
[Anti-Entropy Protocols, Gossips](https://www.cnblogs.com/fxjwind/archive/2013/04/02/2995679.html)      
[DISTRIBUTED ALGORITHMS IN NOSQL DATABASES](https://highlyscalable.wordpress.com/2012/09/18/distributed-algorithms-in-nosql-databases/)


