>本文主要内容翻译自：[The RLPx Transport Protocol](https://github.com/ethereum/devp2p/blob/master/rlpx.md)，其中添加了一些个人的理解，由于密码学水平有限，不正确之处望指正。另外原文可能已经更新，最新内容请直接阅读原文。


本文档定义了RLPx传输协议，一种基于TCP的用于Ethereum节点间通信的传输协议。该协议适用于任意内容的加密帧，但它通常用于承载devp2p应用程序协议。

### 节点标识
所有加密操作都基于`secp256k1`椭圆曲线。每个节点都需要维护一个在会话间保存和复原的静态私钥。建议私钥只能手动重置，例如，通过删除文件或数据库条目。

### ECIES加密
ECIES(Elliptic Curve Integrated Encryption Scheme)是在RLPx握手中使用的非对称加密方法。RLPx使用的密码系统是：
- `secp256k1`椭圆曲线，生成元$G$
- $KDF(k,len)$：`NIST SP 800-56`级联密钥导出函数
- $MAC(k,m)$：`HMAC`使用`SHA-256`哈希函数
- $AES(k,iv,m)$：`AES-256`加密算法`CTR`模式
>（*这里原文是`AES-128`，但是Ethereum源代码中是`AES-256`，所以本文更改为`AES-256`*）

Alice想要发送可以被Bob的静态私钥$k_B$解密的加密消息。Alice知道Bob的静态公钥$K_B$。

为了加密消息$m$，Alice生成了一个随机数$r$，相应的生成了椭圆曲线公钥$R=r*G$并计算出共享密钥$S=P_x$，$(P_x,P_y)=r*K_B$。接着，$k_E\mid\mid k_M=KDF(S,32)$导出加密和认证主密钥，随机生成一个初始向量$iv$。Alice向Bob发送加密消息$R \mid\mid iv \mid\mid c \mid\mid d$，其中$c=AES(k_E,iv,m)$，$d=MAC(k_M,iv\mid\mid c)$。

Bob需要解密消息$R \mid\mid iv \mid\mid c \mid\mid d$，为此，需要导出共享密钥$S=P_x$，其中$(P_x,P_y)=k_B*R$，以及导出加密和认证密钥$k_E \mid\mid k_M=KDF(S,32)$。Bob通过等式$d==MAC(k_M,iv\mid\mid c)$是否成立验证消息后计算$m=AES(k_E,iv\mid\mid c)$获取明文消息。

### 握手
“握手”过程构建了会话阶段中使用的主密钥。握手是在发起端（发起TCP连接请求的节点）和接收端（接受连接的节点）之间完成。

握手协议：

`E`是上面定义的`ECIES`非对称加密函数。
>补充说明： E代表加密；S代表签名；H代表Hash运算
```
auth -> E(remote-pubk, S(ephemeral-privk, static-shared-secret ^ nonce) || H(ephemeral-pubk) || pubk || nonce || 0x0)   # 由握手发起方发送，向对方发送密钥协商需要的本节点（公钥+临时公钥+随机数）
auth-ack -> E(remote-pubk, remote-ephemeral-pubk || nonce || 0x0)   # 接收方回应auth消息，向对方发送密钥协商需要的本节点（临时公钥+随机数），本节点公钥对方已经知道，所以这里不需要发送了。

static-shared-secret = ecdh.agree(privkey, remote-pubk)
```
握手后值的计算（步骤如下）：
```
ephemeral-shared-secret = ecdh.agree(ephemeral-privkey, remote-ephemeral-pubk)
shared-secret = keccak256(ephemeral-shared-secret || keccak256(nonce || initiator-nonce))
aes-secret = keccak256(ephemeral-shared-secret || shared-secret)
# destroy shared-secret
mac-secret = keccak256(ephemeral-shared-secret || aes-secret)
# destroy ephemeral-shared-secret

Initiator:
egress-mac = keccak256.update(mac-secret ^ recipient-nonce || auth-sent-init)
# destroy nonce
ingress-mac = keccak256.update(mac-secret ^ initiator-nonce || auth-recvd-ack)
# destroy remote-nonce

Recipient:
egress-mac = keccak256.update(mac-secret ^ initiator-nonce || auth-sent-ack)
# destroy nonce
ingress-mac = keccak256.update(mac-secret ^ recipient-nonce || auth-recvd-init)
# destroy remote-nonce
```
> 1. 用临时密钥，一定程度上可以保证前向安全性。后文中有前向安全性的描述。
> 2. 握手过程最重要的是协商密钥（对应上面的aes-secret、mac-secret）。
> 3. 密钥协商过程中需要知道本节点和对方节点的（公钥+临时公钥+随机数）。这不是绝对的，不同的密钥协商算法有不同的实现方式，但基本上都需要双方交换一些数据，从而分别推到出相同的密钥。

> 补充一点，整个协商密钥的过程核心是ECDH密钥协商，但ECDH协商的过程前提是要对方是认证过的，可信的，前面的ECIES加密，实际上相当于对消息接收方进行认证，因为只有拥有对应的私钥才能解密消息。

创建加密连接主要流程如下：
1. 发起端向接收端发起TCP连接，发送`auth`消息
2. 接收端接受连接，解密、验证`auth`消息（检查`recovery of signature == keccak256(ephemeral-pubk)`）
3. 接收端生成`auth-ack`消息
4. 接收端导出密钥，发送第一个数据帧
5. 发起端接收到`auth-ack`消息，导出密钥
6. 发起端发送第一个数据帧（*代码中对应Hello packet*）
7. 接收端接收并验证数据帧
8. 发起端接收并验证数据帧
9. 如果`MAC`两边都验证通过，加密握手完成。

> 简单概括就是：建立TCP连接-->密钥协商（auth、auth-ack）-->双发导出相同的密钥-->发送hello（协议协商）-->创建完成。具体实现需要看Ethereum源码。

### 分帧
在`auth`之后的所有数据包都是分帧传输的。任何一方如果第一帧数据包验证失败都可以断开连接。

分帧传输的主要目的是在单一连接上可靠的支持多路复用协议。其次，因数据包分帧，为消息认证码产生了适当的分界点，使得加密流变得简单了。数据帧通过握手产生的密钥进行验证。

数据帧头部提供了数据包的大小和协议信息。
```
frame = header || header-mac || frame-data || frame-mac
header = frame-size || header-data || padding
frame-size = size of frame excluding padding, integer < 2**24, big endian
header-data = rlp.list(protocol-type[, context-id])
protocol-type = integer < 2**16, big endian
context-id = integer < 2**16, big endian
padding = zero-fill to 16-byte boundary
frame-content = any binary data

header-mac = left16(egress-mac.update(aes(mac-secret,egress-mac)) ^ header-ciphertext).digest
frame-mac = left16(egress-mac.update(aes(mac-secret,egress-mac)) ^ left16(egress-mac.update(frame-ciphertext).digest))
egress-mac = keccak256 state, continuously updated with egress bytes
ingress-mac = keccak256 state, continuously updated with ingress bytes

left16(x) is the first 16 bytes of x
|| is concatenate
^ is xor
```
对发送与接收的密文数据不断更新`egress-mac`或`ingress-mac`实现消息认证；对头部数据，是通过将加密输出数据的头部与相应的`mac`进行异或运算（参见`header-mac`）。这样做是为了确保对明文`mac`和密文执行统一操作。所有的mac都是以明文形式发送的。

填充字节用于防止缓存区饥饿，使得帧组件按指定区块字节大小对齐。

### 已知的问题
- RLPx握手被认为是易破解的，因为`aes-secret`和`mac-secret`被重复用于读取和写入。RLPx连接的两端从相同的密钥，nonce和IV生成两个CTR流。如果攻击者知道一个明文，他们就可以用重用的密钥流破解未知明文。
- 帧编码提供了用于多路复用的协议类型字段`protocol-type`，但devp2p未使用该字段。


### RLPx传输协议的前向安全性

RLPx使用了(PerfectForwardSecrecy)，简单来说。链接的两方都生成随机的私钥，通过随机的私钥得到公钥。然后双方交换各自的公钥，这样双方都可以通过自己随机的私钥和对方的公钥来生成一个同样的共享密钥(shared-secret)。后续的通讯使用这个共享密钥作为对称加密算法的密钥。这样来说。如果有一天一方的私钥被泄露，也只会影响泄露之后的消息的安全性，对于之前的通讯是安全的(因为通讯的密钥是随机生成的，用完后就消失了)。

### 前向安全性

前向安全或前向保密FS（ForwardSecrecy），有时也被称为完美前向安全PFS（PerfectForwardSecrecy），是密码学中通讯协议的安全属性，指的是长期使用的主密钥泄漏不会导致过去的会话密钥泄漏。前向安全能够保护过去进行的通讯不受密码或密钥在未来暴露的威胁。如果系统具有前向安全性，就可以保证万一密码或密钥在某个时刻不慎泄露，过去已经进行的通讯依然是安全，不会受到任何影响，即使系统遭到主动攻击也是如此。


>最后，需要说明一下，这篇文档对RLPx协议进行了简述，具体实现协议还是有很多细节需处理，深入请看以太坊源码。

