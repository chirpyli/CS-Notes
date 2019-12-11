Dr. Memory 是一个开源免费的内存检测工具，它能够及时发现内存相关的编程错误，比如未初始化访问、内存非法访问以及内存泄露等。与 Valgrind 类似，可以直接检查已经编译好的可执行文件。用户不用改写被检查程序的源代码，也无须重新链接第三方库文件，使用起来非常方便。


### 安装
在Linux下安装参考[Installing on Linux](http://drmemory.org/docs/page_install_linux.html).

### 使用示例

#### 示例[程序1](./drmemroy1.c)：
```c
#include <stdio.h>

int main(){
	// 对空指针指向的内存区域写，会发生段错误
	int *null_ptr = NULL;
	*null_ptr = 10;

	return 0;
}
```
编译程序，编译时添加```-g```，以产生调试信息。使用```drmemory -- <your program> args```进行检测。
```
~~Dr.M~~ Dr. Memory version 2.0.1
~~Dr.M~~ 
~~Dr.M~~ Error #1: UNADDRESSABLE ACCESS: writing 0x0000000000000000-0x0000000000000004 4 byte(s)
~~Dr.M~~ # 0 main               [/home/sl/Works/github/CS-Notes/Tools/drmemory1.cpp:6]
~~Dr.M~~ Note: @0:00:00.962 in thread 20565
~~Dr.M~~ Note: instruction: mov    $0x0000000a -> (%rax)
~~Dr.M~~ 
~~Dr.M~~ ERRORS FOUND:
~~Dr.M~~       1 unique,     1 total unaddressable access(es)
~~Dr.M~~       0 unique,     0 total uninitialized access(es)
~~Dr.M~~       0 unique,     0 total invalid heap argument(s)
~~Dr.M~~       0 unique,     0 total warning(s)
~~Dr.M~~       0 unique,     0 total,      0 byte(s) of leak(s)
~~Dr.M~~       0 unique,     0 total,      0 byte(s) of possible leak(s)
~~Dr.M~~ ERRORS IGNORED:
~~Dr.M~~      13 unique,    16 total,   6383 byte(s) of still-reachable allocation(s)
~~Dr.M~~          (re-run with "-show_reachable" for details)
~~Dr.M~~ Details: /home/sl/tools/DrMemory-Linux-2.0.1-2/drmemory/logs/DrMemory-a.out.20565.000/results.txt
Segmentation fault (core dumped)
```


#### 示例[程序2](./drmemory2.c)：
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
运行后结果如下：
```
~~Dr.M~~ Dr. Memory version 2.0.1
~~Dr.M~~ 
~~Dr.M~~ Error #1: UNADDRESSABLE ACCESS beyond heap bounds: writing 0x0000000000604d08-0x0000000000604d0c 4 byte(s)
~~Dr.M~~ # 0 f                  [/home/sl/Works/github/CS-Notes/Tools/drmemory2.c:5]
~~Dr.M~~ # 1 main               [/home/sl/Works/github/CS-Notes/Tools/drmemory2.c:9]
~~Dr.M~~ Note: @0:00:00.496 in thread 20926
~~Dr.M~~ Note: next higher malloc: 0x0000000000604d40-0x0000000000605d40
~~Dr.M~~ Note: refers to 0 byte(s) beyond last valid byte in prior malloc
~~Dr.M~~ Note: prev lower malloc:  0x0000000000604ce0-0x0000000000604d08
~~Dr.M~~ Note: instruction: mov    $0x00000000 -> (%rax)
~~Dr.M~~ 
~~Dr.M~~ Error #2: LEAK 40 direct bytes 0x0000000000604ce0-0x0000000000604d08 + 0 indirect bytes
~~Dr.M~~ # 0 replace_malloc               [/drmemory_package/common/alloc_replace.c:2577]
~~Dr.M~~ # 1 f                            [/home/sl/Works/github/CS-Notes/Tools/drmemory2.c:4]
~~Dr.M~~ # 2 main                         [/home/sl/Works/github/CS-Notes/Tools/drmemory2.c:9]
~~Dr.M~~ 
~~Dr.M~~ ERRORS FOUND:
~~Dr.M~~       1 unique,     1 total unaddressable access(es)
~~Dr.M~~       0 unique,     0 total uninitialized access(es)
~~Dr.M~~       0 unique,     0 total invalid heap argument(s)
~~Dr.M~~       0 unique,     0 total warning(s)
~~Dr.M~~       1 unique,     1 total,     40 byte(s) of leak(s)
~~Dr.M~~       0 unique,     0 total,      0 byte(s) of possible leak(s)
~~Dr.M~~ ERRORS IGNORED:
~~Dr.M~~      13 unique,    16 total,   6383 byte(s) of still-reachable allocation(s)
~~Dr.M~~          (re-run with "-show_reachable" for details)
~~Dr.M~~ Details: /home/sl/tools/DrMemory-Linux-2.0.1-2/drmemory/logs/DrMemory-a.out.20926.000/results.txt
```
可以看到2个内存错误都检测到了。

更多选项可参考```drmemory -help```。