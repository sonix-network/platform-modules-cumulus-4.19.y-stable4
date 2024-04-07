/*
 * cel_questone_platform.c - Celestica Questone Platform Support
 *
 * Copyright (c) 2017, 2018, 2019 Cumulus Networks, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 */

 /*
  * The platform module for the Celestica Questone platform
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
#include <linux/hwmon-sysfs.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "cel-questone-cpld.h"

#define PLATFORM_DRIVER_NAME  "cel_questone_platform"
#define CPLD_DRIVER_NAME      "cel_questone_cpld"
#define DRIVER_VERSION	      "1.0"

static struct platform_driver cel_qstone_platform_driver;

/* only one EEPROM that we need to populate the board/ONIE EEPROM */

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

static struct i2c_client
	      *cel_qstone_clients_list[ARRAY_SIZE(i2c_qstone_devices)];

static void cel_qstone_i2c_exit(void)
{
	int i, idx;

	for (i = ARRAY_SIZE(i2c_qstone_devices); i; i--) {
		idx = i - 1;
		if (cel_qstone_clients_list[idx])
			i2c_unregister_device(cel_qstone_clients_list[idx]);
	}
}

/* CPLD driver */

static uint8_t *cel_qstone_cpld_regs;

static inline uint8_t cpld_rd(uint32_t reg)
{
	return ioread8(cel_qstone_cpld_regs + reg - CPLD_IO_BASE);
}

static inline void cpld_wr(uint32_t reg, uint8_t data)
{
	iowrite8(data, cel_qstone_cpld_regs + reg - CPLD_IO_BASE);
}

/* CPLD version registers */

static uint32_t que_version_regs[] = {
	CPLD_BASE_REG_VERSION,
	CPLD_SW1_REG_VERSION,
	CPLD_SW2_REG_VERSION,
	CPLD_SW3_REG_VERSION,
	CPLD_SW4_REG_VERSION,
};

/*
 * a simple sanity check to make sure that the CPLDs exist
 * and are valid. Check for the valid ID in each CPLD's version
 * register. cpld ids are: 1, 1, 2, 3, 4
 */
static int cpld_sanity_check(void)
{
	int i;
	uint8_t data;

	for (i = 0; i < ARRAY_SIZE(que_version_regs); i++) {
		data = cpld_rd(que_version_regs[i]);
		if (((data & CPLDB_ID_MASK) >> 4) != (i ? i : 1))
			return false;
	}
	return true;
}

static ssize_t cpld_version_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	int cpld_idx;
	uint8_t rev;

	cpld_idx = dattr->attr.name[4] - '0';
	if (cpld_idx < 0 ||
	    cpld_idx >= sizeof(que_version_regs) / sizeof(uint32_t))
		return -EINVAL;
	rev = cpld_rd(que_version_regs[cpld_idx]) & CPLDB_VERSION_MASK;
	return sprintf(buf, "%d\n", rev);
}

static SYSFS_ATTR_RO(cpld0_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld1_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld2_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld3_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld4_version, cpld_version_show);

static ssize_t eeprom_write_protect_show(struct device *dev,
					 struct device_attribute *dattr,
					 char *buf)
{
	uint8_t data;

	data = cpld_rd(CPLD_BASE_REG_EEPROM_WP);
	return sprintf(buf, "%lu\n", (data & CPLDB_WP_BOARD_EEPROM) >> 2);
}

static ssize_t eeprom_write_protect_store(struct device *dev,
					  struct device_attribute *dattr,
					  const char *buf, size_t count)
{
	uint8_t data;
	int	bit;
	int	ret;

	ret = kstrtoint(buf, 0, &bit);
	if (ret < 0)
		return ret;

	data = cpld_rd(CPLD_BASE_REG_EEPROM_WP);
	if (bit == 1)
		data |= CPLDB_WP_BOARD_EEPROM;
	else if (bit == 0)
		data &= ~CPLDB_WP_BOARD_EEPROM;
	else
		return -EINVAL;
	cpld_wr(CPLD_BASE_REG_EEPROM_WP, data);
	return count;
}

static SYSFS_ATTR_RW(eeprom_write_protect, eeprom_write_protect_show,
		     eeprom_write_protect_store);

/* SFP28 registers */

#define STRING_NAME_SIZE (25)
#define NUM_SFP_REGS (8)

