#include "spinlock.h"
#include "paging.h"
#include "x86.h"
#include "types.h"
#include "memlayout.h"

typedef struct free_page {
    struct free_page *next;
} free_page;

struct {
    spinlock lock;
    int use_lock; // for early stages of the kernel where we only use a single CPU and interrupts are disabled
    free_page *freelist;
} kmem;

void pmm_free(void *va) {
    free_page *page = (free_page *)va;
    page->next = kmem.freelist;
    kmem.freelist = page;
}

void pmm_free_range(void *vstart, void *vend) {
    for(u8 *p = PGROUNDUP(vstart); p + PGSIZE <= (u8*)vend; p += PGSIZE) {
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
    // free 512KB of memory
    pmm_free_range(KVA_KERNEL_END, (void*)((uintptr_t)KVA_KERNEL_END + (1 << 19)));
}
