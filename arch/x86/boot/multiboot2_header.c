// multiboot2 spec - https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html
// linked as .multiboot2.header section in linker script
#include "multiboot2.h"

struct multiboot_header hdr = {
    .magic = MULTIBOOT2_HEADER_MAGIC,
    .architecture = MULTIBOOT_ARCHITECTURE_I386,
    .header_length = sizeof(struct multiboot_header),
    .checksum = -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + 
        (multiboot_uint32_t)sizeof(struct multiboot_header)
    ),
}; 

struct multiboot_header_tag_framebuffer fb_tag = {
  .type = MULTIBOOT_TAG_TYPE_FRAMEBUFFER,
  .flags = MULTIBOOT_HEADER_TAG_OPTIONAL,
  .size = sizeof(struct multiboot_header_tag_framebuffer),
  // width, height, bpp 0 - don't care
};

struct multiboot_header_tag end = {
  .type = MULTIBOOT_HEADER_TAG_END,
  .size = sizeof(struct multiboot_header_tag),
};
