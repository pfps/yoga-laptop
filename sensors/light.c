/* Rotate Lenovo Yoga (2 Pro) display and touchscreen to match orientation of screen
 * Copyright (c) 2014 Peter F. Patel-Schneider
 *
 * Modified from industrialio buffer test code.
 * Copyright (c) 2008 Jonathan Cameron
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * Command line parameters
 * orientation -n <iio device name> -c <iterations (<0 is forever)> -d <debug level>
 * iio device name defaults to accel_3d
 * iterations defaults to -1
 * debug level defaults to 0 (useful range is -1 to 4)
 *
 * This program reads the Invensense accelerometer and uses gravity to
 * determine which edge of the Yoga screen is up.  If none of the edges is
 * up very much, this is treated as being flat.   When an edge is up twice
 * in a row, the orientation of the screen and touchscreen is adjusted
 * to match.
 *
 * WARNING:  This is not production quality code.
 *	     This code exec's xrandr and xinput as root.
 */

#define _GNU_SOURCE

#include "libs/common.h"
#include "libs/config.h"
#include "libs/device_info.h"

void process_scan(char *data,
		struct iio_channel_info *channels,
		int num_channels, Config config);
int prepare_output(int dev_num, char * dev_dir_name, char * trigger_name,
		struct iio_channel_info *channels, int num_channels,
		void (*callback)(char*, struct iio_channel_info*, int, Config), Config) ;

static char *dev_dir_name;

void sigint_callback_handler(int signum) {
	if (dev_dir_name) {
		/* Disconnect the trigger - just write a dummy name. */
		write_sysfs_string("trigger/current_trigger", dev_dir_name, "NULL");
		/* Stop the buffer */
		write_sysfs_int("buffer/enable", dev_dir_name, 0);
	}
	exit(signum);
}

