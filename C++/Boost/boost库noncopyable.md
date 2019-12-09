### noncopyable
noncopyable允许程序轻松地实现一个禁止拷贝的类，noncopyable位于名字空间boost，为了使用noncopyable组件，需要包含头文件
```c++
#include<boost/noncopyable.hpp>
```

### 实现原理
在C++中定义一个类时，如果不明确定义拷贝构造函数和拷贝赋值操作符，编译器会自动生成这两个函数。当我们希望禁止拷贝类的实例时，就不能用默认生成的这两个函数。这是一个很经典的C++惯用法，原理很好理解，只需要私有化拷贝构造函数和拷贝赋值操作符即可。
```c++
class do_not_copy
{
private:
	do_not_copy(const do_not_copy&);
	do_not_copy& operator=(const do_not_copy&);
};
```

### 用法
noncopyable为实现不可拷贝的类提供了简单清晰的解决方案：从boost::noncopyable派生即可。当我们的自定义类是noncopyable的子类时就会自动私有化父类noncopyable的拷贝构造函数，从而禁止用户从外部访问拷贝构造函数和拷贝赋值函数。

示例代码[noncopyable.cpp](./noncopyable.cpp)
```c++
#include<iostream>
#include<stdio.h>

#include<boost/noncopyable.hpp>

class do_not_copy:boost::noncopyable {
};

int main() {
	using namespace std;
	do_not_copy d1;			//一个不可拷贝的对象
	do_not_copy d2(d1);		//企图拷贝构造，编译出错
	do_not_copy d3;			//一个不可拷贝的对象
	d3 = d1;				//企图拷贝赋值，编译出错

    return 0;
}
```


### 源码实现
有2种实现方法，其一是类似原理中讲的用private的方法。还有一种是使用C++11新的default和delete关键字，具体见以下boost中noncopyable的源代码：
```c++
//  Boost noncopyable.hpp header file  --------------------------------------//

//  (C) Copyright Beman Dawes 1999-2003. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/utility for documentation.

#ifndef BOOST_CORE_NONCOPYABLE_HPP
#define BOOST_CORE_NONCOPYABLE_HPP

#include <boost/config.hpp>

namespace boost {

//  Private copy constructor and copy assignment ensure classes derived from
//  class noncopyable cannot be copied.

//  Contributed by Dave Abrahams

namespace noncopyable_  // protection from unintended ADL
{
  class noncopyable
  {
  protected:
#if !defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS) && !defined(BOOST_NO_CXX11_NON_PUBLIC_DEFAULTED_FUNCTIONS)
      BOOST_CONSTEXPR noncopyable() = default;
      ~noncopyable() = default;
#else
      noncopyable() {}
      ~noncopyable() {}
#endif
#if !defined(BOOST_NO_CXX11_DELETED_FUNCTIONS)
      noncopyable( const noncopyable& ) = delete;
      noncopyable& operator=( const noncopyable& ) = delete;
#else
  private:  // emphasize the following members are private
      noncopyable( const noncopyable& );
      noncopyable& operator=( const noncopyable& );
#endif
  };
}

typedef noncopyable_::noncopyable noncopyable;

} // namespace boost

#endif  // BOOST_CORE_NONCOPYABLE_HPP
```