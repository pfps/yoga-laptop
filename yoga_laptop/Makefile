include /etc/os-release 

obj-m := ideapad-laptop.o
PWD := $(shell pwd)

ifeq "$(NAME)" "Fedora"
KDIR := /usr/src/kernels/$(shell uname -r)
MODNAME := ideapad-laptop.ko
else ifeq ("$(NAME)",$(filter "$(NAME)","arch" "Antergos"))
KDIR := /usr/lib/modules/$(shell uname -r)/build
MODNAME := ideapad-laptop.ko.gz
else
KDIR := /usr/src/linux-headers-$(shell uname -r)
MODNAME := ideapad-laptop.ko
endif
MODDIR := /lib/modules/$(shell uname -r)/kernel/drivers/platform/x86




LOGNAME := $(shell /usr/bin/logname)

default:
	@echo NOTE:  You need to have the kernel-devel package installed
	echo $(KDIR)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	cp ideapad-laptop.ko ideapad-laptop.ko.or
	gzip -9 ideapad-laptop.ko
	mv ideapad-laptop.ko.or ideapad-laptop.ko

install: install-driver install-keys

uninstall: uninstall-driver uninstall-keys

install-driver:	
	install --owner=root --group=root --mode=644 $(MODNAME) $(MODDIR)

install-keys:
	install --owner=root --group=root --mode=644 90-yoga-keyboard.hwdb /usr/lib/udev/hwdb.d
	udevadm hwdb --update
	cp -pr xkeycode /home/${LOGNAME}/.Xmodmap

uninstall-driver:	
	rm -f $(MODDIR)/$(MODNAME)

uninstall-keys:
	rm -f /etc/udev/hwdb.d/90-yoga-keyboard.hwdb
	udevadm hwdb --update
	rm -f /home/${LOGNAME}/.Xmodmap

clean:
	rm -rf *.o *.mod.c .tmp_versions .ideapad* modules.order Module.symvers

veryclean: clean
	rm -f *.ko.gz *.ko
