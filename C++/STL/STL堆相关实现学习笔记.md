### 堆
堆是十分重要的数据结构，我们常用的优先队列就是基于堆实现的数据结构，堆排序也是基于堆实现的，所以，我们要理解堆的实现，之前自己根据堆的原理自己实现了堆，现在来分析一下STL中堆的实现代码，STL的堆实现相比自己实现的代码肯定要多很多，但原理是一样的，我们下面看一下。

### 堆的实现
STL中提供了堆的一系列代码供人使用。我们分析一下。我们知道堆的存储形式是利用数组实现的，那给定一个数组序列，怎么判断它是不是堆呢？下面给出了判断是否是堆的实现代码，其原理十分简单，就是看是否满足堆性质，父节点大于等于子节点（最大堆）。当然这里`is_heap`还有其他重载形式，这里不再列出。
```c++
  /**
   *  @brief  Determines whether a range is a heap using comparison functor.
   *  @param  __first  Start of range.
   *  @param  __last   End of range.
   *  @param  __comp   Comparison functor to use.
   *  @return  True if range is a heap, false otherwise.
   *  @ingroup heap_algorithms
  */
template<typename _RandomAccessIterator, typename _Compare>
inline bool is_heap(_RandomAccessIterator __first, _RandomAccessIterator __last, _Compare __comp) { // 判断数组是不是满足堆性质
    // concept requirements
    __glibcxx_function_requires(_RandomAccessIteratorConcept<_RandomAccessIterator>)
    __glibcxx_requires_valid_range(__first, __last);
    __glibcxx_requires_irreflexive_pred(__first, __last, __comp);

    const auto __dist = std::distance(__first, __last);
    typedef __decltype(__comp) _Cmp;
    __gnu_cxx::__ops::_Iter_comp_iter<_Cmp> __cmp(_GLIBCXX_MOVE(__comp));
    return std::__is_heap_until(__first, __dist, __cmp) == __dist;
}

template<typename _RandomAccessIterator, typename _Distance, typename _Compare>
_Distance __is_heap_until(_RandomAccessIterator __first, _Distance __n, _Compare& __comp) {     
    _Distance __parent = 0;
    for (_Distance __child = 1; __child < __n; ++__child) {     // 实现原理比较好理解，按照堆性质，父节点要大于等于子节点（最大堆）
	    if (__comp(__first + __parent, __first + __child))
	        return __child;
	    if ((__child & 1) == 0)
	        ++__parent;
	}
    return __n;
}
```

如果给定一个数组序列，不满足堆性质，怎么把他转换成具有堆性质呢？这里有`make_heap`实现，我们看一下其实现。其实现原理也是依据堆性质，先找到最后一个非叶子节点（有子节点的节点），检查是否满足父节点大于子节点，如果不满足，交换父子节点的值，如此，一直到根节点。
```c++
  /**
   *  @brief  Construct a heap over a range.
   *  @param  __first  Start of heap.
   *  @param  __last   End of heap.
   *  @ingroup heap_algorithms
   *
   *  This operation makes the elements in [__first,__last) into a heap.
  */
template<typename _RandomAccessIterator>
inline void make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last) {
    // concept requirements
    __glibcxx_function_requires(_Mutable_RandomAccessIteratorConcept<_RandomAccessIterator>)
    __glibcxx_function_requires(_LessThanComparableConcept<typename iterator_traits<_RandomAccessIterator>::value_type>)
    __glibcxx_requires_valid_range(__first, __last);
    __glibcxx_requires_irreflexive(__first, __last);

    __gnu_cxx::__ops::_Iter_less_iter __comp;
    std::__make_heap(__first, __last, __comp);
}

template<typename _RandomAccessIterator, typename _Compare>
void __make_heap(_RandomAccessIterator __first, _RandomAccessIterator __last, _Compare& __comp) {
    typedef typename iterator_traits<_RandomAccessIterator>::value_type _ValueType;
    typedef typename iterator_traits<_RandomAccessIterator>::difference_type _DistanceType;

    if (__last - __first < 2)   
	    return;

    const _DistanceType __len = __last - __first;
    _DistanceType __parent = (__len - 2) / 2;
    while (true) {
	    _ValueType __value = _GLIBCXX_MOVE(*(__first + __parent));
	    std::__adjust_heap(__first, __parent, __len, _GLIBCXX_MOVE(__value), __comp);
	    if (__parent == 0)
	        return;
	    __parent--;
	}
}

template<typename _RandomAccessIterator, typename _Distance, typename _Tp, typename _Compare>
void __adjust_heap(_RandomAccessIterator __first, _Distance __holeIndex, _Distance __len, _Tp __value, _Compare __comp) {
    const _Distance __topIndex = __holeIndex;
    _Distance __secondChild = __holeIndex;
    while (__secondChild < (__len - 1) / 2) {
	    __secondChild = 2 * (__secondChild + 1);
	    if (__comp(__first + __secondChild, __first + (__secondChild - 1)))
	        __secondChild--;
	    *(__first + __holeIndex) = _GLIBCXX_MOVE(*(__first + __secondChild));
	    __holeIndex = __secondChild;
	}
    
    if ((__len & 1) == 0 && __secondChild == (__len - 2) / 2) {
	    __secondChild = 2 * (__secondChild + 1);
	    *(__first + __holeIndex) = _GLIBCXX_MOVE(*(__first + (__secondChild - 1)));
	    __holeIndex = __secondChild - 1;
	}
    __decltype(__gnu_cxx::__ops::__iter_comp_val(_GLIBCXX_MOVE(__comp)))
	__cmp(_GLIBCXX_MOVE(__comp));
    std::__push_heap(__first, __holeIndex, __topIndex, _GLIBCXX_MOVE(__value), __cmp);
}

template<typename _RandomAccessIterator, typename _Distance, typename _Tp, typename _Compare>
void __push_heap(_RandomAccessIterator __first, _Distance __holeIndex, _Distance __topIndex, _Tp __value, _Compare& __comp) {
    _Distance __parent = (__holeIndex - 1) / 2;
    while (__holeIndex > __topIndex && __comp(__first + __parent, __value)) {
	    *(__first + __holeIndex) = _GLIBCXX_MOVE(*(__first + __parent));
	    __holeIndex = __parent;
	    __parent = (__holeIndex - 1) / 2;
	}
    *(__first + __holeIndex) = _GLIBCXX_MOVE(__value);
}
```

