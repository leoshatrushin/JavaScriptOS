#include "multiboot2.h"
#include "types.h"
#include "memlayout.h"
#include "utils.h"
#include "acpi.h"
#include "console.h"
#include "pmm.h"
#include "vmm.h"
#include "pci.h"

struct multiboot_tag_basic_meminfo *tagmem;
struct multiboot_tag_mmap *tagmmap;
struct multiboot_tag_framebuffer *tagfb;
multiboot_uint32_t tag_acpi_type;
struct multiboot_tag_old_acpi *tagold_acpi;
struct multiboot_tag_new_acpi *tagnew_acpi;
struct multiboot_tag *tagacpi;
struct multiboot_tag_module *loaded_module;
struct multiboot_tag *tag_start;

extern u32 lapic_timer_ticks_per_ms;
extern u32 lapic_timer_divider;

void parse_RSDT2(RSDP *rsdpdesc) {
    XSDT *xsdt = P2KV(rsdpdesc->xsdt);
    SDTHeader *sdtHeader = xsdt->sdts;
}

void kernel_main(struct multiboot_tag *multiboot2_info, unsigned int magic) {
    bochs_debug_output("Hello, Kernel!\n");
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        return;
    }

    multiboot2_info = P2KV(multiboot2_info);

    if ((uintptr_t) multiboot2_info & 7) {
        return;
    }
    bochs_debug_output("Checks Passed.\n");

    enumerate_all_pci_buses();
    init_pmm();
    init_vmm();

    size_t total_mbi_size = multiboot2_info->size;

    for (struct multiboot_tag *tag = (struct multiboot_tag *)(multiboot2_info + sizeof(struct multiboot_tag));
       tag->type != MULTIBOOT_TAG_TYPE_END;
       tag += ROUNDUP(tag->size, 8)) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                *tagmem = *(struct multiboot_tag_basic_meminfo *)tag;
                break;
            case MULTIBOOT_TAG_TYPE_MMAP:
                *tagmmap = *(struct multiboot_tag_mmap *)tag;
                break;
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
                *tagfb = *(struct multiboot_tag_framebuffer *)tag;
                break;
            case MULTIBOOT_TAG_TYPE_ACPI_OLD:
                *tagold_acpi = *(struct multiboot_tag_old_acpi *)tag;
                tag_acpi_type = MULTIBOOT_TAG_TYPE_ACPI_OLD;
                break;
            case MULTIBOOT_TAG_TYPE_ACPI_NEW:
                *tagnew_acpi = *(struct multiboot_tag_new_acpi *)tag;
                tag_acpi_type = MULTIBOOT_TAG_TYPE_ACPI_NEW;
                break;
            default:
                break;
        }
    }

    if (tag_acpi_type == MULTIBOOT_TAG_TYPE_ACPI_OLD) {
        return;
    }

    RSDP *rsdpdesc = (RSDP*)tagnew_acpi->rsdp;
    parse_RSDT2(rsdpdesc);

    /*init_idt();*/
    /*load_idt();*/
    /**/
    /*init_ioapic();*/
    /*ioapic_route(IRQ_KEYBOARD, 0, 0);*/
    /*ioapic_route(IRQ_PIT, 0, 1);*/
    /**/
    /*calibrate_lapic_timer();*/
    /*start_lapic_timer(lapic_timer_ticks_per_ms, LAPIC_TIMER_MODE_PERIODIC);*/
}
