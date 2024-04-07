/*
 * 16x40G QSFP+ IO module driver definitions for dell_s6100 platform.
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
#ifndef DELL_S6100_MOD_16X40G_H__
#define DELL_S6100_MOD_16X40G_H__

#define S6100_MOD_16X40G_DRIVER_NAME	"dell-s6100-16x40G"

/**
 * cpld_reg_rd(): Read cpld i2c register
 *
 * @map: regmap configured for the cpld i2c registers
 * @reg: register offset
 * @val: pointer to hold 8-bit register read value
 *
 * Read an 8-bit cpld i2c value from register @reg and return the
 * value in @val.
 *
 * A value of zero is returned on success, a negative errno is
 * returned in error cases.
 */
static inline int
cpld_reg_rd(struct regmap *map, unsigned int reg, uint8_t *val)
{
	int rc;
	unsigned int val32 = 0;

	rc = regmap_read(map, reg, &val32);
	WARN(rc, "regmap_read(map, 0x%x,...) failed: (%d)\n", reg, rc);

	*val = val32 & 0xFF;
	return rc;
}

/**
 * cpld_reg_wr(): Write cpld i2c register
 *
 * @cpld_map: regmap configured for the cpld i2c registers
 * @reg: register offset
 * @val: 8-bit value to write
 *
 * Writes @val to the 8-bit cpld i2c register specified by @reg.
 *
 * A value of zero is returned on success, a negative errno is
 * returned in error cases.
 */
static inline int
cpld_reg_wr(struct regmap *map, unsigned int reg, uint8_t val)
{
	int rc;

	rc = regmap_write(map, reg, val);
	WARN(rc, "regmap_write(map, 0x%x, 0x%x) failed: (%d)\n",
	     reg, val, rc);

	return rc;
}

#define S6100_16X40G_CPLD_VERSION	0x00
#define S6100_16X40G_BOARD_TYPE		0x01
#define S6100_16X40G_SW_SCRATCH		0x02
#define S6100_16X40G_MODULE_ID		0x03
#define S6100_16X40G_SLOT_ID		0x04
#define S6100_16X40G_CPLD_SEP_RST0	0x05
#define S6100_16X40G_PORT_LED_OPMOD	0x09
#define S6100_16X40G_PORT_LED_TEST	0x0A
#define S6100_16X40G_MODULE_LED_CTRL	0x0B
#  define MODULE_LED_BEACON		  1
#  define MODULE_LED_STATUS_MASK	  0xFE
#  define MODULE_LED_STATUS_SHIFT	  0x1
#  define MODULE_LED_STATUS_NORMAL	  0x0
#  define MODULE_LED_STATUS_BOOTING	  0x1
#  define MODULE_LED_STATUS_MAJOR_FAULT	  0x2
#  define MODULE_LED_STATUS_MINOR_FAULT	  0x3
#define S6100_16X40G_QSFP_RESET_CTRL0	0x10
#define S6100_16X40G_QSFP_RESET_CTRL1	0x11
#define S6100_16X40G_QSFP_LPMOD_CTRL0	0x12
#define S6100_16X40G_QSFP_LPMOD_CTRL1	0x13
#define S6100_16X40G_QSFP_INT_STA0	0x14
#define S6100_16X40G_QSFP_INT_STA1	0x15
#define S6100_16X40G_QSFP_ABS_STA0	0x16
#define S6100_16X40G_QSFP_ABS_STA1	0x17
#define S6100_16X40G_QSFP_TRIG_MOD	0x20
#define S6100_16X40G_QSFP_COMBINE	0x21
#define S6100_16X40G_QSFP_INT_INT0	0x22
#define S6100_16X40G_QSFP_INT_INT1	0x23
#define S6100_16X40G_QSFP_ABS_INT0	0x24
#define S6100_16X40G_QSFP_ABS_INT1	0x25
#define S6100_16X40G_QSFP_INT_MASK0	0x26
#define S6100_16X40G_QSFP_INT_MASK1	0x27
#define S6100_16X40G_QSFP_ABS_MASK0	0x28
#define S6100_16X40G_QSFP_ABS_MASK1	0x29
#define S6100_16X40G_PHY_TRIG_MOD	0x30
#define S6100_16X40G_PHY_COMBINE	0x31
#define S6100_16X40G_PHY_INT_STA0	0x32
#define S6100_16X40G_PHY_INT_INT0	0x33
#define S6100_16X40G_PHY_INT_MASK0	0x34

#endif /* DELL_S6100_MOD_16X40G_H__ */
