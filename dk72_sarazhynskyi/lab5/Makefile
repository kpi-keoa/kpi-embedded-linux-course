# (!) using paths with spaces may not work with Kbuild

# this is what is used by Kbuild
obj-m += delayed_work.o

V ?= 2

# kernel dir and kbuild dir
KERNEL ?= ~/kernel
KBUILDDIR ?= $(KERNEL)/linux-5.10.17

# Do not print "Entering directory" on recursive make call if not verbose
MAKEFLAGS += $(if $(value V),,--no-print-directory)

.PHONY: modules tidy

# recur to the original kernel Makefile with some additions
modules:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" V=$(V) modules

tidy:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" V=$(V) clean

