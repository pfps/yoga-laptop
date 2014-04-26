SCRIPTS = setup unstick status

SOURCES = iio_utils.h generic_buffer.c orientation.c light.c

EXECS = orientation generic_buffer light

all: generic_buffer orientation light

generic_buffer: generic_buffer.c iio_utils.h
	cc generic_buffer.c -o generic_buffer

orientation: orientation.c iio_utils.h
	@echo NOTE: In most cases you need to install drivers in the drivers directory for orientation to work
	cc orientation.c -o orientation

hinge: hinge.c iio_utils.h
	cc hinge.c -o hinge

light: light.c
	@echo "This step requires root priviledges"
	cc light.c -o light
	chown root:root light
	chmod u+s light

install:
	@echo "Because the accelerometer is not reliably set up in standard kernels"
	@echo "it is better to run the orientation program directly, at least for"
	@echo "the first few times.  See README for more information."

