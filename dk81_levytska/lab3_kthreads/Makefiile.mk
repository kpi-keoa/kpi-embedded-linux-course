# (!) using paths with spaces may not work with Kbuild

# this is what is used by Kbuild
obj-m += lab3_sync.o
obj-m += lab3_w_lock.o
obj-m += lab3_own_sync.o

V ?= 2

# directory containing Makefile for kernel build
KBUILDDIR ?= ~/kernel/linux-5.15.7

# Do not print "Entering directory" on recursive make call if not verbose
MAKEFLAGS += $(if $(value V),,--no-print-directory)

.PHONY: modules clean tidy

# recur to the original kernel Makefile with some additions
modules:
  $(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" V=$(V) modules

tidy:
  $(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" V=$(V) clean

# we link original clean to target named tidy