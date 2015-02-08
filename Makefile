## -*- Makefile -*-
# Top-level makefile for yoga-laptop project
# Just defers to the lower-level makefiles
#

standard:	programs

programs:
	cd sensors && $(MAKE) all

programs-install:
	cd sensors && $(MAKE) install

sensors-drivers:
	cd sensors/drivers && $(MAKE) default

sensors-drivers-install:
	cd sensors/drivers && $(MAKE) install

ideapad-laptop:
	cd yoga_laptop && $(MAKE) default

ideapad-laptop-install:
	cd yoga_laptop && $(MAKE) install

#install:
#	cd yoga_laptop && $(MAKE) install
#	cd sensors/drivers && $(MAKE) install
#	cd sensors && $(MAKE) install

#### Clean target deletes all generated files ####
clean:

# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state.GNU-amd64-Linux

