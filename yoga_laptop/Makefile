include /etc/os-release 

obj-m := ideapad-laptop.o
PWD := $(shell pwd)

ifeq "$(NAME)" "Fedora"
KDIR := /usr/src/kernels/$(shell uname -r)
else
KDIR := /usr/src/linux-headers-$(shell uname -r)
endif
MODDIR := /lib/modules/$(shell uname -r)/kernel/drivers/platform/x86

LOGNAME := $(shell /usr/bin/logname)

default:
	@echo NOTE:  You need to have the kernel-devel package installed
	echo $(KDIR)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
install: install-driver install-keys

install-driver:
	install --owner=root --group=root --mode=644 ideapad-laptop.ko $(MODDIR)

install-keys:
	install --owner=root --group=root --mode=644 90-yoga-keyboard.hwdb /etc/udev/hwdb.d
	udevadm hwdb --update
	cp -pr xkeycode /home/${LOGNAME}/.Xmodmap

clean:
	rm -rf *.o *.mod.c .tmp_versions .ideapad* modules.order Module.symvers

veryclean: clean
	rm -f *.ko

