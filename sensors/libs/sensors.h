/*
 * File:   print_utils.h
 * Author: buri
 *
 * Created on 2. květen 2014, 11:49
 */

#ifndef SENSORS_H
#define	SENSORS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdbool.h>
#include "iio_utils.h"

/**
 * enable_sensors: enable all the sensors in a device
 * @device_dir: the IIO device directory in sysfs
 * @
 **/
static int enable_sensors(const char *device_dir) {
	DIR *dp;
	FILE *sysfsfp;
	int ret;
	const struct dirent *ent;
	char *scan_el_dir;
	char *filename;

	ret = asprintf(&scan_el_dir, FORMAT_SCAN_ELEMENTS_DIR, device_dir);
	if (ret < 0) {
		ret = -ENOMEM;
		goto error_ret;
	}
	dp = opendir(scan_el_dir);
	if (dp == NULL) {
		ret = -errno;
		goto error_free_name;
	}
	while (ent = readdir(dp), ent != NULL)
		if (strcmp(ent->d_name + strlen(ent->d_name) - strlen("_en"),
				"_en") == 0) {
			ret = asprintf(&filename,
					"%s/%s", scan_el_dir, ent->d_name);
			if (ret < 0) {
				ret = -ENOMEM;
				goto error_close_dir;
			}
			sysfsfp = fopen(filename, "r");
			if (sysfsfp == NULL) {
				ret = -errno;
				free(filename);
				goto error_close_dir;
			}
			fscanf(sysfsfp, "%d", &ret);
			fclose(sysfsfp);
			if (!ret)
				write_sysfs_int(ent->d_name, scan_el_dir, 1);
			free(filename);
		}
	ret = 0;
error_close_dir:
	closedir(dp);
error_free_name:
	free(scan_el_dir);
error_ret:
	return ret;
}

/**
 * size_from_channelarray() - calculate the storage size of a scan
 * @channels:           the channel info array
 * @num_channels:       number of channels
 *
 * Has the side effect of filling the channels[i].location values used
 * in processing the buffer output.
 **/
int size_from_channelarray(struct iio_channel_info *channels, int num_channels) {
	int bytes = 0;
	int i = 0;
	while (i < num_channels) {
		if (bytes % channels[i].bytes == 0)
			channels[i].location = bytes;
		else
			channels[i].location = bytes - bytes % channels[i].bytes
				+ channels[i].bytes;
		bytes = channels[i].location + channels[i].bytes;
		i++;
	}
	return bytes;
}


#ifdef	__cplusplus
}
#endif

#endif	/* SENSORS_H */

