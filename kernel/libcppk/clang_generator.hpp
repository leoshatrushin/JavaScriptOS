#include <ranges>
#include <coroutine>
#include <memory>

namespace std::ranges {

template<range R, class Allocator = std::allocator<std::byte>>
struct elements_of {
    [[no_unique_address]] R range;
    [[no_unique_address]] Allocator allocator = Allocator();
};

template< class R, class Allocator = std::allocator<std::byte> >
elements_of( R&&, Allocator = Allocator() ) -> elements_of<R&&, Allocator>;

}

namespace std {
template<class Ref, class V = void, class Allocator = void>
class generator : public ranges::view_interface<generator<Ref, V, Allocator>> {
private:
using value = conditional_t<is_void_v<V>, remove_cvref_t<Ref>, V>; // exposition only
using reference = conditional_t<is_void_v<V>, Ref&&, Ref>; // exposition only
// 26.8.6, class generator::iterator
class iterator; // exposition only
public:
using yielded =
conditional_t<is_reference_v<reference >, reference , const reference &>;
// 26.8.5, class generator::promise_type
class promise_type;
generator(const generator&) = delete;
generator(generator&& other) noexcept;
~generator();
generator& operator=(generator other) noexcept;
iterator begin();
default_sentinel_t end() const noexcept;
private:
coroutine_handle<promise_type> coroutine_ = nullptr; // exposition only
};

template<class Ref, class V, class Allocator>
class generator<Ref, V, Allocator>::promise_type {
public:
generator get_return_object() noexcept;
suspend_always initial_suspend() const noexcept { return {}; }
suspend_always final_suspend() noexcept;
suspend_always yield_value(yielded val) noexcept;
suspend_always yield_value(const remove_reference_t<yielded>& lval)
requires is_rvalue_reference_v<yielded> &&
constructible_from<remove_cvref_t<yielded>, const remove_reference_t<yielded>&>;
template<class R2, class V2, class Alloc2, class Unused>
requires same_as<typename generator<R2, V2, Alloc2>::yielded, yielded>
suspend_always yield_value(ranges::elements_of<generator<R2, V2, Alloc2>&&, Unused> g) noexcept;
template<ranges::input_range R, class Alloc>
requires convertible_to<ranges::range_reference_t<R>, yielded>
suspend_always yield_value(ranges::elements_of<R, Alloc> r) noexcept;
void await_transform() = delete;
void return_void() const noexcept {}
void unhandled_exception();
void* operator new(size_t size)
requires same_as<Allocator, void> || default_initializable<Allocator>;
template<class Alloc, class... Args>
requires same_as<Allocator, void> || convertible_to<const Alloc&, Allocator>
void* operator new(size_t size, allocator_arg_t, const Alloc& alloc, const Args&...);
template<class This, class Alloc, class... Args>
requires same_as<Allocator, void> || convertible_to<const Alloc&, Allocator>
void* operator new(size_t size, const This&, allocator_arg_t, const Alloc& alloc,
const Args&...);
void operator delete(void* pointer, size_t size) noexcept;
private:
add_pointer_t<yielded> value_ = nullptr; // exposition only
};

template<class Ref, class V, class Allocator>
class generator<Ref, V, Allocator>::iterator {
public:
using value_type = value ;
using difference_type = ptrdiff_t;
iterator (iterator && other) noexcept;
iterator & operator=(iterator && other) noexcept;
reference operator*() const noexcept(is_nothrow_copy_constructible_v<reference >);
iterator & operator++();
void operator++(int);
friend bool operator==(const iterator & i, default_sentinel_t);
private:
coroutine_handle<promise_type> coroutine_; // exposition only
};

namespace __exception_ptr {
    class exception_ptr {
        void _M_release() noexcept;
    };
}

}
