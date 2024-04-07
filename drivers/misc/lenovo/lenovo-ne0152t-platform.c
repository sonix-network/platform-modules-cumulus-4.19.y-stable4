// SPDX-License-Identifier: GPL-2.0+
/*
 * Lenovo NE0152T platform driver
 *
 * Copyright (C) 2018, 2019 Cumulus Networks, Inc.
 * Author: David Yen (dhyen@cumulusnetworks.com)
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
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/sysfs.h>
#include <linux/hwmon-sysfs.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "platform-bitfield.h"

#define DRIVER_NAME	"lenovo_ne0152t_platform"
#define DRIVER_VERSION	"2.0"

/*
 * The platform has one i801 adapter.
 * The platform has one iSMT adapter.
 *
 * The i801 is connected to the following on the CPU board:
 *
 *    so-dimm-0 eeprom (0x52)
 *    so-dimm-0 temp (0x1a)
 *    6c49420d clock buffer (0x69)
 *
 * The i801 is connected to the following on the main switch board:
 *
 *    pca9548#1 8-channel mux (0x70)
 *	 0 sfp0 eeprom (0x50)
 *	 1 sfp1 eeprom (0x50)
 *	 2 sfp2 eeprom (0x50)
 *	 3 sfp3 eeprom (0x50)
 *	 4 lm75 (0x48)
 *	 5 lm75 (0x49)
 *	 6 lm75 on fan board (0x4a)
 *	 7 lm75 on CPU board (0x4b)
 *    pca9548#2 8-channel mux (0x71)
 *	 0 psu0 eeprom (0x50)
 *	   psu0 (0x58)
 *	 1 psu1 eeprom (0x51)
 *	   psu1 (0x59)
 *	 2 <none>
 *	 3 rtc (0x68)
 *	 4 <none>
 *	 5 gl850g-31 usb hub (0x2c)
 *	 6 <none>
 *	 7 <none>
 *
 * The iSMT is connected to the following on the CPU board:
 *
 *    cpu eeprom (0x57)
 *
 * The iSMT is connected to the following on the main switch board:
 *
 *    cpld (0x60)
 *    vpd eeprom (0x51)
 *    cfg eeprom (0x52)
 *    ONIE (board) eeprom (0x53)
 *
 */

enum {
	I2C_I801_BUS = -1,
	I2C_ISMT_BUS,

	/* pca9548 mux1 */
	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,
	I2C_MUX1_BUS4,
	I2C_MUX1_BUS5,
	I2C_MUX1_BUS6,
	I2C_MUX1_BUS7,

	/* pca9548 mux2 */
	I2C_MUX2_BUS0 = 20,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,
	I2C_MUX2_BUS4,
	I2C_MUX2_BUS5,
	I2C_MUX2_BUS6,
	I2C_MUX2_BUS7,
};

/*
 * The list of i2c devices and their bus connections for this platform.
 *
 * First we construct the necessary data struction for each device,
 * using the method specific to the device type.  Then we put them
 * all together in a big table (see i2c_devices below).
 *
 * For muxes, we specify the starting bus number for the block of ports,
 * using the magic mk_pca954*() macros.
 *
 * For eeproms, including ones in the qsfp+ transceivers,
 * we specify the label, i2c address, size, and some flags,
 * all done in mk*_eeprom() macros.  The label is the string
 * that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 *
 * See the comment below for gpio.
 */

mk_pca9548(mux1,  I2C_MUX1_BUS0, 1);
mk_pca9548(mux2,  I2C_MUX2_BUS0, 1);

mk_eeprom(board,  57, 256, AT24_FLAG_IRUGO);
mk_eeprom(vpd,	  51, 256, AT24_FLAG_IRUGO);
mk_eeprom(cfg,	  52, 256, AT24_FLAG_IRUGO);
mk_eeprom(main,	  53, 256, AT24_FLAG_IRUGO);

