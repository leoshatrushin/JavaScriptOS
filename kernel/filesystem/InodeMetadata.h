#pragma once
#include "../LibK/UnixDateTime.h"
#include "stat.h"

class InodeMetadata {
public:
    boolean is_directory() const;
private:
    dev_t fs_dev {0};
    ino_t ino;
    mode_t mode {0};
    nlink_t nlink {0};
    uid_t uid {0};
    gid_t gid {0};
    dev_t device_file_dev;
    off_t size {0};
    blkcnt_t block_count { 0 };
    blksize_t block_size { 0 };
    UnixDateTime atime {};
    UnixDateTime ctime {};
    UnixDateTime mtime {};
    UnixDateTime dtime {};
};
