/*
 * cel_redstone_xp_b_platform.c - Celestica Redstone XP-B Platform Support
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
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/hwmon-sysfs.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "cel-redstone-xp-b-cpld.h"

#define PLATFORM_DRIVER_NAME  "cel_redstone_xp_b_platform"
#define CPLD_DRIVER_NAME      "cel_redstone_xp_b_cpld"
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
static struct cel_i2c_device_info i2c_rxpb_devices[] = {
	mk_i2cdev(CL_I2C_I801_BUS, "24c64", 0x56, &board_56_at24),
};

static struct i2c_client *cel_rxp_b_clients_list[ARRAY_SIZE(i2c_rxpb_devices)];

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
static uint8_t *cel_rxp_b_cpld_regs;

/****************************************************************************
 *
 * CPLD I/O
 *
 */
static uint8_t cpld_rd(uint32_t reg)
{
	uint8_t data;

	data = ioread8(cel_rxp_b_cpld_regs + reg - CPLD_IO_BASE);
	return data;
}

static void cpld_wr(uint32_t reg, uint8_t data)
{
	iowrite8(data, cel_rxp_b_cpld_regs + reg - CPLD_IO_BASE);
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
static ssize_t wp_show(struct device *dev,
		       struct device_attribute *dattr,
		       char *buf)
{
	uint8_t data;

	data = cpld_rd(CPLD4_REG_MISC_CTL);
	return sprintf(buf, "%s\n", (data & CPLD_EEPROM_WP) ? "1" : "0");
}

static ssize_t wp_store(struct device *dev,
			struct device_attribute *dattr,
			const char *buf, size_t count)
{
	unsigned int tmp;

	if (kstrtouint(buf, 0, &tmp) < 0 || tmp > 2)
		return -EINVAL;

	cpld_wr(CPLD4_REG_MISC_CTL, tmp);

	return count;
}
static SYSFS_ATTR_RW(eeprom_write_protect, wp_show, wp_store);

/******************************************************
 *
 * SFP
 *
 *****************************************************/
#define NUM_SFP_REGS 8
#define REG_NAME_LEN 16

struct redstone_xpb_sfp_reg_offset_info {
	uint8_t  mask;
	uint8_t  shift;
};

struct red_xpb_sfp_reg_info {
	uint32_t regs[NUM_SFP_REGS];
	int      active_low;
};

static struct red_xpb_sfp_reg_info rxp_sfp_reg_info[] = {
	{
		/* rx_los */
		.active_low = 1,
		.regs = {
			CPLD2_REG_SFP_1_8_RX_LOS_L,
			CPLD2_REG_SFP_9_16_RX_LOS_L,
			CPLD2_REG_SFP_17_18_RX_LOS_L,
			CPLD3_REG_SFP_19_26_RX_LOS_L,
			CPLD3_REG_SFP_27_34_RX_LOS_L,
			CPLD3_REG_SFP_35_36_RX_LOS_L,
			CPLD5_REG_SFP_37_44_RX_LOS_L,
			CPLD5_REG_SFP_45_48_RX_LOS_L
		},
	},
	{
		/* tx_disable */
		.regs = {
			CPLD2_REG_SFP_1_8_TX_DIS,
			CPLD2_REG_SFP_9_16_TX_DIS,
			CPLD2_REG_SFP_17_18_TX_DIS,
			CPLD3_REG_SFP_19_26_TX_DIS,
			CPLD3_REG_SFP_27_34_TX_DIS,
			CPLD3_REG_SFP_35_36_TX_DIS,
			CPLD5_REG_SFP_37_44_TX_DIS,
			CPLD5_REG_SFP_45_48_TX_DIS
		},
	},
	{
		/* rs */
		.regs = {
			CPLD2_REG_SFP_1_8_RS,
			CPLD2_REG_SFP_9_16_RS,
			CPLD2_REG_SFP_17_18_RS,
			CPLD3_REG_SFP_19_26_RS,
			CPLD3_REG_SFP_27_34_RS,
			CPLD3_REG_SFP_35_36_RS,
			CPLD5_REG_SFP_37_44_RS,
			CPLD5_REG_SFP_45_48_RS
		},
	},
	{
		/* tx_fault */
		.regs = {
			CPLD2_REG_SFP_1_8_TX_FAULT,
			CPLD2_REG_SFP_9_16_TX_FAULT,
			CPLD2_REG_SFP_17_18_TX_FAULT,
			CPLD3_REG_SFP_19_26_TX_FAULT,
			CPLD3_REG_SFP_27_34_TX_FAULT,
			CPLD3_REG_SFP_35_36_TX_FAULT,
			CPLD5_REG_SFP_37_44_TX_FAULT,
			CPLD5_REG_SFP_45_48_TX_FAULT
		},
	},
	{
		/* present */
		.active_low = 1,
		.regs = {
			CPLD2_REG_SFP_1_8_PRES_L,
			CPLD2_REG_SFP_9_16_PRES_L,
			CPLD2_REG_SFP_17_18_PRES_L,
			CPLD3_REG_SFP_19_26_PRES_L,
			CPLD3_REG_SFP_27_34_PRES_L,
			CPLD3_REG_SFP_35_36_PRES_L,
			CPLD5_REG_SFP_37_44_PRES_L,
			CPLD5_REG_SFP_45_48_PRES_L
		},
	}
};

#define NUM_SFP_REG_NAMES (sizeof(rxp_sfp_reg_info) / \
sizeof(struct red_xpb_sfp_reg_info))

