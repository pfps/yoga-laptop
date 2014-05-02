/*
 * File:   device_info.h
 * Author: buri
 *
 * Created on 2. květen 2014, 13:48
 */

#ifndef DEVICE_INFO_H
#define	DEVICE_INFO_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "iio_utils.h"

	struct Device_info_s {
		int device_id;
		int channels_count;
		struct iio_channel_info *channels;
	};

	typedef struct Device_info_s Device_info;

	int prepare_output(Device_info* info, char * dev_dir_name, char * trigger_name,
			void (*callback)(char*, Device_info, Config), Config config) {
		char * buffer_access;
		int ret, scan_size, dev_num = info->device_id, num_channels = info->channels_count;
		struct iio_channel_info *channels = info->channels;

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
			callback(data, *info, config);
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


#ifdef	__cplusplus
}
#endif

#endif	/* DEVICE_INFO_H */

