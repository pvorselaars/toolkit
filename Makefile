ARCH                 = x86_64
KERNEL_VERSION_MAJOR = 6
KERNEL_VERSION       = 6.12


BUILD_DIR            = bin
SOURCE_DIR           = src

TOOLS                = $(notdir $(basename $(wildcard $(SOURCE_DIR)/*.c)))

BINARIES             = $(addprefix $(BUILD_DIR)/, $(TOOLS))

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
	$(MAKE) -j4 -C kernel/linux ARCH=$(ARCH) tinyconfig KCONFIG_ALLCONFIG=../$(ARCH).config

kernel/linux/arch/x86/boot/bzImage: kernel/linux/ kernel/linux/.config etc/initramfs $(BINARIES) etc/rc
	$(MAKE) -j4 -C $<

etc/initramfs:
	echo dir /proc 755 0 0 > $@
	echo dir /dev 755 0 0 >> $@
	echo nod /dev/console 644 0 0 c 5 1 >> $@
	echo dir /bin 755 1000 1000 >> $@
	for b in ${BINARIES}; do \
		echo "file /$$b ../../$$b 771 0 0" >> $@; \
	done
	echo dir /etc 755 0 0 >> $@
	echo file /etc/rc ../../etc/rc 711 0 0 >> $@
	echo slink /init /bin/init 770 0 0 >> $@

tools: ${BINARIES}

${BUILD_DIR}/%: $(SOURCE_DIR)/%.c $(SOURCE_DIR)/syscall.S $(SOURCE_DIR)/crt.S
	@mkdir -p bin
	$(CC) -static -nostdlib -g $^ -o $@

proper:
	make -C kernel/linux mrproper

clean:
	make -C kernel/linux clean
	rm -r bin
	rm disk.img
