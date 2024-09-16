#pragma once
#include "File.h"
#include "Inode.h"

// In-memory copy of an inode
class InodeFile : public File {
public:
    Inode* inode;
};
