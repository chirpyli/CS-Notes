日志能方便地诊断程序原因、统计程序运行数据，是大型软件系统必不可少的组件之一。这里介绍boost日志库。
>boost log ：功能丰富，可扩展性强，文档全面。现代化的 C++ 设计，泛型编程思维，值得研究。缺点是上手慢，体积大。一旦掌握之后，用起来将会得心应手，游刃有余。如果需要丰富的功能（如输出到网络 /XML 、统计功能、更多的个性化设置），建议选择 boost log 。
	

	
### boost log 配置和编译
	
boost log支持以下配置宏，只列出一些常用的，如下所示：
- BOOST_LOG_DYN_LINK —— If defined in user code, the library will assume the binary is built as a dynamically loaded library ("dll" or "so"). Otherwise it is assumed that the library is built in static mode. This macro must be either defined or not defined for all translation units of user application that uses logging. This macro can help with auto-linking on platforms that support it. 
- BOOST_ALL_DYN_LINK —— Same as BOOST_LOG_DYN_LINK but also affects other Boost libraries the same way. 
- BOOST_USE_WINAPI_VERSION —— Affects compilation of both the library and user's code. This macro is Windows-specific. Selects the target Windows version for various Boost libraries, including Boost.Log. Code compiled for a particular Windows version will likely fail to run on the older Windows versions, but may improve performance because of using newer OS features. The macro is expected to have an integer value equivalent to _WIN32_WINNT. 
- BOOST_LOG_NO_THREADS —— If defined, disables multithreading support. Affects the compilation of both the library and users' code. The macro is automatically defined if no threading support is detected. 

有一点要注意：如果你的程序工程中由多个模块构成（例如，由一个.exe和多个.dll构成），当你使用boost log
库时必须built as a shared object。如果只是单个模块（例如：单个.exe或单个.dll）则可以build the library as a static library. 

Boost官方文档原文如下：`One thing should be noted, though. If your application consists of more than one module (e.g. an exe and one or several dll's) that use Boost.Log, the library must be built as a shared object. If you have a single executable or a single module that works with Boost.Log, you may build the library as a static library. `


### 常用命名空间定义
为了简化代码，以下代码非常常用：
```c++
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;
```

### boost log涉及的重点概念或术语定义解释

- **日志记录**：一个独立的消息包，这个消息包还不是实际写到日志里的消息，它只是一个候选的消息。
- **属性**   ： 日志记录中的一个消息片。
- **属性值**：那就是上面所说的属性的值了，可以是各种数据类型。
- **日志槽**（LOG SINK)：日志写向的目标，它要定义日志被写向什么地方，以及如何写。
- **日志源**：应用程序写日志时的入口，其实质是一个logger对象的实例。
- **日志过滤器**：决定日志记录是否要被记录的一组判断。
- **日志格式化**：决定日志记录输出的实际格式。
- **日志核心**：维护者日志源、日志槽、日志过滤器等之间的关系的一个全局中的实体。主要在初始化logging library时用到。


### boost log 设计概述
#### 设计概述
boost log日志库由3个层次构成：收集日志数据，处理日志数据，以及在收集日志数据和处理日志数据2个层次之间的一个层次。

boost log 的设计主要由日志器（ Logger ）、日志核心（ Logging core ）、 Sink 前后端（ frontend，backend ）组成。日志文本以及日志环境由日志器（ Logger ）负责搜集，日志核心负责处理日志数据（例如全局过滤、将日志记录传递给 Sink ）， Sink 前端分为同步、异步以及不考虑线程同步问题的版本，它们负责将日志记录传递给 Sink 后端处理。 Sink 后端负责把日志记录格式化并输出到不同的介质中（例如日志文件、报警以及统计源中）。

如下图所示：

