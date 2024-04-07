/*
 * CPLD sysfs driver for cel_redstone_xp.
 *
 * Copyright (C) 2014 Cumulus Networks, Inc.
 * Author: Alan Liebthal <alanl@cumulusnetworks.com>
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
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <linux/io.h>

#include "cel-redstone-xp-cpld.h"
#include "platform-defs.h"

static const char driver_name[] = "cel_redstone_xp_cpld";
#define DRIVER_VERSION "1.0"

#define CPLDNAMSZ 20

static uint8_t *cel_redstone_xp_cpld_regs;

/******************************************************************************
 *
 * CPLD I/O
 *
 */
static uint8_t cpld_reg_read(uint32_t reg)
{
	uint8_t data;

	data = ioread8(cel_redstone_xp_cpld_regs + reg - CPLD_IO_BASE);
	return data;
}

static void cpld_reg_write(uint32_t reg, uint8_t data)
{
	iowrite8(data, cel_redstone_xp_cpld_regs + reg - CPLD_IO_BASE);
}

#define CEL_REDSTONE_XP_CPLD_STRING_NAME_SIZE 30

/*
 * write protect
 */
static ssize_t write_protect_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	uint8_t data;

	data = cpld_reg_read(CPLD1_REG_WRITE_PROTECT_CTL_OFFSET);
	return sprintf(buf, "0x%02X\n", data);
}

static ssize_t write_protect_store(struct device *dev,
				   struct device_attribute *dattr,
				   const char *buf, size_t count)
{
	int tmp;

	if (sscanf(buf, "%x", &tmp) != 1)
		return -EINVAL;
	cpld_reg_write(CPLD1_REG_WRITE_PROTECT_CTL_OFFSET, tmp);

	return count;
}
static SYSFS_ATTR_RW(write_protect, write_protect_show, write_protect_store);

/*
 * cpld version
 */
static ssize_t cpld_version_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	uint8_t tmp;
	uint8_t type;
	uint8_t rev;

	tmp = cpld_reg_read(CPLD1_REG_VERSION_OFFSET);
	type = (tmp & CPLD1_VERSION_H_MASK) >> CPLD1_VERSION_H_SHIFT;
	rev  = (tmp & CPLD1_VERSION_L_MASK)  >> CPLD1_VERSION_L_SHIFT;

	return sprintf(buf, "%d.%d\n", type, rev);
}
static SYSFS_ATTR_RO(cpld_version, cpld_version_show);

/*
 * cpld-2 version
 */
static ssize_t cpld_2_version_show(struct device *dev,
				   struct device_attribute *dattr,
				   char *buf)
{
	uint8_t tmp;
	uint8_t type;
	uint8_t rev;

	tmp = cpld_reg_read(CPLD2_REG_VERSION_OFFSET);
	type = (tmp & CPLD2_VERSION_H_MASK) >> CPLD2_VERSION_H_SHIFT;
	rev = (tmp & CPLD2_VERSION_L_MASK)  >> CPLD2_VERSION_L_SHIFT;

	return sprintf(buf, "%d.%d\n", type, rev);
}
static SYSFS_ATTR_RO(cpld2_version, cpld_2_version_show);

/*
 * cpld version-3
 */
static ssize_t cpld3_version_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	uint8_t tmp;
	uint8_t type;
	uint8_t rev;

	tmp = cpld_reg_read(CPLD3_REG_VERSION_OFFSET);
	type = (tmp & CPLD3_VERSION_H_MASK) >> CPLD3_VERSION_H_SHIFT;
	rev  = (tmp & CPLD3_VERSION_L_MASK)  >> CPLD3_VERSION_L_SHIFT;

	return sprintf(buf, "%d.%d\n", type, rev);
}
static SYSFS_ATTR_RO(cpld3_version, cpld3_version_show);

/*
 * cpld version-4
 */
