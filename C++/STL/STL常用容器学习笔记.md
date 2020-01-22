这里简要的记述一下STL常用容器的实现原理，要点等内容， 目前梳理的很差，后期再进行整理。

## vector
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

## list
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

## deque
双端队列，具体实现不同于`vector`与`list`，它是一小段一小段连续空间，每段连续空间之间通过指针数组（这个数组中存放的是每个连续空间数组的头指针）串联起来，这样就能访问到所有元素。之所以采用这种存储布局，是有原因的，是有其应用场景的，等分析完源码后，我们就明白其为何要这么做了。
### deque源码分析
我们摘取部分源码看一下其实现细节。双端队列的迭代器实现代码如下（相较于`vector`与`list`，对元素的访问因为其存储布局不同，在每一段连续分配空间的边缘要做特殊处理）：
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

      /**
       *  @brief  Inserts given value into %deque before specified iterator.
       *  @param  __position  An iterator into the %deque.
       *  @param  __x  Data to be inserted.
       *  @return  An iterator that points to the inserted data.
       *
       *  This function will insert a copy of the given value before the
       *  specified location.
       */
      iterator insert(iterator __position, const value_type& __x);

      /**
       *  Erases all the elements.  Note that this function only erases the
       *  elements, and that if the elements themselves are pointers, the
       *  pointed-to memory is not touched in any way.  Managing the pointer is
       *  the user's responsibility.
       */
      void clear() _GLIBCXX_NOEXCEPT { _M_erase_at_end(begin()); }

    protected:
      // Internal constructor functions follow.
	  // 省略部分代码......
	 	
      void _M_push_back_aux(const value_type&);
      void _M_push_front_aux(const value_type&);
      void _M_pop_back_aux();
      void _M_pop_front_aux();

	  // 省略部分代码......
    };
```

`deque`的实现比`vector`和`list`要复杂的多，主要是因为其空间布局不太一样。下面的代码主要是对双端队列队首与队尾的操作（`push_front`、`push_back`、`pop_front`、`pop_back`）中涉及到空间变动的部分代码实现：
```c++
// Called only if _M_impl._M_finish._M_cur == _M_impl._M_finish._M_last - 1.
template<typename _Tp, typename _Alloc>
void deque<_Tp, _Alloc>::_M_push_back_aux(const value_type& __t) {
	_M_reserve_map_at_back();
	*(this->_M_impl._M_finish._M_node + 1) = this->_M_allocate_node();      // map新指针指向新分配的连续空间
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
	*(this->_M_impl._M_start._M_node - 1) = this->_M_allocate_node();       // map指定位置指向新分配的连续空间
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
下面的原代码是调整`map`的，如果`map`没有适当空间插入新的连续空间首地址，就重新分配`map`（这种情况比如，`map`的后面全部插满了，但前面还大量空着，就需要将目前的`map`中的元素进行移动，使`map`的元素分布在中间位置，首尾两端是空闲的，以便于后面插入新元素； 如果是`map`的空间不足了，则需要新分配`map`空间，新空间大小要大于新指针元素数量+2）。
```c++
void _M_reserve_map_at_back(size_type __nodes_to_add = 1) {
    if (__nodes_to_add + 1 > this->_M_impl._M_map_size - (this->_M_impl._M_finish._M_node - this->_M_impl._M_map))
	    _M_reallocate_map(__nodes_to_add, false);
}

void _M_reserve_map_at_front(size_type __nodes_to_add = 1) {
    if (__nodes_to_add > size_type(this->_M_impl._M_start._M_node - this->_M_impl._M_map))
	    _M_reallocate_map(__nodes_to_add, true);
}

template <typename _Tp, typename _Alloc>
void deque<_Tp, _Alloc>::_M_reallocate_map(size_type __nodes_to_add, bool __add_at_front) {
    const size_type __old_num_nodes = this->_M_impl._M_finish._M_node - this->_M_impl._M_start._M_node + 1;
    const size_type __new_num_nodes = __old_num_nodes + __nodes_to_add;

    _Map_pointer __new_nstart;
    if (this->_M_impl._M_map_size > 2 * __new_num_nodes) {
	    __new_nstart = this->_M_impl._M_map + (this->_M_impl._M_map_size - __new_num_nodes) / 2 + (__add_at_front ? __nodes_to_add : 0);	// 这里新map的开始往后移动了一段位置，是为了将来在前部插入的时候有剩余空间，后部空余一段位置也是。
	    if (__new_nstart < this->_M_impl._M_start._M_node)
	    	std::copy(this->_M_impl._M_start._M_node, this->_M_impl._M_finish._M_node + 1, __new_nstart);
	    else
	    	std::copy_backward(this->_M_impl._M_start._M_node, this->_M_impl._M_finish._M_node + 1, __new_nstart + __old_num_nodes);
	} else {
	    size_type __new_map_size = this->_M_impl._M_map_size + std::max(this->_M_impl._M_map_size, __nodes_to_add) + 2;		// 要至少空余2个空闲位置
	    _Map_pointer __new_map = this->_M_allocate_map(__new_map_size);
	    __new_nstart = __new_map + (__new_map_size - __new_num_nodes) / 2 + (__add_at_front ? __nodes_to_add : 0);
	    std::copy(this->_M_impl._M_start._M_node, this->_M_impl._M_finish._M_node + 1, __new_nstart);
	    _M_deallocate_map(this->_M_impl._M_map, this->_M_impl._M_map_size);

	    this->_M_impl._M_map = __new_map;
	    this->_M_impl._M_map_size = __new_map_size;
	}

    this->_M_impl._M_start._M_set_node(__new_nstart);
    this->_M_impl._M_finish._M_set_node(__new_nstart + __old_num_nodes - 1);
}
```
>更详细的还是自己看STL的源码吧，顺便吐槽一下STL的源码，代码太臃肿了，看起来太累了，如果按照其实现原理，自己实现一个mini版STL，应该会简洁许多许多。

到这里，`deque`中比较核心的源码已经基本分析完了，也基本展现了`deque`中几个关键成员函数是如何实现的，其迭代器的实现，其`map`的实现与调整。

### deque与vector、list的对比
`vector`能够实现随机访问，动态扩展，但在头部插入`O(n)`，开销较大，且动态扩展时需要复制所有的元素，同样效率较低。`list`插入、删除头尾部元素效率很高`O(n)`，但是不能随机访问，查找效率`O(n)`，每个节点需要存储前后节点指针，有较大的额外存储开销。而`deque`等于是在两种容器的优缺点进行了一定的平衡，在收尾插入、删除元素，效率很高`O(1)`，在中间插入`O(n)`都差不多，但其能实现随机访问，且动态扩展时不需要复制全体元素，只需要新分配足够的连续存储空间，最多重新复制一下`map`到新`map`，而`map`是各个连续存储空间首地址指针数组，容量相比全体元素小非常多，动态扩展时代价很小。所以，`deque`相比`vector`在更一般的情况下有更高的性能，相比`list`有更小的额外存储空间（但`deque`拥有较大的最小内存开销，原因是它需要`map`和一段连续存储空间开销，即元素数目非常小时开销比`list`大，但当元素数目较多时，空间开销比`list`少）。



