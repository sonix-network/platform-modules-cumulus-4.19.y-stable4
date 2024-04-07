/*
 * QSFP+ I/O CPLD sysfs driver for quanta_ly6_rangeley.
 *
 * Copyright (C) 2014 Cumulus Networks, Inc.
 * Author: Puneet Shenoy <puneet@cumulusnetworks.com>
 * Based on the P2020 driver written by Vidya <vidya@cumulusnetworks.com>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
#include <asm/io.h>

#include "platform-defs.h"
#include "quanta-ly6-cpld.h"

static const char driver_name[] = "quanta_ly6_rangeley_cpld";
#define DRIVER_VERSION "1.0"

/*************************************************/
/* BEGIN CPLD Platform Driver                    */
/*                                               */
/* This driver is responsible for the sysfs intf */
/*************************************************/

#define CPLD_SET_BIT(numberX, posX)          ( numberX |= ( 0x1  << posX)  )
#define CPLD_CLEAR_BIT(numberX, posX)        ( numberX &= (~(0x1 << posX)) )
#define CPLD_TEST_BIT(numberX, posX)         ( numberX & ( 0x1 << posX) )

/* The order in which QSFP Features exist in CPLD */
enum {
	QSFP_LPMODE_IDX = 0,
	QSFP_MOD_ABS_IDX,
	QSFP_INT_IDX,
	QSFP_RESET_IDX,
};

struct quanta_ly6_rangeley_platform_data {
	int idx;
};

/*------------------------------------------------------------------------------
 *
 * Information that need to kept for each device
 *
 */

struct quanta_ly6_rangeley_cpld_data {
	char name[QUANATA_LY6_CPLD_STRING_NAME_SIZE];
};

struct qsfp_status {
	char name[QUANATA_LY6_CPLD_STRING_NAME_SIZE];
	uint8_t index;
	uint8_t active_low;
};

static struct qsfp_status cpld_qsfp_status[] = {
	{
		.name = "present",
		.index = QSFP_MOD_ABS_IDX,
		.active_low = 1,
	},
	{
		.name = "interrupt",
		.index = QSFP_INT_IDX,
		.active_low = 1,
	},
	{
		.name = "lp_mode",
		.index = QSFP_LPMODE_IDX,
	},
	{
		.name = "reset",
		.index = QSFP_RESET_IDX,
		.active_low = 1,
	},
};
static int n_qsfp_status = ARRAY_SIZE(cpld_qsfp_status);

static const uint8_t qsfp_info_offsets[] = { CPLD_QSFP1_4_INFO_REG,
					     CPLD_QSFP5_8_INFO_REG,
					     CPLD_QSFP9_12_INFO_REG,
					     CPLD_QSFP13_16_INFO_REG };

static int n_qsfp_info = ARRAY_SIZE(qsfp_info_offsets);

static int hexToInt32(const char *hex_str, uint32_t *val)
{
	char prefix[] = "0x";
	if (strncasecmp(hex_str, prefix, strlen(prefix)) == 0) {
		hex_str += strlen(prefix);
	}
	return sscanf(hex_str, "%x", val) != 1;
}

static inline uint8_t swan8(uint8_t byte)
{
	return ((byte & 0xf0) >> 4) | ((byte & 0x0f) << 4);
}

static inline uint16_t swan16(uint16_t word)
{
	return (swan8(word >> 8) << 8) | swan8(word);
}

/********************************************************************************
 *
 * CPLD I/O
 *
 */

/**
 * cpld_port_reg_read - Read an 8-bit CPLD port status register over i2c.
 * Note, these registers have their nibbles swapped.
 *
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else a data byte received from the device.
 */
static uint16_t cpld_port_reg_read(struct i2c_client *client, uint32_t reg, uint16_t *val)
{
	int retval = 0;

	retval = i2c_smbus_read_word_data(client, reg);
	if (retval < 0) {
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       client->addr, reg);
		return retval;
	}

	*val = swan16(retval);
	return retval;
}

/**
 * cpld_port_reg_write - Writes an 8-bit CPLD port status register over i2c.
 * Note, these registers have their nibbles swapped.
 *
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else zero on success.
 */

