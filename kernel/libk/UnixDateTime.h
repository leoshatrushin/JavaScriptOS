#include "../../arch/x86//boot/types.h"

class UnixDateTime {
private:
    i64 m_seconds {0};
    u32 m_nanoseconds {0}; // Always less than 1'000'000'000
};
