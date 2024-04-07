/*
 * CPLD sysfs driver for cel_seastone.
 *
 * Copyright (C) 2015 Cumulus Networks, Inc.  All rights reserved.
 * Author: David Yen <dhyen@cumulusnetworks.com>
 *
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA                     
 * 02110-1301, USA.                                                                  
 *
 */

#include <stddef.h>

#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <asm/io.h>

#include "cel-seastone-cpld.h"
#include "platform-defs.h"

static const char driver_name[] = "cel_seastone_cpld";
#define DRIVER_VERSION "1.0"

#define CEL_SEASTONE_CPLD_STRING_NAME_SIZE 20

#define CPLD_IO_BASE 0x100
#define CPLD_IO_SIZE 0x2ff

static uint8_t* cel_seastone_cpld_regs;

/********************************************************************************
 *
 * CPLD I/O
 *
 */
static inline uint8_t cpld_rd(uint32_t reg)
{
	return ioread8(cel_seastone_cpld_regs + reg - CPLD_IO_BASE);
}

static inline void cpld_wr(uint32_t reg, uint8_t data)
{
	iowrite8(data, cel_seastone_cpld_regs + reg - CPLD_IO_BASE);
}

/*
 * write protect
 */
static ssize_t write_protect_show(struct device * dev,
				 struct device_attribute * dattr,
				 char * buf)
{
	uint8_t data;

	data = cpld_rd(CPLD1_REG_WRITE_PROTECT_CTL_OFFSET);
	return sprintf(buf, "0x%02X\n", data);
}

static ssize_t write_protect_store(struct device * dev,
				   struct device_attribute * dattr,
				   const char * buf, size_t count)
{
	int tmp;

	if (sscanf(buf, "%x", &tmp) != 1) {
		return -EINVAL;
	}
	cpld_wr(CPLD1_REG_WRITE_PROTECT_CTL_OFFSET, tmp);

	return count;
}
static SYSFS_ATTR_RW(write_protect, write_protect_show, write_protect_store);

/*
 * reset source
 */
static ssize_t reset_source_show(struct device * dev,
				 struct device_attribute * dattr,
				 char * buf)
{
	uint8_t data;

	data = cpld_rd(CPLD1_REG_RESET_SRC_OFFSET);
	return sprintf(buf, "0x%02X\n", data);
}

static SYSFS_ATTR_RO(reset_source, reset_source_show);

/*
 * cpld version
 */
static uint32_t sea_version_regs[] = {
	CPLD1_REG_VERSION_OFFSET,
	CPLD2_REG_VERSION_OFFSET,
	CPLD3_REG_VERSION_OFFSET,
	CPLD4_REG_VERSION_OFFSET,
	CPLD5_REG_VERSION_OFFSET,
};

static ssize_t cpld_version_show(struct device * dev,
                                 struct device_attribute * dattr,
                                 char * buf)
{
	int cpld_idx;
	uint8_t tmp;
	uint8_t type;
	uint8_t rev;

	cpld_idx = dattr->attr.name[4] - '1';
	if (cpld_idx < 0 || cpld_idx > sizeof(sea_version_regs)/sizeof(uint32_t))
		return -EINVAL;

	tmp = cpld_rd(sea_version_regs[cpld_idx]);
	type = (tmp & CPLD_VERSION_H_MASK) >> CPLD_VERSION_H_SHIFT;
	rev  = (tmp & CPLD_VERSION_L_MASK)  >> CPLD_VERSION_L_SHIFT;

	return sprintf(buf, "%d.%d\n", type, rev);
}
static SYSFS_ATTR_RO(cpld1_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld2_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld3_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld4_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld5_version, cpld_version_show);

/*
 * Board type
 */
static ssize_t board_type_show(struct device * dev,
                               struct device_attribute * dattr,
                               char * buf)
{
	char    redstone_name[] = "Redstone-XP";
	char    smallstone_name[] = "Smallstone-XP";
	char    seastone_name[] = "Seastone";
	char   *name;
	uint8_t tmp;

	tmp = cpld_rd(CPLD1_REG_BOARD_TYPE_OFFSET);

	name = redstone_name;
	if (tmp == CPLD1_BOARD_TYPE_SEA_VAL)
		name = seastone_name;
	else if (tmp == CPLD1_BOARD_TYPE_SMALLXP_VAL)
		name = smallstone_name;

	return sprintf(buf, "%s\n", name);
}
static SYSFS_ATTR_RO(board_type, board_type_show);

