// 分析代码运行结果
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