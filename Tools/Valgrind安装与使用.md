[Valgrind](http://valgrind.org/)是一款用于内存调试、内存泄漏检测以及性能分析的软件开发工具。Valgrind工具组提供了一套调试与分析错误的工具包，能够帮助你的程序工作的更加准确，更加快速。


### Valgrind的主要功能
Valgrind工具包包含多个工具，如Memcheck、Cachegrind、Helgrind、Callgrind、Massif。下面分别介绍个工具的作用：

#### Memcheck 
Memcheck工具主要检查下面的程序错误：

- 使用未初始化的内存 (Use of uninitialised memory)
- 使用已经释放了的内存 (Reading/writingmemory after it has been free’d)
- 使用超过 malloc分配的内存空间(Reading/writing off the end of malloc’d blocks)
- 对堆栈的非法访问 (Reading/writinginappropriate areas on the stack)
- 申请的空间是否有释放 (Memory leaks –where pointers to malloc’d blocks are lost forever)
- malloc/free/new/delete申请和释放内存的匹配(Mismatched use of malloc/new/new [] vs free/delete/delete [])
- src和dst的重叠(Overlapping src and dst pointers in memcpy() and related functions)

#### Callgrind
Callgrind收集程序运行时的一些数据，函数调用关系等信息，还可以有选择地进行cache模拟。在运行结束时，它会把分析数据写入一个文件。callgrind_annotate可以把这个文件的内容转化成可读的形式。

#### Cachegrind
它模拟 CPU中的一级缓存I1、D1和L2二级缓存，能够精确地指出程序中cache的丢失和命中。如果需要，它还能够为我们提供cache丢失次数，内存引用次数，以及每行代码，每个函数，每个模块，整个程序产生的指令数。这对优化程序有很大的帮助。

#### Helgrind
它主要用来检查多线程程序中出现的竞争问题。Helgrind寻找内存中被多个线程访问，而又没有一贯加锁的区域，这些区域往往是线程之间失去同步的地方，而且会导致难以发掘的错误。Helgrind实现了名为” Eraser” 的竞争检测算法，并做了进一步改进，减少了报告错误的次数。

#### Massif
堆栈分析器，它能测量程序在堆栈中使用了多少内存，告诉我们堆块，堆管理块和栈的大小。Massif能帮助我们减少内存的使用，在带有虚拟内存的现代系统中，它还能够加速我们程序的运行，减少程序停留在交换区中的几率。



### 安装

```shell
wget http://valgrind.org/downloads/valgrind-3.13.0.tar.bz2
tar -xjvf valgrind-3.13.0.tar.bz2 
cd valgrind-3.13.0/

./autogen.sh
./configure
make
sudo make install
```

#### 用法

Valgrind命令的格式如下：
```
valgrind [valgrind-options] your-prog [your-prog options]
```

>-tool=<name> 最常用的选项。运行 Valgrind中名为toolname的工具。默认Memcheck。

>-h 显示帮助信息。


更多用法可学习参考[Valgrind User Manual](http://valgrind.org/docs/manual/manual.html)。


#### 用法示例——在Memcheck下运行你的程序

如果你的程序按照以下方式运行:
```
myprog arg1 arg2
```
请使用下述命令来执行内存检查:
```
valgrind --leak-check=yes myprog arg1 arg2
```
Memcheck是默认的工具，开启--leak-check选项会启动内存泄露检查。你的程序会比正常运行慢很多(大概20到30倍)，并且会使用更多的内存。Memcheck会记录检测到的内存错误和内存泄露信息。



```c
#include <stdlib.h>

void f(void) {
   int* x = malloc(10 * sizeof(int));
   x[10] = 0;        // problem 1: heap block overrun
}                    // problem 2: memory leak -- x not freed

int main(void) {
   f();
   return 0;
}
```
上面是一段有内存错误的程序，对它进行测试。
```
gcc -g -o a a.c   
valgrind --leak-check=yes ./a
```
输出如下信息：
```
==11298== Memcheck, a memory error detector  #11298进程号
==11298== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==11298== Using Valgrind-3.13.0 and LibVEX; rerun with -h for copyright info
==11298== Command: ./a
==11298== 
==11298== Invalid write of size 4   #检测出problem1，提示非法写操作
==11298==    at 0x400544: f (a.c:6)
==11298==    by 0x400555: main (a.c:11)
==11298==  Address 0x5204068 is 0 bytes after a block of size 40 alloc'd
==11298==    at 0x4C2DBF6: malloc (vg_replace_malloc.c:299)
==11298==    by 0x400537: f (a.c:5)
==11298==    by 0x400555: main (a.c:11)
==11298== 
==11298== 
==11298== HEAP SUMMARY:    #检测出problem2
==11298==     in use at exit: 40 bytes in 1 blocks
==11298==   total heap usage: 1 allocs, 0 frees, 40 bytes allocated
==11298== 
==11298== 40 bytes in 1 blocks are definitely lost in loss record 1 of 1
==11298==    at 0x4C2DBF6: malloc (vg_replace_malloc.c:299)
==11298==    by 0x400537: f (a.c:5)
==11298==    by 0x400555: main (a.c:11)
==11298== 
==11298== LEAK SUMMARY:   #内存泄露的信息
==11298==    definitely lost: 40 bytes in 1 blocks #肯定丢失的部分，这种报告必须处理
==11298==    indirectly lost: 0 bytes in 0 blocks
==11298==      possibly lost: 0 bytes in 0 blocks
==11298==    still reachable: 0 bytes in 0 blocks
==11298==         suppressed: 0 bytes in 0 blocks
==11298== 
==11298== For counts of detected and suppressed errors, rerun with: -v
==11298== ERROR SUMMARY: 2 errors from 2 contexts (suppressed: 0 from 0)  #可以看到上面程序的2个内存错误都检测到了。
```