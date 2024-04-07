// SPDX-License-Identifier: GPL-2.0+
/*
 * Dell EMC S5048F CPLD4 Driver
 *
 * Copyright (C) 2020 Cumulus Networks, Inc.  All Rights Reserved.
 * Author: David Yen <dhyen@cumulusnetworks.com>
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
#include <linux/hwmon-sysfs.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "dellemc-s5048f.h"

#define DRIVER_NAME    S5048F_CPLD4_NAME
#define DRIVER_VERSION "1.0"

/* bitfield accessor functions */

static int cpld_read16(struct i2c_client *client, int addr)
{
	int ret;

	/* cpld uses 16-bit addressing */
	ret = i2c_smbus_write_byte_data(client, 0x0, addr);
	if (ret < 0) {
		dev_err(&client->dev,
			"16-bit addr write failed for addr: 0x%x\n", addr);
		return ret;
	}
	ret = i2c_smbus_read_byte(client);
	if (ret < 0)
		dev_err(&client->dev, "read failed for addr: 0x%x\n", addr);
	return ret;
}

static int cpld_write16(struct i2c_client *client, int addr, u8 val)
{
	int ret;

	/* cpld uses 16-bit addressing */
	ret = i2c_smbus_write_word_data(client, 0x0, ((val << 8) | addr));
	if (ret < 0)
		dev_err(&client->dev, "write failed for addr: 0x%x\n", addr);

	return ret;
}

static int cpld_read_reg(struct device *dev,
			 int reg,
			 int nregs,
			 u32 *val)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);

	ret = cpld_read16(client, reg);
	if (ret < 0) {
		dev_err(dev, "CPLD4 read error - reg: 0x%02x\n", reg);
		return -EINVAL;
	}
	*val = ret;
	return 0;
}

static int cpld_write_reg(struct device *dev,
			  int reg,
			  int nregs,
			  u32 val)
{
	int ret;
	struct i2c_client *client = to_i2c_client(dev);

	ret = cpld_write16(client, reg, val);
	if (ret < 0)
		dev_err(dev, "CPLD4 write error - reg: 0x%02x\n", reg);

	return ret;
}

/* CPLD register bitfields with enum-like values */

static const char * const board_type_values[] = {
	"Reserved",	/* 0 */
	"Z9100",	/* 1 */
	"S6100",	/* 2 */
	"Z9100-SFP28",	/* 3 */
	"Z9100-Cavium", /* 4 */
	"S5048F",	/* 5 */
	"Reserved",	/* 6 */
	"Reserved",	/* 7 */
};

static const char * const cpld_id_values[] = {
	"Reserved", /* 0 */
	"CPLD1",    /* 1 */
	"CPLD2",    /* 2 */
	"CPLD3",    /* 3 */
	"CPLD4",    /* 4 */
	"Reserved", /* 5 */
	"Reserved", /* 6 */
	"Reserved", /* 7 */
};

static const char * const test_amber_values[] = {
	PLATFORM_LED_OFF,		  /* 0 */
	PLATFORM_LED_AMBER_BLINKING,	  /* 1 */
	PLATFORM_LED_AMBER,		  /* 2 */
	PLATFORM_LED_AMBER_SLOW_BLINKING, /* 3 */
};

static const char * const test_green_values[] = {
	PLATFORM_LED_OFF,		  /* 0 */
	PLATFORM_LED_GREEN_BLINKING,	  /* 1 */
	PLATFORM_LED_GREEN,		  /* 2 */
	PLATFORM_LED_GREEN_SLOW_BLINKING, /* 3 */
};

static const char * const trigger_values[] = {
	"falling edge", /* 0 */
	"rising edge",	/* 1 */
	"both edges",	/* 2 */
	"low level",	/* 3 */
};

/* CPLD registers */

cpld_bf_ro(major_version, CPLD_VERSION_REG,
	   CPLD_MAJOR_VER, NULL, 0);
cpld_bf_ro(minor_version, CPLD_VERSION_REG,
	   CPLD_MINOR_VER, NULL, 0);
cpld_bf_ro(board_type, BOARD_TYPE_REG,
	   BOARD_TYPE, board_type_values, 0);
cpld_bf_rw(scratchpad, SW_SCRATCH_REG,
	   SW_SCRATCH, NULL, 0);
