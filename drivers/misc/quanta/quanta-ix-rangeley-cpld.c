/*
 * CPLD sysfs driver for quanta_ix_rangeley.
 *
 * Copyright (C) 2016 Cumulus Networks, Inc.
 * Author: Vidya Sagar Ravipati <vidya@cumulusnetworks.com>
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

#include "platform-defs.h"
#include "quanta-ix-cpld.h"

#define DRIVER_NAME "quanta_ix_rangeley_cpld"
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
	_LPMODE_IDX,
	_MOD_ABS_IDX,
	_INT_IDX,
	_RESET_IDX,
};

enum {
	_SFP_RX_LOS_IDX = 0,
	_SFP_PRESENT_IDX,
	_SFP_TX_DISABLE_IDX,
	_SFP_TX_FAULT_IDX,
};

enum {
	MNTR_ENABLE_IDX = 0,
	MNTR_RESET_TIME_LO_IDX,
	MNTR_RESET_TIME_HI_IDX,
};

struct quanta_ix_rangeley_platform_data {
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
	char name[QUANATA_IX_CPLD_STRING_NAME_SIZE];
	u8 index;
	u8 active_low;
	/*
	 * number of bits per cpld register
	 * LED CPLD - 8  bit
	 * IO  CPLD - 16 bit
	 */
	u8 num_bits;
	u8 num_reg;
	u8 reg[QUANTA_IX_REG_MAX];
};

static struct port_status ix1_qsfp_status[] = {
	{
		.name = "present",
		.index = _MOD_ABS_IDX,
		.active_low = 1,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX1_IO_QSFP_1_4_INFO_REG,
			 IX1_IO_QSFP_5_8_INFO_REG,
			 IX1_IO_QSFP_9_12_INFO_REG,
			 IX1_IO_QSFP_13_16_INFO_REG},
	},
	{
		.name = "interrupt",
		.index = _INT_IDX,
		.active_low = 1,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX1_IO_QSFP_1_4_INFO_REG,
			 IX1_IO_QSFP_5_8_INFO_REG,
			 IX1_IO_QSFP_9_12_INFO_REG,
			 IX1_IO_QSFP_13_16_INFO_REG},
	},
	{
		.name = "lp_mode",
		.index = _LPMODE_IDX,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX1_IO_QSFP_1_4_INFO_REG,
			 IX1_IO_QSFP_5_8_INFO_REG,
			 IX1_IO_QSFP_9_12_INFO_REG,
			 IX1_IO_QSFP_13_16_INFO_REG},
	},
	{
		.name = "reset",
		.index = _RESET_IDX,
		.active_low = 1,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX1_IO_QSFP_1_4_INFO_REG,
			 IX1_IO_QSFP_5_8_INFO_REG,
			 IX1_IO_QSFP_9_12_INFO_REG,
			 IX1_IO_QSFP_13_16_INFO_REG},
	},
};

static struct port_status ix1_led_status[] = {
	{
		.name = "100G",
		.index = _INV_IDX,
		.num_bits = 8,
		.num_reg = 2,
		.reg = { IX1_LED_100G_1_8_INFO_REG,
			 IX1_LED_100G_9_16_INFO_REG,},
	},
	{
		.name = "40G",
		.index = _INV_IDX,
		.num_bits = 8,
		.num_reg = 2,
		.reg = { IX1_LED_40G_1_8_INFO_REG,
			 IX1_LED_40G_9_16_INFO_REG,},
	},
	{
		.name = "25G",
		.index = _INV_IDX,
		.num_bits = 8,
		.num_reg = 2,
		.reg = { IX1_LED_25G_1_8_INFO_REG,
			 IX1_LED_25G_9_16_INFO_REG},
	},
	{
		.name = "10G",
		.index = _INV_IDX,
		.num_bits = 8,
		.num_reg = 2,
		.reg = { IX1_LED_10G_1_8_INFO_REG,
			 IX1_LED_10G_9_16_INFO_REG},
	},
	{
		.name = "50G",
		.index = _INV_IDX,
		.num_bits = 8,
		.num_reg = 4,
		.reg = { IX1_LED_50G_1_4_INFO_REG,
			 IX1_LED_50G_5_8_INFO_REG,
			 IX1_LED_50G_9_12_INFO_REG,
			 IX1_LED_50G_13_16_INFO_REG},
	},
};

