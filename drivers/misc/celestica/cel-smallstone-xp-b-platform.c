/*
 * cel_smallstone_xp_b_platform.c - Celestica Questone Platform Support
 *
 * Copyright (C) 2017 Cumulus Networks, Inc. all rights reserved
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
  */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "cel-smallstone-xp-b-cpld.h"

#define PLATFORM_DRIVER_NAME  "cel_smallstone_xp_b_platform"
#define CPLD_DRIVER_NAME      "cel_smallstone_xp_b_cpld"
#define DRIVER_VERSION        "1.0"

/*
 * only one EEPROM that we need to populate the board/ONIE EEPROM
 */
mk_eeprom_psize(board, 56, 8192, 32, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);

/*
 * struct cel_i2c_device_info -- i2c device container struct.  This
 * struct helps organize the i2c_board_info structures and the created
 * i2c_client objects.
 *
 */
struct cel_i2c_device_info {
	int bus;
	struct i2c_board_info board_info;
};

/*
 * I2C Device Table.  Use the mk_i2cdev() macro to construct the entries.
 * Each entry is a bus number and a i2c_board_info.  The i2c_board_info
 * structure specifies the device type, address, and platform data specific
 * to the device type.
 */
static struct cel_i2c_device_info i2c_qstone_devices[] = {
	mk_i2cdev(CL_I2C_I801_BUS, "24c64", 0x56, &board_56_at24),
};

static struct i2c_client *cel_sxp_b_clients[ARRAY_SIZE(i2c_qstone_devices)];

static struct i2c_adapter *get_adapter(int bus)
{
	int bail = 20;
	struct i2c_adapter *adapter;

	for (; bail; bail--) {
		adapter = i2c_get_adapter(bus);
		if (adapter)
			return adapter;
		msleep(100);
	}
	return NULL;
}

static struct i2c_client *add_i2c_client(int bus,
					 struct i2c_board_info *board_info)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = get_adapter(bus);
	if (!adapter) {
		pr_err("could not get adapter %u\n", bus);
		return ERR_PTR(-ENODEV);
	}
	client = i2c_new_device(adapter, board_info);
	if (!client) {
		pr_err("could not add device\n");
		return ERR_PTR(-ENODEV);
	}
	return client;
}

static int get_bus_by_name(char *name)
{
	struct i2c_adapter *adapter;
	int i;

	for (i = 0; i < CL_I2C_MUX1_BUS0; i++) {
		adapter = get_adapter(i);
		if (adapter &&
		    (strncmp(adapter->name, name, strlen(name)) == 0)) {
			return i;
		}
	}
	return -1;
}

/*---------------------------------------------------------------------
 *
 * CPLD driver
 *
 *-------------------------------------------------------------------*/
static uint8_t *cel_sxp_b_cpld_regs;

/**********************************************************************
 *
 * CPLD I/O
 *
 */
static uint8_t cpld_rd(uint32_t reg)
{
	uint8_t data;

	data = ioread8(cel_sxp_b_cpld_regs + reg - CPLD_IO_BASE);
	return data;
}

static void cpld_wr(uint32_t reg, uint8_t data)
{
	iowrite8(data, cel_sxp_b_cpld_regs + reg - CPLD_IO_BASE);
}

static uint32_t cel_red_xpb_ver_regs[] = {
	CPLD2_REG_VERSION, CPLD3_REG_VERSION,
	CPLD4_REG_VERSION, CPLD5_REG_VERSION,
};

/*
 * a simple sanity check to make sure that the CPLDs exist
 * and are valid. Check for the valid ID in each CPLD's version
 * register. cpld ids are: 2, 3, 4, 5
 */
static int cpld_sanity_check(void)
{
	int i;
	uint8_t data;

	for (i = 0; i < ARRAY_SIZE(cel_red_xpb_ver_regs); i++) {
		data = cpld_rd(cel_red_xpb_ver_regs[i]);
		if (((data & CPLD_ID_MASK) >> CPLD_ID_SHIFT) != (i + 2))
			return false;
	}
	return true;
}

/*
 * cpld version
 */
static ssize_t cpld_version_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	int i;
	uint8_t data;
	char dot = 'v';

	for (i = 0; i < ARRAY_SIZE(cel_red_xpb_ver_regs); i++) {
		data = cpld_rd(cel_red_xpb_ver_regs[i]);
		sprintf(buf + strlen(buf), "%c%u", dot,
			data & CPLD_VERSION_MASK);
		dot = '.';
	}
	strcat(buf, "\n");
	return strlen(buf);
}

static SYSFS_ATTR_RO(cpld_version, cpld_version_show);

