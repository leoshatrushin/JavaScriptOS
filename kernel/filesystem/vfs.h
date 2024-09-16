#pragma once
#include "../arch/x86/boot/types.h"
#include "../arch/x86/boot/param.h"
#include "stat.h"

// device switch - maps major device number to device functions
struct devsw {
  int (*read)(inode*, char*, int);
  int (*write)(inode*, char*, int);
};

extern struct devsw devsw[];

extern file_description open_file_table[NOFILEHANDLE];
