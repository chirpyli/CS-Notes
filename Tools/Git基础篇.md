Git是开源的<font color=green>分布式版本控制系统</font>，可以说是开发人员必备了，每个程序员都应该掌握。这里主要讲Git的安装与配置，工作区、暂存区、版本库的概念以及常用的一些命令。

### Linux下Git的安装与配置

Ubuntu下可以运行以下命令安装：```sudo apt install git ```，安装成功后可查看Git版本：```git --version```。
#### Git配置
Git 提供了一个叫做 git config 的工具，专门用来配置或读取相应的工作环境变量。

这些环境变量，决定了Git 在各个环节的具体工作方式和行为。这些变量可以存放在以下三个不同的地方：

- /etc/gitconfig文件：系统中对所有用户都普遍适用的配置。若使用 git config 时用“--system”选项，读写的就是这个文件。可使用```git config -e --system```对配置文件进行配置。
- ~/.gitconfig文件：用户目录下的配置文件只适用于该用户。若使用 git config 时用“--global”选项，读写的就是这个文件。可使用```git config -e --global```对配置文件进行配置。
- 当前项目的 Git 目录中的配置文件（也就是工作目录中的 .git/config 文件）：这里的配置仅仅针对当前项目有效。每一个级别的配置都会覆盖上层的相同配置，所以 .git/config 里的配置会覆盖 /etc/gitconfig中的同名变量。可进入对应项目目录使用```git config -e```对配置文件进行配置。
>（Git的配置文件是INI格式的）
#### 配置用户信息
开始前，需要先配置用户名和邮件地址：
```
git config --global user.name "your name"
git config --global user.email your_email
```
>如果用了 --global 选项，那么更改的配置文件就是位于你用户主目录下的那个，以后你所有的项目都会默认使用这里配置的用户信息。如果要在某个特定的项目中使用其他名字或者电邮，只要去掉 --global 选项重新配置即可，新的设定保存在当前项目的 .git/config 文件里。

查看已有的配置信息：```git config --list```。



### Git基本概念（工作区、暂存区、版本库）
理解Git是怎么工作的，主要是要理解工作区、暂存区、版本库这三个概念。

- 工作区：就是你在电脑里能看到的目录。
- 暂存区：英文叫stage, 或index。一般存放在 ".git目录下" 下的index文件（.git/index）中，所以我们把暂存区有时也叫作索引（index）。
- 版本库：工作区有一个隐藏目录.git，这个不算工作区，而是Git的版本库。

下面这个图展示了工作区、版本库中的暂存区和版本库之间的关系：
![这里写图片描述](../images/git.jpg)

<font color=red>工作区-->暂存区</font>
当对工作区修改（或新增）的文件执行 "git add" 命令时，暂存区的目录树被更新，同时工作区修改（或新增）的文件内容被写入到对象库中的一个新的对象中，而该对象的ID被记录在暂存区的文件索引中。

<font color=red>暂存区-->版本库</font>
当执行提交操作（git commit）时，暂存区的目录树写到版本库（对象库）中，master 分支会做相应的更新。即 master 指向的目录树就是提交时暂存区的目录树（这句话相对绕口一些，这里以master分支为例）。


### 忽略文件（.gitignore）
一般我们总会有些文件无需纳入 Git 的管理，也不希望它们总出现在未跟踪文件列表。 通常都是些自动生成的文件，比如日志文件，或者编译过程中创建的临时文件等。 在这种情况下，我们可以创建一个名为`.gitignore`的文件，列出要忽略的文件模式。要养成一开始就设置好`.gitignore`文件的习惯，以免将来误提交这类无用的文件。示例如下：
```
# no .a files
*.a

# but do track lib.a, even though you're ignoring .a files above
!lib.a

# only ignore the TODO file in the current directory, not subdir/TODO
/TODO

# ignore all files in the build/ directory
build/

# ignore doc/notes.txt, but not doc/server/arch.txt
doc/*.txt

# ignore all .pdf files in the doc/ directory
doc/**/*.pdf
```

>GitHub 有一个十分详细的针对数十种项目及语言的`.gitignore`文件列表：[gitignore](https://github.com/github/gitignore)，可以参考。
### Git常用命令
#### 创建仓库
- ```git init ```：使用当前目录作为Git仓库
- ```git init <directory> ```：使用指定目录作为Git仓库

#### git clone
- ```git clone <repertory-url>```：克隆仓库
- ```git clone <repertory-url> <directory>```：克隆到指定目录

#### git diff
- ```git diff```：查看工作区的最新改动，即工作区与提交暂存区的差异
- ```git diff --cached```：查看已缓存的改动，即提交暂存区与版本库中的差异
- ```git diff HEAD```：查看工作区与HEAD（当前工作分支）的差异
- ```git diff --stat```：显示摘要而非整个 diff：
>HEAD ：代表版本库中最近一次提交     
>^    ：指代父提交      
>HEAD^：代表当前提交的上一次提交        
>HEAD^^：以此类推......

#### git reset
- ```git reset --hard HEAD^```：回退到上一次提交。彻底撤销最近的提交，工作区和暂存区都回退到上一次提交的状态。
- ```git reset --hard <commit>```：回退到指定提交
>git reset --hard参数会进行彻底回退，工作区、暂存区、版本库内容一致，即回退到指定commit的那个状态

#### 查看项目的分支们(包括本地和远程) 
- ```git branch -a```

#### 删除本地分支与远程分支
- ```git branch -d <BranchName>```：	删除本地分支 
- ```git push origin --delete <BranchName>```： 删除远程分支 

#### 其他命令
- `git add`：文件添加到缓存
- `git add .`：添加项目的所有文件
- `git status`：查看当前状态
- `git commit -m"注释"`：将缓存区内容添加到仓库
- `git mv file_from file_to`： 重命名某个文件
- `git remote rename old_name new_name`： 远程仓库重命名

>参考：     
[Git官网](https://git-scm.com/)     
[Git Book](https://git-scm.com/book/zh/v2)