int main(int argc, char **argv) {
	/* Configuration variables */
	char *trigger_name = NULL, *config_file = "conf/light.ini";
	Config config = Config_default;
	GKeyFile* gfile;
	GError* gerror;
	static int version_flag = 0, help_flag = 0;

	// Update default settings
	config.device_name = "als";

	/* Arguments definition */
	static char* version = "light version 0.3\n";
	static char* help;
	asprintf(&help, "light monitors ambient light sensor and adjusts backlight acordingly\
	\n\
	Options:\n\
	  --help					Print this help message and exit\n\
	  --version					Print version information and exit\n\
	  --count=<iterations>		If >0 run for only this number of iterations [%d]\n\
	  --name=<device>			Industrial IO accelerometer device name [%s]\n\
	  --usleep=<microtime>		Polling sleep time in microseconds [%u]\n\
	  --debug=<level>			Print out debugging information (-1 through 4) [%d]\n\
	  --ambient-max=<integer>	Minimum ambient light required for full backlight [%u]\n\
	  --backlight-max=<integer>	Max output defined /sys/class/backlight/intel_backlight/max_brightness [%u]\n",
			config.iterations, (char*)config.device_name, config.poll_timeout, config.debug_level,
			config.light_ambient_max, config.light_backlight_max);

	/* Device info */
	Device_info info;

	/* Other variables */
	int ret, c, i;
	char * dummy;


	/*if(g_key_file_load_from_file(gfile, config_file, G_KEY_FILE_NONE, &gerror) != TRUE) {
		fprintf(stderr, "Config file not found, loading default values (%s)...\n", gerror->message);
	} else {
		config.device_name = g_key_file_get_string(gfile, "Device", "Name", &gerror);
	}*/

	static struct option long_options[] = {
		{"version", no_argument, &version_flag, 1},
		{"help", no_argument, &help_flag, 1},
		{"count", required_argument, 0, 'c'},
		{"name", required_argument, 0, 'n'},
		{"touchscreen", required_argument, 0, 't'},
		{"usleep", required_argument, 0, 'u'},
		{"debug", required_argument, 0, 'd'},
		{"ambient-max", required_argument, 0, 'a'},
		{"backlight-max", required_argument, 0, 'b'},
		{0, 0, 0, 0}
	};
	int option_index = 0;

	while ((c = getopt_long(argc, argv, "c:n:d:t:u:", long_options, &option_index))
			!= -1) {
		switch (c) {
			case 0:
				break;
			case 'c':
				config.iterations = strtol(optarg, &dummy, 10);
				break;
			case 'n':
				config.device_name = optarg;
				break;
			case 'd':
				config.debug_level = strtol(optarg, &dummy, 10);
				break;
			case 'u':
				config.poll_timeout = strtol(optarg, &dummy, 10);
				break;
			case 'a':
				config.light_ambient_max = strtol(optarg, &dummy, 10);
				break;
			case 'b':
				config.light_backlight_max = strtol(optarg, &dummy, 10);
				break;
			case '?':
				printf("Invalid flag, use --help for aditional info\n");
				return -1;
		}
	}

	if (version_flag) {
		printf("%s", version);
		exit(0);
	}
	if (help_flag) {
		printf("%s", help);
		exit(0);
	}

	signal(SIGINT, sigint_callback_handler);
	signal(SIGHUP, sigint_callback_handler);

	/* Find the device requested */
	info.device_id = find_type_by_name(config.device_name, "iio:device");
	if (info.device_id < 0) {
		printf("Failed to find the %s sensor\n", config.device_name);
		ret = -ENODEV;
		goto error_ret;
	}
	if (config.debug_level > DEBUG_ALL) printf("iio device number being used is %d\n", info.device_id);

	/* enable the sensors in the device */
	asprintf(&dev_dir_name, "%siio:device%d", iio_dir, info.device_id);
	if (ret < 0) {
		ret = -ENOMEM;
		goto error_ret;
	}

	enable_sensors(dev_dir_name);

	if (trigger_name == NULL) {
		/* Build the trigger name. */
		ret = asprintf(&trigger_name,
				"%s-dev%d", config.device_name, info.device_id);
		if (ret < 0) {
			ret = -ENOMEM;
			goto error_ret;
		}
	}
	/* Verify the trigger exists */
	ret = find_type_by_name(trigger_name, "trigger");
	if (ret < 0) {
		printf("Failed to find the trigger %s\n", trigger_name);
		ret = -ENODEV;
		goto error_free_triggername;
	}
	if (config.debug_level > DEBUG_ALL) printf("iio trigger number being used is %d\n", ret);

	/* Parse the files in scan_elements to identify what channels are present */
	ret = build_channel_array(dev_dir_name, &(info.channels), &(info.channels_count));
	if (ret) {
		printf("Problem reading scan element information\n");
		printf("diag %s\n", dev_dir_name);
		goto error_free_triggername;
	}

	for (i = 0; i != config.iterations; i++) {
		prepare_output(info.device_id, dev_dir_name, trigger_name, info.channels, info.channels_count, &process_scan, config);
		usleep(config.poll_timeout);
	}

	return 0;

error_free_triggername:
	free(trigger_name);
error_free_dev_dir_name:
	free(dev_dir_name);
error_ret:
	return ret;
}

/**
 * process_scan() - print out the values in SI units
 * @data:               pointer to the start of the scan
 * @channels:           information about the channels. Note
 *  size_from_channelarray must have been called first to fill the
 *  location offsets.
 * @num_channels:       number of channels
 **/
