#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#define CORE_SIZE 500 * 1024 * 1024

int main(){
	struct rlimit rlmt;
	if (getrlimit(RLIMIT_CORE, &rlmt) == -1) {
		return -1;
	}
	printf("Before set rlimit core dump current is:%d, max is:%d\n", (int)rlmt.rlim_cur, (int)rlmt.rlim_max);

	rlmt.rlim_cur = (rlim_t)CORE_SIZE;
	rlmt.rlim_max = (rlim_t)CORE_SIZE;

	if (setrlimit(RLIMIT_CORE, &rlmt) == -1) {
		return -1;
	}

	if (getrlimit(RLIMIT_CORE, &rlmt) == -1) {
		return -1;
	}
	printf("After set rlimit core dump current is:%d, max is:%d\n", (int)rlmt.rlim_cur, (int)rlmt.rlim_max);

	// 对空指针指向的内存区域写，会发生段错误 -->产生core文件
	int *null_ptr = NULL;
	*null_ptr = 10;

	return 0;
}