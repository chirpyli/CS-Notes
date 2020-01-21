这里简要的记述一下STL常用容器的实现原理，要点等内容， 目前梳理的很差，后期再进行整理。

### vector
`vector`是比较常用的stl容器，用法与数组是非类似，其内部实现是连续空间分配，与数组的不同之处在于可弹性增加空间，而`array`是静态空间，分配后不能动态扩展。`vecotr`的实现较为简单，主要的关键点在于当空间不足时，会新分配当前空间2倍的空间，将旧空间数据拷贝到新空间，然后删除旧空间。
```c++
struct _Vector_impl: public _Tp_alloc_type {
    pointer _M_start;   // 元素头
    pointer _M_finish;  // 元素尾
    pointer _M_end_of_storage;  // 可用空间尾，
          // 省略部分代码...
};
```
这个是向尾部添加元素的代码实现，可以看到如果当前还有剩余空间的话，直接在尾部添加，如果没有剩余空间，则会动态扩展。
```c++
void push_back(const value_type& __x) {
    if (this->_M_impl._M_finish != this->_M_impl._M_end_of_storage) {
	    _Alloc_traits::construct(this->_M_impl, this->_M_impl._M_finish, __x);
	    ++this->_M_impl._M_finish;
    } else
	  _M_realloc_insert(end(), __x);
}

template<typename _Tp, typename _Alloc>
void vector<_Tp, _Alloc>::_M_realloc_insert(iterator __position, const _Tp& __x) {
    const size_type __len = _M_check_len(size_type(1), "vector::_M_realloc_insert");      // 2倍当前大小
    const size_type __elems_before = __position - begin();
    pointer __new_start(this->_M_allocate(__len));
    pointer __new_finish(__new_start);
    __try {
	  // The order of the three operations is dictated by the C++11
	  // case, where the moves could alter a new element belonging
	  // to the existing vector.  This is an issue only for callers
	  // taking the element by lvalue ref (see last bullet of C++11
	  // [res.on.arguments]).
	  _Alloc_traits::construct(this->_M_impl, __new_start + __elems_before, __x);
	  __new_finish = pointer();

	  __new_finish = std::__uninitialized_move_if_noexcept_a(this->_M_impl._M_start, __position.base(), __new_start, _M_get_Tp_allocator();

	  ++__new_finish;

	  __new_finish = std::__uninitialized_move_if_noexcept_a(__position.base(), this->_M_impl._M_finish, __new_finish, _M_get_Tp_allocator());
	}__catch(...) {
	  if (!__new_finish)
	    _Alloc_traits::destroy(this->_M_impl,
				   __new_start + __elems_before);
	  else
	    std::_Destroy(__new_start, __new_finish, _M_get_Tp_allocator());
	  _M_deallocate(__new_start, __len);
	  __throw_exception_again;
	}
      std::_Destroy(this->_M_impl._M_start, this->_M_impl._M_finish, _M_get_Tp_allocator());
      _M_deallocate(this->_M_impl._M_start, this->_M_impl._M_end_of_storage - this->_M_impl._M_start);
      this->_M_impl._M_start = __new_start;
      this->_M_impl._M_finish = __new_finish;
      this->_M_impl._M_end_of_storage = __new_start + __len;
    }

// Called by _M_fill_insert, _M_insert_aux etc.
size_type _M_check_len(size_type __n, const char* __s) const {
    if (max_size() - size() < __n)
	  __throw_length_error(__N(__s));

    const size_type __len = size() + std::max(size(), __n);       // 二倍长
    return (__len < size() || __len > max_size()) ? max_size() : __len;
}

pointer _M_allocate(size_t __n) {
    typedef __gnu_cxx::__alloc_traits<_Tp_alloc_type> _Tr;
    return __n != 0 ? _Tr::allocate(_M_impl, __n) : pointer();
}
```
```c++

```
使用时可使用`[]`，因为其已实现重载`[]`。
```c++
      reference
      operator[](size_type __n) _GLIBCXX_NOEXCEPT {
	      __glibcxx_requires_subscript(__n);
	      return *(this->_M_impl._M_start + __n);
      }
```

>使用时要注意迭代器失效问题，这个在很多STL容器中都有这个问题。

### list
链表`list`，与`vector`不同，元素在内存中不连续分配，不支持随机存取，好处就是插入与删除时间复杂度为`O(1)`。在STL中，其实现的是双向链表，其节点的定义可以看到有前驱和后继指针，实现也较为简单。
```c++
  /// An actual node in the %list.
template<typename _Tp>
struct _List_node : public __detail::_List_node_base {
    _Tp _M_data;
    _Tp* _M_valptr() { return std::__addressof(_M_data); }
    _Tp const* _M_valptr() const { return std::__addressof(_M_data); }
};

struct _List_node_base {
    _List_node_base* _M_next;
    _List_node_base* _M_prev;

    static void swap(_List_node_base& __x, _List_node_base& __y) _GLIBCXX_USE_NOEXCEPT;
    void _M_transfer(_List_node_base* const __first, _List_node_base* const __last) _GLIBCXX_USE_NOEXCEPT;
    void _M_reverse() _GLIBCXX_USE_NOEXCEPT;
    void _M_hook(_List_node_base* const __position) _GLIBCXX_USE_NOEXCEPT;
    void _M_unhook() _GLIBCXX_USE_NOEXCEPT;
};
```