mk_eeprom(psu0,	  50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu1,	  51, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

mk_port_eeprom(port49, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port50, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port51, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port52, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

/*
 * Main i2c device table
 *
 * We use the mk_i2cdev() macro to construct the entries.
 * Each entry is a bus number and a i2c_board_info.
 * The i2c_board_info structure specifies the device type, address,
 * and platform data specific to the device type.
 */

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_I801_BUS,	 "pca9548", 0x70, &mux1_platform_data),
	mk_i2cdev(I2C_I801_BUS,	 "pca9548", 0x71, &mux2_platform_data),
	mk_i2cdev(I2C_ISMT_BUS,	 "24c02",   0x57, &board_57_at24),
	mk_i2cdev(I2C_ISMT_BUS,	 "cpld",    0x60, NULL),
	mk_i2cdev(I2C_ISMT_BUS,	 "24c02",   0x51, &vpd_51_at24),
	mk_i2cdev(I2C_ISMT_BUS,	 "24c02",   0x52, &cfg_52_at24),
	mk_i2cdev(I2C_ISMT_BUS,	 "24c02",   0x53, &main_53_at24),

	/* devices on mux1 */
	mk_i2cdev(I2C_MUX1_BUS0, "24c02",   0x50, &port49_50_at24),
	mk_i2cdev(I2C_MUX1_BUS1, "24c02",   0x50, &port50_50_at24),
	mk_i2cdev(I2C_MUX1_BUS2, "24c02",   0x50, &port51_50_at24),
	mk_i2cdev(I2C_MUX1_BUS3, "24c02",   0x50, &port52_50_at24),
	mk_i2cdev(I2C_MUX1_BUS4, "lm77",    0x48, NULL),
	mk_i2cdev(I2C_MUX1_BUS5, "lm75",    0x49, NULL),
	mk_i2cdev(I2C_MUX1_BUS6, "lm75",    0x4a, NULL),
	mk_i2cdev(I2C_MUX1_BUS7, "lm75",    0x4b, NULL),

	/* devices on mux2 */
	mk_i2cdev(I2C_MUX2_BUS0, "24c02",   0x50, &psu0_50_at24),
	mk_i2cdev(I2C_MUX2_BUS1, "24c02",   0x51, &psu1_51_at24),
};

#define NUM_CPLD_DEVICES 1

/* Utility functions for I2C */

static struct i2c_client *cpld_devices[NUM_CPLD_DEVICES];
static int num_cpld_devices;

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

static int i2c_init(void)
{
	int i801_bus;
	int ismt_bus;
	int i;
	int ret;

	i801_bus = cumulus_i2c_find_adapter("SMBus I801 adapter");
	if (i801_bus < 0) {
		pr_err("Could not find the I801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	ismt_bus = cumulus_i2c_find_adapter("SMBus iSMT adapter");
	if (ismt_bus < 0) {
		pr_err("Could not find the iSMT adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	num_cpld_devices = 0;
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == I2C_I801_BUS)
			bus = i801_bus;
		else if (bus == I2C_ISMT_BUS)
			bus = ismt_bus;

		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_devices[i].client = client;
		if (strcmp(i2c_devices[i].board_info.type, "cpld") == 0)
			cpld_devices[num_cpld_devices++] = client;
	}
	return 0;

err_exit:
	i2c_exit();
	return ret;
}

/* bitfield accessor functions */

static int cpld_read_reg(struct device *dev, int reg, int nregs, u32 *val)
{
	int ret;
	int cpld_id = 0;

	ret = i2c_smbus_read_byte_data(cpld_devices[cpld_id], reg);
	if (ret < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, reg);
		return -EINVAL;
	}
	*val = ret;
	return 0;
}

static int cpld_write_reg(struct device *dev, int reg, int nregs, u32 val)
{
	int ret = 0;
	int cpld_id = 0;

	ret = i2c_smbus_write_byte_data(cpld_devices[cpld_id], reg, val);
	if (ret) {
		pr_err("CPLD write error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, reg);
	}
	return ret;
}

/* CPLD register bitfields with enum-like values */

#define PLATFORM_LED_AMBER_SLOW_BLINKING "amber_slow_blinking"
#define PLATFORM_LED_BLUE_SLOW_BLINKING	 "blue_slow_blinking"

static const char * const led_color_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN_BLINKING,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_OFF,
};

static const char * const system_led_color_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_BLUE_BLINKING,
	PLATFORM_LED_BLUE,
	PLATFORM_LED_OFF,
};

static const char * const fan_direction_values[] = {
	"B2F",
	"F2B",
};

static const char * const reset_button_values[] = {
	"not_pressed",
	"reserved",
	"pressed_and_released",
	"pressed_and_held",
};

/* CPLD registers */

mk_bf_ro(cpld, external_cpu,	   0x01, 6, 1, NULL, 0);
mk_bf_ro(cpld, poe_support,	   0x01, 5, 1, NULL, 0);
mk_bf_ro(cpld, product_id,	   0x01, 0, 4, NULL, 0);