/*------------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[CEL_SEASTONE_CPLD_STRING_NAME_SIZE];
	uint32_t reg;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_status",
		.reg  = CPLD4_REG_SYS_LED_CTRL_OFFSET,
		.mask = CPLD4_SYS_LED_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD4_SYS_LED_GREEN},
			{ PLATFORM_LED_GREEN_SLOW_BLINKING, CPLD4_SYS_LED_GREEN_SLOW_BLINK},
			{ PLATFORM_LED_GREEN_BLINKING, CPLD4_SYS_LED_GREEN_FAST_BLINK},
			{ PLATFORM_LED_OFF, CPLD4_SYS_LED_OFF},
		},
	},
	{
		.name = "led_psu1",
		.reg = CPLD4_REG_SYS_LED_CTRL_OFFSET,
		.mask = CPLD4_PSU1_LED_L,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN, 0},
			{ PLATFORM_LED_OFF, CPLD4_PSU1_LED_L},
		}
	},
	{
		.name = "led_psu2",
		.reg = CPLD4_REG_SYS_LED_CTRL_OFFSET,
		.mask = CPLD4_PSU2_LED_L,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN, 0},
			{ PLATFORM_LED_OFF, CPLD4_PSU2_LED_L},
		}
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

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined target\n");
	}

	/* read the register */
	tmp = cpld_rd(target->reg);

	/* find the color */
	tmp &= target->mask;
	for (i = 0; i < target->n_colors; i++) {
		if (tmp == target->colors[i].value) {
			break;
		}
	}
	if (i == target->n_colors) {
		return sprintf(buf, "undefined color\n");
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
	tmp = cpld_rd(target->reg);
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;
	cpld_wr(target->reg, tmp);

	return count;
}
static SYSFS_ATTR_RW(led_status, led_show, led_store);
static SYSFS_ATTR_RW(led_psu1, led_show, led_store);
static SYSFS_ATTR_RW(led_psu2, led_show, led_store);


/******************************************************
 *
 * QSFP
 *
 *****************************************************/

#define SEASTONE_NUM_QSFP_REGS (6)

struct seastone_qsfp_info {
	char     name[CEL_SEASTONE_CPLD_STRING_NAME_SIZE];
	int      invert;
	uint32_t regs[SEASTONE_NUM_QSFP_REGS];
};

struct seastone_qsfp_shift_mask {
	int shift;
	uint8_t mask;
};

static struct seastone_qsfp_shift_mask qsfp_shift_mask[] = {
	{ .shift = 0,  .mask = 0xff },
	{ .shift = 8,  .mask = 0x03 },
	{ .shift = 10, .mask = 0xff },
	{ .shift = 18, .mask = 0x07 },
	{ .shift = 21, .mask = 0xff },
	{ .shift = 29, .mask = 0x07 },
};

static struct seastone_qsfp_info seastone_qsfp_list[] =
{
	{
		/*
		 * qsfp_reset
		 *  a bit mask for 32 qsfp ports [0:31] => port1 - port32
		 */
		.name = "qsfp_reset",
		.invert = true,
		.regs = {CPLD2_REG_QSFP_1_8_RESET_L_OFFSET,
			 CPLD2_REG_QSFP_9_10_RESET_L_OFFSET,
			 CPLD3_REG_QSFP_11_18_RESET_L_OFFSET,
			 CPLD3_REG_QSFP_19_21_RESET_L_OFFSET,
			 CPLD5_REG_QSFP_22_29_RESET_L_OFFSET,
			 CPLD5_REG_QSFP_30_32_RESET_L_OFFSET,
	                },
	},
	{
		/*
		 * qsfp_lp_mode
		 *  a bit mask for 32 qsfp ports [0:31] => port1 - port32
		 */
		.name = "qsfp_lp_mode",
		.regs = {CPLD2_REG_QSFP_1_8_LPMOD_OFFSET,
			 CPLD2_REG_QSFP_9_10_LPMOD_OFFSET,
			 CPLD3_REG_QSFP_11_18_LPMOD_OFFSET,
			 CPLD3_REG_QSFP_19_21_LPMOD_OFFSET,
			 CPLD5_REG_QSFP_22_29_LPMOD_OFFSET,
			 CPLD5_REG_QSFP_30_32_LPMOD_OFFSET,
		},
	},
	{
		/*
		 * qsfp_present
		 *  a bit mask for 32 qsfp ports [0:31] => port1 - port32
		 */
		.name = "qsfp_present",
		.invert = true,
		.regs = {CPLD2_REG_QSFP_1_8_ABSENT_OFFSET,
			 CPLD2_REG_QSFP_9_10_ABSENT_OFFSET,
			 CPLD3_REG_QSFP_11_18_ABSENT_OFFSET,
			 CPLD3_REG_QSFP_19_21_ABSENT_OFFSET,
			 CPLD5_REG_QSFP_22_29_ABSENT_OFFSET,
			 CPLD5_REG_QSFP_30_32_ABSENT_OFFSET,
		},
	},
};
static int num_qsfp_attribs = ARRAY_SIZE(seastone_qsfp_list);