![这里写图片描述](https://www.boost.org/doc/libs/1_64_0/libs/log/doc/html/images/log/Design.png)

箭头显示了日志信息的流向。日志信息不一定会存储在日志记录中，因为日志处理的结果可能包括一些操作，而实际上不将信息存储在任何地方。举个例子，如果应用程序处于一个危险的状态，它可以发出一个特殊的日志记录进行特定的处理，以便用户可以看到在系统托盘上的应用程序图标上显示的一个错误消息作为提示通知，并听到一个报警声音。这是一个非常重要的特性。boost log不仅仅用于经典日志记录，还可以向应用程序用户发出一些重要事件并积累统计数据等（图中最后三个箭头所指向的三个用途：经典日志记录、报警通知、数据统计）。

#### 日志源（Logging sources）
boost log 支持自定义日志源，但它不是用来过滤级别的（因为过滤功能用sink的filtering就够了），它的日志源可以包含特定环境的信息（例如在网络连接 network_connection 中的日志源可以携带远程 IP 地址这个属性，这样从那个日志源发出的每一条日志信息都包含此属性）。

*更多请参考以下官方英文说明：*
*Getting back to the figure, in the left side your application emits log records with help of loggers - special objects that provide streams to format messages that will eventually be put to log. The library provides a number of different logger types and you can craft many more yourself, extending the existing ones. Loggers are designed as a mixture of distinct features that can be combined with each other in any combination. You can simply develop your own feature and add it to the soup. You will be able to use the constructed logger just like the others - embed it into your application classes or create and use a global instance of the logger. Either approach provides its benefits. Embedding a logger into some class provides a way to differentiate logs from different instances of the class. On the other hand, in functional-style programming it is usually more convenient to have a single global logger somewhere and have a simple access to it.*

*Generally speaking, the library does not require the use of loggers to write logs. The more generic term "log source" designates the entity that initiates logging by constructing a log record. Other log sources might include captured console output of a child application or data received from network. However, loggers are the most common kind of log sources.*

#### 属性和属性值（Attributes and attribute values）
有时程序需要记录的信息往往不仅仅包含一条消息，还需要包含执行环境的一些属性（例如网络对端的 IP 地址）。 boost::log 提供了属性集功能，属性集不仅仅包括常用的数据（例如计数器、时间、计时器、线程 ID 等信息）还支持自定义属性。用户可以将程序的任意上下文放入日志记录对象中，然后在 sink 中进行处理。

*更多请参考以下官方英文说明：*
*In order to initiate logging a log source must pass all data, associated with the log record, to the logging core. This data or, more precisely, the logic of the data acquisition is represented with a set of named attributes. Each attribute is, basically, a function, whose result is called "attribute value" and is actually processed on further stages. An example of an attribute is a function that returns the current time. Its result - the particular time point - is the attribute value.*

*There are three kinds of attribute sets:*

- *global*
- *thread-specific*
- *source-specific*

*You can see in the figure that the former two sets are maintained by the logging core and thus need not be passed by the log source in order to initiate logging. Attributes that participate in the global attribute set are attached to any log record ever made. Obviously, thread-specific attributes are attached only to the records made from the thread in which they were registered in the set. The source-specific attribute set is maintained by the source that initiates logging, these attributes are attached only to the records being made through that particular source.*

*When a source initiates logging, attribute values are acquired from attributes of all three attribute sets. These attribute values then form a single set of named attribute values, which is processed further. You can add more attribute values to the set; these values will only be attached to the particular log record and will not be associated with the logging source or logging core. As you may notice, it is possible for a same-named attribute to appear in several attribute sets. Such conflicts are solved on priority basis: global attributes have the least priority, source-specific attributes have the highest; the lower priority attributes are discarded from consideration in case of conflicts.*

#### 日志核心和过滤器（Logging core and filtering）
boost log支持两层过滤，可以通过core::set_filter 设置全局过滤器，也能通过 sink::set_filter 设置 sink 过滤器。配合 lambda 表达式或者函数对象，boost log可以生成无论多么复杂的过滤器。（例如只把某个级别的日志写入文件，或者把统计日志、trace日志分别重定向到不同的sink中，然后输出到不同的介质）。

*更多请参考以下官方英文说明：*
*When the set of attribute values is composed, the logging core decides if this log record is going to be processed in sinks. This is called filtering. There are two layers of filtering available: the global filtering is applied first within the logging core itself and allows quickly wiping away unneeded log records; the sink-specific filtering is applied second, for each sink separately. The sink-specific filtering allows directing log records to particular sinks. Note that at this point it is not significant which logging source emitted the record, the filtering relies solely on the set of attribute values attached to the record.*

*It must be mentioned that for a given log record filtering is performed only once. Obviously, only those attribute values attached to the record before filtering starts can participate in filtering. Some attribute values, like log record message, are typically attached to the record after the filtering is done; such values cannot be used in filters, they can only be used by formatters and sinks themselves.*


#### 日志槽和格式化（Sinks and formatting）
boost log可以针对不同sink设置不同参数。如果想要设置不同的格式，只需要传递一个 formatter对象给sink即可。 boost log支持不同的sink按照文件大小、时间等参数来旋转 。boost log支持宽字符日志，sink可以通过设置locale进行必要的编码转换。

boost log支持同步和异步sink，同步sink在将日志传递给backend时会加互斥锁 。 boost log的同步异步 sink 切换起来非常方便，只需要修改类型名就可以了。

boost log支持自定义 sink ，可以实现一条日志信息复制分流到多个 sink 进行处理，也可以在 sink 中以不同的格式输出日志。boost 将日志抽象成 record 对象，不仅仅包含了日志的文本，还可以包含更丰富的自定义属性。



*更多请参考以下官方英文说明：*
*If a log record passes filtering for at least one sink the record is considered to be consumable. If the sink supports formatted output, this is the point when log message formatting takes place. The formatted message along with the composed set of attribute values is passed to the sink that accepted the record. Note that formatting is performed on the per-sink basis so that each sink can output log records in its own specific format.*

*As you may have noticed on the figure above, sinks consist of two parts: the frontend and the backend. This division is made in order to extract the common functionality of sinks, such as filtering, formatting and thread synchronization, into separate entities (frontends). Sink frontends are provided by the library, most likely users won't have to re-implement them. Backends, on the other hand, are one of the most likely places for extending the library. It is sink backends that do the actual processing of log records. There can be a sink that stores log records into a file; there can be a sink that sends log records over the network to the remote log processing node; there can be the aforementioned sink that puts record messages into tool-tip notifications - you name it. The most commonly used sink backends are already provided by the library.*

*Along with the primary facilities described above, the library provides a wide variety of auxiliary tools, such as attributes, support for formatters and filters, represented as lambda expressions, and even basic helpers for the library initialization. You will find their description in the Detailed features description section. However, for new users it is recommended to start discovering the library from the Tutorial section.*

### 其他
- boost log提供很多使用小工具，例如支持日志排序、输出操作、binary dump 操作（ logging::dump(packet.data(), packet.size() 对于输出内存块很方便）。
- boost log支持从配置文件中读取日志配置信息，可以通过修改配置文件来修改日志过滤器、格式等配置。
- 对于不同的输出方式，boost log提供了一些现成的sink backend ，例如输出到 Windows事件日志、调试器、Linux syslog接口、文本文件等。


>更多请参考[boost.log](http://www.boost.org/doc/libs/1_64_0/libs/log/doc/html/index.html)。           