cpld_bf_ro(cpld_id, CPLD_ID_REG,
	   CPLD_ID, cpld_id_values, 0);
cpld_bt_rw(led_test_mode, PORT_LED_OPMOD_REG,
	   OP_MOD, NULL, 0);
cpld_bf_rw(led_test_amber, PORT_LED_TEST_REG,
	   OP_CTRL_AMBER, test_amber_values, 0);
cpld_bf_rw(led_test_green, PORT_LED_TEST_REG,
	   OP_CTRL_GREEN, test_green_values, 0);
cpld_bf_rw(sfp28_present_trig, SFP28_TRIG_MOD_REG,
	   SFP28_PRS_TRIG, trigger_values, 0);
cpld_bf_rw(sfp28_rx_los_trig, SFP28_TRIG_MOD_REG,
	   SFP28_RXLOS_TRIG, trigger_values, 0);
cpld_bt_ro(sfp28_present_combine, SFP28_COMBINE_REG,
	   SFP28_ABS_COMBINE, NULL, 0);
cpld_bt_ro(sfp28_rx_los_combine, SFP28_COMBINE_REG,
	   SFP28_RXLOS_COMBINE, NULL, 0);
cpld_bf_rw(qsfp28_present_trig, QSFP28_TRIG_MOD_REG,
	   QSFP28_PRS_TRIG, trigger_values, 0);
cpld_bf_rw(qsfp28_int_trig, QSFP28_TRIG_MOD_REG,
	   QSFP28_INT_TRIG, trigger_values, 0);
cpld_bt_ro(qsfp28_present_combine, QSFP28_COMBINE_REG,
	   QSFP28_PRES_COMBINE, NULL, 0);
cpld_bt_ro(qsfp28_int_combine, QSFP28_COMBINE_REG,
	   QSFP28_INT_COMBINE, NULL, 0);

/* special case for sfp registers */

struct sff_info {
	int reg;
	int active_low;
};

static struct sff_info sfp_reg[] = {
	{SFP28_TXDISABLE_CTRL0_REG, 0},
	{SFP28_RS_CTRL0_REG,	    0},
	{SFP28_RXLOS_STA0_REG,	    0},
	{SFP28_TXFAULT_STA0_REG,    0},
	{SFP28_ABS_STA0_REG,	    1}, /* present is active low */
	{SFP28_RXLOS_INT0_REG,	    0},
	{SFP28_ABS_INT0_REG,	    0},
	{SFP28_RXLOS_MASK0_REG,	    0},
	{SFP28_ABS_MASK0_REG,	    0},
};

static ssize_t sfp_show(struct device *dev, struct device_attribute *dattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = to_i2c_client(dev);
	int idx = attr->index;
	int reg = sfp_reg[idx].reg;
	int i;
	s32 tmp;
	u64 val = 0;

	for (i = 0; i < NUM_CPLD4_SFP28_REGS; i++) {
		tmp = cpld_read16(client, (reg + i));
		if (tmp < 0) {
			dev_err(dev, "CPLD4 read error - reg: 0x%02X\n",
				reg);
			return -EINVAL;
		}
		if (sfp_reg[idx].active_low)
			tmp = ~tmp;
		val |= ((u64)tmp & 0xff) << i * 8;
	}
	return sprintf(buf, "0x%03llx\n", val & 0x1ff);
}

static ssize_t sfp_store(struct device *dev, struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = to_i2c_client(dev);
	int idx = attr->index;
	int reg = sfp_reg[idx].reg;
	int ret;
	u64 val = 0;
	int i;
	u8 tmp;

	ret = kstrtou64(buf, 0, &val);
	if (ret < 0)
		return ret;

	for (i = 0; i < NUM_CPLD4_SFP28_REGS; i++) {
		tmp = val >> (i * 8) & 0xff;
		if (sfp_reg[idx].active_low)
			tmp = ~tmp;
		ret = cpld_write16(client, (reg + i), tmp);
		if (ret) {
			dev_err(dev, "CPLD4 write error - reg: 0x%02X\n",
				reg);
			return -EINVAL;
		}
	}
	return count;
}

