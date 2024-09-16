#pragma once
#include "../libk/types.h"

#define NAME_MAX 32

class FileSystem {
private:
    u32 FileSystemID;
    char FileSystemName[NAME_MAX];
};
