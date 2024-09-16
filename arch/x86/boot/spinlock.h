#pragma once
#include "types.h"

// mutex lock
typedef struct {
  u32 locked;
  // For debugging:
  char *name;
  struct cpu *cpu; // cpu holding the lock
  u32 pcs[10];     // call stack (an array of program counters) that locked the lock.
} spinlock;
