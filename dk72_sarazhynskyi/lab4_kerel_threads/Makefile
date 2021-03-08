# (!) using paths with spaces may not work with Kbuild

# this is what is used by Kbuild
obj-m += no_lock.o kernel_lock.o my_lock.o

V ?= 2

# directory containing Makefile for kernel build
KBUILDDIR ?= ../linux-5.10.17
PWD = $(CURDIR)

# Do not print "Entering directory" on recursive make call if not verbose
MAKEFLAGS += $(if $(value V),,--no-print-directory)

.PHONY: modules clean tidy run

# recur to the original kernel Makefile with some additions
modules: tidy
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" V=$(V) modules

tidy:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" V=$(V) clean

