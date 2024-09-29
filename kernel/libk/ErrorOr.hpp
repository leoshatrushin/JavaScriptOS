#pragma once
#include <kernel/console.h>
#include <kernel/arch/x86/asm.hpp>

template<typename T>
class ErrorOr {
public:
    ErrorOr(T _value) : value(_value), error{false} {}
    ErrorOr(int error_code) : error_code(error_code), error{true} {}

    bool is_error() const { return error; }

    T release_value() { return move(value); }
    int release_error() { return error_code; }

private:
    T value;
    int error_code;
    bool error;
};

#define TRY(expr)                                                                                    \
({                                                                                                   \
    auto&& _temporary_result = (expr);                                                               \
    if (_temporary_result.is_error()) [[unlikely]]                                                   \
        return _temporary_result.release_error();                                                    \
    _temporary_result.release_value();                                                               \
})

#define TRYHALT(expr)                                                                                \
({                                                                                                   \
    auto&& _temporary_result = (expr);                                                               \
    if (_temporary_result.is_error()) [[unlikely]]                                                   \
        hlt();                                                                                       \
    _temporary_result.release_value();                                                               \
})

/*#define CASTHALT(type, expr)                                                                         \*/
/*({                                                                                                   \*/
/*    auto _temporary_result = dynamic_cast<type*>(&expr);                                             \*/
/*    if (_temporary_result == nullptr) [[unlikely]]                                                   \*/
/*        hlt();                                                                                       \*/
/*    *_temporary_result;                                                                              \*/
/*})*/

template<typename T, typename U>
T CASTHALT(U& expr) {
    auto tmp = dynamic_cast<T*>(&expr);
    if (tmp == nullptr) [[unlikely]]
        hlt();
    return *tmp;
}

#define ASSERTHALT(expr)                                                                             \
({                                                                                                   \
    if (!(expr)) [[unlikely]]                                                                        \
        hlt();                                                                                       \
})
