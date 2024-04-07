/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * platform_bitfield.h -- Register bitfield support
 *
 * Copyright (C) 2012-2015, 2019 Cumulus Networks, Inc.  All rights reserved.
 * Author: Ellen Wang <ellen@cumulusnetworks.com>
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

#ifndef PLATFORM_BITFIELD_H__
#define PLATFORM_BITFIELD_H__

#include <linux/bits.h>
#include <linux/sysfs.h>
#include <linux/device.h>

/*******************************************************************************
 *
 *                              Useful Macros
 *
 *  These macros provide field and bit maniplation.
 *
 ******************************************************************************/

/**
 *  @brief Retrieve the value of a field in a register. This doesn't actually
 *         access the hardware, it merely isolates and shifts a value to extract
 *         a field.
 *  @param [in] _reg The register value which contains the _field
 *  @param [in] _field The name of the field, from the constants, below, without
 *         the trailing _LSB or _MSB.
 *  @return The value of the field
 */
#define GET_FIELD(_reg, _field) \
	(((_reg) & GENMASK(_field##_MSB, _field##_LSB)) >> _field##_LSB)

/**
 *  @brief Set the value of a field in a register. This doesn't actually access
 *         the hardware, it merely isolates and shifts a field within a value to
 *         place that field within the value.
 *  @param [in] _reg The register value which contains the _field
 *  @param [in] _field The name of the field, from the constants, below, without
 *         the trailing _LSB or _MSB.
 *  @param [in] _val The value of the field to place within the register.
 *  @return The new register value with the modified field value
 */
#define PUT_FIELD(_reg, _field, _val) \
	(((_reg) & ~GENMASK(_field##_MSB, _field##_LSB)) | \
	 ((_val) << _field##_LSB))

/**
 *  @brief Get the width of a field in a register.
 *  @param [in] _field The name of the field, from the constants, below, without
 *         the trailing _LSB or _MSB.
 *  @return The number of bits in the field
 */
#define FIELD_WIDTH(_field) \
	(_field##_MSB - _field##_LSB + 1)

/**
 *  @brief Get the value of a bit in a register. Very similar to #GET_FIELD
 *         except it operates on only one bit.
 *  @param [in] _reg The register value which contains the _bit
 *  @param [in] _bit The name of the bit field, from the constants, below,
 *         without the trailing _BIT.
 *  @return The of the bit field
 */
#define GET_BIT(_reg, _bit) \
	(((_reg) >> _bit##_BIT) & 1)

/**
 *  @brief Put the value of a bit in a register. Very similar to #PUT_FIELD
 *         except it operates on only one bit.
 *  @param [in] _reg The register value which contains the _bit
 *  @param [in] _bit The name of the bit field, from the constants, below,
 *         without the trailing _BIT.
 *  @param [in] _val The value of the bit. Any non-zero value is 1.
 *  @return The new register value with the modified bit field value
 */
#define PUT_BIT(_reg, _bit, _val) \
	(((_reg) & ~(1 << _bit##_BIT)) | ((!!(_val)) << _bit##_BIT))

/**
 *  @brief Create a bit-mask of N bits in the least significant bits
 *  @param [in] _n The number of bits in the mask
 *  @return The bit mask
 */
#define BF_MASK(_n) \
	((u32)(((u64)1 << (_n)) - 1))

/*
 * Support for exposing a bunch of registers (like in a CPLD) in sysfs.
 *
 * Values can be single 8-bit registers, multiple registers (little
 * endian), bitfields (possibly across several registers).
 * The values can be expressed in hex, decimal, or enum-like strings.
 *
 * First, some examples:
 *
 * Let's say we have a CPLD (called "qcpld") that is accessed
 * as a bunch of 8-bit registers.  Register 0 contains
 * the revision number.  Bits 0 to 3 are the minor version
 * and bits 4 to 7 are the major version.
 * We would then write this:
 *
 * mk_bf_ro(qcpld, cpld_major_version, 0x00, 4, 4, NULL, 0)
 * mk_bf_ro(qcpld, cpld_minor_version, 0x00, 0, 4, NULL, 0)
 *
 * The arguments are:
 *   a prefix to make things unique
 *   the sysfs node name
 *   the register number
 *   the bit offset in the register
 *   the field width in bits
 *   a string array to map value into names (see below)
 *   flags
 *
 * Of course, this is a read-only register, so we use mk_bf_ro().
 *
 * To show the version number in decimal, do this:
 *
 * mk_bf_ro(qcpld, cpld_major_version, 0x00, 4, 4, NULL, BF_DECIMAL)
 * mk_bf_ro(qcpld, cpld_minor_version, 0x00, 0, 4, NULL, BF_DECIMAL)
 *
 * Second example:  If register 0x10 controls the QSFP reset lines
 * (bit 0 goes to QSFP 1, bit 1 goes to QSFP 2, etc.), the we
 * can expose it like this:
 *
 * mk_bf_rw(qcpld, qsfp_reset, 0x10, 0, 8, NULL, 0)
 *
 * And it's a read-write register (mk_bf_rw()).
 *
 * Of course, there are really 32 port and the reset lines
 * are registers 0x10 to 0x13, then you can put them all
 * together:
 *
 * mk_bf_rw(qcpld, qsfp_reset, 0x10, 0, 32, NULL, 0)
 *
 * Now, the sysfs node qsfp_reset contains a 32-bit number,
 * displayed in hex.  Each bit reflects the state of a reset signal.
 *
 * Everything is little-endian.  Add big-endian support if you
 * need it.
 *
 * But it turns out that the bits are low-active (like
 * QSFP reset pins are), but we want to present a high-active
 * interface.  So do this:
 *
 * mk_bf_rw(qcpld, qsfp_reset, 0x10, 0, 32, NULL, BF_COMPLEMENT)
 *
 * We can also create a sysfs node for each line, if that's what
 * we want:
 *
 * mk_bf_rw(qcpld, qsfp1_reset,  0x10, 0, 1, NULL, BF_COMPLEMENT)
 * mk_bf_rw(qcpld, qsfp2_reset,  0x10, 1, 1, NULL, BF_COMPLEMENT)
 * ...
 * mk_bf_rw(qcpld, qsfp8_reset,  0x10, 7, 1, NULL, BF_COMPLEMENT)
 * mk_bf_rw(qcpld, qsfp9_reset,  0x11, 0, 1, NULL, BF_COMPLEMENT)
 * mk_bf_rw(qcpld, qsfp10_reset, 0x11, 1, 1, NULL, BF_COMPLEMENT)
 * ...
 * mk_bf_rw(qcpld, qsfp31_reset, 0x13, 6, 1, NULL, BF_COMPLEMENT)
 * mk_bf_rw(qcpld, qsfp32_reset, 0x13, 7, 1, NULL, BF_COMPLEMENT)
 *
 * Now, say, we have a 3-bit, read-write field named "led_color"
 * starting at bit 2 in register 0x07:
 *
 * mk_bf_rw(qcpld, led_color, 0x07, 2, 3, NULL, 0)
 *
 * The sysfs node displays the values in hex: 0x0 to 0x7.
 *
 * By the way, writing the value generates two CPLD operations:
 * a read and a write.  Keep this in mind.
 *
 * To give the values nice names, define an array of strings:
 * static const char * const colors_values[] = {
 *    "off", "red", "green", "yellow",
 *    "blue", "magenta", "cyan", "white"
 * };
 * mk_bf_rw(qcpld, color, 0x07, 2, 3, color_values, 0)
 *
 * The names correspond to numeric values 0, 1, ..., 7 (duh!). For LED
 * colors you should use the defines in platform-def.h.
 *
 * But we forgot something.  Before any of this, we need to
 * define two accessor functions:
 *
 * static int qcpld_read_reg(struct device *dev,
 *                           int reg,
 *                           int nregs,
 *                           u32 *val)
 * {
 *     // Read register(s) reg to (reg - nregs - 1) into *val.
 *     // Return 0 or -errno on failure.
 *     //
 *     // Maybe like this:
 *     struct i2c_client *client = to_i2c_client(dev);
 *     int i;
 *
 *     *val = 0;
 *     for (i = 0; i < nregs; i++, reg++) {
 *         int ret = i2c_smbus_read_byte_data(client, reg);
 *         if (ret < 0)
 *             return ret;
 *         *val |= ret << i * 8;
 *     }
 *     return 0;
 * }
 *
 * static int qcpld_write_reg(struct device *dev,
 *                            int reg,
 *                            int nregs,
 *                            u32 val)
 * {
 *     // Write val to register(s) reg to (reg - nregs - 1).
 *     // Return 0 or -errno on failure.
 *     //
 *     // Maybe like this:
 *     struct i2c_client *client = to_i2c_client(dev);
 *
 *     for (; nregs > 0; nregs--, reg++, val >>= 8) {
 *         int ret = i2c_smbus_write_byte_data(client, reg, val);
 *         if (ret < 0)
 *             return ret;
 *     }
 *     return 0;
 * }
 *
 * However, because i2c devices are so common, we have predefined
 * functions for them, so this is enough:
 *
 * static int qcpld_read_reg(struct device *dev,
 *                           int reg,
 *                           int nregs,
 *                           u32 *val)
 * {
 *     return cumulus_bf_i2c_read_reg(dev, reg, nregs, val);
 * }
 * static int qcpld_write_reg(struct device *dev,
 *                            int reg,
 *                            int nregs,
 *                            u32 val)
 * {
 *     return cumulus_bf_i2c_write_reg(dev, reg, nregs, val);
 * }
 *
 * Or just do this:
 * #define qcpld_read_reg cumulus_bf_i2c_read_reg
 * #define qcpld_write_reg cumulus_bf_i2c_write_reg
 *
 * Finally, at the end of all the field definitions,
 * we need this bit of boilerplate:
 *
 * struct attribute *qcpld_attrs[] = {
 *     &qcpld_major_version.attr,
 *     &qcpld_minor_version.attr,
 *     &qcpld_qsfp_reset.attr,
 *     &qcpld_led_color.attr,
 *     NULL
 * };
 * static struct attribute_group qcpld_attr_group = {
 *     .attrs = qcpld_attrs,
 * };
 *
 * And at some point in the device's probe function, we need to call
 * sysfs_create_group() with qcpld_attr_group, maybe like this:
 *
 * ret = sysfs_create_group(&dev->dev.kobj, &qcpld_attr_group)
 * if (ret) {
 *     dev_err(&dev->dev, "sysfs_create_group failed\n");
 *     return ret;
 * }
 *
 * A few things to keep in mind:
 *
 * Maximum value string length is PLATFORM_LED_COLOR_NAME_SIZE - 1,
 * but isn't enforced anywhere.  Also not enforced is the size
 * of the values array, which of course must match the field width.
 * So be cool.
 *
 * Fields can span up to 4 registers (when offset + width > 8),
 * little endian.  This limit is also not enforced.
 *
 * In general, given a w-bit wide field at offset o starting
 * at register r, enough registers are accessed to cover
 * o + w bits, starting at register r.  This is true even
 * if o is greater than 8.  Writing a field reads the current
 * value first, unless o is 0 and w is a multiple of 8.
 *
 * To complement the bits, give the BF_COMPLEMENT flag (for example,
 * to present a high-active interface to a low-active bit).
 *
 * To show a value in decimal, give the BF_DECIMAL flag.
 *
 * To show a value as a signed number, give the BF_SIGNED flag.
 * Don't use this with string values.  Bad things may happen.
 *
 * Writes to a field (decimal or not) can be in any base, but
 * must match BF_SIGNED, and fit in width bits.
 * For example, for a 4-bit value, the input must be in
 * the interval [-8, 7] if signed, or [0, 15] if unsigned.
 */

struct bf {
	const char *name;
	const char * const *values;
	u8 flags;
	u16 reg;
	u32 reg32;
	u8 shift;
	u8 width;
};

/* flags */
#define BF_COMPLEMENT	0x01		/* complement value */
#define BF_DECIMAL	0x02		/* show value in decimal */
#define BF_SIGNED	0x04		/* value is signed */

#define mk_bf_rw(_prefix, _name, _reg, _shift, _width, _values, _flags) \
	mk_bf_struct(_prefix, _name, _reg, _shift, _width, _values, _flags); \
	mk_bf_show(_prefix, _name) \
	mk_bf_store(_prefix, _name) \
	static struct device_attribute _prefix##_##_name = \
		__ATTR(_name, 0644, \
		       _prefix##_##_name##_show, _prefix##_##_name##_store)

#define mk_bf_ro(_prefix, _name, _reg, _shift, _width, _values, _flags) \
	mk_bf_struct(_prefix, _name, _reg, _shift, _width, _values, _flags); \
	mk_bf_show(_prefix, _name) \
	static struct device_attribute _prefix##_##_name = \
		__ATTR(_name, 0444, _prefix##_##_name##_show, NULL)

#define mk_bf_struct(_prefix, _name, _reg, _shift, _width, _values, _flags) \
	static struct bf _prefix##_##_name##_bf = { \
		.name = #_name, \
		.reg = (_reg), \
		.shift = (_shift), \
		.width = (_width), \
		.values = (_values), \
		.flags = (_flags), \
	}

#define mk_bf_show(_prefix, _name) \
	static ssize_t \
	_prefix##_##_name##_show(struct device *_dev, \
				 struct device_attribute *_dattr, \
				 char *_buf) \
	{ \
		return cumulus_bf_show(_dev, _dattr, _buf, \
				       &_prefix##_##_name##_bf, \
				       _prefix##_read_reg); \
	}

#define mk_bf_store(_prefix, _name) \
	static ssize_t \
	_prefix##_##_name##_store(struct device *_dev, \
				  struct device_attribute *_dattr, \
				  const char *_buf, \
				  size_t _size) \
	{ \
		return cumulus_bf_store(_dev, _dattr, _buf, _size, \
					&_prefix##_##_name##_bf, \
					_prefix##_read_reg, \
					_prefix##_write_reg); \
	}

/* Shortcut macros for defining cpld bits, fields and registers */
#define cpld_bf_ro(_name, _reg, _field, _values, _flags) \
	mk_bf_ro(cpld, _name, _reg, _field##_LSB, FIELD_WIDTH(_field), \
		 _values, _flags)

#define cpld_bf_rw(_name, _reg, _field, _values, _flags) \
	mk_bf_rw(cpld, _name, _reg, _field##_LSB, FIELD_WIDTH(_field), \
		 _values, _flags)

#define cpld_bt_ro(_name, _reg, _field, _values, _flags) \
	mk_bf_ro(cpld, _name, _reg, _field##_BIT, 1, _values, _flags)

#define cpld_bt_rw(_name, _reg, _field, _values, _flags) \
	mk_bf_rw(cpld, _name, _reg, _field##_BIT, 1, _values, _flags)

#define cpld_rg_ro(_name, _reg, _values, _flags) \
	mk_bf_ro(cpld, _name, _reg, 0, 8, _values, _flags)

#define cpld_rg_rw(_name, _reg, _values, _flags) \
	mk_bf_rw(cpld, _name, _reg, 0, 8, _values, _flags)

/* Same as above, but for 32-bit device addresses */
#define mk_bf_rw32(_prefix, _name, _reg, _shift, _width, _values, _flags) \
	mk_bf_struct32(_prefix, _name, _reg, _shift, _width, _values, _flags); \
	mk_bf_show32(_prefix, _name) \
	mk_bf_store32(_prefix, _name) \
	static struct device_attribute _prefix##_##_name = \
		__ATTR(_name, 0644, \
		       _prefix##_##_name##_show, _prefix##_##_name##_store)

#define mk_bf_ro32(_prefix, _name, _reg, _shift, _width, _values, _flags) \
	mk_bf_struct32(_prefix, _name, _reg, _shift, _width, _values, _flags); \
	mk_bf_show32(_prefix, _name) \
	static struct device_attribute _prefix##_##_name = \
		__ATTR(_name, 0444, _prefix##_##_name##_show, NULL)

#define mk_bf_struct32(_prefix, _name, _reg, _shift, _width, _values, _flags) \
	static struct bf _prefix##_##_name##_bf = { \
		.name  = #_name, \
		.reg32 = (_reg), \
		.shift = (_shift), \
		.width = (_width), \
		.values = (_values), \
		.flags = (_flags), \
	}

#include <linux/cumulus-platform.h>

#define mk_bf_show32(_prefix, _name) \
	static ssize_t \
	_prefix##_##_name##_show(struct device *_dev, \
				 struct device_attribute *_dattr, \
				 char *_buf) \
	{ \
		return cumulus_bf_show32(_dev, _dattr, _buf, \
					 &_prefix##_##_name##_bf, \
					 _prefix##_read_reg); \
	}

#define mk_bf_store32(_prefix, _name) \
	static ssize_t \
	_prefix##_##_name##_store(struct device *_dev, \
				  struct device_attribute *_dattr, \
				  const char *_buf, \
				  size_t _size) \
	{ \
		return cumulus_bf_store32(_dev, _dattr, _buf, _size, \
					  &_prefix##_##_name##_bf, \
					  _prefix##_read_reg, \
					  _prefix##_write_reg); \
	}

/* Shortcut macros for defining cpld bits, fields and registers */
#define cpld_bf_ro32(_name, _reg, _field, _values, _flags) \
	mk_bf_ro32(cpld, _name, _reg, _field##_LSB, FIELD_WIDTH(_field), \
		 _values, _flags)

#define cpld_bf_rw32(_name, _reg, _field, _values, _flags) \
	mk_bf_rw32(cpld, _name, _reg, _field##_LSB, FIELD_WIDTH(_field), \
		 _values, _flags)

#define cpld_bt_ro32(_name, _reg, _field, _values, _flags) \
	mk_bf_ro32(cpld, _name, _reg, _field##_BIT, 1, _values, _flags)

#define cpld_bt_rw32(_name, _reg, _field, _values, _flags) \
	mk_bf_rw32(cpld, _name, _reg, _field##_BIT, 1, _values, _flags)

#define cpld_rg_ro32(_name, _reg, _values, _flags) \
	mk_bf_ro32(cpld, _name, _reg, 0, 8, _values, _flags)

#define cpld_rg_rw32(_name, _reg, _values, _flags) \
	mk_bf_rw32(cpld, _name, _reg, 0, 8, _values, _flags)

#endif /* PLATFORM_BITFIELD_H__ */
