ARCH                 = x86_64
KERNEL_VERSION_MAJOR = 6
KERNEL_VERSION       = 6.12

TOOLS                = init sh echo ls cp mount cat

BUILD_DIR            = bin
SOURCE_DIR           = src

BINARIES             = $(foreach T, $(TOOLS), $(BUILD_DIR)/$T)

.PHONY: proper clean kernel efi qemu debug

efi: disk.img kernel/linux/arch/x86/boot/bzImage
	mcopy -oi $^ ::EFI/BOOT/BOOTX64.EFI

disk.img:
	dd if=/dev/zero of=disk.img bs=1K count=5760
	mformat -i disk.img -h 4 -t 80 -s 36
	mmd -oi disk.img EFI EFI/BOOT

run: efi
	qemu-system-$(ARCH) -serial stdio -bios /usr/share/ovmf/OVMF.fd -hda disk.img

debug: efi
	qemu-system-$(ARCH) -S -s -serial stdio -bios /usr/share/ovmf/OVMF.fd -hda disk.img

kernel/linux-$(KERNEL_VERSION).tar.xz:
	wget -P kernel/ https://cdn.kernel.org/pub/linux/kernel/v$(KERNEL_VERSION_MAJOR).x/linux-$(KERNEL_VERSION).tar.xz

kernel/linux/: kernel/linux-$(KERNEL_VERSION).tar.xz
	tar -xf $< -C kernel --skip-old-files
	mv kernel/linux-$(KERNEL_VERSION) kernel/linux

kernel/linux/.config: kernel/$(ARCH).config
	$(MAKE) -j8 -C kernel/linux ARCH=$(ARCH) tinyconfig KCONFIG_ALLCONFIG=../$(ARCH).config

kernel/linux/arch/x86/boot/bzImage: kernel/linux/ kernel/linux/.config etc/initramfs $(BINARIES) etc/rc
	$(MAKE) -j4 -C $<

${BUILD_DIR}/%: $(SOURCE_DIR)/%.c
	@mkdir -p bin
	$(CC) --static -g $^ -o $@

proper:
	make -C kernel/linux mrproper

clean:
	make -C kernel/linux clean
	rm -r bin
	rm disk.img
