#### 1. 运行下面的[C++代码](./code1.cpp)，得到的结果是什么？
```c++
#include<iostream>
using namespace std;

class A  
{  
public:  
    A(int value): m_value(value) {}  
    
    void Print1() {  
        cout << "hello world" << endl;  
    }  
    
    void Print2() {  
        cout << m_value << endl;  
    }  
    
    virtual void Print3() {  
        cout << "hello world" << endl;  
    }  
private:  
    int m_value;  
};

int main() {
	A* pA;
	pA->Print1();
	pA->Print2();
	pA->Print3();

	return 0;
}
/*
运行结果：
hello world
Segmentation fault (core dumped
 * /
```
答案是：Print1调用正常，打印出hello world，但运行至Print2时，程序崩溃。调用Print1时，并不需要pA的地址，因为Print1的函数地址是固定的。编译器会给Print1传入一个this指针，该指针为NULL，但在Print1中该this指针并没有用到。**只要程序运行时没有访问不该访问的内存就不会出错**，因此运行正常。在运行print2时，需要this指针才能得到m_value的值。由于此时this指针为NULL，因此程序崩溃了。对于Print3这一虚函数，C++在调用虚函数的时候，要根据实例（即this指针指向的实例）中虚函数表指针得到虚函数表，再从虚函数表中找到函数的地址。由于这一步需要访问实例的地址（即this指针），而此时this指针为空指针，因此导致内存访问出错。


#### 2. 运行下面的[C++代码](./code2.cpp)，得到的结果是什么？
```c++
#include<iostream>
using namespace std;

class A
{
public:  
    A() {  
        Print();  
    }  
    virtual void Print()  
    {  
        cout << "A is constructed." << endl;
    }  
}; 

class B: public A  
{  
public:  
    B() {  
        Print();  
    }  
    virtual void Print() {  
        cout << "B is constructed." << endl; 
    }  
};  

int main() {  
    A* pA = new B();  
    delete pA;  

    return 0;  
}
```
先后打印出两行:A is constructed. B is constructed. 调用B的构造函数时，先会调用B的基类A的构造函数。然后在A的构造函数里调用Print。由于此时实例的类型B的部分还没有构造好，本质上它只是A的一个实例，他的虚函数表指针指向的是类型A的虚函数表。因此此时调用的Print是A::Print。接着调用类型B的构造函数，并调用Print。此时已经开始构造B，并且虚函数表的指针已指向类B的虚函数表地址，因此此时调用的Print是B::Print。

#### 3. 运行下面的[C++代码](./code3.cpp)，输出是什么？
```c++
#include<iostream>
using namespace std;

class A{  
public:  
    A(): n2(0), n1(n2 + 2) {}  
   
    void Print() {  
        std::cout << "n1: " << n1 << ", n2: " << n2 << std::endl;  
    }  
private:  
    int n1;  
    int n2;  
}; 
 
int main() {  
    A a;  
    a.Print();  

    return 0;  
} 
```
输出n1是一个随机的数字，n2为0。**在C++中，成员变量的初始化顺序与变量在类型中的申明顺序相同，而与它们在构造函数的初始化列表中的顺序无关。** 因此首先初始化n1，而初始n1的参数n2还没有初始化，是一个随机值。初始化n2时，根据参数0对其初始化，故n2=0。


#### 4. 编译运行下面的[C++代码](./code4.cpp)，结果是什么？（A）编译错误；（B）编译成功，运行时程序崩溃；（C）编译运行正常，输出10。请选择正确答案并分析原因。
```c++
#include<iostream>
using namespace std;

class A{  
private:  
    int value;  
public:  
    A(int n) {  
        value = n;  
    }  
    A(A other) {  
        value = other.value;  
    }  
    void Print() {  
       std::cout << value << std::endl;  
    }  
};  

int main() {  
    A a = 10;  
    A b = a;  
    b.Print();  
    return 0;  
} 
```
编译错误。在复制构造函数中传入的参数是A的一个实例。**由于是传值，把形参拷贝到实参会调用复制构造函数。因此如果允许复制构造函数传值，那么会形成永无休止的递归并造成栈溢出。因此C++的标准不允许复制构造函数传值参数，而必须是传引用或者常量引用。** 复制构造函数的参数需要改为：const A& other。

