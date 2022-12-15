QEMU_ARGS = -cpu qemu64 -M q35 -m 3G -cdrom MkallOS.iso -boot d -smp 4 -rtc base=localtime -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0
CFLAGS = -fexceptions -std=gnu11 -ffreestanding -fno-stack-protector   -fno-pic -Werror=implicit -Werror=implicit-function-declaration -Werror=implicit-int   -Werror=int-conversion   -Werror=incompatible-pointer-types -Werror=int-to-pointer-cast -Werror=return-type -Wunused   -mabi=sysv -mno-80387 -mno-mmx   -mno-3dnow -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -pedantic -I sys/include/ -D_KERNEL
CC = cross/bin/x86_64-elf-gcc
LD = cross/bin/x86_64-elf-ld
OBJDIR = obj
SRCDIR = sys/src/

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
OBJDIR = obj
SRCDIR = sys/src
SRC = $(call rwildcard,$(SRCDIR),*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))

.PHONY: all
all: $(OBJS) asmfiles limine link iso

.PHONY: iso
iso:
	@echo "Generating ISO..."
	@mkdir -p iso_root
	@mkdir -p iso_root/boot
	@cp meta/limine.cfg \
		limine/limine.sys limine/limine-cd.bin limine/limine-cd-efi.bin iso_root/
	@cp sys/kernel.sys iso_root/boot/
	@xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o MkallOS.iso
	@echo "Deploying Limine..."
	@limine/limine-deploy MkallOS.iso
	@rm -rf iso_root

limine:
	@echo "Fetching Limine binaries..."
	@git clone https://github.com/limine-bootloader/limine.git --branch=v4.0-binary --depth=1
	@make -C limine

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: asmfiles
asmfiles:
	@#echo "Compiling assembly sources..."
	@#for source in $$(find sys/src/ -name "*.asm"); do nasm -felf64 $$source; done

.PHONY: link
link:
		mkdir -p asmobj/
		@#mv $(shell find sys/src/ -name "*.o") asmobj/
		@echo "Linking object files..."
		@$(LD) $(shell find obj/ -name "*.o") -nostdlib -zmax-page-size=0x1000 -static -Tsys/link.ld -o sys/kernel.sys

.PHONY: run
run:
	qemu-system-x86_64 --enable-kvm $(QEMU_ARGS) -serial stdio

.PHONY: clean
clean:
	rm -rf obj

.PHONY: clean_all
clean_all:
	rm -rf configure autom4te.cache config.status config.log install-sh Makefile
