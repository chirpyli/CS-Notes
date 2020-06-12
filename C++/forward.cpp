#include<iostream>
using namespace std;

#define FORWARD

class shape {
public:
    shape(){}
    virtual ~shape(){}
};

class circle: public shape {
public:

};


void foo(const shape&) {
    puts("foo(const shape&)");
}

void foo(shape&&) {
    puts("foo(shape&&)");
}

# ifndef FORWARD
void bar(const shape& s) {
    puts("bar(const shape&)");
    foo(s);
}

void bar(shape&& s) {
    puts("bar(shape&&)");  
    foo(s);
}
#else 
template<class T>
void bar(T&& s) {
    foo(std::forward<T>(s));
}
#endif

int main() {
    bar(circle());

    return 0;
}