### deque
双端队列，具体实现不同于`vector`与`list`，它是一小段一小段连续空间，每段连续空间之间通过指针数组（这个数组中存放的是每个连续空间数组的头指针）串联起来，这样就能访问到所有元素。我们摘取部分源码看一下其实现细节。双端队列的迭代器实现代码如下（相较于`vector`与`list`，对元素的访问因为其存储布局不同，在每一段连续分配空间的边缘要做特殊处理）：
```c++
#define _GLIBCXX_DEQUE_BUF_SIZE 512       // 默认连续空间大小

  _GLIBCXX_CONSTEXPR inline size_t __deque_buf_size(size_t __size) { 
        return (__size < _GLIBCXX_DEQUE_BUF_SIZE ? size_t(_GLIBCXX_DEQUE_BUF_SIZE / __size) :size_t(1)); 
    }
template<typename _Tp, typename _Ref, typename _Ptr>
    struct _Deque_iterator {
      typedef _Deque_iterator<_Tp, _Tp&, _Tp*>	     iterator;
      typedef _Deque_iterator<_Tp, const _Tp&, const _Tp*> const_iterator;
      typedef _Tp*					 _Elt_pointer;
      typedef _Tp**					_Map_pointer;

      static size_t _S_buffer_size() _GLIBCXX_NOEXCEPT { 
            return __deque_buf_size(sizeof(_Tp)); 
      }

      typedef std::random_access_iterator_tag	iterator_category;
      typedef _Tp				value_type;
      typedef _Ptr				pointer;
      typedef _Ref				reference;
      typedef size_t				size_type;
      typedef ptrdiff_t				difference_type;
      typedef _Deque_iterator			_Self;

      _Elt_pointer _M_cur;          // 当前位置
      _Elt_pointer _M_first;        // 每一小段空间的开始
      _Elt_pointer _M_last;         // 每一小段空间的结束
      _Map_pointer _M_node;         // 指针数组，可通过这里访问到所有连续存储空间片段

      // 构造函数
      _Deque_iterator(_Elt_pointer __x, _Map_pointer __y) _GLIBCXX_NOEXCEPT : _M_cur(__x), _M_first(*__y),
_M_last(*__y + _S_buffer_size()), _M_node(__y) { }

      _Deque_iterator() _GLIBCXX_NOEXCEPT: _M_cur(), _M_first(), _M_last(), _M_node() { }

      _Deque_iterator(const iterator& __x) _GLIBCXX_NOEXCEPT: _M_cur(__x._M_cur), _M_first(__x._M_first),
	_M_last(__x._M_last), _M_node(__x._M_node) { }

      iterator _M_const_cast() const _GLIBCXX_NOEXCEPT {
          return iterator(_M_cur, _M_node);     // 返回当前元素迭代器
      }

      reference operator*() const _GLIBCXX_NOEXCEPT { 
          return *_M_cur; 
      }

      pointer operator->() const _GLIBCXX_NOEXCEPT { 
          return _M_cur; 
      }

      // 重载++运算符，可以看到，当_M_cur指向本段连续空间尾部时，访问下一个元素的话是下一段连续空间的首地址
      _Self& operator++() _GLIBCXX_NOEXCEPT {   
	    ++_M_cur;     
	    if (_M_cur == _M_last) {  
	        _M_set_node(_M_node + 1);   // 移向下一段连续存储空间
	        _M_cur = _M_first;          // 下一段连续空间的首元素
	    }
	    return *this;
      }

      _Self operator++(int) _GLIBCXX_NOEXCEPT {
	    _Self __tmp = *this;
	    ++*this;
	    return __tmp;
      }

      _Self& operator--() _GLIBCXX_NOEXCEPT {
	    if (_M_cur == _M_first) {             // 与++类似，如果当前是第一个元素，--时，就应该调到上一个连续存储空间
	        _M_set_node(_M_node - 1);
	        _M_cur = _M_last;           // 移到上一段空间的最后，
	    }
	    --_M_cur;                       // 因为是[start, last)区间，这里要--_M_cur；
	    return *this;
      }

      _Self operator--(int) _GLIBCXX_NOEXCEPT {
	    _Self __tmp = *this;
	    --*this;
	    return __tmp;
      }

      _Self& operator+=(difference_type __n) _GLIBCXX_NOEXCEPT {
	    const difference_type __offset = __n + (_M_cur - _M_first);
	    if (__offset >= 0 && __offset < difference_type(_S_buffer_size()))  // 如果当前连续空间满足
	       _M_cur += __n;
	    else {  // 如果当前段连续空间不够用了，需要计算跳到连续空间
	        const difference_type __node_offset = __offset > 0 ? __offset / difference_type(_S_buffer_size()) : -difference_type((-__offset - 1) / _S_buffer_size()) - 1;
	        _M_set_node(_M_node + __node_offset);
	        _M_cur = _M_first + (__offset - __node_offset * difference_type(_S_buffer_size()));
	    }
	    return *this;
      }

      _Self operator+(difference_type __n) const _GLIBCXX_NOEXCEPT {
	    _Self __tmp = *this;
	    return __tmp += __n;
      }

      _Self& operator-=(difference_type __n) _GLIBCXX_NOEXCEPT {
          return *this += -__n; }

      _Self operator-(difference_type __n) const _GLIBCXX_NOEXCEPT {
	    _Self __tmp = *this;
	    return __tmp -= __n;
      }

      reference operator[](difference_type __n) const _GLIBCXX_NOEXCEPT { return *(*this + __n); }

      // Prepares to traverse new_node.  Sets everything except _M_cur, which should therefore be set by the caller immediately afterwards, based on _M_first and _M_last.
      void _M_set_node(_Map_pointer __new_node) _GLIBCXX_NOEXCEPT {     // 跳到新的一段连续存储空间
	    _M_node = __new_node;
	    _M_first = *__new_node;
	    _M_last = _M_first + difference_type(_S_buffer_size());
      }
    };
```
从上面`deque`迭代器的实现来看，主要需要注意的地方就是每段连续空间的边缘。看完迭代器后，我们看一下`deque`类的实现代码，这里删减掉大部分代码，保留部分代码。其中重点看一下`deque`中最常用的`push_front`、`pop_front`与`push_back`、`pop_back`的实现。`push_back`时间复杂度`O(1)`比较好理解，过程类似于`vector`，但`push_front`为什么也是`O(1)`呢？如果在头部插入一个元素，第一个连续空间距离起始`start`还有剩余空间的的话，直接插入就好了，如果没有剩余空间的话，就创建一段新的连续空间，将首地址放到`map`中，如果`map`没有空间放置这个首地址，就调整`map`，再插入首地址，详细过程请看源码的具体实现：
```c++
 template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
    class deque : protected _Deque_base<_Tp, _Alloc> {
      typedef _Deque_base<_Tp, _Alloc>			_Base;
      typedef typename _Base::_Tp_alloc_type		_Tp_alloc_type;
      typedef typename _Base::_Alloc_traits		_Alloc_traits;
      typedef typename _Base::_Map_pointer		_Map_pointer;

    public:
      typedef _Tp					value_type;
      typedef typename _Alloc_traits::pointer		pointer;
      typedef typename _Alloc_traits::const_pointer	const_pointer;
      typedef typename _Alloc_traits::reference		reference;
      typedef typename _Alloc_traits::const_reference	const_reference;
      typedef typename _Base::iterator			iterator;
      typedef typename _Base::const_iterator		const_iterator;
      typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;
      typedef std::reverse_iterator<iterator>		reverse_iterator;
      typedef size_t					size_type;
      typedef ptrdiff_t					difference_type;
      typedef _Alloc					allocator_type;

    protected:
      static size_t _S_buffer_size() _GLIBCXX_NOEXCEPT { return __deque_buf_size(sizeof(_Tp)); }

      // Functions controlling memory layout, and nothing else.
      using _Base::_M_initialize_map;
      using _Base::_M_create_nodes;
      using _Base::_M_destroy_nodes;
      using _Base::_M_allocate_node;
      using _Base::_M_deallocate_node;
      using _Base::_M_allocate_map;
      using _Base::_M_deallocate_map;
      using _Base::_M_get_Tp_allocator;

      /**
       *  A total of four data members accumulated down the hierarchy.
       *  May be accessed via _M_impl.*
       */
      using _Base::_M_impl;

    public:
	  // 省略构造函数与析构函数......

      /*
       *  @brief  Assigns a given value to a %deque.
       *  @param  __n  Number of elements to be assigned.
       *  @param  __val  Value to be assigned.
       *
       *  This function fills a %deque with @a n copies of the given
       *  value.  Note that the assignment completely changes the
       *  %deque and that the resulting %deque's size is the same as
       *  the number of elements assigned.
       */
      void assign(size_type __n, const value_type& __val) { _M_fill_assign(__n, __val); }

	  // 省略其他assign重载函数......	


      /// Get a copy of the memory allocation object.
      allocator_type get_allocator() const _GLIBCXX_NOEXCEPT{ return _Base::get_allocator(); }

      // iterators
      /**
       *  Returns a read/write iterator that points to the first element in the
       *  %deque.  Iteration is done in ordinary element order.
       */
      iterator begin() _GLIBCXX_NOEXCEPT { return this->_M_impl._M_start; }
      const_iterator begin() const _GLIBCXX_NOEXCEPT { return this->_M_impl._M_start; }

      /**
       *  Returns a read/write iterator that points one past the last
       *  element in the %deque.  Iteration is done in ordinary
       *  element order.
       */
      iterator end() _GLIBCXX_NOEXCEPT{ return this->_M_impl._M_finish; }
      const_iterator end() const _GLIBCXX_NOEXCEPT { return this->_M_impl._M_finish; }


	// 省略其他迭代器相关代码......

      // [23.2.1.2] capacity
      /**  Returns the number of elements in the %deque.  */
      size_type size() const _GLIBCXX_NOEXCEPT { return this->_M_impl._M_finish - this->_M_impl._M_start; }

      /**  Returns the size() of the largest possible %deque.  */
      size_type max_size() const _GLIBCXX_NOEXCEPT { return _Alloc_traits::max_size(_M_get_Tp_allocator()); }

      /**
       *  @brief  Resizes the %deque to the specified number of elements.
       *  @param  __new_size  Number of elements the %deque should contain.
       *
       *  This function will %resize the %deque to the specified
       *  number of elements.  If the number is smaller than the
       *  %deque's current size the %deque is truncated, otherwise
       *  default constructed elements are appended.
       */
      void resize(size_type __new_size) {
		  const size_type __len = size();
		  if (__new_size > __len)
	  		  _M_default_append(__new_size - __len);
		  else if (__new_size < __len)
	  		  _M_erase_at_end(this->_M_impl._M_start + difference_type(__new_size));
      }

#if __cplusplus >= 201103L
      /**  A non-binding request to reduce memory use.  */
      void shrink_to_fit() noexcept { _M_shrink_to_fit(); }
#endif

      /**
       *  Returns true if the %deque is empty.  (Thus begin() would
       *  equal end().)
       */
      bool empty() const _GLIBCXX_NOEXCEPT { return this->_M_impl._M_finish == this->_M_impl._M_start; }

      // element access
      /**
       *  @brief Subscript access to the data contained in the %deque.
       *  @param __n The index of the element for which data should be
       *  accessed.
       *  @return  Read/write reference to data.
       *
       *  This operator allows for easy, array-style, data access.
       *  Note that data access with this operator is unchecked and
       *  out_of_range lookups are not defined. (For checked lookups
       *  see at().)
       */
      reference operator[](size_type __n) _GLIBCXX_NOEXCEPT {
		  __glibcxx_requires_subscript(__n);
		  return this->_M_impl._M_start[difference_type(__n)];
      }

    protected:
      /// Safety check used only from at().
      void _M_range_check(size_type __n) const {
		  if (__n >= this->size())
	  		  __throw_out_of_range_fmt(__N("deque::_M_range_check: __n "
				       "(which is %zu)>= this->size() "
				       "(which is %zu)"), __n, this->size());
      }

    public:
      /**
       *  @brief  Provides access to the data contained in the %deque.
       *  @param __n The index of the element for which data should be
       *  accessed.
       *  @return  Read/write reference to data.
       *  @throw  std::out_of_range  If @a __n is an invalid index.
       *
       *  This function provides for safer data access.  The parameter
       *  is first checked that it is in the range of the deque.  The
       *  function throws out_of_range if the check fails.
       */
      reference at(size_type __n) {
		  _M_range_check(__n);
		  return (*this)[__n];
      }

      /**
       *  @brief  Provides access to the data contained in the %deque.
       *  @param __n The index of the element for which data should be
       *  accessed.
       *  @return  Read-only (constant) reference to data.
       *  @throw  std::out_of_range  If @a __n is an invalid index.
       *
       *  This function provides for safer data access.  The parameter is first
       *  checked that it is in the range of the deque.  The function throws
       *  out_of_range if the check fails.
       */
      const_reference at(size_type __n) const {
		  _M_range_check(__n);
		  return (*this)[__n];
      }

      /**
       *  Returns a read/write reference to the data at the first
       *  element of the %deque.
       */
      reference front() _GLIBCXX_NOEXCEPT {
		  __glibcxx_requires_nonempty();
		  return *begin();
      }

      /**
       *  Returns a read/write reference to the data at the last element of the
       *  %deque.
       */
      reference back() _GLIBCXX_NOEXCEPT {
		  __glibcxx_requires_nonempty();
		  iterator __tmp = end();
		  --__tmp;
		  return *__tmp;
      }


      /**
       *  @brief  Add data to the front of the %deque.
       *  @param  __x  Data to be added.
       *
       *  This is a typical stack operation.  The function creates an
       *  element at the front of the %deque and assigns the given
       *  data to it.  Due to the nature of a %deque this operation
       *  can be done in constant time.
       */
      void push_front(const value_type& __x) {		// 如果第一段连续空间头部还有剩余空间的话，直接插入元素
		  if (this->_M_impl._M_start._M_cur != this->_M_impl._M_start._M_first) {
	      	_Alloc_traits::construct(this->_M_impl, this->_M_impl._M_start._M_cur - 1, __x);
	      	--this->_M_impl._M_start._M_cur;
	  	  } else		// 如果没有，在前部重新分配空间
	    	  _M_push_front_aux(__x);
      }

      /**
       *  @brief  Add data to the end of the %deque.
       *  @param  __x  Data to be added.
       *
       *  This is a typical stack operation.  The function creates an
       *  element at the end of the %deque and assigns the given data
       *  to it.  Due to the nature of a %deque this operation can be
       *  done in constant time.
       */
      void push_back(const value_type& __x) {
		  if (this->_M_impl._M_finish._M_cur != this->_M_impl._M_finish._M_last - 1) {
	    	  _Alloc_traits::construct(this->_M_impl, this->_M_impl._M_finish._M_cur, __x);
	          ++this->_M_impl._M_finish._M_cur;
	  	  } else 
			_M_push_back_aux(__x);
      }

      /**
       *  @brief  Removes first element.
       *
       *  This is a typical stack operation.  It shrinks the %deque by one.
       *
       *  Note that no data is returned, and if the first element's data is
       *  needed, it should be retrieved before pop_front() is called.
       */
      void pop_front() _GLIBCXX_NOEXCEPT {
		  __glibcxx_requires_nonempty();
		  if (this->_M_impl._M_start._M_cur != this->_M_impl._M_start._M_last - 1) {
	    	  _Alloc_traits::destroy(this->_M_impl,	this->_M_impl._M_start._M_cur);
	    	  ++this->_M_impl._M_start._M_cur;
	  	  } else
	  	  _M_pop_front_aux();
      }

      /**
       *  @brief  Removes last element.
       *
       *  This is a typical stack operation.  It shrinks the %deque by one.
       *
       *  Note that no data is returned, and if the last element's data is
       *  needed, it should be retrieved before pop_back() is called.
       */
      void pop_back() _GLIBCXX_NOEXCEPT {
		  __glibcxx_requires_nonempty();
		  if (this->_M_impl._M_finish._M_cur != this->_M_impl._M_finish._M_first) {
	          --this->_M_impl._M_finish._M_cur;
	    	  _Alloc_traits::destroy(this->_M_impl, this->_M_impl._M_finish._M_cur);
	  	  } else
	  	  _M_pop_back_aux();
      }

#if __cplusplus >= 201103L
      /**
       *  @brief  Inserts an object in %deque before specified iterator.
       *  @param  __position  A const_iterator into the %deque.
       *  @param  __args  Arguments.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert an object of type T constructed
       *  with T(std::forward<Args>(args)...) before the specified location.
       */
      template<typename... _Args>
	iterator
	emplace(const_iterator __position, _Args&&... __args);

      /**
       *  @brief  Inserts given value into %deque before specified iterator.
       *  @param  __position  A const_iterator into the %deque.
       *  @param  __x  Data to be inserted.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert a copy of the given value before the
       *  specified location.
       */
      iterator
      insert(const_iterator __position, const value_type& __x);
#else
      /**
       *  @brief  Inserts given value into %deque before specified iterator.
       *  @param  __position  An iterator into the %deque.
       *  @param  __x  Data to be inserted.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert a copy of the given value before the
       *  specified location.
       */
      iterator
      insert(iterator __position, const value_type& __x);
#endif

#if __cplusplus >= 201103L
      /**
       *  @brief  Inserts given rvalue into %deque before specified iterator.
       *  @param  __position  A const_iterator into the %deque.
       *  @param  __x  Data to be inserted.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert a copy of the given rvalue before the
       *  specified location.
       */
      iterator
      insert(const_iterator __position, value_type&& __x)
      { return emplace(__position, std::move(__x)); }

      /**
       *  @brief  Inserts an initializer list into the %deque.
       *  @param  __p  An iterator into the %deque.
       *  @param  __l  An initializer_list.
       *
       *  This function will insert copies of the data in the
       *  initializer_list @a __l into the %deque before the location
       *  specified by @a __p.  This is known as <em>list insert</em>.
       */
      iterator
      insert(const_iterator __p, initializer_list<value_type> __l)
      {
	auto __offset = __p - cbegin();
	_M_range_insert_aux(__p._M_const_cast(), __l.begin(), __l.end(),
			    std::random_access_iterator_tag());
	return begin() + __offset;
      }
#endif

#if __cplusplus >= 201103L
      /**
       *  @brief  Inserts a number of copies of given data into the %deque.
       *  @param  __position  A const_iterator into the %deque.
       *  @param  __n  Number of elements to be inserted.
       *  @param  __x  Data to be inserted.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert a specified number of copies of the given
       *  data before the location specified by @a __position.
       */
      iterator
      insert(const_iterator __position, size_type __n, const value_type& __x)
      {
	difference_type __offset = __position - cbegin();
	_M_fill_insert(__position._M_const_cast(), __n, __x);
	return begin() + __offset;
      }
#else
      /**
       *  @brief  Inserts a number of copies of given data into the %deque.
       *  @param  __position  An iterator into the %deque.
       *  @param  __n  Number of elements to be inserted.
       *  @param  __x  Data to be inserted.
       *
       *  This function will insert a specified number of copies of the given
       *  data before the location specified by @a __position.
       */
      void
      insert(iterator __position, size_type __n, const value_type& __x)
      { _M_fill_insert(__position, __n, __x); }
#endif

#if __cplusplus >= 201103L
      /**
       *  @brief  Inserts a range into the %deque.
       *  @param  __position  A const_iterator into the %deque.
       *  @param  __first  An input iterator.
       *  @param  __last   An input iterator.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert copies of the data in the range
       *  [__first,__last) into the %deque before the location specified
       *  by @a __position.  This is known as <em>range insert</em>.
       */
      template<typename _InputIterator,
	       typename = std::_RequireInputIter<_InputIterator>>
	iterator
	insert(const_iterator __position, _InputIterator __first,
	       _InputIterator __last)
	{
	  difference_type __offset = __position - cbegin();
	  _M_insert_dispatch(__position._M_const_cast(),
			     __first, __last, __false_type());
	  return begin() + __offset;
	}
#else
      /**
       *  @brief  Inserts a range into the %deque.
       *  @param  __position  An iterator into the %deque.
       *  @param  __first  An input iterator.
       *  @param  __last   An input iterator.
       *
       *  This function will insert copies of the data in the range
       *  [__first,__last) into the %deque before the location specified
       *  by @a __position.  This is known as <em>range insert</em>.
       */
      template<typename _InputIterator>
	void
	insert(iterator __position, _InputIterator __first,
	       _InputIterator __last)
	{
	  // Check whether it's an integral type.  If so, it's not an iterator.
	  typedef typename std::__is_integer<_InputIterator>::__type _Integral;
	  _M_insert_dispatch(__position, __first, __last, _Integral());
	}
#endif

      /**
       *  @brief  Remove element at given position.
       *  @param  __position  Iterator pointing to element to be erased.
       *  @return  An iterator pointing to the next element (or end()).
       *
       *  This function will erase the element at the given position and thus
       *  shorten the %deque by one.
       *
       *  The user is cautioned that
       *  this function only erases the element, and that if the element is
       *  itself a pointer, the pointed-to memory is not touched in any way.
       *  Managing the pointer is the user's responsibility.
       */
      iterator
#if __cplusplus >= 201103L
      erase(const_iterator __position)
#else
      erase(iterator __position)
#endif
      { return _M_erase(__position._M_const_cast()); }

      /**
       *  @brief  Remove a range of elements.
       *  @param  __first  Iterator pointing to the first element to be erased.
       *  @param  __last  Iterator pointing to one past the last element to be
       *                erased.
       *  @return  An iterator pointing to the element pointed to by @a last
       *           prior to erasing (or end()).
       *
       *  This function will erase the elements in the range
       *  [__first,__last) and shorten the %deque accordingly.
       *
       *  The user is cautioned that
       *  this function only erases the elements, and that if the elements
       *  themselves are pointers, the pointed-to memory is not touched in any
       *  way.  Managing the pointer is the user's responsibility.
       */
      iterator
#if __cplusplus >= 201103L
      erase(const_iterator __first, const_iterator __last)
#else
      erase(iterator __first, iterator __last)
#endif
      { return _M_erase(__first._M_const_cast(), __last._M_const_cast()); }

      /**
       *  @brief  Swaps data with another %deque.
       *  @param  __x  A %deque of the same element and allocator types.
       *
       *  This exchanges the elements between two deques in constant time.
       *  (Four pointers, so it should be quite fast.)
       *  Note that the global std::swap() function is specialized such that
       *  std::swap(d1,d2) will feed to this function.
       *
       *  Whether the allocators are swapped depends on the allocator traits.
       */
      void
      swap(deque& __x) _GLIBCXX_NOEXCEPT
      {
#if __cplusplus >= 201103L
	__glibcxx_assert(_Alloc_traits::propagate_on_container_swap::value
			 || _M_get_Tp_allocator() == __x._M_get_Tp_allocator());
#endif
	_M_impl._M_swap_data(__x._M_impl);
	_Alloc_traits::_S_on_swap(_M_get_Tp_allocator(),
				  __x._M_get_Tp_allocator());
      }

      /**
       *  Erases all the elements.  Note that this function only erases the
       *  elements, and that if the elements themselves are pointers, the
       *  pointed-to memory is not touched in any way.  Managing the pointer is
       *  the user's responsibility.
       */
      void
      clear() _GLIBCXX_NOEXCEPT
      { _M_erase_at_end(begin()); }

    protected:
      // Internal constructor functions follow.

      // called by the range constructor to implement [23.1.1]/9

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 438. Ambiguity in the "do the right thing" clause
      template<typename _Integer>
	void
	_M_initialize_dispatch(_Integer __n, _Integer __x, __true_type)
	{
	  _M_initialize_map(static_cast<size_type>(__n));
	  _M_fill_initialize(__x);
	}

      // called by the range constructor to implement [23.1.1]/9
      template<typename _InputIterator>
	void
	_M_initialize_dispatch(_InputIterator __first, _InputIterator __last,
			       __false_type)
	{
	  _M_range_initialize(__first, __last,
			      std::__iterator_category(__first));
	}

      // called by the second initialize_dispatch above
      //@{
      /**
       *  @brief Fills the deque with whatever is in [first,last).
       *  @param  __first  An input iterator.
       *  @param  __last  An input iterator.
       *  @return   Nothing.
       *
       *  If the iterators are actually forward iterators (or better), then the
       *  memory layout can be done all at once.  Else we move forward using
       *  push_back on each value from the iterator.
       */
      template<typename _InputIterator>
	void
	_M_range_initialize(_InputIterator __first, _InputIterator __last,
			    std::input_iterator_tag);

      // called by the second initialize_dispatch above
      template<typename _ForwardIterator>
	void
	_M_range_initialize(_ForwardIterator __first, _ForwardIterator __last,
			    std::forward_iterator_tag);
      //@}

      /**
       *  @brief Fills the %deque with copies of value.
       *  @param  __value  Initial value.
       *  @return   Nothing.
       *  @pre _M_start and _M_finish have already been initialized,
       *  but none of the %deque's elements have yet been constructed.
       *
       *  This function is called only when the user provides an explicit size
       *  (with or without an explicit exemplar value).
       */
      void
      _M_fill_initialize(const value_type& __value);

#if __cplusplus >= 201103L
      // called by deque(n).
      void
      _M_default_initialize();
#endif

      // Internal assign functions follow.  The *_aux functions do the actual
      // assignment work for the range versions.

      // called by the range assign to implement [23.1.1]/9

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 438. Ambiguity in the "do the right thing" clause
      template<typename _Integer>
	void
	_M_assign_dispatch(_Integer __n, _Integer __val, __true_type)
	{ _M_fill_assign(__n, __val); }

      // called by the range assign to implement [23.1.1]/9
      template<typename _InputIterator>
	void
	_M_assign_dispatch(_InputIterator __first, _InputIterator __last,
			   __false_type)
	{ _M_assign_aux(__first, __last, std::__iterator_category(__first)); }

      // called by the second assign_dispatch above
      template<typename _InputIterator>
	void
	_M_assign_aux(_InputIterator __first, _InputIterator __last,
		      std::input_iterator_tag);

      // called by the second assign_dispatch above
      template<typename _ForwardIterator>
	void
	_M_assign_aux(_ForwardIterator __first, _ForwardIterator __last,
		      std::forward_iterator_tag)
	{
	  const size_type __len = std::distance(__first, __last);
	  if (__len > size())
	    {
	      _ForwardIterator __mid = __first;
	      std::advance(__mid, size());
	      std::copy(__first, __mid, begin());
	      _M_range_insert_aux(end(), __mid, __last,
				  std::__iterator_category(__first));
	    }
	  else
	    _M_erase_at_end(std::copy(__first, __last, begin()));
	}

      // Called by assign(n,t), and the range assign when it turns out
      // to be the same thing.
      void
      _M_fill_assign(size_type __n, const value_type& __val)
      {
	if (__n > size())
	  {
	    std::fill(begin(), end(), __val);
	    _M_fill_insert(end(), __n - size(), __val);
	  }
	else
	  {
	    _M_erase_at_end(begin() + difference_type(__n));
	    std::fill(begin(), end(), __val);
	  }
      }

      void _M_push_back_aux(const value_type&);
      void _M_push_front_aux(const value_type&);
      void _M_pop_back_aux();
      void _M_pop_front_aux();
      //@}

      // Internal insert functions follow.  The *_aux functions do the actual
      // insertion work when all shortcuts fail.

      // called by the range insert to implement [23.1.1]/9

      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 438. Ambiguity in the "do the right thing" clause
      template<typename _Integer>
	void
	_M_insert_dispatch(iterator __pos,
			   _Integer __n, _Integer __x, __true_type)
	{ _M_fill_insert(__pos, __n, __x); }

      // called by the range insert to implement [23.1.1]/9
      template<typename _InputIterator>
	void
	_M_insert_dispatch(iterator __pos,
			   _InputIterator __first, _InputIterator __last,
			   __false_type)
	{
	  _M_range_insert_aux(__pos, __first, __last,
			      std::__iterator_category(__first));
	}

      // called by the second insert_dispatch above
      template<typename _InputIterator>
	void
	_M_range_insert_aux(iterator __pos, _InputIterator __first,
			    _InputIterator __last, std::input_iterator_tag);

      // called by the second insert_dispatch above
      template<typename _ForwardIterator>
	void
	_M_range_insert_aux(iterator __pos, _ForwardIterator __first,
			    _ForwardIterator __last, std::forward_iterator_tag);

      // Called by insert(p,n,x), and the range insert when it turns out to be
      // the same thing.  Can use fill functions in optimal situations,
      // otherwise passes off to insert_aux(p,n,x).
      void
      _M_fill_insert(iterator __pos, size_type __n, const value_type& __x);

      // called by insert(p,x)
#if __cplusplus < 201103L
      iterator
      _M_insert_aux(iterator __pos, const value_type& __x);
#else
      template<typename... _Args>
	iterator
	_M_insert_aux(iterator __pos, _Args&&... __args);
#endif

      // called by insert(p,n,x) via fill_insert
      void
      _M_insert_aux(iterator __pos, size_type __n, const value_type& __x);

      // called by range_insert_aux for forward iterators
      template<typename _ForwardIterator>
	void
	_M_insert_aux(iterator __pos,
		      _ForwardIterator __first, _ForwardIterator __last,
		      size_type __n);


      // Internal erase functions follow.

      void
      _M_destroy_data_aux(iterator __first, iterator __last);

      // Called by ~deque().
      // NB: Doesn't deallocate the nodes.
      template<typename _Alloc1>
	void
	_M_destroy_data(iterator __first, iterator __last, const _Alloc1&)
	{ _M_destroy_data_aux(__first, __last); }

      void
      _M_destroy_data(iterator __first, iterator __last,
		      const std::allocator<_Tp>&)
      {
	if (!__has_trivial_destructor(value_type))
	  _M_destroy_data_aux(__first, __last);
      }

      // Called by erase(q1, q2).
      void
      _M_erase_at_begin(iterator __pos)
      {
	_M_destroy_data(begin(), __pos, _M_get_Tp_allocator());
	_M_destroy_nodes(this->_M_impl._M_start._M_node, __pos._M_node);
	this->_M_impl._M_start = __pos;
      }

      // Called by erase(q1, q2), resize(), clear(), _M_assign_aux,
      // _M_fill_assign, operator=.
      void
      _M_erase_at_end(iterator __pos)
      {
	_M_destroy_data(__pos, end(), _M_get_Tp_allocator());
	_M_destroy_nodes(__pos._M_node + 1,
			 this->_M_impl._M_finish._M_node + 1);
	this->_M_impl._M_finish = __pos;
      }

      iterator
      _M_erase(iterator __pos);

      iterator
      _M_erase(iterator __first, iterator __last);

#if __cplusplus >= 201103L
      // Called by resize(sz).
      void
      _M_default_append(size_type __n);

      bool
      _M_shrink_to_fit();
#endif

      //@{
      /// Memory-handling helpers for the previous internal insert functions.
      iterator
      _M_reserve_elements_at_front(size_type __n)
      {
	const size_type __vacancies = this->_M_impl._M_start._M_cur
				      - this->_M_impl._M_start._M_first;
	if (__n > __vacancies)
	  _M_new_elements_at_front(__n - __vacancies);
	return this->_M_impl._M_start - difference_type(__n);
      }

      iterator
      _M_reserve_elements_at_back(size_type __n)
      {
	const size_type __vacancies = (this->_M_impl._M_finish._M_last
				       - this->_M_impl._M_finish._M_cur) - 1;
	if (__n > __vacancies)
	  _M_new_elements_at_back(__n - __vacancies);
	return this->_M_impl._M_finish + difference_type(__n);
      }

      void
      _M_new_elements_at_front(size_type __new_elements);

      void
      _M_new_elements_at_back(size_type __new_elements);
      //@}


      //@{
      /**
       *  @brief Memory-handling helpers for the major %map.
       *
       *  Makes sure the _M_map has space for new nodes.  Does not
       *  actually add the nodes.  Can invalidate _M_map pointers.
       *  (And consequently, %deque iterators.)
       */
      void
      _M_reserve_map_at_back(size_type __nodes_to_add = 1)
      {
	if (__nodes_to_add + 1 > this->_M_impl._M_map_size
	    - (this->_M_impl._M_finish._M_node - this->_M_impl._M_map))
	  _M_reallocate_map(__nodes_to_add, false);
      }

      void
      _M_reserve_map_at_front(size_type __nodes_to_add = 1)
      {
	if (__nodes_to_add > size_type(this->_M_impl._M_start._M_node
				       - this->_M_impl._M_map))
	  _M_reallocate_map(__nodes_to_add, true);
      }

      void
      _M_reallocate_map(size_type __nodes_to_add, bool __add_at_front);
      //@}

#if __cplusplus >= 201103L
      // Constant-time, nothrow move assignment when source object's memory
      // can be moved because the allocators are equal.
      void
      _M_move_assign1(deque&& __x, /* always equal: */ true_type) noexcept
      {
	this->_M_impl._M_swap_data(__x._M_impl);
	__x.clear();
	std::__alloc_on_move(_M_get_Tp_allocator(), __x._M_get_Tp_allocator());
      }

      // When the allocators are not equal the operation could throw, because
      // we might need to allocate a new map for __x after moving from it
      // or we might need to allocate new elements for *this.
      void
      _M_move_assign1(deque&& __x, /* always equal: */ false_type)
      {
	constexpr bool __move_storage =
	  _Alloc_traits::_S_propagate_on_move_assign();
	_M_move_assign2(std::move(__x), __bool_constant<__move_storage>());
      }

      // Destroy all elements and deallocate all memory, then replace
      // with elements created from __args.
      template<typename... _Args>
      void
      _M_replace_map(_Args&&... __args)
      {
	// Create new data first, so if allocation fails there are no effects.
	deque __newobj(std::forward<_Args>(__args)...);
	// Free existing storage using existing allocator.
	clear();
	_M_deallocate_node(*begin()._M_node); // one node left after clear()
	_M_deallocate_map(this->_M_impl._M_map, this->_M_impl._M_map_size);
	this->_M_impl._M_map = nullptr;
	this->_M_impl._M_map_size = 0;
	// Take ownership of replacement memory.
	this->_M_impl._M_swap_data(__newobj._M_impl);
      }

      // Do move assignment when the allocator propagates.
      void
      _M_move_assign2(deque&& __x, /* propagate: */ true_type)
      {
	// Make a copy of the original allocator state.
	auto __alloc = __x._M_get_Tp_allocator();
	// The allocator propagates so storage can be moved from __x,
	// leaving __x in a valid empty state with a moved-from allocator.
	_M_replace_map(std::move(__x));
	// Move the corresponding allocator state too.
	_M_get_Tp_allocator() = std::move(__alloc);
      }

      // Do move assignment when it may not be possible to move source
      // object's memory, resulting in a linear-time operation.
      void
      _M_move_assign2(deque&& __x, /* propagate: */ false_type)
      {
	if (__x._M_get_Tp_allocator() == this->_M_get_Tp_allocator())
	  {
	    // The allocators are equal so storage can be moved from __x,
	    // leaving __x in a valid empty state with its current allocator.
	    _M_replace_map(std::move(__x), __x.get_allocator());
	  }
	else
	  {
	    // The rvalue's allocator cannot be moved and is not equal,
	    // so we need to individually move each element.
	    _M_assign_aux(std::__make_move_if_noexcept_iterator(__x.begin()),
			  std::__make_move_if_noexcept_iterator(__x.end()),
			  std::random_access_iterator_tag());
	    __x.clear();
	  }
      }
#endif
    };
```

