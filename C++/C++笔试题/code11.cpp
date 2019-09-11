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