enum {
	SFP_PRESENT_IDX = 0,
	SFP_TX_DISABLE_IDX,
	SFP_RX_LOS_IDX,
	SFP_TX_FAULT_IDX,
};

struct sfp_info {
	char	 name[STRING_NAME_SIZE];
	u8	 index;
	u8	 active_low;
	uint32_t regs[NUM_SFP_REGS];
};

struct shift_mask {
	int shift;
	uint8_t mask;
};

static struct shift_mask sfp_shift_mask[] = {
	{ .shift = 0,  .mask = 0xff },
	{ .shift = 8,  .mask = 0xff },
	{ .shift = 16, .mask = 0x03 },
	{ .shift = 18, .mask = 0xff },
	{ .shift = 26, .mask = 0xff },
	{ .shift = 34, .mask = 0x03 },
	{ .shift = 36, .mask = 0xff },
	{ .shift = 44, .mask = 0x0f },
};

static struct sfp_info sfp_regs[] = {
	{
		.name = "present",
		.index = SFP_PRESENT_IDX,
		.active_low = 1,
		.regs = {CPLD_SW2_REG_SFP_1_8_ABS,
			 CPLD_SW2_REG_SFP_9_16_ABS,
			 CPLD_SW2_REG_SFP_17_18_ABS,
			 CPLD_SW3_REG_SFP_19_26_ABS,
			 CPLD_SW3_REG_SFP_27_34_ABS,
			 CPLD_SW3_REG_SFP_35_36_ABS,
			 CPLD_SW4_REG_SFP_37_44_ABS,
			 CPLD_SW4_REG_SFP_45_48_ABS,
			},
	},
	{
		.name = "tx_disable",
		.index = SFP_TX_DISABLE_IDX,
		.active_low = 0,
		.regs = {CPLD_SW2_REG_SFP_1_8_TX_DISABLE,
			 CPLD_SW2_REG_SFP_9_16_TX_DISABLE,
			 CPLD_SW2_REG_SFP_17_18_TX_DISABLE,
			 CPLD_SW3_REG_SFP_19_26_TX_DISABLE,
			 CPLD_SW3_REG_SFP_27_34_TX_DISABLE,
			 CPLD_SW3_REG_SFP_35_36_TX_DISABLE,
			 CPLD_SW4_REG_SFP_37_44_TX_DISABLE,
			 CPLD_SW4_REG_SFP_45_48_TX_DISABLE,
			},
	},
	{
		.name = "rx_los",
		.index = SFP_RX_LOS_IDX,
		.active_low = 1,
		.regs = {CPLD_SW2_REG_SFP_1_8_RXLOS_L,
			 CPLD_SW2_REG_SFP_9_16_RXLOS_L,
			 CPLD_SW2_REG_SFP_17_18_RXLOS_L,
			 CPLD_SW3_REG_SFP_19_26_RXLOS_L,
			 CPLD_SW3_REG_SFP_27_34_RXLOS_L,
			 CPLD_SW3_REG_SFP_35_36_RXLOS_L,
			 CPLD_SW4_REG_SFP_37_44_RXLOS_L,
			 CPLD_SW4_REG_SFP_45_48_RXLOS_L,
			},
	},
	{
		.name = "tx_fault",
		.index = SFP_TX_FAULT_IDX,
		.active_low = 0,
		.regs = {CPLD_SW2_REG_SFP_1_8_TX_FAULT,
			 CPLD_SW2_REG_SFP_9_16_TX_FAULT,
			 CPLD_SW2_REG_SFP_17_18_TX_FAULT,
			 CPLD_SW3_REG_SFP_19_26_TX_FAULT,
			 CPLD_SW3_REG_SFP_27_34_TX_FAULT,
			 CPLD_SW3_REG_SFP_35_36_TX_FAULT,
			 CPLD_SW4_REG_SFP_37_44_TX_FAULT,
			 CPLD_SW4_REG_SFP_45_48_TX_FAULT,
			},
	},
};

static ssize_t sfp_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	uint64_t data;
	int i;
	uint8_t	read_val;
	struct sfp_info *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &sfp_regs[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	data = 0;
	for (i = 0; i < NUM_SFP_REGS; i++) {
		read_val = cpld_rd(target->regs[i]);
		if (target->active_low)
			read_val = ~read_val;
		read_val &= sfp_shift_mask[i].mask;
		data |= ((uint64_t)read_val << sfp_shift_mask[i].shift);
	}

	return sprintf(buf, "0x%012llx\n", data & 0xffffffffffff);
}

