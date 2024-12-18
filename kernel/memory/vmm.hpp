#pragma once
#include <kernel/arch/x86/paging.h>
#include <kernel/libk/types.h>

void init_vmm();
pt_t* setupkvm();
boolean mappages(pt_t *pml4, u8 *va, size_t size, u8 *pa, boolean w, boolean u);
