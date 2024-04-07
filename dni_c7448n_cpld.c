/*
 * CPLD glue driver for dni-c7448n as described by a flattened OF device tree
 *
 * Copyright (C) 2014 Cumulus Networks, Inc.
 * Author: Puneet Shenoy <puneet@cumulusnetworks.com>
 *         Alan Liebthal <alanl@cumulusnetworks.com>
 *
 * Code copied generously from the dni-7448 CPLD driver written by:
 * Copyright (C) 2011 Cumulus Networks, LLC
 * Author: JR Rivers <jrrivers@cumulusnetworks.com>

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stddef.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>

#include <asm/io.h>

#include "platform_defs.h"
#include "dni_c7448n_cpld.h"

static const char driver_name[] = "dni_c7448n_cpld";
#define DRIVER_VERSION "1.0"

/*------------------------------------------------------------------------------
 *
 * Driver resident static variables
 *
 */

static DEFINE_MUTEX(dni_c7448n_cpld_mutex);

static uint8_t cpld_rd(uint8_t *reg)
{
	return readb(reg);
}

static void cpld_wr(uint8_t *reg, uint8_t data)
{
	writeb(data, reg);
}

/*
 * board version
 */
static ssize_t board_revision_show(struct device * dev,
                                   struct device_attribute * dattr,
                                   char * buf)
{
	uint8_t tmp;
	uint8_t type;
	uint8_t rev;
	uint8_t crev;
	uint8_t * regs;

	regs = dev_get_drvdata(dev);

	tmp = cpld_rd(regs + DNIC7448N_CPLD_BOARD_REV_OFFSET);
	type = (tmp & DNIC7448N_CPLD_BOARD_TYPE_MASK) >> DNIC7448N_CPLD_BOARD_TYPE_SHIFT;
	rev  = (tmp & DNIC7448N_CPLD_BOARD_REV_MASK)  >> DNIC7448N_CPLD_BOARD_REV_SHIFT;

	crev = cpld_rd(regs + DNIC7448N_CPLD_REV_OFFSET);

	return sprintf(buf, "%d.%d:%d\n", type, rev, crev);
}
static SYSFS_ATTR_RO(board_revision, board_revision_show);

/*
 * bulk power supply status
 */
static ssize_t bulk_power_show(struct device * dev,
                               struct device_attribute * dattr,
                               char * buf)
{
	uint8_t tmp;
	uint8_t mask;
	uint8_t present_l;
	uint8_t pwr_ok_l;
	uint8_t alert_l;
	uint8_t bad = 0;
	uint8_t * regs;

	regs = dev_get_drvdata(dev);
	tmp = cpld_rd(regs + DNIC7448N_CPLD_BULK_PS_OFFSET);

	if (strcmp(dattr->attr.name, xstr(PLATFORM_PS_NAME_1)) == 0) {
		mask      = DNIC7448N_CPLD_BULK_PS_0_MASK;
		present_l = DNIC7448N_CPLD_BULK_PS_0_PRESENT_L;
		pwr_ok_l  = DNIC7448N_CPLD_BULK_PS_0_OK_L;
		alert_l   = DNIC7448N_CPLD_BULK_PS_0_ALERT_L;
	} else {
		mask      = DNIC7448N_CPLD_BULK_PS_1_MASK;
		present_l = DNIC7448N_CPLD_BULK_PS_1_PRESENT_L;
		pwr_ok_l  = DNIC7448N_CPLD_BULK_PS_1_OK_L;
		alert_l   = DNIC7448N_CPLD_BULK_PS_1_ALERT_L;
	}

	tmp &= mask;
	if ( ~tmp & present_l) {
		sprintf(buf, PLATFORM_INSTALLED);
		if (!(~tmp & pwr_ok_l)) {
			bad++;
			strcat( buf, ", "PLATFORM_PS_POWER_BAD);
		}
		else if (~tmp & alert_l) {
			bad++;
			strcat( buf, ", "PLATFORM_PS_POWER_BAD);
		}
		else if ( bad == 0) {
			strcat( buf, ", "PLATFORM_OK);
		}
	}
	else {
		// Not present
		sprintf(buf, PLATFORM_NOT_INSTALLED);
	}
	strcat( buf, "\n");

	return strlen(buf);
}
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_0, bulk_power_show);
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_1, bulk_power_show);


/*
 * board power status (kind of silly... nothing would work if this was bad)
 */
