#pragma once
#include <type_traits>

namespace std {
    template <typename Enum>
    constexpr auto to_underlying(Enum e) noexcept -> typename std::underlying_type<Enum>::type {
        return static_cast<typename std::underlying_type<Enum>::type>(e);
    }
}