#### 5. 运行如下的[C++代码](./code5.cpp)，输出是什么？
```c++
#include<iostream>
using namespace std;

class A {  
public:  
    virtual void Fun(int number = 10) {  
        std::cout << "A::Fun with number " << number;  
    }  
};  
       
class B:public A  
{  
public:  
   virtual void Fun(int number = 20) {  
        std::cout << "B::Fun with number " << number;  
    }  
};  
       
int main() {  
    B b;  
    A &a = b;  
    a.Fun();  

	return 0;
}  
```
输出  B::Fun with number 10。由于a是一个指向B实例的引用，因此在运行的时候会调用B::Fun。但缺省参数是在编译期决定的。在编译的时候，编译器只知道a是一个类型a的引用，具体指向什么类型在编译期是不能确定的，因此会按照A::Fun的声明把缺省参数number设为10。这一题的**关键在于理解确定缺省参数的值是在编译的时候，但确定引用、指针的虚函数调用哪个类型的函数是在运行的时候**。

#### 6. 运行如下的[C代码](./code6.c)，输出是什么？
```c
#include<stdio.h>

char* GetString1() {  
   char p[] = "Hello World";    //指向临时分配的桟空间，当运行至函数体外时，空间将被释放  
   return p;  
}  

char* GetString2() {  
   char *p = "Hello World";     //指向全局常量区  
   return p;  
}  

int main() {  
    printf("GetString1 returns: %s. \n", GetString1());  
    printf("GetString2 returns: %s. \n", GetString2());  
    
    return 0;  
}
```
输出两行，第一行GetString1 returns: 后面跟的是一串随机的内容，而第二行GetString2 returns: Hello World.两个函数的区别在于GetString1中是一个数组，而GetString2中是一个指针。运行到GetString1时，p是一个数组，会开辟一块内存，并拷贝"Hello World"初始化该数组。接着返回数组的首地址并退出该函数。由于p是GetString1内的一个局部变量，当运行到这个函数外面的时候，这个数组的内存会被释放掉。因此在_tmain函数里再去访问这个数组的内容时，结果是随机的。运行到GetString2时，p是一个指针，它指向的是字符串常量区的一个常量字符串。该常量字符串是一个全局的，并不会因为退出函数GetString2而被释放掉。

#### 7. 运行下图中[C代码](./code7.c)，输出的结果是什么？
```c
#include<stdio.h>

int main() {  
    char str1[] = "hello world";//桟空间  
    char str2[] = "hello world";//桟空间，临时分配，地址不同  
    char* str3 = "hello world";//常量区  
    char* str4 = "hello world";//指向同一块全局常量区  
    if(str1 == str2)  
       printf("str1 and str2 are same.\n");  
    else  
       printf("str1 and str2 are not same.\n");  
    if(str3 == str4)  
       printf("str3 and str4 are same.\n");  
    else  
        printf("str3 and str4 are not same.\n");  
       
    return 0;  
}  
```
这个题目与上一题目类似。str1和str2是两个字符串数组。我们会为它们分配两个长度为12个字节的空间，并把"hello world"的内容分别拷贝到数组中去。这是两个初始地址不同的数组，因此比较str1和str2的值，会不相同。str3和str4是两个指针，我们无需为它们分配内存以存储字符串的内容，而只需要把它们指向"hello world“在内存中的地址就可以了。由于"hello world”是常量字符串，它在内存中只有一个拷贝，因此str3和str4指向的是同一个地址。因此比较str3和str4的值，会是相同的。

#### 8. 运行Test,输出结果是什么？
```c++
#include<iostream>
using namespace std;

void Test() {  
    class B  
    {  
    public:  
        B(void) {  
            cout << "B\t";  
        }  
        ~B(void) {  
            cout << "~B\t";  
        }  
    }; 

    struct C  
    {  
        C(void) {  
            cout << "C\t";  
        }  
        ~C(void) {  
            cout << "~C\t";  
        }  
    };  
    
    struct D : B  
    {  
        D() {  
            cout << "D\t";  
        }  
        ~D() {  
            cout << "~D\t";  
        }  

    private:  
        C c;  
    };  

    D d;  
}  

int main() {
    Test();

    return 0;
}
```
运行结果：B   C   D   ~D   ~ C   ~B。当实例化D对象时，由于继承自B，因而首先调用B的构造函数，之后初始化私有成员C，完成父类的构造与私有成员的初始化后再进入D的构造函数体内；之后，按照相反顺序完成对象的析构操作。初始化与赋值是不同的，一般初始化是在初始化列表完成的，构造函数体中进行的是赋值操作。

