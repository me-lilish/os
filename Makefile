# Toolchain notes:
# - Preferred: i686-elf-g++ / i686-elf-ld, as used in the OS labs.
# - On Windows/MSYS2, add the cross compiler bin directory plus /usr/bin to PATH.
# - This Makefile builds a bootable floppy-style kernel.img so mkisofs is optional.
TOOL_PREFIX ?= /opt/cross/bin/i686-elf-
CXX := $(TOOL_PREFIX)g++
LD := $(TOOL_PREFIX)ld
NASM ?= /mingw64/bin/nasm
QEMU ?= /mingw64/bin/qemu-system-x86_64

KERNEL_SECTORS ?= 31

KERNEL_DIRS := kernel kernel/core kernel/drivers kernel/mm kernel/process kernel/syscalls kernel/arch kernel/shell
AS_SOURCES := $(foreach dir,$(KERNEL_DIRS),$(wildcard $(dir)/*.asm))
AS_SOURCES := $(filter-out kernel/kernel_entry.asm,$(AS_SOURCES))
C_SOURCES := $(foreach dir,$(KERNEL_DIRS),$(wildcard $(dir)/*.cpp))
HEADERS := $(foreach dir,$(KERNEL_DIRS),$(wildcard $(dir)/*.h))

OBJ := $(C_SOURCES:.cpp=.o)
ASOBJ := $(AS_SOURCES:.asm=.o)
ENTRY_OBJ := kernel/kernel_entry.o

CXXFLAGS := -Wall -Wextra -Os -m32 -ffreestanding -nostdlib \
	-fno-exceptions -fno-rtti -fno-asynchronous-unwind-tables \
	-fno-pie -fno-stack-protector -fno-use-cxa-atexit \
	-fno-threadsafe-statics -fno-leading-underscore \
	-ffunction-sections -fdata-sections

LDFLAGS := -m elf_i386 -T link.ld --oformat binary --gc-sections

.PHONY: all prepare run iso clean size-check

all: kernel.img

prepare:
	mkdir -p bin iso

kernel.img: prepare bin/bootsect.bin bin/kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd seek=0 conv=notrunc if=bin/bootsect.bin of=$@ bs=512 count=1
	dd seek=1 conv=notrunc if=bin/kernel.bin of=$@ bs=512 count=$(KERNEL_SECTORS)

bin/kernel.bin: $(ENTRY_OBJ) $(ASOBJ) $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@
	$(MAKE) size-check

size-check:
	@size=$$(wc -c < bin/kernel.bin); \
	max=$$(( $(KERNEL_SECTORS) * 512 )); \
	echo "kernel.bin size: $$size bytes (bootloader reads max $$max bytes)"; \
	if [ $$size -gt $$max ]; then \
		echo "ERROR: kernel.bin is larger than the sectors loaded by boot/bootsect.asm."; \
		echo "Either reduce code size or, if your instructor permits it, increase the bootloader sector count."; \
		exit 1; \
	fi

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.asm
	$(NASM) -f elf32 $< -o $@

bin/%.bin: boot/%.asm
	$(NASM) $< -f bin -o $@

run: all
	$(QEMU) -drive file=kernel.img,format=raw,if=floppy -m 256M

iso: kernel.img
	cp kernel.img iso/kernel.img
	mkisofs -o os.iso -V LiziOS -b kernel.img iso

clean:
	rm -f bin/*.bin bin/*.elf *.dis kernel.img iso/kernel.img os.iso
	rm -f $(ENTRY_OBJ) $(OBJ) $(ASOBJ)
