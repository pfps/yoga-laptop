obj-m := hid-sensor-trigger.o hid-sensor-hub.o \
	hid-sensor-als.o hid-sensor-accel-3d.o hid-sensor-gyro-3d.o hid-sensor-magn-3d.o hid-sensor-incl-3d.o \
	hid-sensor-orient.o hid-sensor-custom.o hid-sensor-generic.o

PWD := $(shell pwd)

ifeq ($shell grep Fedora /etc/issue,"")
KDIR := /usr/src/linux-headers-$(shell uname -r)
DDIR := /lib/modules/$(shell uname -r)/kernel/drivers/
else
KDIR := /usr/src/kernels/$(shell uname -r)
DDIR := /usr/lib/modules/$(shell uname -r)/kernel/drivers/
endif



default:
	echo $(KDIR)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

hid-example:	hid-example.c
	cc hid-example.c -o hid-example

clean:
	rm -rf *~ *.o *.mod.c .hid-sensor* .tmp_versions modules.order Module.symvers

veryclean:	clean
	rm -f *.ko

install:
	install --owner=root --group=root --mode=644 hid-sensor-hub.ko $(DDIR)/hid
	install --owner=root --group=root --mode=644 hid-sensor-trigger.ko $(DDIR)/iio/common/hid-sensors
	install --owner=root --group=root --mode=644 hid-sensor-accel-3d.ko $(DDIR)/iio/accel
	install --owner=root --group=root --mode=644 hid-sensor-als.ko $(DDIR)/iio/light
	install --owner=root --group=root --mode=644 hid-sensor-gyro-3d.ko $(DDIR)/iio/gyro
	install --owner=root --group=root --mode=644 hid-sensor-magn-3d.ko $(DDIR)/iio/magnetometer 
