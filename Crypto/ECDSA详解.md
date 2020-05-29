ECDSA(Elliptic Curve Digital Signature Algorithm)，椭圆曲线数字签名算法。

#### 椭圆曲线数字签名生成
假设Alice希望对消息$m$进行签名，所采用的椭圆曲线参数为$D=(p,a,b,G,n,h)$，对应的密钥对为$(k,Q)$，其中$Q$为公钥，$k$为私钥。

Alice将按如下步骤进行签名：

1. 产生一个随机数$d$，$1 \leq d \leq n-1$.              （签名算法首先生成一个临时私公钥对，该临时密钥对用于计算$r$和$s$值。）
2. 计算$dG=(x_1,y_1)$，将$x_1$转化为整数$\overline{x_1}$. 
3. 计算$r=\overline{x_1} \ mod \  n$，若$r=0$，则转向第1步.     （$r$值为临时公钥的$x$坐标值）
4. 计算$d^{-1} \ mod \ n$.
5. 计算哈希值$H(m)$，并将得到的比特串转化为整数$e$.
6. 计算$s=d^{-1}(e+kr) \ mod \ n$，若$s=0$，则转向第1步.
7. $(r,s)$即为Alice对消息$m$的签名.

>$d^{-1}$ is the multiplicative inverse of $d$ modulo $n$.逆元。

#### 椭圆曲线签名验证
为验证Alice对消息$m$的签名$(r,s)$，Bob需要得到Alice所用的椭圆曲线参数$D=(p,a,b,G,n,h)$以及Alice的公钥$Q$。

步骤如下：
1. 验证$r$和$s$是区间$[1,n-1]$上的整数.
2. 计算$H(m)$并将其转化为整数$e$.
3. 计算$w=s^{-1} \ mod \ n$.
4. 计算$u_1=ew \ mod \ n$以及$u_2=rw \ mod \ n$.
5. 计算$X=(x_1,y_1)=u_1G+u_2Q$.
6. 若$X=O$，则拒绝签名，否则将$X$的$x$坐标$x_1$转化为整数$\overline{x_1}$，并计算$v=\overline{x_1} \ mod \ n$.
7. 当且仅当$v=r$时，签名通过验证.

#### 椭圆曲线签名正确性
要证明$v=r$，只需要证明$X=dG$即可。

证明步骤：

令：$C=u_1G + u_2Q = u_1G+u_2kG=(u_1+u_2k)G$

将$u_1$、$u_2$带入：$C=(ew+rwk)G=(e+rk)wG=(e+rk)s^{-1}G$

由$s=d^{-1}(e+kr) \mod  n$得出$s^{-1}=d(e+kr)^{-1} \mod n$，带入：
$C=(e+kr)d(d+kr)^{-1}G = dG$

证明完毕。


>参考文档：     
[Elliptic Curve Digital Signature Algorithm
](https://en.wikipedia.org/wiki/Elliptic_Curve_Digital_Signature_Algorithm)     
[Elliptic Curve Cryptography: ECDH and ECDSA](https://andrea.corbellini.name/2015/05/30/elliptic-curve-cryptography-ecdh-and-ecdsa/)   
[Understanding How ECDSA Protects Your Data.](https://www.instructables.com/id/Understanding-how-ECDSA-protects-your-data/)        