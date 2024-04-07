/*
 * CPLD sysfs driver for quanta_ix8.
 *
 * Copyright (C) 2018,2019 Cumulus Networks, Inc.  All Rights Reserved
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
 */

#include <stddef.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/hwmon-sysfs.h>

#include "platform-defs.h"
#include "quanta-ix8.h"

#define DRIVER_NAME "quanta_ix8_cpld"
#define DRIVER_VERSION "1.0"

/*************************************************/
/* BEGIN CPLD Platform Driver                    */
/*                                               */
/* This driver is responsible for the sysfs intf */
/*************************************************/

#define CPLD_SET_BIT(numberX, posX)          (numberX |= (1ULL  << posX))
#define CPLD_CLEAR_BIT(numberX, posX)        (numberX &= (~(1ULL << posX)))
#define CPLD_TEST_BIT(numberX, posX)         (numberX & (1ULL << posX))

/* The order in which  Features exist in CPLD */

enum {
	_INV_IDX = -1,
	_SFP28_RX_LOS_IDX = 0,
	_SFP28_PRESENT_IDX,
	_SFP28_TX_DISABLE_IDX,
	_SFP28_TX_FAULT_IDX,
};

struct quanta_ix8_bde_platform_data {
	int idx;
};

static inline u8 swan8(u8 byte)
{
	return ((byte & 0xf0) >> 4) | ((byte & 0x0f) << 4);
}

static inline u16 swan16(u16 word)
{
	return (swan8(word >> 8) << 8) | swan8(word);
}

/*------------------------------------------------------------------------------
 *
 * Information that need to kept for each i/o device
 *
 */

struct port_status {
	char name[QUANTA_IX8_CPLD_STRING_NAME_SIZE];
	u8 index;
	u8 active_low;
	u8 num_bits;
	u8 num_reg;
	u8 reg[QUANTA_IX8_REG_MAX];
};

static struct port_status sfp28_status[] = {
	{
		.name = "rx_los",
		.index = _SFP28_RX_LOS_IDX,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX8_IO_SFP28_1_4_INFO_REG,
			 IX8_IO_SFP28_5_8_INFO_REG,
			 IX8_IO_SFP28_9_12_INFO_REG,
			 IX8_IO_SFP28_13_16_INFO_REG,
		},
	},
	{
		.name = "present",
		.index = _SFP28_PRESENT_IDX,
		.active_low = 1,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX8_IO_SFP28_1_4_INFO_REG,
			 IX8_IO_SFP28_5_8_INFO_REG,
			 IX8_IO_SFP28_9_12_INFO_REG,
			 IX8_IO_SFP28_13_16_INFO_REG,
		},
	},
	{
		.name = "tx_disable",
		.index = _SFP28_TX_DISABLE_IDX,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX8_IO_SFP28_1_4_INFO_REG,
			 IX8_IO_SFP28_5_8_INFO_REG,
			 IX8_IO_SFP28_9_12_INFO_REG,
			 IX8_IO_SFP28_13_16_INFO_REG,
		},
	},
	{
		.name = "tx_fault",
		.index = _SFP28_TX_FAULT_IDX,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX8_IO_SFP28_1_4_INFO_REG,
			 IX8_IO_SFP28_5_8_INFO_REG,
			 IX8_IO_SFP28_9_12_INFO_REG,
			 IX8_IO_SFP28_13_16_INFO_REG,
		},
	},
};

/**************************************************************************
 *
 * CPLD I/O
 *
 */

/*
 * cpld_port_reg_word_read - Read a 16-bit CPLD port status register.
 * Note, these registers have their nibbles swapped.
 *
 * Read word register reg into *val.
 * Return 0 or -errno on failure.
 */
static int cpld_port_reg_word_read(struct i2c_client *client,
				   u32 reg,
				   u16 *val)
{
	int ret;

	ret = i2c_smbus_read_word_data(client, reg);
	if (ret < 0) {
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       client->addr, reg);
		return ret;
	}

	*val = swan16(ret);
	return 0;
}

/*
 * cpld_port_reg_word_write - Write a 16-bit CPLD port status register.
 * Note, these registers have their nibbles swapped.
 *
 * Write val to word register reg.
 * Return 0 or -errno on failure.
 */
