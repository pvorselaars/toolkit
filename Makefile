ARCH 								 = x86_64
KERNEL_VERSION_MAJOR = 6
KERNEL_VERSION 			 = 6.0.9

all: kernel build/initramfs/initramfs.cpio.gz

run: all
	qemu-system-$(ARCH) -nographic -no-reboot \
									    -kernel build/kernel/linux-$(KERNEL_VERSION)/arch/x86/boot/bzImage \
									    -initrd build/initramfs/initramfs.cpio.gz \
									    --append "panic=1 console=ttyS0"

build/kernel/linux-$(KERNEL_VERSION).tar.xz: 
	@echo --- Getting and verifying kernel source tarball
	@mkdir -p build/kernel
	wget -P build/kernel/ https://cdn.kernel.org/pub/linux/kernel/v$(KERNEL_VERSION_MAJOR).x/linux-$(KERNEL_VERSION).tar.xz
	wget -P build/kernel/ https://cdn.kernel.org/pub/linux/kernel/v$(KERNEL_VERSION_MAJOR).x/linux-$(KERNEL_VERSION).tar.sign
	cd build/kernel && xz -cd linux-$(KERNEL_VERSION).tar.xz | gpg --verify linux-$(KERNEL_VERSION).tar.sign -

build/kernel/linux-$(KERNEL_VERSION)/: build/kernel/linux-$(KERNEL_VERSION).tar.xz
	@echo --- Verifying and extracting kernel source tarball

	tar -xf $< -C build/kernel --skip-old-files		

build/kernel/linux-${KERNEL_VERSION}/arch/x86/boot/bzImage: build/kernel/linux-$(KERNEL_VERSION)/
	@echo --- Building kernel
	cp kernel/$(ARCH).config $<
	$(MAKE) -C $< ARCH=$(ARCH) tinyconfig KCONFIG_ALLCONFIG=$(ARCH).config
	$(MAKE) -C $<

kernel: build/kernel/linux-${KERNEL_VERSION}/arch/x86/boot/bzImage

build/initramfs/initramfs.cpio.gz: build/tools/init
	@echo --- Building and compressing the root filesystem
	cp build/tools/init build/initramfs/fs/
	cd build/initramfs/fs && find . | cpio -oH newc -R root:root | gzip > ../initramfs.cpio.gz

build/tools/%: tools/%.c
	@echo --- Building $@
	mkdir -p build/tools
	$(CC) --static $< -o $@

proper:
	make -C build/kernel/linux-${KERNEL_VERSION} mrproper

clean:
	rm -r build
