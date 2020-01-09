# (!) using paths with spaces may not work with Kbuild
INCDIR := inc
SRCDIR := src
obj-m := hivemod.o

obj-m += $(addprefix $(SRCDIR)/,$(OBJ))

EXTRA_CFLAGS += -I$(INCDIR) -std=gnu11

KBUILDDIR := /lib/modules/$(shell uname -r)/build/

.PHONY: modules clean tidy all run test

all: | tidy modules run

test:
	gcc src/user_app_test.c -o .build/user_test
	cd .build
	sudo ./user_test

.build:
	cp -r ./$(SRCDIR) ./$@
	cp Makefile $@/

modules: .build
	cd $< && $(MAKE) -C "$(KBUILDDIR)" M="$(PWD)/$<" modules

tidy:
	-rm -rf .build

.ONESHELL:
clean:
	cd .build
	-rm -f modules.order Module.symvers
	-rm -rf .tmp_versions
	-rm -f $(obj-m) $(obj-m:.o=.mod.o) $(obj-m:.o=.mod.c)
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m)))
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.ko)))
	-rm -f $(addsuffix .cmd,$(addprefix .,$(obj-m:.o=.mod.o)))

show:
	sudo dmesg -k | tail -21

.ONESHELL:
run:
	sudo sh -c "insmod .build/$(obj-m:.o=.ko); sleep 1; dmesg -k | tail -10"

stop:
	sudo rmmod $(obj-m:.o=)



	