static ssize_t cpld4_version_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	uint8_t tmp;
	uint8_t type;
	uint8_t rev;

	tmp = cpld_reg_read(CPLD4_REG_VERSION_OFFSET);
	type = (tmp & CPLD4_VERSION_H_MASK) >> CPLD4_VERSION_H_SHIFT;
	rev  = (tmp & CPLD4_VERSION_L_MASK)  >> CPLD4_VERSION_L_SHIFT;

	return sprintf(buf, "%d.%d\n", type, rev);
}
static SYSFS_ATTR_RO(cpld4_version, cpld4_version_show);

/*
 * cpld version-5
 */
static ssize_t cpld5_version_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	uint8_t tmp;
	uint8_t type;
	uint8_t rev;

	tmp = cpld_reg_read(CPLD5_REG_VERSION_OFFSET);
	type = (tmp & CPLD5_VERSION_H_MASK) >> CPLD5_VERSION_H_SHIFT;
	rev  = (tmp & CPLD5_VERSION_L_MASK)  >> CPLD5_VERSION_L_SHIFT;

	return sprintf(buf, "%d.%d\n", type, rev);
}
static SYSFS_ATTR_RO(cpld5_version, cpld5_version_show);

/*------------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[CEL_REDSTONE_XP_CPLD_STRING_NAME_SIZE];
	uint32_t reg;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_system",
		.reg  = CPLD4_REG_SYS_LED_CTRL_OFFSET,
		.mask = CPLD4_SYS_LED_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD4_SYS_LED_GREEN},
			{ PLATFORM_LED_GREEN_SLOW_BLINKING,
			  CPLD4_SYS_LED_GREEN_SLOW_BLINK},
			{ PLATFORM_LED_GREEN_BLINKING,
			  CPLD4_SYS_LED_GREEN_FAST_BLINK},
			{ PLATFORM_LED_OFF, CPLD4_SYS_LED_OFF},
		},
	},
	{
		.name = "led_psu1",
		.reg = CPLD4_REG_SYS_LED_CTRL_OFFSET,
		.mask = CPLD4_PSU1_LED_L,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN, 0},
			{ PLATFORM_LED_OFF, CPLD4_PSU1_LED_L},
		}
	},
	{
		.name = "led_psu2",
		.reg = CPLD4_REG_SYS_LED_CTRL_OFFSET,
		.mask = CPLD4_PSU2_LED_L,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN, 0},
			{ PLATFORM_LED_OFF, CPLD4_PSU2_LED_L},
		}
	},
};

static int n_leds = ARRAY_SIZE(cpld_leds);

/*
 * LEDs
 */
static ssize_t led_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	uint8_t tmp;
	int i;
	struct led *target = NULL;

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (!target)
		return sprintf(buf, "undefined target\n");

	/* read the register */
	tmp = cpld_reg_read(target->reg);

	/* find the color */
	tmp &= target->mask;
	for (i = 0; i < target->n_colors; i++)
		if (tmp == target->colors[i].value)
			break;
	if (i == target->n_colors)
		return sprintf(buf, "undefined color\n");
	else
		return sprintf(buf, "%s\n", target->colors[i].name);
}

static ssize_t led_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	uint8_t tmp;
	int i;
	struct led *target = NULL;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];

	/* find the target led */
	for (i = 0; i < n_leds; i++)
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	if (!target)
		return -EINVAL;

	/* find the color */
	if (sscanf(buf, "%19s", raw) <= 0)
		return -EINVAL;
	for (i = 0; i < target->n_colors; i++)
		if (strcmp(raw, target->colors[i].name) == 0)
			break;
	if (i == target->n_colors)
		return -EINVAL;

	tmp = cpld_reg_read(target->reg);
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;
	cpld_reg_write(target->reg, tmp);

	return count;
}
static SYSFS_ATTR_RW(led_system, led_show, led_store);
static SYSFS_ATTR_RW(led_psu1, led_show, led_store);
static SYSFS_ATTR_RW(led_psu2, led_show, led_store);

