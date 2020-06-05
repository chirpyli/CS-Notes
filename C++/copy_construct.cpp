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
	A d = fun2();    //函数返回一个类对象时，可以可能会被编译器优化，从而可能没有调用拷贝构造
	d = a;           //d已经初始化过了，这里是赋值，调用赋值函数

	return 0;
}