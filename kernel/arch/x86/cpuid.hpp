#include <kernel/arch/x86/asm.hpp>

namespace cpuid {

inline bool supports_1g_pages() {
    auto result = cpuid_raw(0x80000001);
    return result.edx & (1 << 26);
}

}
