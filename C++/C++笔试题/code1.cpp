// 分析代码运行结果 

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
 */