`deque`的实现比`vector`和`list`要复杂的多，主要是因为其空间布局不太一样。下面的代码主要是对双端队列队首与队尾的操作（`push_front`、`push_back`、`pop_front`、`pop_back`）中涉及到空间变动的部分代码实现：
```c++
// Called only if _M_impl._M_finish._M_cur == _M_impl._M_finish._M_last - 1.
template<typename _Tp, typename _Alloc>
void deque<_Tp, _Alloc>::_M_push_back_aux(const value_type& __t) {
	_M_reserve_map_at_back();
	*(this->_M_impl._M_finish._M_node + 1) = this->_M_allocate_node();
	__try {
	    this->_M_impl.construct(this->_M_impl._M_finish._M_cur, __t);
	    this->_M_impl._M_finish._M_set_node(this->_M_impl._M_finish._M_node + 1);
	    this->_M_impl._M_finish._M_cur = this->_M_impl._M_finish._M_first;
	} __catch(...) {
	    _M_deallocate_node(*(this->_M_impl._M_finish._M_node + 1));
	    __throw_exception_again;
	}
}

// Called only if _M_impl._M_start._M_cur == _M_impl._M_start._M_first.
template<typename _Tp, typename _Alloc>
void deque<_Tp, _Alloc>::_M_push_front_aux(const value_type& __t) {
	_M_reserve_map_at_front();
	*(this->_M_impl._M_start._M_node - 1) = this->_M_allocate_node();
	__try {
	    this->_M_impl._M_start._M_set_node(this->_M_impl._M_start._M_node - 1);
	    this->_M_impl._M_start._M_cur = this->_M_impl._M_start._M_last - 1;
	    this->_M_impl.construct(this->_M_impl._M_start._M_cur, __t);
	} __catch(...) {
	    ++this->_M_impl._M_start;
	    _M_deallocate_node(*(this->_M_impl._M_start._M_node - 1));
	    __throw_exception_again;
	}
}

// Called only if _M_impl._M_finish._M_cur == _M_impl._M_finish._M_first.
template <typename _Tp, typename _Alloc>
void deque<_Tp, _Alloc>::_M_pop_back_aux() {
    _M_deallocate_node(this->_M_impl._M_finish._M_first);
    this->_M_impl._M_finish._M_set_node(this->_M_impl._M_finish._M_node - 1);
    this->_M_impl._M_finish._M_cur = this->_M_impl._M_finish._M_last - 1;
    _Alloc_traits::destroy(_M_get_Tp_allocator(), this->_M_impl._M_finish._M_cur);
}

  // Called only if _M_impl._M_start._M_cur == _M_impl._M_start._M_last - 1.
  // Note that if the deque has at least one element (a precondition for this
  // member function), and if
  //   _M_impl._M_start._M_cur == _M_impl._M_start._M_last,
  // then the deque must have at least two nodes.
  template <typename _Tp, typename _Alloc>
void deque<_Tp, _Alloc>::_M_pop_front_aux() {
    _Alloc_traits::destroy(_M_get_Tp_allocator(), this->_M_impl._M_start._M_cur);
    _M_deallocate_node(this->_M_impl._M_start._M_first);
    this->_M_impl._M_start._M_set_node(this->_M_impl._M_start._M_node + 1);
    this->_M_impl._M_start._M_cur = this->_M_impl._M_start._M_first;
}
```

>更详细的还是自己看STL的源码吧，顺便吐槽一下STL的源码，代码太臃肿了，看起来太累了，如果按照其实现原理，自己实现一个mini版STL，应该会简洁许多许多。

