#obj-m := yoga_laptop.o test.o
obj-m := ideapad-laptop.o
#KDIR := /lib/modules/$(shell uname -r)/build
KDIR := /usr/src/kernels/$(shell uname -r)
PWD := $(shell pwd)
default:
	@echo NOTE:  You need to have the kernel-devel package installed
	echo $(KDIR)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.mod.c .tmp_versions .ideapad* modules.order Module.symvers

veryclean: clean
	rm -f *.ko


install-driver:
	install --owner=root --group=root --mode=644 ideapad-laptop.ko /usr/lib/modules/$(shell uname -r)/kernel/drivers/platform/x86

install-keys:
	install --owner=root --group=root --mode=644 90-yoga-keyboard.hwdb /etc/udev/hwdb.d
	udevadm hwdb --update
	@echo NOTE: You also need to cp -pr xkeycode "~/.Xmodmap"
# How about adding "keycode 202 = Break" to /etc/X11/Xmodmap??

install: install-driver install-keys
