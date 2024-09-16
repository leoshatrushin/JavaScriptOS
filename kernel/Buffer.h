#pragma once
#include "libk/types.h"

#define BLOCK_SIZE 512

typedef struct buffer {
    u8 dirty : 1;
    u8 valid : 1;
    u32 dev;
    u32 blockno;
    //struct sleeplock lock;
    u32 refcnt;
    struct buffer *next; // MRU cache
    struct buffer *prev;
    struct buffer *qnext; // disk queue
    byte data[BLOCK_SIZE];
} buffer;