static struct port_status ix1_ix2_mntr_status[] = {
	{
		.name = "enable",
		.index = MNTR_ENABLE_IDX,
		.active_low = 0,
		.num_bits = 8,
		.num_reg = 1,
		.reg = { IX1_MNTR_ENABLE_REG },
	},
	{
		.name = "reset_time_lo",
		.index = MNTR_RESET_TIME_LO_IDX,
		.active_low = 0,
		.num_bits = 8,
		.num_reg = 1,
		.reg = { IX1_MNTR_RESET_TIME_LO_REG },
	},
	{
		.name = "reset_time_hi",
		.index = MNTR_RESET_TIME_HI_IDX,
		.active_low = 0,
		.num_bits = 8,
		.num_reg = 1,
		.reg = { IX1_MNTR_RESET_TIME_HI_REG },
	},
};

static struct port_status ix2_sfp_status[] = {
	{
		.name = "present",
		.index = _SFP_PRESENT_IDX,
		.active_low = 1,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX2_IO_SFP_1_4_INFO_REG,
			 IX2_IO_SFP_5_8_INFO_REG,
			 IX2_IO_SFP_9_12_INFO_REG,
			 IX2_IO_SFP_13_16_INFO_REG},
	},
	{
		.name = "tx_fault",
		.index = _SFP_TX_FAULT_IDX,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX2_IO_SFP_1_4_INFO_REG,
			 IX2_IO_SFP_5_8_INFO_REG,
			 IX2_IO_SFP_9_12_INFO_REG,
			 IX2_IO_SFP_13_16_INFO_REG},
	},
	{
		.name = "tx_disable",
		.index = _SFP_TX_DISABLE_IDX,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX2_IO_SFP_1_4_INFO_REG,
			 IX2_IO_SFP_5_8_INFO_REG,
			 IX2_IO_SFP_9_12_INFO_REG,
			 IX2_IO_SFP_13_16_INFO_REG},
		.reg = { IX1_IO_QSFP_1_4_INFO_REG,
			 IX1_IO_QSFP_5_8_INFO_REG,
			 IX1_IO_QSFP_9_12_INFO_REG,
			 IX1_IO_QSFP_13_16_INFO_REG},
	},
	{
		.name = "rx_los",
		.index = _SFP_RX_LOS_IDX,
		.num_bits = 16,
		.num_reg = 4,
		.reg = { IX2_IO_SFP_1_4_INFO_REG,
			 IX2_IO_SFP_5_8_INFO_REG,
			 IX2_IO_SFP_9_12_INFO_REG,
			 IX2_IO_SFP_13_16_INFO_REG},
	},
};

static struct port_status ix2_led_status[] = {
	{
		.name = "100G",
		.index = _INV_IDX,
		.num_bits = 8,
		.num_reg = 1,
		.reg = { IX2_LED_100G_49_56_INFO_REG,},
	},
	{
		.name = "40G",
		.index = _INV_IDX,
		.num_bits = 8,
		.num_reg = 1,
		.reg = { IX2_LED_40G_49_56_INFO_REG,},
	},
	{
		.name = "25G",
		.index = _INV_IDX,
		.num_bits = 8,
		.num_reg = 7,
		.reg = { IX2_LED_25G_1_8_INFO_REG,
			 IX2_LED_25G_9_16_INFO_REG,
			 IX2_LED_25G_17_24_INFO_REG,
			 IX2_LED_25G_25_32_INFO_REG,
			 IX2_LED_25G_33_40_INFO_REG,
			 IX2_LED_25G_41_48_INFO_REG,
			 IX2_LED_25G_49_56_INFO_REG,},
	},
	{
		.name = "10G",
		.index = _INV_IDX,
		.num_bits = 8,
		.num_reg = 7,
		.reg = { IX2_LED_10G_1_8_INFO_REG,
			 IX2_LED_10G_9_16_INFO_REG,
			 IX2_LED_10G_17_24_INFO_REG,
			 IX2_LED_10G_25_32_INFO_REG,
			 IX2_LED_10G_33_40_INFO_REG,
			 IX2_LED_10G_41_48_INFO_REG,
			 IX2_LED_10G_49_56_INFO_REG,},
	},
	{
		.name = "10G_blink",
		.index = _INV_IDX,
		.num_bits = 8,
		.num_reg = 7,
		.reg = { IX2_LED_10G_1_8_BLINK_REG,
			 IX2_LED_10G_9_16_BLINK_REG,
			 IX2_LED_10G_17_24_BLINK_REG,
			 IX2_LED_10G_25_32_BLINK_REG,
			 IX2_LED_10G_33_40_BLINK_REG,
			 IX2_LED_10G_41_48_BLINK_REG,
			 IX2_LED_10G_49_56_BLINK_REG,},
	},
	{
		.name = "50G",
		.index = _INV_IDX,
		.num_bits = 8,
		.num_reg = 2,
		.reg = { IX2_LED_50G_49_52_INFO_REG,
			 IX2_LED_50G_53_56_INFO_REG},
	},
};