static int cpld_port_reg_word_write(struct i2c_client *client,
				    u32 reg,
				    u16 val)
{
	int ret;

	ret = i2c_smbus_write_word_data(client, reg, swan16(val));
	if (ret < 0) {
		pr_err("I2C write error - addr: 0x%02X, offset: 0x%02X, val: 0x%02X",
		       client->addr, reg, val);
		return ret;
	}
	return 0;
}

/*
 * cpld_port_reg_byte_read - Read an 8-bit CPLD port status register.
 *
 * Read byte register reg into *val.
 * Return 0 or -errno on failure.
 */
static int cpld_port_reg_byte_read(struct i2c_client *client,
				   u32 reg,
				   u8 *val)
{
	int ret;

	ret = i2c_smbus_read_word_data(client, reg);
	if (ret < 0) {
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       client->addr, reg);
		return ret;
	}

	*val = ret;
	return 0;
}

/*
 * cpld_port_reg_byte_write - Write an 8-bit CPLD port status register.
 *
 * Write val to byte register reg.
 * Return 0 or -errno on failure.
 */
static int cpld_port_reg_byte_write(struct i2c_client *client,
				    u32 reg,
				    u8 val)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret < 0) {
		pr_err("I2C write error - addr: 0x%02X, offset: 0x%02X, val: 0x%02X",
		       client->addr, reg, val);
		return ret;
	}

	return 0;
}

/*------------------------------------------------------------------------------
 *
 * QSFP28 I/O show/store methods
 *
 */

static ssize_t port_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int i, retval, num_bits;
	u16 reg_val;
	u64 val = 0, cpld_val;
	u64 shift_val;
	struct port_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;
	int num_platform_status;
	int sindex;

	num_platform_status = ARRAY_SIZE(sfp28_status);

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &sfp28_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	/*
	 * Read the number of registers
	 */
	for (i = 0; i < target->num_reg; i++) {
		shift_val = 0;
		retval = cpld_port_reg_word_read(client, target->reg[i],
						 &reg_val);
		if (retval < 0)
			return retval;
		shift_val = (uint16_t)reg_val;
		val |= ((shift_val) << (i * target->num_bits));
	}

	num_bits = target->num_reg * target->num_bits;
	if (target->index != _INV_IDX) {
		cpld_val = val;
		val = 0;
		/*
		 * Logic to find out the number of bits per status
		 */
		num_bits = num_bits / num_platform_status;
		for (i = 0; i < num_bits ; i++) {
			sindex = (num_platform_status * i) + target->index;
			if (CPLD_TEST_BIT(cpld_val, sindex))
				CPLD_SET_BIT(val, i);
		}
	}

	if (target->active_low)
		val = ~val;

	val = val & GENMASK_ULL(num_bits - 1, 0);

	return sprintf(buf, "0x%04llx\n", val);
}

static ssize_t port_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	int i, j, retval, num_bits;
	u16 reg_val, new_val, val16;
	u64 val;
	struct port_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;
	int num_platform_status;
	int sindex, num_sbits, pindex;

	retval = kstrtou64(buf, 0, &val);
	if (retval != 0)
		return retval;

	num_platform_status = ARRAY_SIZE(sfp28_status);

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &sfp28_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	num_bits = target->num_reg * target->num_bits;

	if (target->index != _INV_IDX)
		num_bits = num_bits / num_platform_status;

	if (target->active_low)
		val = ~val;

	val = val & GENMASK_ULL(num_bits - 1, 0);

	/*
	 * Read the number of registers
	 */
	for (i = 0; i < target->num_reg; i++) {
		retval = cpld_port_reg_word_read(client, target->reg[i],
						 &reg_val);
		if (retval < 0) {
			/* Error out */
			return retval;
		}

		new_val = reg_val;

		if (target->index == _INV_IDX) {
			val16 = (val >> (i * target->num_bits)) &
					GENMASK_ULL(target->num_bits - 1, 0);
			if (reg_val != val16)
				new_val = val16;
		} else {
			/*
			 * Number of status bits per register
			 * Ex: Number of QSFP present bits in
			 * specific register
			 */
			num_sbits = target->num_bits / num_platform_status;

			for (j = 0; j < num_sbits; j++) {
				/*
				 * index of particular port in sysfs value
				 * Ex: Index in qsfp1_present
				 */
				pindex = (i * num_sbits) + j;
				/*
				 * index of particular status in cpld register
				 * value
				 * Ex: Index of Index in qsfp1_present
				 */
				/*
				 * I/O status of particular port in  cpld
				 * register
				 */
				sindex = (j * num_sbits) + target->index;

				if (CPLD_TEST_BIT(val, pindex)) {
					if (!CPLD_TEST_BIT(reg_val, sindex))
						CPLD_SET_BIT(new_val, sindex);
				} else {
					if (CPLD_TEST_BIT(reg_val, sindex))
						CPLD_CLEAR_BIT(new_val, sindex);
				}
			}
		}
		if (new_val != reg_val) {
			retval = cpld_port_reg_word_write(client,
							  target->reg[i],
							  new_val);
			if (retval)
				return retval;
		}
	}

	return count;
}

