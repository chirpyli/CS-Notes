### 关于logger对象
简单的理解就是日志源，以下是定义logger对象的几种方式。还有更多的定义方式，详见官方文档。
```c++
src::logger lg;		//the non-thread-safe ones
src::logger_mt lg_mt;	//the thread-safe ones
BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::logger_mt)		//利用宏定义定义全局对象
```
logger与logger_mt两者的区别：

- *For the non-thread-safe loggers it is safe for different threads to write logs through different instances of loggers and thus there should be a separate logger for each thread that writes logs.*
- *The thread-safe counterparts can be accessed from different threads concurrently, but this will involve locking and may slow things down in case of intense logging. The thread-safe logger types have the _mt suffix in their name.*


### boost log库logger使用示例
两个不同的logger定义方式，注意一下。
```c++
#include<iostream>
#include <boost/move/utility_core.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;

//Here my_logger is a user-defined tag name that will be used later to retrieve the logger instance and logger_mt is the logger type. 
BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::logger_mt)

void log_fun1() {
	src::logger lg;
//--------------
	logging::record rec = lg.open_record();
	if (rec)
	{
		logging::record_ostream strm(rec);
		strm << "Hello, World!";
		strm.flush();
		lg.push_record(boost::move(rec));
	}
	//---这段代码等同于BOOST_LOG------
	//其实BOOST_LOG的内部也是用类似上面的代码实现的。
}

void log_fun2() {
	src::logger_mt& lg = my_logger::get();  //获取logger对象
	BOOST_LOG(lg)<<"Greetings from the global logger!";
}

int main() {
	logging::add_file_log("sample.log");
	logging::add_common_attributes();
	
	log_fun1();
	log_fun2();

    return 0;
}
```
注释一：`my_logger::get()`
1. The lg will refer to the one and only instance of the logger throughout the application, even if the application consists of multiple modules. The get function itself is thread-safe, so there is no need in additional synchronization around it. 



### boost log 相关源码
下面是`logger.hpp`的源码
```c++
/*
 *          Copyright Andrey Semashev 2007 - 2015.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   logger.hpp
 * \author Andrey Semashev
 * \date   08.03.2007
 *
 * The header contains implementation of a simplistic logger with no features.
 */

#ifndef BOOST_LOG_SOURCES_LOGGER_HPP_INCLUDED_
#define BOOST_LOG_SOURCES_LOGGER_HPP_INCLUDED_

#include <boost/log/detail/config.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/features.hpp>
#include <boost/log/sources/threading_models.hpp>
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/log/detail/light_rw_mutex.hpp>
#endif // !defined(BOOST_LOG_NO_THREADS)
#include <boost/log/detail/header.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace sources {

#ifdef BOOST_LOG_USE_CHAR

/*!
 * \brief Narrow-char logger. Functionally equivalent to \c basic_logger.
 *
 * See \c basic_logger class template for a more detailed description.
 */
class logger :
    public basic_composite_logger< char, logger, single_thread_model, features< > >
{
    BOOST_LOG_FORWARD_LOGGER_MEMBERS(logger)
};

#if !defined(BOOST_LOG_NO_THREADS)

/*!
 * \brief Narrow-char thread-safe logger. Functionally equivalent to \c basic_logger.
 *
 * See \c basic_logger class template for a more detailed description.
 */
class logger_mt :
    public basic_composite_logger< char, logger_mt, multi_thread_model< boost::log::aux::light_rw_mutex >, features< > >
{
    BOOST_LOG_FORWARD_LOGGER_MEMBERS(logger_mt)
};

#endif // !defined(BOOST_LOG_NO_THREADS)
#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

/*!
 * \brief Wide-char logger. Functionally equivalent to \c basic_logger.
 *
 * See \c basic_logger class template for a more detailed description.
 */
class wlogger :
    public basic_composite_logger< wchar_t, wlogger, single_thread_model, features< > >
{
    BOOST_LOG_FORWARD_LOGGER_MEMBERS(wlogger)
};

#if !defined(BOOST_LOG_NO_THREADS)

/*!
 * \brief Wide-char thread-safe logger. Functionally equivalent to \c basic_logger.
 *
 * See \c basic_logger class template for a more detailed description.
 */
class wlogger_mt :
    public basic_composite_logger< wchar_t, wlogger_mt, multi_thread_model< boost::log::aux::light_rw_mutex >, features< > >
{
    BOOST_LOG_FORWARD_LOGGER_MEMBERS(wlogger_mt)
};

#endif // !defined(BOOST_LOG_NO_THREADS)
#endif // BOOST_LOG_USE_WCHAR_T

} // namespace sources

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#include <boost/log/detail/footer.hpp>

#endif // BOOST_LOG_SOURCES_LOGGER_HPP_INCLUDED_
```