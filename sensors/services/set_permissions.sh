#! /bin/sh
#
# /opt/set_permissions.sh

/bin/chgrp users /sys/bus/iio/devices/iio:device*/scan_elements/in_*_en /sys/bus/iio/devices/iio:device*/buffer/* /sys/bus/iio/devices/iio:device*/trigger/current_trigger
/bin/chmod g+w   /sys/bus/iio/devices/iio:device*/scan_elements/in_*_en /sys/bus/iio/devices/iio:device*/buffer/* /sys/bus/iio/devices/iio:device*/trigger/current_trigger
/bin/chgrp users /dev/iio:device*
/bin/chmod g+rw  /dev/iio:device*
