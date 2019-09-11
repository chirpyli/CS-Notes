// 分析代码运行结果

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