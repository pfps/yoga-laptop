obj-m := hid-sensor-incl-3d.o hid-sensor-orient.o hid-sensor-custom.o hid-sensor-generic.o
#obj-m := hid-sensor-generic.o
#obj-m := hid-sensor-orient.o
#obj-m := hid-sensor-incl-3d.o
#obj-m := hid-sensor-hub.o
KDIR := /usr/src/kernels/$(shell uname -r)
PWD := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

hid-example:	hid-example.c
	cc hid-example.c -o hid-example

clean:
	rm -f *~ *.o modules.order Module.symvers