int hex_to_int(const char *hex_str, uint32_t *val_ptr)
{
	char prefix[] = "0x";

	if (strncasecmp(hex_str, prefix, strlen(prefix)) == 0) {
		hex_str += strlen(prefix);
		return sscanf(hex_str, "%x", val_ptr) != 1;
	}
	return sscanf(hex_str, "%u", val_ptr) != 1;
}

static int hex_to_int64(const char *hex_str, uint64_t *val)
{
	char prefix[] = "0x";

	if (strncasecmp(hex_str, prefix, strlen(prefix)) == 0)
		hex_str += strlen(prefix);

	return sscanf(hex_str, "%llx", val) != 1;
}

/******************************************************
 *
 * SFP
 *
 *****************************************************/
#define NUM_SFP_REGS 8
#define REG_NAME_LEN 16

struct redstone_xp_sfp_reg_offset_info {
	uint8_t  mask;
	uint8_t  shift;
};

struct redstone_xp_sfp_reg_info {
	char node_name[REG_NAME_LEN];
	uint32_t regs[NUM_SFP_REGS];
	int      active_low;
};

static struct redstone_xp_sfp_reg_info redxp_sfp_reg_info[] = {
	{
		.node_name = "sfp_rx_los",
		.active_low = 1,
		.regs = {
			CPLD2_SFP_1_8_RX_LOS_REGISTER,
			CPLD2_SFP_9_16_RX_LOS_REGISTER,
			CPLD2_SFP_17_18_RX_LOS_REGISTER,
			CPLD3_SFP_19_26_RX_LOS_REGISTER,
			CPLD3_SFP_27_34_RX_LOS_REGISTER,
			CPLD3_SFP_35_36_RX_LOS_REGISTER,
			CPLD5_SFP_37_44_RX_LOS_REGISTER,
			CPLD5_SFP_45_48_RX_LOS_REGISTER
		},
	},
	{
		.node_name = "sfp_tx_disable",
		.regs = {
			CPLD2_SFP_1_8_TX_DISABLE_REGISTER,
			CPLD2_SFP_9_16_TX_DISABLE_REGISTER,
			CPLD2_SFP_17_18_TX_DISABLE_REGISTER,
			CPLD3_SFP_19_26_TX_DISABLE_REGISTER,
			CPLD3_SFP_27_34_TX_DISABLE_REGISTER,
			CPLD3_SFP_35_36_TX_DISABLE_REGISTER,
			CPLD5_SFP_37_44_TX_DISABLE_REGISTER,
			CPLD5_SFP_45_48_TX_DISABLE_REGISTER
		},
	},
	{
		.node_name = "sfp_tx_fault",
		.regs = {
			CPLD2_SFP_1_8_TX_FAULT_REGISTER,
			CPLD2_SFP_9_16_TX_FAULT_REGISTER,
			CPLD2_SFP_17_18_TX_FAULT_REGISTER,
			CPLD3_SFP_19_26_TX_FAULT_REGISTER,
			CPLD3_SFP_27_34_TX_FAULT_REGISTER,
			CPLD3_SFP_35_36_TX_FAULT_REGISTER,
			CPLD5_SFP_37_44_TX_FAULT_REGISTER,
			CPLD5_SFP_45_48_TX_FAULT_REGISTER
		},
	},
	{
		.node_name = "sfp_present",
		.active_low = 1,
		.regs = {
			CPLD2_SFP_1_8_PRESENT_REGISTER,
			CPLD2_SFP_9_16_PRESENT_REGISTER,
			CPLD2_SFP_17_18_PRESENT_REGISTER,
			CPLD3_SFP_19_26_PRESENT_REGISTER,
			CPLD3_SFP_27_34_PRESENT_REGISTER,
			CPLD3_SFP_35_36_PRESENT_REGISTER,
			CPLD5_SFP_37_44_PRESENT_REGISTER,
			CPLD5_SFP_45_48_PRESENT_REGISTER
		},
	}
};

