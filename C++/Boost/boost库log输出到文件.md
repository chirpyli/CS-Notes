### 程序日志带来的问题？
上面几个例子都是将日志信息输出到控制台上，实际应用中不常用，最常用的情况是我们将日志信息输出到文本文件中，方便我们以后查看，同时程序不运行时日志信息也不会丢掉。输出到文件中后，又带来一些问题。假如程序一直运行，日志信息就要无限的记录下去吗？过多长时间要清理日志，或者日志文件超出多大就删除文件重新记录。我想变更记录的格式怎么办？如何为日志文件命名，以方便我们管理等等？

### boost log 记录日志到文件示例
#### 代码示例1：
添加一个输出日志信息的文件名，将日志信息输出到sample.log文件中
```c++
#include<stdio.h>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

namespace logging=boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

void init1() {
	logging::add_file_log("sample.log");    // 最后运行结果可以看到，日志信息被记录到sample.log的文件中。
	logging::core::get()->set_filter(logging::trivial::severity>=logging::trivial::info);
}

int main() {
	init1();
	BOOST_LOG_TRIVIAL(trace)<<"A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

	return 0;
}
```
注释一：关于`sink`
1. *Sometimes trivial logging doesn't provide enough flexibility. For example, one may want a more sophisticated logic of log processing, rather than simply printing it on the console. **In order to customize this, you have to construct logging sinks and register them with the logging core. This should normally be done only once somewhere in the startup code of your application.***
2. 现在我们遇到了sink的概念。sink决定日志信息输出到什么地方。在前面两篇文章中，没有添加sink，是因为boost log使用console作为默认的sink。 现在，因为调用了add_file_log函数，一旦自己添加了sink,默认的sink就不在有效，被file sink取代。要设置sink, 需要将其添加到core中
```c++
	logging::add_file_log("sample.log");
```

注释二：`add_file_log()`
1. *As the name implies, the function initializes a logging sink that stores log records into a text file. The function also accepts a number of customization options, such as the file rotation interval and size limits.*
2. 下个例子中将更详细的应用这个函数


#### 代码示例2：
设置日志文件名，文件大小的限制，输出信息格式等。
```c++
#include<stdio.h>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

namespace logging=boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

void init() {
	logging::add_file_log
    (
        keywords::file_name = "sample_%N.log",                                        /*< file name pattern >*/
        keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
        keywords::format = "[%TimeStamp%]: %Message%"                                 /*< log record format >*/
    );

	logging::core::get()->set_filter(logging::trivial::severity>=logging::trivial::info);
}

int main() {
	init();
	logging::add_common_attributes();

	using namespace logging::trivial;
	src::severity_logger<severity_level> lg;

    BOOST_LOG_SEV(lg, trace) << "A trace severity message";
    BOOST_LOG_SEV(lg, debug) << "A debug severity message";
    BOOST_LOG_SEV(lg, info) << "An informational severity message";
    BOOST_LOG_SEV(lg, warning) << "A warning severity message";
    BOOST_LOG_SEV(lg, error) << "An error severity message";
    BOOST_LOG_SEV(lg, fatal) << "A fatal severity message";

	system("pause");
	return 0;
}
```
注释一：keywords
|keywords|说明|
|-----|----|
|file_name|The keyword allows to pass log file name the rotating file stream methods|
|rotation_size|The keyword allows to pass maximum log file size to the file sink|
|time_based_rotation|The keyword allows to pass time-based file rotation predicate to the file sink backend|
|format|The keyword for passing format specifiers to functions|


注释二：add_common_attributes()
The function adds commonly used attributes to the logging system. Specifically, the following attributes are registered globally:
|attributes|说明|
|-----|-----|
|LineID |logging records counter with value type <tt>unsigned int</tt>|
|TimeStamp|local time generator with value type <tt>boost::posix_time::ptime</tt>|
 |ProcessID|current process identifier with value type <tt>attributes::current_process_id::value_type</tt>|
|ThreadID|in multithreaded builds, current thread identifier with value type <tt>attributes::current_thread_id::value_type</tt>|