static struct redstone_xpb_sfp_reg_offset_info redxp_reg_offset_info[] = {
	{
		.mask = 0xff,
		.shift = 0,
	},
	{
		.mask = 0xff,
		.shift = 8,
	},
	{
		.mask = 0x03,
		.shift = 16,
	},
	{
		.mask = 0xff,
		.shift = 18,
	},
	{
		.mask = 0xff,
		.shift = 26,
	},
	{
		.mask = 0x03,
		.shift = 34,
	},
	{
		.mask = 0xff,
		.shift = 36,
	},
	{
		.mask = 0x0f,
		.shift = 44,
	},
};

enum {
	SFP_RXLOS_IDX,
	SFP_TXDIS_IDX,
	SFP_RS_IDX,
	SFP_TXFAULT_IDX,
	SFP_PRESENT_IDX,
};

static ssize_t sfp_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct red_xpb_sfp_reg_info *reg_info = &rxp_sfp_reg_info[attr->index];
	uint64_t val = 0, rval;
	int i;

	for (i = 0; i < NUM_SFP_REGS; i++) {
		rval = cpld_rd(reg_info->regs[i]);
		val |= (rval & (uint64_t)redxp_reg_offset_info[i].mask) <<
			(uint64_t)redxp_reg_offset_info[i].shift;
	}
	if (reg_info->active_low)
		val = ~val;

	return sprintf(buf, "0x%012llx\n", val & 0xffffffffffff);
}

static ssize_t sfp_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct red_xpb_sfp_reg_info *reg_info = &rxp_sfp_reg_info[attr->index];
	uint64_t  data;
	int i;

	if (kstrtou64(buf, 0, &data) < 0)
		return -EINVAL;

	for (i = 0; i < NUM_SFP_REGS; i++) {
		cpld_wr(reg_info->regs[i],
			(data >> redxp_reg_offset_info[i].shift)
			 & redxp_reg_offset_info[i].mask);
	}

	return count;
}
static SENSOR_DEVICE_ATTR_RO(sfp_rx_los,     sfp_show,
			     SFP_RXLOS_IDX);
static SENSOR_DEVICE_ATTR_RW(sfp_tx_disable, sfp_show, sfp_store,
			     SFP_TXDIS_IDX);
static SENSOR_DEVICE_ATTR_RW(sfp_rs,         sfp_show, sfp_store,
			     SFP_RS_IDX);
static SENSOR_DEVICE_ATTR_RO(sfp_tx_fault,   sfp_show,
			     SFP_TXFAULT_IDX);
static SENSOR_DEVICE_ATTR_RO(sfp_present,    sfp_show,
			     SFP_PRESENT_IDX);

/******************************************************
 *
 * QSFP
 *
 *****************************************************/
struct red_xpb_qsfp_reg_info {
	uint32_t reg;
	int      active_low;
};

static struct red_xpb_qsfp_reg_info rxpb_qsfp_info[] = {
	{	/* qsfp_reset */
		.reg = CPLD4_REG_QSFP_1_6_RESET_L,
		.active_low = 1,
	},
	{	/* qsfp_present */
		.reg = CPLD4_REG_QSFP_1_6_PRESENT_L,
		.active_low = 1,
	},
	{	/* qsfp_lp_mod */
		.reg = CPLD4_REG_QSFP_1_6_LP_MOD,
	},
};

enum {
	QSFP_RESET_IDX,
	QSFP_PRESENT_IDX,
	QSFP_LPMOD_IDX,
};

static ssize_t qsfp_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct red_xpb_qsfp_reg_info *reg_info = &rxpb_qsfp_info[attr->index];
	uint8_t data;

	data = cpld_rd(reg_info->reg);
	if (reg_info->active_low)
		data = ~data;
	return sprintf(buf, "0x%02X\n", data & CPLD4_QSFP_MASK);
}

static ssize_t qsfp_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct red_xpb_qsfp_reg_info *reg_info = &rxpb_qsfp_info[attr->index];
	int val;

	if (kstrtouint(buf, 0, &val) != 0 || val > CPLD4_QSFP_MASK)
		return -EINVAL;

	if (reg_info->active_low)
		val = ~val;
	cpld_wr(reg_info->reg, (uint8_t)val);

	return count;
}
static SENSOR_DEVICE_ATTR(qsfp_lp_mode, S_IRUGO | S_IWUSR, qsfp_show,
			  qsfp_store, QSFP_LPMOD_IDX);
static SENSOR_DEVICE_ATTR(qsfp_reset,   S_IRUGO | S_IWUSR, qsfp_show,
			  qsfp_store, QSFP_RESET_IDX);
static SENSOR_DEVICE_ATTR(qsfp_present, S_IRUGO, qsfp_show,
			  NULL, QSFP_PRESENT_IDX);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */
