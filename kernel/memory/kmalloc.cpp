#include <cstddef>
#include <limits>
#include <kernel/libk/types.h>
#include <kernel/libk/utils.h>
#include <kernel/libk/ErrorOr.hpp>
#include <new>
#include <kernel/console.h>

constexpr size_t MAX_ALLOC = std::numeric_limits<u16>::max();
std::byte kmem[MAX_ALLOC];
u16 bytes_allocated = 0;

/* TYPES */

struct block_header {
    union {
        u16 _size;
        struct {
            u16 left_allocated : 1;
            u16 allocated : 1;
            u16 __size : 14;
        };
    };

    struct proxy {
        proxy(block_header& parent) : parent(parent) {}
        proxy& operator=(u16 val) {
            parent._size = (val & ~0b11) | (parent._size & 0b11);
            return *this;
        }
        operator u16() const { return parent._size & ~0b11; }
    private:
        block_header& parent;
    };
    u16 size() const { return _size & ~0b11; }
    proxy size() { return proxy{*this}; }
};

struct free_block : block_header {
    u16 prev;
    u16 next;
};

// copy of header
typedef block_header free_block_footer;

constexpr size_t MIN_BLOCK_SIZE = sizeof(free_block) + sizeof(free_block_footer);

void copy_header_to_footer(free_block* block) {
    free_block_footer* footer = (free_block_footer*)((u8*)block + block->size() - sizeof(free_block_footer));
    *footer = *block;
}

/* FREELIST */

free_block* freelist = (free_block*)&kmem[0];

void init_kheap() {
    freelist->size() = MAX_ALLOC;
    freelist->prev = 0;
    freelist->next = 0;
}

u16 block_idx(free_block* block) {
    return (u16)((std::byte*)block - kmem);
}

void insert_into_free_list(free_block *block) {
    free_block* previousFreelist = freelist;
    freelist = block;
    block->next = block_idx(previousFreelist);
}

void remove_from_free_list(free_block* block) {
    // LIFO delete
    if (block->prev == 0) {
        if (block->next == 0) {
            freelist = nullptr;
        } else {
            freelist = (free_block*)&kmem[block->next];
        }
    } else {
        ((free_block*)&kmem[block->prev])->next = block->next;
    }

    if (block->next != 0) {
        ((free_block*)&kmem[block->next])->prev = block->prev;
    }
}

/* KMALLOC */

block_header* place(free_block* suitable_block, size_t alloc_size);

void* kmalloc(size_t alloc_size) {
    if (alloc_size == 0 || alloc_size > MAX_ALLOC) return nullptr;

    alloc_size = ROUNDUP2N(alloc_size, 4);

    alloc_size = MAX(alloc_size, sizeof(free_block));

    free_block* suitable_block = nullptr;
    for (u16* cur_free_block_ptr = &freelist->next;
            ;
            cur_free_block_ptr = &((free_block*)&kmem[*cur_free_block_ptr])->next) {
        free_block* cur_free_block = (free_block*)&kmem[*cur_free_block_ptr];
        if (alloc_size + sizeof(block_header) <= cur_free_block->size()) {
            suitable_block = cur_free_block;
            break;
        }
        if (*cur_free_block_ptr == 0) break;
    }

    if (!suitable_block) return nullptr;

    return (u8*)place(suitable_block, alloc_size) + sizeof(block_header);
}