static ssize_t board_power_show(struct device * dev,
                                struct device_attribute * dattr,
                                char * buf)
{
	uint8_t tmp;
	uint8_t * regs;

	regs = dev_get_drvdata(dev);
	tmp = cpld_rd(regs + DNIC7448N_CPLD_BOARD_PWR_STATUS_OFFSET);

	if (tmp == (( DNIC7448N_CPLD_VCC_3V3_GOOD
                | DNIC7448N_CPLD_NET_SOC_1V_GOOD
                | DNIC7448N_CPLD_DDR_1V5_GOOD
                | DNIC7448N_CPLD_PHY_1V_1to24_GOOD
                | DNIC7448N_CPLD_PHY_1V_25to52_GOOD)
                & ~DNIC7448N_CPLD_CPU_1V_GOOD_L
                & ~DNIC7448N_CPLD_PHY_1V5_GOOD_L)) {
		return sprintf(buf, PLATFORM_OK "\n");
	}
	return sprintf(buf, "bad - 0x%02x\n", tmp);
}
static SYSFS_ATTR_RO(board_power, board_power_show);

/*
 * fan status
 */
static ssize_t fan_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	uint8_t tmp;
	uint8_t mask;
	uint8_t present_l;
	uint8_t fan_0_ok;
	uint8_t fan_1_ok;
	uint8_t * regs;

	regs = dev_get_drvdata(dev);
	tmp = cpld_rd(regs + DNIC7448N_CPLD_FAN_STATUS_OFFSET);

	if (strcmp(dattr->attr.name, "fan_0") == 0) {
		mask      = DNIC7448N_CPLD_FAN_0_MASK;
		present_l = DNIC7448N_CPLD_FAN_0_PRESENT_L;
		fan_0_ok  = DNIC7448N_CPLD_FAN_0_A_OK;
		fan_1_ok  = DNIC7448N_CPLD_FAN_0_B_OK;
	} else {
		mask      = DNIC7448N_CPLD_FAN_1_MASK;
		present_l = DNIC7448N_CPLD_FAN_1_PRESENT_L;
		fan_0_ok  = DNIC7448N_CPLD_FAN_1_A_OK;
		fan_1_ok  = DNIC7448N_CPLD_FAN_1_B_OK;
	}

	tmp &= mask;
	if (tmp & present_l) {
		return sprintf(buf, PLATFORM_NOT_INSTALLED "\n");
	} else if ((!(tmp & fan_0_ok)) |
                     (!(tmp & fan_1_ok))) {
		return sprintf(buf, PLATFORM_FAN_NOT_SPINNING "\n");
	} else {
		return sprintf(buf, PLATFORM_OK "\n");
	}
}
static SYSFS_ATTR_RO(fan_0, fan_show);
static SYSFS_ATTR_RO(fan_1, fan_show);

