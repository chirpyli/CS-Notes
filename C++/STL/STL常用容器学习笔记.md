这里简要的记述一下STL常用容器的实现原理，要点等内容。

### vector
`vector`是比较常用的stl容器，用法与数组是非类似，其内部实现是连续空间分配，与数组的不同之处在于可弹性增加空间，而`array`是静态空间，分配后不能动态扩展。`vecotr`的实现较为简单，主要的关键点在于当空间不足时，会新分配当前空间2倍的空间，将旧空间数据拷贝到新空间，然后删除旧空间。

使用时可使用`[]`，因为其已实现重载`[]`。
```c++
      reference
      operator[](size_type __n) _GLIBCXX_NOEXCEPT
      {
	__glibcxx_requires_subscript(__n);
	return *(this->_M_impl._M_start + __n);
      }
```

>使用时要注意迭代器失效问题，这个在很多STL容器中都有这个问题。


### list
链表`list`，在STL中，其实现的是双向链表，其节点的定义继承自`_List_node_base`，可以看到有前驱和后继指针，实现也较为简单。
```c++
    struct _List_node_base
    {
      _List_node_base* _M_next;
      _List_node_base* _M_prev;

      static void
      swap(_List_node_base& __x, _List_node_base& __y) _GLIBCXX_USE_NOEXCEPT;

      void
      _M_transfer(_List_node_base* const __first,
		  _List_node_base* const __last) _GLIBCXX_USE_NOEXCEPT;

      void
      _M_reverse() _GLIBCXX_USE_NOEXCEPT;

      void
      _M_hook(_List_node_base* const __position) _GLIBCXX_USE_NOEXCEPT;

      void
      _M_unhook() _GLIBCXX_USE_NOEXCEPT;
    };
```

### deque
