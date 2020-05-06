### 静态绑定与动态绑定
讨论静态绑定与动态绑定，首先需要理解的是绑定，何为绑定？<u>函数调用与函数本身的关联，以及成员访问与变量内存地址间的关系，称为绑定。</u> 理解了绑定后再理解静态与动态。

- 静态绑定：指在程序编译过程中，把函数调用与响应调用所需的代码结合的过程，称为静态绑定。发生在编译期。
- 动态绑定：指在执行期间判断所引用对象的实际类型，根据实际的类型调用其相应的方法。程序运行过程中，把函数调用与响应调用所需的代码相结合的过程称为动态绑定。发生于运行期。

### C++中动态绑定
在C++中动态绑定是通过虚函数实现的，是多态实现的具体形式。而虚函数是通过虚函数表实现的。这个表中记录了虚函数的地址，解决继承、覆盖的问题，保证动态绑定时能够根据对象的实际类型调用正确的函数。这个虚函数表在什么地方呢？C++标准规格说明书中说到，<font color=blue>编译器必须要保证虚函数表的指针存在于对象实例中最前面的位置（这是为了保证正确取到虚函数的偏移量）</font>。也就是说，我们可以通过对象实例的地址得到这张虚函数表，然后可以遍历其中的函数指针，并调用相应的函数。

### 虚函数的工作原理
要想弄明白动态绑定，就必须弄懂虚函数的工作原理。C++中虚函数的实现一般是通过虚函数表实现的（C++规范中没有规定具体用哪种方法，但大部分的编译器厂商都选择此方法）。类的虚函数表是一块连续的内存，每个内存单元中记录一个JMP指令的地址。<u>编译器会为每个有虚函数的类创建一个虚函数表，该虚函数表将被该类的所有对象共享。</u> 类的每个虚成员占据虚函数表中的一行。如果类中有N个虚函数，那么其虚函数表将有N*4字节的大小。

虚函数（virtual）是通过虚函数表来实现的，**在这个表中，主要是一个类的虚函数的地址表**，这张表解决了继承、覆盖的问题，保证其真实反映实际的函数。这样，在有虚函数的类的实例中分配了指向这个表的指针的内存（位于对象实例的最前面），所以，当用父类的指针来操作一个子类的时候，这张虚函数表就显得尤为重要，指明了实际所应调用的函数。它是如何指明的呢？后面会讲到。


>JMP指令是汇编语言中的无条件跳转指令，无条件跳转指令可转到内存中任何程序段。转移地址可在指令中给出，也可以在寄存器中给出，或在储存器中指出。
  