void process_scan(char *data,
		struct iio_channel_info *channels,
		int num_channels, Config config) {
	int k;
	for (k = 0; k < num_channels; k++) {
		/*pfps printf("PROCESS SCAN channel %d bytes %d location %d signed %d data %x %d\n",k,
				channels[k].bytes,channels[k].location,
				channels[k].is_signed,(data+channels[k].location),*(data+channels[k].location));*/
		switch (channels[k].bytes) {
				/* only a few cases implemented so far */
			case 2:
				print2byte(*(uint16_t *) (data + channels[k].location),
						&channels[k]);
				break;
			case 4:
				if (!channels[k].is_signed) {
					uint32_t val = *(uint32_t *)
							(data + channels[k].location);
					printf("SCALED %05f ", ((float) val +
							channels[k].offset) *
							channels[k].scale);
				} else {
					int32_t val = *(int32_t *) (data + channels[k].location);
					/*pfps printf("VAL RAW %d %8x  ",channels[k].location,val); */
					val = val >> channels[k].shift;
					/*pfps printf("SHIFT %d %8x  ",channels[k].shift,val); */
					if (channels[k].bits_used < 32) val &= ((uint32_t) 1 << channels[k].bits_used) - 1;
					/*pfps printf("MASK %d %8x  ",channels[k].bits_used,val); */
					val = (int32_t) (val << (32 - channels[k].bits_used)) >> (32 - channels[k].bits_used);
					/*pfps printf("FIX %x\n",val); */
					/*printf("%s %4d %6.1f  ", channels[k].name,
							val, ((float) val + channels[k].offset) * channels[k].scale);*/
					int backlight = limit_interval(1, config.light_ambient_max, val * config.light_backlight_max / config.light_ambient_max);
					printf("Current backlight level: %d\n", backlight);
					FILE* fp = fopen("/sys/class/backlight/intel_backlight/brightness", "w");
					fprintf(fp, "%d", backlight);
					fclose(fp);
				}
				break;
			case 8:
				if (channels[k].is_signed) {
					int64_t val = *(int64_t *)
							(data +
							channels[k].location);
					if ((val >> channels[k].bits_used) & 1)
						val = (val & channels[k].mask) |
						~channels[k].mask;
					/* special case for timestamp */
					if (channels[k].scale == 1.0f &&
							channels[k].offset == 0.0f)
						printf("TIME %" PRId64 " ", val);
					else
						printf("SCALED %05f ", ((float) val +
							channels[k].offset) *
							channels[k].scale);
				}
				break;
			default:
				break;
		}
	}
	printf("\n");
}
int prepare_output(int dev_num, char * dev_dir_name, char * trigger_name,
		struct iio_channel_info *channels, int num_channels,
		void (*callback)(char*, struct iio_channel_info*, int, Config), Config config) {
	char * buffer_access;
	int ret, scan_size;

	int fp, buf_len = 127;
	int i;
	char * data, * inverted;
	ssize_t read_size;

	/* Set the device trigger to be the data ready trigger */
	ret = write_sysfs_string_and_verify("trigger/current_trigger",
			dev_dir_name, trigger_name);
	if (ret < 0) {
		printf("Failed to write current_trigger file %s\n", strerror(-ret));
		goto error_ret;
	}

	/*	Setup ring buffer parameters */
	ret = write_sysfs_int("buffer/length", dev_dir_name, 128);
	if (ret < 0) goto error_ret;
	/* Enable the buffer */
	ret = write_sysfs_int_and_verify("buffer/enable", dev_dir_name, 1);
	if (ret < 0) {
		printf("Unable to enable the buffer %d\n", ret);
		goto error_ret;
	}
	scan_size = size_from_channelarray(channels, num_channels);
	data = malloc(scan_size * buf_len);
	if (!data) {
		ret = -ENOMEM;
		goto error_ret;
	}

	ret = asprintf(&buffer_access, "/dev/iio:device%d", dev_num);
	if (ret < 0) {
		ret = -ENOMEM;
		goto error_free_data;
	}
	/* Attempt to open non blocking to access dev */
	fp = open(buffer_access, O_RDONLY | O_NONBLOCK);
	/*  printf("OPEN file %s\n",buffer_access); */
	if (fp == -1) { /* If it isn't there make the node */
		printf("Failed to open %s : %s\n", buffer_access, strerror(errno));
		ret = -errno;
		goto error_free_buffer_access;
	}

	/* Actually read the data */
	/*  printf("Readin g from %s\n",buffer_access); */
	struct pollfd pfd = {.fd = fp, .events = POLLIN,};
	if (config.debug_level > 3) printf("Polling the data\n");
	poll(&pfd, 1, -1);
	if (config.debug_level > 3) printf("Reading the data\n");
	read_size = read(fp, data, buf_len * scan_size);
	if (config.debug_level > 3) printf("Read the data\n");
	if (read_size == -EAGAIN) {
		printf("nothing available\n");
	} else {
		callback(data, channels, num_channels, config);
	}

	/* Stop the buffer */
	ret = write_sysfs_int("buffer/enable", dev_dir_name, 0);
	if (ret < 0)
		goto error_close_buffer_access;

	/* Disconnect the trigger - just write a dummy name. */
	write_sysfs_string("trigger/current_trigger", dev_dir_name, "NULL");

error_close_buffer_access:
	close(fp);
	/*	printf("CLOSE fp %s\n",buffer_access); */
error_free_buffer_access:
	free(buffer_access);
error_free_data:
	free(data);
error_ret:
	return ret;
}
