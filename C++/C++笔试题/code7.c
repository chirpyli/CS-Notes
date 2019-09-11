#include<stdio.h>

int main() {  
    char str1[] = "hello world";//桟空间  
    char str2[] = "hello world";//桟空间，临时分配，地址不同  
    char* str3 = "hello world";//常量区  
    char* str4 = "hello world";//指向同一块全局常量区  
    if(str1 == str2)  
       printf("str1 and str2 are same.\n");  
    else  
       printf("str1 and str2 are not same.\n");  
    if(str3 == str4)  
       printf("str3 and str4 are same.\n");  
    else  
        printf("str3 and str4 are not same.\n");  
       
    return 0;  
}  