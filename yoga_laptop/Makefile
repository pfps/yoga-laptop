obj-m := yoga_laptop.o
#KDIR := /lib/modules/$(shell uname -r)/build
KDIR := /usr/src/kernels/$(shell uname -r)
PWD := $(shell pwd)
default:
	echo $(KDIR)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

dist:
	tar cvf yoga_laptop.tar README Makefile fix-keys yoga_laptop.c yoga_laptop.ko
