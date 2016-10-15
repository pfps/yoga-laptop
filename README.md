Yoga - Laptop
===========


Note: Built initially for the Lenovo yoga 2 Pro.  Works for some other
Lenovo Yoga laptops.  Depends on which sensor chips are in the laptop.


Systems and information to make Lenovo Ideapad Yoga laptops work better.
Best tested for Yoga 2 Pro, but most of this works on other Ideapad Yogas.
Thinkpad Yogas may need modifications, and can't use ideapad-laptop.

These systems include kernel modules, so you have to have added
kernel-headers and kernel-devel to your system.  For the orientation program
and sensor drivers you also need to have the IIO subsystem included in your
system.  If you are running a kernel older than 3.13 you will very likely
have problems.  If you are running a current kernel you should already have
the required drivers and a working ideapad-laptop module.


1/ orientation and light programs

   The orientation program re-orients the screen so that the top of the
   screen is physically up.  The program needs the hid-sensor-accel-3d
   sensor driver.  See "docs/Orientation and rotation" for more information.

   The light program adjusts the brightness of the screen in response to
   changes in ambient brightness.  The program needs the hid-sensor-als
   sensor driver.

   There is also a generic program that can be used to test sensors -
   generic_buffer. 

   You need to have the libXrandr-devel package installed.

   To compile and install:
	make programs
	sudo make programs-install

   You may have to first install the sensor drivers, but only do this if
   necessary because these drivers are old and might cause problems in newer
   kernels. 


2/ yoga_laptop/ideapad-laptop.c

   OBSOLETE, use with caution

   Changes to this file should instead be made to the ideapad-laptop kernel
   module in the Linux kernel.

   NOT NEEDED in Fedora 20 or Fedora 19
   Probably not needed in 3.16 or newer kernels

   A patch to the ideapad-laptop kernel module to make Wifi work on the Yoga
   2 Pro.  The ideapad-laptop moodule also handles several ACPI-related keys
   on the Yoga keyboard.  A similar patch will be in 3.16, but one that
   works for the Yoga Ideapad 1 and Yoga 2 11/13/Pro.  This better patch has
   been backported to Fedora 19 and Fedora 20 as of the end of June 2014.

   To compile and install (warning - this installs a kernel module and may
   break your system, and has to be done each time you install a new kernel,
   after booting into the new kernel):
	```bash
	make ideapad-laptop
	```
	```bash
	sudo make ideapad-laptop-install
	```
	
   The installation also sets up udev rules and an xmodmap file to handle
   the touchpad_toggle and break keys that are not usually handled correctly.

   Reboot your system to load the module


2/ sensor drivers

   OBSOLETE, use with caution

   Changes to these files should instead be made to the appropriate Linux
   kernel modules

   NOT NEEDED in 3.15 kernels or newer or Fedora 19 or newer 

   Patched 3.14 drivers for five of the sensors in the Yoga laptops.  The
   patch adds a quirk so that the hub initializes correctly.  The quirk has
   been added to 3.15 and 3.15 should have better drivers than the ones
   here, so don't use these drivers with a 3.15 or newer kernel.  The 3.15
   drivers also have more quirks, perhaps even the right quirk for the
   Thinkpad Yoga.

   To compile and install (warning - this installs several kernel modules and
   may break your system, and has to be done each time you install a new kernel,
   after booting into the new kernel):
	```bash
	make sensors-drivers
	```
	```bash
	sudo make sensors-drivers-install
	```
   Reboot your system to load the drivers

3/ orientation and light programs

   The orientation program re-orients the screen so that the top of the
   screen is physically up.  The program needs the hid-sensor-accel-3d
   sensor driver.  See "docs/Orientation and rotation" for more information.

   The light program adjusts the brightness of the screen in response to
   changes in ambient brightness.  The program needs the hid-sensor-als
   sensor driver.

   There is also a generic program that can be used to test sensors -
   generic_buffer. 

   You need to have the libXrandr-devel package installed.
   ```bash
   sudo apt-get install libxrandr-dev
   ```

   To compile and install (after first compiling and installing the sensor
   drivers if necessary): 
	```bash
	make programs
	```
	```bash
	sudo make programs-install
	```
