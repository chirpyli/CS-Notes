/* send process*/
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

    int fd = open("comm", O_WRONLY);
    if (fd < 0) {
        perror("open pipe failure.");
    }
    
    char* msg = "process of send.";
    write(fd, msg, strlen(msg));
    close(fd);

    return 0;
}