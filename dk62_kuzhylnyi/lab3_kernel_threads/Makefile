# based on https://linux.die.net/lkmpg/hello2.html by Peter Jay Salzman
# (!) using paths with spaces may not work with Kbuild


ARCH:=armv7l
CROSS_COMPILE:=gcc-arm-none-eabi

CC=$(CROSS_COMPILE)gcc

# this is what is used by Kbuild
obj-m += src/firstmod.o
EXTRA_CFLAGS := -O2 -std=gnu11 -Wno-declaration-after-statement 
# CFLAGS_firstmod.o =  -std=gnu11
# directory containing Makefile for kernel build
KBUILDDIR := /lib/modules/$(shell uname -r)/build/


.PHONY: modules clean tidy

# recur to the original kernel Makefile with some additions
modules:
	$(MAKE)  -C "$(KBUILDDIR)" M="$(PWD)" modules

tidy:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" clean

# we link original clean to target named tidy
clean:
	-rm -f modules.order Module.symvers
	-rm -rf .tmp_versions
	-rm -f $(obj-m) $(obj-m:.o=.mod.o) $(obj-m:.o=.mod.c)
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m)))
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.ko)))
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.mod.o)))

	
