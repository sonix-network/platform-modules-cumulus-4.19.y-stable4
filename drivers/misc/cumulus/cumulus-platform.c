// SPDX-License-Identifier: GPL-2.0+
/*
 * Common Cumulus platform module APIs.
 *
 * Copyright (C) 2017 Cumulus Networks, Inc.
 * Author: Curt Brune <curt@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * https://www.gnu.org/licenses/gpl-2.0-standalone.html
 */

/**
 * This module acts as a library of common routines, used by Cumulus
 * platform kernel modules.
 *
 * This module is not a device driver.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/cumulus-platform.h>
#include "platform-defs.h"

#define CUMULUS_PLATFORM_MODULE_VERSION "1.0"

/**
 * cumulus_i2c_find_adapter() - look-up i2c adapter
 * @name: name of i2c adapter to find
 *
 * Attempt to find the i2c adapter with the name @name.  Returns the
 * non-negative adapter index when found.  Returns -ENODEV if not
 * found.
 */
int cumulus_i2c_find_adapter(const char *name)
{
	int i;
	int rc = -ENODEV;
	struct i2c_adapter *adapter;

	for (i = 0; i < 50; i++) {
		adapter = i2c_get_adapter(i);
		if (adapter) {
			if (!strncmp(adapter->name, name, strlen(name)))
				rc = i;
			i2c_put_adapter(adapter);
		}
		if (rc >= 0)
			break;
	}

	return rc;
}
EXPORT_SYMBOL_GPL(cumulus_i2c_find_adapter);

/**
 * cumulus_i2c_add_client()
 * @adapter: i2c adapter number
 * @info: i2c_board_info
 *
 * Attempt to create a new i2c_client, attached to i2c @adapter.
 * @info contains specific information about the device to create.
 *
 * On succes returns a newly allocated i2c_client structure.  On
 * failure returns ERR_PTR(-ENODEV).
 */
struct i2c_client *
cumulus_i2c_add_client(int bus, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	int i = 20;

	/* Some i2c mux/switch adapters are slow to arrive in the
	 * device model. Also depends on whether the device driver is
	 * loaded via hotplug or not.  Allow up to 1500 msec for the
	 * adapter to show up.
	 */
	do {
		adapter = i2c_get_adapter(bus);
		if (adapter)
			break;
		msleep(100);
	} while (i--);

	if (!adapter)
		return ERR_PTR(-ENXIO);

	client = i2c_new_device(adapter, info);
	if (!client)
		client = ERR_PTR(-ENODEV);

	i2c_put_adapter(adapter);
	return client;
}
EXPORT_SYMBOL_GPL(cumulus_i2c_add_client);

/**
 * cumulus_gpio_map_show()
 * @dev: device driver object
 * @dattr: device attribute object
 * @buf: character buffer filled on output
 *
 * Return an array of GPIO signal names separated by a newline
 * character.  The GPIO names are ordered by the GPIO number.
 *
 * Format of output:
 * line 1     -- base GPIO number
 * line 2     -- 1st GPIO name
 * ....
 * line n + 1 -- Nth GPIO name
 *
 */
ssize_t cumulus_gpio_map_show(struct device *dev,
			      struct gpio_chip *chip,
			      char *buf)
{
	int i;
	int length = PAGE_SIZE, size;
	char *p = buf;

	size = snprintf(p, length, "%u\n", chip->base);
	length -= size;
	p += size;

	for (i = 0; i < chip->ngpio; i++) {
		size = snprintf(p, length, "%s\n", chip->names[i]);
		if (size > length) {
			dev_warn(dev,
				 "GPIO map string larger than PAGE_SIZE, output truncated.\n");
			break;
		}
		length -= size;
		p += size;
	}

	return p - buf;
}
EXPORT_SYMBOL_GPL(cumulus_gpio_map_show);

/**
 * @brief Return the value of a sysfs attribute
 *
 * @param dev The device which is being read
 * @param dattr The attribute being shown
 * @param buf A buffer into which the attribute value will be placed
 * @param bif A description of the attribute being accessed
 * @param read A function used for reading the device
 *
 * @return ssize_t The number of bytes written into buf, or negative value
 *                 for an error
 */
