# based on https://linux.die.net/lkmpg/hello2.html by Peter Jay Salzman


SRCDIR = src
BUILDDIR = .build
EXTRA_CFLAGS:= -std=gnu11
# this is what is used by Kbuild
obj-m += kthread_mod.o

# directory containing Makefile for kernel build
KBUILDDIR := /lib/modules/$(shell uname -r)/build/

.PHONY: modules clean tidy

# recur to the original kernel Makefile with some additions
# make build directory and recursive copy files to BUILDDIR
$(BUILDDIR):
	mkdir $@
	cp -r ./Makefile $(SRCDIR)/* $(BUILDDIR)/

all: modules

modules: $(BUILDDIR)
	cd $(BUILDDIR)
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)/$(BUILDDIR)" modules

run:
	sudo insmod $(BUILDDIR)/$(obj-m:.o=.ko) kth_num=5 incr_val=10000

stop:
	sudo rmmod $(obj-m:.o=)

show:
	sudo dmesg -k | tail -20

# the tidy has been upgraded because there was a bug: when updating 
# the firstmod.c file, its copy wasn't upgrading in the src directory
# and the compilation was held and used with the old version of the file
tidy:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)/$(BUILDDIR)" clean
	rm -rf $(BUILDDIR)

# we link original clean to target named tidy
clean:
	cd $(BUILDDIR)
	-rm -f modules.order Module.symvers
	-rm -rf .tmp_versions
	-rm -f $(obj-m) $(obj-m:.o=.mod.o) $(obj-m:.o=.mod.c)
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m)))
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.ko)))
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.mod.o)))

	
