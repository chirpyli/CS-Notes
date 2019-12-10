DH密钥交换是一种安全协议，它可以让双方在不安全的信道上创建一个密钥。双方互相发送的数据就算被第三方知晓，也无法知道加密信息的密钥。

**其解决问题的主要思想**可以用下图来解释：

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190530134008494.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==,size_16,color_FFFFFF,t_70)

Alice和Bob想要协商出一个只有它们两人知道的颜色，不能让第三方知道，怎么办呢？解决办法如下：
1. 先从它们共同拥有的颜色（图中为黄色）开始，这个黄色是大家都知道的，第三方知道也没有关系。
2. Alice选了一个只有自己知道的颜色（图中为红色），并将之混入大家知道的黄色中，形成新的颜色（图中为棕色）。
3. Bob也选了一个只有自己知道的颜色（图中为淡绿色），并将之混入大家都知道的黄色中，形成新的颜色（图中为浅蓝色）。
4. Alice和Bob交换混合后的颜色。（这里假定人们很难从混合颜色中找到是哪两种颜色混合的，安全性保证取决于此，所以即使第三方知道了混合后的颜色也没有用，因为它推断不出来只有Alice和Bob自己掌握的红色和淡绿色）
5. Alice收到Bob发送过来的混合色后，再加入只有自己知道的红色，得到秘密颜色=黄色+红色+淡绿色（图中为土色，在图的最下方）。
6. Bob收到Alice发送过来的混合色后，再加入只有自己知道的淡绿色，得到秘密颜色=黄色+淡绿色+红色。
7. 至此，Alice和Bob各自拥有了只有它们两人知道的秘密颜色，且秘密颜色是相同的。

这里的关键是，混合后的颜色，人们无法知晓是由哪两种颜色混合而成的。由此，很容易想到数学难题，离散对数问题。**数学描述如下：**

![在这里插入图片描述](https://img-blog.csdnimg.cn/20190530134339996.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3NfbGlzaGVuZw==,size_16,color_FFFFFF,t_70)
>这里，`a`只有Alice知道，`b`只有Bob知道，`g,p`是公开的，`K`是最终计算出的共享密钥。

**一般描述如下：**
1. Alice和Bob协商一个有限循环群`G`和它的一个生成元`g`，一个大素数`p`;
2. Alice生成一个随机数`a`，计算`A = g^a mod p`，将`A`发送给Bob；
3. Bob生成一个随机数`b`，计算`B = g^b mod p`，将`B`发送给Alice；
4. Alice计算`K = B^a mod p = (g^b)^a mod p`，得到共享密钥`K`；
5. Bob计算`K = A^b mod p = (g^a)^b mod p`，得到共享密钥`K`；
>`(g^b)^a = (g^a)^b`因为群是乘法交换的，涉及到数论及代数的内容。Alice和Bob同时协商出`K`，作为共享密钥。

最后，**安全性问题**，DH密钥交换可以防窃听（即，你知道我们交换的数据也没关系），但是DH本身并没有提供通讯双方的身份验证服务（正确交换的前提是，Alice必须确保对方是Bob），无法抵御中间人攻击。

>参考文档：[Diffie–Hellman key exchange](https://en.wikipedia.org/wiki/Diffie%E2%80%93Hellman_key_exchange#References)