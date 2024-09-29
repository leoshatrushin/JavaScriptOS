#pragma once

#define PTE_SIZE 8
#define PGSIZE 4096
#define PGSIZE_2M 2097152
#define PGSIZE_1G 1073741824
#define PTE_PER_PT PGSIZE / PTE_SIZE

#ifndef __ASSEMBLER__
#include "../../Libk/types.h"
#include "memlayout.h"
#include "../../libk/utils.h"

typedef struct {
    u32 offset : 12;
    u32 pti : 9;
    u32 pdi : 9;
    u32 pdpti : 9;
    u32 pml4i : 9;
    u32 sgn : 16;
} __attribute__((packed)) va_4k;

typedef struct {
    u32 offset : 21;
    u32 pdi : 9;
    u32 pdpti : 9;
    u32 pml4i : 9;
    u32 sgn : 16;
} __attribute__((packed)) va_2m;

typedef struct {
    u32 offset : 30;
    u32 pdpti : 9;
    u32 pml4i : 9;
    u32 sgn : 16;
} __attribute__((packed)) va_1g;

// Page table entry
// page fault raised if translation fails for any reason
// exception 14, triggering interrupt 14
// general protection fault raised if VA is non-canonical (e.g. not sign-extended)
typedef union {
    struct {
    u8  p : 1;         // Present
    u8  w : 1;         // Writeable
    u8  u : 1;         // accessible by user (CPL=3 in segment register)
    u8  pwt : 1;       // Page Level Write Through, usually write-back preferred, see PAT
    u8  pcd : 1;       // Page Level Cache Disable, usually 0, see PAT
    u8  accessed : 1;  // set on translation
    u8  dirty : 1;     // set on write, supposed to only apply to PT but some emulators set it on other levels
    u8  large : 1;     // (PDPT+PD) Page Size, translation stops and maps a 1G/2M page, 'cpuid' for 1G support
                       // x86 supports mixing page sizes
                       // (PT) Page Attribute Table Index, selects PAT entry together with PWT and PCD
    u8  global : 1;    // (PT only) when CR3 is loaded or a task switch occurs, do not eject entry
                       // 'cpuid' for support
    u8  layer : 3;     // avail, OS-defined
    u32 addr : 28;     // Table/Page base address (high bits 12+)
    u16 rsrv0 : 12;
    u8  avail2 : 7;
    u8  pk : 4;        // Protection Key or available, 'cpuid' for support
                       // if CR4.PKE is set, PKRU register is used to control user access
                       // if CR4.PKS is set, PKRS register is used to control supervisor access
    u32 nx : 1;        // No Execute or reserved, also called XD, 'cpuid' for support and enable in EFER
    };
    u64 raw;
} __attribute__((packed)) pte_t;

typedef pte_t pt_t[PTE_PER_PT] __attribute__((aligned(4096)));

#define PTE_ADDR(pte) (void*)((uintptr_t)(pte)->addr << 12)
#define KVA_TO_PTE_ADDR(addr) (u32)((uintptr_t)KV2P(addr) >> 12)
#define PA_TO_PTE_ADDR(addr) (u32)((uintptr_t)addr >> 12)

#define PTE_LAYER(x) ((x) << 9)

#define PGROUNDUP(ptr) (void*)ROUNDUP2N((uintptr_t)ptr, PGSIZE)
#define PGROUNDDOWN(ptr) (void*)ROUNDDOWN2N((uintptr_t)ptr, PGSIZE)

// Page fault error code
// Describes what was attempted, not why it failed
// CR2 contains the faulting address
struct pf_err {
    u32 p : 1;        // Protection violation (else non-present page)
    u32 rw : 1;       // Write
    u32 us : 1;       // User
    u32 rsvd : 1;     // encountered reserved bit set to 1
    u32 id : 1;       // Instruction fetch
    u32 rsrv : 27;
} __attribute__((packed));

union cr3 {
    struct {
    u16 todo : 12;    // Changes meaning depending on cr4.14, for now 0
    u64 pa_pml4 : 52;
    } __attribute__((packed));
    u64 raw;
};

#endif // __ASSEMBLER__

#define PTE_P   0x1
#define PTE_W   0x2
#define PTE_PS 0x80
#define PTE_NX 0x8000000000000000

#define VA_4K(pml4, pdpt, pd, pt, offset) \
    ((pml4) << 39 | (pdpt) << 30 | (pd) << 21 | (pt) << 12 | (offset))

#ifdef __cplusplus
constexpr u64 _1GB = 1 << 30;
constexpr u64 _2MB = 1 << 21;
constexpr u64 _4KB = 1 << 12;
#else
#define _1GB (1 << 30)
#define _2MB (1 << 21)
#define _4KB (1 << 12)
#endif
