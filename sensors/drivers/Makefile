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

dist:	clean
	rm *.ko

