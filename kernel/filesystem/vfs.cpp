#include "File.h"
#include "InodeFile.h"
#include "PathString.h"
#include "../process/Process.h"
#include "../libk/ErrorOr.h"
#include "../api/posix/errno.h"

static InodeFile root_directory;

static ErrorOr<File*> resolve_path(char* path) {
    InodeFile* parent;
    if (path[0] == '/') {
        parent = &root_directory;
    } else {
        parent = Process::current()->cwd;
        parent->refcnt++;
    }

   PathString pathstr(path);

   for (auto part : pathstr) {
       if (!parent->inode->metadata()->is_directory()) {
           return ENOTDIR;
       }

       Inode* child = parent->inode->lookup(part);
   }
}
