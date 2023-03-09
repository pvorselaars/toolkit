ARCH                 = x86_64
KERNEL_VERSION_MAJOR = 6
KERNEL_VERSION       = 6.0.9
COMMIT               = $(shell git log -n 1 --pretty=format:"%h@%cs")

TOOLS                = sh echo

BUILD_DIR            = build
SOURCE_DIR           = tools
FS_DIR               = $(BUILD_DIR)/initramfs/fs

INIT                 = $(BUILD_DIR)/init/init
BINARIES             = $(foreach T, $(TOOLS), $(BUILD_DIR)/tools/$T)

all: initramfs kernel

run: all
	qemu-system-$(ARCH) -nographic \
                      -no-reboot \
                      -kernel build/kernel/linux-${KERNEL_VERSION}/arch/$(ARCH)/boot/bzImage \
                      -initrd build/initramfs/initramfs.cpio.gz \
                      --append "panic=1 loglevel=7 console=ttyS0"

build/kernel/linux-$(KERNEL_VERSION).tar.xz: 
	@echo --- Getting and verifying kernel source tarball
	wget -P build/kernel/ https://cdn.kernel.org/pub/linux/kernel/v$(KERNEL_VERSION_MAJOR).x/linux-$(KERNEL_VERSION).tar.xz
	wget -P build/kernel/ https://cdn.kernel.org/pub/linux/kernel/v$(KERNEL_VERSION_MAJOR).x/linux-$(KERNEL_VERSION).tar.sign
	cd build/kernel && xz -cd linux-$(KERNEL_VERSION).tar.xz | gpg --verify linux-$(KERNEL_VERSION).tar.sign -

build/kernel/linux-$(KERNEL_VERSION)/: build/kernel/linux-$(KERNEL_VERSION).tar.xz
	@echo --- Extracting kernel source tarball
	tar -xf $< -C build/kernel --skip-old-files		

build/kernel/linux-${KERNEL_VERSION}/arch/x86/boot/bzImage: build/kernel/linux-$(KERNEL_VERSION)/
	@echo --- Building kernel
	cp kernel/$(ARCH).config $<
	$(MAKE) -C $< ARCH=$(ARCH) tinyconfig KCONFIG_ALLCONFIG=$(ARCH).config
	$(MAKE) -C $<

kernel: build/kernel/linux-${KERNEL_VERSION}/arch/x86/boot/bzImage

initramfs: build/initramfs/initramfs.cpio.gz

build/initramfs/initramfs.cpio.gz: directories $(foreach T,$(TOOLS), $(FS_DIR)/bin/$T) $(FS_DIR)/init
	@echo --- Compressing the root filesystem
	cd build/initramfs/fs;  find . | cpio -oH newc -R root:root | gzip > ../initramfs.cpio.gz

$(FS_DIR)/bin/%: build/tools/%
	cp $^ $@

$(FS_DIR)/%: build/init/%
	cp $^ $@

build/init/%: tools/%/*.c
	@echo --- Building $* from $^
	$(CC) --static -g -DTOOLKIT_VERSION=\"$(COMMIT)\" -I$(dir $<) $^ -o $@

build/tools/%: tools/%/*.c
	@echo --- Building $* from $^
	$(CC) --static -g -DTOOLKIT_VERSION=\"$(COMMIT)\" -I$(dir $<) $^ -o $@

directories:
	@mkdir -p build/kernel
	@mkdir -p build/init
	@mkdir -p build/tools
	@mkdir -p build/initramfs/fs
	@mkdir -p build/initramfs/fs/bin

proper:
	make -C build/kernel/linux-${KERNEL_VERSION} mrproper

clean:
	rm -r build/init
	rm -r build/tools
	rm -r build/initramfs
