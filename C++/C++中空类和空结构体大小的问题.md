### 问题的提出
有一个空类A，sizeof(A)=?  这个问题在笔试中经常遇到，答案是多少呢？
```c++
class A {
};
```

### 代码求解
```c++
#include "stdafx.h"
#include<iostream>
using namespace std;

class A {
};

struct B {
};

int main() {
	cout << sizeof(A) << endl;
	cout << sizeof(B) << endl;
	system("pause");
    return 0;
}
```
运行结果：
![这里写图片描述](https://imgconvert.csdnimg.cn/aHR0cDovL2ltZy5ibG9nLmNzZG4ubmV0LzIwMTcwNzEyMjIxMDE3NTYw?x-oss-process=image/format,png)

可以看到C++中空类和空结构体的大小都是1。

### 结果为什么是1呢？
对于结构体和空类大小是1这个问题，首先这是一个C++问题，在C语言下空结构体大小为0(当然这是编译器相关的)。这里的空类和空结构体是指类或结构体中没有任何成员。
在C++下，空类和空结构体的大小是1（编译器相关），这是为什么呢？为什么不是0？这是因为，C++标准中规定，“**no object shall have the same address in memory as any other variable**” ，就是**任何不同的对象不能拥有相同的内存地址**。 如果空类大小为0，若我们声明一个这个类的对象数组，那么数组中的每个对象都拥有了相同的地址，这显然是违背标准的。所以，C++标准规定不同的对象不能拥有相同的地址。那么怎样才能保证这个条件被满足呢？最简单的方法莫过于不允许任何类型的大小为0。所以**编译器为每个空类或者空结构体都增加了一个虚设的字节**（有的编译器可能加的更多），这样这些空类和空结构的大小就不会是0，就可以保证他们的对象拥有彼此独立的地址。
