/*
 *  yoga-laptop.c - Lenovo Yoga ACPI Extras
 *
 *  Modified from idead-laptop.c
 *  Not a finished product
 *
 *  Copyright © 2010 Intel Corporation
 *  Copyright © 2010 David Woodhouse <dwmw2@infradead.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <acpi/acpi_bus.h>
#include <acpi/acpi_drivers.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/input/sparse-keymap.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/i8042.h>

#define CFG_BT_BIT      (16)
#define CFG_3G_BIT      (17)
#define CFG_WIFI_BIT    (18)
#define CFG_CAMERA_BIT  (19)

enum {
        VPCCMD_R_VPC1 = 0x10,
        VPCCMD_R_BL_MAX,	/* not on Yoga ? */
        VPCCMD_R_BL,		/* not on Yoga ? */
        VPCCMD_W_BL,		/* not on Yoga ? */
        VPCCMD_R_WIFI,		/* not on Yoga ? */
        VPCCMD_W_WIFI,		/* not on Yoga ? */
        VPCCMD_R_BT,		/* not on Yoga ? */
        VPCCMD_W_BT,		/* not on Yoga ? */
        VPCCMD_R_BL_POWER,	/* not on Yoga ? */
        VPCCMD_R_NOVO,
        VPCCMD_R_VPC2,
        VPCCMD_R_TOUCHPAD,
        VPCCMD_W_TOUCHPAD,
        VPCCMD_R_CAMERA,
        VPCCMD_W_CAMERA,
        VPCCMD_R_3G,		/* not on Yoga ? */
        VPCCMD_W_3G,		/* not on Yoga ? */
        VPCCMD_R_ODD, /* 0x21 */
        VPCCMD_W_FAN,
        VPCCMD_R_RF,		/* not on Yoga ? */
        VPCCMD_W_RF,		/* not on Yoga ? */
        VPCCMD_R_FAN = 0x2B,
        VPCCMD_R_SPECIAL_BUTTONS = 0x31,
        VPCCMD_W_BL_POWER = 0x33,
};

struct ideapad_private {
        struct platform_device *platform_device;
        struct input_dev *inputdev;
        struct backlight_device *blightdev;
        struct dentry *debug;
        unsigned long cfg;
};

static acpi_handle ideapad_handle;
static struct ideapad_private *ideapad_priv;

/*
 * ACPI Helpers
 */
#define IDEAPAD_EC_TIMEOUT (100) /* in ms */

static int read_method_int(acpi_handle handle, const char *method, int *val)
{
        acpi_status status;
        unsigned long long result;

        status = acpi_evaluate_integer(handle, (char *)method, NULL, &result);
        if (ACPI_FAILURE(status)) {
                *val = -1;
                return -1;
        } else {
                *val = result;
                return 0;
        }
}

static int method_vpcr(acpi_handle handle, int cmd, int *ret)
{
        acpi_status status;
        unsigned long long result;
        struct acpi_object_list params;
        union acpi_object in_obj;

        params.count = 1;
        params.pointer = &in_obj;
        in_obj.type = ACPI_TYPE_INTEGER;
        in_obj.integer.value = cmd;

        status = acpi_evaluate_integer(handle, "VPCR", &params, &result);

        if (ACPI_FAILURE(status)) {
                *ret = -1;
                return -1;
        } else {
                *ret = result;
                return 0;
        }
}

static int method_vpcw(acpi_handle handle, int cmd, int data)
{
        struct acpi_object_list params;
        union acpi_object in_obj[2];
        acpi_status status;

        params.count = 2;
        params.pointer = in_obj;
        in_obj[0].type = ACPI_TYPE_INTEGER;
        in_obj[0].integer.value = cmd;
        in_obj[1].type = ACPI_TYPE_INTEGER;
        in_obj[1].integer.value = data;

        status = acpi_evaluate_object(handle, "VPCW", &params, NULL);
        if (status != AE_OK)
                return -1;
        return 0;
}

static int read_ec_data(acpi_handle handle, int cmd, unsigned long *data)
{
        int val;
        unsigned long int end_jiffies;

        if (method_vpcw(handle, 1, cmd))
                return -1;

        for (end_jiffies = jiffies+(HZ)*IDEAPAD_EC_TIMEOUT/1000+1;
             time_before(jiffies, end_jiffies);) {
                schedule();
                if (method_vpcr(handle, 1, &val))
                        return -1;
                if (val == 0) {
                        if (method_vpcr(handle, 0, &val))
                                return -1;
                        *data = val;
                        return 0;
                }
        }
        pr_err("timeout in read_ec_cmd\n");
        return -1;
}

