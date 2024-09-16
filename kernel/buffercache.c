#include "buffer.h"
#include "sata.h"
#define BUFFER_CACHE_SIZE 64

buffer bufs[BUFFER_CACHE_SIZE];
buffer* bufsend = bufs + BUFFER_CACHE_SIZE;
buffer* mru = bufs;
buffer* lru = bufs;

static void doubly_linked_list_push_front(buffer* b) {
    b->next = mru->next;
    b->prev = mru;
    mru->prev = b;
    mru = b;
}

static void doubly_linked_list_remove(buffer* b) {
    b->prev->next = b->next;
    b->next->prev = b->prev;
}

void init_buffer_cache() {
    mru = bufs;
    lru = bufs;
    bufsend = bufs + BUFFER_CACHE_SIZE;
    for (buffer* b = bufs; b < bufs + BUFFER_CACHE_SIZE; ++b) {
        doubly_linked_list_push_front(b);
    }
}

static buffer* buffer_get(u32 dev, u32 blockno) {
    // block cached
    buffer* b = mru;
    do {
        if (b->dev == dev && b->blockno == blockno) {
            return b;
        }
        b = b->next;
    } while (b != mru);

    // recycle unused buffer
    b = lru;
    do {
        if (b->refcnt == 0 && !b->dirty) {
            b->dev = dev;
            b->blockno = blockno;
            b->valid = 0;
            b->refcnt = 1;
            return b;
        }
        b = b->next;
    } while (b != lru);

    // should not happen
    return 0;
}

buffer* buffer_read(u32 dev, u32 blockno) {
    buffer* b = buffer_get(dev, blockno);
    if (!b->valid) {
        sata_sync(b);
    }
    return b;
}

void buffer_write(buffer* b) {
    b->dirty = 1;
    sata_sync(b);
}

void buffer_release(buffer* b) {
    b->refcnt--;
    if (b->refcnt == 0) {
        doubly_linked_list_remove(b);
        doubly_linked_list_push_front(b);
    }
}
