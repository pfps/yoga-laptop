/*
 * File:   config.h
 * Author: buri
 *
 * Created on 2. květen 2014, 12:59
 */

#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#define DEBUG_ALL -1
#define DEBUG_TRACE 0
#define DEBUG_INFO 1
#define DEBUG_ERROR 2
#define DEBUG_CRITICAL 3

	struct Config_s {
		/* Common config */
		// Output level
		int debug_level;
		// Time to waite between polls
		unsigned int poll_timeout;
		// Times to run the script
		int iterations;
		// Device name
		const char* device_name;

		/* Orientation config*/
		const char *or_touchScreenName;
		const char *or_wacomPenName;

		/* Light config */
		// Top value used for scaling
		unsigned int light_ambient_max;
		// Max backlight value
		unsigned int light_backlight_max;
	} Config_default = {
		DEBUG_ERROR,
		1000000,	// 1 second
		-1,			// infinite iterations
		"",

		// Orientation
		"ELAN Touchscreen",
		"Wacom ISDv4 EC Pen stylus",

		// Light
		1400,
		937
	};

	typedef struct Config_s Config;


#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

