#pragma once
#include "cstddef.hpp"

template <typename Alloc>
struct allocator_traits {
    typedef typename Alloc::value_type value_type;

    static value_type* allocate(Alloc& a, size_t n) {
        return a.allocate(n);
    }

    static void deallocate(Alloc& a, value_type* p, size_t n) {
        a.deallocate(p, n);
    }
};
