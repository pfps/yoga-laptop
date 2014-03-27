obj-m := ideapad-laptop.o
PWD := $(shell pwd)

# this is a simple check for which distibution we are on - a better solution is probably need
ifeq ($shell grep Fedora /etc/issue,"")
KDIR := /usr/src/kernels/$(shell uname -r)
else
KDIR := /usr/src/linux-headers-$(shell uname -r)
endif
MODDIR := /lib/modules/$(shell uname -r)/kernel/drivers/platform/x86


default:
	@echo NOTE:  You need to have the kernel-devel package installed
	echo $(KDIR)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.mod.c .tmp_versions .ideapad* modules.order Module.symvers

veryclean: clean
	rm -f *.ko

install-driver:
	install --owner=root --group=root --mode=644 ideapad-laptop.ko $(MODDIR)

install-keys:
	install --owner=root --group=root --mode=644 90-yoga-keyboard.hwdb /etc/udev/hwdb.d
	udevadm hwdb --update
	@echo NOTE: You also need to cp -pr xkeycode "~/.Xmodmap"
# How about adding "keycode 202 = Break" to /etc/X11/Xmodmap??

install: install-driver install-keys
