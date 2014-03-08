obj-m := hid-sensor-trigger.o hid-sensor-hub.o \
	hid-sensor-als.o hid-sensor-accel-3d.o hid-sensor-gyro-3d.o hid-sensor-magn-3d.o hid-sensor-incl-3d.o \
	hid-sensor-orient.o hid-sensor-custom.o hid-sensor-generic.o

KDIR := /usr/src/kernels/$(shell uname -r)
PWD := $(shell pwd)
#ccflags-y := -E


default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

hid-example:	hid-example.c
	cc hid-example.c -o hid-example

clean:
	rm -rf *~ *.o *.mod.c .hid-sensor* .tmp_versions modules.order Module.symvers

veryclean:	clean
	rm -f *.ko

install:
	install --owner=root --group=root --mode=644 hid-sensor-hub.ko /usr/lib/modules/$(shell uname -r)/kernel/drivers/hid
	install --owner=root --group=root --mode=644 hid-sensor-trigger.ko /usr/lib/modules/$(shell uname -r)/kernel/drivers/iio/common/hid-sensors
	install --owner=root --group=root --mode=644 hid-sensor-accel-3d.ko /usr/lib/modules/$(shell uname -r)/kernel/drivers/iio/accel
	install --owner=root --group=root --mode=644 hid-sensor-als.ko /usr/lib/modules/$(shell uname -r)/kernel/drivers/iio/light
	install --owner=root --group=root --mode=644 hid-sensor-gyro-3d.ko /usr/lib/modules/$(shell uname -r)/kernel/drivers/iio/gyro
	install --owner=root --group=root --mode=644 hid-sensor-magn-3d.ko /usr/lib/modules/$(shell uname -r)/kernel/drivers/iio/magnetometer 
