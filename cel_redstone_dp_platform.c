/*
 * cel_redstone_dp_platform.c - Celestica Redstone-DP Platform Support
 *
 * Copyright (C) 2018 Cumulus Networks, Inc.  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/i2c/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/at24.h>
#include <linux/gpio.h>

#include <linux/cumulus-platform.h>
#include <linux/hwmon-sysfs.h>
#include "platform_defs.h"
#include "platform_bitfield.h"
#include "cel_redstone_dp.h"

#define PLATFORM_DRIVER_NAME  "cel_redstone_dp_platform"
#define CPLD_DRIVER_NAME      "cel_redstone_dp_cpld"
#define DRIVER_VERSION	      "1.0"

/* Only one EEPROM, but it's not the board/ONIE EEPROM */

mk_eeprom(cpu, 56, 256, AT24_FLAG_IRUGO);

/*
 * I2C Device Table.  Use the mk_i2cdev() macro to construct the entries.
 * Each entry is a bus number and a i2c_board_info.  The i2c_board_info
 * structure specifies the device type, address, and platform data specific
 * to the device type.
 */

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_I801_BUS, "24c02", 0x56, &cpu_56_at24),
};

/* CPLD driver */

static uint8_t *cpld_regs;

/* Accessor functions for reading and writing the CPLD registers */

static int cpld_read_reg(struct device *dev,
			 int reg,
			 int nregs,
			 u32 *val)
{
	*val = ioread8(cpld_regs + reg - CPLD_IO_BASE);
	return 0;
}

static int cpld_write_reg(struct device *dev,
			  int reg,
			  int nregs,
			  u32 val)
{
	iowrite8(val, cpld_regs + reg - CPLD_IO_BASE);
	return 0;
}

/* string arrays */

static const char * const i2c_sel_values[] = {
	"CPU",
	"BMC",
};

static const char * const board_type_values[] = {
	"Reserved",
	"Redstone-XP",
	"Smallstone-XP",
	"Seastone",
	"Questone",
	"Midstone",
	"Redstone-DP",
	"Reserved",
};

/* bitfield definitions */

mk_bf_rw(cpld, i2c_select,
	 CPLD_I2C_SELECT_REG, 0, 1, i2c_sel_values, 0);

mk_bf_ro(cpld, cpld1_version,
	 CPLD1_VERSION_REG, 0, 4, NULL, BF_DECIMAL);
mk_bf_ro(cpld, cpld2_version,
	 CPLD2_VERSION_REG, 0, 4, NULL, BF_DECIMAL);
mk_bf_ro(cpld, cpld3_version,
	 CPLD3_VERSION_REG, 0, 4, NULL, BF_DECIMAL);
mk_bf_ro(cpld, cpld4_version,
	 CPLD4_VERSION_REG, 0, 4, NULL, BF_DECIMAL);

mk_bf_ro(cpld, board_rev,
	 CPLD1_BOARD_REVISION_REG, 0, 3, NULL, 0);
