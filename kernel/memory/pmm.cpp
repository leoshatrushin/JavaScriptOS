#include <cstddef>
#include <kernel/spinlock.hpp>
#include <kernel/arch/x86/paging.h>
#include <kernel/arch/x86/memlayout.h>

void* const VA_KERNEL_END = KV2V(KVA_KERNEL_END);

typedef struct free_page {
    struct free_page *next;
} free_page;

struct {
    spinlock lock;
    int use_lock; // for early stages of the kernel where we only use a single CPU and interrupts are disabled
    free_page *freelist;
} kmem;

void pmm_free(const void *va) {
    free_page* page = (free_page*)va;
    page->next = kmem.freelist;
    kmem.freelist = page;
}

void pmm_free_range(void* vstart, void* vend) {
    for(u8* p = (u8*)PGROUNDUP(vstart); p + PGSIZE <= vend; p += PGSIZE) {
        pmm_free(p);
    }
}

void *pmm_alloc() {
    free_page *node = kmem.freelist;
    if(node) {
        kmem.freelist = node->next;
    }
    return node;
}

void init_pmm() {
    pmm_free_range(VA_KERNEL_END, PHYSTOP);
}
