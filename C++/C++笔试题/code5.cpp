// 分析代码运行结果
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