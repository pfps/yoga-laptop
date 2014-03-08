#obj-m := yoga_laptop.o test.o
obj-m := ideapad-laptop.o
#KDIR := /lib/modules/$(shell uname -r)/build
KDIR := /usr/src/kernels/$(shell uname -r)
PWD := $(shell pwd)
default:
	echo $(KDIR)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -f *.o *.mod.c modules.order Module.symvers

veryclean: clean
	rm -f *.ko


install:
	install  --owner=root --group=root --mode=644 ideapad_laptop.ko /usr/lib/modules/$(shell uname -r)/kernel/drivers/platform/x86
