# Variant

An example of a std::variant-like type implementation.

The implementation creates dispatch table at compile time. Therefore, the invocation of the callable object in `visit` function is guaranteed to be is in constant time, independently of the number of types in variant.

This implementation has no `constexpr` support, because `reinterpret_cast` and placement new (`::new (ptr) T (val)`) are currently not allowed in `constexpr` context. An implementation with `constexpr` support is discussed e.g [here](http://talesofcpp.fusionfenix.com/post-20/eggs.variant---part-ii-the-constexpr-experience)