### boost log 相关源码
下面这段代码是`keywords::format`相关的源码
```c++
/*
 *          Copyright Andrey Semashev 2007 - 2015.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   keywords/format.hpp
 * \author Andrey Semashev
 * \date   14.03.2009
 *
 * The header contains the \c format keyword declaration.
 */

#ifndef BOOST_LOG_KEYWORDS_FORMAT_HPP_INCLUDED_
#define BOOST_LOG_KEYWORDS_FORMAT_HPP_INCLUDED_

#include <boost/parameter/keyword.hpp>
#include <boost/log/detail/config.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace keywords {

//! The keyword for passing format specifiers to functions
BOOST_PARAMETER_KEYWORD(tag, format)	//这里是重点

} // namespace keywords

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_KEYWORDS_FORMAT_HPP_INCLUDED_
```

下面这段是common_attributes.hpp源码
```c
/*
 *          Copyright Andrey Semashev 2007 - 2015.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   common_attributes.hpp
 * \author Andrey Semashev
 * \date   16.05.2008
 *
 * The header contains implementation of convenience functions for registering commonly used attributes.
 */

#ifndef BOOST_LOG_UTILITY_SETUP_COMMON_ATTRIBUTES_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_SETUP_COMMON_ATTRIBUTES_HPP_INCLUDED_

#include <iostream>
#include <boost/log/detail/config.hpp>
#include <boost/log/core/core.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/counter.hpp>
#include <boost/log/attributes/current_process_id.hpp>
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/log/attributes/current_thread_id.hpp>
#endif
#include <boost/log/detail/default_attribute_names.hpp>
#include <boost/log/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

/*!
 * \brief Simple attribute initialization routine
 *
 * The function adds commonly used attributes to the logging system. Specifically, the following
 * attributes are registered globally:
 *
 * \li LineID - logging records counter with value type <tt>unsigned int</tt>
 * \li TimeStamp - local time generator with value type <tt>boost::posix_time::ptime</tt>
 * \li ProcessID - current process identifier with value type
 *     <tt>attributes::current_process_id::value_type</tt>
 * \li ThreadID - in multithreaded builds, current thread identifier with
 *     value type <tt>attributes::current_thread_id::value_type</tt>
 */
inline void add_common_attributes()
{
    shared_ptr< core > pCore = core::get();   //获取单实例
    //添加几个默认的属性值
    pCore->add_global_attribute(
        aux::default_attribute_names::line_id(),
        attributes::counter< unsigned int >(1));
    pCore->add_global_attribute(
        aux::default_attribute_names::timestamp(),
        attributes::local_clock());
    pCore->add_global_attribute(
        aux::default_attribute_names::process_id(),
        attributes::current_process_id());
#if !defined(BOOST_LOG_NO_THREADS)
    pCore->add_global_attribute(
        aux::default_attribute_names::thread_id(),
        attributes::current_thread_id());
#endif
}

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#include <boost/log/detail/footer.hpp>

#endif // BOOST_LOG_UTILITY_SETUP_COMMON_ATTRIBUTES_HPP_INCLUDED_
```

以下是default_attribute_names.hpp源代码，接上一个源码看
```c
/*
 *          Copyright Andrey Semashev 2007 - 2015.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   default_attribute_names.hpp
 * \author Andrey Semashev
 * \date   15.01.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/doc/libs/release/libs/log/doc/html/index.html.
 */

#ifndef BOOST_LOG_DETAIL_DEFAULT_ATTRIBUTE_NAMES_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_DEFAULT_ATTRIBUTE_NAMES_HPP_INCLUDED_

#include <boost/log/detail/config.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace aux {

namespace default_attribute_names {

BOOST_LOG_API attribute_name severity();
BOOST_LOG_API attribute_name channel();
BOOST_LOG_API attribute_name message();
BOOST_LOG_API attribute_name line_id();    //这下面四个就是上面源文件中用到的属性名
BOOST_LOG_API attribute_name timestamp();
BOOST_LOG_API attribute_name process_id();
BOOST_LOG_API attribute_name thread_id();

} // namespace default_attribute_names

} // namespace aux

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#include <boost/log/detail/footer.hpp>

#endif // BOOST_LOG_DETAIL_DEFAULT_ATTRIBUTE_NAMES_HPP_INCLUDED_
```