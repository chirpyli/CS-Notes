// 分析代码运行结果

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