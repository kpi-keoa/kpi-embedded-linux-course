# (!) using paths with spaces may not work with Kbuild

# this is what is used by Kbuild
obj-m += hivemod.o

V ?= 2

# directory containing Makefile for kernel build
KBUILDDIR ?= ../linux-5.8.7

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

	
