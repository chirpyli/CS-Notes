本文描述了使用`ECIES`加解密时的计算步骤。ECIES加密可以认为是先基于ECDH协商密钥，再用刚刚协商一致的密钥进行对称加密，这样能够结合非对称加密和对称加密的优点。

#### 前提
为了向Bob发送`ECIES`加密信息，Alice需要以下信息：
- 密码学套件（ $KDF$，$MAC$，对称加密$E$）
- 椭圆曲线$(p,a,b,G,n,h)$
- Bob的公钥：$K_B$（$K_B=k_BG$，$k_B\in[1,n-1]$）
- 共享信息：$S_1$，$S_2$
- 无穷远点$O$


#### 加密步骤
Alice加密消息$m$的步骤如下：
1. 生成随机数$r\in[1,n-1]$，计算$R=rG$。      *（生成临时密钥对）*
2. 导出共享密钥：$S=P_x$，其中$P=P(P_x,P_y)=rK_B$，($P \neq O$)。  *(ECDH密钥协商)*             
3. $KDF$导出对称加密密钥和$MAC$密钥：$k_E||k_M=KDF(S||S_1)$。
4. 加密消息$m$：$c=E(k_E;m)$。    *(对称加密消息)*
5. 计算加密后的消息和$S_2$的$MAC$：$d=MAC(k_M;c||S_2)$
6. 输出：$R||c||d$


#### 解密步骤
Bob解密密文$R||c||d$的步骤如下：
1. 导出共享密钥：$S=P_x$，其中$P=P(P_x,P_y)=k_BR$，(Bob与Alice会计算出相同的结果，因为$P=k_BR=k_BrG=rk_BG=rK_B$)
2. 导出与Alice相同的密钥：$k_E||k_M=KDF(S||S_1)$
3. 验证$MAC$，如果$d\neq MAC(k_M;c||S_2)$则失败
4. 解密密文：$m=E^{-1}(k_E;c)$


>参考文档：[Integrated Encryption Scheme](https://en.wikipedia.org/wiki/Integrated_Encryption_Scheme)
