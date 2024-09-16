#pragma once
#include "buffer.h"

// if dirty, write to disk and clear dirty
// else read from disk
// set valid
void sata_sync(buffer* b);
