/*
 * File:   common.h
 * Author: buri
 *
 * Created on 2. květen 2014, 11:36
 */

#ifndef COMMON_H
#define	COMMON_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <linux/types.h>
#include <string.h>
#include <poll.h>
#include <endian.h>
#include <getopt.h>
#include <inttypes.h>
#include <syslog.h> /*pfps*/
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <X11/extensions/Xrandr.h>


#include "iio_utils.h"
#include "print_utils.h"
#include "sensors.h"

#ifdef	__cplusplus
}
#endif

#endif	/* COMMON_H */