向堆中添加元素后，调用`push_heap`使之满足堆性质。在堆尾添加了一个元素后，可能破坏堆性质，进行“上移”，直到满足堆性质。  
```c++
/**
   *  @brief  Push an element onto a heap.
   *  @param  __first  Start of heap.
   *  @param  __last   End of heap + element.
   *  @ingroup heap_algorithms
   *
   *  This operation pushes the element at last-1 onto the valid heap
   *  over the range [__first,__last-1).  After completion,
   *  [__first,__last) is a valid heap.
*/
template<typename _RandomAccessIterator>
inline void push_heap(_RandomAccessIterator __first, _RandomAccessIterator __last) {
    typedef typename iterator_traits<_RandomAccessIterator>::value_type _ValueType;
    typedef typename iterator_traits<_RandomAccessIterator>::difference_type _DistanceType;

    // concept requirements
    __glibcxx_function_requires(_Mutable_RandomAccessIteratorConcept<_RandomAccessIterator>)
    __glibcxx_function_requires(_LessThanComparableConcept<_ValueType>)
    __glibcxx_requires_valid_range(__first, __last);
    __glibcxx_requires_irreflexive(__first, __last);
    __glibcxx_requires_heap(__first, __last - 1);

    __gnu_cxx::__ops::_Iter_less_val __comp;
    _ValueType __value = _GLIBCXX_MOVE(*(__last - 1));
    std::__push_heap(__first, _DistanceType((__last - __first) - 1), _DistanceType(0), _GLIBCXX_MOVE(__value), __comp);
}
```

弹出一个元素到内部容器的尾端，破坏了堆性质后，执行“下移”保证堆性质。
```c++
  /**
   *  @brief  Pop an element off a heap using comparison functor.
   *  @param  __first  Start of heap.
   *  @param  __last   End of heap.
   *  @param  __comp   Comparison functor to use.
   *  @ingroup heap_algorithms
   *
   *  This operation pops the top of the heap.  The elements __first
   *  and __last-1 are swapped and [__first,__last-1) is made into a
   *  heap.  Comparisons are made using comp.
  */
template<typename _RandomAccessIterator, typename _Compare>
inline void pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last, _Compare __comp) {
    // concept requirements
    __glibcxx_function_requires(_Mutable_RandomAccessIteratorConcept<_RandomAccessIterator>)
    __glibcxx_requires_valid_range(__first, __last);
    __glibcxx_requires_irreflexive_pred(__first, __last, __comp);
    __glibcxx_requires_non_empty_range(__first, __last);
    __glibcxx_requires_heap_pred(__first, __last, __comp);

    if (__last - __first > 1) {
	    typedef __decltype(__comp) _Cmp;
	    __gnu_cxx::__ops::_Iter_comp_iter<_Cmp> __cmp(_GLIBCXX_MOVE(__comp));
	    --__last;
	    std::__pop_heap(__first, __last, __last, __cmp);
	}
}

template<typename _RandomAccessIterator, typename _Compare>
inline void __pop_heap(_RandomAccessIterator __first, _RandomAccessIterator __last, _RandomAccessIterator __result, _Compare& __comp) {
    typedef typename iterator_traits<_RandomAccessIterator>::value_type _ValueType;
    typedef typename iterator_traits<_RandomAccessIterator>::difference_type _DistanceType;

    _ValueType __value = _GLIBCXX_MOVE(*__result);
    *__result = _GLIBCXX_MOVE(*__first);
    std::__adjust_heap(__first, _DistanceType(0), _DistanceType(__last - __first), _GLIBCXX_MOVE(__value), __comp);
}
```

STL中也给出了堆排序的实现，代码如下：
```c++
 /**
   *  @brief  Sort a heap using comparison functor.
   *  @param  __first  Start of heap.
   *  @param  __last   End of heap.
   *  @param  __comp   Comparison functor to use.
   *  @ingroup heap_algorithms
   *
   *  This operation sorts the valid heap in the range [__first,__last).
   *  Comparisons are made using __comp.
  */
template<typename _RandomAccessIterator, typename _Compare>
inline void sort_heap(_RandomAccessIterator __first, _RandomAccessIterator __last, _Compare __comp) {
    // concept requirements
    __glibcxx_function_requires(_Mutable_RandomAccessIteratorConcept<_RandomAccessIterator>)
    __glibcxx_requires_valid_range(__first, __last);
    __glibcxx_requires_irreflexive_pred(__first, __last, __comp);
    __glibcxx_requires_heap_pred(__first, __last, __comp);

    typedef __decltype(__comp) _Cmp;
    __gnu_cxx::__ops::_Iter_comp_iter<_Cmp> __cmp(_GLIBCXX_MOVE(__comp));
    std::__sort_heap(__first, __last, __cmp);
}

template<typename _RandomAccessIterator, typename _Compare>
void __sort_heap(_RandomAccessIterator __first, _RandomAccessIterator __last, _Compare& __comp) {
    while (__last - __first > 1) {
	    --__last;
	    std::__pop_heap(__first, __last, __last, __comp);
	}
}
```