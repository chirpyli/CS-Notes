对于数据库开发者，必须要熟悉SQL，可以从《SQL必知必会》入门，讲解了SQL较为基础的内容，快速对SQL建立全貌的认识。另外通过这本书，你可以了解到不同数据库语法的不同（不同的地方不多，但需要注意）。注意，这里是阅读本书所做的笔记，所以不像写文章那样较有逻辑性。可能里面的内容是较为零散的。

主要涉及到的数据库：
- IBM DB2
- Oracle
- MySQL、MariaDB
- PostgreSQL
- SQL Server
- SQLite

#### 第1课 了解SQL
SQL结构化查询语言（Structure Query Language）。SQL是一种专门用来与数据库沟通的语言，设计SQL的目的是很好地完成一项任务——提供一种从数据库中读写数据的简单有效的方法。设想一下如果没有SQL，会出现什么情况呢？你要如何操作数据库呢，比如你要查询一个数据，你要怎么描述才能让数据库知道你要查的是什么呢？如何描述才能让数据库精准的知道你要对数据库做如何的操作呢？SQL就是解决这个问题的。就像一门编程语言，是把现实世界中的事物用编程语言抽象表达为计算机硬件能理解的指令。中间通过编译器实现，所以SQL是数据库能够理解的语言，用于方便用户抽象表达如何操纵数据。


#### 第2课 检索数据
1. 使用SELECT检索数据时，如果不是特别需要，最好不要用"*"通配符，检索不需要的列通常会降低检索速度和应用程序的性能。

2. 限制返回结果，返回一定数量的行或者第一行，不同数据库用法不同:
    - SQL Server:  用TOP关键字
    - Oracle:        基于ROWNUM行计数器计算行
    - PostgreSQL、MySQL、MariaDB: LIMIT关键字
```sql
SELECT TOP 5 prod_name FROM Products;       -- SQL Server 只检索前5行
SELECT prod_name FROM Products WHERE ROWNUM <=5;    -- Oracle  
SELECT prod_name FROM Products LIMIT 5;     -- PostgreSQL、MySQL
SELECT prod_name FROM Products LIMIT 5 OFFSET 5;    -- PostgreSQL、MySQL 返回从第5行起的5行数据。   LIMIT带的OFFSET指定从哪儿开始
```
> 在自研数据库兼容Oracle中，一般ROWNUM都会列为兼容项之一。

#### 第3课 排序检索数据
0. 使用SELECT语句的ORDER BY子句，根据需要排序检索词的数据。
```sql
postgres=# select * from t1 order by b;
 a | b  
---+----
 1 |  1
 6 |  1
 3 |  1
 4 |  2
 5 |  3
 2 |  3
 7 | 10
(7 rows)
```

1. 要注意ORDER BY子句的位置，指定一条ORDER BY子句时，应该保证它是SELECT语句中最后一条子句。如果它不是最后的子句，将会出错。
```sql
postgres=# select * from t1 order by b where a=1;
ERROR:  syntax error at or near "where"
LINE 1: select * from t1 order by b where a=1;
```

2. 如果需要按不止一个列进行数据排序，需要注意按多个列排序的规则。

3. 数据排序不限于升序排序，如果用降序排序，需指定DESC关键字。

#### 第4课 过滤数据
> 使用SELECT语句的WHERE子句指定搜索条件

1. 需要注意WHERE子句操作符，不等于有两种表示方法`!=`、`<>`。

#### 第5课 高级数据过滤
1. 在WHERE子句中可使用AND、OR、IN、NOT操作符进行高级数据过滤，尤其是IN。IN操作符用来指定条件范围。
```sql
postgres=# select * from t1 where a in (1,2,3);
 a | b 
---+---
 1 | 1
 2 | 3
 3 | 1
(3 rows)
```

#### 第6课 用通配符进行过滤
0. 可使用LIKE操作符进行统配搜索，以实现复杂过滤。
> 通配符： 用来匹配值的一部分的特殊字符。
```sql
postgres=# select * from t1 where c like 'han%';
 a | b |    c     
---+---+----------
 8 | 1 | hangzhou
 9 | 2 | hangkand
(2 rows)
```