/*
 * write protect
 */
static ssize_t write_protect_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	uint8_t data;

	data = cpld_rd(CPLD4_REG_MISC_CTL);
	return sprintf(buf, "%s\n", (data & CPLD_EEPROM_WP) ? "1" : "0");
}

static ssize_t write_protect_store(struct device *dev,
				   struct device_attribute *dattr,
				   const char *buf, size_t count)
{
	unsigned int tmp;

	if (kstrtouint(buf, 0, &tmp) < 0 || tmp > 2)
		return -EINVAL;

	cpld_wr(CPLD4_REG_MISC_CTL, tmp);

	return count;
}
static SYSFS_ATTR_RW(eeprom_write_protect, write_protect_show,
		     write_protect_store);

/******************************************************
 *
 * QSFP
 *
 *****************************************************/
#define NUM_QSFP_REGS 4
struct qsfp_info_struct {
	uint32_t regs[NUM_QSFP_REGS];
	char name[PLATFORM_LED_COLOR_NAME_SIZE];
	int active_low;
};

static struct qsfp_info_struct qsfp_info[] = {
	{
		.regs = {CPLD2_REG_QSFP_1_8_RESET_L,
			 CPLD2_REG_QSFP_9_16_RESET_L,
			 CPLD3_REG_QSFP_17_24_RESET_L,
			 CPLD3_REG_QSFP_25_32_RESET_L},
		.name = "qsfp_reset",
		.active_low = 1,
	},
	{
		.regs = {CPLD2_REG_QSFP_1_8_LP_MOD,
			 CPLD2_REG_QSFP_9_16_LP_MOD,
			 CPLD3_REG_QSFP_17_24_LP_MOD,
			 CPLD3_REG_QSFP_25_32_LP_MOD},
		.name = "qsfp_lp_mode",
	},
	{
		.regs = {CPLD2_REG_QSFP_1_8_PRESENT_L,
			 CPLD2_REG_QSFP_9_16_PRESENT_L,
			 CPLD3_REG_QSFP_17_24_PRESENT_L,
			 CPLD3_REG_QSFP_25_32_PRESENT_L},
		.name = "qsfp_present",
		.active_low = 1,
	},
	{
		.regs = {CPLD2_REG_QSFP_1_8_I2C_READY,
			 CPLD2_REG_QSFP_9_16_I2C_READY,
			 CPLD3_REG_QSFP_17_24_I2C_READY,
			 CPLD3_REG_QSFP_25_32_I2C_READY},
		.name = "qsfp_i2c_ready",
	},
};

static struct qsfp_info_struct *get_qsfp_info(struct device_attribute *dattr)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(qsfp_info); i++)
		if (strncmp(dattr->attr.name, qsfp_info[i].name,
			    strlen(qsfp_info[i].name)) == 0)
			return &qsfp_info[i];
	return 0;
}

static ssize_t qsfp_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	struct qsfp_info_struct *info_ptr;
	uint32_t value = 0;
	int i;

	info_ptr = get_qsfp_info(dattr);
	if (!info_ptr)
		return -EINVAL;

	for (i = NUM_QSFP_REGS; i; i--) {
		value <<= 8;
		value |= cpld_rd(info_ptr->regs[i - 1]);
	}
	if (info_ptr->active_low)
		value = ~value;
	return sprintf(buf, "0x%X\n", value);
}

static ssize_t qsfp_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	struct qsfp_info_struct *info_ptr = get_qsfp_info(dattr);
	u32 val;
	int i;

	if (!info_ptr)
		return -EINVAL;

	if (kstrtou32(buf, 0, &val) != 0)
		return -EINVAL;

	if (info_ptr->active_low)
		val = ~val;

	for (i = 0; i < NUM_QSFP_REGS; i++) {
		cpld_wr(info_ptr->regs[i], val & 0xff);
		val >>= 8;
	}
	return count;
}
static SYSFS_ATTR_RW(qsfp_lp_mode, qsfp_show, qsfp_store);
static SYSFS_ATTR_RW(qsfp_reset,   qsfp_show, qsfp_store);
static SYSFS_ATTR_RO(qsfp_present, qsfp_show);
static SYSFS_ATTR_RO(qsfp_i2c_ready, qsfp_show);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */
static struct attribute *cel_sxp_b_cpld_attrs[] = {
	&dev_attr_eeprom_write_protect.attr,
	&dev_attr_cpld_version.attr,
	&dev_attr_qsfp_reset.attr,
	&dev_attr_qsfp_lp_mode.attr,
	&dev_attr_qsfp_present.attr,
	&dev_attr_qsfp_i2c_ready.attr,
	NULL,
};

