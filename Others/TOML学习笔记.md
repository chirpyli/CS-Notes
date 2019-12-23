常见的配置文件格式有ini，yaml，toml等，今天学习一下toml。TOML（Tom's Obvious, Minimal Language），因为它的作者是 GitHub联合创始人Tom Preston-Werner 。github地址：https://github.com/toml-lang/toml。

### ini,yaml,toml的对比
In some ways TOML is very similar to JSON: simple, well-specified, and maps easily to ubiquitous data types. <font color=Blue>JSON is great for serializing data that will mostly be read and written by computer programs. </font>Where <font color=Green>TOML differs from JSON is its emphasis on being easy for humans to read and write.</font> Comments are a good example: they serve no purpose when data is being sent from one program to another, but are very helpful in a configuration file that may be edited by hand.

The YAML format is oriented towards configuration files just like TOML. For many purposes, however, <font color=Green>YAML is an overly complex solution.</font> TOML aims for simplicity, a goal which is not apparent in the YAML specification: http://www.yaml.org/spec/1.2/spec.html

The INI format is also frequently used for configuration files. The format is not standardized, however, and usually does not handle more than one or two levels of nesting.
复杂度：yaml > toml > ini
综上，TOML是适合做配置文件的，易于书写，简单且能够表示较为复杂的配置。

### toml的目标，也是toml的特点
TOML 的目标是成为一个极简的配置文件格式。TOML 被设计成可以无歧义地被映射为哈希表，从而被多种语言解析。
*TOML aims to be a minimal configuration file format that's easy to read due to obvious semantics. TOML is designed to map unambiguously to a hash table. TOML should be easy to parse into data structures in a wide variety of languages.*

### 语法规范
最详细的都在这里，下面那个中文版是对应的翻译。

Gitbub ReadMe:https://github.com/toml-lang/toml/blob/master/README.md       
中文版：https://segmentfault.com/a/1190000000477752