#### 9. 下列程序输出结果是什么？
```c
#include<iostream>
using namespace std;

class A  
{  
public:  
    int a;          //4字节  
    char b;         //1字节  
    double c;       //8字节，以此为基本单位进行字节对齐，上面的两个变量对齐后共为8字节，加上当前字节数，共为8+8=16字节。  
    virtual void print()//虚函数，构建虚函数表，虚函数表指针需要4字节，字节对其，扩充为8字节  
    {  
        cout << "this is father's function!" << endl;  
    }  
    
    virtual void print1()//地址存于虚函数表  
    {  
        cout << "this is father's function1!" << endl;  
    }  
    
    virtual void print2()//无需分配内存  
    {  
        cout << "this is father's function2!" << endl;  
    }  
private:  
    float d;//4字节，字节对其，扩充为8字节  
};  
  
class B : A//首先承载A的大小：32字节  
{  
public:  
    virtual void print()//修改虚函数表地址  
    {  
        cout << "this is children's function!" << endl;  
    }  
    void print1()//仅存有函数入口地址，无需分配内存  
    {  
        cout << "this is children's function1!" << endl;  
    }  
private:  
    char e;//1字节，字节对齐，扩充为8字节(可以发现，继承后，字节对齐单位也放生变化) 
};  

int main(void) {  
    cout << sizeof(A) << "  " << sizeof(B) << endl;  

    return 0;  
}  
```
运行结果：32，32.这个题目解决的关键在于掌握字节对齐的相关知识点。具体见上面注释。

#### 10. 以下[程序](./code10.cpp)，在编译与运行时或发生什么？
```c++
#include<iostream>
using namespace std;

class A  
{  
public:  
    virtual void foo() {}  
};  

class B  
{  
public:  
    virtual void foo() {}  
};  

class C : public A , public B  
{  
public:  
    virtual void foo() {}  
};  

void bar1(A *pa) {  
    B *pc = dynamic_cast<B*>(pa);//运行期遍历继承树  
    cout << pc << endl;
}  

void bar2(A *pa) {  
    B *pc = static_cast<B*>(pa);//两个类无关，编译出错  
} 

void bar3() {  
    C c;  
    A *pa = &c;  
    B *pb = static_cast<B*>(static_cast<C*>(pa));//存在继承关系，编译正确  
    cout << pa << endl;
    cout << pb << endl;

}

int main(void) {  
    A a;
    bar1(&a);
    bar3();

    return 0;  
}   
```
对于bar1，dynamic_cast是在运行时遍历继承树，所以，在编译时不会报错。但是因为A和B无继承关系，所以运行时报错。static_cast：编译器隐式执行的任何类型转换都可由它显示完成。其中对于：（1）基本类型。如可以将int转换为double(编译器会执行隐式转换)，但是不能将int*用它转换到double*（没有此隐式转换）。（2）对于用户自定义类型，如果两个类无关，则会出错，如果存在继承关系，则可以在基类和派生类之间进行任何转型，在编译期间不会出错。所以bar3可以通过编译。

#### 11. 执行下列[程序](./code11.cpp)，会发生什么？
```c++
#include<iostream>
#include<stdio.h>
using namespace std;

class A  
{  
public:  
    string a;  
    
    void f1() {  
        printf("Hello World");  
    }  
    
    void f2() {  
        a = "Hello World";  
        printf("%s",a.c_str());  
    }  
    
    virtual void f3() {  
        a = "Hello World";  
        printf("%s",a.c_str());  
    }  
    static void f4() {  
        printf("Hello World");  
    }  
};  
      
int main(void) {  
    A *aptr = NULL;  //创建一个A对象，对象指针为空，意味着对象仅有空壳，无法借助指针访问成员变量  
    aptr->f1();      //运行成功，调用f1函数仅需函数入口地址，无需访问对象中的成员变量  
    // aptr->f2();      //运行失败，调用f2需访问成员变量  
    // aptr->f3();      //运行失败，同上  
    aptr->f4();      //静态成员不属于任何对象，运行成功  
    return 0;  
}  
```
此题解答如程序注释所示。

#### 12. 下列[函数](./code12.c)运行情况如何？
```c
#include<string.h>
#include<stdio.h>

int func() {  
    char b[2]={0};  
    strcpy(b,"aaa");  
    printf("%s", b);    
}

int main() {
    func();
}
```
Debug版崩溃，Release版正常。因为在Debug中有ASSERT断言保护，所以要崩溃，而在Release中就会删掉ASSERT，所以正常运行。但是不推荐这么做，因为这样会覆盖不属于自己的内存。