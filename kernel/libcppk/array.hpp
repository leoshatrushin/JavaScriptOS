#pragma once
#include "cstddef.hpp"

namespace mystd {

template <typename T, mystd::size_t N>
struct array {
private:
    T elems[N];
public:
    constexpr void fill(const T& value) {
        for (mystd::size_t i = 0; i < N; ++i) {
            elems[i] = value;
        }
    }

    constexpr T& operator[](mystd::size_t pos) noexcept {
        return elems[pos];
    }

    constexpr const T& operator[](mystd::size_t pos) const noexcept {
        return elems[pos];
    }

    constexpr bool operator==(const array& other) const {
        for (mystd::size_t i = 0; i < N; ++i) {
            if (elems[i] != other.elems[i]) {
                return false;
            }
        }
        return true;
    }
};

}
