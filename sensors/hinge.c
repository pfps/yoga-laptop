/* Rotate Lenovo Yoga (2 Pro) display and ELAN Touchscreen to match hinge of screen 
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
 * hinge -n <iio device name> -c <iterations (<0 is forever)> -d <debug level>
 * iio device name defaults to accel_3d
 * iterations defaults to -1
 * debug level defaults to 0 (useful range is -1 to 4)
 *
 *	Determing the Hinge Angle
 *
 * hinge is the beginnings of a system to determine the hinge angle from the
 * two accelerometers.  It is not functional as of yet.
 *
 * WARNING:  This is not production quality code.  
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
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
#include "iio_utils.h"

#include <X11/extensions/Xrandr.h>


static int debug_level = 0;
static char *touchScreenName = "ELAN Touchscreen";

static bool screen_orientation_lock = false;
static int screen_orientation = -1;
static int previous_screen_orientation = -1;

enum { FLAT, TOP, RIGHT, BOTTOM, LEFT };	/* various orientations */

int rotate_left_orientation ( orientation ) {
  return (LEFT == orientation) ? TOP : orientation+1;
}


/**
 * size_from_channelarray() - calculate the storage size of a scan
 * @channels:           the channel info array
 * @num_channels:       number of channels
 *
 * Has the side effect of filling the channels[i].location values used
 * in processing the buffer output.
 **/
int size_from_channelarray(struct iio_channel_info *channels, int num_channels)
{
        int bytes = 0;
        int i = 0;
        while (i < num_channels) {
                if (bytes % channels[i].bytes == 0)
                        channels[i].location = bytes;
                else
                        channels[i].location = bytes - bytes%channels[i].bytes
                                + channels[i].bytes;
                bytes = channels[i].location + channels[i].bytes;
                i++;
        }
        return bytes;
}

/**
 * process_scan_1() - get an integer value for a particular channel
 * @data:               pointer to the start of the scan
 * @channels:           information about the channels. Note
 *  size_from_channelarray must have been called first to fill the
 *  location offsets.
 * @num_channels:       number of channels
 * ch_name:		name of channel to get
 * ch_val:		value for the channel 
 * ch_present:		whether the channel is present
 **/
void process_scan_1(char *data, struct iio_channel_info *channels, int num_channels,
		    char *ch_name, int *ch_val, bool *ch_present)
{
  int k;
  for (k = 0; k < num_channels; k++) {
    if ( 0 == strcmp(channels[k].name,ch_name) ) {
      switch (channels[k].bytes) {
	/* only a few cases implemented so far */
      case 2:
	break;
      case 4:
	if (!channels[k].is_signed) {
	  uint32_t val = *(uint32_t *) (data + channels[k].location);
	  val = val >> channels[k].shift;
	  if ( channels[k].bits_used < 32 ) val &= ((uint32_t)1 << channels[k].bits_used) - 1;
	  *ch_val = (int)val;
	  *ch_present = true;
	} else {
	  int32_t val = *(int32_t *) (data + channels[k].location);
	  val = val >> channels[k].shift;
	  if ( channels[k].bits_used < 32 ) val &= ((uint32_t)1 << channels[k].bits_used) - 1;
	  val = (int32_t)(val << (32 - channels[k].bits_used)) >> (32 - channels[k].bits_used);
	  *ch_val = (int)val;
	  *ch_present = true;
	}
	break;
      case 8:
	break;
      }
    }
  }
}


/**
 * process_scan_3() - get three integer values - see above 
 **/
void process_scan_3(char *data, struct iio_channel_info *channels, int num_channels,
		    char *ch_name_1, int *ch_val_1, bool *ch_present_1,
		    char *ch_name_2, int *ch_val_2, bool *ch_present_2,
		    char *ch_name_3, int *ch_val_3, bool *ch_present_3)
{
  process_scan_1(data,channels,num_channels,ch_name_1,ch_val_1,ch_present_1);
  process_scan_1(data,channels,num_channels,ch_name_2,ch_val_2,ch_present_2);
  process_scan_1(data,channels,num_channels,ch_name_3,ch_val_3,ch_present_3);
}



/**
 * enable_sensors: enable all the sensors in a device
 * @device_dir: the IIO device directory in sysfs
 * @
 **/
