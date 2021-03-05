# (!) using paths with spaces may not work with Kbuild

# this is what is used by Kbuild
obj-m += mymod.o

V ?= 2

# kbuild dir
KBUILDDIR ?= $(KERNEL)/linux-5.10.17

.PHONY: modules tidy

# recur to the original kernel Makefile with some additions
modules:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" V=$(V) modules

tidy:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" V=$(V) clean