#define NUM_SFP_REG_NAMES (sizeof(redxp_sfp_reg_info) / \
sizeof(struct redstone_xp_sfp_reg_info))

static struct redstone_xp_sfp_reg_offset_info redxp_reg_offset_info[] = {
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

static ssize_t sfp_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	struct redstone_xp_sfp_reg_info *reg_info = NULL;
	uint64_t val = 0, rval;
	int i;

	for (i = 0; i < NUM_SFP_REG_NAMES; i++)
		if (strcmp(redxp_sfp_reg_info[i].node_name,
			   dattr->attr.name) == 0) {
			reg_info = &redxp_sfp_reg_info[i];
			break;
		}

	for (i = 0; i < NUM_SFP_REGS; i++) {
		rval = cpld_reg_read(reg_info->regs[i]);
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
	struct redstone_xp_sfp_reg_offset_info *reg_info;
	uint64_t  data;
	uint32_t *regs = NULL;
	int i;

	if (hex_to_int64(buf, &data))
		return -EINVAL;

	for (i = 0; i < NUM_SFP_REG_NAMES; i++)
		if (strcmp(redxp_sfp_reg_info[i].node_name,
			   dattr->attr.name) == 0) {
			regs = redxp_sfp_reg_info[i].regs;
			break;
		}
	for (i = 0; i < NUM_SFP_REGS; i++) {
		reg_info = &redxp_reg_offset_info[i];
		cpld_reg_write(regs[i],
			       (data >> reg_info->shift) & reg_info->mask);
	}

	return count;
}
static SYSFS_ATTR_RO(sfp_rx_los,     sfp_show);
static SYSFS_ATTR_RW(sfp_tx_disable, sfp_show, sfp_store);
static SYSFS_ATTR_RO(sfp_tx_fault,   sfp_show);
static SYSFS_ATTR_RO(sfp_present,    sfp_show);

/******************************************************
 *
 * QSFP
 *
 *****************************************************/
/*
 * used to set the LED mode on the QSFP ports
 * bits 0 -> 5 are QSFP1 -> QSFP6 which are SWP49 -> SWP54
 * a 1 means 4x10G mode.
 */
static ssize_t qsfp_led_mode_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	return sprintf(buf, "0x%02X\n",
		       cpld_reg_read(CPLD5_REG_QSFP_PORT_MODE_OFFSET));
}

static ssize_t qsfp_led_mode_store(struct device *dev,
				   struct device_attribute *dattr,
				   const char *buf, size_t count)
{
	uint32_t val;

	if (hex_to_int(buf, &val))
		return -EINVAL;
	cpld_reg_write(CPLD5_REG_QSFP_PORT_MODE_OFFSET, (uint8_t)val);

	return count;
}

static SYSFS_ATTR_RW(qsfp_led_mode, qsfp_led_mode_show, qsfp_led_mode_store);

/*
 * qsfp_reset
 *  a bit mask for 6 qsfp ports [0:5]
 *  this bit is low active in hardware, but we hide that fact,
 *  so a 1 means reset
 */
static ssize_t qsfp_reset_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	uint8_t rd_data = cpld_reg_read(CPLD4_QSFP_RESET_CONTROL_REGISTER);

	return sprintf(buf, "0x%02X\n", (~rd_data) & CPLD4_RESET_CONTROL_MASK);
}

static ssize_t qsfp_reset_store(struct device *dev,
				struct device_attribute *dattr,
				const char *buf, size_t count)
{
	unsigned long int  data;

	if (kstrtoul(buf, 0, &data) < 0)
		return -EINVAL;
	data = ~(data & CPLD4_RESET_CONTROL_MASK);
	cpld_reg_write(CPLD4_QSFP_RESET_CONTROL_REGISTER, data);

	return count;
}
static SYSFS_ATTR_RW(qsfp_reset, qsfp_reset_show, qsfp_reset_store);

