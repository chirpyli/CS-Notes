有时候我们需要统计一下代码量，这时候就需要代码统计工具，推荐cloc。安装如下：
```shell
  npm install -g cloc                    # https://www.npmjs.com/package/cloc
  sudo apt-get install cloc              # Debian, Ubuntu
  sudo yum install cloc                  # Red Hat, Fedora
  sudo pacman -S cloc                    # Arch
  sudo pkg install cloc                  # FreeBSD
  sudo port install cloc                 # Mac OS X with MacPorts
```

用法示例：
```shell
sl@Li:~/Works/opensource$ cloc pytorch/
    5080 text files.
    5055 unique files.                                          
    1970 files ignored.

http://cloc.sourceforge.net v 1.60  T=22.37 s (188.9 files/s, 38509.4 lines/s)
--------------------------------------------------------------------------------
Language                      files          blank        comment           code
--------------------------------------------------------------------------------
C++                            1455          37191          19300         251819
Python                         1005          43787          40777         195427
C/C++ Header                   1436          26954          27740         156103
C                                44           2617           1009          15844
CMake                           138           1816           3076          10990
YAML                             25           2162            998           8785
Objective C++                     5            691            286           6216
Bourne Shell                     88            803            891           3342
CSS                               3            284             82           1355
DOS Batch                        14            101              1            353
HTML                              4             17             33            235
XML                               1              5              7            185
make                              4             29             17             68
Teamcenter def                    1             20              0             60
Bourne Again Shell                1              5              2             36
Lua                               1              5              0             28
PowerShell                        1              2              0             15
vim script                        1              0              0              3
--------------------------------------------------------------------------------
SUM:                           4227         116489          94219         650864
--------------------------------------------------------------------------------
```

>更多参考：[Count Lines of Code](http://cloc.sourceforge.net/)