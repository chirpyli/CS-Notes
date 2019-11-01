##### 流程控制
与其他语言类似，略。

##### 条件测试
命令`test`或`[`可以测试一个条件是否成立，如果测试结果为真，则该命令的`Exit Status`为0，如果测试结果为假，则命令的`Exit Status`为1。
>传给命令的各参数之间应该用空格隔开，比如，$VAR、-gt、3、]是[命令的四个参数，它们之间必须用空格隔开。

常见的测试命令如下表所示：
字符串比较	|结果
|--|--|
string1 = string2	|如果两个字符串相同，结果就为真
string1 != string2|	如果两个字符串不同，结果就为真
-n string	|如果字符串不为空，则结果为真
-z string	|如果字符串为一个空串（null），则结果为真

算术比较|	结果
|--|--|
expression1 -eq expression2|	如果两个表达式相等，则结果为真
expression1 -ne expression2	|如果两个表达式不等，则结果为真
expression1 -gt expression2|如果expression1大于expression2，则为真
expression1 -ge expression2|如果expression1大于等于expression2,则为真
expression1 -lt expression2|如果expression1小于expression2，则为真
expression1 -le expression2|如果expression1小于等于expression2，则为真
!expression|	表达式为假，则结果就为真；反之亦然


文件条件测试|	结果
|--|--|
-d file|	如果文件是一个目录，则为真
-f file	|如果文件是一个普通文件，则为真；也可以用来测试文件是否存在
-r file	|如果文件可读，则结果为真
-s file	|如果文件大小不为0，则结果为真
-w file	|如果文件可写，则结果为真
-x file	|如果文件可执行，则结果为真


##### 函数
Shell函数没有参数列表并不表示不能传参数，事实上，函数就像是迷你脚本，调用函数时可以传任意个参数，在函数内同样是用`$0、$1、$2`等变量来提取参数，函数中的位置参数相当于函数的局部变量，改变这些变量并不会影响函数外面的`$0、$1、$2`等变量。函数中可以用`return`命令返回，如果`return`后面跟一个数字则表示函数的`Exit Status`。

代码示例：
```Shell
#!/bin/bash

main(){
    VAR=$#
    echo 'main args num='$VAR
    for ARG in $@;
    do echo $ARG;
    done
    echo '---------'
}

#打印大于3的参数，只是示例，没有进行参数合法性检查
greate_num(){
    NUM=0
    for ARG in $@;
        do
            if [ $ARG -gt 3 ]; then
                echo $ARG
                NUM=$(($NUM+1))
            fi
        done
    return $NUM
}

main $@

greate_num 1 2 3 4 5 6
echo 'greate_num result='$?
```

输出结果：
```
sl@Li:~/Works/study/shell$ ./script.sh 1 2
main args num=2
1
2
---------
4
5
6
greate_num result=3
```

##### 位置参数和特殊变量
有很多特殊变量是被Shell自动赋值的，如下表所示：
|特殊变量 | 描述|
|--|--|
$0	|当前程序的名称，相当于C语言main函数的argv[0]
$1、$2...	|这些称为位置参数（Positional Parameter），相当于C语言main函数的argv[1]、argv[2]...
$#	|参数个数，相当于C语言main函数的argc - 1，注意这里的#后面不表示注释
$@	|表示参数列表"$1" "$2" ...，例如可以用在for循环中的in后面。
$?	|上一条命令的Exit Status
$$	|当前Shell的进程号

代码示例见上面函数部分的的示例代码。

##### Shell脚本的调试方法
Shell提供了一些用于调试脚本的选项，如下所示：
|选项|描述|
|--|--|
-n|读一遍脚本中的命令但不执行，用于检查脚本中的语法错误
-v|一边执行脚本，一边将执行过的脚本命令打印到标准错误输出
-x|提供跟踪执行信息，将执行的每一条命令和结果依次打印出来

使用这些选项有三种方法：

1. 在命令行提供参数（```sh -x ./script.sh```）
2. 在脚本开头提供参数
```Shell
#! /bin/sh -x
```
3. 在脚本中用```set```命令启用或禁用参数
```Shell
#! /bin/sh
if [ -z "$1" ]; then
  set -x
  echo "ERROR: Insufficient Args."
  exit 1
  set +x
fi
```
`set -x`和`set +x`分别表示启用和禁用`-x`参数，这样可以只对脚本中的某一段进行跟踪调试。