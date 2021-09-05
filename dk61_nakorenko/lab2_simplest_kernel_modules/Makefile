# based on https://linux.die.net/lkmpg/hello2.html by Peter Jay Salzman
# (!) using paths with spaces may not work with Kbuild

# this is what is used by Kbuild
obj-m += src/firstmod.o

# directory containing Makefile for kernel build
KBUILDDIR := /lib/modules/4.19.69-1-MANJARO/build/

.PHONY: modules clean tidy

# recur to the original kernel Makefile with some additions
modules:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" modules

tidy:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" clean

# we link original clean to target named tidy
clean:
	-rm -f src/modules.order Module.symvers
	-rm -rf src/.tmp_versions
	-rm -f src/$(obj-m) $(obj-m:.o=.mod.o) $(obj-m:.o=.mod.c)
	-rm -f src/$(addsuffix .cmd,$(addprefix .,$(obj-m)))
	-rm -f src/$(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.ko)))
	-rm -f src/$(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.mod.o)))

