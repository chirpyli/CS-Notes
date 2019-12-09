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