static ssize_t sfp_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	int i;
	uint64_t val;
	uint8_t	write_val;
	struct sfp_info *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &sfp_regs[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	if (kstrtou64(buf, 0, &val))
		return -EINVAL;

	if (target->active_low)
		val = ~val;

	for (i = 0; i < NUM_SFP_REGS; i++) {
		write_val = (val >> sfp_shift_mask[i].shift) & 0xff;
		cpld_wr(target->regs[i], write_val);
	}

	return count;
}

static SENSOR_DEVICE_ATTR_RO(sfp28_48_1_present,    sfp_show,
			     SFP_PRESENT_IDX);
static SENSOR_DEVICE_ATTR_RW(sfp28_48_1_tx_disable, sfp_show, sfp_store,
			     SFP_TX_DISABLE_IDX);
static SENSOR_DEVICE_ATTR_RO(sfp28_48_1_rx_los,     sfp_show,
			     SFP_RX_LOS_IDX);
static SENSOR_DEVICE_ATTR_RO(sfp28_48_1_tx_fault,   sfp_show,
			     SFP_TX_FAULT_IDX);

/* QSFP28 registers */

enum {
	QSFP_PRESENT_IDX = 0,
	QSFP_RESET_IDX,
	QSFP_LPMODE_IDX,
};

struct qsfp_info {
	char name[STRING_NAME_SIZE];
	u8 index;
	u8 active_low;
	uint32_t reg;
};

static struct qsfp_info qsfp_regs[] = {
	{
		.name = "present",
		.index = QSFP_PRESENT_IDX,
		.active_low = 1,
		.reg = CPLD_SW1_REG_ZQSFP_1_6_ABS,
	},
	{
		.name = "reset",
		.index = QSFP_RESET_IDX,
		.active_low = 1,
		.reg = CPLD_SW1_REG_ZQSFP_1_6_RESET,
	},
	{
		.name = "lpmode",
		.index = QSFP_LPMODE_IDX,
		.active_low = 0,
		.reg = CPLD_SW1_REG_ZQSFP_1_6_LPMOD,
	},
};

static ssize_t qsfp_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	s32 data;
	struct qsfp_info *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &qsfp_regs[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	data = cpld_rd(target->reg);
	if (data < 0)
		return data;

	if (target->active_low)
		data = ~data;

	return sprintf(buf, "0x%02x\n", (u8)data);
}

static ssize_t qsfp_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	int retval;
	unsigned int val;
	struct qsfp_info *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &qsfp_regs[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	retval = kstrtouint(buf, 0, &val);
	if (retval != 0)
		return retval;

	if (target->active_low)
		val = ~val;

	cpld_wr(target->reg, (u8)val);
	return count;
}

static SENSOR_DEVICE_ATTR_RO(qsfp28_54_49_present, qsfp_show,
			     QSFP_PRESENT_IDX);
static SENSOR_DEVICE_ATTR_RW(qsfp28_54_49_reset,   qsfp_show, qsfp_store,
			     QSFP_RESET_IDX);
static SENSOR_DEVICE_ATTR_RW(qsfp28_54_49_lpmode,  qsfp_show, qsfp_store,
			     QSFP_LPMODE_IDX);

/* sysfs registration */

static struct attribute *cel_qstone_cpld_attrs[] = {
	&dev_attr_cpld0_version.attr,
	&dev_attr_cpld1_version.attr,
	&dev_attr_cpld2_version.attr,
	&dev_attr_cpld3_version.attr,
	&dev_attr_cpld4_version.attr,
	&dev_attr_eeprom_write_protect.attr,
	&sensor_dev_attr_sfp28_48_1_present.dev_attr.attr,
	&sensor_dev_attr_sfp28_48_1_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp28_48_1_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp28_48_1_tx_fault.dev_attr.attr,
	&sensor_dev_attr_qsfp28_54_49_present.dev_attr.attr,
	&sensor_dev_attr_qsfp28_54_49_reset.dev_attr.attr,
	&sensor_dev_attr_qsfp28_54_49_lpmode.dev_attr.attr,
	NULL,
};

