拷贝构造函数调用的几种情况：
1. 当用类的一个对象去初始化该类的另一个对象（或引用）时系统自动调用拷贝构造函数实现拷贝赋值。
2. 若函数的形参为类对象，调用函数时，实参赋值给形参，系统自动调用拷贝构造函数。
3. 当函数的返回值是类对象时，系统自动调用拷贝构造函数。

```c++
#include<ctime>
#include<cstdlib>
#include<iterator>
#include<algorithm>
#include<iostream>
#include<numeric>
using namespace std;

class A {
public:
	A():data(0){}
	A(const A& a){
		data = a.data;
		cout << "拷贝构造函数调用\n";
	}
	A& operator=(const A&a){
		data = a.data;
		cout << "调用赋值函数\n";
		return *this;
	}

	int data;
};

void fun1(A a) {
	return ;
}

A fun2() {
	A a;
	return a;
}

int main() {
	A a;
	A b(a);          //用类的一个对象a去初始化另一个对象b
	A c = a;         //用类的一个对象a去初始化另一个对象c，注意这里是初始化，不是赋值
	fun1(a);         //形参为类对象，实参初始化形参，调用拷贝构造函数。
	A d = fun2();    //函数返回一个类对象时
	d = a;           //d已经初始化过了，这里是赋值，调用赋值函数

	return 0;
}
```