/*
 * qsfp_lp_mode
 *  a bit mask for 6 qsfp ports [0:5]
 *  set a port's bit to 1 for low power mode
 */
static ssize_t qsfp_lp_mode_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	uint8_t rd_data = cpld_reg_read(CPLD4_QSFP_LP_MODE_REGISTER);

	return sprintf(buf, "0x%02X\n", rd_data & CPLD4_LP_MODE_MASK);
}

static ssize_t qsfp_lp_mode_store(struct device *dev,
				  struct device_attribute *dattr,
				  const char *buf, size_t count)
{
	unsigned long int  data;

	if (kstrtoul(buf, 0, &data) < 0)
		return -EINVAL;
	cpld_reg_write(CPLD4_QSFP_LP_MODE_REGISTER,
		       (uint8_t)data & CPLD4_LP_MODE_MASK);

	return count;
}
static SYSFS_ATTR_RW(qsfp_lp_mode, qsfp_lp_mode_show, qsfp_lp_mode_store);

/*
 * qsfp_present
 *  a bit mask for 6 qsfp ports [0:5]
 *  this bit is low active in hardware, but we hide that fact so
 *  a 1 means QSFP is present
 */
static ssize_t qsfp_present_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	uint8_t rd_data = cpld_reg_read(CPLD4_QSFP_PRESENT_REGISTER);

	return sprintf(buf, "0x%02X\n", (~rd_data) & CPLD4_PRESENT_MASK);
}
static SYSFS_ATTR_RO(qsfp_present, qsfp_present_show);

/*
 * qsfp_interrupt
 *  a bit mask for 6 qsfp ports [0:5]
 *  a 1 indicates interrupt
 */
static ssize_t qsfp_interrupt_show(struct device *dev,
				   struct device_attribute *dattr,
				   char *buf)
{
	uint8_t rd_data = cpld_reg_read(CPLD4_QSFP_INTERRUPT_REGISTER);

	return sprintf(buf, "0x%02X\n", rd_data & CPLD4_INTERRUPT_MASK);
}
static SYSFS_ATTR_RO(qsfp_interrupt, qsfp_interrupt_show);

/*
 * qsfp_led_error
 *
 * Turn a qsfp port's error led on/off
 *
 * 24 bit mask, 6 ports, 4 bits per port
 *  1 => no error, 0 => error
 */

static uint32_t qsfp_error_led_regs[] = {
	CPLD5_REG_PORT_1_2_ERROR_LED_OFFSET,
	CPLD5_REG_PORT_3_4_ERROR_LED_OFFSET,
	CPLD5_REG_PORT_5_6_ERROR_LED_OFFSET,
};

static ssize_t qsfp_error_led_show(struct device *dev,
				   struct device_attribute *dattr,
				   char *buf)
{
	uint32_t value;
	int i;

	value = 0;
	for (i = 0; i < sizeof(qsfp_error_led_regs) / sizeof(uint32_t); i++)
		value |= (cpld_reg_read(qsfp_error_led_regs[i]) << (i * 8));
	return sprintf(buf, "%x\n", value);
}

static ssize_t qsfp_error_led_store(struct device *dev,
				    struct device_attribute *dattr,
				    const char *buf, size_t count)
{
	uint32_t value;
	uint8_t data;
	int i;

	if (hex_to_int(buf, &value) != 0)
		return -EINVAL;
	for (i = 0; i < sizeof(qsfp_error_led_regs) / sizeof(uint32_t); i++) {
		data = (value >> (i * 8)) & 0xff;
		cpld_reg_write(qsfp_error_led_regs[i], data);
	}
	return count;
}

static SYSFS_ATTR_RW(qsfp_led_error, qsfp_error_led_show, qsfp_error_led_store);

/*
 * Help / README
 *
 * The string length must be less than 4K.
 *
 */
#define HELP_STR "Description of the CPLD driver files:\n\
\n\
\n\
cpld_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for CPLD 1\n\
  CPLD version register,in the following format:\n\