int hexToInt(const char *hexStr, uint32_t *valPtr)
{
	char prefix[] = "0x";
	if (strncasecmp(hexStr, prefix, strlen(prefix)) == 0) {
		hexStr += strlen(prefix);
		return sscanf(hexStr, "%x", valPtr) != 1;
	}
	return sscanf(hexStr, "%u", valPtr) != 1;
}

struct seastone_qsfp_info *get_qsfp_attrib(const char *name)
{
	int        index;

	for (index = 0; index < num_qsfp_attribs; index++) {
		if (strncmp(seastone_qsfp_list[index].name, name,
                    strlen(seastone_qsfp_list[index].name)) == 0) {
			return &seastone_qsfp_list[index];
		}
	}
	return NULL;
}

static ssize_t qsfp_attrib_show(struct device *dev,
                                struct device_attribute *dattr,
                                char *buf)
{
	struct seastone_qsfp_info *attrib_ptr;
	uint32_t   data;
	int        index;
	uint8_t    read_val;

	attrib_ptr = get_qsfp_attrib(dattr->attr.name);
	if (!attrib_ptr)
		return -EINVAL;

	data = 0;
	for (index = 0; index < SEASTONE_NUM_QSFP_REGS; index++) {
		read_val = cpld_rd(attrib_ptr->regs[index]) & qsfp_shift_mask[index].mask;
		data |= ((uint32_t)read_val << qsfp_shift_mask[index].shift);
	}
	if (attrib_ptr->invert)
		data = ~data;

	return sprintf(buf, "0x%08X\n", data);
}

static ssize_t qsfp_attrib_store(struct device *dev,
                                 struct device_attribute * dattr,
                                 const char * buf, size_t count)
{
	struct seastone_qsfp_info *attrib_ptr;
	int      index;
	uint32_t val;
	uint8_t  write_val;

	attrib_ptr = get_qsfp_attrib(dattr->attr.name);
	if (!attrib_ptr)
		return -EINVAL;

	if (hexToInt(buf, &val))
		return -EINVAL;

	if (attrib_ptr->invert)
		val = ~val;

	for (index = 0; index < SEASTONE_NUM_QSFP_REGS; index++) {
		write_val = (val >> qsfp_shift_mask[index].shift) & 0xff;
		cpld_wr(attrib_ptr->regs[index], write_val);
	}

	return count;
}
static SYSFS_ATTR_RW(qsfp_reset,    qsfp_attrib_show, qsfp_attrib_store);
static SYSFS_ATTR_RW(qsfp_lp_mode,  qsfp_attrib_show, qsfp_attrib_store);
static SYSFS_ATTR_RW(qsfp_present,  qsfp_attrib_show, NULL);

/*
 * Help / README
 *
 * The string length must be less than 4K.
 *
 */
#define HELP_STR "Description of the CPLD driver files:\n\
\n\
cpld1_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for CPLD 1\n\
  CPLD version register,in the following format:\n\
\n\
  cpld1_version\n\
\n\
  Example: 1.3\n\
\n\
\n\
cpld2_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for CPLD 2\n\
  CPLD version register,in the following format:\n\
\n\
  cpld2_version\n\
\n\
  Example: 2.3\n\
\n\
\n\
cpld3_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for CPLD 3\n\
  CPLD version register,in the following format:\n\
\n\
  cpld3_version\n\
\n\
  Example: 3.3\n\
\n\
\n\
cpld4_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for CPLD 4\n\
  CPLD version register,in the following format:\n\
\n\
  cpld4_version\n\
\n\
  Example: 4.3\n\
\n\
\n\
cpld5_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for CPLD 5\n\
  CPLD version register,in the following format:\n\
\n\
  cpld4_version\n\
\n\
  Example: 5.3\n\
\n\
\n\
led_status\n\
\n\
  Read-Write:\n\
\n\
  System State LED color.\n\
  The following values are possible: green, green blinking,\n\
  green slow blinking and off.\n\
\n\
\n\
led_psu1\n\
\n\
  Read-Write:\n\
\n\
  PSU1 LED color.\n\
  The following values are possible: green, off.\n\
\n\
\n\
led_psu2\n\
\n\
  Read-Write:\n\
\n\
  PSU2 LED color.\n\
  The following values are possible: green, off.\n\
\n\
\n\
qsfp_reset\n\
\n\
  Read-Write:\n\