// splitting policy
block_header* place(free_block* suitable_block, size_t alloc_size) {
    remove_from_free_list(suitable_block);
    block_header* allocated_block;
    if (suitable_block->size() - alloc_size >= MIN_BLOCK_SIZE) {
        // split
        // allocate right side of block
        allocated_block = (block_header*)((u8*)suitable_block + suitable_block->size() - alloc_size);
        allocated_block->size() = alloc_size;
        allocated_block->left_allocated = 0;

        // create free block from left side of block
        suitable_block->size() = suitable_block->size() - alloc_size;
        suitable_block->allocated = 0;
        copy_header_to_footer(suitable_block);
        insert_into_free_list(suitable_block);
    } else {
        // don't split, just allocate whole block
        allocated_block = (block_header*)suitable_block;
        allocated_block->size() = (u16)suitable_block->size();
        allocated_block->left_allocated = 1;
    }
    // allocate block
    allocated_block->allocated = 1;

    // update right block's left allocated bit
    block_header* next_block = (block_header*)((u8*)allocated_block + allocated_block->size());
    next_block->left_allocated = 1;

    bytes_allocated += allocated_block->size();
    /*debug_printf("Allocated %u bytes, %u bytes allocated\n", (u64)allocated_block->size(),*/
    /*        (u64)bytes_allocated);*/

    return allocated_block;
}

/* KFREE */

void coalesce(free_block* block);

void kfree(void* ptr) {
    if (!ptr) return;

    free_block* freed_block = (free_block*)((u8*)ptr - sizeof(block_header));

    bytes_allocated -= freed_block->size();
    //debug_printf("Freed     %u bytes, %u bytes allocated\n", (u64)freed_block->size(), (u64)bytes_allocated);

    coalesce(freed_block);
}

void coalesce(free_block* block) {
    free_block* coalescedBlock = block;
    block_header* right_block = (block_header*)((u8*)block + block->size());
    u16 coalesced_size = block->size();
    // coalesce left block
    if (!block->left_allocated) {
        free_block_footer* left_footer = (free_block_footer*)((u8*)block - sizeof(free_block_footer));
        coalesced_size += left_footer->size();
        free_block* left_block = (free_block*)((u8*)block - left_footer->size());
        remove_from_free_list(left_block);
        coalescedBlock = left_block;
    }

    // coalesce right block
    // actually checks left pointer if right block is free, but that's fine
    if (!right_block->allocated) {
        free_block* right_free_block = (free_block*)right_block;
        coalesced_size += right_free_block->size();
        remove_from_free_list(right_free_block);
    } else {
        right_block->left_allocated = 0;
    }

    // create free block
    coalescedBlock->size() = coalesced_size;
    coalescedBlock->allocated = 0;
    coalescedBlock->left_allocated = 1;
    copy_header_to_footer(coalescedBlock);
    insert_into_free_list(coalescedBlock);
}

/* OPERATORS */

void* operator new(size_t size)
{
    void* ptr = kmalloc(size);
    ASSERTHALT(ptr);
    return ptr;
}

void* operator new(size_t size, std::nothrow_t const&) noexcept
{
    return kmalloc(size);
}

/*void* operator new(size_t size, std::align_val_t al)*/
/*{*/
/*    void* ptr = kmalloc_aligned(size, (size_t)al);*/
/*    ASSERTHALT(ptr);*/
/*    return ptr;*/
/*}*/

/*void* operator new(size_t size, std::align_val_t al, std::nothrow_t const&) noexcept*/
/*{*/
/*    return kmalloc_aligned(size, (size_t)al);*/
/*}*/

void* operator new[](size_t size)
{
    void* ptr = kmalloc(size);
    ASSERTHALT(ptr);
    return ptr;
}

void* operator new[](size_t size, std::nothrow_t const&) noexcept
{
    return kmalloc(size);
}

void operator delete(void* ptr) noexcept
{
    // All deletes in kernel code should have a known size.
    //ASSERTHALT(false);
    kfree(ptr);
}

void operator delete(void* ptr, size_t) noexcept
{
    //return kfree_sized(ptr, size);
    kfree(ptr);
}

/*void operator delete(void* ptr, size_t size, std::align_val_t) noexcept*/
/*{*/
/*    return kfree_sized(ptr, size);*/
/*}*/

void operator delete[](void* ptr) noexcept
{
    // All deletes in kernel code should have a known size.
    //ASSERTHALT(false);
    kfree(ptr);
}

void operator delete[](void* ptr, size_t) noexcept
{
    //return kfree_sized(ptr, size);
    kfree(ptr);
}
