#pragma once
#include <type_traits>
#include <utility>

// Define a minimal version of std namespace
namespace mystd {

    // Utility to remove reference and cv-qualifiers
    template <typename T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

    // Forward declaration for ADL
    // These functions will be found via argument-dependent lookup
    // when user-defined begin/end functions are provided.

    // Primary template for begin
    template <typename T>
    constexpr auto begin(T& t) 
        -> decltype(t.begin()) 
    {
        return t.begin();
    }

    // Overload for arrays
    template <typename T, std::size_t N>
    constexpr T* begin(T (&array)[N]) noexcept {
        return array;
    }

    // Primary template for end
    template <typename T>
    constexpr auto end(T& t) 
        -> decltype(t.end()) 
    {
        return t.end();
    }

    // Overload for arrays
    template <typename T, std::size_t N>
    constexpr T* end(T (&array)[N]) noexcept {
        return array + N;
    }

    // Overload for types with free begin/end functions via ADL
    // These are found only if member functions are not present.

    // SFINAE helpers to detect if a free begin/end exists
    namespace detail {
        using std::begin;
        using std::end;

        // Detect free begin
        template <typename T>
        using free_begin_t = decltype(begin(std::declval<T&>()));

        template <typename T>
        constexpr bool has_free_begin = std::is_same_v<free_begin_t<T>, free_begin_t<T>>;

        // Detect free end
        template <typename T>
        using free_end_t = decltype(end(std::declval<T&>()));

        template <typename T>
        constexpr bool has_free_end = std::is_same_v<free_end_t<T>, free_end_t<T>>;
    }

    // Overload for free begin using ADL
    template <typename T>
    constexpr auto begin(const T& t) 
        -> std::enable_if_t<
            !std::is_member_function_pointer_v<decltype(&T::begin)> &&
            detail::has_free_begin<T>,
            decltype(begin(t))
        >
    {
        using std::begin;
        return begin(t);
    }

    // Overload for free end using ADL
    template <typename T>
    constexpr auto end(const T& t) 
        -> std::enable_if_t<
            !std::is_member_function_pointer_v<decltype(&T::end)> &&
            detail::has_free_end<T>,
            decltype(end(t))
        >
    {
        using std::end;
        return end(t);
    }

} // namespace mystd
