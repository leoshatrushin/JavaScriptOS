#pragma once
#include "Inode.h"
#include "File.h"
#include "stat.h"

class OpenFileDescription {
private:
    File* inode;
    off_t offset;
    u32 status_flags;
    struct access_mode {
        u8 readable : 1;
        u8 writable : 1;
        u8 blocking : 1;
        u8 append : 1;
    };
    //u32 signal_driven_io_settings;
typedef struct {
    enum { FD_NONE, FD_PIPE, FD_INODE } type;
    int ref; // reference count
    char readable;
    char writable;
    struct pipe *pipe;
} file_description;


};
