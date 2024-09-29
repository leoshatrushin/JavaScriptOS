#include <cstddef>
#ifdef __clang__
#include <kernel/libcppk/clang_generator.hpp>
#else
#include <generator>
#endif

extern "C" inline void abort() {
    // Implement appropriate behavior for your kernel, such as halting the CPU
    while (true) { asm volatile ("hlt"); }
}

extern "C" inline void __builtin_abort() {
    // Implement appropriate behavior for your kernel, such as halting the CPU
    while (true) { asm volatile ("hlt"); }
}

namespace std {

template<typename T>
class allocator {
public:
    using value_type = T;

    allocator() noexcept = default;

    template<typename U>
    allocator(const allocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t) noexcept {
        ::operator delete(p);
    }
};

namespace __exception_ptr {
    inline void exception_ptr::_M_release() noexcept {
        // Minimal implementation or leave empty if exceptions are not used
    }
    void exception_ptr::_M_addref() noexcept {
        // Empty implementation since exceptions are not used
    }

}

void rethrow_exception(std::exception_ptr) {
    // Handle or ignore as appropriate
    while (true) { asm volatile ("hlt"); }
}

} // namespace std

/*constexpr std::size_t PoolSize = 512;*/
/*alignas(std::max_align_t) inline static char memoryPool[PoolSize];*/
/**/
/*// Memory block structure for tracking allocations*/
/*struct Block {*/
/*    Block* next;*/
/*    std::size_t size;*/
/*    char* address;*/
/*};*/
/**/
/*inline static Block* freeList = nullptr;  // Free list to track available blocks*/
/*inline static std::size_t poolOffset = 0; // Offset to track current position in the pool*/
/**/
/*// Custom Static Allocator with deallocation support*/
/*template <typename T>*/
/*struct StaticAllocator {*/
/*    using value_type = T;*/
/**/
/*    StaticAllocator() noexcept = default;*/
/**/
/*    template <typename U>*/
/*    constexpr StaticAllocator(const StaticAllocator<U>&) noexcept {}*/
/**/
/*    // Allocate memory from the static memory pool*/
/*    T* allocate(std::size_t n) {*/
/*        std::size_t requiredSize = n * sizeof(T);*/
/**/
/*        // Search the free list for an available block*/
/*        Block** prev = &freeList;*/
/*        for (Block* current = freeList; current; current = current->next) {*/
/*            if (current->size >= requiredSize) {*/
/*                // We found a block that fits the requested size*/
/*                *prev = current->next;  // Remove this block from the free list*/
/*                return reinterpret_cast<T*>(current->address);*/
/*            }*/
/*            prev = &current->next;*/
/*        }*/
/**/
/*        // If no free block is found, allocate from the static pool*/
/*        if (poolOffset + requiredSize > PoolSize) {*/
/*            hlt();*/
/*        }*/
/**/
/*        T* result = reinterpret_cast<T*>(memoryPool + poolOffset);*/
/*        poolOffset += requiredSize;*/
/*        return result;*/
/*    }*/
/**/
/*    // Deallocate memory and add it to the free list*/
/*    void deallocate(T* p, std::size_t n) noexcept {*/
/*        std::size_t blockSize = n * sizeof(T);*/
/*        char* blockAddress = reinterpret_cast<char*>(p);*/
/**/
/*        // Add the block to the free list*/
/*        Block* newBlock = reinterpret_cast<Block*>(blockAddress);*/
/*        newBlock->address = blockAddress;*/
/*        newBlock->size = blockSize;*/
/*        newBlock->next = freeList;*/
/*        freeList = newBlock;*/
/*    }*/
/**/
/*    // Rebind allocator to another type*/
/*    template <typename U>*/
/*    struct rebind {*/
/*        using other = StaticAllocator<U>;*/
/*    };*/
/*};*/
