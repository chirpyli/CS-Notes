在SQL语句中，SELECT语句是必须要重点掌握的语句之一。这里我们学习一下查询语句。

### SELECT语法
我们以PostgreSQL中的SELECT语法为例，其他数据库基本相同，只有少数地方不同（比如limit等，oracle有rownum），使用时请注意。
```sql
SELECT [ ALL | DISTINCT [ ON ( expression [, ...] ) ] ]     -- ALL:查询结果是表的全部 ， DISTINCT：查询结果是不包含重复行的记录集
 [ * | expression [ [ AS ] output_name ] [, ...] ]  -- *：所有列 
 [ FROM from_item [, ...] ]     -- 查询的数据来源
 [ WHERE condition ]            -- where子句，查询条件
 [ GROUP BY grouping_element [, ...] ]      -- group by 子句
 [ HAVING condition ]                       -- having 过滤
 [ WINDOW window_name AS ( window_definition ) [, ...] ]
 [ { UNION | INTERSECT | EXCEPT } [ ALL | DISTINCT ] select ]   -- union组合查询
 [ ORDER BY expression [ ASC | DESC | USING operator ] [ NULLS  -- order by 排序， 
 { FIRST | LAST } ] [, ...] ]
 [ LIMIT { count | ALL } ]      -- limit限制返回结果集的数目
 [ OFFSET start [ ROW | ROWS ] ]
 [ FETCH { FIRST | NEXT } [ count ] { ROW | ROWS } { ONLY | WITH
 TIES } ]
 [ FOR { UPDATE | NO KEY UPDATE | SHARE | KEY SHARE }
 [ OF table_name [, ...] ] [ NOWAIT | SKIP LOCKED ] [...] ]

where from_item can be one of:
 [ ONLY ] table_name [ * ] [ [ AS ] alias [ ( column_alias
 [, ...] ) ] ]
 [ TABLESAMPLE sampling_method ( argument [, ...] )
 [ REPEATABLE ( seed ) ] ]
 [ LATERAL ] ( select ) [ AS ] alias [ ( column_alias
 [, ...] ) ]
 with_query_name [ [ AS ] alias [ ( column_alias [, ...] ) ] ]
 [ LATERAL ] function_name ( [ argument [, ...] ] )
 [ WITH ORDINALITY ] [ [ AS ] alias [ ( column_alias
 [, ...] ) ] ]
 [ LATERAL ] function_name ( [ argument [, ...] ] ) [ AS ] alias
 ( column_definition [, ...] )
 [ LATERAL ] function_name ( [ argument [, ...] ] ) AS
 ( column_definition [, ...] )
 [ LATERAL ] ROWS FROM( function_name ( [ argument [, ...] ] )
 [ AS ( column_definition [, ...] ) ] [, ...] )
 [ WITH ORDINALITY ] [ [ AS ] alias [ ( column_alias
 [, ...] ) ] ]
 from_item [ NATURAL ] join_type from_item [ ON join_condition |
 USING ( join_column [, ...] ) ]

and grouping_element can be one of:
 ( )
 expression
 ( expression [, ...] )
 ROLLUP ( { expression | ( expression [, ...] ) } [, ...] )
 CUBE ( { expression | ( expression [, ...] ) } [, ...] )
 GROUPING SETS ( grouping_element [, ...] )

and with_query is:
 with_query_name [ ( column_name [, ...] ) ] AS [ [ NOT ]
 MATERIALIZED ] ( select | values | insert | update | delete )
TABLE [ ONLY ] table_name [ * ]
```

我们举几个简单查询的例子：
```sql
-- 建表
postgres=# create table student(id int, class int, name text);
CREATE TABLE

-- 中间插入数据略

-- 简单查询
postgres=# select * from student ;
 id | class |     name     
----+-------+--------------
  1 |     1 | houjunnan
  2 |     1 | fangsheng
  3 |     1 | yulianjie
  4 |     1 | yejing
  5 |     1 | txf
  6 |     2 | nanli
  7 |     2 | wuyefei
  8 |     2 | fangjun
  9 |     3 | fangxiaoxiao
 10 |     4 | zhanglihua
(10 rows)

-- 查询统计每个班级学生的数量，并按班级号降序排序
postgres=# select class,count(*) from student group by class having count(*) > 0 order by class desc;
 class | count 
-------+-------
     4 |     1
     3 |     1
     2 |     3
     1 |     5
(4 rows)
```

另外我们可以查看执行计划，我们看一下数据库是如何执行的，最简单的顺序扫描：
```sql
postgres=# explain select * from student;
                         QUERY PLAN                         
------------------------------------------------------------
 Seq Scan on student  (cost=0.00..22.00 rows=1200 width=40)
(1 row)
```
执行计划这不是这次的重点，后面会讲到。

### 子查询
常规的查询，基本直接看上面的SELECT语法就能非常容易的理解，这也是SQL设计的目的，简单易懂易用。这里我们重点看一下子查询，子查询是特殊的条件查询，出现在WHERE子句中的SELECT语句被称为子查询。子查询本质上就是一个完整的SELECT语句。子查询在很多情况下可以进行逻辑优化，逻辑优化阶段可以将子查询上拉，通过JOIN的方式进行优化。具体的优化部分，这里不再细述，后续文章会讲到逻辑优化。我们这次只关注语法使用部分。

###### 带IN的子查询
语法形式为:
```sql
-- 判断expr表达式的值是否在子查询的结果中。
... WHERE expr [NOT] IN (子查询) 
```


