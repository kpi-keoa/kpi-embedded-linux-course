# (!) using paths with spaces may not work with Kbuild

# this is what is used by Kbuild
TARGET?=no_sync

obj-m += $(TARGET).o

V ?= 2

# kernel dir and kbuild dir
KERNEL ?= ~/kernel
KBUILDDIR ?= $(KERNEL)/linux-5.9.1

RAM ?= 512M

# Do not print "Entering directory" on recursive make call if not verbose
MAKEFLAGS += $(if $(value V),,--no-print-directory)

.PHONY: modules clean tidy

# recur to the original kernel Makefile with some additions
modules:
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

sim: modules
	cd $(KERNEL) &&\
	qemu-system-x86_64 -no-kvm -m $(RAM) -smp 4 -kernel  "bzImage" \
	-initrd "initramfs.cpio.gz" -append "console=ttyS0" -nographic \
	-drive file=fat:rw:$(PWD),format=raw,media=disk
