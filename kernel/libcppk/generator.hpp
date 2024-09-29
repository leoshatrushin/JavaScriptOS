#include <coroutine>
#include <cstddef>
#include <array>
#include <optional>
#include "../arch/x86/asm.h"
#include "memory_resource.hpp"
#include <ranges>

namespace mystd {

template<typename T>
class [[nodiscard]] generator : public std::ranges::view_interface<generator<T>> {
    inline static std::array<std::byte, 4096> mem;
    inline static mystd::pmr::monotonic_buffer_resource mempool{mem.data(), mem.size()};
    public:
        struct promise_type {
            auto get_return_object() {
                return std::coroutine_handle<promise_type>::from_promise(*this);
            }
            auto initial_suspend() { return std::suspend_always{}; }
            std::optional<T> coroValue;
            auto yield_value(T val) {
                coroValue = val;
                return std::suspend_always{};
            }
            void return_void() { }
            void unhandled_exception() { hlt(); }
            auto final_suspend() noexcept { return std::suspend_always{}; }
            void* operator new(std::size_t sz) {
                return mempool.allocate(sz);
            }
            void operator delete(void* ptr, std::size_t sz) {
                mempool.deallocate(ptr, sz, 0);
            }
        };
    private:
        std::coroutine_handle<promise_type> handle;
    public:
        generator(auto h) : handle{h} { }
        ~generator() { if (handle) handle.destroy(); }
        // no copy or move supported
        generator(const generator&) = delete;
        generator& operator=(const generator&) = delete;
        struct iterator {
            std::coroutine_handle<promise_type> iter_handle;
            iterator(auto p) : iter_handle{p} { }
            void getNext() {
                if (iter_handle) {
                    iter_handle.resume();
                    if (iter_handle.done()) {
                        iter_handle = nullptr;
                    }
                }
            }
            int operator*() const {
                ASSERTHALT(iter_handle != nullptr);
                return iter_handle.promise().coroValue.value();
            }
            iterator operator++() {
                getNext();
                return *this;
            }
            bool operator== (const iterator& i) const = default;
        };
        iterator begin() const {
            if (!handle || handle.done()) {
                return iterator{nullptr};
            }
            iterator iter{handle};
            iter.getNext();
            return iter;
        }
        iterator end() const {
            return iterator{nullptr};
        }
};

}
