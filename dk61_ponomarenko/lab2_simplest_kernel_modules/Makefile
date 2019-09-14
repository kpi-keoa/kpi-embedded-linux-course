# based on https://linux.die.net/lkmpg/hello2.html by Peter Jay Salzman
# (!) using paths with spaces may not work with Kbuild

# this is what is used by Kbuild
obj-m += src/firstmod.o

# directory containing Makefile for kernel build
KBUILDDIR := /lib/modules/$(shell uname -r)/build/

.PHONY: modules clean tidy

# recur to the original kernel Makefile with some additions
modules:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" modules

tidy:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" clean

# we link original clean to target named tidy
clean:
	-rm -rf src/modules.order src/Module.symvers
	-rm -rf src/.tmp_versions
	-rm -rf src/$(obj-m) $(obj-m:.o=.mod.o) src/$(obj-m:.o=.mod.c)
	-rm -rf src/$(addsuffix .cmd,$(addprefix .,$(obj-m)))
	-rm -rf src/$(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.ko)))
	-rm -rf src/$(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.mod.o)))