static SENSOR_DEVICE_ATTR_RO(sfp28_1_16_rx_los,      port_show,             0);
static SENSOR_DEVICE_ATTR_RO(sfp28_1_16_present,     port_show,             1);
static SENSOR_DEVICE_ATTR_RW(sfp28_1_16_tx_disable,  port_show, port_store, 2);
static SENSOR_DEVICE_ATTR_RO(sfp28_1_16_tx_fault,    port_show,             3);
static SENSOR_DEVICE_ATTR_RO(sfp28_17_32_rx_los,     port_show,             0);
static SENSOR_DEVICE_ATTR_RO(sfp28_17_32_present,    port_show,             1);
static SENSOR_DEVICE_ATTR_RW(sfp28_17_32_tx_disable, port_show, port_store, 2);
static SENSOR_DEVICE_ATTR_RO(sfp28_17_32_tx_fault,   port_show,             3);
static SENSOR_DEVICE_ATTR_RO(sfp28_33_48_rx_los,     port_show,             0);
static SENSOR_DEVICE_ATTR_RO(sfp28_33_48_present,    port_show,             1);
static SENSOR_DEVICE_ATTR_RW(sfp28_33_48_tx_disable, port_show, port_store, 2);
static SENSOR_DEVICE_ATTR_RO(sfp28_33_48_tx_fault,   port_show,             3);

/*------------------------------------------------------------------------------
 *
 * LED show/store methods
 *
 */

static ssize_t led_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int retval;
	u8 val = 0;

	retval = cpld_port_reg_byte_read(client, IX8_LED_DECODER_REG, &val);

	if (retval < 0)
		return retval;

	return sprintf(buf, "0x%02x\n", (uint8_t)val);
}

static ssize_t led_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);

	int retval;
	u8 val;

	retval = kstrtou8(buf, 0, &val);
	if (retval != 0)
		return retval;

	retval = cpld_port_reg_byte_write(client, IX8_LED_DECODER_REG, val);
	if (retval < 0)
		return retval;

	return count;
}

static SENSOR_DEVICE_ATTR_RW(port_1_26_led_decoder,  led_show, led_store, 0);
static SENSOR_DEVICE_ATTR_RW(port_27_56_led_decoder, led_show, led_store, 1);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */

static struct attribute *sfp28_1_16_attrs[] = {
	&sensor_dev_attr_sfp28_1_16_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp28_1_16_present.dev_attr.attr,
	&sensor_dev_attr_sfp28_1_16_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp28_1_16_tx_fault.dev_attr.attr,
	NULL,
};

static struct attribute *sfp28_17_32_attrs[] = {
	&sensor_dev_attr_sfp28_17_32_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp28_17_32_present.dev_attr.attr,
	&sensor_dev_attr_sfp28_17_32_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp28_17_32_tx_fault.dev_attr.attr,
	NULL,
};

static struct attribute *sfp28_33_48_attrs[] = {
	&sensor_dev_attr_sfp28_33_48_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp28_33_48_present.dev_attr.attr,
	&sensor_dev_attr_sfp28_33_48_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp28_33_48_tx_fault.dev_attr.attr,
	NULL,
};

static struct attribute *port_1_26_led_decoder_attrs[] = {
	&sensor_dev_attr_port_1_26_led_decoder.dev_attr.attr,
	NULL,
};

static struct attribute *port_27_56_led_decoder_attrs[] = {
	&sensor_dev_attr_port_27_56_led_decoder.dev_attr.attr,
	NULL,
};

static struct attribute_group sfp28_1_16_attr_group = {
	.attrs = sfp28_1_16_attrs,
};

static struct attribute_group sfp28_17_32_attr_group = {
	.attrs = sfp28_17_32_attrs,
};

static struct attribute_group sfp28_33_48_attr_group = {
	.attrs = sfp28_33_48_attrs,
};