mk_bf_rw(cpld, sfp0_tx_disable,	   0x02, 7, 1, NULL, 0);
mk_bf_ro(cpld, sfp0_present,	   0x02, 6, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, sfp0_tx_fault,	   0x02, 5, 1, NULL, 0);
mk_bf_ro(cpld, sfp0_rx_los,	   0x02, 4, 1, NULL, 0);

mk_bf_rw(cpld, sfp1_tx_disable,	   0x02, 3, 1, NULL, 0);
mk_bf_ro(cpld, sfp1_present,	   0x02, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, sfp1_tx_fault,	   0x02, 1, 1, NULL, 0);
mk_bf_ro(cpld, sfp1_rx_los,	   0x02, 0, 1, NULL, 0);

mk_bf_rw(cpld, sfp2_tx_disable,	   0x03, 7, 1, NULL, 0);
mk_bf_ro(cpld, sfp2_present,	   0x03, 6, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, sfp2_tx_fault,	   0x03, 5, 1, NULL, 0);
mk_bf_ro(cpld, sfp2_rx_los,	   0x03, 4, 1, NULL, 0);

mk_bf_rw(cpld, sfp3_tx_disable,	   0x03, 3, 1, NULL, 0);
mk_bf_ro(cpld, sfp3_present,	   0x03, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, sfp3_tx_fault,	   0x03, 1, 1, NULL, 0);
mk_bf_ro(cpld, sfp3_rx_los,	   0x03, 0, 1, NULL, 0);

mk_bf_rw(cpld, pca9548_reset,	   0x04, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, is_production_cpld, 0x0b, 7, 1, NULL, 0);
mk_bf_ro(cpld, cpld_rev,	   0x0b, 0, 6, NULL, 0);

