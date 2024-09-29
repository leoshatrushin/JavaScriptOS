#pragma once

namespace mystd {
template <typename T, T v>
struct integral_constant {
    static constexpr T value = v;

    // Type aliases for value_type and type
    using value_type = T;
    using type = integral_constant<T, v>;

    constexpr operator value_type() const noexcept {
        return value;
    }

    static constexpr value_type get_value() noexcept {
        return value;
    }
};

// Convenient type aliases
using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

};

namespace details
{
    template<typename B>
    mystd::true_type test_ptr_conv(const volatile B*);
    template<typename>
    mystd::false_type test_ptr_conv(const volatile void*);
 
    template<typename B, typename D>
    auto test_is_base_of(int) -> decltype(test_ptr_conv<B>(static_cast<D*>(nullptr)));
    template<typename, typename>
    auto test_is_base_of(...) -> mystd::true_type; // private or ambiguous base
}

namespace mystd {

template <typename Enum>
struct underlying_type {
    typedef __underlying_type(Enum) type;
};

template <typename Enum>
using underlying_type_t = typename underlying_type<Enum>::type;

template<class T>
struct type_identity { using type = T; };

template<class T>
auto try_add_lvalue_reference(int) -> type_identity<T&>;
template<class T>
auto try_add_lvalue_reference(...) -> type_identity<T>;

template<class T>
struct add_lvalue_reference
    : decltype(try_add_lvalue_reference<T>(0)) {};

template<class T>
auto try_add_rvalue_reference(int) -> type_identity<T&&>;
template<class T>
auto try_add_rvalue_reference(...) -> type_identity<T>;

template<class T>
struct add_rvalue_reference
    : decltype(try_add_rvalue_reference<T>(0)) {};

template<typename... Ts>
using void_t = void;

template<typename T>
struct is_class {
    static const bool value = __is_class(T);
};

template<typename Base, typename Derived>
struct is_base_of :
    mystd::integral_constant<
        bool,
        mystd::is_class<Base>::value &&
        mystd::is_class<Derived>::value &&
        decltype(details::test_is_base_of<Base, Derived>(0))::value
    > {};

// Helper variable template
template<class Base, class Derived>
inline constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;

}
