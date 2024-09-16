#pragma once

template<typename T>
class ErrorOr {
public:
    ErrorOr(T result) : result(result) {}
    ErrorOr(int error_code) : error_code(error_code) {}
private:
    T result;
    int error_code;
};
