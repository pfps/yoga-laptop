SCRIPTS = setup unstick status

SOURCES = iio_utils.h generic_buffer.c orientation.c light.c

EXECS = orientation generic_buffer light

LOGNAME := $(shell /usr/bin/logname)
GROUP := $(shell /usr/bin/id -gn `/usr/bin/logname`)

all: generic_buffer orientation light

generic_buffer: generic_buffer.c iio_utils.h
	cc generic_buffer.c $(CFLAGS) -o generic_buffer

orientation: orientation.c iio_utils.h
	cc orientation.c `pkg-config --cflags --libs libnotify` $(CFLAGS) -o orientation

hinge: hinge.c iio_utils.h
	cc hinge.c $(CFLAGS) -o hinge

light: light.c
	cc `pkg-config --cflags glib-2.0 --libs glib-2.0` $(CFLAGS) -o light light.c

install:	orientation permissions
	install --owner=root --group=root --mode=755 orientation /usr/bin
	install --owner=root --group=root --mode=755 light /usr/bin
	install --owner=root --group=root --mode 644 conf/85-orientation.rules /lib/udev/rules.d
	@echo NOTE: This does not install drivers from the drivers directory, but only install them if necessary.

clean:
	rm -f *.o orientation light generic_buffer