static struct attribute_group cel_qstone_cpld_attr_group = {
	.attrs = cel_qstone_cpld_attrs,
};

/* driver interface */

static int cel_qstone_cpld_probe(struct platform_device *dev)
{
	int ret = 0;

	cel_qstone_cpld_regs = ioport_map(CPLD_IO_BASE, CPLD_IO_SIZE);
	if (!cel_qstone_cpld_regs) {
		pr_err("cpld: unabled to map iomem\n");
		ret = -ENODEV;
		goto err_exit;
	}

	if (!cpld_sanity_check()) {
		pr_err("cpld: could not validate CPLD registers.\n");
		ret = -EIO;
		goto err_exit;
	}

	ret = sysfs_create_group(&dev->dev.kobj, &cel_qstone_cpld_attr_group);
	if (ret) {
		pr_err("cpld: sysfs_create_group failed for cpld driver");
		goto err_unmap;
	}
	cpld_wr(CPLD_BASE_REG_I2C_CTL,
		cpld_rd(CPLD_BASE_REG_I2C_CTL) | CPLDB_I2C_SEL_L);
	return ret;

err_unmap:
	iounmap(cel_qstone_cpld_regs);

err_exit:
	return ret;
}

/*------------------------------------------------------------------------------
 *
 * cpld driver interface
 *
 */
static struct platform_device *cel_qstone_cpld_device;

static int cel_qstone_cpld_remove(struct platform_device *ofdev)
{
	struct kobject *kobj = &ofdev->dev.kobj;

	iounmap(cel_qstone_cpld_regs);
	sysfs_remove_group(kobj, &cel_qstone_cpld_attr_group);

	platform_set_drvdata(ofdev, NULL);
	dev_info(&ofdev->dev, "removed\n");
	return 0;
}

static struct platform_driver cel_qstone_cpld_driver = {
	.probe = cel_qstone_cpld_probe,
	.remove = cel_qstone_cpld_remove,
	.driver = {
		.name  = CPLD_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int cel_qstone_cpld_init(void)
{
	int ret;

	ret = platform_driver_register(&cel_qstone_cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for cpld device\n");
		goto err_exit;
	}

	cel_qstone_cpld_device = platform_device_alloc(CPLD_DRIVER_NAME, 0);
	if (!cel_qstone_cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device\n");
		ret = -ENOMEM;
		goto err_driver;
	}

	ret = platform_device_add(cel_qstone_cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_device;
	}
	return 0;

err_device:
	platform_device_unregister(cel_qstone_cpld_device);

err_driver:
	platform_driver_unregister(&cel_qstone_cpld_driver);

err_exit:
	return ret;
}

static int cel_qstone_platform_probe(struct platform_device *dev)
{
	return 0;
}

static int cel_qstone_platform_remove(struct platform_device *dev)
{
	return 0;
}

static int cel_qstone_i2c_init(void)
{
	int I801_bus;
	int i;
	int ret;
	struct i2c_client *client;

	I801_bus = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
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

		client = cumulus_i2c_add_client(i2c_qstone_devices[i].bus,
					&i2c_qstone_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		cel_qstone_clients_list[i] = client;
	}
	return 0;

err_exit:
	return ret;
}

static int __init cel_qstone_platform_init(void)
{
	int ret;

	ret = cel_qstone_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		goto err_exit;
	}

	ret = cel_qstone_cpld_init();
	if (ret) {
		pr_err("Initializing CPLD subsystem failed\n");
		goto err_exit2;
	}

	pr_info(PLATFORM_DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;

err_exit2:
	cel_qstone_i2c_exit();
err_exit:
	return ret;
}

static void __exit cel_qstone_platform_exit(void)
{
	cel_qstone_i2c_exit();
	platform_driver_unregister(&cel_qstone_platform_driver);
	pr_info(PLATFORM_DRIVER_NAME ": version " DRIVER_VERSION " unloaded\n");
}

static struct platform_driver cel_qstone_platform_driver = {
	.driver = {
		.name = PLATFORM_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = cel_qstone_platform_probe,
	.remove = cel_qstone_platform_remove,
};

module_init(cel_qstone_platform_init);
module_exit(cel_qstone_platform_exit);

MODULE_AUTHOR("Alan Liebthal (alanl@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica Questone Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
