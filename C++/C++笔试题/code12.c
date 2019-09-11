#include<string.h>
#include<stdio.h>

int func() {  
    char b[2]={0};  
    strcpy(b,"aaa");  
    printf("%s", b);    
}

int main() {
    func();
}