1. 常用通配符
    - `%`表示任何字符出现任意次数。
    - `_`只匹配单个字符
    - `[]`用来指定一个字符集，它必须匹配指定位置（通配符的位置）的一个字符。

#### 第7课 创建计算字段

1. 为什么要有计算字段这个功能呢？有啥用呢？ 

    有的时候，存储在数据库表中的数据可能不是应用程序所需要的格式，比如下面的两个例子：
    - 需要显示公司名，同时显示公司地址，但这两个信息存储在不同的表列中。
    - 需要根据表数据进行诸如总数、平均数的计算。
    
    在上面的两个例子中，存储在表中的数据都不是应用程序所需要的，我们需要直接从数据库中检索出转换、计算或格式化过的数据，而不是检索出数据，然后再在客户端应用程序中重新格式化。这个时候就需要计算字段，计算字段并不实际存在于数据库表中，是在运行时在SELECT语句内创建的。

2. 拼接字段
在SQL中可使用一个特殊的操作符来拼接两个列。不同数据库使用的操作符可能不同：
    - SQL Server: 使用"+"号
    - MySQL、Oracle、PostgreSQL：使用"||"。
```sql
postgres=# select c || '=' || a from t1 where b < 3;
  ?column?  
------------
 my data=1
 my data=3
 my data=4
 my data=6
 hangzhou=8
 hangkand=9
(6 rows)
```
上面拼接后的输出的新计算列，名字是什么呢，目前是一个未命名的列`?column?`，不能用于客户端应用中，因为客户端没法引用它，为了解决这个问题，SQL支持别名，别名用AS关键字赋予，是一个字段或值的替换名。
```sql
postgres=# select c || '=' || a as kv from t1 where b < 3;
     kv     
------------
 my data=1
 my data=3
 my data=4
 my data=6
 hangzhou=8
 hangkand=9
(6 rows)
```

3. 执行算术计算
计算字段的另一常见用途是对检索出的数据进行算术计算。
```sql
postgres=# select a,b,a*b as ac from t1 where c='my data';
 a | b  | ac 
---+----+----
 1 |  1 |  1
 2 |  3 |  6
 3 |  1 |  3
 4 |  2 |  8
 5 |  3 | 15
 6 |  1 |  6
 7 | 10 | 70
(7 rows)
```

#### 第8课 使用函数处理数据
不同数据库对函数的支持差异较大，使用的时候需要注意，尤其是涉及到日期时间等函数。这也导致了自研数据库开发兼容Oracle,MySQL中一块比较大的工作。
```sql
postgres=# select current_date;
 current_date 
--------------
 2021-04-29
(1 row)
```

#### 第9课 汇总数据
SQL聚集函数：
- AVG()
- COUNT()
- MAX()
- MIN()
- SUM()
见名知意，举个例子：
```sql
postgres=# select count(*) from t1;
 count 
-------
     9
(1 row)
```

#### 第10课 分组数据
0. 如何分组数据，以便汇总表内容的子集，涉及到两个SELECT子句：GROUP BY子句和HAVING子句。                         
```sql
-- 使用分组将数据分为过个逻辑组，对每个组进行聚集计算
postgres=# select c,count(*) from t1 group by c;
    c     | count 
----------+-------
 hangkand |     1
 my data  |     7
 hangzhou |     1
(3 rows)
```
1. GROUP BY子句需要注意GROUP BY子句必须出现在WHERE子句后，ORDER BY子句前。
```sql
postgres=# select c,count(*) from t1 where a>1 group by c order by c;
    c     | count 
----------+-------
 hangkand |     1
 hangzhou |     1
 my data  |     6
(3 rows)
```
2. 除了能有GROUP BY分组数据外，有的时候还需要对分组数据进行过滤，就需要HAVING子句。
```sql
-- 过滤掉数量不大于1的分组
postgres=# select c,count(*) from t1 where a>1 group by c having count(*) >1;
    c    | count 
---------+-------
 my data |     6
(1 row)
```

#### 第11课 使用子查询
子查询:嵌套在其他查询中的查询
```sql
postgres=# select a from t1 where b in (select a from t2 where a < 3);
 a 
---
 2
(1 row)
```

#### 第12课 联结表
