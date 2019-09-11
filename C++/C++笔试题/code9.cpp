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
    // int f; 
};  

int main(void) {  
    cout << sizeof(A) << "  " << sizeof(B) << endl;  

    return 0;  
}  