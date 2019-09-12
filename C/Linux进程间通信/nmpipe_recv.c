/* recv process*/
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>

int main () {
    if (-1 == mkfifo("comm", 0666)) {
        if (EEXIST != errno) {
            perror("mkfifo failure.");
            exit(EXIT_FAILURE);
        }
    }

    int fd = open("comm", O_RDONLY);
	if (fd < 0) {
        perror("open pipe failure.");
    }
    char* buf = (char*)malloc(80);
    bzero(buf, 80);
	read(fd, buf, 80);
	printf("recv from other process: %s\n", buf);
    close(fd);
	free(buf);

    return 0;
}