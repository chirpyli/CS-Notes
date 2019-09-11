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