static s32 cpld_port_reg_write(struct i2c_client *client, uint32_t reg, uint16_t val)
{
	int res;

	res = i2c_smbus_write_word_data(client, reg, swan16(val));
	if (res) {
		pr_err("I2C write error - addr: 0x%02X, offset: 0x%02X, val: 0x%02X",
		       client->addr, reg, val);
	}
	return res;
}

/*------------------------------------------------------------------------------
 *
 * show/store methods
 *
 */
static s32 qsfp_read_status(struct i2c_client *client, int status, uint16_t *readval)
{
	int i = 0;
	uint16_t status_val = 0;
	uint16_t  val = 0;
	int port, pinfo_index, retval = 0;

	for (i = 0; i < n_qsfp_info; i++) {
		retval = cpld_port_reg_read(client, qsfp_info_offsets[i], &val);
		if (retval < 0 ) {
			/* Error out */
			return retval;
		} else {
			for (pinfo_index = 0; pinfo_index < 4; pinfo_index++) {
				port = (4 * i) + pinfo_index;
				if (CPLD_TEST_BIT(val, ((4 * pinfo_index) + status))) {
					CPLD_SET_BIT(status_val, port);
				}
			}
		}
	}

	*readval = status_val;
	return retval;
}

static s32 qsfp_write_status(struct i2c_client *client,
			     int status, int val)
{
	int i = 0;
	int retval = 0;
	uint16_t  cur_status = 0;
	int port, pinfo_index;

	for (i = 0; i < n_qsfp_info; i++) {
		retval = cpld_port_reg_read(client, qsfp_info_offsets[i], &cur_status);
		if (retval < 0 ) {
			return retval;
		} else {
			for (pinfo_index = 0; pinfo_index < 4; pinfo_index++) {
				port = (4 * i) + pinfo_index;
				if (CPLD_TEST_BIT(val,  port)) {
					CPLD_SET_BIT(cur_status, ((4 * pinfo_index) + status));
				} else {
					CPLD_CLEAR_BIT(cur_status, ((4 * pinfo_index) + status));
				}
			}

			retval = cpld_port_reg_write(client, qsfp_info_offsets[i], cur_status);
			if (retval) {
				return retval;
			}
		}
	}

	return retval;
}

static ssize_t qsfp_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int i, retval;
	uint16_t val;
	uint8_t name_len = 6; /* strlen("qsfpX_"); */
	struct qsfp_status *target = NULL;

	/* find the target register */
	for (i = 0; i < n_qsfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len, cpld_qsfp_status[i].name) == 0) {
			target = &cpld_qsfp_status[i];
			break;
		}
	}
	if (target == NULL)
		return -EINVAL;

	retval = qsfp_read_status(client, target->index, &val);
	if (retval < 0)
		return retval;

	if (target->active_low)
		val = ~val;

	return sprintf(buf, "0x%02x\n", val);
}

static ssize_t qsfp_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{

	int ret, i;
	uint8_t name_len = 6; /* strlen("qsfpX_"); */
	uint32_t val32;
	uint16_t val;
	struct qsfp_status *target = NULL;
	struct i2c_client *client = to_i2c_client(dev);

	if (hexToInt32(buf, &val32))
		return -EINVAL;

	val = val32 & 0xff;
	for (i = 0; i < n_qsfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len, cpld_qsfp_status[i].name) == 0) {
			target = &cpld_qsfp_status[i];
			break;
		}
	}
	if (target == NULL)
		return -EINVAL;

	if (target->active_low)
		val = ~val;

	ret = qsfp_write_status(client, target->index, val);
	if (ret < 0)
		return ret;

	return count;
}

static SYSFS_ATTR_RO(qsfp1_present, qsfp_show);
static SYSFS_ATTR_RO(qsfp1_interrupt,   qsfp_show);
static SYSFS_ATTR_RW(qsfp1_lp_mode, qsfp_show, qsfp_store);
static SYSFS_ATTR_RW(qsfp1_reset,   qsfp_show, qsfp_store);
static SYSFS_ATTR_RO(qsfp2_present, qsfp_show);
static SYSFS_ATTR_RO(qsfp2_interrupt,   qsfp_show);
static SYSFS_ATTR_RW(qsfp2_lp_mode, qsfp_show, qsfp_store);
static SYSFS_ATTR_RW(qsfp2_reset,   qsfp_show, qsfp_store);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */

