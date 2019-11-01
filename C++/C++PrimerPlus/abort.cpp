// 异常处理
#include<iostream>
using namespace std;

double hmean(double a, double b);

int main() {
    double x, y, z;
    cout << "enter two numbers: ";
    while (cin >> x >> y) {
        try {
            z = hmean(x, y);
        }
        catch (const char* s)
        {
            cout << s << endl;
            cout << "Enter a new pair of numbers:";
            continue;
        }
        cout << "result is :" << z << endl;
    }

    return 0;
}

double hmean(double a, double b) {
    if (a == -b) {
        //抛出一个异常，这里是字符串，也可以是一个对象，相应的catch里捕获的也是一个对象
        throw "bad hmean() arguments: a = -b not allowed."; 
    }
    return 2.0 * a * b / (a + b);

}