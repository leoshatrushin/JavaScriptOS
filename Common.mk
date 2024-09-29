CXFLAGS :=  -ffreestanding \
            -g \
            -O0 \
            -Wall \
            -Wextra \
            -Werror \
            -Wno-unused-parameter \
            -Wno-unused-variable \
            -Wno-unused-value \
            -I. \
            -mno-red-zone \
            -mno-sse \
            -mcmodel=large \
            -fno-stack-protector \
            -fno-pic

CFLAGS :=   $(CXFLAGS) \
            -std=gnu23

# -Wno-missing-field-initializers - warning on designated initializers for some reason
CPPFLAGS := $(CXFLAGS) \
            -std=gnu++23 \
            -Wno-missing-field-initializers \
            -fno-exceptions \
            -fno-non-call-exceptions \
            -fno-rtti