static int enable_sensors(const char *device_dir)
{
	DIR *dp;
	FILE *sysfsfp;
	int i;
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
			if ( !ret )
			  write_sysfs_int(ent->d_name,scan_el_dir,1);
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


int get_accel_3d(int dev_num, char * dev_dir_name, char * trigger_name,
	       struct iio_channel_info *channels, int num_channels,
	       int *accel_x, int *accel_y, int *accel_z) {
  char * buffer_access;
  int ret, scan_size;

  int fp, buf_len = 127;
  int i;
  char * data;
  ssize_t read_size;

  bool present_x, present_y, present_z;

  /* Set the device trigger to be the data ready trigger */
  ret = write_sysfs_string_and_verify("trigger/current_trigger",
				      dev_dir_name, trigger_name);
  if (ret < 0) {
    printf("Failed to write current_trigger file %s\n",strerror(-ret));
    goto error_ret;
  }

  /* Setup ring buffer parameters */
  ret = write_sysfs_int("buffer/length", dev_dir_name, 128);
  if (ret < 0) goto error_ret;
  /* Enable the buffer */
  ret = write_sysfs_int_and_verify("buffer/enable", dev_dir_name, 1);
  if (ret < 0) {
    printf("Unable to enable the buffer %d\n",ret);
    goto error_ret;
  }
  scan_size = size_from_channelarray(channels, num_channels);
  data = malloc(scan_size*buf_len);
  if (!data) { ret = -ENOMEM; goto error_ret;
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
    printf("Failed to open %s : %s\n", buffer_access,strerror(errno));
    ret = -errno;
    goto error_free_buffer_access;
  }

  /* Actually read the data */
  /*  printf("Reading from %s\n",buffer_access); */
    struct pollfd pfd = { .fd = fp, .events = POLLIN, };
    if (debug_level > 3) printf("Polling the data\n");
    poll(&pfd, 1, -1);
    if (debug_level > 3) printf("Reading the data\n");
    read_size = read(fp, data, buf_len*scan_size);
    if (debug_level > 3) printf("Read the data\n");
    if (read_size == -EAGAIN) {
      printf("nothing available\n");
    } else
      for (i = 0; i < read_size/scan_size; i++) {
	process_scan_3(data + scan_size*i, channels, num_channels,
		       "in_accel_x",accel_x,&present_x,
		       "in_accel_y",accel_y,&present_y,
		       "in_accel_z",accel_z,&present_z);
      }

  /* Stop the buffer */
  ret = write_sysfs_int("buffer/enable", dev_dir_name, 0);
  if (ret < 0)
    goto error_close_buffer_access;

  /* Disconnect the trigger - just write a dummy name. */
  write_sysfs_string("trigger/current_trigger", dev_dir_name, "NULL");

  ret = 0;

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


int find_orientation(int dev_num, char * dev_dir_name, char * trigger_name,
		     struct iio_channel_info *channels, int num_channels ) {
  int accel_x, accel_y, accel_z;
  int status;
  int orientation;

  status = get_accel_3d(dev_num, dev_dir_name, trigger_name, channels, num_channels,
			&accel_x, &accel_y, &accel_z);
  if ( status ) return status;

  /* Determine orientation */
  int accel_x_abs = abs(accel_x);
  int accel_y_abs = abs(accel_y);
  int accel_z_abs = abs(accel_z);
  if ( accel_z_abs > 4 * accel_x_abs && accel_z_abs > 4 * accel_y_abs ) {
    orientation = FLAT;
  } else if ( 3 * accel_y_abs > 2 * accel_x_abs ) {
    orientation = accel_y > 0 ? BOTTOM : TOP;
  } else orientation = accel_x > 0 ? LEFT : RIGHT;
  if (debug_level > 1) printf("Orientation %d: %5d %5d %5d\n",
				    orientation,accel_x,accel_y,accel_z);
  
  return(orientation);
}


/* symbolic orientation as used in xrandr */
char * symbolic_orientation(orientation) { 
  char * orient;
  switch ( orientation ) {
  case FLAT : orient = "flat"; break;
  case BOTTOM : orient = "inverted"; break;
  case TOP : orient = "normal"; break;
  case LEFT : orient = "left"; break;
  case RIGHT : orient = "right"; break;
  }
  return orient;
}


void rotate_to(orient) {
  char * xrandr = "/usr/bin/xrandr";
  char * xinput = "/usr/bin/xinput";
  char * tsnormal[] = { xinput, "set-prop", touchScreenName, "Coordinate Transformation Matrix",
			"1", "0", "0", "0", "1", "0", "0", "0", "1", (char *)NULL };
  char * tsright[]  = { xinput, "set-prop", touchScreenName, "Coordinate Transformation Matrix",
			"0", "1", "0", "-1", "0", "1", "0", "0", "1", (char *)NULL };
  char * tsleft[]   = { xinput, "set-prop", touchScreenName, "Coordinate Transformation Matrix",
			"0", "-1", "1", "1", "0", "0", "0", "0", "1", (char *)NULL };
  char * tsinverted[] = { xinput, "set-prop", touchScreenName, "Coordinate Transformation Matrix",
			  "-1", "0", "1", "0", "-1", "1", "0", "0", "1", (char *)NULL };
  int status, pid;
  char *orientation = symbolic_orientation(orient);

  printf ("ROTATE to orientation %s\n",orientation);
  screen_orientation = orient;
  if ( 0 == (pid = fork()) ) { /* rotate the screen */
    execl(xrandr,xrandr,"--orientation",orientation,(char *)NULL);
  } else {
    wait(&status);
    if (status) printf("First child (xrandr) returned %d\n",status);
    if ( 0 == (pid = fork()) ) { /* rotate the touchscreen */
      switch ( orient ) {
      case TOP : execv(xinput,tsnormal);
	break;
      case BOTTOM : execv(xinput,tsinverted);
	break;
      case LEFT : execv(xinput,tsleft);
	break;
      case RIGHT : execv(xinput,tsright);
	break;
      }
    } else {
      wait(&status);
      if (status) printf("Second child (xinput) returned %d\n",status);
    }
  }
}



static char *dev_accel_dir_name;
static char *dev_custom_dir_name;


void sigint_callback_handler(int signum) {
  if ( dev_accel_dir_name ) {
    /* Disconnect the trigger - just write a dummy name. */
    write_sysfs_string("trigger/current_trigger", dev_accel_dir_name, "NULL");
    /* Stop the buffer */
    write_sysfs_int("buffer/enable", dev_accel_dir_name, 0);
  }
  if ( dev_custom_dir_name ) {
    /* Disconnect the trigger - just write a dummy name. */
    write_sysfs_string("trigger/current_trigger", dev_custom_dir_name, "NULL");
    /* Stop the buffer */
    write_sysfs_int("buffer/enable", dev_custom_dir_name, 0);
  }
  exit(signum);
}

static time_t last_sigusr_time = 0;

void sigusr_callback_handler(int signum) {
  int now = time(NULL);

  previous_screen_orientation = screen_orientation;
  if ( now <= last_sigusr_time + 1 ) {
    screen_orientation_lock = true;
    printf("Quick second signal suspends rotation at %ld diff %ld\n",
	     (long)now, (long)(now-last_sigusr_time));
    last_sigusr_time = 0;
    rotate_to(rotate_left_orientation(screen_orientation));
  } else {
    screen_orientation_lock = ! screen_orientation_lock;
    if ( debug_level > -1 )
      printf("Signal %s rotation at %ld diff %ld\n",
	     screen_orientation_lock ? "suspends" : "resumes", (long)now, (long)(now-last_sigusr_time));
    last_sigusr_time = now;
  }
}


int setup_device(const char * dev_name, char **dev_dir_name, char **dev_trigger_name,
		 int *dev_num, struct iio_channel_info **dev_channels, int *dev_num_channels) {
  int ret;
  /* Find the device requested */
  *dev_num = find_type_by_name(dev_name, "iio:device");
  if (*dev_num < 0) {
    printf("Failed to find the %s\n", dev_name);
    ret = -ENODEV;
    goto error_ret;
  }
  
  if (debug_level > -1) printf("iio device number being used is %d\n", *dev_num);
  /* enable the sensors */
  asprintf(dev_dir_name, "%siio:device%d", iio_dir, *dev_num);
  if (ret < 0) {
    ret = -ENOMEM;
    goto error_ret;
  }
  enable_sensors(*dev_dir_name);
  /* Build the trigger name. */
  if (*dev_trigger_name == NULL) {
    ret = asprintf(dev_trigger_name, "%s-dev%d", dev_name, *dev_num);
    if (ret < 0) {
      ret = -ENOMEM;
      goto error_ret;
    }
  }
  /* Verify the trigger exists */
  ret = find_type_by_name(*dev_trigger_name, "trigger");
  if (ret < 0) {
    printf("Failed to find the trigger %s\n", *dev_trigger_name);
    ret = -ENODEV;
    goto error_free_triggername;
  }
  if (debug_level > -1) printf("iio trigger number being used is %d\n", ret);
  /* Parse the files in scan_elements to identify what channels are present */
  ret = build_channel_array(*dev_dir_name, dev_channels, dev_num_channels);
  if (ret) {
    printf("Problem reading scan element information\n");
    printf("diag %s\n", *dev_dir_name);
    goto error_free_triggername;
  }
  return(0);
error_free_triggername:
  free(*dev_trigger_name);
error_free_dev_dir_name:
  free(*dev_dir_name);
error_ret:
        return ret;
}


static char* help = "orientation monitors the Yoga accelerometer and\n\
rotates the screen and touchscreen to match\n\
\n\
Options:\n\
  --help		Print this help message and exit\n\
  --version		Print version information and exit\n\
  --count=iterations	If >0 run for only this number of iterations [-1]\n\
  --name=accel_name	Industrial IO accelerometer device name [accel_3d]\n\
  --touchscreen=ts_name	TouchScreen name [ELAN Touchscreen]\n\
  --debug=level		Print out debugging information (-1 through 4) [0]\n\
\n\
orientation responds to single SIGUSR1 interrupts by toggling whether it\n\
rotates the screen and two SIGUSR1 interrupts within a second or two by \n\
rotating the screen clockwise and suspending rotations.\n\
Use via something like\n\
    pkill --signal SIGUSR1 --exact orientation\n";

static char* version = "orientation version 0.2\n";


int main(int argc, char **argv)
{
  char *dev_accel_trigger_name = NULL, *dev_accel_name = "accel_3d";
  int dev_accel_num;
  struct iio_channel_info *dev_accel_channels = NULL;
  int dev_accel_num_channels;
  int screen_accel_x, screen_accel_y, screen_accel_z;

  char *dev_custom_trigger_name = NULL, *dev_custom_name = "custom";
  int dev_custom_num;
  struct iio_channel_info *dev_custom_channels;
  int dev_custom_num_channels;
  int base_accel_x, base_accel_y, base_accel_z;

  int ret, c, i, iterations = -1;
  char * dummy;
  static int version_flag = 0, help_flag = 0;
  int status;

  unsigned int sleeping = 1000000;



  static struct option long_options[] =
  { {"version", no_argument, &version_flag, 1},
    {"help", no_argument, &help_flag, 1},
    {"count", required_argument, 0, 'c'},
    {"name",  required_argument, 0, 'n'},
    {"touchscreen",required_argument, 0, 't'},
    {"debug", required_argument, 0, 'd'},
    {0, 0, 0, 0} };
  int option_index = 0;

  while ( (c = getopt_long(argc, argv, "c:n:d:t:", long_options,&option_index))
	  != -1 ) {
    switch (c) {
    case 0:
      break;
    case 'c':
      iterations = strtol(optarg, &dummy, 10);
      break;
    case 'n':
      dev_accel_name = optarg;
      break;
    case 't':
      touchScreenName = optarg;
      break;
    case 'd':
      debug_level = strtol(optarg, &dummy, 10);
      break;
    case '?':
      printf("Invalid flag\n");
      return -1;
    }
  }

  if ( version_flag ) { printf(version); exit(0); }
  if ( help_flag ) { printf(help); exit(0); }

  signal(SIGINT,sigint_callback_handler);
  signal(SIGHUP,sigint_callback_handler);
  signal(SIGUSR1,sigusr_callback_handler);

  setup_device(dev_accel_name,&dev_accel_dir_name,&dev_accel_trigger_name,
	       &dev_accel_num,&dev_accel_channels,&dev_accel_num_channels);
  printf("Set up %s %s %s %d %d %x\n",dev_accel_name,dev_accel_dir_name,dev_accel_trigger_name,
	 dev_accel_num,dev_accel_num_channels,dev_accel_channels);
  setup_device(dev_custom_name,&dev_custom_dir_name,&dev_custom_trigger_name,
	       &dev_custom_num,&dev_custom_channels,&dev_custom_num_channels);
  printf("Set up %s %s %s %d %d %x\n",dev_custom_name,dev_custom_dir_name,dev_custom_trigger_name,
	 dev_custom_num,dev_custom_num_channels,dev_custom_channels);


  for ( i = 0; i != iterations; i++ ) {

    printf("\n\nReport for %d\n",time(NULL));

    status = get_accel_3d(dev_custom_num, dev_custom_dir_name, dev_custom_trigger_name, dev_custom_channels, dev_custom_num_channels,
			  &base_accel_x, &base_accel_y, &base_accel_z);
    if ( status ) printf("Base Accel Status %d\n",status);
    printf("Base accel %d %d %d\n",base_accel_x, base_accel_y, base_accel_z);

    status = get_accel_3d(dev_accel_num, dev_accel_dir_name, dev_accel_trigger_name, dev_accel_channels, dev_accel_num_channels,
			  &screen_accel_x, &screen_accel_y, &screen_accel_z);
    if ( status ) printf("Screen Accel Status %d\n",status);
    printf("Screen accel %d %d %d\n",screen_accel_x, screen_accel_y, screen_accel_z);

    usleep(sleeping);
  }

  return 0;

}
