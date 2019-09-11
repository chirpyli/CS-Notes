#include<stdio.h>

char* GetString1() {  
   char p[] = "Hello World";    //指向临时分配的桟空间，当运行至函数体外时，空间将被释放  
   return p;  
}  

char* GetString2() {  
   char *p = "Hello World";     //指向全局常量区  
   return p;  
}  

int main() {  
    printf("GetString1 returns: %s. \n", GetString1());  
    printf("GetString2 returns: %s. \n", GetString2());  
    
    return 0;  
}