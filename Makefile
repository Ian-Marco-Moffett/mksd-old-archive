# Description: MKSD Makefile to build it.
# Author(s): alan, amrix

.POSIX:
all: $(KERNEL) limine iso

KERNEL = sys/kernel.sys

QEMUFLAGS = \
	--enable-kvm -serial stdio -cpu qemu64 -M q35 -m 256M -boot d -smp 2 \
	-rtc base=localtime -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0

CFLAGS = \
	-fexceptions --std=gnu11 -ffreestanding -fno-stack-protector -fno-pic \
	-Werror=implicit -Werror=implicit-function-declaration \
	-Werror=implicit-int -Werror=int-conversion \
	-Werror=incompatible-pointer-types -Werror=int-to-pointer-cast \
	-Werror=return-type -Wunused -mabi=sysv -mno-80387 -mno-mmx -mno-3dnow \
	-mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -pedantic \
	-I sys/include/ -D_KERNEL -Wno-pointer-sign

LDFLAGS = -nostdlib -zmax-page-size=0x1000 -static -Tsys/link.ld

OBJS = \
	sys/src/init.o sys/src/acpi/acpi.o sys/src/arch/x64/cpu.o \
	sys/src/arch/x64/exceptions.o sys/src/arch/x64/idt.o \
	sys/src/arch/x64/io.o sys/src/lib/hashmap.o \
	sys/src/lib/log.o sys/src/lib/string.o sys/src/mm/heap.o \
	sys/src/mm/mmap.o sys/src/mm/pmm.o sys/src/fs/vfs.o sys/src/mm/vmm.o

$(KERNEL): $(OBJS)
	@ echo -e "[\033[1;32mLD\033[0m] $^"
	@ $(LD) $(OBJS) $(LDFLAGS) -o $@

limine:
	@ echo -e "[\033[1;33mLIMINE\033[0m] Fetching binaries"
	git clone https://github.com/limine-bootloader/limine.git \
	--branch=v4.0-binary --depth=1
	make -C limine
	@ echo -e "[\033[1;33mLIMINE\033[0m] Fetched binaries"

iso: $(KERNEL) limine
	@ echo -e "[\033[1;35mISO\033[0m] Generating ISO"
	mkdir -p iso_root
	mkdir -p iso_root/boot
	cp meta/limine.cfg limine/limine.sys limine/limine-cd.bin \
	limine/limine-cd-efi.bin iso_root/
	cp sys/kernel.sys meta/*.bmp iso_root/boot/
	xorriso -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size 4 \
		-boot-info-table --efi-boot limine-cd-efi.bin -efi-boot-part \
		--efi-boot-image --protective-msdos-label iso_root \
        -o MKSD.iso
	@ echo -e "[\033[1;33mLIMINE\033[0m] Deploying limine onto to the iso"
	limine/limine-deploy MKSD.iso
	rm -rf iso_root
	@ echo -e "[\033[1;35mISO\033[0m] Generated ISO"

run:
	qemu-system-x86_64 $(QEMU_ARGS) -cdrom MKSD.iso

clean:
	rm -f $(OBJS) $(KERNEL)
