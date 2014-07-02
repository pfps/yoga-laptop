yoga-laptop
===========

Systems and information to make Lenovo Ideapad Yoga laptops work better.
Best tested for Yoga 2 Pro, but most of this works on other Ideapad Yogas.
Thinkpad Yogas may need modifications, and can't use ideapad-laptop.

These systems include kernel modules, so you have to have added
kernel-headers and kernel-devel to your system.  For the orientation program
and sensor drivers you also need to have the IIO subsystem included in your
system.  If you are running a kernel older than 3.13 you may have problems.

1/ yoga_laptop/ideapad-laptop.c

   A patch to the ideapad-laptop kernel module to make Wifi work on yogas.
   The ideapad-laptop moodule also handles several ACPI-related keys on the
   Yoga keyboard.   A similar patch has been backported to Fedora, and is in
   3.15 (I think, 3.16 for sure) kernels. 

   To compile and install (warning - this installs a kernel module and may
   break your system, and has to be done each time you install a new kernel,
   after booting into the new kernel):
	make ideapad-laptop
	sudo make ideapad-laptop-install
   The installation also sets up udev rules and an xmodmap file to handle
   the touchpad_toggle and break keys that are not usually handled correctly.

   Reboot your system to get load the module

2/ sensor drivers

   Patched 3.14 drivers for five of the sensors in the Yoga laptops.  The
   patch adds a quirk so that the hub initializes correctly.  The quirk has
   been added to 3.15 and 3.15 should have better drivers than the ones
   here.

   To compile and install (warning - this installs several kernel modules and
   may break your system, and has to be done each time you install a new kernel,
   after booting into the new kernel):
	make sensors-drivers
	sudo make sensors-drivers-install

   Reboot your system to load the drivers

3/ orientation and light programs

   The orientation program re-orients the screen so that the top of the
   screen is physically up.  The program needs the hid-sensor-accel-3d
   sensor driver.  See "docs/Orientation and rotation" for more information.

   The light program adjusts the brightness of the screen in response to
   changes in ambient brightness.  The program needs the hid-sensor-als
   sensor driver.

   To compile and install (after first compiling and installing the sensor
   drivers): 
	make programs
	sudo make programs-install

