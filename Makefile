include Config.mk
include Common.mk

VERSION := 0.0.0

# toolchain
CC = $(TOOLPREFIX)gcc
CXX = $(TOOLPREFIX)g++
AS = $(TOOLPREFIX)as
LD = $(TOOLPREFIX)ld
QEMU = qemu-system-x86_64

# inputs
C_SRCS = kernel_main.c # or #(shell find -name "*.c")
ASM_SRCS = arch/x86/boot/boot.S arch/x86/boot/multiboot2.S
TARGET = build/kernel.elf

# flags
ASM_FLAGS := -f elf64

C_DEBUG_FLAGS := -g \
		-DDEBUG=1

ASM_DEBUG_FLAGS := -g \
		-F dwarf

PRJ_FOLDERS := arch

# auto populated variables, avoid duplication
SRC_C_FILES := $(shell find $(PRJ_FOLDERS) -type f -name "*.c")
SRC_CPP_FILES := $(shell find $(PRJ_FOLDERS) -type f -name "*.cpp")
SRC_H_FILES := $(shell find $(PRJ_FOLDERS) -type f -name "*.h")
SRC_HPP_FILES := $(shell find $(PRJ_FOLDERS) -type f -name "*.hpp")
SRC_ASM_FILES := $(shell find $(PRJ_FOLDERS) -type f -name "*.S")
#SRC_FONT_FILES := $(shell find $(FONT_FOLDER) -type f -name "*.psf")
# '%' is the section of text we want to keep in each line
# OBJ_ASM_FILE := $(patsubst %.S, $(BUILD_FOLDER)/%.o, $(SRC_ASM_FILES))
# OBJ_C_FILE := $(patsubst %.c, $(BUILD_FOLDER)/%.o, $(SRC_C_FILES))
OBJ_ASM_FILE = build/kernel/arch/x86/boot.o
OBJ_C_FILE = build/kernel/memory/vmm.o \
			 build/libc/stdio.o
OBJ_CPP_FILE = build/kernel/kernel_main.o \
			   build/kernel/console.o \
			   build/kernel/boot/multiboot2_header.o \
			   build/kernel/memory/pmm.o \
			   build/kernel/memory/kmalloc.o \
			   build/kernel/devices/pci/API.o \
			   build/kernel/devices/pci/Device.o \
			   build/kernel/devices/pci/HostController.o
#OBJ_FONT_FILE := $(patsubst $(FONT_FOLDER)/%.psf, $(BUILD_FOLDER)/%.o, $(SRC_FONT_FILES))

.PHONY: default all clean

ISO_IMAGE_FILENAME := $(IMAGE_BASE_NAME)-$(TOOLPREFIX)-$(VERSION).iso

default: build

build: $(BUILD_FOLDER)/$(ISO_IMAGE_FILENAME)

run: $(BUILD_FOLDER)/$(ISO_IMAGE_FILENAME)
	qemu-system-x86_64 -cdrom $(BUILD_FOLDER)/$(ISO_IMAGE_FILENAME)

$(BUILD_FOLDER)/$(ISO_IMAGE_FILENAME): $(BUILD_FOLDER)/kernel.bin grub.cfg
	@echo "Creating ISO image ..."
	mkdir -p $(BUILD_FOLDER)/isofiles/boot/grub
	cp grub.cfg $(BUILD_FOLDER)/isofiles/boot/grub
	cp $(BUILD_FOLDER)/kernel.bin $(BUILD_FOLDER)/isofiles/boot
	grub-mkrescue -o $(BUILD_FOLDER)/$(ISO_IMAGE_FILENAME) $(BUILD_FOLDER)/isofiles
	@echo "ISO image created, placed at $(BUILD_FOLDER)/$(ISO_IMAGE_FILENAME)"

$(BUILD_FOLDER)/%.o: %.S
	echo "$(<D)"
	#@echo "Assembling file: $<"
	mkdir -p "$(@D)"
	$(CC) $(CXFLAGS) -nostdinc -c "$<" -o "$@"

$(BUILD_FOLDER)/%.o: %.c
	echo "$(@D)"
	#@echo "Compiling C file: $<"
	mkdir -p "$(@D)"
	$(CC) $(CFLAGS) "$<" -c -o "$@"

$(BUILD_FOLDER)/%.o: %.cpp
	echo "$(@D)"
	#@echo "Compiling CPP file: $<"
	mkdir -p "$(@D)"
	$(CXX) $(CPPFLAGS) "$<" -c -o "$@"

$(BUILD_FOLDER)/kernel.bin: $(OBJ_ASM_FILE) $(OBJ_C_FILE) $(OBJ_CPP_FILE) linker.ld
	$(LD) -n -o $(BUILD_FOLDER)/kernel.bin -T linker.ld $(OBJ_ASM_FILE) $(OBJ_C_FILE) $(OBJ_CPP_FILE)

all: $(OBJS)
	@echo "Linking program ..."
	$(LD) $(LD_FLAGS) $(OBJS) -o $(TARGET)
	@echo "Program linked, placed @ $(TARGET)"

GDBPORT = 1234
QEMUGDB = -gdb tcp::$(GDBPORT)
gdb: ASM_FLAGS += $(ASM_DEBUG_FLAGS)
gdb: $(BUILD_FOLDER)/$(ISO_IMAGE_FILENAME)
	$(QEMU) -cdrom $(BUILD_FOLDER)/$(ISO_IMAGE_FILENAME) -machine q35 -monitor unix:qemu-monitor-socket,server,nowait -serial file:javascriptos.log -debugcon stdio -m 1G -d int -no-reboot -no-shutdown -S $(QEMUGDB)

clean:
	@echo "Cleaning build files ..."
	-rm -r build/
	@echo "Cleaning done!"
