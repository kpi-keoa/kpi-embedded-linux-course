obj-m += lab3_deflock.o
obj-m += lab3_nolock.o
obj-m += lab3_mylock.o

# directory containing Makefile for kernel build
KBUILDDIR ?= ../linux-5.14.7

.PHONY: modules tidy

# recur to the original kernel Makefile with some additions
modules:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" modules

tidy:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)" clean