\n\
  Apply reset to the QSFP ports.\n\
  A bitmask with bits 0:31 for QSFP ports 1 - 32\n\
  Set a port's bit to 1 to initiate reset\n\
\n\
\n\
qsfp_lp_mode\n\
\n\
  Read-Write:\n\
\n\
  Put a QSFP in into or out of low power mode\n\
  A bitmask with bits 0:31 for QSFP ports 1 - 32\n\
  A 1 indicates low power mode.\n\
\n\
\n\
qsfp_present\n\
\n\
  Read-Only:\n\
\n\
  Indicate whether a there is a QSFP present.\n\
  A bitmask with bits 0:31 for QSFP ports 1 - 32\n\
  A 1 indicates that the QSFP is present.\n\
\n\
\n\
help\n\
README\n\
\n\
  Read-Only:\n\
\n\
  The text you are reading now.\n\
\n\
"

static ssize_t help_show(struct device * dev,
			 struct device_attribute * dattr,
			 char * buf)
{
	return sprintf(buf, HELP_STR);
}

static SYSFS_ATTR_RO(help, help_show);
static SYSFS_ATTR_RO(README, help_show);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */
static struct attribute *cel_seastone_cpld_attrs[] = {
	&dev_attr_write_protect.attr,
	&dev_attr_reset_source.attr,
	&dev_attr_cpld1_version.attr,
	&dev_attr_cpld2_version.attr,
	&dev_attr_cpld3_version.attr,
	&dev_attr_cpld4_version.attr,
	&dev_attr_cpld5_version.attr,
	&dev_attr_board_type.attr,
	&dev_attr_led_status.attr,
	&dev_attr_led_psu1.attr,
	&dev_attr_led_psu2.attr,
	&dev_attr_qsfp_reset.attr,
	&dev_attr_qsfp_lp_mode.attr,
	&dev_attr_qsfp_present.attr,
	&dev_attr_help.attr,
	&dev_attr_README.attr,
	NULL,
};
static struct attribute_group cel_seastone_cpld_attr_group = {
	.attrs = cel_seastone_cpld_attrs,
};

/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */
static int cel_seastone_cpld_probe(struct platform_device * dev)
{
	int ret = 0;

	printk("sea: probe\n");
	cel_seastone_cpld_regs = ioport_map(CPLD_IO_BASE, CPLD_IO_SIZE);

        if (!cel_seastone_cpld_regs) {
	     pr_err("cpld: unabled to map iomem\n");
	     ret = -ENODEV;
	     goto err_exit;
	}

        ret = sysfs_create_group(&dev->dev.kobj, &cel_seastone_cpld_attr_group);
        if (ret) {
	     pr_err("cpld: sysfs_create_group failed for cpld driver");
	     goto err_unmap;
        }
	return ret;

err_unmap:
        iounmap(cel_seastone_cpld_regs);

err_exit:
        return ret;
}

static int cel_seastone_cpld_remove(struct platform_device * ofdev)
{
	struct kobject * kobj = &ofdev->dev.kobj;

	iounmap(cel_seastone_cpld_regs);
	sysfs_remove_group(kobj, &cel_seastone_cpld_attr_group);

	platform_set_drvdata(ofdev, NULL);
	dev_info(&ofdev->dev, "removed\n");
	return 0;
}

static struct platform_driver cel_seastone_cpld_driver = {
	.probe = cel_seastone_cpld_probe,
	.remove = cel_seastone_cpld_remove,
	.driver = {
		.name  = driver_name,
		.owner = THIS_MODULE,
	},
};


/*------------------------------------------------------------------------------
 *
 * module interface
 *
 */

/* move the action of this to probe and remove and change these to
   of_register_platform_driver/register_platform_driver
   and
   of_unregister_platform_driver/unregister_platform_driver
*/
static struct platform_device *cel_seastone_cpld_device;

static int __init cel_seastone_cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&cel_seastone_cpld_driver);
	if (rv) {
		goto err_exit;
	}

	cel_seastone_cpld_device = platform_device_alloc(driver_name, 0);
        if (!cel_seastone_cpld_device) {
	     pr_err("platform_device_alloc() failed for cpld device\n");
	     rv = -ENOMEM;
	     goto err_unregister;
        }

	rv = platform_device_add(cel_seastone_cpld_device);
        if (rv) {
	     pr_err("platform_device_add() failed for cpld device.\n");
	     goto err_dealloc;
        }
        return 0;

err_dealloc:
	platform_device_unregister(cel_seastone_cpld_device);

err_unregister:
        platform_driver_unregister(&cel_seastone_cpld_driver);

err_exit:
	printk(KERN_ERR "%s platform_driver_register failed (%i)\n",
	       driver_name, rv);

	return rv;
}

static void __exit cel_seastone_cpld_exit(void)
{
	platform_device_unregister(cel_seastone_cpld_device);
	platform_driver_unregister(&cel_seastone_cpld_driver);
}

MODULE_AUTHOR("David Yen <dhyen@cumulusnetworks.com>");
MODULE_DESCRIPTION("CPLD driver for Celestica Inc., Seastone");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(cel_seastone_cpld_init);
module_exit(cel_seastone_cpld_exit);