首先我们定义一个带有虚函数的基类
```c++
class Base {
public:
	virtual void fun1(){
		cout<<"base fun1!\n";
	}
	virtual void fun2(){
		cout<<"base fun2!\n";
	}
	virtual void fun3(){
		cout<<"base fun3!\n";
	}

	int a;
};
```
查看其内存布局
![这里写图片描述](https://user-gold-cdn.xitu.io/2019/7/5/16bc0bae9efd8c28?w=873&h=1032&f=png&s=42257)
我们可以看到在Base类的内存布局上，第一个位置上存放虚函数表指针，接下来才是Base的成员变量。另外，存在着虚函数表，该表里存放着Base类的所有virtual函数。

>Linux下可以使用`g++ -fdump-class-hierarchy  cppfile.cpp`查看，会生成`cppfile.cpp.002t.class`文件，进行查看。

既然虚函数表指针通常放在对象实例的最前面的位置，那么我们应该可以通过代码来访问虚函数表，通过下面这段代码加深对虚函数表的理解：
```c++
#include "stdafx.h"
#include<iostream>
using namespace std;

class Base {
public:
	virtual void fun1(){
		cout<<"base fun1!\n";
	}
	virtual void fun2(){
		cout<<"base fun2!\n";
	}
	virtual void fun3(){
		cout<<"base fun3!\n";
	}

	int a;
};

int _tmain(int argc, _TCHAR* argv[]) {
	typedef void(*pFunc)(void);
	Base b;
	cout<<"虚函数表指针地址："<<(int*)(&b)<<endl;

	//对象最前面是指向虚函数表的指针，虚函数表中存放的是虚函数的地址
	pFunc pfun;
	pfun=(pFunc)*((int*)(*(int*)(&b)));  //这里存放的都是地址，所以才一层又一层的指针
	pfun();
	pfun=(pFunc)*((int*)(*(int*)(&b))+1);
	pfun();
	pfun=(pFunc)*((int*)(*(int*)(&b))+2);
	pfun();

	system("pause");
	return 0;
}
```
运行结果：      
![这里写图片描述](https://user-gold-cdn.xitu.io/2019/7/5/16bc0bae9ff8f6c8?w=403&h=230&f=png&s=20170)

通过这个例子，对虚函数表指针，虚函数表这些有了足够的理解。下面再深入一些。C++又是如何利用基类指针和虚函数来实现多态的呢？这里，我们就需要弄明白在继承环境下虚函数表是如何工作的。目前只理解单继承，至于虚继承，多重继承待以后再理解。
单继承代码如下：
```c++
class Base {
public:
	virtual void fun1(){
		cout<<"base fun1!\n";
	}
	virtual void fun2(){
		cout<<"base fun2!\n";
	}
	virtual void fun3(){
		cout<<"base fun3!\n";
	}

	int a;
};

class Child:public Base {
public:
	void fun1(){
		cout<<"Child fun1\n";
	}
	void fun2(){
		cout<<"Child fun2\n";
	}
	virtual void fun4(){
		cout<<"Child fun4\n";
	}
};
```
内存布局对比：      
![这里写图片描述](https://user-gold-cdn.xitu.io/2019/7/5/16bc0baea10547d0?w=285&h=307&f=png&s=4176)
![这里写图片描述](https://user-gold-cdn.xitu.io/2019/7/5/16bc0baea118d05d?w=361&h=375&f=png&s=5236)     
通过对比，我们可以看到：        

- 在单继承中，Child类覆盖了Base类中的同名虚函数，在虚函数表中体现为对应位置被Child类中的新函数替换，而没有被覆盖的函数则没有发生变化。
- 对于子类自己的虚函数，直接添加到虚函数表后面。

另外，我们注意到，类Child和类Base中都只有一个vfptr指针（指向的是不同的虚函数表），前面我们说过，该指针指向虚函数表，我们分别输出类Child和类Base的vfptr:
```c++
int _tmain(int argc, _TCHAR* argv[]) {
	typedef void(*pFunc)(void);
	Base b;
	Child c;
	cout<<"Base类的虚函数表指针地址："<<(int*)(&b)<<endl;
	cout<<"Child类的虚函数表指针地址："<<(int*)(&c)<<endl;

	system("pause");
	return 0;
}
```
运行结果：      
![这里写图片描述](https://user-gold-cdn.xitu.io/2019/7/5/16bc0baea3e9ebb4?w=483&h=150&f=png&s=18750)

可以看到，类Child和类Base分别拥有自己的虚函数表指针vfptr和虚函数表vftable。

下面这段代码，说明了<font color=blue>父类和基类拥有不同的虚函数表，同一个类拥有相同的虚函数表，同一个类的不同对象的地址（存放虚函数表指针的地址）不同。</font>
```c++
int _tmain(int argc, _TCHAR* argv[]) {
	Base b;
	Child c1,c2;
	cout<<"Base类的虚函数表的地址："<<(int*)(*(int*)(&b))<<endl;
	cout<<"Child类c1的虚函数表的地址："<<(int*)(*(int*)(&c1))<<endl;	//虚函数表指针指向的地址值
	cout<<"Child类c2的虚函数表的地址："<<(int*)(*(int*)(&c2))<<endl;

	system("pause");
	return 0;
}
```
运行结果：
![这里写图片描述](https://user-gold-cdn.xitu.io/2019/7/5/16bc0baea53c59c3?w=653&h=250&f=png&s=35880)

在定义该派生类对象时，先调用其基类的构造函数，然后再初始化vfptr，最后再调用派生类的构造函数（ 从二进制的视野来看，所谓基类子类是一个大结构体，其中this指针开头的四个字节存放虚函数表头指针。<u>执行子类的构造函数的时候，首先调用基类构造函数，this指针作为参数，在基类构造函数中填入基类的vfptr，然后回到子类的构造函数，填入子类的vfptr，覆盖基类填入的vfptr。如此以来完成vfptr的初始化)。</u>也就是说，vfptr指向vftable发生在构造函数期间完成的。 

动态绑定例子：
```c++
#include "stdafx.h"
#include<iostream>
using namespace std;

class Base {
public:
	virtual void fun1(){
		cout<<"base fun1!\n";
	}
	virtual void fun2(){
		cout<<"base fun2!\n";
	}
	virtual void fun3(){
		cout<<"base fun3!\n";
	}

	int a;
};

class Child:public Base {
public:
	void fun1(){
		cout<<"Child fun1\n";
	}
	void fun2(){
		cout<<"Child fun2\n";
	}
	virtual void fun4(){
		cout<<"Child fun4\n";
	}
};


int _tmain(int argc, _TCHAR* argv[]) {
	Base* p=new Child;
	p->fun1();
	p->fun2();
	p->fun3();

	system("pause");
	return 0;
}

```
运行结果：
![这里写图片描述](https://user-gold-cdn.xitu.io/2019/7/5/16bc0baecd30d60f?w=303&h=190&f=png&s=16339)
结合上面的内存布局：
![这里写图片描述](https://user-gold-cdn.xitu.io/2019/7/5/16bc0baed13aa980?w=361&h=375&f=png&s=5236)

其实，在new Child时构造了一个子类的对象，子类对象按上面所讲，在构造函数期间完成虚函数表指针vfptr指向Child类的虚函数表，将这个对象的地址赋值给了Base类型的指针p，当调用p->fun1()时，发现是虚函数，调用虚函数指针查找虚函数表中对应虚函数的地址，这里就是&Child::fun1。调用p->fun2()情况相同。调用p->fun3()时，子类并没有重写父类虚函数，但依旧通过调用虚函数指针查找虚函数表，发现对应函数地址是&Base::fun3。所以上面的运行结果如上图所示。

到这里，你是否已经明白为什么指向子类实例的基类指针可以调用子类（虚）函数？每一个实例对象中都存在一个vfptr指针，编译器会先取出vfptr的值，这个值就是虚函数表vftable的地址，再根据这个值来到vftable中调用目标函数。所以，只要vfptr不同，指向的虚函数表vftable就不同，而不同的虚函数表中存放着对应类的虚函数地址，这样就实现了多态的”效果“。

---
>可以参考[从编译器的辅助信息看c++对象内存布局](https://www.jianshu.com/p/5eb786351b7d)