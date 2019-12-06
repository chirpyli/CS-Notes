### 初识matlab
最初接触matlab是在大学时学习数学建模参加全国大学生数学建模竞赛时接触的，对于数学建模而言，Matlab是一款相当不错的建模辅助工具，因为 Matlab中有统计函数，线性分析函数，插值函数，非线性分析函数等等这些数模必备的函数，而且，Matlab强大的绘图功能可使很多数学演算过程变得可视化。这些对于分析问题及验证结果的正确性非常有帮助。
### matlab简介
Matlab是MATrix LABoratory的缩写。Matlab开发的最初目的是为软件中的矩阵运算提供方便。Matlab是一种基于向量的高级程序语言，它将计算、可视化与程序设计集成在一个易用的环境中。换言之，Matlab采用技术计算语言，几乎与专业领域中所使用的数学表达式相同。Matlab的典型应用包括：

- **数学与计算**
- 算法开发
- 数据获取
- 建模、仿真和原型化
- **数据分析、数据挖掘和可视化**
- 科学与工程图学；
- 应用开发，包括图形用户界面构造。

Matlab中的基本数据元素是矩阵，它提供了各种矩阵运算和操作，并有较强的绘图能力。同是，Matlab的强大功能就在于提供了还在不断扩大的工具箱。Matlab中的每一个工具箱都以一门专门理论为背景，并为之服务。它将理论中所涉及的公式运算、方程求解全部编写成了Matlab环境下的称为M函数或M文件的子程序。设计者只需根据自己的需要，直接调用函数名，输入变量与参数，运行函数，便可立即得到结果。Matlab是广为流传、备受人们喜爱的一种软件环境，目前，它在控制工程、生物医学工程、信号分析、语音处理、**图像处理**、统计分析、雷达工程、计算机技术和数学等各领域中都有极其广泛的应用。


### matlab常用工具箱
matlab包括拥有数百个内部函数的主包和三十几种工具包。工具包又可以分为功能性工具包和学科工具包。功能工具包用来扩充matlab的符号计算，可视化建模仿真，文字处理及实时控制等功能。学科工具包是专业性比较强的工具包，控制工具包，信号处理工具包，通信工具包等都属于此类。
- Matlab Main Toolbox——matlab主工具箱
- Control System Toolbox——控制系统工具箱
- Communication Toolbox——通讯工具箱
- Financial Toolbox——财政金融工具箱
- System Identification Toolbox——系统辨识工具箱
- Fuzzy Logic Toolbox——模糊逻辑工具箱
- Higher-Order Spectral Analysis Toolbox——高阶谱分析工具箱
- **Image Processing Toolbox——图象处理工具箱**
- **computer vision system toolbox----计算机视觉工具箱**
- LMI Control Toolbox——线性矩阵不等式工具箱
- Model predictive Control Toolbox——模型预测控制工具箱
- μ-Analysis and Synthesis Toolbox——μ分析工具箱
- **Neural Network Toolbox——神经网络工具箱**
- Optimization Toolbox——优化工具箱
- Partial Differential Toolbox——偏微分方程工具箱
- Robust Control Toolbox——鲁棒控制工具箱
- **Signal Processing Toolbox——信号处理工具箱**
- Spline Toolbox——样条工具箱
- Statistics Toolbox——统计工具箱
- Symbolic Math Toolbox——符号数学工具箱
- Simulink Toolbox——动态仿真工具箱
- Wavele Toolbox——小波工具箱
- DSP system toolbox-----DSP处理工具箱

### matlab图像处理工具箱
图像处理工具箱是Matlab环境下开发出来的许多工具箱之一，它以数字图像处理理论为基础，用Matlab语言构造出一系列用于图像数据显示与处理的M函数。这些函数包括：

- 几何运算，包括缩放、旋转和裁剪；
- 分析操作，包括边缘检测，四叉树分解；
- 增强操作，包括亮度调整，直方图均衡化，去噪声；
- 2D FIR滤波器设计；
- 图像变换，包括离散余弦变换和Radon变换；
- 邻域与块处理；
- 感兴趣区域处理；
- 二值图像处理，包括**形态学操作**；
- 彩色空间变换；
- 彩色地图管理。

