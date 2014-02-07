SCRIPTS = setup unstick status

SOURCES = iio_utils.h generic_buffer.c orientation.c

EXECS = orientation generic_buffer

all: generic_buffer orientation hinge

generic_buffer: generic_buffer.c iio_utils.h
	cc generic_buffer.c -o generic_buffer

orientation: orientation.c iio_utils.h
	cc orientation.c -o orientation

hinge: hinge.c iio_utils.h
	cc hinge.c -o hinge

dist:
	tar -cf orientation.tar Makefile README TODO ${SCRIPTS} ${SOURCES} ${EXECS}
