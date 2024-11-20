#include "acpi.h"

struct APIC* get_MADT_item(MADT* table, u8 type, u8 offset) {
    if ( strncmp(table->header.Signature, MADT_ID, 4) != 0 ) {
        return NULL;
    }
    struct APIC* item = (struct APIC*)&table->apic_structures;
    u64 total_length = 0;
    u8 counter = 0;
    while (total_length + sizeof(MADT) < table->header.length && counter <= offset ) {
        if (item->type == type) {
            // Should it return null if offset > #items of required type? or return the last one found?
            if(counter ==  offset) {
                return item;
            }
            counter++;
        }
        total_length = total_length + item->length;
        item = (MADT_Item *)((uint64_t)madt_base + (uint64_t) total_length);
    }
    return NULL;
}
