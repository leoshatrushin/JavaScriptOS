#pragma once

namespace mystd {

template <typename T>
struct remove_reference {
    typedef T type;
};

template <typename T>
struct remove_reference<T&> {
    typedef T type;
};

template <typename T>
struct remove_reference<T&&> {
    typedef T type;
};

template <typename T>
typename mystd::remove_reference<T>::type&& move(T&& arg) {
    return static_cast<typename mystd::remove_reference<T>::type&&>(arg);
}

};
