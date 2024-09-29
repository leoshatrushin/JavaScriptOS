#include <kernel/libk/types.h>
#include <kernel/arch/x86/memlayout.h>
#include <kernel/arch/x86/paging.h>
#include <kernel/arch/x86/asm.hpp>
#include <kernel/memory/pmm.hpp>
#include <kernel/libk/utils.h>

extern pte_t kpml4[PTE_PER_PT];
pt_t kpdpe;

static pt_t* derefpte(pte_t* pte, u8 alloc) {
    pt_t* pt = 0;
    if (pte->p) {
        pt = (pt_t*)P2KV(PTE_ADDR(pte));
    } else {
        if (!alloc || (pt = (pt_t*)pmm_alloc()) == 0) return 0;
        __builtin_memset(pt, PTE_LAYER(pte->layer - 1), PGSIZE);
        *pte = (pte_t){ .p = 1, .w = 1, .addr = KVA_TO_PTE_ADDR(pt) };
    }
    return pt;
}

static pte_t* getaddrpte(pt_t* pml4, void* va_ptr, u8 alloc) {
    va_4k* va = (va_4k*)va_ptr;

    pte_t* pml4e = &(*pml4)[va->pml4i];
    pt_t* pdpt = derefpte(pml4e, alloc);

    pte_t* pdpte = &(*pdpt)[va->pdpti];
    if (pdpte->large) return pdpte;
    pt_t* pd = derefpte(pdpte, alloc);

    pte_t* pde = &(*pd)[va->pdi];
    if (pde->large) return pde;
    pt_t* pt = derefpte(pde, alloc);

    return &(*pt)[va->pti];
}

static boolean mappages(pt_t* pml4, u8* va, size_t size, u8* pa, boolean w, boolean u) {
    u8* last_page = (u8*)PGROUNDDOWN(va + size - 1);
    for (va = (u8*)PGROUNDDOWN(va); va <= last_page; va += PGSIZE, pa += PGSIZE) {
        pte_t *pte = getaddrpte(pml4, va, 1);
        if (!pte) return 1;
        pte->w = w;
        pte->u = u;
        pte->addr = PA_TO_PTE_ADDR(pa);
        pte->p = 1;
    }
    return 0;
}

// This table defines the kernel's mappings, which are present in
// every process's page table.
static struct kmap { // simultaneously define struct type and variable
    void *va_begin; // starting virtual address
    void *pa_begin;
    void *pa_end;
    u64 w;
} kmap[2];

// Set up a pgdir with page table for kernel mappings in kmap
// The kernel expects this in every pgdir
pt_t* setupkvm(void) {
    struct kmap* k;

    pt_t* pml4 = (pt_t*)pmm_alloc();
    if (pml4 == 0) return 0;

    __builtin_memset(pml4, PTE_LAYER(4), PGSIZE);
    // map all entries in kmap
    for (k = kmap; k < &kmap[NELEM(kmap)]; ++k)
        if (!mappages(pml4, (u8*)k->va_begin, (size_t)((uintptr_t)k->pa_end - (uintptr_t)k->pa_begin),
                      (u8*)k->pa_begin, k->w, 0)) {
            // abort - free all page tables and pgdir
            //freevm(pml4);
            return 0;
        }
    return pml4;
}

void init_vmm() {
    //kmap[0] = (struct kmap){ (void*)KVA_KERNEL_BEGIN, PA_KERNEL_BEGIN,      PA_KERNEL_DATA_BEGIN, 0};
    //kmap[1] = (struct kmap){ KVA_KERNEL_DATA_BEGIN,   PA_KERNEL_DATA_BEGIN, PA_KERNEL_END,        1};
    for (int i = 0; i < PTE_PER_PT - 2; ++i) {
        kpdpe[i] = { .p = 1, .w = 1, .large = 1, .addr = PA_TO_PTE_ADDR(i * _1GB) };
    }
    kpdpe[PTE_PER_PT - 2] = { .p = 1, .w = 1, .large = 1, .addr = PA_TO_PTE_ADDR(0) };
    kpdpe[PTE_PER_PT - 1] = { .p = 1, .w = 1, .large = 1, .addr = PA_TO_PTE_ADDR(_1GB) };
    kpml4[511] = { .p = 1, .w = 1, .addr = KVA_TO_PTE_ADDR(kpdpe) };
    // flush TLB
    lcr3({.raw = (u64)KV2P(kpml4)});
}
