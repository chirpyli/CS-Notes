// 函数模板示例代码

#include<iostream>
#include<string>
using namespace std;

struct City {
    string name;
    int population;
};

template<class T>
void Swap(T &a, T &b);

template<> void Swap<City>(City &a, City &b);

template<class T1, class T2>
void printplus(T1 a, T2 b);

int main() {
    int a = 10;
    decltype(a) b = 20;
    Swap(a, b);
    cout << a << b;

    double da = 10.1;
    double db = 100.1;
    Swap(da, db);
    cout << da << db << endl;;

    City ca = {"beijing", 2100};
    City cb = {"shanghai", 2400};
    Swap(ca, cb);    
    cout << ca.name << ":" << ca.population << endl;
    cout << cb.name << ":" << cb.population << endl;

    printplus(a, da);    

}

// 常规模板
template<class T>
void Swap(T &a, T &b) {
    T tmp;
    tmp = a;
    a = b;
    b = tmp;
}

// 具体化模板
template<> void Swap<City>(City &a, City &b) {
    int tmp = a.population;
    a.population = b.population;
    b.population = tmp;
}

template<class T1, class T2>
void printplus(T1 a, T2 b) {
    decltype(a+b) c = a + b;        // 如果这里不用decltype，无法判断a+b的结果是T1类型还是T2类型或者其他类型
    cout << c;
}