static struct attribute *cel_rxp_b_cpld_attrs[] = {
	&dev_attr_eeprom_write_protect.attr,
	&dev_attr_cpld_version.attr,
	&sensor_dev_attr_sfp_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp_rs.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_fault.dev_attr.attr,
	&sensor_dev_attr_sfp_present.dev_attr.attr,
	&sensor_dev_attr_qsfp_reset.dev_attr.attr,
	&sensor_dev_attr_qsfp_lp_mode.dev_attr.attr,
	&sensor_dev_attr_qsfp_present.dev_attr.attr,
	NULL,
};

static struct attribute_group cel_rxp_b_cpld_attr_group = {
	.attrs = cel_rxp_b_cpld_attrs,
};

/*------------------------------------------------------------------------------
 *
 * cpld driver interface
 *
 */
static int cel_rxp_b_i2c_init(void)
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
	for (i = 0; i < ARRAY_SIZE(i2c_rxpb_devices); i++) {
		if (i2c_rxpb_devices[i].bus == CL_I2C_I801_BUS)
			i2c_rxpb_devices[i].bus = I801_bus;

		client = add_i2c_client(i2c_rxpb_devices[i].bus,
					&i2c_rxpb_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		cel_rxp_b_clients_list[i] = client;
	}
	return 0;

err_exit:
	return ret;
}

static void cel_rxp_b_i2c_exit(void)
{
	int i, idx;

	for (i = ARRAY_SIZE(i2c_rxpb_devices); i; i--) {
		idx = i - 1;
		if (cel_rxp_b_clients_list[idx])
			i2c_unregister_device(cel_rxp_b_clients_list[idx]);
	}
}

static int cel_rxp_b_cpld_probe(struct platform_device *dev)
{
	int ret = 0;

	ret = cel_rxp_b_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		goto err_exit;
	}

	cel_rxp_b_cpld_regs = ioport_map(CPLD_IO_BASE, CPLD_IO_SIZE);
	if (!cel_rxp_b_cpld_regs) {
		pr_err("cpld: unable to map iomem\n");
		ret = -ENODEV;
		goto err_exit;
	}

	if (!cpld_sanity_check()) {
		pr_err("cpld: could not validate CPLD registers.\n");
		ret = -EIO;
		goto err_unmap;
	}

	ret = sysfs_create_group(&dev->dev.kobj, &cel_rxp_b_cpld_attr_group);
	if (ret) {
		pr_err("cpld: sysfs_create_group failed for cpld driver");
		goto err_unmap;
	}
	return ret;

err_unmap:
	iounmap(cel_rxp_b_cpld_regs);

err_exit:
	cel_rxp_b_i2c_exit();
	return ret;
}

static int cel_rxp_b_cpld_remove(struct platform_device *dev)
{
	iounmap(cel_rxp_b_cpld_regs);
	cel_rxp_b_i2c_exit();
	return 0;
}

static struct platform_device *cel_rxp_b_cpld_device = NULL;

static struct platform_driver cel_rxp_b_cpld_driver = {
	.probe = cel_rxp_b_cpld_probe,
	.remove = cel_rxp_b_cpld_remove,
	.driver = {
		.name  = CPLD_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init cel_rxp_b_platform_init(void)
{
	int ret;

	pr_info(PLATFORM_DRIVER_NAME": version "DRIVER_VERSION" initializing\n");

	if (!driver_find(cel_rxp_b_cpld_driver.driver.name,
			 &platform_bus_type)) {
		ret = platform_driver_register(&cel_rxp_b_cpld_driver);
		if (ret) {
			pr_err("platform_driver_register() failed for cpld device\n");
			goto err_exit;
		}
	}

	if (cel_rxp_b_cpld_device == NULL) {
		cel_rxp_b_cpld_device = platform_device_alloc(CPLD_DRIVER_NAME, 0);
		if (!cel_rxp_b_cpld_device) {
			pr_err("platform_device_alloc() failed for cpld device\n");
			ret = -ENOMEM;
			goto err_driver;
		}

		ret = platform_device_add(cel_rxp_b_cpld_device);
		if (ret) {
			pr_err("platform_device_add() failed for cpld device.\n");
			goto err_device;
		}
	}

	pr_info(PLATFORM_DRIVER_NAME": version "DRIVER_VERSION
		" successfully initialized\n");
	return ret;

err_device:
	platform_device_unregister(cel_rxp_b_cpld_device);

err_driver:
	platform_driver_unregister(&cel_rxp_b_cpld_driver);

err_exit:
	return ret;
}

static void __exit cel_rxp_b_platform_exit(void)
{
	platform_device_unregister(cel_rxp_b_cpld_device);
	cel_rxp_b_cpld_device = NULL;
	platform_driver_unregister(&cel_rxp_b_cpld_driver);
	pr_info(PLATFORM_DRIVER_NAME ": version " DRIVER_VERSION " unloaded\n");
}

module_init(cel_rxp_b_platform_init);
module_exit(cel_rxp_b_platform_exit);

MODULE_AUTHOR("Alan Liebthal (alanl@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica Redstone XP-B Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
