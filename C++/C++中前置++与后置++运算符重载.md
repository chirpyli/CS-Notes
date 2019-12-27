C++中前置++ 与后置++是不同的，具体不同，下面代码中有体现。在后置++里，人为添加一个参数（int），主要是为了区别前置++，这个参数不会被使用。这里的int是个哑元。一个函数的参数只有类型没有名字，则这个参数称之为哑元。

```c++
class A
{
public:
	A& operator++()//前置++，返回的是引用
	{
		data += 1;
		return *this;
	}
	const A operator++(int)//后置++，返回的是值
	{
		A old(*this);
		++(*this);	//调用前置++
		return old;
	}
//从代码可以看出，前置++比后置++效率高，不用产生临时对象，不用调用拷贝构造函数
	int data;
};

ostream& operator<<(ostream& os, A& a) {
	os<<a.data<<endl;
	return os;
}

int main() {
	A a={1};
	cout<<a;//1
	A b=++a;
	cout<<b;//2
	cout<<a;//2
	A c=a++;
	cout<<c;//2
	cout<<a;//3

	return 0;
}
```