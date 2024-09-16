#include "types.h"
#include "memlayout.h"
#include "paging.h"
#include "x86.h"
#include "pmm.h"
#include "utils.h"

extern pte_t kpml4[PTE_PER_PT];

static pt_t* derefpte(pte_t* pte, u8 alloc) {
    pt_t* pt = 0;
    if (pte->p) {
        pt = P2KV(PTE_ADDR(pte));
    } else {
        if (!alloc || (pt = pmm_alloc()) == 0) return 0;
        __builtin_memset(pt, PTE_LAYER(pte->layer - 1), PGSIZE);
        *pte = (pte_t){ .addr = VA_TO_PTE_ADDR(pt), .p = 1, .w = 1 };
    }
    return pt;
}

static pte_t* getaddrpte(pt_t* pml4, void* va_ptr, u8 alloc) {
    va_4k* va = (va_4k*)va_ptr;

    pte_t* pml4e = &(*pml4)[va->pml4i];
    pt_t* pdpt = derefpte(pml4e, alloc);

    pte_t* pdpte = &(*pdpt)[va->pdpti];
    if (pdpte->ps) return pdpte;
    pt_t* pd = derefpte(pdpte, alloc);

    pte_t* pde = &(*pd)[va->pdi];
    if (pde->ps) return pde;
    pt_t* pt = derefpte(pde, alloc);

    return &(*pt)[va->pti];
}

static boolean mappages(pt_t *pml4, u8 *va, size_t size, u8 *pa, boolean w, boolean u) {
    u8* last_page = PGROUNDDOWN(va + size - 1);
    for (va = PGROUNDDOWN(va); va <= last_page; va += PGSIZE, pa += PGSIZE) {
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
    struct kmap *k;

    pt_t* pml4 = pmm_alloc();
    if (pml4 == 0) return 0;

    __builtin_memset(pml4, PTE_LAYER(4), PGSIZE);
    // map all entries in kmap
    for (k = kmap; k < &kmap[NELEM(kmap)]; ++k)
        if (!mappages(pml4, k->va_begin, (uintptr_t)k->pa_end - (uintptr_t)k->pa_begin,
                      k->pa_begin, k->w, 0)) {
            // abort - free all page tables and pgdir
            //freevm(pml4);
            return 0;
        }
    return pml4;
}

void init_vmm() {
    kmap[0] = (struct kmap){ (void*)KVA_KERNEL_BEGIN, PA_KERNEL_BEGIN,      PA_KERNEL_DATA_BEGIN, 0};
    kmap[1] = (struct kmap){ KVA_KERNEL_DATA_BEGIN,   PA_KERNEL_DATA_BEGIN, PA_KERNEL_END,        1};
}
