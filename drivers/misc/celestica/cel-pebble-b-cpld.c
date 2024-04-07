// SPDX-License-Identifier: GPL-2.0+
/*
 * CPLD sysfs driver for Celestica Pebble BMC E1052
 *
 * Copyright (C) 2015, 2017, 2019 Cumulus Networks, Inc.
 * Author: Dave Olson <olson@cumulusnetworks.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.
 */

#include <stddef.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <linux/io.h>

#include "cel-pebble-b-cpld.h"
#include "platform-defs.h"
#include "platform-bitfield.h"

#define DRIVER_NAME    "cel_pebble_b_cpld"
#define DRIVER_VERSION "1.0"

static u8 *cel_pebble_b_cpld_regs;

/*
 *
 * CPLD I/O
 *
 */

static inline int cpld_read_reg(struct device *dev,
				int reg,
				int nregs,
				u32 *val)
{
	int i;

	*val = 0;
	for (i = 0; i < nregs; i++, reg++)
		*val |= ioread8(cel_pebble_b_cpld_regs + reg) << i * 8;

	return 0;
}

static inline int cpld_write_reg(struct device *dev,
				 int reg,
				 int nregs,
				 u32 val)
{
	for (; nregs > 0; nregs--, reg++, val >>= 8)
		iowrite8(val & 0xff, cel_pebble_b_cpld_regs + reg);

	return 0;
}

static const char * const sys_led_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN_BLINKING,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_YELLOW,
};

mk_bf_ro(cpld, cpld_major_revision,  CPLD_MMC_VERSION_REG,
	 CPLD_MMC_VERSION_H_SHIFT, 4, NULL, BF_DECIMAL);
mk_bf_ro(cpld, cpld_minor_revision,  CPLD_MMC_VERSION_REG,
	 CPLD_MMC_VERSION_L_SHIFT, 4, NULL, BF_DECIMAL);
mk_bf_rw(cpld, flash_write_protect, CPLD_MMC_FLASH_WP_REG, 0, 8, NULL, 0);
mk_bf_rw(cpld, led_system_status, CPLD_MMC_SYS_LED_REG,
	 CPLD_MMC_SYS_LED_GREEN_SHIFT, 2, sys_led_values, 0);

struct attribute *cel_pebble_b_cpld_attrs[] = {
	&cpld_cpld_major_revision.attr,
	&cpld_cpld_minor_revision.attr,
	&cpld_flash_write_protect.attr,
	&cpld_led_system_status.attr,
	NULL
};

static struct attribute_group cel_pebble_b_cpld_attr_group = {
	.attrs = cel_pebble_b_cpld_attrs,
};

/*------------------------------------------------------------------------------
 *
 * module interface
 *
 */
static struct platform_device *cel_pebble_b_cpld_device;

static int cel_pebble_b_cpld_probe(struct platform_device *dev)
{
	int ret;

	cel_pebble_b_cpld_regs = ioport_map(CPLD_IO_BASE,
					    CPLD_IO_BASE + CPLD_IO_SIZE);
	if (!cel_pebble_b_cpld_regs) {
		dev_err(&dev->dev, "unable to map iomem\n");
		ret = -ENODEV;
		goto err_exit;
	}

	ret = sysfs_create_group(&dev->dev.kobj, &cel_pebble_b_cpld_attr_group);
	if (ret) {
		dev_err(&dev->dev, "sysfs_create_group failed for cpld driver");
		goto err_unmap;
	}

err_unmap:
	iounmap(cel_pebble_b_cpld_regs);

err_exit:
	return ret;
}

static int cel_pebble_b_cpld_remove(struct platform_device *dev)
{
	iounmap(cel_pebble_b_cpld_regs);
	return 0;
}

static struct platform_driver cel_pebble_b_cpld_driver = {
	.driver = {
	.name = DRIVER_NAME,
	.owner = THIS_MODULE,
	},
	.probe = cel_pebble_b_cpld_probe,
	.remove = cel_pebble_b_cpld_remove,
};

static int __init cel_pebble_b_cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&cel_pebble_b_cpld_driver);
	if (rv)
		goto err_exit;

	cel_pebble_b_cpld_device = platform_device_alloc(DRIVER_NAME, 0);
	if (!cel_pebble_b_cpld_device) {
		pr_err(DRIVER_NAME
		       ": platform_device_alloc() failed for cpld device\n");
		rv = -ENOMEM;
		goto err_unregister;
	}

	rv = platform_device_add(cel_pebble_b_cpld_device);
	if (rv) {
		pr_err(DRIVER_NAME
		       ": platform_device_add() failed for cpld device.\n");
		goto err_dealloc;
	}
	return 0;

err_dealloc:
	platform_device_unregister(cel_pebble_b_cpld_device);

err_unregister:
	platform_driver_unregister(&cel_pebble_b_cpld_driver);

err_exit:
	pr_err(DRIVER_NAME ": platform_driver_register failed (%i)\n", rv);
	return rv;
}

static void __exit cel_pebble_b_cpld_exit(void)
{
	platform_device_unregister(cel_pebble_b_cpld_device);
	platform_driver_unregister(&cel_pebble_b_cpld_driver);
}

MODULE_AUTHOR("Dave Olson <olson@cumulusnetworks.com>");
MODULE_DESCRIPTION("Platform CPLD driver for Celestica Pebble BMC E1052");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(cel_pebble_b_cpld_init);
module_exit(cel_pebble_b_cpld_exit);
