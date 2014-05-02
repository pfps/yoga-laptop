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

#ifdef	__cplusplus
}
#endif

#endif	/* DEVICE_INFO_H */