mk_bf_ro(cpld, psu_pwr2_all_ok,	   0x11, 3, 1, NULL, 0);
mk_bf_ro(cpld, psu_pwr2_present,   0x11, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr1_all_ok,	   0x11, 1, 1, NULL, 0);
mk_bf_ro(cpld, psu_pwr1_present,   0x11, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr2_alert,	   0x12, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, psu_pwr2_power_on,  0x12, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr1_alert,	   0x12, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, psu_pwr1_power_on,  0x12, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_rw(cpld, i210_reset,	   0x18, 7, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, 10g_phy_reset,	   0x18, 6, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, 1g_phy6_reset,	   0x18, 5, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, 1g_phy5_reset,	   0x18, 4, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, 1g_phy4_reset,	   0x18, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, 1g_phy3_reset,	   0x18, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, 1g_phy2_reset,	   0x18, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, 1g_phy1_reset,	   0x18, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_rw(cpld, platform_reset,	   0x25, 7, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, global_reset,	   0x25, 6, 1, NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, mac_reset,	   0x29, 6, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, wd_reset,	   0x29, 5, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, usb_reset,	   0x29, 4, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, pcie1_reset,	   0x29, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, pcie0_reset,	   0x29, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, sw_reset,	   0x29, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, system_reset,	   0x29, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, reset_button,	   0x2f, 5, 2, reset_button_values, 0);
mk_bf_rw(cpld, button_en,	   0x2f, 2, 1, NULL, 0);
mk_bf_ro(cpld, push_button,	   0x2f, 0, 1, NULL, 0);

mk_bf_rw(cpld, led_system,	   0x32, 6, 2, system_led_color_values, 0);
mk_bf_rw(cpld, led_psu,		   0x32, 4, 2, led_color_values, 0);
mk_bf_rw(cpld, led_fan,		   0x32, 2, 2, led_color_values, 0);
mk_bf_rw(cpld, led_stacking,	   0x32, 0, 2, led_color_values, 0);

mk_bf_rw(cpld, onie_eeprom_wp,	   0x36, 2, 1, NULL, 0);
mk_bf_rw(cpld, nvram_eeprom_wp,	   0x36, 1, 1, NULL, 0);
mk_bf_rw(cpld, vpd_eeprom_wp,	   0x36, 0, 1, NULL, 0);

mk_bf_ro(cpld, fan1_present,	   0x37, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fan2_present,	   0x37, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fan3_present,	   0x37, 2, 1, NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, fan1_direction,	   0x37, 4, 1, fan_direction_values, 0);
mk_bf_ro(cpld, fan2_direction,	   0x37, 5, 1, fan_direction_values, 0);
mk_bf_ro(cpld, fan3_direction,	   0x37, 6, 1, fan_direction_values, 0);

mk_bf_rw(cpld, led_fan1,	   0x38, 0, 2, led_color_values, 0);
mk_bf_rw(cpld, led_fan2,	   0x38, 2, 2, led_color_values, 0);
mk_bf_rw(cpld, led_fan3,	   0x38, 4, 2, led_color_values, 0);

mk_bf_ro(cpld, pcb_rev_id,	   0x47, 0, 3, NULL, 0);
mk_bf_ro(cpld, pcb_rev,		   0x47, 3, 5, NULL, 0);

/* special case for fan speeds to get rpms */

#define FAN1R_SPEED_REG 0x26
#define FAN2R_SPEED_REG 0x27
#define FAN3R_SPEED_REG 0x28
#define FAN1_SPEED_REG	0x2c
#define FAN2_SPEED_REG	0x2d
#define FAN3_SPEED_REG	0x2e

static int fan_speed_reg[] = {
	FAN1R_SPEED_REG,
	FAN2R_SPEED_REG,
	FAN3R_SPEED_REG,
	FAN1_SPEED_REG,
	FAN2_SPEED_REG,
	FAN3_SPEED_REG,
};

#define NUM_FAN_SPEED_REGS ARRAY_SIZE(fan_speed_reg)

static ssize_t fan_show(struct device *dev, struct device_attribute *dattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int idx = attr->index;
	int reg;
	int cpld_id = 0;
	int ret;
	int rpm;

	if (idx < 0 || idx >= NUM_FAN_SPEED_REGS)
		return sprintf(buf, "Invalid fan speed register index: %d\n",
			       idx);

	reg = fan_speed_reg[idx];
	ret = i2c_smbus_read_byte_data(cpld_devices[cpld_id], reg);
	if (ret < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, reg);
		return -EINVAL;
	}

	rpm = ret * 1250 / 11;
	return sprintf(buf, "%d\n", rpm);
}

static SENSOR_DEVICE_ATTR_RO(fan1_input, fan_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan2_input, fan_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan3_input, fan_show, 2);
static SENSOR_DEVICE_ATTR_RO(fan4_input, fan_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan5_input, fan_show, 4);
static SENSOR_DEVICE_ATTR_RO(fan6_input, fan_show, 5);

/* special case for pwm to convert to range 0 to 255 */

#define FAN_PWM_REG 0x2b

static ssize_t pwm_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	int ret;
	int cpld_id = 0;
	int reg = FAN_PWM_REG;

	ret = i2c_smbus_read_byte_data(cpld_devices[cpld_id], reg);
	if (ret < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, reg);
		return -EINVAL;
	}

	/* The PWM register contains a value between 0x00 and 0x10
	 * inclusive, representing the fan duty cycle in 6.25%
	 * increments.	A value of 0x10 is 100% duty cycle.
	 *
	 * For hwmon devices map the pwm value into the range 0 to
	 * 255.
	 *
	 * hwmon = pwm * 255 / 16
	 */

	ret = (ret * 255) / 16;
	return sprintf(buf, "%d\n", ret);
}

static ssize_t pwm_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	int ret;
	int cpld_id = 0;
	uint32_t pwm = 0;
	int reg = FAN_PWM_REG;

	ret = kstrtou32(buf, 0, &pwm);
	if (ret != 0)
		return ret;

	pwm = clamp_val(pwm, 0, 255);

	/* See comments above in pwm_show for the mapping */
	pwm = (pwm * 16) / 255;

	ret = i2c_smbus_write_byte_data(cpld_devices[cpld_id], reg, pwm);
	if (ret) {
		pr_err("CPLD write error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, reg);
	}

	ret = i2c_smbus_read_byte_data(cpld_devices[cpld_id], reg);

	return count;
}

