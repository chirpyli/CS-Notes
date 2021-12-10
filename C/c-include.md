这里学习一下#include。`#include`是C语言预处理命令的一种。`#include` 的处理过程很简单，就是将头文件的内容插入到该命令所在的位置，从而把头文件和当前源文件连接成一个源文件，这与复制粘贴的效果相同。

一般的我们都非常熟悉通过`#include "*.h"`引入头文件，但其实，`#include`也可以有别的用法，比如下面的用法：
在`main.c`源文件中，增加`#include "print.c"`，等同于将`print.c`中的代码复制到`main.c`的指定位置中。

```c++
/* 
 *#include study examples 
 * main.c
 */
#include<stdio.h>

#include "print.c"  

int main() {
    print("test #include");

    return 0;
}

```
`print.c`源代码如下：
```c++
/* print.c */
void print(const char *str) {
    printf("%s \n", str);
}
```

我们通过`gcc -E main.c`只进行预处理看一下处理后的结果：
```c++
# 1 "main.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 1 "<command-line>" 2
# 1 "main.c"

# 1 "/usr/include/stdio.h" 1 3 4
# 27 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/features.h" 1 3 4
# 375 "/usr/include/features.h" 3 4
# 1 "/usr/include/sys/cdefs.h" 1 3 4
# 392 "/usr/include/sys/cdefs.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 393 "/usr/include/sys/cdefs.h" 2 3 4
# 376 "/usr/include/features.h" 2 3 4
# 399 "/usr/include/features.h" 3 4
# 1 "/usr/include/gnu/stubs.h" 1 3 4
# 10 "/usr/include/gnu/stubs.h" 3 4
# 1 "/usr/include/gnu/stubs-64.h" 1 3 4
# 11 "/usr/include/gnu/stubs.h" 2 3 4
# 400 "/usr/include/features.h" 2 3 4
# 28 "/usr/include/stdio.h" 2 3 4

// 中间代码太多了，忽略掉，只保留关键部分。

# 943 "/usr/include/stdio.h" 3 4

# 3 "main.c" 2

# 1 "print.c" 1

// 可以看到被#include的文件内容复制到了#include "print.c"的位置。
void print(const char *str) {
    printf("%s \n", str);
}
# 5 "main.c" 2

int main() {
    print("test #include");

    return 0;
}
```