举例说明：
```sql
postgres=# select * from school;
 cid | students 
-----+----------
   1 |        5
   2 |        3
   3 |        1
   4 |        1
(4 rows)

-- 带IN的子查询，查询学校里除了1班其他班级的学生情况
postgres=# select * from student where class in (select cid from school where cid > 1);
 id | class |     name     
----+-------+--------------
  6 |     2 | nanli
  7 |     2 | wuyefei
  8 |     2 | fangjun
  9 |     3 | fangxiaoxiao
 10 |     4 | zhanglihua
(5 rows)
-- 执行计划如下：
postgres=# explain select * from student where class in (select cid from school where cid > 1);
                                QUERY PLAN                                 
---------------------------------------------------------------------------
 Hash Join  (cost=44.59..76.41 rows=600 width=40)
   Hash Cond: (student.class = school.cid)      -- 可以看到，数据库对子查询进行了优化，通过Hash Join的方式进行处理。 
   ->  Seq Scan on student  (cost=0.00..22.00 rows=1200 width=40)
   ->  Hash  (cost=42.11..42.11 rows=198 width=4)
         ->  HashAggregate  (cost=40.13..42.11 rows=198 width=4)
               Group Key: school.cid
               ->  Seq Scan on school  (cost=0.00..38.25 rows=753 width=4)
                     Filter: (cid > 1)
(8 rows)


-- 上面等同于下面的语句
postgres=# select * from student where class in (2,3,4);
 id | class |     name     
----+-------+--------------
  6 |     2 | nanli
  7 |     2 | wuyefei
  8 |     2 | fangjun
  9 |     3 | fangxiaoxiao
 10 |     4 | zhanglihua
(5 rows)

postgres=# explain select * from student where class in (2,3,4);
                        QUERY PLAN                        
----------------------------------------------------------
 Seq Scan on student  (cost=0.00..26.50 rows=18 width=40)
   Filter: (class = ANY ('{2,3,4}'::integer[]))     
(2 rows)
```

###### 带有EXIST的子查询
语法形式为:
```sql
... WHERE [NOT] EXIST (子查询) 
```
如果子查询有返回结果，则exists结果为真，否则为假。举例说明：
>一般使用时会将父查询中的数据放到子查询中做条件验证，如果验证通过，即子查询中有返回结果，则exist结果为真，如果验证没有通过，即子查询中没有返回结果，则exists结果为假。

```sql
-- 当子查询为假时，exists结果为假，输出班级1,2的学生信息，3,4班级ID不满足后面子查询语句条件，没有结果，返回false。
postgres=# select * from student where exists (select * from school where student.class=school.cid and school.cid < 3);
 id | class |   name    
----+-------+-----------
  1 |     1 | houjunnan
  2 |     1 | fangsheng
  3 |     1 | yulianjie
  4 |     1 | yejing
  5 |     1 | txf
  6 |     2 | nanli
  7 |     2 | wuyefei
  8 |     2 | fangjun
(8 rows)

-- 查看执行计划，与上面的查询语句类似，都进行了逻辑优化。
postgres=# explain select * from student where exists (select * from school where student.class=school.cid and school.cid < 3);
                                QUERY PLAN                                 
---------------------------------------------------------------------------
 Hash Join  (cost=44.59..76.41 rows=600 width=40)
   Hash Cond: (student.class = school.cid)
   ->  Seq Scan on student  (cost=0.00..22.00 rows=1200 width=40)
   ->  Hash  (cost=42.11..42.11 rows=198 width=4)
         ->  HashAggregate  (cost=40.13..42.11 rows=198 width=4)
               Group Key: school.cid
               ->  Seq Scan on school  (cost=0.00..38.25 rows=753 width=4)
                     Filter: (cid < 3)
(8 rows)
```
通过这个例子，我们再介绍一个概念：关联子查询。关联子查询是指和外部查询有关联的子查询，具体来说就是在这个子查询里使用了外部查询包含的列。


###### 带有比较运算符的子查询
带有比较运算符的子查询是指父查询与子查询之间用比较运算符进行连接。当用户确切知道内层查询返回单个值时，可以用>、<、=、>=、<=、!=或<>等比较运算符。下面距离说明：
```sql
postgres=# select * from class where cid < (select cid from school order by students limit 1);
 cid |  teacher  | school 
-----+-----------+--------
   1 | yuminhong | 一中
   1 | chengeng  | 一中
   2 | hengxin   | 一中
   1 | xinjj     | 二中
(4 rows)

-- 带有比较运算符时，子查询必须返回单值才行
postgres=# select * from class where cid < (select cid from school order by students);
ERROR:  more than one row returned by a subquery used as an expression
```

###### 带有ANY/ALL的子查询
上面讲了子查询返回单值可以用比较运算符，但返回多值时要用ANY或ALL谓词修饰符。而使用ANY或ALL谓词的时候必须同时使用比较运算符，语义如下：
- ">ANY" 大于子查询结果中的某个值
- "> ALL" 大于子查询结果中的所有值
- <ANY 小于子查询结果中的某个值
- <ALL 小于子查询结果中的所有值
- ">=ANY" 大于等于子查询结果中的某个值
- ">=ALL" 大于等于子查询结果中的所有值
- <=ANY 小于等于子查询结果中的某个值
- <=ALL 小于等于子查询结果中的所有值
- =ANY 等于子查询结果中的某个值
- =ALL 等于子查询结果中的所有值
- !=（或<>）ANY 不等于子查询结果中的某个值
- !=（或<>）ALL 不等于子查询结果中的所有值

举例说明：
```sql
postgres=# select rownum,* from rownum where id <ANY (select class from student); 
 rownum | id |  name  
--------+----+--------
      1 |  1 | 3a
      2 |  2 | 645abc
      3 |  3 | 1
(3 rows)
```