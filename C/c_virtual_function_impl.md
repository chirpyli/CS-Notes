### 用C模拟实现类虚函数
这里并不是用C去实现C++中的虚函数，而是在C中类虚函数的实现方法。代码如下：

```c++
/* use c impl virtual function */

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

// 相当于C++中的虚函数表
typedef struct
{
    void (*myadd)(void *p);
    void (*myminus)(void *p);
} vtable;

// 相当于C++中的纯虚函数
typedef struct Parent
{
    vtable *vptr;
} Parent;


// 继承自Parent，vptr相当于虚函数表指针
typedef struct A
{
    vtable* vptr;   
    int a;
} A;

// 用C的方式模拟C++中类方法的实现，第一个参数相当于this指针，可以有多个参数，这里省略
void adda(void *p)
{
    A *mp = (A*) p;
    mp->a += 1;
    printf("A plus one = %d \n", mp->a);
}

void minusa(void *p)
{
    A *mp = (A*) p;
    mp->a -= 1;
    printf("A minus one = %d \n", mp->a);
}

// 继承自Parent，vptr相当于虚函数表指针
typedef struct B
{
    vtable* vptr;
    int b;
} B;

void addb(void *p)
{
    B *mp = (B*) p;
    mp->b += 1;
    printf("B plus one = %d \n", mp->b);
}

void minusb(void *p)
{
    B *mp = (B*) p;
    mp->b -= 1;
    printf("B minus one = %d \n", mp->b);
}

void main()
{
    vtable va = {&adda, &minusa};   // 在C++中构造阶段负责指向不同的虚表
    vtable vb = {&addb, &minusb};
    A a = {&va, 0};     // 这里人为指定指向不同的虚表
    B b = {&vb, 100};

    Parent *p = (Parent*)&a;
    p->vptr->myadd(p);      // 执行adda的动作
    p = (Parent*)&b;
    p->vptr->myadd(p);      // 执行addb的动作
}
```