SCRIPTS = setup unstick status

SOURCES = iio_utils.h generic_buffer.c orientation.c

EXECS = orientation generic_buffer

all: generic_buffer orientation

generic_buffer: generic_buffer.c iio_utils.h
	cc generic_buffer.c -o generic_buffer

orientation: orientation.c iio_utils.h
	@echo NOTE: In most cases you need to install drivers in the drivers directory for orientation to work
	cc orientation.c -o orientation

hinge: hinge.c iio_utils.h
	cc hinge.c -o hinge

install:
	@echo "Because the accelerometer is not reliably set up in standard kernels"
	@echo "it is better to run the orientation program directly, at least for"
	@echo "the first few times.  See README for more information."

