#include "FileSystem.h"
#include "../libk/ErrorOr.h"

class BlockBasedFileSystem : public FileSystem {
protected:
    ErrorOr<u32> read_block(u64 index, size_t n, u64 offset = 0) const;
};