此外，Matlab中的信号处理工具箱、神经网络工具箱、模糊逻辑工具箱和小波工具箱也用于协助执行图像处理任务。

### matlab常用函数
#### matlab常用内部常数
- eps：浮点相对精度
- exp：自然对数的底数e 
- i 或j：基本虚数单位
- inf 或Inf：无限大， 例如1/0
- nan或NaN：非数值（Not a number），例如0/0
- **pi：圆周率p（= 3.1415926...）**
- realmax：系统所能表示的最大数值
- realmin：系统所能表示的最小数值
- nargin: 函数的输入引数个数
- nargout: 函数的输出引数个数
- lasterr：存放最新的错误信息
- lastwarn：存放最新的警告信息

#### matlab常用基本数学函数
- abs(x)：纯量的绝对值或向量的长度
- angle(z)：复数z的相角(Phase angle)
- sqrt(x)：开平方
- real(z)：复数z的实部
- imag(z)：复数z的虚部
- conj(z)：复数z的共轭复数
- round(x)：四舍五入至最近整数
- fix(x)：无论正负,只取其整数部分
- floor(x)：下取整，即舍去正小数至最近整数
- ceil(x)：上取整，即加入正小数至最近整数
- rat(x)：将实数x化为多项分数展开
- rats(x)：将实数x化为分数表示
- sign(x)：符号函数(Signum function)
- rem(x,y)：求x除以y的余数
- gcd(x,y)：整数x和y的最大公因数
- lcm(x,y)：整数x和y的最小公倍数
- exp(x) ：自然指数
- pow2(x)：2的指数
- log(x)：以e为底的对数，即自然对数或
- log2(x)：以2为底的对数
- log10(x)：以10为底的对数

#### matlab常用三角函数
- sin(x)：正弦函数
- cos(x)：余弦函数
- tan(x)：正切函数
- asin(x)：反正弦函数
- acos(x)：反余弦函数
- atan(x)：反正切函数
- atan2(x,y)：四象限的反正切函数
- sinh(x)：双曲正弦函数
- cosh(x)：双曲余弦函数
- tanh(x)：双曲正切函数
- asinh(x)：反双曲正弦函数
- acosh(x)：反双曲余弦函数
- atanh(x)：反双曲正切函数

#### 适用于向量的常用函数

- min(x): 向量x的元素的最小值
- max(x): 向量x的元素的最大值
- mean(x): 向量x的元素的平均值
- median(x): 向量x的元素的中位数
- std(x): 向量x的元素的标准差
- diff(x): 向量x的相邻元素的差
- sort(x): 对向量x的元素进行排序（Sorting）
- length(x): 向量x的元素个数
- norm(x): 向量x的欧氏（Euclidean）长度
- sum(x): 向量x的元素总和
- prod(x): 向量x的元素总乘积
- cumsum(x): 向量x的累计元素总和
- cumprod(x): 向量x的累计元素总乘积
- dot(x, y): 向量x和y的内积
- cross(x, y): 向量x和y的外积

#### matlab基本绘图函数　　
 - plot: x轴和y轴均为线性刻度（Linear scale）
 -  loglog: x轴和y轴均为对数刻度（Logarithmic scale）
 - semilogx: x轴为对数刻度，y轴为线性刻度
 - semilogy: x轴为线性刻度，y轴为对数刻度
##### plot绘图函数的参数
字元颜色 字元图线型态
y 黄色. 点
k 黑色o 圆
w 白色x x
b 蓝色+ +
g 绿色* *
r 红色- 实线
c 亮青色: 点线
m 锰紫色-. 点虚线
-- 虚线

#### 注解
```matlab
xlabel('Input Value'); % x轴注解
ylabel('Function Value'); % y轴注解
title('Two Trigonometric Functions'); % 图形标题
legend('y = sin(x)','y = cos(x)'); % 图形注解
grid on; % 显示格线
```
#### 二维绘图函数
- bar 长条图
- errorbar 图形加上误差范围
- fplot 较精确的函数图形
- polar 极座标图
- hist 累计图
- rose 极座标累计图
- stairs 阶梯图
- stem 针状图
- fill 实心图
- feather 羽毛图
- compass 罗盘图
- quiver 向量场图