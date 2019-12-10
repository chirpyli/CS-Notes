在对程序进行性能调优时，一般要首先进行性能测试，然后进行性能分析，分析后才进行性能调优，性能监控等。对性能分析，可使用系统或者性能分析工具进行性能分析。


### Linux性能分析工具
可参考[Linux性能分析工具](https://zhuanlan.zhihu.com/p/35879028).

### 性能分析工具——Valgrind
在使用Valgrind进行性能分析时，主要使用其中的Callgrind。

Callgrind是一个分析工具，可以将程序运行中的函数之间的通话记录作为调用图记录。默认情况下，收集的数据由执行的指令数，它们与源行的关系，函数之间的调用者/被调用者关系以及这些调用的数量组成。可选地，高速缓存模拟和/或分支预测（类似于Cachegrind）可以产生关于应用的运行时行为的更多信息。

在程序终止时，将配置文件数据写入文件。为了呈现数据，并对分析进行交互式控制，提供了两个命令行工具：

- callgrind_annotate —— 该命令读入配置文件数据，并打印排序的功能列表，可选地使用源注释。
对于数据的图形可视化，请尝试 KCachegrind，它是基于KDE / Qt的GUI，可以轻松导航Callgrind生成的大量数据。
- callgrind_control —— 此命令使您能够交互地观察和控制当前在Callgrind的控制下运行的程序的状态，而不停止程序。您可以获取统计信息以及当前的堆栈跟踪，您可以请求归零或转储配置文件数据。

**基础用法：**
```
valgrind --tool=callgrind [callgrind options] your-program [program options]
```
程序终止后，将生成一个名为的配置文件数据文件 callgrind.out.<pid> ，其中pid是要分析的程序的进程ID。数据文件包含有关执行功能的程序中进行的调用以及 指令读取（Ir）事件计数的信息。
要从配置文件数据文件生成功能函数摘要，请使用：
```
callgrind_annotate [options] callgrind.out.<pid>
```

**使用示例：**

0. 待分析的源程序代码如下：
```c
#include<iostream>
using namespace std;

void fun0(){
    int num = 0;
    for(int i=1;i<=100;i++){
        num += i;
    }
    cout<<"fun0 num="<<num<<endl;
}

void fun1(){
    int num = 0;
    for(int i=1;i<=10000;i++){
        num += i;
    }
    cout<<"fun1 num="<<num<<endl;
}

int main(){
    fun0();
    fun1();

    return 0;
}
```

1. 使用callgrind进行分析：
```
admin@PC:~/Works/study/valgrind$ valgrind --tool=callgrind ./main
==19516== Callgrind, a call-graph generating cache profiler
==19516== Copyright (C) 2002-2017, and GNU GPL'd, by Josef Weidendorfer et al.
==19516== Using Valgrind-3.13.0 and LibVEX; rerun with -h for copyright info
==19516== Command: ./main
==19516== 
==19516== For interactive control, run 'callgrind_control -h'.
fun0 num=5050
fun1 num=50005000
==19516== 
==19516== Events    : Ir
==19516== Collected : 2232495
==19516== 
==19516== I   refs:      2,232,495
admin@PC:~/Works/study/valgrind$ ls
atomic.cpp  callgrind.out.19516  main  main.cpp

```
可以看到新生成了callgrind.out.19516这个文件。

2. 使用callgrind_annotate解析callgrind.out文件。
```
admin@PC:~/Works/study/valgrind$ callgrind_annotate callgrind.out.19516 
--------------------------------------------------------------------------------
Profile data file 'callgrind.out.19516' (creator: callgrind-3.13.0)
--------------------------------------------------------------------------------
I1 cache: 
D1 cache: 
LL cache: 
Timerange: Basic block 0 - 374610
Trigger: Program termination
Profiled target:  ./main (PID 19516, part 1)
Events recorded:  Ir
Events shown:     Ir
Event sort order: Ir
Thresholds:       99
Include dirs:     
User annotated:   
Auto-annotation:  off

--------------------------------------------------------------------------------
       Ir 
--------------------------------------------------------------------------------
2,232,495  PROGRAM TOTALS

--------------------------------------------------------------------------------
     Ir  file:function
--------------------------------------------------------------------------------
924,371  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-lookup.c:_dl_lookup_symbol_x [/lib/x86_64-linux-gnu/ld-2.23.so]
568,053  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-lookup.c:do_lookup_x [/lib/x86_64-linux-gnu/ld-2.23.so]
251,994  /build/glibc-Cl5G7W/glibc-2.23/elf/../sysdeps/x86_64/dl-machine.h:_dl_relocate_object
110,536  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-lookup.c:check_match [/lib/x86_64-linux-gnu/ld-2.23.so]
 80,586  /build/glibc-Cl5G7W/glibc-2.23/elf/do-rel.h:_dl_relocate_object
 72,818  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/multiarch/../strcmp.S:strcmp [/lib/x86_64-linux-gnu/ld-2.23.so]
 60,024  main.cpp:fun1() [/home/sl/Works/study/valgrind/main]
 56,475  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-addr.c:_dl_addr [/lib/x86_64-linux-gnu/libc-2.23.so]
 12,304  ???:std::locale::_Impl::_M_install_facet(std::locale::id const*, std::locale::facet const*) [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  6,660  /build/glibc-Cl5G7W/glibc-2.23/elf/../elf/dl-runtime.c:_dl_fixup [/lib/x86_64-linux-gnu/ld-2.23.so]
  5,526  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-misc.c:_dl_name_match_p [/lib/x86_64-linux-gnu/ld-2.23.so]
  5,504  /build/glibc-Cl5G7W/glibc-2.23/wcsmbs/btowc.c:btowc [/lib/x86_64-linux-gnu/libc-2.23.so]
  4,284  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-version.c:_dl_check_map_versions [/lib/x86_64-linux-gnu/ld-2.23.so]
  4,250  /build/glibc-Cl5G7W/glibc-2.23/elf/../sysdeps/x86_64/dl-trampoline.h:_dl_runtime_resolve_avx'2 [/lib/x86_64-linux-gnu/ld-2.23.so]
  3,692  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-cache.c:_dl_cache_libcmp [/lib/x86_64-linux-gnu/ld-2.23.so]
  3,171  ???:std::ctype<wchar_t>::_M_initialize_ctype() [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  2,820  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-deps.c:_dl_map_object_deps [/lib/x86_64-linux-gnu/ld-2.23.so]
  2,496  ???:__dynamic_cast [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  2,134  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-load.c:_dl_map_object_from_fd [/lib/x86_64-linux-gnu/ld-2.23.so]
  2,130  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-version.c:match_symbol [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,841  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-load.c:_dl_map_object [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,806  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/strlen.S:strlen [/lib/x86_64-linux-gnu/libc-2.23.so]
  1,530  /build/glibc-Cl5G7W/glibc-2.23/elf/get-dynamic-info.h:_dl_map_object_from_fd
  1,360  ???:__cxxabiv1::__si_class_type_info::__do_dyncast(long, __cxxabiv1::__class_type_info::__sub_kind, __cxxabiv1::__class_type_info const*, void const*, __cxxabiv1::__class_type_info const*, void const*, __cxxabiv1::__class_type_info::__dyncast_result&) const [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  1,268  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-cache.c:_dl_load_cache_lookup [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,191  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-object.c:_dl_new_object [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,184  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/multiarch/../memcpy.S:memcpy [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,144  ???:std::locale::_Impl::_Impl(unsigned long) [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  1,128  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-fini.c:_dl_fini [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,104  ???:std::ctype<char>::_M_widen_init() const [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  1,026  /build/glibc-Cl5G7W/glibc-2.23/wctype/wctype_l.c:wctype_l [/lib/x86_64-linux-gnu/libc-2.23.so]
  1,021  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-minimal.c:__libc_memalign [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,012  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/multiarch/../memcmp.S:__GI_memcmp [/lib/x86_64-linux-gnu/libc-2.23.so]
  1,011  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-load.c:open_verify.constprop.7 [/lib/x86_64-linux-gnu/ld-2.23.so]
    964  ???:std::locale::id::_M_id() const [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
    922  /build/glibc-Cl5G7W/glibc-2.23/elf/rtld.c:dl_main [/lib/x86_64-linux-gnu/ld-2.23.so]
    896  /build/glibc-Cl5G7W/glibc-2.23/wcsmbs/./wcsmbsload.h:btowc
    871  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-reloc.c:_dl_relocate_object [/lib/x86_64-linux-gnu/ld-2.23.so]
    814  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/multiarch/../memset.S:memset [/lib/x86_64-linux-gnu/ld-2.23.so]
    768  /build/glibc-Cl5G7W/glibc-2.23/wcsmbs/wctob.c:wctob [/lib/x86_64-linux-gnu/libc-2.23.so]
    723  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/multiarch/../memcmp.S:bcmp [/lib/x86_64-linux-gnu/ld-2.23.so]
    651  /build/glibc-Cl5G7W/glibc-2.23/malloc/malloc.c:malloc_consolidate [/lib/x86_64-linux-gnu/libc-2.23.so]
    640  /build/glibc-Cl5G7W/glibc-2.23/iconv/gconv_simple.c:__gconv_btwoc_ascii [/lib/x86_64-linux-gnu/libc-2.23.so]
    636  main.cpp:fun0() [/home/sl/Works/study/valgrind/main]
    590  /build/glibc-Cl5G7W/glibc-2.23/misc/../sysdeps/unix/sysv/linux/wordsize-64/mmap.c:mmap [/lib/x86_64-linux-gnu/ld-2.23.so]
    552  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/cacheinfo.c:intel_check_word [/lib/x86_64-linux-gnu/libc-2.23.so]
    540  ???:__cxxabiv1::__vmi_class_type_info::__do_dyncast(long, __cxxabiv1::__class_type_info::__sub_kind, __cxxabiv1::__class_type_info const*, void const*, __cxxabiv1::__class_type_info const*, void const*, __cxxabiv1::__class_type_info::__dyncast_result&) const [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
    539  /build/glibc-Cl5G7W/glibc-2.23/elf/../elf/dl-sysdep.c:_dl_sysdep_start [/lib/x86_64-linux-gnu/ld-2.23.so]
    524  /build/glibc-Cl5G7W/glibc-2.23/elf/./dl-map-segments.h:_dl_map_object_from_fd
    512  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-profstub.c:_dl_mcount_wrapper_check [/lib/x86_64-linux-gnu/libc-2.23.so]
    483  /build/glibc-Cl5G7W/glibc-2.23/malloc/arena.c:ptmalloc_init.part.5 [/lib/x86_64-linux-gnu/libc-2.23.so]
    481  /build/glibc-Cl5G7W/glibc-2.23/elf/../string/bits/string2.h:handle_ld_preload
    480  ???:std::ios_base::ios_base() [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
    479  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-environ.c:_dl_next_ld_env_entry [/lib/x86_64-linux-gnu/ld-2.23.so]

```
它向你展示了每个函数所运行的指令数量，由高到低排序。
可以看到fun1函数的指令数量是fun0的100倍，从源码看也是如此。
```
60,024  main.cpp:fun1() [/home/sl/Works/study/valgrind/main]
636  main.cpp:fun0() [/home/sl/Works/study/valgrind/main]

```
3. Callgrind 注解源文件
```
admin@PC:~/Works/study/valgrind$ callgrind_annotate callgrind.out.19516 main.cpp 
--------------------------------------------------------------------------------
Profile data file 'callgrind.out.19516' (creator: callgrind-3.13.0)
--------------------------------------------------------------------------------
I1 cache: 
D1 cache: 
LL cache: 
Timerange: Basic block 0 - 374610
Trigger: Program termination
Profiled target:  ./main (PID 19516, part 1)
Events recorded:  Ir
Events shown:     Ir
Event sort order: Ir
Thresholds:       99
Include dirs:     
User annotated:   main.cpp
Auto-annotation:  off

--------------------------------------------------------------------------------
       Ir 
--------------------------------------------------------------------------------
2,232,495  PROGRAM TOTALS

--------------------------------------------------------------------------------
     Ir  file:function
--------------------------------------------------------------------------------
924,371  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-lookup.c:_dl_lookup_symbol_x [/lib/x86_64-linux-gnu/ld-2.23.so]
568,053  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-lookup.c:do_lookup_x [/lib/x86_64-linux-gnu/ld-2.23.so]
251,994  /build/glibc-Cl5G7W/glibc-2.23/elf/../sysdeps/x86_64/dl-machine.h:_dl_relocate_object
110,536  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-lookup.c:check_match [/lib/x86_64-linux-gnu/ld-2.23.so]
 80,586  /build/glibc-Cl5G7W/glibc-2.23/elf/do-rel.h:_dl_relocate_object
 72,818  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/multiarch/../strcmp.S:strcmp [/lib/x86_64-linux-gnu/ld-2.23.so]
 60,024  main.cpp:fun1() [/home/sl/Works/study/valgrind/main]
 56,475  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-addr.c:_dl_addr [/lib/x86_64-linux-gnu/libc-2.23.so]
 12,304  ???:std::locale::_Impl::_M_install_facet(std::locale::id const*, std::locale::facet const*) [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  6,660  /build/glibc-Cl5G7W/glibc-2.23/elf/../elf/dl-runtime.c:_dl_fixup [/lib/x86_64-linux-gnu/ld-2.23.so]
  5,526  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-misc.c:_dl_name_match_p [/lib/x86_64-linux-gnu/ld-2.23.so]
  5,504  /build/glibc-Cl5G7W/glibc-2.23/wcsmbs/btowc.c:btowc [/lib/x86_64-linux-gnu/libc-2.23.so]
  4,284  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-version.c:_dl_check_map_versions [/lib/x86_64-linux-gnu/ld-2.23.so]
  4,250  /build/glibc-Cl5G7W/glibc-2.23/elf/../sysdeps/x86_64/dl-trampoline.h:_dl_runtime_resolve_avx'2 [/lib/x86_64-linux-gnu/ld-2.23.so]
  3,692  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-cache.c:_dl_cache_libcmp [/lib/x86_64-linux-gnu/ld-2.23.so]
  3,171  ???:std::ctype<wchar_t>::_M_initialize_ctype() [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  2,820  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-deps.c:_dl_map_object_deps [/lib/x86_64-linux-gnu/ld-2.23.so]
  2,496  ???:__dynamic_cast [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  2,134  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-load.c:_dl_map_object_from_fd [/lib/x86_64-linux-gnu/ld-2.23.so]
  2,130  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-version.c:match_symbol [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,841  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-load.c:_dl_map_object [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,806  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/strlen.S:strlen [/lib/x86_64-linux-gnu/libc-2.23.so]
  1,530  /build/glibc-Cl5G7W/glibc-2.23/elf/get-dynamic-info.h:_dl_map_object_from_fd
  1,360  ???:__cxxabiv1::__si_class_type_info::__do_dyncast(long, __cxxabiv1::__class_type_info::__sub_kind, __cxxabiv1::__class_type_info const*, void const*, __cxxabiv1::__class_type_info const*, void const*, __cxxabiv1::__class_type_info::__dyncast_result&) const [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  1,268  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-cache.c:_dl_load_cache_lookup [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,191  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-object.c:_dl_new_object [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,184  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/multiarch/../memcpy.S:memcpy [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,144  ???:std::locale::_Impl::_Impl(unsigned long) [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  1,128  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-fini.c:_dl_fini [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,104  ???:std::ctype<char>::_M_widen_init() const [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
  1,026  /build/glibc-Cl5G7W/glibc-2.23/wctype/wctype_l.c:wctype_l [/lib/x86_64-linux-gnu/libc-2.23.so]
  1,021  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-minimal.c:__libc_memalign [/lib/x86_64-linux-gnu/ld-2.23.so]
  1,012  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/multiarch/../memcmp.S:__GI_memcmp [/lib/x86_64-linux-gnu/libc-2.23.so]
  1,011  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-load.c:open_verify.constprop.7 [/lib/x86_64-linux-gnu/ld-2.23.so]
    964  ???:std::locale::id::_M_id() const [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
    922  /build/glibc-Cl5G7W/glibc-2.23/elf/rtld.c:dl_main [/lib/x86_64-linux-gnu/ld-2.23.so]
    896  /build/glibc-Cl5G7W/glibc-2.23/wcsmbs/./wcsmbsload.h:btowc
    871  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-reloc.c:_dl_relocate_object [/lib/x86_64-linux-gnu/ld-2.23.so]
    814  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/multiarch/../memset.S:memset [/lib/x86_64-linux-gnu/ld-2.23.so]
    768  /build/glibc-Cl5G7W/glibc-2.23/wcsmbs/wctob.c:wctob [/lib/x86_64-linux-gnu/libc-2.23.so]
    723  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/multiarch/../memcmp.S:bcmp [/lib/x86_64-linux-gnu/ld-2.23.so]
    651  /build/glibc-Cl5G7W/glibc-2.23/malloc/malloc.c:malloc_consolidate [/lib/x86_64-linux-gnu/libc-2.23.so]
    640  /build/glibc-Cl5G7W/glibc-2.23/iconv/gconv_simple.c:__gconv_btwoc_ascii [/lib/x86_64-linux-gnu/libc-2.23.so]
    636  main.cpp:fun0() [/home/sl/Works/study/valgrind/main]
    590  /build/glibc-Cl5G7W/glibc-2.23/misc/../sysdeps/unix/sysv/linux/wordsize-64/mmap.c:mmap [/lib/x86_64-linux-gnu/ld-2.23.so]
    552  /build/glibc-Cl5G7W/glibc-2.23/string/../sysdeps/x86_64/cacheinfo.c:intel_check_word [/lib/x86_64-linux-gnu/libc-2.23.so]
    540  ???:__cxxabiv1::__vmi_class_type_info::__do_dyncast(long, __cxxabiv1::__class_type_info::__sub_kind, __cxxabiv1::__class_type_info const*, void const*, __cxxabiv1::__class_type_info const*, void const*, __cxxabiv1::__class_type_info::__dyncast_result&) const [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
    539  /build/glibc-Cl5G7W/glibc-2.23/elf/../elf/dl-sysdep.c:_dl_sysdep_start [/lib/x86_64-linux-gnu/ld-2.23.so]
    524  /build/glibc-Cl5G7W/glibc-2.23/elf/./dl-map-segments.h:_dl_map_object_from_fd
    512  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-profstub.c:_dl_mcount_wrapper_check [/lib/x86_64-linux-gnu/libc-2.23.so]
    483  /build/glibc-Cl5G7W/glibc-2.23/malloc/arena.c:ptmalloc_init.part.5 [/lib/x86_64-linux-gnu/libc-2.23.so]
    481  /build/glibc-Cl5G7W/glibc-2.23/elf/../string/bits/string2.h:handle_ld_preload
    480  ???:std::ios_base::ios_base() [/usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.21]
    479  /build/glibc-Cl5G7W/glibc-2.23/elf/dl-environ.c:_dl_next_ld_env_entry [/lib/x86_64-linux-gnu/ld-2.23.so]

--------------------------------------------------------------------------------
-- User-annotated source: main.cpp
--------------------------------------------------------------------------------
    Ir 

     .  #include<iostream>
     .  using namespace std;
     .  
     .  
     3  void fun0(){
     1      int num = 0;
   403      for(int i=1;i<=100;i++){
   200          num += i;
     .      }
    26      cout<<"fun0 num="<<num<<endl;
19,106  => /build/glibc-Cl5G7W/glibc-2.23/elf/../sysdeps/x86_64/dl-trampoline.h:_dl_runtime_resolve_avx'2 (3x)
     3  }
     .  
     3  void fun1(){
     1      int num = 0;
40,003      for(int i=1;i<=10000;i++){
20,000          num += i;
     .      }
    14      cout<<"fun1 num="<<num<<endl;
   344  => ???:std::ostream::operator<<(std::ostream& (*)(std::ostream&)) (1x)
   302  => ???:std::basic_ostream<char, std::char_traits<char> >& std::operator<< <std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*) (1x)
   499  => ???:std::ostream::operator<<(int) (1x)
     3  }
     .  
     2  int main(){
     1      fun0();
19,742  => main.cpp:fun0() (1x)
     1      fun1();
61,169  => main.cpp:fun1() (1x)
     .  
     1      return 0;
Negative repeat count does nothing at /usr/local/bin/callgrind_annotate line 828, <INPUTFILE> line 26.
    21  }102,068  => main.cpp:__static_initialization_and_destruction_0(int, int) (1x)

--------------------------------------------------------------------------------
Ir 
--------------------------------------------------------------------------------
 3  percentage of events annotated
```


>参考文档：[Callgrind: a call-graph generating cache and branch prediction profiler](http://valgrind.org/docs/manual/cl-manual.html)