static int write_ec_cmd(acpi_handle handle, int cmd, unsigned long data)
{
        int val;
        unsigned long int end_jiffies;

        if (method_vpcw(handle, 0, data))
                return -1;
        if (method_vpcw(handle, 1, cmd))
                return -1;

        for (end_jiffies = jiffies+(HZ)*IDEAPAD_EC_TIMEOUT/1000+1;
             time_before(jiffies, end_jiffies);) {
                schedule();
                if (method_vpcr(handle, 1, &val))
                        return -1;
                if (val == 0)
                        return 0;
        }
        pr_err("timeout in write_ec_cmd\n");
        return -1;
}


/*
 * module init/exit
 */
static const struct acpi_device_id ideapad_device_ids[] = {
  {"ACPI0003", 0},
  {"ELAN1001", 0},
  {"INT0800", 0},
  {"INT33A1", 0},
  {"INT33C0", 0},
  {"INT33C2", 0},
  {"INT33C3", 0},
  {"INT33C5", 0},
  {"INT33C6", 0},
  {"INT33C7", 0},
  {"INT33D0", 0},
  {"INT33D2", 0},
  {"INT33D3", 0},
  {"INT33D4", 0},
  {"INT3F0D", 0},
  {"INTL9C60", 0},
  { "VPC2004", 0},  /* what should this be changed to */
  {"PNP0000", 0},
  {"PNP0100", 0},
  {"PNP0103", 0},
  {"PNP0200", 0},
  /*  {"PNP0A08", 0}, */
  {"PNP0B00", 0},
  {"PNP0C02", 0},
  {"PNP0C02", 0},
  {"PNP0C02", 0},
  {"PNP0C02", 0},
  {"PNP0C04", 0},
  {"PNP0C0A", 0},
  {"PNP0C0C", 0},
  {"PNP0C0D", 0},
  /*  {"PNP0C0F", 0}, */
  {"SYN2B2C", 0},
  { "", 0},
};
MODULE_DEVICE_TABLE(acpi, ideapad_device_ids);


static int ideapad_acpi_add(struct acpi_device *adevice)
{
        int cfg;
        struct ideapad_private *priv;

        if (read_method_int(adevice->handle, "_CFG", &cfg))
                return -ENODEV;

        priv = kzalloc(sizeof(*priv), GFP_KERNEL);
        if (!priv)
                return -ENOMEM;
        dev_set_drvdata(&adevice->dev, priv);
        ideapad_priv = priv;
        ideapad_handle = adevice->handle;
        priv->cfg = cfg;


        return 0;

}

static int ideapad_acpi_remove(struct acpi_device *adevice)
{
        struct ideapad_private *priv = dev_get_drvdata(&adevice->dev);

        dev_set_drvdata(&adevice->dev, NULL);
        kfree(priv);

        return 0;
}



static void ideapad_acpi_notify(struct acpi_device *adevice, u32 event)
{
        struct ideapad_private *priv = dev_get_drvdata(&adevice->dev);
        acpi_handle handle = adevice->handle;
        unsigned long vpc1, vpc2;

        if (read_ec_data(handle, VPCCMD_R_VPC1, &vpc1))
                return;
        if (read_ec_data(handle, VPCCMD_R_VPC2, &vpc2))
                return;

        vpc1 = (vpc2 << 8) | vpc1;


        pr_err("Yoga ACPI notify event: %s %x %lx\n", acpi_device_hid(adevice), event, vpc1);

}

static int ideapad_acpi_resume(struct device *device)
{
        return 0;
}

static SIMPLE_DEV_PM_OPS(ideapad_pm, NULL, ideapad_acpi_resume);

static struct acpi_driver yoga_acpi_driver = {
        .name = "yoga_acpi",
        .class = "Yoga",
        .ids = ideapad_device_ids,
        .ops.add = ideapad_acpi_add,
        .ops.remove = ideapad_acpi_remove,
        .ops.notify = ideapad_acpi_notify,
        .drv.pm = &ideapad_pm,
        .owner = THIS_MODULE,
};
module_acpi_driver(yoga_acpi_driver);

MODULE_AUTHOR("David Woodhouse <dwmw2@infradead.org> (originally)");
MODULE_DESCRIPTION("Yoga ACPI Extras");
MODULE_LICENSE("GPL");