static SENSOR_DEVICE_ATTR_RW(sfp28_48_38_tx_disable,   sfp_show, sfp_store, 0);
static SENSOR_DEVICE_ATTR_RW(sfp28_48_38_rs_ctrl,      sfp_show, sfp_store, 1);
static SENSOR_DEVICE_ATTR_RO(sfp28_48_38_rx_los,       sfp_show, 2);
static SENSOR_DEVICE_ATTR_RO(sfp28_48_38_tx_fault,     sfp_show, 3);
static SENSOR_DEVICE_ATTR_RO(sfp28_48_38_present,      sfp_show, 4);
static SENSOR_DEVICE_ATTR_RO(sfp28_48_38_rx_los_int,   sfp_show, 5);
static SENSOR_DEVICE_ATTR_RO(sfp28_48_38_present_int,  sfp_show, 6);
static SENSOR_DEVICE_ATTR_RW(sfp28_48_38_rx_los_mask,  sfp_show, sfp_store, 7);
static SENSOR_DEVICE_ATTR_RW(sfp28_48_38_present_mask, sfp_show, sfp_store, 8);

static struct sff_info qsfp_reg[] = {
	{QSFP28_RESET_CTRL0_REG, 1},
	{QSFP28_LPMOD_CTRL0_REG, 0},
	{QSFP28_INT_STA0_REG,	 0},
	{QSFP28_ABS_STA0_REG,	 1}, /* present is active low */
	{QSFP28_INT_INT0_REG,	 0},
	{QSFP28_ABS_INT0_REG,	 0},
	{QSFP28_INT_MASK0_REG,	 0},
	{QSFP28_ABS_MASK0_REG,	 0},
};

static ssize_t qsfp_show(struct device *dev, struct device_attribute *dattr,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = to_i2c_client(dev);
	int idx = attr->index;
	int reg = qsfp_reg[idx].reg;
	int i;
	s32 tmp;
	u64 val = 0;

	for (i = 0; i < NUM_CPLD4_QSFP28_REGS; i++) {
		tmp = cpld_read16(client, (reg + i));
		if (tmp < 0) {
			dev_err(dev, "CPLD4 read error - reg: 0x%02X\n",
				reg);
			return -EINVAL;
		}
		if (qsfp_reg[idx].active_low)
			tmp = ~tmp;
		val |= ((u64)tmp & 0xff) << i * 8;
	}
	return sprintf(buf, "0x%02llx\n", val & 0x3f);
}

static ssize_t qsfp_store(struct device *dev, struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = to_i2c_client(dev);
	int idx = attr->index;
	int reg = qsfp_reg[idx].reg;
	int ret;
	u64 val = 0;
	int i;
	u8 tmp;

	ret = kstrtou64(buf, 0, &val);
	if (ret < 0)
		return ret;

	for (i = 0; i < NUM_CPLD4_QSFP28_REGS; i++) {
		tmp = val >> (i * 8) & 0xff;
		if (qsfp_reg[idx].active_low)
			tmp = ~tmp;
		ret = cpld_write16(client, (reg + i), tmp);
		if (ret) {
			dev_err(dev, "CPLD4 write error - reg: 0x%02X\n",
				reg);
			return -EINVAL;
		}
	}
	return count;
}

static SENSOR_DEVICE_ATTR_RW(qsfp28_54_49_reset,	  qsfp_show,
			     qsfp_store, 0);
static SENSOR_DEVICE_ATTR_RW(qsfp28_54_49_lpmode,	  qsfp_show,
			     qsfp_store, 1);
static SENSOR_DEVICE_ATTR_RO(qsfp28_54_49_interrupt,	  qsfp_show, 2);
static SENSOR_DEVICE_ATTR_RO(qsfp28_54_49_present,	  qsfp_show, 3);
static SENSOR_DEVICE_ATTR_RO(qsfp28_54_49_interrupt_int,  qsfp_show, 4);
static SENSOR_DEVICE_ATTR_RO(qsfp28_54_49_present_int,	  qsfp_show, 5);
static SENSOR_DEVICE_ATTR_RW(qsfp28_54_49_interrupt_mask, qsfp_show,
			     qsfp_store, 6);
