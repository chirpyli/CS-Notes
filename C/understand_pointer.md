在阅读《深入理解计算机系统》时，读到理解指针这一节，写的非常好，这里对其进行学习总结。下面的内容部分摘自《深入理解计算机系统》，部分是自己的学习总结。

### 理解指针
指针是C语言的一个核心特色，以一种统一的方式，对不同数据结构中的元素产生引用。下面重点介绍一些指针和它们映射到机器代码的关键原则。
- 每个指针都对应一个类型。这个类型表明该指针指向的是哪一类对象。以下面的指针声明为例：`int *ip;`，变量`ip`是一个指向`int`类型对象的指针，通常如果对象类型为`T`，那么指针的类型为`T*`。特殊的`void *`类型代表通用指针。比如说，`void *malloc(size_t size)`函数返回一个通用指针，然后通过强制类型转换或者赋值操作那样的隐私强制类型转换，将它转换成一个有类型的指针。**指针类型不是机器代码中的一部分，是C语言提供的一种抽象，帮助程序员避免寻址错误。**
> 可以这么理解，指针的具体地址值相当于是一个寻址开始的地址，什么时候结束是由指针类型的`size`大小决定何时结束，比如`char*`，从起始地址读一个字节就结束，`int*`，从起始地址读4个字节就结束。下面我们写一段C代码，验证我们对指针的理解。
```c
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

typedef enum NodeType {
    INT,
    CHAR
} NodeType;

typedef struct Node {
    NodeType type;
} Node;

typedef struct IntNode {
    NodeType type;
    int value;
} IntNode;

typedef struct CharNode {
    NodeType type;
    char value;
} CharNode;

char* printNodeType(NodeType type) {

    switch (type)
    {
    case INT:
        return "INT";    
    case CHAR:
        return "CHAR";
    default:
        return "UNKNOW";
    }

    return NULL;
}

// 重点理解这个函数
void printNode(Node* n) {
    assert(n != NULL);

    switch (n->type)
    {
    case INT:
        {
            IntNode* in = (IntNode*)n;
            printf("type:%s value:%d \n", printNodeType(in->type), in->value);
        }
        break;
    case CHAR:
        {
            CharNode* cn = (CharNode*)n;
            printf("type:%s value:%d \n", printNodeType(cn->type), cn->value);
        }
        break;
    default:
        printf("unknow node \n");
        break;
    }
}

int main() {
    IntNode *a = (IntNode*)malloc(sizeof(IntNode));
    a->type = INT;
    a->value = 1024;
    printNode(a);

    CharNode *b = (CharNode*)malloc(sizeof(CharNode));
    b->type = CHAR;
    b->value = 'a';
    printNode(b);

    return 0;
}
```
运行结果：
```
type:INT value:1024 
type:CHAR value:97 
```
这个代码可以验证上面的理解。

- 每个指针都有一个值。这个值是某个指定类型的对象的地址。特殊的`NULL(0)`值表示该指针没有指向任何地方。
- 将指针从一种类型强制转换成另一种类型，只改变了它的类型，并不改变它的值。**强制类型转换的一个效果是改变指针运算的伸缩。**
> 从上面的代码示例可以验证上面这一点。
- 指针也可以指向函数。函数指针的值是该函数机器代码表示中第一条指令的地址。