# (!) using paths with spaces may not work with Kbuild

# this is what is used by Kbuild
obj-m += no_lock.o kernel_lock.o my_lock.o

V ?= 2

# directory containing Makefile for kernel build
KBUILDDIR ?= ../linux-5.8.8
PWD = $(CURDIR)

# variables connected with "run"

bzImagePATH = ../bzImage
initramfsPATH = ../initramfs.cpio.gz
RAM_SIZE = 1024M

# Do not print "Entering directory" on recursive make call if not verbose
MAKEFLAGS += $(if $(value V),,--no-print-directory)

.PHONY: modules clean tidy run

# recur to the original kernel Makefile with some additions
modules: tidy
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" V=$(V) modules

tidy:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" V=$(V) clean

# we link original clean to target named tidy
clean:
	-rm -rf .tmp_versions
	-rm -f modules.order .modules.order.cmd Module.symvers .Module.symvers.cmd
	-rm -f $(obj-m) $(obj-m:.o=.mod) $(obj-m:.o=.mod.o) $(obj-m:.o=.mod.c) .$(obj-m:.o=.mod.cmd)
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m)))
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.ko)))
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.mod.o)))

run:
	qemu-system-x86_64 -enable-kvm -m $(RAM_SIZE) -smp 4 -kernel  $(bzImagePATH) \
	-initrd $(initramfsPATH) -append "console=ttyS0" -nographic \
	-drive file=fat:rw:$(PWD),format=raw,media=disk