/**************************************************************************
 *
 * CPLD I/O
 *
 */

/**
 * cpld_port_reg_word_read - Read an 16-bit CPLD port status register over i2c.
 * Note, these registers have their nibbles swapped.
 *
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else a data word received from the device.
 */
static u16 cpld_port_reg_word_read(struct i2c_client *client, u32 reg,
				   u16 *val)
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
 * cpld_port_reg_word_write - Writes an 16-bit CPLD port status register over i2c.
 * Note, these registers have their nibbles swapped.
 *
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else zero on success.
 */

static s32 cpld_port_reg_word_write(struct i2c_client *client, u32 reg,
				    u16 val)
{
	int res;

	res = i2c_smbus_write_word_data(client, reg, swan16(val));
	if (res) {
		pr_err("I2C write error - addr: 0x%02X, offset: 0x%02X, val: 0x%02X",
		       client->addr, reg, val);
	}
	return res;
}

/**
 * cpld_port_reg_byte_read - Read an 8-bit CPLD port status register over i2c.
 *
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else a data byte received from the device.
 */
static u8 cpld_port_reg_byte_read(struct i2c_client *client, u32 reg,
				  u8 *val)
{
	int retval = 0;

	retval = i2c_smbus_read_word_data(client, reg);
	if (retval < 0) {
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       client->addr, reg);
		return retval;
	}

	*val = retval;
	return retval;
}

/**
 * cpld_port_reg_byte_write - Writes an 8-bit CPLD port status register over i2c.
 *
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else zero on success.
 */

static s32 cpld_port_reg_byte_write(struct i2c_client *client, u32 reg,
				    u8 val)
{
	int res;

	res = i2c_smbus_write_byte_data(client, reg, val);
	if (res) {
		pr_err("I2C write error - addr: 0x%02X, offset: 0x%02X, val: 0x%02X",
		       client->addr, reg, val);
	}
	return res;
}

/*------------------------------------------------------------------------------
 *
 * QSFP I/O show/store methods
 *
 */

