// 友元及运算符重载 示例程序

#include<iostream>
using namespace std;

class T {
public:
    T(int n):count(n){}
    T operator+(const T & t) {
        this->count = this->count + t.count;
        return *this;
    }
    friend std::ostream & operator<<(std::ostream & os, const T & t);
private:
    int count;
};

std::ostream & operator<<(std::ostream & os, const T & t) {
    os << t.count;
}

int main() {
    T t1(10);
    T t2(1);
    cout << (t1 + t2) << endl;
}