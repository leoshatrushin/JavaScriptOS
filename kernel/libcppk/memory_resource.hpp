#include <cstddef>
#include <cstdint>
#include "../libk/ErrorOr.h"

namespace mystd::pmr {

class monotonic_buffer_resource {
public:
    // Constructor: accepts an initial buffer and an optional upstream allocator
    monotonic_buffer_resource(void* buffer, std::size_t buffer_size)
        : m_current(reinterpret_cast<char*>(buffer))
        , m_end(reinterpret_cast<char*>(buffer) + buffer_size)
    {
        ASSERTHALT(buffer != nullptr && "Initial buffer cannot be null");
    }

    void* allocate(std::size_t bytes, std::size_t alignment = alignof(std::max_align_t)) {
        // Calculate the adjustment needed to satisfy the alignment
        std::size_t space = m_end - m_current;
        std::uintptr_t current_addr = reinterpret_cast<std::uintptr_t>(m_current);
        std::size_t adjustment = align_forward_adjustment(current_addr, alignment);

        if (adjustment + bytes > space) {
            // Not enough space in the current buffer, allocate a new block
            //return allocate_from_upstream(bytes, alignment);
            hlt();
        }

        // Allocate from current buffer
        char* alloc_addr = m_current + adjustment;
        m_current += adjustment + bytes;
        return static_cast<void*>(alloc_addr);
    }

    void deallocate(void*, std::size_t, std::size_t) noexcept { }
private:
    // Calculate the adjustment needed to align the address
    static std::size_t align_forward_adjustment(std::uintptr_t address, std::size_t alignment) {
        std::size_t misalignment = address & (alignment - 1);
        if (misalignment == 0)
            return 0;
        return alignment - misalignment;
    }

    char* m_current;
    char* m_end;
};

}
