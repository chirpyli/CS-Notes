#include <stdio.h>

int main(){
	// 对空指针指向的内存区域写，会发生段错误
	int *null_ptr = NULL;
	*null_ptr = 10;

	return 0;
}