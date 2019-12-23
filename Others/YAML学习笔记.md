接触yaml是用来做配置文件的，所以首要的是学习yaml的基本编写规则，暂不做太深入的学习。学习yaml，可以参考[wiki](https://en.wikipedia.org/wiki/YAML)，基本各方面都讲到了。下面是摘自wiki的：
>YAML (YAML Ain't Markup Language) is a human-readable data serialization language. It is commonly used for configuration files, but could be used in many applications where data is being stored (e.g. debugging output) or transmitted (e.g. document headers). 


### 语法概要
在yaml里面，结构通过缩进来表示，连续的项目（如：数组元素、集合元素）通过减号“-”来表示，map结构里面的键值对（key/value）用冒号“:”来分割。yaml也有用来描述好几行相同结构数据的缩写语法，数组用“[]”包括起来，hash用“{}”来包括。

### 基本规则
YAML有以下基本规则： 
1. 大小写敏感 
2. 使用缩进表示层级关系 
3. 禁止使用tab缩进，只能使用空格键 
4. 缩进长度没有限制，只要元素对齐就表示这些元素属于一个层级。 
5. 使用#表示注释 
6. 字符串可以不用引号标注
7. 用on、1、true来表示true；off、0、false来表示false

### 三种数据结构
#### 1. map，散列表 
使用冒号（：）表示键值对，同一缩进的所有键值对属于一个map，示例：
（注意，：后面要有一个空格）
```yaml
# YAML表示
age : 12
name : huang

# 对应的Json表示
{'age':12,'name':'huang'}
```
也可以将一个map写在一行：
```yaml
# YAML表示
{age:12,name:huang}

# 对应的Json表示
{'age':12,'name':'huang'}
```

#### 2. list，数组 
使用连字符（-）表示：
```yaml
# YAML表示
- a
- b
- 12

# 对应Json表示
['a','b',12]
```
也可以写在一行：
```yaml
# YAML表示
[a,b,c]

# 对应Json表示
[ 'a', 'b', 'c' ]
```
#### 3. scalar，纯量 
数据最小的单位，不可以再分割。

### 数据结构嵌套
map和list的元素可以是另一个map或者list或者是纯量。由此出现4种常见的数据嵌套： 
#### 1. map嵌套map
```yaml
# YAML表示
websites:
 YAML: yaml.org 
 Ruby: ruby-lang.org 
 Python: python.org 
 Perl: use.perl.org 

# 对应Json表示
{ websites: 
   { YAML: 'yaml.org',
     Ruby: 'ruby-lang.org',
     Python: 'python.org',
     Perl: 'use.perl.org' } }
```

#### 2. map嵌套list
```yaml
# YAML表示
languages:
 - Ruby
 - Perl
 - Python 
 - c

# 对应Json表示
{ languages: [ 'Ruby', 'Perl', 'Python', 'c' ] }
```

#### 3. list嵌套list
```yaml
# YAML表示
-
  - Ruby
  - Perl
  - Python 
- 
  - c
  - c++
  - java

# 对应Json表示
[ [ 'Ruby', 'Perl', 'Python' ], [ 'c', 'c++', 'java' ] ]
```
除此以外，还可以如下表示该结构
```yaml
# 方法2
- - Ruby
  - Perl
  - Python 
- - c
  - c++
  - java

# 方法3
- [Ruby,Perl,Python]
- [c,c++,java]
```

#### 4. list嵌套map
```yaml
# YAML表示
-
  id: 1
  name: huang
-
  id: 2
  name: liao

# 对应Json表示
[ { id: 1, name: 'huang' }, { id: 2, name: 'liao' } ]
```

### 其他
#### 空值的表示
YAML有几种空值的表现方式，分别是 “null”、”~” 和 不填。
```yaml
# YAML表示
text1: null
text2: ~
text3: 

# 对应Json表示
{text1: null, text2: null, text3: null}
```

---
>需要学习更多的话，可以参考：[YAML Ain’t Markup Language (YAML™) Version 1.2](http://www.yaml.org/spec/1.2/spec.html).