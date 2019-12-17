// c++ 智能指针
#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<cassert>

using namespace std;

void cpp11();
void smart_pointer();

class Useless {
public:
    Useless();
    // explicit Useless(int k);
    Useless(int k, char ch);
    Useless(const Useless& u);
    Useless(Useless&& u);   // 移动构造函数
    ~Useless();
    void show_data() const;
    Useless operator+(const Useless& u) const;
private:
    int n;
    char* pc;
    static int ct;
    void show_object() const;

};

int Useless::ct = 0;
Useless::Useless() {
    ++ct;
    n = 0;
    pc = nullptr;
    cout << "default constructor called, number of objects: " << ct << endl;
}

Useless::Useless(int k, char ch):n(k) {
    ++ct;
    cout << "init constructor called, number of object: " << ct << endl;
    pc = new char[n];
    show_object();
}

Useless::Useless(const Useless& u):n(u.n) {
    ++ct;
    cout << "copy constructor called, number of object: " << ct << endl;
    pc = new char[n];
    for (int i = 0; i < n; ++i) 
        pc[i] = u.pc[i];
    show_object();
}

Useless::Useless(Useless&& u):n(u.n) {
    ++ct;
    cout << "move constructor called, number of object: " << ct << endl;
    pc = u.pc;
    u.pc = nullptr;
    u.n = 0;
    show_object();
}

Useless::~Useless() {
    cout << "destructor called; objects left: " << --ct << endl;
    show_object();
    delete [] pc;
}

void Useless::show_object() const {
    cout << "n: " << n << " , data addr: " <<  (void *)pc << endl;
}

void Useless::show_data() const {
    cout << "data: ";
    for (int i = 0; i < n; ++i) {
        cout << pc[i] << " ";
    }
    cout << endl;
}

Useless Useless::operator+(const Useless& u) const {
    assert(u.n == n);
    Useless t = Useless(n, '0');
    for (int i = 0; i < n; ++i) {
        t.pc[i] = pc[i] + u.pc[i];
    }

    return t;
}

int main() {
    smart_pointer();
    cpp11();
    Useless one(10, 1);
    Useless two(one);
    two.show_data();
    Useless three(10, 64);
    Useless four (one + three);
    four.show_data();

    return 0;
}

void smart_pointer() {
    unique_ptr<string> p1(new string("auto"));
    cout << *p1 << endl;
    unique_ptr<string> p2;
    p2 = std::move(p1); // 这里必须加move，否则编译错误，相比auto_ptr，增加了安全性
    cout << *p2 << endl;    

    // shared_ptr采用引用计数的方式管理所指向的对象
    shared_ptr<string> p3 = make_shared<string>("shared pointer.");
    cout << *p3 << p3.use_count() << endl;
    {
        auto p4 = p3;
        cout << *p4 << p4.use_count() << endl;
    }
    cout << *p3 << p3.use_count() << endl;

    weak_ptr<string> p5(p3);
    auto p6 = p5.lock();
    cout << *p6 << p5.use_count() << endl;
}

void cpp11() {
    cout << "long long:" << sizeof(long long) << endl;
    cout << "char16_t: " << sizeof(char16_t) << endl;
    cout << "char32_t: " << sizeof(char32_t) << endl;

    std::vector<int> v1(6);
    for (auto &x : v1) {
        x = std::rand();
    }

    cout << "v1: ";
    for (auto i : v1) {
        cout << i << " ";
    }
    cout << endl;

}