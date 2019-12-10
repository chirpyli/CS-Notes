Dr. Memory 是一个开源免费的内存检测工具，它能够及时发现内存相关的编程错误，比如未初始化访问、内存非法访问以及内存泄露等。与 Valgrind 类似，可以直接检查已经编译好的可执行文件。用户不用改写被检查程序的源代码，也无须重新链接第三方库文件，使用起来非常方便。


### 安装
在Linux下安装参考[Installing on Linux](http://drmemory.org/docs/page_install_linux.html).

### 使用示例

#### 示例程序1：
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
![在这里插入图片描述](https://img-blog.csdn.net/20180918100611117?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)


#### 示例程序2：
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
![在这里插入图片描述](https://img-blog.csdn.net/20180918102323276?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
可以看到2个内存错误都检测到了。

更多选项可参考```drmemory -help```。