static struct attribute_group cel_sxp_b_cpld_attr_group = {
	.attrs = cel_sxp_b_cpld_attrs,
};

/*------------------------------------------------------------------------------
 *
 * cpld driver interface
 *
 */
static int cel_sxp_b_cpld_probe(struct platform_device *dev)
{
	int ret = 0;

	cel_sxp_b_cpld_regs = ioport_map(CPLD_IO_BASE, CPLD_IO_SIZE);
	if (!cel_sxp_b_cpld_regs) {
		pr_err("cpld: unable to map iomem\n");
		ret = -ENODEV;
		goto err_exit;
	}

	if (!cpld_sanity_check()) {
		pr_err("cpld: could not validate CPLD registers.\n");
		ret = -EIO;
		goto err_unmap;
	}

	ret = sysfs_create_group(&dev->dev.kobj, &cel_sxp_b_cpld_attr_group);
	if (ret) {
		pr_err("cpld: sysfs_create_group failed for cpld driver");
		goto err_unmap;
	}
	return ret;

err_unmap:
	iounmap(cel_sxp_b_cpld_regs);

err_exit:
	return ret;
}

static int cel_sxp_b_cpld_remove(struct platform_device *dev)
{
	iounmap(cel_sxp_b_cpld_regs);
	return 0;
}

static struct platform_device *cel_sxp_b_cpld_device;

static struct platform_driver cel_sxp_b_cpld_driver = {
	.probe = cel_sxp_b_cpld_probe,
	.remove = cel_sxp_b_cpld_remove,
	.driver = {
		.name  = CPLD_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int cel_sxp_b_cpld_init(void)
{
	int ret;

	ret = platform_driver_register(&cel_sxp_b_cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for cpld device\n");
		goto err_exit;
	}

	cel_sxp_b_cpld_device = platform_device_alloc(CPLD_DRIVER_NAME, 0);
	if (!cel_sxp_b_cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device\n");
		ret = -ENOMEM;
		goto err_driver;
	}

	ret = platform_device_add(cel_sxp_b_cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_device;
	}
	return 0;

err_device:
	platform_device_unregister(cel_sxp_b_cpld_device);

err_driver:
	platform_driver_unregister(&cel_sxp_b_cpld_driver);

err_exit:
	return ret;
}

static void cel_sxp_b_cpld_exit(void)
{
	platform_driver_unregister(&cel_sxp_b_cpld_driver);
	platform_device_unregister(cel_sxp_b_cpld_device);
}

static int cel_sxp_b_i2c_init(void)
{
	int I801_bus;
	int i;
	int ret;
	struct i2c_client *client;

	I801_bus = get_bus_by_name(I801_ADAPTER_NAME);
	if (I801_bus < 0) {
		pr_err("could not find I801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	/* populate the platform specific i2c devices
	*/
	for (i = 0; i < ARRAY_SIZE(i2c_qstone_devices); i++) {
		if (i2c_qstone_devices[i].bus == CL_I2C_I801_BUS)
			i2c_qstone_devices[i].bus = I801_bus;

		client = add_i2c_client(i2c_qstone_devices[i].bus,
					&i2c_qstone_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		cel_sxp_b_clients[i] = client;
	}
	return 0;

err_exit:
	return ret;
}

static void cel_sxp_b_i2c_exit(void)
{
	int i, idx;

	for (i = ARRAY_SIZE(i2c_qstone_devices); i; i--) {
		idx = i - 1;
		if (cel_sxp_b_clients[idx])
			i2c_unregister_device(cel_sxp_b_clients[idx]);
	}
}

static int __init cel_sxp_b_platform_init(void)
{
	int ret;

	ret = cel_sxp_b_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		goto err_exit;
	}

	ret = cel_sxp_b_cpld_init();
	if (ret) {
		pr_err("Initializing CPLD subsystem failed\n");
		goto err_exit2;
	}

	pr_info(PLATFORM_DRIVER_NAME ": version %s successfully loaded\n",
		DRIVER_VERSION);
	return 0;

err_exit2:
	cel_sxp_b_i2c_exit();
err_exit:
	return ret;
}

static void __exit cel_sxp_b_platform_exit(void)
{
	cel_sxp_b_cpld_exit();
	cel_sxp_b_i2c_exit();
	pr_info(PLATFORM_DRIVER_NAME ": ver " DRIVER_VERSION " unloaded\n");
}

module_init(cel_sxp_b_platform_init);
module_exit(cel_sxp_b_platform_exit);

MODULE_AUTHOR("Alan Liebthal (alanl@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica Smallstone XP-B Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