\n\
  cpld_version\n\
\n\
  Example: 1.3\n\
\n\
\n\
cpld2_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for CPLD 2\n\
  CPLD version register,in the following format:\n\
\n\
  cpld_version\n\
\n\
  Example: 2.3\n\
\n\
\n\
cpld3_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for CPLD 3\n\
  CPLD version register,in the following format:\n\
\n\
  cpld_version\n\
\n\
  Example: 3.3\n\
\n\
\n\
cpld4_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for CPLD 4\n\
  CPLD version register,in the following format:\n\
\n\
  cpld_version\n\
\n\
  Example: 4.3\n\
\n\
\n\
cpld5_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for CPLD 5\n\
  CPLD version register,in the following format:\n\
\n\
  cpld_version\n\
\n\
  Example: 5.3\n\
\n\
\n\
led_system\n\
\n\
  Read-Write:\n\
\n\
  System State LED color.\n\
  The following values are possible: green, green blinking,\n\
  green slow blinking and off.\n\
\n\
\n\
led_psu1\n\
led_psu2\n\
\n\
  Read-Only:\n\
\n\
  Indicates the operational status of the psu.\n\
   'green' - psu is installed and operational\n\
   'off'   - psu is not installed or has failed\n\
\n\
\n\
qsfp_led_mode\n\
\n\
  Read-Write:\n\
\n\
  Read or set a QSFP port's LED mode.\n\
  1 means 4x10G.  0 means 40G.\n\
\n\
  1 bit per QSFP port. The bit controls whether the QSFP port's \n\
  3 additional LEDs will light up.\n\
\n\
\n\
qsfp_reset\n\
\n\
  Read-Write:\n\
\n\
  Apply reset to the QSFP ports.\n\
  A bitmask with bits 0:5 for QSFP ports 49 - 54\n\
  Set a ports bit to 1 to initiate reset\n\
\n\
\n\
qsfp_lp_mode\n\
\n\
  Read-Write:\n\
\n\
  Put a QSFP in into or out of low power mode\n\
  A bitmask with bits 0:5 for QSFP ports 49 - 54\n\
  A 1 indicates low power mode.\n\
\n\
\n\
sfp_tx_disable\n\
\n\
  Read-Write:\n\
\n\
  Set TX Disable state on a SFP port.\n\
  A bitmask with bits 0:47 for QSFP ports 1 - 48\n\
  A 1 indicates transmit disabled.\n\
\n\
\n\
sfp_present\n\
\n\
  Read-Only:\n\
\n\
  Indicates whether there is an SFP present.\n\
  A bitmask with bits 0:47 for SFP ports 1 - 48\n\
  A 1 indicates module present.\n\
\n\
\n\
qsfp_present\n\
\n\
  Read-Only:\n\
\n\
  Indicates whether there is a QSFP present.\n\
  A bitmask with bits 0:5 for QSFP ports 49-54\n\
  A 1 indicates that the QSFP is present.\n\
\n\
\n\
qsfp_interrupt\n\
\n\
  Read-Only:\n\
\n\
  Indicate the interrupt status of the QSFP ports.\n\
  A bitmask with bits 0:5 for QSFP ports 49-54\n\
  A 1 indicates interrupt.\n\
\n\
\n\
qsfp_error_led\n\
\n\
  Read-Write:\n\
\n\
  Set the error state of a QSFP port led.\n\
  A mask of 24 bits, 6 ports - 4 bits per port\n\
    1 => no error, 0 => error\n\
\n\
\n\
help\n\
README\n\
\n\
  Read-Only:\n\
\n\
  The text you are reading now.\n\
\n\
"

static ssize_t help_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	return sprintf(buf, HELP_STR);
}

