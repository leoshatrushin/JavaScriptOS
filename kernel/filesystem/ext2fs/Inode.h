#include "../Inode.h"

class Ext2FSInode : public Inode {
public:
    virtual ErrorOr<Inode*> lookup(const char* name) const override;
};