static struct attribute *quanta_ly6_rangeley_cpld_1_16_attrs[] = {
	&dev_attr_qsfp1_present.attr,
	&dev_attr_qsfp1_interrupt.attr,
	&dev_attr_qsfp1_lp_mode.attr,
	&dev_attr_qsfp1_reset.attr,
	NULL,
};
static struct attribute *quanta_ly6_rangeley_cpld_17_32_attrs[] = {
	&dev_attr_qsfp2_present.attr,
	&dev_attr_qsfp2_interrupt.attr,
	&dev_attr_qsfp2_lp_mode.attr,
	&dev_attr_qsfp2_reset.attr,
	NULL,
};


static struct attribute_group quanta_ly6_rangeley_cpld_1_16_attr_group = {
	.attrs = quanta_ly6_rangeley_cpld_1_16_attrs,
};
static struct attribute_group quanta_ly6_rangeley_cpld_17_32_attr_group = {
	.attrs = quanta_ly6_rangeley_cpld_17_32_attrs,
};



/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static int quanta_ly6_rangeley_cpld_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)

{
	struct quanta_ly6_rangeley_cpld_data *data;
	struct quanta_ly6_rangeley_platform_data *pdata;
	int retval;
	int cpld_idx;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA)) {
		dev_err(&client->dev, "smbud read byte and word data not supported.\n");
		return -ENODEV;
	}

	if (!(data = kzalloc(sizeof(struct quanta_ly6_rangeley_cpld_data),
			     GFP_KERNEL))) {
		dev_err(&client->dev, "out of memory.\n");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, data);

	pdata = dev_get_platdata(&client->dev);
	cpld_idx = pdata->idx;

	if (cpld_idx == 1) {
		retval = sysfs_create_group(&client->dev.kobj, &quanta_ly6_rangeley_cpld_1_16_attr_group);
		if (retval) {
			dev_err(&client->dev, "failed to create sysfs group for cpld %d\n", cpld_idx);
		}
	} else if (cpld_idx == 2) {
		retval = sysfs_create_group(&client->dev.kobj, &quanta_ly6_rangeley_cpld_17_32_attr_group);
		if (retval) {
			dev_err(&client->dev, "failed to create sysfs group for cpld %d\n", cpld_idx);
		}
	} else {
		dev_err(&client->dev, "%s unsupported idx (%d)\n",
			driver_name, cpld_idx);
		retval = -EIO;
	}

	if (retval < 0) {
		kfree(data);
	}
	return retval;
}


static int quanta_ly6_rangeley_cpld_remove(struct i2c_client *client)
{
	struct quanta_ly6_rangeley_cpld_data *data = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &quanta_ly6_rangeley_cpld_1_16_attr_group);
	sysfs_remove_group(&client->dev.kobj, &quanta_ly6_rangeley_cpld_17_32_attr_group);

	kfree(data);

	return 0;
}

static const struct i2c_device_id quanta_ly6_rangeley_cpld_ids[] = {
        { "ly6_rangeley_cpld", 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, quanta_ly6_rangeley_cpld_ids);

static struct i2c_driver quanta_ly6_rangeley_cpld_driver = {
	.driver = {
		.name  = "ly6_rangeley_cpld",
		.owner = THIS_MODULE,
	},
	.probe = quanta_ly6_rangeley_cpld_probe,
	.remove = quanta_ly6_rangeley_cpld_remove,
	.id_table = quanta_ly6_rangeley_cpld_ids,
};

/*------------------------------------------------------------------------------
 *
 * module interface
 *
 */

static int __init quanta_ly6_rangeley_cpld_init(void)
{
	return i2c_add_driver(&quanta_ly6_rangeley_cpld_driver);
}

static void __exit quanta_ly6_rangeley_cpld_exit(void)
{
	return i2c_del_driver(&quanta_ly6_rangeley_cpld_driver);
}

MODULE_AUTHOR("Puneet Shenoy <puneet@cumulusnetworks.com>");
MODULE_DESCRIPTION("QSFP+ I/O CPLD driver for Quanta LY6 RANGELEY");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(quanta_ly6_rangeley_cpld_init);
module_exit(quanta_ly6_rangeley_cpld_exit);