ssize_t cumulus_bf_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf,
			struct bf *bif,
			bf_read_func *read)
{
	int nregs = (bif->shift + bif->width + 7) / 8;
	u32 mask = BF_MASK(bif->width);
	u32 val;
	char *sign = "";
	int ret;

	ret = (*read)(dev, bif->reg, nregs, &val);
	if (ret)
		return ret;

	val = val >> bif->shift & mask;
	if (bif->flags & BF_COMPLEMENT)
		val ^= mask;
	if (bif->flags & BF_SIGNED && val & (mask ^ mask >> 1)) {
		sign = "-";
		val = (val ^ mask) + 1;
	}

	if (bif->values)
		return sprintf(buf, "%s\n", bif->values[val]);
	if (bif->width < 2 || bif->flags & BF_DECIMAL)
		return sprintf(buf, "%s%u\n", sign, val);
	return sprintf(buf, "%s0x%0*x\n", sign, (bif->width + 3) / 4, val);
}
EXPORT_SYMBOL_GPL(cumulus_bf_show);

/**
 * @brief Write the specified sysfs attribute to a device
 *
 * @param dev The device which is being written
 * @param dattr The attribute being written
 * @param buf The value to write to the device
 * @param size The number of bytes in buf
 * @param bif A description of the attribute being accessed
 * @param read A function used for reading the device
 * @param write A function used for writing to the device
 *
 * @return ssize_t The number of bytes in buf which were processed, or a
 *                 negative value on error
 */
ssize_t cumulus_bf_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf,
			 size_t size,
			 struct bf *bif,
			 bf_read_func *read,
			 bf_write_func *write)
{
	int nregs = (bif->shift + bif->width + 7) / 8;
	u32 mask = BF_MASK(bif->width);
	u32 newval;
	u32 oldval = 0;
	int ret;

	if (bif->values) {
		char str[PLATFORM_LED_COLOR_NAME_SIZE];

		#if PLATFORM_LED_COLOR_NAME_SIZE != 20
		#error PLATFORM_LED_COLOR_NAME_SIZE is supposed to be 20
		#endif
		if (sscanf(buf, "%19s", str) != 1)
			return -EINVAL;
		for (newval = 0;; newval++) {
			if (strcmp(str, bif->values[newval]) == 0)
				break;
			if (newval == mask)
				return -EINVAL;
		}
	} else {
		/* make sure the new value fits in bif->width bits */
		if (bif->flags & BF_SIGNED) {
			s32 max = mask >> 1;
			s32 min = -(mask ^ mask >> 1);
			s32 v;

			ret = kstrtos32(buf, 0, &v);
			if (ret)
				return ret;
			if (v < min || v > max)
				return -EINVAL;
			newval = v & mask;
		} else {
			ret = kstrtou32(buf, 0, &newval);
			if (ret)
				return ret;
			if (newval > mask)
				return -EINVAL;
		}
	}

	if (bif->flags & BF_COMPLEMENT)
		newval ^= mask;
	newval <<= bif->shift;

	/*
	 * If we're not writing whole registers, then read
	 * the old value and fill in the unmodified bits.
	 */
	if (bif->shift != 0 || bif->width % 8 != 0) {
		ret = (*read)(dev, bif->reg, nregs, &oldval);
		if (ret)
			return ret;
	}
	newval |= oldval & ~(mask << bif->shift);

	ret = (*write)(dev, bif->reg, nregs, newval);
	if (ret)
		return ret;

	return size;
}
EXPORT_SYMBOL_GPL(cumulus_bf_store);

/**
 * @brief Return the value of a sysfs attribute
 *
 * @param dev The device which is being read
 * @param dattr The attribute being shown
 * @param buf A buffer into which the attribute value will be placed
 * @param bif A description of the attribute being accessed
 * @param read A function used for reading the device
 *
 * @return ssize_t The number of bytes written into buf, or negative value
 *                 for an error
 */
ssize_t cumulus_bf_show32(struct device *dev,
			  struct device_attribute *dattr,
			  char *buf,
			  struct bf *bif,
			  bf_read_func *read)
{
	int nregs = (bif->shift + bif->width + 7) / 8;
	u32 mask = BF_MASK(bif->width);
	u32 val;
	char *sign = "";
	int ret;

	ret = (*read)(dev, bif->reg32, nregs, &val);
	if (ret)
		return ret;

	val = val >> bif->shift & mask;
	if (bif->flags & BF_COMPLEMENT)
		val ^= mask;
	if (bif->flags & BF_SIGNED && val & (mask ^ mask >> 1)) {
		sign = "-";
		val = (val ^ mask) + 1;
	}

	if (bif->values)
		return sprintf(buf, "%s\n", bif->values[val]);
	if (bif->width < 2 || bif->flags & BF_DECIMAL)
		return sprintf(buf, "%s%u\n", sign, val);
	return sprintf(buf, "%s0x%0*x\n", sign, (bif->width + 3) / 4, val);
}
EXPORT_SYMBOL_GPL(cumulus_bf_show32);