static SENSOR_DEVICE_ATTR_RW(fan_pwm, pwm_show, pwm_store, 0);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_external_cpu.attr,
	&cpld_poe_support.attr,
	&cpld_product_id.attr,

	&cpld_sfp0_tx_disable.attr,
	&cpld_sfp0_present.attr,
	&cpld_sfp0_tx_fault.attr,
	&cpld_sfp0_rx_los.attr,

	&cpld_sfp1_tx_disable.attr,
	&cpld_sfp1_present.attr,
	&cpld_sfp1_tx_fault.attr,
	&cpld_sfp1_rx_los.attr,

	&cpld_sfp2_tx_disable.attr,
	&cpld_sfp2_present.attr,
	&cpld_sfp2_tx_fault.attr,
	&cpld_sfp2_rx_los.attr,

	&cpld_sfp3_tx_disable.attr,
	&cpld_sfp3_present.attr,
	&cpld_sfp3_tx_fault.attr,
	&cpld_sfp3_rx_los.attr,

	&cpld_pca9548_reset.attr,

	&cpld_is_production_cpld.attr,
	&cpld_cpld_rev.attr,

	&cpld_psu_pwr2_all_ok.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr2_alert.attr,
	&cpld_psu_pwr2_power_on.attr,
	&cpld_psu_pwr1_alert.attr,
	&cpld_psu_pwr1_power_on.attr,

	&cpld_i210_reset.attr,
	&cpld_10g_phy_reset.attr,
	&cpld_1g_phy6_reset.attr,
	&cpld_1g_phy5_reset.attr,
	&cpld_1g_phy4_reset.attr,
	&cpld_1g_phy3_reset.attr,
	&cpld_1g_phy2_reset.attr,
	&cpld_1g_phy1_reset.attr,
	&cpld_platform_reset.attr,
	&cpld_global_reset.attr,

	&cpld_mac_reset.attr,
	&cpld_wd_reset.attr,
	&cpld_usb_reset.attr,
	&cpld_pcie1_reset.attr,
	&cpld_pcie0_reset.attr,
	&cpld_sw_reset.attr,
	&cpld_system_reset.attr,

	&cpld_reset_button.attr,
	&cpld_button_en.attr,
	&cpld_push_button.attr,

	&cpld_led_system.attr,
	&cpld_led_psu.attr,
	&cpld_led_fan.attr,
	&cpld_led_stacking.attr,

	&cpld_onie_eeprom_wp.attr,
	&cpld_nvram_eeprom_wp.attr,
	&cpld_vpd_eeprom_wp.attr,

	&cpld_fan1_present.attr,
	&cpld_fan2_present.attr,
	&cpld_fan3_present.attr,

	&cpld_fan1_direction.attr,
	&cpld_fan2_direction.attr,
	&cpld_fan3_direction.attr,

	&cpld_led_fan1.attr,
	&cpld_led_fan2.attr,
	&cpld_led_fan3.attr,

	&cpld_pcb_rev_id.attr,
	&cpld_pcb_rev.attr,

	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan_pwm.dev_attr.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

/* CPLD initialization */

static int cpld_probe(struct platform_device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->dev.kobj, &cpld_attr_group);
	if (ret)
		pr_err("Failed to create sysfs group for cpld driver\n");

	return ret;
}

static int cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver cpld_driver = {
	.driver = {
		.name = "lenovo_ne0152t_cpld",
		.owner = THIS_MODULE,
	},
	.probe = cpld_probe,
	.remove = cpld_remove,
};

static struct platform_device *cpld_device;

static int cpld_init(void)
{
	int ret;

	if (num_cpld_devices != NUM_CPLD_DEVICES) {
		pr_err("Error: number of CPLD devices: %d; expected: %d\n",
		       num_cpld_devices, NUM_CPLD_DEVICES);
		return -ENODEV;
	}

	ret = platform_driver_register(&cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for CPLD device\n");
		return ret;
	}

	cpld_device = platform_device_alloc("lenovo_ne0152t_cpld", 0);
	if (!cpld_device) {
		pr_err("platform_device_alloc() failed for CPLD device\n");
		platform_driver_unregister(&cpld_driver);
		return -ENOMEM;
	}

	ret = platform_device_add(cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for CPLD device\n");
		platform_device_put(cpld_device);
		return ret;
	}

	pr_info("CPLD driver loaded\n");
	return 0;
}

static void cpld_exit(void)
{
	platform_driver_unregister(&cpld_driver);
	platform_device_unregister(cpld_device);
	pr_err("CPLD driver unloaded\n");
}

/*
 * Module init and exit
 */

static int __init lenovo_ne0152t_init(void)
{
	int ret;

	ret = i2c_init();
	if (ret) {
		pr_err("I2C subsystem initialization failed\n");
		return ret;
	}

	ret = cpld_init();
	if (ret) {
		pr_err("CPLD initialization failed\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit lenovo_ne0152t_exit(void)
{
	cpld_exit();
	i2c_exit();
	pr_info(DRIVER_NAME " driver unloaded\n");
}

module_init(lenovo_ne0152t_init);
module_exit(lenovo_ne0152t_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Lenovo NE0152T platform driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
