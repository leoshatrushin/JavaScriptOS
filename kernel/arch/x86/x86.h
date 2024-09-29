#define N_VECTORS          256

#define CR0_PE             (1 <<  0) // Protection Enable
#define CR0_WP             (1 << 16) // Write Protect
#define CR0_PG             (1 << 31) // Paging

#define CR4_PAE            (1 <<  5) // physical address extension

#define CPUID_01_ECX_X2APIC (1 << 21) // x2APIC
#define CPUID_01_EDX_APIC   (1 <<  9) // APIC

// EAX 0x80000001:EDX
#define CPUID_EDX_1G_PAGES (1 << 26) // 1GB pages