/**
 * @brief Write the specified sysfs attribute to a device
 *
 * @param dev The device which is being written
 * @param dattr The attribute being written
 * @param buf The value to write to the device
 * @param size The number of bytes in buf
 * @param bif A description of the attribute being accessed
 * @param read A function used for reading the device
 * @param write A function used for writing to the device
 *
 * @return ssize_t The number of bytes in buf which were processed, or a
 *                 negative value on error
 */
ssize_t cumulus_bf_store32(struct device *dev,
			   struct device_attribute *dattr,
			   const char *buf,
			   size_t size,
			   struct bf *bif,
			   bf_read_func *read,
			   bf_write_func *write)
{
	int nregs = (bif->shift + bif->width + 7) / 8;
	u32 mask = BF_MASK(bif->width);
	u32 newval;
	u32 oldval = 0;
	int ret;

	if (bif->values) {
		char str[PLATFORM_LED_COLOR_NAME_SIZE];

		#if PLATFORM_LED_COLOR_NAME_SIZE != 20
		#error PLATFORM_LED_COLOR_NAME_SIZE is supposed to be 20
		#endif
		if (sscanf(buf, "%19s", str) != 1)
			return -EINVAL;
		for (newval = 0;; newval++) {
			if (strcmp(str, bif->values[newval]) == 0)
				break;
			if (newval == mask)
				return -EINVAL;
		}
	} else {
		/* make sure the new value fits in bif->width bits */
		if (bif->flags & BF_SIGNED) {
			s32 max = mask >> 1;
			s32 min = -(mask ^ mask >> 1);
			s32 v;

			ret = kstrtos32(buf, 0, &v);
			if (ret)
				return ret;
			if (v < min || v > max)
				return -EINVAL;
			newval = v & mask;
		} else {
			ret = kstrtou32(buf, 0, &newval);
			if (ret)
				return ret;
			if (newval > mask)
				return -EINVAL;
		}
	}

	if (bif->flags & BF_COMPLEMENT)
		newval ^= mask;
	newval <<= bif->shift;

	/*
	 * If we're not writing whole registers, then read
	 * the old value and fill in the unmodified bits.
	 */
	if (bif->shift != 0 || bif->width % 8 != 0) {
		ret = (*read)(dev, bif->reg32, nregs, &oldval);
		if (ret)
			return ret;
	}
	newval |= oldval & ~(mask << bif->shift);

	ret = (*write)(dev, bif->reg32, nregs, newval);
	if (ret)
		return ret;

	return size;
}
EXPORT_SYMBOL_GPL(cumulus_bf_store32);

/**
 * @brief Read registers on an I2C device
 *
 * @param dev The I2C device
 * @param reg The base register address to read
 * @param nregs The number of consecutive registers to read
 * @param val The value read, max 32-bits
 *
 * @return int 0 on success and < 0 for error
 */
int cumulus_bf_i2c_read_reg(struct device *dev,
			    int reg,
			    int nregs,
			    u32 *val)
{
	struct i2c_client *client = to_i2c_client(dev);
	int i;

	*val = 0;	/* squash use-before-init warning */
	for (i = 0; i < nregs; i++, reg++) {
		int ret = i2c_smbus_read_byte_data(client, reg);

		if (ret < 0)
			return ret;
		*val |= ret << i * 8;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(cumulus_bf_i2c_read_reg);

/**
 * @brief Write registers on an I2C device
 *
 * @param dev The I2C device
 * @param reg The base register address to write
 * @param nregs The number of consecutive registers to write
 * @param val The value to write
 *
 * @return int 0 on success, and < 0 for an error
 */
int cumulus_bf_i2c_write_reg(struct device *dev,
			     int reg,
			     int nregs,
			     u32 val)
{
	struct i2c_client *client = to_i2c_client(dev);

	for (; nregs > 0; nregs--, reg++, val >>= 8) {
		int ret = i2c_smbus_write_byte_data(client, reg, val);

		if (ret < 0)
			return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(cumulus_bf_i2c_write_reg);

MODULE_AUTHOR("Curt Brune <curt@cumulusnetworks.com");
MODULE_DESCRIPTION("Cumulus Platform Module Library");
MODULE_LICENSE("GPL");
MODULE_VERSION(CUMULUS_PLATFORM_MODULE_VERSION);
