ARCH                 = x86_64
KERNEL_VERSION_MAJOR = 6
KERNEL_VERSION       = 6.0.9
COMMIT               = $(shell git log -n 1 --pretty=format:"%h@%cs")

TOOLS                = init sh

BUILD_DIR            = build
SOURCE_DIR           = tools

BINARIES             = $(foreach T, $(TOOLS), build/tools/$T)

all: directories kernel build/initramfs/initramfs.cpio.gz

run: all
	qemu-system-$(ARCH) -nographic \
                      -no-reboot \
                      -kernel build/kernel/linux-${KERNEL_VERSION}/arch/x86/boot/bzImage \
                      -initrd build/initramfs/initramfs.cpio.gz \
                      --append "panic=1 loglevel=7 console=ttyS0"

build/kernel/linux-$(KERNEL_VERSION).tar.xz: 
	@echo --- Getting and verifying kernel source tarball
	wget -P build/kernel/ https://cdn.kernel.org/pub/linux/kernel/v$(KERNEL_VERSION_MAJOR).x/linux-$(KERNEL_VERSION).tar.xz
	wget -P build/kernel/ https://cdn.kernel.org/pub/linux/kernel/v$(KERNEL_VERSION_MAJOR).x/linux-$(KERNEL_VERSION).tar.sign
	cd build/kernel && xz -cd linux-$(KERNEL_VERSION).tar.xz | gpg --verify linux-$(KERNEL_VERSION).tar.sign -

build/kernel/linux-$(KERNEL_VERSION)/: build/kernel/linux-$(KERNEL_VERSION).tar.xz
	@echo --- Extracting kernel source tarball
	tar -xf $(input) -C build/kernel --skip-old-files		

build/kernel/linux-${KERNEL_VERSION}/arch/x86/boot/bzImage: build/kernel/linux-$(KERNEL_VERSION)/
	@echo --- Building kernel
	cp kernel/$(ARCH).config $<
	$(MAKE) -C $< ARCH=$(ARCH) tinyconfig KCONFIG_ALLCONFIG=$(ARCH).config
	$(MAKE) -C $<

kernel: build/kernel/linux-${KERNEL_VERSION}/arch/x86/boot/bzImage

build/initramfs/initramfs.cpio.gz: $(BINARIES)
	@echo --- Building the root filesystem
	mkdir -p build/initramfs/fs
	cd build/initramfs/fs && \
	mkdir -p bin etc

  # binaries
	cp build/tools/init build/initramfs/fs/
	cp build/tools/sh build/initramfs/fs/bin/

	@echo --- Compressing the root filesystem
	cd build/initramfs/fs && \
	find . | cpio -oH newc -R root:root | gzip > ../initramfs.cpio.gz

build/tools/%: tools/%/*.c
	@echo --- Building $* from $^
	$(CC) --static -g -DTOOLKIT_VERSION=\"$(COMMIT)\" -I$(dir $<) $^ -o $@

directories:
	@mkdir -p build/tools
	@mkdir -p build/kernel

proper:
	make -C build/kernel/linux-${KERNEL_VERSION} mrproper

clean:
	rm -r build/tools
	rm -r build/initramfs
