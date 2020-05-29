[前面一篇](https://juejin.im/post/5d1f0146f265da1bcc196203)讲过DH密钥交换算法，ECDH（Elliptic Curve Diffie-Hellman）顾名思义就是ECC+DH，安全性保证由椭圆曲线离散对数难题来保证。其思想与DH一致。


#### 椭圆曲线密码学
椭圆曲线密码学是属于非对称密码学的。其公私钥计算公式如下：
* 私钥是一个随机数$d$，取值范围在${1,...,n-1}$，其中$n$是子群的阶
* 公钥是点$H=dG$，$G$是子群的基点

如果我们知道私钥$d$和椭圆曲线参数$G$，求公钥$H$是很容易的，但是只知道公钥$H$和椭圆曲线参数$G$，求解私钥$d$是非常困难的，需要解决离散对数难题，椭圆曲线的安全性保证有赖于此。

#### ECDH密钥交换
ECDH工作过程如下（以Alice和Bob为例）：

0. 首先Alice和Bob需要使用同一条椭圆曲线，参数$(p,a,b,G,n,h)$相同。
1. Alice和Bob生成各自的公私钥。Ailce生成私钥$d_A$，公钥$H_A=d_AG$，Bob生成私钥$d_B$，公钥$H_B=d_BG$。
2. Alice和Bob交换各自的公钥$H_A$和$H_B$。
3. Alice计算$S=d_AH_B$，Bob计算$S=d_BH_A$。可以推导出它们计算的值相同。推导过程$S=d_AH_B=d_Ad_BG=d_B(d_AG)=d_BH_A$。

至此，Alice和Bob得到了相同的$S$，但是第三方在知道椭圆曲线和$H_A$，$H_B$的情况下，是无法获知$S$的，因为要推导出$S$，就必须知道其中一个私钥，这必须破解椭圆曲线离散对数难题，是做不到的。

应用上，一般先利用ECDH协商共享密钥，再利用共享密钥计算出对称密钥等其他需要的密钥，通信双发对数据进行对称加密安全通信。


>参考文档：      
[The Elliptic Curve Diffie-Hellman (ECDH)](https://koclab.cs.ucsb.edu/teaching/ecc/project/2015Projects/Haakegaard+Lang.pdf)       
[Elliptic Curve Cryptography: ECDH and ECDSA](https://andrea.corbellini.name/2015/05/30/elliptic-curve-cryptography-ecdh-and-ecdsa/)