static SYSFS_ATTR_RO(help, help_show);
static SYSFS_ATTR_RO(README, help_show);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */
static struct attribute *cel_redstone_xp_cpld_attrs[] = {
	&dev_attr_cpld_version.attr,
	&dev_attr_led_system.attr,
	&dev_attr_led_psu1.attr,
	&dev_attr_led_psu2.attr,
	&dev_attr_write_protect.attr,
	&dev_attr_cpld2_version.attr,
	&dev_attr_cpld3_version.attr,
	&dev_attr_cpld4_version.attr,
	&dev_attr_cpld5_version.attr,
	&dev_attr_sfp_rx_los.attr,
	&dev_attr_sfp_tx_disable.attr,
	&dev_attr_sfp_tx_fault.attr,
	&dev_attr_sfp_present.attr,
	&dev_attr_qsfp_led_mode.attr,
	&dev_attr_qsfp_reset.attr,
	&dev_attr_qsfp_lp_mode.attr,
	&dev_attr_qsfp_present.attr,
	&dev_attr_qsfp_interrupt.attr,
	&dev_attr_qsfp_led_error.attr,
	&dev_attr_help.attr,
	&dev_attr_README.attr,
	NULL,
};

static struct attribute_group cel_redstone_xp_cpld_attr_group = {
	.attrs = cel_redstone_xp_cpld_attrs,
};

/*------------------------------------------------------------------------------
 *
 * module interface
 *
 */
static struct platform_device *cel_redstone_xp_cpld_device;

static int cel_redstone_xp_cpld_probe(struct platform_device *dev)
{
	int ret;

	cel_redstone_xp_cpld_regs = ioport_map(CPLD_IO_BASE, CPLD_IO_SIZE);
	if (!cel_redstone_xp_cpld_regs) {
		pr_err("cpld: unabled to map iomem\n");
		ret = -ENODEV;
		goto err_exit;
	}

	ret = sysfs_create_group(&dev->dev.kobj,
				 &cel_redstone_xp_cpld_attr_group);
	if (ret) {
		pr_err("cpld: sysfs_create_group failed for cpld driver");
		goto err_unmap;
	}

err_unmap:
	iounmap(cel_redstone_xp_cpld_regs);

err_exit:
	return ret;
}

static int cel_redstone_xp_cpld_remove(struct platform_device *dev)
{
	iounmap(cel_redstone_xp_cpld_regs);
	return 0;
}

static struct platform_driver cel_redstone_xp_cpld_driver = {
	.driver = {
		.name = "cel_redstone_xp_cpld",
		.owner = THIS_MODULE,
	},
	.probe = cel_redstone_xp_cpld_probe,
	.remove = cel_redstone_xp_cpld_remove,
};

static int __init cel_redstone_xp_cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&cel_redstone_xp_cpld_driver);
	if (rv)
		goto err_exit;

	cel_redstone_xp_cpld_device =
		platform_device_alloc("cel_redstone_xp_cpld", 0);
	if (!cel_redstone_xp_cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device\n");
		rv = -ENOMEM;
		goto err_unregister;
	}

	rv = platform_device_add(cel_redstone_xp_cpld_device);
	if (rv) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_dealloc;
	}
	return 0;

err_dealloc:
	platform_device_unregister(cel_redstone_xp_cpld_device);

err_unregister:
	platform_driver_unregister(&cel_redstone_xp_cpld_driver);

err_exit:
	pr_err(KERN_ERR "%s platform_driver_register failed (%i)\n",
	       driver_name, rv);
	return rv;
}

static void __exit cel_redstone_xp_cpld_exit(void)
{
	platform_driver_unregister(&cel_redstone_xp_cpld_driver);
	platform_device_unregister(cel_redstone_xp_cpld_device);
}

MODULE_AUTHOR("Alan Liebthal <alanl@cumulusnetworks.com>");
MODULE_DESCRIPTION("Platform CPLD driver for Celestica Inc., Redstone XP");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(cel_redstone_xp_cpld_init);
module_exit(cel_redstone_xp_cpld_exit);
