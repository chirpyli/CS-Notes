boost是一个功能强大、构造精巧、跨平台、开源并且完全免费的C++程序库。C++编程时会经常用到。使用boost库，第一步就是安装了，这里只说明Linux下如何安装，Windows下安装与配置请参考[《boost安装（windows、linux）》](https://blog.csdn.net/s_lisheng/article/details/72871218)


最简单的方法，运行```sudo apt-get install libboost-all-dev```。

安装后可在`/usr/include`中查看到`boost`目录。也可在[Boost官网](https://www.boost.org/)下载源码安装。

运行完后可用如下代码测试是否安装成功。

测试代码[boost_install.cpp](./boost_install.cpp)：
```c++
#include <iostream>
#include<boost/version.hpp>
#include<boost/config.hpp>

using namespace std;

int main() {
    cout << BOOST_VERSION << endl;
    cout << BOOST_LIB_VERSION << endl;
    cout << BOOST_PLATFORM << endl;
    cout << BOOST_COMPILER << endl;
    cout << BOOST_STDLIB << endl;

  return 0;
}
```

安装成功的话会打印如下内容：
```
105800
1_58
linux
GNU C++ version 5.4.0 20160609
GNU libstdc++ version 20160609
```