static SENSOR_DEVICE_ATTR_RW(qsfp28_54_49_present_mask,	  qsfp_show,
			     qsfp_store, 7);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_major_version.attr,
	&cpld_minor_version.attr,
	&cpld_board_type.attr,
	&cpld_scratchpad.attr,
	&cpld_cpld_id.attr,
	&cpld_led_test_mode.attr,
	&cpld_led_test_amber.attr,
	&cpld_led_test_green.attr,
	&cpld_sfp28_present_trig.attr,
	&cpld_sfp28_rx_los_trig.attr,
	&cpld_sfp28_present_combine.attr,
	&cpld_sfp28_rx_los_combine.attr,
	&cpld_qsfp28_present_trig.attr,
	&cpld_qsfp28_int_trig.attr,
	&cpld_qsfp28_present_combine.attr,
	&cpld_qsfp28_int_combine.attr,

	&sensor_dev_attr_sfp28_48_38_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp28_48_38_rs_ctrl.dev_attr.attr,
	&sensor_dev_attr_sfp28_48_38_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp28_48_38_tx_fault.dev_attr.attr,
	&sensor_dev_attr_sfp28_48_38_present.dev_attr.attr,
	&sensor_dev_attr_sfp28_48_38_rx_los_int.dev_attr.attr,
	&sensor_dev_attr_sfp28_48_38_present_int.dev_attr.attr,
	&sensor_dev_attr_sfp28_48_38_rx_los_mask.dev_attr.attr,
	&sensor_dev_attr_sfp28_48_38_present_mask.dev_attr.attr,

	&sensor_dev_attr_qsfp28_54_49_reset.dev_attr.attr,
	&sensor_dev_attr_qsfp28_54_49_lpmode.dev_attr.attr,
	&sensor_dev_attr_qsfp28_54_49_interrupt.dev_attr.attr,
	&sensor_dev_attr_qsfp28_54_49_present.dev_attr.attr,
	&sensor_dev_attr_qsfp28_54_49_interrupt_int.dev_attr.attr,
	&sensor_dev_attr_qsfp28_54_49_present_int.dev_attr.attr,
	&sensor_dev_attr_qsfp28_54_49_interrupt_mask.dev_attr.attr,
	&sensor_dev_attr_qsfp28_54_49_present_mask.dev_attr.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

static int cpld_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	s32 temp;
	int ret = 0;

	/* make sure the adpater supports i2c smbus reads */
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(dev, "adapter does not support I2C_FUNC_SMBUS_BYTE_DATA\n");
		ret = -EINVAL;
		goto err;
	}

	/* probe the hardware by reading the CPLD_ID register */
	temp = cpld_read16(client, CPLD_ID_REG);
	if (temp < 0) {
		dev_err(dev, "read CPLD_ID register error: %d\n",
			temp);
		ret = temp;
		goto err;
	} else if (temp != 4) {
		dev_err(dev, "detected incorrect CPLD ID value: %d\n",
			temp);
		ret = -EINVAL;
		goto err;
	}

	/* read the CPLD version register */
	temp = cpld_read16(client, CPLD_VERSION_REG);
	if (temp < 0) {
		dev_err(dev, "read CPLD version register error: %d\n",
			temp);
		ret = temp;
		goto err;
	}

	/* create sysfs node */
	ret = sysfs_create_group(&dev->kobj, &cpld_attr_group);
	if (ret) {
		dev_err(dev, "failed to create sysfs group for CPLD device\n");
		goto err;
	}

	/* all clear */
	dev_info(dev, "device probed, CPLD4 rev: %lu.%lu\n",
		 GET_FIELD(temp, CPLD_MAJOR_VER),
		 GET_FIELD(temp, CPLD_MINOR_VER));

err:
	return ret;
}

static int cpld_remove(struct i2c_client *client)
{
	struct device *dev = &client->dev;

	sysfs_remove_group(&dev->kobj, &cpld_attr_group);
	return 0;
}

static const struct i2c_device_id cpld_id[] = {
	{ DRIVER_NAME, 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, cpld_id);

static struct i2c_driver cpld_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe_new = cpld_probe,
	.remove = cpld_remove,
	.id_table = cpld_id,
};

/* module init/exit */

static int __init cpld_init(void)
{
	int ret;

	ret = i2c_add_driver(&cpld_driver);
	if (ret) {
		pr_err(DRIVER_NAME ": driver failed to load\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;
}

static void __exit cpld_exit(void)
{
	i2c_del_driver(&cpld_driver);
	pr_info(DRIVER_NAME ": unloaded\n");
}

module_init(cpld_init);
module_exit(cpld_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC S5048F CPLD4 Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