/*------------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[DNIC7448N_CPLD_STRING_NAME_SIZE];
	unsigned int offset;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_temp",
		.offset = DNIC7448N_CPLD_SYSTEM_LED_OFFSET,
		.mask = DNIC7448N_CPLD_TEMP_LED,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_RED, DNIC7448N_CPLD_TEMP_LED_RED },
			{ PLATFORM_LED_OFF, DNIC7448N_CPLD_TEMP_LED_OFF },
		},
	},
	{
		.name = "led_master",
		.offset = DNIC7448N_CPLD_SYSTEM_LED_OFFSET,
		.mask = DNIC7448N_CPLD_MASTER_LED,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, DNIC7448N_CPLD_MASTER_LED_GREEN },
			{ PLATFORM_LED_BLUE, DNIC7448N_CPLD_MASTER_LED_BLUE },
			{ PLATFORM_LED_OFF, DNIC7448N_CPLD_MASTER_LED_OFF },
		},
	},
	{
		.name = "led_system",
		.offset = DNIC7448N_CPLD_SYSTEM_LED_OFFSET,
		.mask = DNIC7448N_CPLD_STATUS_LED,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_BLUE, DNIC7448N_CPLD_STATUS_LED_BLUE },
			{ PLATFORM_LED_BLUE_BLINKING, DNIC7448N_CPLD_STATUS_LED_BLUE_BLINK },
			{ PLATFORM_LED_RED, DNIC7448N_CPLD_STATUS_LED_RED },
			{ PLATFORM_LED_RED_BLINKING, DNIC7448N_CPLD_STATUS_LED_RED_BLINK },
		},
	},
	{
		.name = "led_fan_tray_0",
		.offset = DNIC7448N_CPLD_FAN_TRAY_LED_CTRL_OFFSET,
		.mask = DNIC7448N_CPLD_FAN_TRAY_0_LED,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, DNIC7448N_CPLD_FAN_TRAY_0_LED_GREEN },
			{ PLATFORM_LED_RED, DNIC7448N_CPLD_FAN_TRAY_0_LED_RED },
			{ PLATFORM_LED_OFF, DNIC7448N_CPLD_FAN_TRAY_0_LED_OFF },
		},
	},
	{
		.name = "led_fan_tray_1",
		.offset = DNIC7448N_CPLD_FAN_TRAY_LED_CTRL_OFFSET,
		.mask = DNIC7448N_CPLD_FAN_TRAY_1_LED,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, DNIC7448N_CPLD_FAN_TRAY_1_LED_GREEN },
			{ PLATFORM_LED_RED, DNIC7448N_CPLD_FAN_TRAY_1_LED_RED },
			{ PLATFORM_LED_OFF, DNIC7448N_CPLD_FAN_TRAY_1_LED_OFF },
		},
	},
	{
		.name = "led_locate",
		.offset = DNIC7448N_CPLD_SYSTEM_LED_OFFSET,
		.mask = DNIC7448N_CPLD_LOCATE_LED,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_BLUE, DNIC7448N_CPLD_LOCATE_LED_BLUE},
			{ PLATFORM_LED_BLUE_BLINKING, DNIC7448N_CPLD_LOCATE_LED_BLUE_BLINK},
			{ PLATFORM_LED_OFF, DNIC7448N_CPLD_LOCATE_LED_OFF},
		},
	},
	{
		.name = "led_diag",
		.offset = DNIC7448N_CPLD_SYSTEM_LED_OFFSET,
		.mask = DNIC7448N_CPLD_DIAG_LED,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN, DNIC7448N_CPLD_DIAG_LED_GREEN},
			{ PLATFORM_LED_OFF, DNIC7448N_CPLD_DIAG_LED_OFF},
		},
	},
};
static int n_leds = ARRAY_SIZE(cpld_leds);

/*
 * LEDs
 */
static ssize_t led_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	uint8_t tmp;
	int i;
	struct led * target = NULL;
	uint8_t * regs;

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined\n");
	}

	/* read the register */
	regs = dev_get_drvdata(dev);
	tmp = cpld_rd(regs + target->offset);

	/* find the color */
	tmp &= target->mask;
	for (i = 0; i < target->n_colors; i++) {
		if (tmp == target->colors[i].value) {
			break;
		}
	}
	if (i == target->n_colors) {
		return sprintf(buf, "undefined\n");
	} else {
		return sprintf(buf, "%s\n", target->colors[i].name);
	}
}

static ssize_t led_store(struct device * dev,
                         struct device_attribute * dattr,
                         const char * buf, size_t count)
{
	uint8_t tmp;
	int i;
	struct led * target = NULL;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];
	uint8_t * regs;

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL) {
		return -EINVAL;
	}

	/* find the color */
	if (sscanf(buf, "%19s", raw) <= 0) {
		return -EINVAL;
	}
	for (i = 0; i < target->n_colors; i++) {
		if (strcmp(raw, target->colors[i].name) == 0) {
			break;
		}
	}
	if (i == target->n_colors) {
		return -EINVAL;
	}

	/* set the new value */
	mutex_lock(&dni_c7448n_cpld_mutex);

	regs = dev_get_drvdata(dev);
	tmp = cpld_rd(regs + target->offset);
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;

	cpld_wr((regs + target->offset), tmp);
	mutex_unlock(&dni_c7448n_cpld_mutex);

	return count;
}
static SYSFS_ATTR_RW(led_temp, led_show, led_store);
static SYSFS_ATTR_RW(led_master, led_show, led_store);
static SYSFS_ATTR_RW(led_system, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_0, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_1, led_show, led_store);
static SYSFS_ATTR_RW(led_diag, led_show, led_store);
static SYSFS_ATTR_RW(led_locate, led_show, led_store);

/*
 * slot_enable
 *  There is a slot power enable bit in the CPLD that needs to be
 *  turned on in order for ports 49 and 51 to work.
 *
 *   0: slot power is off, ports are disabled
 *   1: slot power is on, ports are enabled
 */
static ssize_t slot_enable_show(struct device * dev,
                                struct device_attribute * dattr,
                                char * buf)
{
	uint8_t * regs;
	uint8_t read_val;
	unsigned int val;

	val = 0;
	regs = dev_get_drvdata(dev);
	read_val = cpld_rd(regs + DNIC7448N_CPLD_MODULE_CTL_OFFSET);
	if ((read_val & DNIC7448N_CPLD_MODULE_SLOT1_PWR_L) == 0)
		val = 1;

	return sprintf(buf, "%u\n", val);
}

