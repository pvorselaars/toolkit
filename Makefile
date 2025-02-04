ARCH                 = x86_64
KERNEL_VERSION_MAJOR = 6
KERNEL_VERSION       = 6.8.7
COMMIT               = $(shell git log -n 1 --pretty=format:"%h@%cs")

TOOLS                = init sh echo ls cp mount cat

BUILD_DIR            = bin
SOURCE_DIR           = src

BINARIES             = $(foreach T, $(TOOLS), $(BUILD_DIR)/$T)

.PHONY: proper clean kernel efi qemu debug

efi: disk.img linux/linux-${KERNEL_VERSION}/arch/x86/boot/bzImage
	mcopy -oi $^ ::EFI/BOOT/BOOTX64.EFI

disk.img:
	dd if=/dev/zero of=disk.img bs=1K count=5760
	mformat -i disk.img -h 4 -t 80 -s 36
	mmd -oi disk.img EFI EFI/BOOT

run: efi
	qemu-system-$(ARCH) -serial stdio \
                      -bios /usr/share/ovmf/OVMF.fd \
                      -drive file=disk.img,format=raw

debug: efi
	qemu-system-$(ARCH) -S -s \
                      -serial stdio \
                      -bios /usr/share/ovmf/OVMF.fd \
                      -drive file=disk.img,format=raw

linux/linux-$(KERNEL_VERSION).tar.xz:
	@echo --- Getting kernel source tarball
	wget -P linux/ https://cdn.kernel.org/pub/linux/kernel/v$(KERNEL_VERSION_MAJOR).x/linux-$(KERNEL_VERSION).tar.xz

linux/linux-$(KERNEL_VERSION)/: linux/linux-$(KERNEL_VERSION).tar.xz
	@echo --- Extracting kernel source tarball
	tar -xf $< -C linux --skip-old-files

linux/linux-$(KERNEL_VERSION)/.config: linux/$(ARCH).config
	$(MAKE) -j8 -C linux/linux-${KERNEL_VERSION} ARCH=$(ARCH) tinyconfig KCONFIG_ALLCONFIG=../$(ARCH).config

linux/linux-${KERNEL_VERSION}/arch/x86/boot/bzImage: linux/linux-$(KERNEL_VERSION)/ linux/linux-$(KERNEL_VERSION)/.config etc/initramfs $(BINARIES) etc/rc
	@echo --- Building kernel
	$(MAKE) -j8 -C $<

${BUILD_DIR}/%: $(SOURCE_DIR)/%.c
	@echo --- Building $@ from $^
	@mkdir -p bin
	$(CC) --static -g -DTOOLKIT_VERSION=\"$(COMMIT)\" -I$(dir $<) $^ -o $@


kernel: linux/linux-${KERNEL_VERSION}/arch/x86/boot/bzImage

proper:
	make -C linux/linux-${KERNEL_VERSION} mrproper

clean:
	make -C linux/linux-${KERNEL_VERSION} clean
	rm -r bin
	rm disk.img