mk_bf_rw(cpld, pca9548a_u36_reset,
	 CPLD1_RESET_CONTROL_1_REG, 7, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548a_u1_reset,
	 CPLD1_RESET_CONTROL_1_REG, 6, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, i210_reset,
	 CPLD1_RESET_CONTROL_1_REG, 5, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, dpll_reset,
	 CPLD1_RESET_CONTROL_1_REG, 4, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, bcm88375_reset,
	 CPLD1_RESET_CONTROL_1_REG, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, cpld4_reset,
	 CPLD1_RESET_CONTROL_1_REG, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, cpld3_reset,
	 CPLD1_RESET_CONTROL_1_REG, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, cpld2_reset,
	 CPLD1_RESET_CONTROL_1_REG, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, bcm52311_reset,
	 CPLD1_RESET_CONTROL_2_REG, 4, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, bcm52311_pcie_reset,
	 CPLD1_RESET_CONTROL_2_REG, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, bcm52311_core_reset,
	 CPLD1_RESET_CONTROL_2_REG, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, ucd90120_reset,
	 CPLD1_RESET_CONTROL_2_REG, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, zl30253_reset,
	 CPLD1_RESET_CONTROL_2_REG, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_rw(cpld, qsfp28_reset,
	 CPLD1_QSFP_1_6_RESET_REG, 0, 6, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_lpmode,
	 CPLD1_QSFP_1_6_LPMOD_REG, 0, 6, NULL, 0);
mk_bf_rw(cpld, qsfp28_present,
	 CPLD1_QSFP_1_6_ABS_REG, 0, 6, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_interrupt,
	 CPLD1_QSFP_1_6_INT_N_REG, 0, 6, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_i2c_ready,
	 CPLD1_QSFP_1_6_I2C_READY_REG, 0, 6, NULL, 0);
mk_bf_rw(cpld, qsfp28_power_good,
	 CPLD1_QSFP_1_6_PG_REG,	  0, 6, NULL, 0);

mk_bf_rw(cpld, board_type,
	 CPLD1_SWITCH_BOARD_TYPE_REG,  0, 8, board_type_values, 0);

/* sfp registers */

#define NUM_SFP_REGS 8

static uint8_t cpld_rd(uint32_t reg)
{
	uint8_t data;

	data = ioread8(cpld_regs + reg - CPLD_IO_BASE);
	return data;
}

static void cpld_wr(uint32_t reg, uint8_t data)
{
	iowrite8(data, cpld_regs + reg - CPLD_IO_BASE);
}

struct reg_info {
	uint32_t regs[NUM_SFP_REGS];
	int	 active_low;
};

static struct reg_info sfp_reg_info[] = {
	{
		.regs = {
			CPLD2_SFP_1_8_RX_LOS_REG,
			CPLD2_SFP_9_16_RX_LOS_REG,
			CPLD2_SFP_17_18_RX_LOS_REG,
			CPLD3_SFP_19_26_RX_LOS_REG,
			CPLD3_SFP_27_34_RX_LOS_REG,
			CPLD3_SFP_35_36_RX_LOS_REG,
			CPLD4_SFP_37_44_RX_LOS_REG,
			CPLD4_SFP_45_48_RX_LOS_REG,
		},
		.active_low = 1,
	},
	{
		.regs = {
			CPLD2_SFP_1_8_TX_DISABLE_REG,
			CPLD2_SFP_9_16_TX_DISABLE_REG,
			CPLD2_SFP_17_18_TX_DISABLE_REG,
			CPLD3_SFP_19_26_TX_DISABLE_REG,
			CPLD3_SFP_27_34_TX_DISABLE_REG,
			CPLD3_SFP_35_36_TX_DISABLE_REG,
			CPLD4_SFP_37_44_TX_DISABLE_REG,
			CPLD4_SFP_45_48_TX_DISABLE_REG,
		},
		.active_low = 0,
	},
	{
		.regs = {
			CPLD2_SFP_1_8_RS_REG,
			CPLD2_SFP_9_16_RS_REG,
			CPLD2_SFP_17_18_RS_REG,
			CPLD3_SFP_19_26_RS_REG,
			CPLD3_SFP_27_34_RS_REG,
			CPLD3_SFP_35_36_RS_REG,
			CPLD4_SFP_37_44_RS_REG,
			CPLD4_SFP_45_48_RS_REG,
		},
		.active_low = 0,
	},
	{
		.regs = {
			CPLD2_SFP_1_8_TX_FAULT_REG,
			CPLD2_SFP_9_16_TX_FAULT_REG,
			CPLD2_SFP_17_18_TX_FAULT_REG,
			CPLD3_SFP_19_26_TX_FAULT_REG,
			CPLD3_SFP_27_34_TX_FAULT_REG,
			CPLD3_SFP_35_36_TX_FAULT_REG,
			CPLD4_SFP_37_44_TX_FAULT_REG,
			CPLD4_SFP_45_48_TX_FAULT_REG,
		},
		.active_low = 0,
	},
	{
		.regs = {
			CPLD2_SFP_1_8_MOD_ABS_REG,
			CPLD2_SFP_9_16_MOD_ABS_REG,
			CPLD2_SFP_17_18_MOD_ABS_REG,
			CPLD3_SFP_19_26_MOD_ABS_REG,
			CPLD3_SFP_27_34_MOD_ABS_REG,
			CPLD3_SFP_35_36_MOD_ABS_REG,
			CPLD4_SFP_37_44_MOD_ABS_REG,
			CPLD4_SFP_45_48_MOD_ABS_REG,
		},
		.active_low = 1,
	},
};

struct reg_offset_info {
	uint8_t mask;
	uint8_t shift;
};

static struct reg_offset_info sfp_reg_offset_info[] = {
	{ 0xff,	 0 },
	{ 0xff,	 8 },
	{ 0x03, 16 },
	{ 0xff, 18 },
	{ 0xff, 26 },
	{ 0x03, 34 },
	{ 0xff, 36 },
	{ 0x0f, 44 },
};

enum {
	SFP_RX_LOS_IDX,
	SFP_TX_DISABLE_IDX,
	SFP_RS_IDX,
	SFP_TX_FAULT_IDX,
	SFP_MOD_ABS_IDX,
};

static ssize_t sfp_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct reg_info *reg_info = &sfp_reg_info[attr->index];
	uint64_t val = 0, rval;
	int i;

	for (i = 0; i < NUM_SFP_REGS; i++) {
		rval = cpld_rd(reg_info->regs[i]);
		val |= (rval & (uint64_t)sfp_reg_offset_info[i].mask) <<
			(uint64_t)sfp_reg_offset_info[i].shift;
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
	struct reg_info *reg_info = &sfp_reg_info[attr->index];
	uint64_t  data;
	int i;

	if (kstrtou64(buf, 0, &data) < 0)
		return -EINVAL;

	for (i = 0; i < NUM_SFP_REGS; i++) {
		cpld_wr(reg_info->regs[i],
			(data >> sfp_reg_offset_info[i].shift)
			& sfp_reg_offset_info[i].mask);
	}

	return count;
}

static SENSOR_DEVICE_ATTR_RO(sfp_rx_los,
			     sfp_show,		  SFP_RX_LOS_IDX);
static SENSOR_DEVICE_ATTR_RW(sfp_tx_disable,
			     sfp_show, sfp_store, SFP_TX_DISABLE_IDX);
static SENSOR_DEVICE_ATTR_RW(sfp_rs,
			     sfp_show, sfp_store, SFP_RS_IDX);
static SENSOR_DEVICE_ATTR_RO(sfp_tx_fault,
			     sfp_show,		  SFP_TX_FAULT_IDX);
static SENSOR_DEVICE_ATTR_RO(sfp_present,
			     sfp_show,		  SFP_MOD_ABS_IDX);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_i2c_select.attr,
	&cpld_cpld1_version.attr,
	&cpld_cpld2_version.attr,
	&cpld_cpld3_version.attr,
	&cpld_cpld4_version.attr,

	&cpld_board_rev.attr,
	&cpld_pca9548a_u36_reset.attr,
	&cpld_pca9548a_u1_reset.attr,
	&cpld_i210_reset.attr,
	&cpld_dpll_reset.attr,
	&cpld_bcm88375_reset.attr,
	&cpld_cpld4_reset.attr,
	&cpld_cpld3_reset.attr,
	&cpld_cpld2_reset.attr,
	&cpld_bcm52311_reset.attr,
	&cpld_bcm52311_pcie_reset.attr,
	&cpld_bcm52311_core_reset.attr,
	&cpld_ucd90120_reset.attr,
	&cpld_zl30253_reset.attr,
	&cpld_qsfp28_reset.attr,
	&cpld_qsfp28_lpmode.attr,
	&cpld_qsfp28_present.attr,
	&cpld_qsfp28_interrupt.attr,
	&cpld_qsfp28_i2c_ready.attr,
	&cpld_qsfp28_power_good.attr,
	&cpld_board_type.attr,

	&sensor_dev_attr_sfp_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp_rs.dev_attr.attr,
	&sensor_dev_attr_sfp_tx_fault.dev_attr.attr,
	&sensor_dev_attr_sfp_present.dev_attr.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

/* CPLD driver initialization */

static int cpld_probe(struct platform_device *dev)
{
	int ret;

	cpld_regs = ioport_map(CPLD_IO_BASE, CPLD_IO_SIZE);
	if (!cpld_regs) {
		pr_err("cpld_probe: unable to map iomem\n");
		return -ENODEV;
	}

	ret = sysfs_create_group(&dev->dev.kobj, &cpld_attr_group);
	if (ret) {
		pr_err("cpld_probe: sysfs_create_group failed for cpld driver");
		iounmap(cpld_regs);
		return ret;
	}

	return 0;
}

static int cpld_remove(struct platform_device *dev)
{
	iounmap(cpld_regs);
	return 0;
}

static struct platform_device *cpld_device;

static struct platform_driver cpld_driver = {
	.probe = cpld_probe,
	.remove = cpld_remove,
	.driver = {
		.name  = CPLD_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int cpld_init(void)
{
	int ret;

	ret = platform_driver_register(&cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for cpld device\n");
		goto err_exit;
	}

	cpld_device = platform_device_alloc(CPLD_DRIVER_NAME, 0);
	if (!cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device\n");
		ret = -ENOMEM;
		goto err_driver;
	}

	ret = platform_device_add(cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_device;
	}
	return 0;

err_device:
	platform_device_unregister(cpld_device);

err_driver:
	platform_driver_unregister(&cpld_driver);

err_exit:
	return ret;
}

static void cpld_exit(void)
{
	platform_driver_unregister(&cpld_driver);
	platform_device_unregister(cpld_device);
}

/* I2C driver initialization */

static int i2c_init(void)
{
	int i801_bus;
	int i;
	struct i2c_client *client;

	i801_bus = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (i801_bus < 0) {
		pr_err("could not find i801 adapter bus\n");
		return -ENODEV;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;

		if (bus == I2C_I801_BUS)
			bus = i801_bus;
		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client))
			return PTR_ERR(client);
		i2c_devices[i].client = client;
	}
	return 0;
}

static void i2c_exit(void)
{
	int i;

	for (i = ARRAY_SIZE(i2c_devices); --i >= 0;) {
		struct i2c_client *c = i2c_devices[i].client;

		if (c) {
			i2c_devices[i].client = NULL;
			i2c_unregister_device(c);
		}
	}
}

/* Platform initialization */

static int __init cel_red_dp_platform_init(void)
{
	int ret;

	ret = i2c_init();
	if (ret) {
		pr_err("I2C subsystem initialization failed\n");
		return ret;
	}

	ret = cpld_init();
	if (ret) {
		pr_err("CPLD subsystem initialization failed\n");
		i2c_exit();
		return ret;
	}

	pr_info("%s: version %s successfully loaded\n",
		PLATFORM_DRIVER_NAME, DRIVER_VERSION);
	return 0;
}

static void __exit cel_red_dp_platform_exit(void)
{
	cpld_exit();
	i2c_exit();
	pr_info(PLATFORM_DRIVER_NAME ": version " DRIVER_VERSION " unloaded\n");
}

module_init(cel_red_dp_platform_init);
module_exit(cel_red_dp_platform_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica Redstone-DP Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
