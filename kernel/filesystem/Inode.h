#pragma once
#include "user.h"
#include "stat.h"
#include "InodeMetadata.h"
#include "../libk/ErrorOr.h"

// On-disk i-node structure
class Inode {
public:
    virtual InodeMetadata* metadata() const = 0;
    virtual ErrorOr<Inode*> lookup(const char* name) const = 0;
private:
    bool metadata_dirty;
};
