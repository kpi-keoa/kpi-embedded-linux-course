# based on https://linux.die.net/lkmpg/hello2.html by Peter Jay Salzman
# piece of code with directory organization taken from Dana Dovzhenko's 
# Makefile https://github.com/kpi-keoa/kpi-embedded-linux-course/blob/master/
# 	dk62_dovzhenko/lab2_simplest_kernel_modules/Makefile
# (!) using paths with spaces may not work with Kbuild


SRCDIR = src
BUILDDIR = .build
EXTRA_CFLAGS:= -std=gnu11

# this is what is used by Kbuild
obj-m += firstmod.o

# directory containing Makefile for kernel build
KBUILDDIR := /lib/modules/$(shell uname -r)/build/

.PHONY: modules clean tidy

# recur to the original kernel Makefile with some additions

make all: tidy modules

# make build directory and recursive copy files to BUILDDIR
$(BUILDDIR):
	mkdir $@
	cp -r ./Makefile $(SRCDIR)/* $(BUILDDIR)/

modules: $(BUILDDIR)
	cd $(BUILDDIR)
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)/$(BUILDDIR)" modules

run:
	sudo insmod $(BUILDDIR)/$(obj-m:.o=.ko) number_of_cycles=100000 \
number_of_threads=5

stop:
	sudo rmmod $(obj-m:.o=)

show:
	sudo dmesg -k | tail -10

# the tidy has been upgraded because there was a bug: when updating 
# the firstmod.c file, its copy wasn't upgrading in the src directory
# and the compilation was held and used with the old version of the file
tidy:
	$(MAKE) -C "$(KBUILDDIR)" M="$(PWD)/$(BUILDDIR)" clean
	rm -rf $(BUILDDIR)

# we link original clean to target named tidy
clean:
	cd $(BUILDDIR)
	-rm -f modules.order Module.symvers
	-rm -rf .tmp_versions
	-rm -f $(obj-m) $(obj-m:.o=.mod.o) $(obj-m:.o=.mod.c)
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m)))
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.ko)))
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.mod.o)))

help:
	@echo "if you compile project for the first time use: make modules"
	@echo "then you can just use:"
	@echo "		make all - to clean up and compile project again"
	@echo "		make run/stop - to insert/remove module"
	@echo "		make show - to show last 10 lines of kernel log" 
	@echo "		make clean - to clean your dir from build artefacts"
	@echo "		make tidy - to fully clean your fir"
	
