// Linux进程间通信——管道
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<string.h>

int main () {
    char* msg;
    char buf[20];
    int pipe_filed[2];
    pipe(pipe_filed);
    pid_t pid = fork();
    if(pid < 0) {
        perror("fork errir.");
        exit(1);
    } else if (0 == pid) {
        msg = "child";
        sleep(3);
        write(pipe_filed[1], msg, sizeof(msg));
        printf("child process send: %s\n", msg);
    } else {
        read(pipe_filed[0], buf, sizeof(buf));
        printf("parent process recv: %s\n", buf);

        int status;
        wait(&status);
        if (WIFEXITED(status))
            printf("Child exited with code %d\n", WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child terminated abnormally, signal %d\n", WTERMSIG(status));
    }

    return 0;
}