static ssize_t slot_enable_store(struct device * dev,
                                 struct device_attribute * dattr,
                                 const char * buf, size_t count)
{
	uint8_t * regs;
	uint8_t reg_val;
	unsigned int val;

	if ( sscanf(buf, "%u", &val) != 1 || val > 1)
		return -EINVAL;

	regs = dev_get_drvdata(dev);
	reg_val = cpld_rd(regs + DNIC7448N_CPLD_MODULE_CTL_OFFSET);
	reg_val &= ~DNIC7448N_CPLD_MODULE_SLOT1_PWR_L;
	if (!val)
		reg_val |= DNIC7448N_CPLD_MODULE_SLOT1_PWR_L;
	cpld_wr(regs + DNIC7448N_CPLD_MODULE_CTL_OFFSET, reg_val);

	return count;
}
static SYSFS_ATTR_RW(slot_enable, slot_enable_show, slot_enable_store);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */
static struct attribute *dni_c7448n_cpld_attrs[] = {
	&dev_attr_board_revision.attr,
	&dev_attr_psu_pwr1.attr,
	&dev_attr_psu_pwr2.attr,
	&dev_attr_board_power.attr,
	&dev_attr_fan_0.attr,
	&dev_attr_fan_1.attr,
	&dev_attr_led_temp.attr,
	&dev_attr_led_master.attr,
	&dev_attr_led_system.attr,
	&dev_attr_led_fan_tray_0.attr,
	&dev_attr_led_fan_tray_1.attr,
	&dev_attr_led_diag.attr,
	&dev_attr_led_locate.attr,
	&dev_attr_slot_enable.attr,
	NULL,
};

static struct attribute_group dni_c7448n_cpld_attr_group = {
	.attrs = dni_c7448n_cpld_attrs,
};


/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static int dni_c7448n_cpld_setup(uint8_t* regs)
{
	return 0;
}

static int dni_c7448n_cpld_probe(struct platform_device * ofdev)
{
	int retval = 0;
	struct device_node * np = ofdev->dev.of_node;
	struct kobject * kobj = &ofdev->dev.kobj;
	uint8_t * regs;

	if (dev_get_drvdata(&ofdev->dev)) {
		dev_info(&ofdev->dev, "already probed\n");
		regs = dev_get_drvdata(&ofdev->dev);
	} else {
		regs = of_iomap(np,0);
		if (!regs) {
			return -EIO;
		}
	}

	retval = sysfs_create_group(kobj, &dni_c7448n_cpld_attr_group);
	if (retval) {
		return retval;
	}

	if (dni_c7448n_cpld_setup(regs)) {
		return -EIO;
	}

	dev_set_drvdata(&ofdev->dev, regs);
	dev_info(&ofdev->dev, "probed & iomapped @ %p\n", regs);

	return 0;
}

static int dni_c7448n_cpld_remove(struct platform_device * ofdev)
{
	struct kobject * kobj = &ofdev->dev.kobj;

	sysfs_remove_group(kobj, &dni_c7448n_cpld_attr_group);
	dev_info(&ofdev->dev, "removed\n");
	return 0;
}

static struct of_device_id dni_c7448n_cpld_ids[] = {
	{
		.compatible = "dni,c7448n-cpld",
	},
	{ /* end of list */ },
};

static struct platform_driver dni_c7448n_cpld_driver = {
	.probe = dni_c7448n_cpld_probe,
	.remove = dni_c7448n_cpld_remove,
	.driver = {
		.name  = driver_name,
		.owner = THIS_MODULE,
		.of_match_table = dni_c7448n_cpld_ids,
	},
};

/*------------------------------------------------------------------------------
 *
 * module interface
 *
 */
static int __init dni_c7448n_cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&dni_c7448n_cpld_driver);
	if (rv) {
		printk(KERN_ERR
               "%s platform_driver_register failed (%i)\n",
		driver_name, rv);
	}
	return rv;
}

static void __exit dni_c7448n_cpld_exit(void)
{
	return platform_driver_unregister(&dni_c7448n_cpld_driver);
}

MODULE_AUTHOR("Alan Liebthal <alanl@cumulusnetworks.com>");
MODULE_DESCRIPTION("CPLD driver for Delta Networks Inc. C7448N");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(dni_c7448n_cpld_init);
module_exit(dni_c7448n_cpld_exit);