static ssize_t port_status_show(struct device *dev,
				struct device_attribute *dattr,
				char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int i, retval, num_bits;
	u16 reg_val;
	u64 val = 0, cpld_val;
	u64 shift_val;
	u8 name_len = 10; /* strlen("ixX_qsfpY_"); */
	struct port_status *target = NULL;
	struct port_status *platform_port_status = NULL;
	int num_platform_status;
	int sindex;

	if (strncasecmp(dattr->attr.name, IX1_PLATFORM_NAME,
			strlen(IX1_PLATFORM_NAME)) == 0) {
		platform_port_status = ix1_qsfp_status;
		num_platform_status = ARRAY_SIZE(ix1_qsfp_status);
	} else if (strncasecmp(dattr->attr.name, IX2_PLATFORM_NAME,
			       strlen(IX2_PLATFORM_NAME)) == 0) {
		platform_port_status = ix2_sfp_status;
		num_platform_status = ARRAY_SIZE(ix2_sfp_status);
	} else {
		pr_err("Unknown platform name:%s\n", dattr->attr.name);
		return -ENXIO;
	}

	/* find the target register */
	for (i = 0; i < num_platform_status; i++) {
		if (strcasecmp(dattr->attr.name + name_len,
			       platform_port_status[i].name) == 0) {
			target = &platform_port_status[i];
			break;
		}
	}
	if (!target)
		return -EINVAL;

	/*
	 * Read the number of registers
	 */
	for (i = 0; i < target->num_reg; i++) {
		shift_val = 0;
		retval = cpld_port_reg_word_read(client, target->reg[i],
						 &reg_val);
		if (retval < 0) {
			/* Error out */
			return retval;
		}
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

	return sprintf(buf, "0x%llx\n", val);
}

static ssize_t port_status_store(struct device *dev,
				 struct device_attribute *dattr,
				 const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	int i, j, retval, num_bits;
	u16 reg_val, new_val, val16;
	u64 val;
	u8 name_len = 10; /* strlen("ixX_qsfpY_"); */
	struct port_status *target = NULL;
	struct port_status *platform_port_status = NULL;
	int num_platform_status;
	int sindex, num_sbits, pindex;

	retval = kstrtou64(buf, 0, &val);
	if (retval != 0)
		return retval;

	if (strncasecmp(dattr->attr.name, IX1_PLATFORM_NAME,
			strlen(IX1_PLATFORM_NAME)) == 0) {
		platform_port_status = ix1_qsfp_status;
		num_platform_status = ARRAY_SIZE(ix1_qsfp_status);
	} else if (strncasecmp(dattr->attr.name, IX2_PLATFORM_NAME,
			       strlen(IX2_PLATFORM_NAME)) == 0) {
		platform_port_status = ix2_sfp_status;
		num_platform_status = ARRAY_SIZE(ix2_sfp_status);
	} else {
		pr_err("Unknown platform name:%s\n", dattr->attr.name);
		return -ENXIO;
	}

	/* find the target register */
	for (i = 0; i < num_platform_status; i++) {
		if (strcasecmp(dattr->attr.name + name_len,
			       platform_port_status[i].name) == 0) {
			target = &platform_port_status[i];
			break;
		}
	}

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

/*------------------------------------------------------------------------------
 *
 * I2C bus monitor show/store methods
 *
 */
static ssize_t mntr_show(struct device *dev,
				struct device_attribute *dattr,
				char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int i, retval;
	u8 reg_val;
	u64 val = 0;
	u8 name_len = 9; /* strlen("ixX_mntr_"); */
	struct port_status *target = NULL;
	struct port_status *platform_port_status = NULL;
	int num_platform_status;

	if (strncasecmp(dattr->attr.name, IX1_PLATFORM_NAME,
			strlen(IX1_PLATFORM_NAME)) == 0) {
		platform_port_status = ix1_ix2_mntr_status;
		num_platform_status = ARRAY_SIZE(ix1_ix2_mntr_status);
	} else if (strncasecmp(dattr->attr.name, IX2_PLATFORM_NAME,
			       strlen(IX2_PLATFORM_NAME)) == 0) {
		platform_port_status = ix1_ix2_mntr_status;
		num_platform_status = ARRAY_SIZE(ix1_ix2_mntr_status);
	} else {
		pr_err("Unknown platform name:%s\n", dattr->attr.name);
		return -ENXIO;
	}

	/*
	 * Find the target register
	 */
	for (i = 0; i < num_platform_status; i++) {
		if (strcasecmp(dattr->attr.name + name_len,
			       platform_port_status[i].name) == 0) {
			target = &platform_port_status[i];
			break;
		}
	}
	if (!target)
		return -EINVAL;

	if (target->num_bits != 8) {
		pr_err("mntr_show: target %s has unexpected num_bits= %d\n",
			target->name, target->num_bits);
		return -EINVAL;
	}

	if (target->num_reg != 1) {
		pr_err("mntr_show: target %s has unexpected num_reg= %d\n",
			target->name, target->num_reg);
		return -EINVAL;
	}

	/*
	 * Read the register and return.
	 */
	retval = cpld_port_reg_byte_read(client, target->reg[0], &reg_val);
	if (retval < 0) {
		/* Error out */
		return retval;
	}

	val = (u64)retval;
	if (target->active_low)
		val = ~val;
	return sprintf(buf, "0x%llx\n", val);
}

static ssize_t mntr_store(struct device *dev,
				 struct device_attribute *dattr,
				 const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	int i, retval;
	u8  reg_val;
	u64 val;
	u8 name_len = 9; /* strlen("ixX_mntr_"); */
	struct port_status *target = NULL;
	struct port_status *platform_port_status = NULL;
	int num_platform_status;

	retval = kstrtou64(buf, 0, &val);
	if (retval != 0)
		return retval;

	if (strncasecmp(dattr->attr.name, IX1_PLATFORM_NAME,
			strlen(IX1_PLATFORM_NAME)) == 0) {
		platform_port_status = ix1_ix2_mntr_status;
		num_platform_status = ARRAY_SIZE(ix1_ix2_mntr_status);
	} else if (strncasecmp(dattr->attr.name, IX2_PLATFORM_NAME,
			       strlen(IX2_PLATFORM_NAME)) == 0) {
		platform_port_status = ix1_ix2_mntr_status;
		num_platform_status = ARRAY_SIZE(ix1_ix2_mntr_status);
	} else {
		pr_err("Unknown platform name:%s\n", dattr->attr.name);
		return -ENXIO;
	}

	/* find the target register */
	for (i = 0; i < num_platform_status; i++) {
		if (strcasecmp(dattr->attr.name + name_len,
			       platform_port_status[i].name) == 0) {
			target = &platform_port_status[i];
			break;
		}
	}
	if (!target)
		return -EINVAL;

	if (target->num_bits != 8) {
		pr_err("mntr_store: target %s has unexpected num_bits= %d\n",
			target->name, target->num_bits);
		return -EINVAL;
	}

	if (target->num_reg != 1) {
		pr_err("mntr_store: target %s has unexpected num_reg= %d\n",
			target->name, target->num_reg);
		return -EINVAL;
	}

	if (target->active_low)
		val = ~val;
	val = val & GENMASK_ULL(target->num_bits - 1, 0);
	reg_val = (u8)val;

	if (strcasecmp(target->name, "enable") == 0) {
		static unsigned int logged = 0;

		if ((reg_val & 0x01) == 1) {
			/*
			 * The bus monitor feature is currently disabled by
			 * default.
			 *
			 * If anyone enables it by writing 0x1 to the `enable'
			 * register, we want to log a message to that effect
			 * (once) so it's obvious that's what happened.
			 */
			if (logged == 0) {
				pr_err("Enabled bus monitor watchdog\n");
				logged = 1;
			}
		} else {
			logged = 0;
		}
	}

	retval = cpld_port_reg_byte_write(client, target->reg[0], reg_val);
	if (retval)
		return retval;

	return count;
}

static SYSFS_ATTR_RW(ix1_mntr_enable,        mntr_show, mntr_store);
static SYSFS_ATTR_RW(ix1_mntr_reset_time_lo, mntr_show, mntr_store);
static SYSFS_ATTR_RW(ix1_mntr_reset_time_hi, mntr_show, mntr_store);

static SYSFS_ATTR_RO(ix1_qsfp1_present, port_status_show);
static SYSFS_ATTR_RO(ix1_qsfp1_interrupt,   port_status_show);
static SYSFS_ATTR_RW(ix1_qsfp1_lp_mode, port_status_show, port_status_store);
static SYSFS_ATTR_RW(ix1_qsfp1_reset,   port_status_show, port_status_store);
static SYSFS_ATTR_RO(ix1_qsfp2_present, port_status_show);
static SYSFS_ATTR_RO(ix1_qsfp2_interrupt,   port_status_show);
static SYSFS_ATTR_RW(ix1_qsfp2_lp_mode, port_status_show, port_status_store);
static SYSFS_ATTR_RW(ix1_qsfp2_reset,   port_status_show, port_status_store);

static SYSFS_ATTR_RW(ix2_mntr_enable,        mntr_show, mntr_store);
static SYSFS_ATTR_RW(ix2_mntr_reset_time_lo, mntr_show, mntr_store);
static SYSFS_ATTR_RW(ix2_mntr_reset_time_hi, mntr_show, mntr_store);

static SYSFS_ATTR_RO(ix2_sfps1_present,    port_status_show);
static SYSFS_ATTR_RO(ix2_sfps1_tx_fault,   port_status_show);
static SYSFS_ATTR_RW(ix2_sfps1_tx_disable, port_status_show, port_status_store);
static SYSFS_ATTR_RO(ix2_sfps1_rx_los,     port_status_show);
static SYSFS_ATTR_RO(ix2_sfps2_present,    port_status_show);
static SYSFS_ATTR_RO(ix2_sfps2_tx_fault,   port_status_show);
static SYSFS_ATTR_RW(ix2_sfps2_tx_disable, port_status_show, port_status_store);
static SYSFS_ATTR_RO(ix2_sfps2_rx_los,     port_status_show);
static SYSFS_ATTR_RO(ix2_sfps3_present,    port_status_show);
static SYSFS_ATTR_RO(ix2_sfps3_tx_fault,   port_status_show);
static SYSFS_ATTR_RW(ix2_sfps3_tx_disable, port_status_show, port_status_store);
static SYSFS_ATTR_RO(ix2_sfps3_rx_los,     port_status_show);

/*------------------------------------------------------------------------------
 *
 * QSFP LED show/store methods
 *
 */

static ssize_t port_led_show(struct device *dev,
			     struct device_attribute *dattr,
			     char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int i, retval, num_bits;
	u64 val = 0;
	u64 shift_val;
	u8 reg_val;
	u8 name_len = 9; /* strlen("ixX_ledY_"); */
	struct port_status *target = NULL;
	struct port_status *platform_led_status = NULL;
	int num_platform_leds;

	/*
	 * Find the platform specific led register
	 */
	if (strncasecmp(dattr->attr.name, IX1_PLATFORM_NAME,
			strlen(IX1_PLATFORM_NAME)) == 0) {
		platform_led_status = ix1_led_status;
		num_platform_leds = ARRAY_SIZE(ix1_led_status);
	} else if (strncasecmp(dattr->attr.name, IX2_PLATFORM_NAME,
			  strlen(IX2_PLATFORM_NAME)) == 0) {
		platform_led_status = ix2_led_status;
		num_platform_leds = ARRAY_SIZE(ix2_led_status);
	} else {
		pr_err("Unknown platform name:%s\n",
		       dattr->attr.name);
		return -ENXIO;
	}

	/* find the target register */
	for (i = 0; i < num_platform_leds; i++) {
		if (strcasecmp(dattr->attr.name + name_len,
			       platform_led_status[i].name) == 0) {
			target = &platform_led_status[i];
			break;
		}
	}

	if (!target)
		return -EINVAL;

	/*
	 * Read the number of registers
	 */
	for (i = 0; i < target->num_reg; i++) {
		shift_val = 0;
		retval = cpld_port_reg_byte_read(client,
						 target->reg[i],
						 &reg_val);
		if (retval < 0)
			/* Error out */
			return retval;
		shift_val = (uint8_t)reg_val;
		val |= ((reg_val) << (i * target->num_bits));
	}

	num_bits = target->num_reg * target->num_bits;

	if (target->active_low)
		val = ~val;

	val = val & GENMASK_ULL(num_bits - 1, 0);

	return sprintf(buf, "0x%llx\n", val);
}

static ssize_t port_led_store(struct device *dev,
			      struct device_attribute *dattr,
			      const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	int i, retval, num_bits;
	u8 val8;
	u64 val;
	u8 name_len = 9; /* strlen("ixX_ledY_"); */
	u8 new_val, reg_val = 0x0;
	struct port_status *target = NULL;
	struct port_status *platform_led_status = NULL;
	int num_platform_leds;

	retval = kstrtou64(buf, 0, &val);
	if (retval != 0)
		return retval;

	/*
	 * Find the platform specific led register
	 */
	if (strncasecmp(dattr->attr.name, IX1_PLATFORM_NAME,
			strlen(IX1_PLATFORM_NAME)) == 0) {
		platform_led_status = ix1_led_status;
		num_platform_leds = ARRAY_SIZE(ix1_led_status);
	} else if (strncasecmp(dattr->attr.name, IX2_PLATFORM_NAME,
			  strlen(IX2_PLATFORM_NAME)) == 0) {
		platform_led_status = ix2_led_status;
		num_platform_leds = ARRAY_SIZE(ix2_led_status);
	} else {
		pr_err("Unknown platform name:%s\n",
		       dattr->attr.name);
		return -ENXIO;
	}

	/* find the target register */
	for (i = 0; i < num_platform_leds; i++) {
		if (strcasecmp(dattr->attr.name + name_len,
			       platform_led_status[i].name) == 0) {
			target = &platform_led_status[i];
			break;
		}
	}

	if (!target)
		return -EINVAL;

	num_bits = target->num_reg * target->num_bits;

	if (target->active_low)
		val = ~val;

	for (i = 0; i < target->num_reg; i++) {
		retval = cpld_port_reg_byte_read(client, target->reg[i],
						 &reg_val);
		if (retval < 0) {
			/* Error out */
			return retval;
		}

		new_val = reg_val;
		val8 = (val >> (i * target->num_bits)) &
					GENMASK_ULL(target->num_bits - 1, 0);

		if (reg_val != val8)
			new_val = val8;

		if (new_val != reg_val) {
			retval = cpld_port_reg_byte_write(client,
							  target->reg[i],
							  new_val);
			if (retval < 0)
				return retval;
		}
	}

	return count;
}

static SYSFS_ATTR_RW(ix1_led1_100G, port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix1_led2_100G, port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix1_led1_40G,  port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix1_led2_40G,  port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix1_led1_25G,  port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix1_led2_25G,  port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix1_led1_10G,  port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix1_led2_10G,  port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix1_led1_50G,  port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix1_led2_50G,  port_led_show, port_led_store);

static SYSFS_ATTR_RW(ix2_led1_100G, port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix2_led1_40G,  port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix2_led1_25G,  port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix2_led1_10G,  port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix2_led1_10G_blink, port_led_show, port_led_store);
static SYSFS_ATTR_RW(ix2_led1_50G,  port_led_show, port_led_store);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */

static struct attribute *ix1_led_cpld_1_16_attrs[] = {
	&dev_attr_ix1_led1_100G.attr,
	&dev_attr_ix1_led1_40G.attr,
	&dev_attr_ix1_led1_25G.attr,
	&dev_attr_ix1_led1_10G.attr,
	&dev_attr_ix1_led1_50G.attr,
	NULL,
};

static struct attribute *ix1_io_cpld_1_16_attrs[] = {
	&dev_attr_ix1_qsfp1_present.attr,
	&dev_attr_ix1_qsfp1_interrupt.attr,
	&dev_attr_ix1_qsfp1_lp_mode.attr,
	&dev_attr_ix1_qsfp1_reset.attr,
	NULL,
};

static struct attribute *ix1_led_cpld_17_32_attrs[] = {
	&dev_attr_ix1_led2_100G.attr,
	&dev_attr_ix1_led2_40G.attr,
	&dev_attr_ix1_led2_25G.attr,
	&dev_attr_ix1_led2_10G.attr,
	&dev_attr_ix1_led2_50G.attr,
	NULL,
};

static struct attribute *ix1_io_cpld_17_32_attrs[] = {
	&dev_attr_ix1_qsfp2_present.attr,
	&dev_attr_ix1_qsfp2_interrupt.attr,
	&dev_attr_ix1_qsfp2_lp_mode.attr,
	&dev_attr_ix1_qsfp2_reset.attr,
	NULL,
};

static struct attribute *ix1_mntr_attrs[] = {
	&dev_attr_ix1_mntr_enable.attr,
	&dev_attr_ix1_mntr_reset_time_lo.attr,
	&dev_attr_ix1_mntr_reset_time_hi.attr,
	NULL,
};

static struct attribute *ix2_io_cpld_1_16_attrs[] = {
	&dev_attr_ix2_sfps1_present.attr,
	&dev_attr_ix2_sfps1_tx_fault.attr,
	&dev_attr_ix2_sfps1_tx_disable.attr,
	&dev_attr_ix2_sfps1_rx_los.attr,
	NULL,
};

static struct attribute *ix2_io_cpld_17_32_attrs[] = {
	&dev_attr_ix2_sfps2_present.attr,
	&dev_attr_ix2_sfps2_tx_fault.attr,
	&dev_attr_ix2_sfps2_tx_disable.attr,
	&dev_attr_ix2_sfps2_rx_los.attr,
	NULL,
};

static struct attribute *ix2_io_cpld_33_48_attrs[] = {
	&dev_attr_ix2_sfps3_present.attr,
	&dev_attr_ix2_sfps3_tx_fault.attr,
	&dev_attr_ix2_sfps3_tx_disable.attr,
	&dev_attr_ix2_sfps3_rx_los.attr,
	NULL,
};

static struct attribute *ix2_led_cpld_1_52_attrs[] = {
	&dev_attr_ix2_led1_100G.attr,
	&dev_attr_ix2_led1_40G.attr,
	&dev_attr_ix2_led1_25G.attr,
	&dev_attr_ix2_led1_10G.attr,
	&dev_attr_ix2_led1_10G_blink.attr,
	&dev_attr_ix2_led1_50G.attr,
	NULL,
};

static struct attribute *ix2_mntr_attrs[] = {
	&dev_attr_ix2_mntr_enable.attr,
	&dev_attr_ix2_mntr_reset_time_lo.attr,
	&dev_attr_ix2_mntr_reset_time_hi.attr,
	NULL,
};

static struct attribute *ix2_uart_switch_cpld_attrs[] = {
	NULL,
};

static struct attribute_group ix1_led_cpld_1_16_attr_group = {
	.attrs = ix1_led_cpld_1_16_attrs,
};

static struct attribute_group ix1_io_cpld_1_16_attr_group = {
	.attrs = ix1_io_cpld_1_16_attrs,
};

static struct attribute_group ix1_led_cpld_17_32_attr_group = {
	.attrs = ix1_led_cpld_17_32_attrs,
};

static struct attribute_group ix1_io_cpld_17_32_attr_group = {
	.attrs = ix1_io_cpld_17_32_attrs,
};

static struct attribute_group ix1_mntr_attr_group = {
	.attrs = ix1_mntr_attrs,
};

static struct attribute_group ix2_io_cpld_1_16_attr_group = {
	.attrs = ix2_io_cpld_1_16_attrs,
};

static struct attribute_group ix2_io_cpld_33_48_attr_group = {
	.attrs = ix2_io_cpld_33_48_attrs,
};

static struct attribute_group ix2_led_cpld_1_52_attr_group = {
	.attrs = ix2_led_cpld_1_52_attrs,
};

static struct attribute_group ix2_io_cpld_17_32_attr_group = {
	.attrs = ix2_io_cpld_17_32_attrs,
};

static struct attribute_group ix2_mntr_attr_group = {
	.attrs = ix2_mntr_attrs,
};

static struct attribute_group ix2_uart_switch_cpld_attr_group = {
	.attrs = ix2_uart_switch_cpld_attrs,
};

/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static int cpld_probe(struct i2c_client *client,
		      const struct i2c_device_id *id)

{
	struct quanta_ix_rangeley_platform_data *pdata;
	int retval;
	int cpld_idx;
	struct kobject *kobj = &client->dev.kobj;

	pr_info(DRIVER_NAME " probed\n");

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_BYTE_DATA
				     | I2C_FUNC_SMBUS_WORD_DATA)) {
		dev_err(&client->dev, "smbud read byte and word data not supported.\n");
		return -ENODEV;
	}

	pdata = dev_get_platdata(&client->dev);
	cpld_idx = pdata->idx;

	/*
	 * Create sysfs nodes.
	 */
	switch (cpld_idx) {
	case IX1_LED_QSFP28_1_16_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &ix1_led_cpld_1_16_attr_group);
		break;
	case IX1_IO_QSFP28_1_16_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &ix1_io_cpld_1_16_attr_group);
		break;
	case IX1_LED_QSFP28_17_32_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &ix1_led_cpld_17_32_attr_group);
		break;
	case IX1_IO_QSFP28_17_32_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &ix1_io_cpld_17_32_attr_group);
		break;
	case IX1_I2C_BUS_MNTR_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &ix1_mntr_attr_group);
		break;
	case IX2_IO_SFP28_33_48_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &ix2_io_cpld_33_48_attr_group);
		break;
	case IX2_IO_SFP28_1_16_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &ix2_io_cpld_1_16_attr_group);
		break;
	case IX2_LED_1_52_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &ix2_led_cpld_1_52_attr_group);
		break;
	case IX2_IO_SFP28_17_32_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &ix2_io_cpld_17_32_attr_group);
		break;
	case IX2_I2C_BUS_MNTR_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &ix2_mntr_attr_group);
		break;
	case IX2_UART_SWITCH_CPLD_ID:
		retval = sysfs_create_group(kobj,
					    &ix2_uart_switch_cpld_attr_group);
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

	sysfs_remove_group(kobj, &ix1_led_cpld_1_16_attr_group);
	sysfs_remove_group(kobj, &ix1_io_cpld_1_16_attr_group);
	sysfs_remove_group(kobj, &ix1_led_cpld_17_32_attr_group);
	sysfs_remove_group(kobj, &ix1_io_cpld_17_32_attr_group);
	sysfs_remove_group(kobj, &ix1_mntr_attr_group);
	sysfs_remove_group(kobj, &ix2_io_cpld_33_48_attr_group);
	sysfs_remove_group(kobj, &ix2_io_cpld_1_16_attr_group);
	sysfs_remove_group(kobj, &ix2_led_cpld_1_52_attr_group);
	sysfs_remove_group(kobj, &ix2_io_cpld_17_32_attr_group);
	sysfs_remove_group(kobj, &ix2_mntr_attr_group);
	sysfs_remove_group(kobj, &ix2_uart_switch_cpld_attr_group);

	dev_info(&client->dev, "device removed\n");
	return 0;
}

static const struct i2c_device_id cpld_i2c_ids[] = {
	{
		.name = "ix_rangeley_cpld",
	},

	{ /* end of list */ },
};
MODULE_DEVICE_TABLE(i2c, cpld_i2c_ids);

static struct i2c_driver cpld_driver = {
	.driver = {
		.name  = "quanta_ix_rangeley_cpld",
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

static int __init quanta_ix_rangeley_cpld_init(void)
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

static void __exit quanta_ix_rangeley_cpld_exit(void)
{
	i2c_del_driver(&cpld_driver);
	pr_info(DRIVER_NAME " CPLD driver unloaded\n");
}

MODULE_AUTHOR("Vidya Sagar Ravipati <vidya@cumulusnetworks.com>");
MODULE_DESCRIPTION("CPLD driver for Quanta IX RANGELEY Platforms");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(quanta_ix_rangeley_cpld_init);
module_exit(quanta_ix_rangeley_cpld_exit);
