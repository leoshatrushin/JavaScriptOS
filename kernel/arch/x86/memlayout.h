#pragma once

#define VA_PMAP_BEGIN   0xFFFF880000000000
#define VA_PMAP_END     0xFFFF880040000000

// MUST BE THE SAME AS IN linker.ld
#define KVA_KERNEL_BEGIN 0xFFFFFFFF80000000

#ifndef __ASSEMBLER__
#include "../../libk/types.h"
extern u8 PA_KERNEL_BEGIN[];
extern u8 KVA_KERNEL_DATA_BEGIN[];
extern u8 PA_KERNEL_DATA_BEGIN[];
extern u8 KVA_KERNEL_END[];
extern u8 PA_KERNEL_END[];
#define KV2P(a) ((void*)((uintptr_t)(a) - KVA_KERNEL_BEGIN))
#define P2KV(a) ((void*)((uintptr_t)(a) + KVA_KERNEL_BEGIN))
#define KV2V(a) ((void*)((a) - (uintptr_t)(KVA_KERNEL_BEGIN - VA_PMAP_BEGIN)))
#define V2KV(a) ((void*)((a) + (uintptr_t)(KVA_KERNEL_BEGIN - VA_PMAP_BEGIN)))
#else
#define KV2P(x) ((x) - KVA_KERNEL_BEGIN)
#define P2KV(x) ((x) + KVA_KERNEL_BEGIN)
#endif
