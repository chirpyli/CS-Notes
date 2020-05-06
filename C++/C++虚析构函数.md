C++虚析构函数主要关注两个问题，什么时候要用虚析构以及它是怎么工作的。下面回答这两个问题。
### <font color=blue>问题1：什么时候要用虚析构函数?</font>
	
<u>答案：通过基类的指针来删除派生类的对象时，基类的析构函数应该是虚的。这样做是为了当用一个基类的指针删除一个派生类的对象时，派生类的析构函数会被调用。</u>
	
原因：用对象指针来调用一个函数，有以下两种情况：<u>如果是虚函数，会调用派生类中的版本; 如果是非虚函数，会调用指针所指类型的实现版本。</u> 析构函数也会遵循以上两种情况。当对象出了作用域或是我们删除对象指针，析构函数就会被调用。当派生类对象出了作用域，派生类的析构函数会先调用，然后再调用它父类的析构函数，这样能保证分配给对象的内存得到正确释放。但是，如果我们删除一个指向派生类对象的基类指针，而基类析构函数又是非虚的话， 那么就会先调用基类的析构函数(上面第2种情况)，派生类的析构函数得不到调用。这样会造成销毁对象不完全。
	
简要解释就是：析构函数执行时先调用派生类的析构函数，其次才调用基类的析构函数。如果析构函数不是虚函数，而程序执行时又要通过基类的指针去销毁派生类的动态对象，那么用delete销毁对象时，只调用了基类的析构函数，未调用派生类的析构函数。这样会造成销毁对象不完全。

>注意：并不是要把所有类的析构函数都写成虚函数。因为当类里面有虚函数的时候，编译器会给类添加一个虚函数表，里面来存放虚函数指针，这样就会增加类的存储空间。所以，只有当一个类被用来作为基类的时候，才把析构函数写成虚函数。

代码实例如下：
```c++
class Base {
public:
    Base() { cout<<"Base Constructor"<<endl; }
 //   ~Base() { cout<<"Base Destructor"<<endl; }
	 virtual ~Base() { cout<<"Base Destructor"<<endl; }
};

class Derived: public Base{
public:
    Derived() { cout<<"Derived Constructor"<<endl; }
    ~Derived() { cout<<"Derived Destructor"<<endl; }
};

int main(){
    Base *p = new Derived();
    delete p;
    return 0;
}
```	
未添加虚析构函数输出：
```
BaseConstructor

DerivedConstructor

BaseDestructor
```

添加虚析构函数输出：
```
BaseConstructor

DerivedConstructor

DerivedDestructor

BaseDestructor
```


### <font color=blue> 问题2：C++中的虚函数是如何工作的？</font>

虚函数依赖虚函数表进行工作。如果一个类中，有函数被关键词virtual进行修饰， 那么一个虚函数表就会被构建起来保存这个类中虚函数的地址。同时，编译器会为这个类添加一个隐藏指针（虚函数表的指针）指向虚函数表。如果在派生类中没有重写虚函数，那么，派生类中虚表存储的是父类虚函数的地址。每当虚函数被调用时， 虚表会决定具体去调用哪个函数。因此，C++中的动态绑定是通过虚函数表机制进行的。当我们用基类指针指向派生类时，虚表指针vptr指向派生类的虚函数表。 这个机制可以保证派生类中的虚函数被调用到。


示例代码：	
```C++
#include <iostream>
using namespace std;

class Shape {
public:
  Shape(){}
  Shape(int edge_length){
    this->edge_length = edge_length;
  }
  virtual ~Shape(){
    cout<<"Shape destructure."<<endl;
  }

  virtual int circumstance(){
    cout<<"circumstance of base class."<<endl;
    return 0;
  }

protected:
  int edge_length;

};

class Triangle: public Shape{
public:
  Triangle(){}

  Triangle(int edge_length){
    this->edge_length = edge_length;
  }

  ~Triangle(){
    cout<<"Triangle destructure."<<endl;
  }

  int circumstance(){
    cout<<"circumstance of child class."<<endl;
    return 3 * this->edge_length;
  }
};


int main() {
  Shape *x = new Shape();
  x->circumstance();
  Shape *y = new Triangle(10);
  int num = y->circumstance();
  cout<<num<<endl;

  delete x;
  delete y;

  return 0;
}
```
运行结果：
```
circumstance of base class.
circumstance of child class.
30
Shape destructure.
Triangle destructure.
Shape destructure.
```