static struct attribute_group port_1_26_led_decoder_attr_group = {
	.attrs = port_1_26_led_decoder_attrs,
};

static struct attribute_group port_27_56_led_decoder_attr_group = {
	.attrs = port_27_56_led_decoder_attrs,
};

/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static int cpld_probe(struct i2c_client *client,
		      const struct i2c_device_id *id)

{
	struct quanta_ix8_bde_platform_data *pdata;
	int retval;
	int cpld_idx;
	struct kobject *kobj = &client->dev.kobj;

	pr_info(DRIVER_NAME " probed\n");

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_BYTE_DATA
				     | I2C_FUNC_SMBUS_WORD_DATA)) {
		dev_err(&client->dev, "smbus read byte and word data not supported.\n");
		return -ENODEV;
	}

	pdata = dev_get_platdata(&client->dev);
	cpld_idx = pdata->idx;

	/*
	 * Create sysfs nodes.
	 */
	switch (cpld_idx) {
	case IX8_IO_SFP28_1_16_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &sfp28_1_16_attr_group);
		break;
	case IX8_IO_SFP28_17_32_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &sfp28_17_32_attr_group);
		break;
	case IX8_IO_SFP28_33_48_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &sfp28_33_48_attr_group);
		break;
	case IX8_LED_SFP28_QSFP28_27_56_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &port_27_56_led_decoder_attr_group);
		break;
	case IX8_LED_SFP28_1_26_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &port_1_26_led_decoder_attr_group);
		break;
	default:
		dev_err(&client->dev, "%s unsupported idx (%d)\n",
			DRIVER_NAME, cpld_idx);
		retval = -EIO;
		break;
	}

	if (retval) {
		dev_err(&client->dev, "sysfs_create_group for cpld %d failed\n",
			cpld_idx);
		goto err;
	}

	/*
	 * All clear from this point on
	 */
	dev_info(&client->dev,
		 "device created %d,\n", cpld_idx);

#ifdef DEBUG
	for (i = 0; i < CPLD_NREGS; i++) {
		ret = i2c_smbus_read_byte_data(client, i);
		dev_dbg(&client->dev,
			ret < 0 ? "CPLD[%d] reg[%d] read error %d\n" :
			"CPLD[%d] reg[%d] %#04x\n", cpld_idx, i, ret);
	}
#endif

	return 0;
err:
	return retval;
}

static int cpld_remove(struct i2c_client *client)
{
	struct kobject *kobj = &client->dev.kobj;

	sysfs_remove_group(kobj, &sfp28_1_16_attr_group);
	sysfs_remove_group(kobj, &sfp28_17_32_attr_group);
	sysfs_remove_group(kobj, &sfp28_33_48_attr_group);
	sysfs_remove_group(kobj, &port_1_26_led_decoder_attr_group);
	sysfs_remove_group(kobj, &port_27_56_led_decoder_attr_group);

	dev_info(&client->dev, "device removed\n");
	return 0;
}

static const struct i2c_device_id cpld_i2c_ids[] = {
	{
		.name = "quanta_ix8_cpld",
	},

	{ /* end of list */ },
};
MODULE_DEVICE_TABLE(i2c, cpld_i2c_ids);

static struct i2c_driver cpld_driver = {
	.driver = {
		.name  = "quanta_ix8_cpld",
		.owner = THIS_MODULE,
	},
	.probe = cpld_probe,
	.remove = cpld_remove,
	.id_table = cpld_i2c_ids,
};

/*------------------------------------------------------------------------------
 *
 * module interface
 *
 */

static int __init quanta_ix8_cpld_init(void)
{
	int rv;

	pr_info(DRIVER_NAME " loading CPLD driver\n");
	rv = i2c_add_driver(&cpld_driver);
	if (rv) {
		pr_err(DRIVER_NAME " i2c_add_driver failed (%i)\n",
		       rv);
	}
	return rv;
}

static void __exit quanta_ix8_cpld_exit(void)
{
	i2c_del_driver(&cpld_driver);
	pr_info(DRIVER_NAME " CPLD driver unloaded\n");
}

MODULE_AUTHOR("David Yen <dhyen@cumulusnetworks.com>");
MODULE_DESCRIPTION("CPLD driver for Quanta IX8 Platform");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(quanta_ix8_cpld_init);
module_exit(quanta_ix8_cpld_exit);
