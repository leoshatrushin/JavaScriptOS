#pragma once

#define VA_PMAP_BEGIN   0xFFFF880000000000
#define VA_PMAP_END     0xFFFF880040000000

// keep in sync with linker.ld
#define KVA_KERNEL_BEGIN 0xFFFFFFFF80000000

#ifndef __ASSEMBLER__
#include <kernel/libk/types.h>
extern u8 PA_KERNEL_BEGIN[];
extern u8 KVA_KERNEL_DATA_BEGIN[];
extern u8 PA_KERNEL_DATA_BEGIN[];
extern u8 KVA_KERNEL_END[];
extern u8 PA_KERNEL_END[];

#ifndef __cplusplus
#define KV2P(a) ((void*)((uintptr_t)(a) - KVA_KERNEL_BEGIN))
#define P2KV(a) ((void*)((uintptr_t)(a) + KVA_KERNEL_BEGIN))
#define KV2V(a) ((void*)((a) - (uintptr_t)(KVA_KERNEL_BEGIN - VA_PMAP_BEGIN)))
#define V2KV(a) ((void*)((a) + (uintptr_t)(KVA_KERNEL_BEGIN - VA_PMAP_BEGIN)))
#else
inline void* KV2P(void* a) { return (void*)((uintptr_t)a - KVA_KERNEL_BEGIN); }
inline void* P2KV(void* a) { return (void*)((uintptr_t)a + KVA_KERNEL_BEGIN); }
inline void* KV2V(void* a) { return (void*)((uintptr_t)a - (uintptr_t)(KVA_KERNEL_BEGIN - VA_PMAP_BEGIN)); }
inline void* V2KV(void* a) { return (void*)((uintptr_t)a + (uintptr_t)(KVA_KERNEL_BEGIN - VA_PMAP_BEGIN)); }

inline void* const PHYSTOP = reinterpret_cast<void*>(0x40000000);
#endif

#else
#define KV2P(x) ((x) - KVA_KERNEL_BEGIN)
#define P2KV(x) ((x) + KVA_KERNEL_BEGIN)
#endif
