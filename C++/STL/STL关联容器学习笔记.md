这里简单学习一下STL关联容器，主要是map、multimap、set、multiset以及unordered_map。前四个底层实现都是利用红黑树实现的，查找算法时间复杂度为`O(log(n))`，而unordered_map从名字上就知道是无序容器，其实现原理类似哈希表，查找算法时间复杂度`O(1)`。


### set、multiset、map、multimap
set容器是一个存储有序唯一元素的数据结构。不允许有重复元素。其底层实现为红黑树。multiset原理与set相同，不同的是允许有重复元素存在。

map映射是可以用任何类型的数据作为索引的表，不允许有重复元素，其实现原理与set类似，底层都是红黑树。multimap允许有重复元素存在。


```c++
template<typename _Key, typename _Compare = std::less<_Key>,typename _Alloc = std::allocator<_Key> >
class set 
public:
    typedef _Key     key_type;
    typedef _Key     value_type;    // 与map不同
    typedef _Compare key_compare;
    typedef _Compare value_compare; 
    typedef _Alloc   allocator_type;

private:
    typedef typename __gnu_cxx::__alloc_traits<_Alloc>::template rebind<_Key>::other _Key_alloc_type;
    typedef _Rb_tree<key_type, value_type, _Identity<value_type>, key_compare, _Key_alloc_type> _Rep_type;
    
    _Rep_type _M_t;  // Red-black tree representing set.    红黑树

    typedef __gnu_cxx::__alloc_traits<_Key_alloc_type> _Alloc_traits;

#if __cplusplus > 201402L
      using node_type = typename _Rep_type::node_type;
      using insert_return_type = typename _Rep_type::insert_return_type;
#endif
```
map与set其实现原理类似，不同的是map中每个元素是std::pair对。
```c++
template <typename _Key, typename _Tp, typename _Compare = std::less<_Key>,typename _Alloc=std::allocator<std::pair<const _Key, _Tp> > >
class map {
public:
    typedef _Key key_type;
    typedef _Tp mapped_type;
    typedef std::pair<const _Key, _Tp> value_type;      // 其值是pair对的形式存储
    typedef _Compare key_compare;                       // RB-tree，需要对元素进行比较
    typedef _Alloc allocator_type;

private:
    /// This turns a red-black tree into a [multi]map.
    typedef typename __gnu_cxx::__alloc_traits<_Alloc>::template rebind<value_type>::other _Pair_alloc_type;
    typedef _Rb_tree<key_type, value_type, _Select1st<value_type>, key_compare, _Pair_alloc_type> _Rep_type;

    /// The actual tree structure.
    _Rep_type _M_t;     // 底层实现RB-tree

    typedef __gnu_cxx::__alloc_traits<_Pair_alloc_type> _Alloc_traits;

public:
    // 重载[]
    mapped_type& operator[](const key_type& __k) {
	    // concept requirements
	    __glibcxx_function_requires(_DefaultConstructibleConcept<mapped_type>)
        iterator __i = lower_bound(__k);
	    // __i->first is greater than or equivalent to __k.
	    if (__i == end() || key_comp()(__k, (*__i).first))
#if __cplusplus >= 201103L
	        __i = _M_t._M_emplace_hint_unique(__i, std::piecewise_construct, std::tuple<const key_type&>(__k),
std::tuple<>());
#else
	        __i = insert(__i, value_type(__k, mapped_type()));
#endif
	    return (*__i).second;
    }
}
```
其他具体实现细节这里不再列出，只要懂红黑树的原理，很容易理解。
### unordered_map
这个实现与map不同，其底层是哈希表，理解了哈希表就很容易理解unordered_map。
```c++
template<class _Key, class _Tp,class _Hash = hash<_Key>,class _Pred = std::equal_to<_Key>,class _Alloc=std::allocator<std::pair<const _Key, _Tp> > >
class unordered_map {
    typedef __umap_hashtable<_Key, _Tp, _Hash, _Pred, _Alloc>  _Hashtable;
    _Hashtable _M_h;    // 可以看到底层实现是哈希表

public:
    // typedefs:
    //@{
    /// Public typedefs.
    typedef typename _Hashtable::key_type	key_type;
    typedef typename _Hashtable::value_type	value_type;
    typedef typename _Hashtable::mapped_type	mapped_type;
    typedef typename _Hashtable::hasher	hasher;
    typedef typename _Hashtable::key_equal	key_equal;
    typedef typename _Hashtable::allocator_type allocator_type;
    //@}

#if __cplusplus > 201402L
      using node_type = typename _Hashtable::node_type;
      using insert_return_type = typename _Hashtable::insert_return_type;
#endif
```