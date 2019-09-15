# based on https://linux.die.net/lkmpg/hello2.html by Peter Jay Salzman
# (!) using paths with spaces may not work with Kbuild

# this is what is used by Kbuild
SRC = src/
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
	-rm -f modules.order Module.symvers
	-rm -rf .tmp_versions $(SRC).tmp_versions
	-rm -f *.cmd *.o 
	-rm -f $(SRC).*.cmd $(SRC)*.o $(SRC)firstmod.mod.c
	