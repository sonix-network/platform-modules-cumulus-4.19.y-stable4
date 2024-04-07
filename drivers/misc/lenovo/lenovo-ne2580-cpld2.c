// SPDX-License-Identifier: GPL-2.0+
/*
 * Lenovo NE2580 CPLD2 Driver
 *
 * Copyright (c) 2019 Cumulus Networks, Inc.  All rights reserved.
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
#include "lenovo-ne2580.h"

#define DRIVER_NAME	   NE2580_CPLD2_NAME
#define DRIVER_VERSION	   "1.1"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* cpld register bitfields with enum-like values */

static const char * const system_direction_values[] = {
	"B2F", /* 0 */
	"F2B", /* 1 */
};

static const char * const green_led_values[] = {
	PLATFORM_LED_OFF,                 /* 0 */
	PLATFORM_LED_GREEN,               /* 1 */
	PLATFORM_LED_GREEN_SLOW_BLINKING, /* 2 */
	PLATFORM_LED_GREEN_BLINKING,      /* 3 */
};

static const char * const psu_values[] = {
	"550W", /* 0 */
	"800W", /* 1 */
};

static const char * const fan_direction_values[] = {
	"F2B", /* 0 */
	"B2F", /* 1 */
};

static const char * const led_colors[] = {
	PLATFORM_LED_OFF,                 /* 0 */
	PLATFORM_LED_GREEN,               /* 1 */
	PLATFORM_LED_GREEN_SLOW_BLINKING, /* 2 */
	PLATFORM_LED_GREEN_BLINKING,      /* 3 */
};

static const char * const led_fantray_colors[] = {
	PLATFORM_LED_OFF,   /* 0 */
	PLATFORM_LED_GREEN, /* 1 */
	PLATFORM_LED_RED,   /* 2 */
	"green_and_red",    /* 3 */
};

/* CPLD registers */

cpld_bt_ro(cpld2_system_airflow, LENOVO_NE2580_CPLD2_SYSTEM_BOARD_STATUS_REG,
	   LENOVO_NE2580_CPLD2_SYS_AIR_DIR, system_direction_values, 0);
cpld_bt_ro(cpld2_triton_model, LENOVO_NE2580_CPLD2_SYSTEM_BOARD_STATUS_REG,
	   LENOVO_NE2580_CPLD2_TRITON_MODEL_SEL, NULL, 0);

cpld_bt_rw(cpld2_system_ready, LENOVO_NE2580_CPLD2_SWITCH_READY_REG,
	   LENOVO_NE2580_CPLD2_SYS_RDY, NULL, 0);

cpld_bf_rw(led_power, LENOVO_NE2580_CPLD2_SYSTEM_STATUS_LED_CONTROL_REG,
	   LENOVO_NE2580_CPLD2_PWR_LED, led_colors, 0);
cpld_bf_rw(led_fan, LENOVO_NE2580_CPLD2_SYSTEM_STATUS_LED_CONTROL_REG,
	   LENOVO_NE2580_CPLD2_FAN_LED, led_colors, 0);

cpld_bt_rw(int_sfp_io3_b, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO3_B_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(int_sfp_io3_a, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO3_A_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(int_sfp_io2_b, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO2_B_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(int_sfp_io2_a, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO2_A_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(int_sfp_io1_b, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO1_B_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(int_sfp_io1_a, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO1_A_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(int_sfp_io0_b, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO0_B_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(int_sfp_io0_a, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO0_A_INT_N, NULL, BF_COMPLEMENT);

cpld_bt_rw(int_sfp_io0_c, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO0_C_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(int_sfp_io5_b, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO5_B_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(int_sfp_io5_a, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO5_A_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(int_sfp_io4_b, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO4_B_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(int_sfp_io4_a, LENOVO_NE2580_CPLD2_INTERRUPT_1_INDICATE_REG,
	   LENOVO_NE2580_CPLD2_SFP_IO4_A_INT_N, NULL, BF_COMPLEMENT);

cpld_bt_rw(mux_int_mode_en, LENOVO_NE2580_CPLD2_INTERRUPT_CONTROL_REG,
	   LENOVO_NE2580_CPLD2_MUX_INT_MODE_EN, NULL, 0);

cpld_bt_ro(psu_pwr2_type, LENOVO_NE2580_CPLD2_PSU_STATUS_REG,
	   LENOVO_NE2580_CPLD2_PSU2_TYPE, psu_values, 0);
cpld_bt_ro(psu_pwr2_present, LENOVO_NE2580_CPLD2_PSU_STATUS_REG,
	   LENOVO_NE2580_CPLD2_PSU2_PRESNT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_all_ok, LENOVO_NE2580_CPLD2_PSU_STATUS_REG,
	   LENOVO_NE2580_CPLD2_PSU2_PWROK, NULL, 0);
cpld_bt_ro(psu_pwr1_type, LENOVO_NE2580_CPLD2_PSU_STATUS_REG,
	   LENOVO_NE2580_CPLD2_PSU1_TYPE, psu_values, 0);
cpld_bt_ro(psu_pwr1_present, LENOVO_NE2580_CPLD2_PSU_STATUS_REG,
	   LENOVO_NE2580_CPLD2_PSU1_PRESNT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_all_ok, LENOVO_NE2580_CPLD2_PSU_STATUS_REG,
	   LENOVO_NE2580_CPLD2_PSU1_PWROK, NULL, 0);

cpld_bt_ro(pgd_p5v, LENOVO_NE2580_CPLD2_SYSTEM_PWR_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PGD_P5V, NULL, 0);
cpld_bt_ro(pgd_p3v3_stby, LENOVO_NE2580_CPLD2_SYSTEM_PWR_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PGD_P3V3_STBY, NULL, 0);
cpld_bt_ro(pgd_p1v8_a, LENOVO_NE2580_CPLD2_SYSTEM_PWR_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PGD_P1V8_A, NULL, 0);
cpld_bt_ro(pgd_p3v3_sys, LENOVO_NE2580_CPLD2_SYSTEM_PWR_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PGD_P3V3_SYS, NULL, 0);
cpld_bt_ro(pgd_p3v3_a, LENOVO_NE2580_CPLD2_SYSTEM_PWR_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PGD_P3V3_A, NULL, 0);
cpld_bt_ro(pgd_p3v3_b, LENOVO_NE2580_CPLD2_SYSTEM_PWR_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PGD_P3V3_B, NULL, 0);
cpld_bt_ro(pgd_p1v2, LENOVO_NE2580_CPLD2_SYSTEM_PWR_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PGD_P1V2, NULL, 0);

cpld_bt_ro(pgd_p0v8_a, LENOVO_NE2580_CPLD2_SYSTEM_PWR_STATUS_2_REG,
	   LENOVO_NE2580_CPLD2_PGD_P0V8_A, NULL, 0);
cpld_bt_ro(pgd_p0v89_rov, LENOVO_NE2580_CPLD2_SYSTEM_PWR_STATUS_2_REG,
	   LENOVO_NE2580_CPLD2_PGD_P0V89_ROV, NULL, 0);
cpld_bt_ro(sw_pwr_ready, LENOVO_NE2580_CPLD2_SYSTEM_PWR_STATUS_2_REG,
	   LENOVO_NE2580_CPLD2_SW_PWR_READY, NULL, 0);
cpld_bt_ro(pgd_p1v8_a_stby, LENOVO_NE2580_CPLD2_SYSTEM_PWR_STATUS_2_REG,
	   LENOVO_NE2580_CPLD2_PGD_P1V8_A_STBY, NULL, 0);

cpld_bf_rw(pwm1, LENOVO_NE2580_CPLD2_FAN1_PWM_REG,
	   LENOVO_NE2580_CPLD2_PLD_PWM_FAN1, NULL, 0);
cpld_bf_rw(pwm2, LENOVO_NE2580_CPLD2_FAN2_PWM_REG,
	   LENOVO_NE2580_CPLD2_PLD_PWM_FAN2, NULL, 0);
cpld_bf_rw(pwm3, LENOVO_NE2580_CPLD2_FAN3_PWM_REG,
	   LENOVO_NE2580_CPLD2_PLD_PWM_FAN3, NULL, 0);
cpld_bf_rw(pwm4, LENOVO_NE2580_CPLD2_FAN4_PWM_REG,
	   LENOVO_NE2580_CPLD2_PLD_PWM_FAN4, NULL, 0);
cpld_bf_rw(pwm5, LENOVO_NE2580_CPLD2_FAN5_PWM_REG,
	   LENOVO_NE2580_CPLD2_PLD_PWM_FAN5, NULL, 0);

cpld_bt_ro(fan5_direction, LENOVO_NE2580_CPLD2_FAN_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PLD_FAN5_DIR, fan_direction_values, 0);
cpld_bt_ro(fan4_direction, LENOVO_NE2580_CPLD2_FAN_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PLD_FAN4_DIR, fan_direction_values, 0);
cpld_bt_ro(fan3_direction, LENOVO_NE2580_CPLD2_FAN_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PLD_FAN3_DIR, fan_direction_values, 0);
cpld_bt_ro(fan2_direction, LENOVO_NE2580_CPLD2_FAN_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PLD_FAN2_DIR, fan_direction_values, 0);
cpld_bt_ro(fan1_direction, LENOVO_NE2580_CPLD2_FAN_STATUS_1_REG,
	   LENOVO_NE2580_CPLD2_PLD_FAN1_DIR, fan_direction_values, 0);

cpld_bt_ro(fan5_present, LENOVO_NE2580_CPLD2_FAN_STATUS_2_REG,
	   LENOVO_NE2580_CPLD2_PLD_FAN5_PRES_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan4_present, LENOVO_NE2580_CPLD2_FAN_STATUS_2_REG,
	   LENOVO_NE2580_CPLD2_PLD_FAN4_PRES_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan3_present, LENOVO_NE2580_CPLD2_FAN_STATUS_2_REG,
	   LENOVO_NE2580_CPLD2_PLD_FAN3_PRES_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan2_present, LENOVO_NE2580_CPLD2_FAN_STATUS_2_REG,
	   LENOVO_NE2580_CPLD2_PLD_FAN2_PRES_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan1_present, LENOVO_NE2580_CPLD2_FAN_STATUS_2_REG,
	   LENOVO_NE2580_CPLD2_PLD_FAN1_PRES_N, NULL, BF_COMPLEMENT);

cpld_bt_rw(set_max_fan_speed, LENOVO_NE2580_CPLD2_FAN_SPEED_CONTROL_REG,
	   LENOVO_NE2580_CPLD2_FAN_SPEED_MAX, NULL, 0);

/* special case for cpld version register */

static ssize_t cpld2_version_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	s32 temp;
	int reg = LENOVO_NE2580_CPLD2_VERSION_REG;

	temp = i2c_smbus_read_byte_data(client, reg);
	if (temp < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, reg);
		return -EINVAL;
	}

	return sprintf(buf, "%d.%d\n",
		       (int)GET_FIELD(temp, LENOVO_NE2580_CPLD2_MAJOR_VERSION),
		       (int)GET_FIELD(temp, LENOVO_NE2580_CPLD2_MINOR_VERSION));
}

static SENSOR_DEVICE_ATTR_RO(cpld2_version, cpld2_version_show, 0);

/* special case for cpld date registers */

static ssize_t cpld2_date_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	s32 temp1;
	int reg1 = LENOVO_NE2580_CPLD2_RELEASE_DATE_1_REG;
	s32 temp2;
	int reg2 = LENOVO_NE2580_CPLD2_RELEASE_DATE_2_REG;

	temp1 = i2c_smbus_read_byte_data(client, reg1);
	if (temp1 < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, reg1);
		return -EINVAL;
	}
	temp2 = i2c_smbus_read_byte_data(client, reg2);
	if (temp2 < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, reg2);
		return -EINVAL;
	}

	return sprintf(buf, "%d/%d/%d\n",
		       (int)GET_FIELD(temp1, LENOVO_NE2580_CPLD2_RELEASE_MONTH),
		       (int)GET_FIELD(temp2, LENOVO_NE2580_CPLD2_RELEASE_DATE),
		       (int)GET_FIELD(temp1, LENOVO_NE2580_CPLD2_RELEASE_YEAR) +
		       2014);
}

static SENSOR_DEVICE_ATTR_RO(cpld2_date, cpld2_date_show, 0);

/* special case for fan speed registers */

static ssize_t fan_show(struct device *dev, struct device_attribute *dattr,
			char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int reg = attr->index;
	s32 temp1;
	s32 temp2;

	temp1 = i2c_smbus_read_byte_data(client, reg);
	if (temp1 < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, reg);
		return -EINVAL;
	}
	temp2 = i2c_smbus_read_byte_data(client, reg + 1);
	if (temp2 < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, reg + 1);
		return -EINVAL;
	}

	return sprintf(buf, "%d\n", (temp1 * 256) + temp2);
}

static SENSOR_DEVICE_ATTR_RO(fan1_input,  fan_show,
			     LENOVO_NE2580_CPLD2_FAN1_INLET_TACH_1_REG);
static SENSOR_DEVICE_ATTR_RO(fan2_input,  fan_show,
			     LENOVO_NE2580_CPLD2_FAN2_INLET_TACH_1_REG);
static SENSOR_DEVICE_ATTR_RO(fan3_input,  fan_show,
			     LENOVO_NE2580_CPLD2_FAN3_INLET_TACH_1_REG);
static SENSOR_DEVICE_ATTR_RO(fan4_input,  fan_show,
			     LENOVO_NE2580_CPLD2_FAN4_INLET_TACH_1_REG);
static SENSOR_DEVICE_ATTR_RO(fan5_input,  fan_show,
			     LENOVO_NE2580_CPLD2_FAN5_INLET_TACH_1_REG);
static SENSOR_DEVICE_ATTR_RO(fan6_input,  fan_show,
			     LENOVO_NE2580_CPLD2_FAN1_OUTLET_TACH_1_REG);
static SENSOR_DEVICE_ATTR_RO(fan7_input,  fan_show,
			     LENOVO_NE2580_CPLD2_FAN2_OUTLET_TACH_1_REG);
static SENSOR_DEVICE_ATTR_RO(fan8_input,  fan_show,
			     LENOVO_NE2580_CPLD2_FAN3_OUTLET_TACH_1_REG);
static SENSOR_DEVICE_ATTR_RO(fan9_input,  fan_show,
			     LENOVO_NE2580_CPLD2_FAN4_OUTLET_TACH_1_REG);
static SENSOR_DEVICE_ATTR_RO(fan10_input, fan_show,
			     LENOVO_NE2580_CPLD2_FAN5_OUTLET_TACH_1_REG);

/* special case for fan led registers */

static ssize_t led_fantray_show(struct device *dev,
				struct device_attribute *dattr,
				char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int bit = attr->index;
	int green_reg = LENOVO_NE2580_CPLD2_FAN_LED_CONTROL_1_REG;
	int red_reg = LENOVO_NE2580_CPLD2_FAN_LED_CONTROL_2_REG;
	s32 green;
	s32 red;
	int green_bit;
	int red_bit;
	int index;

	green = i2c_smbus_read_byte_data(client, green_reg);
	if (green < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, green_reg);
		return -EINVAL;
	}
	green_bit = (green >> bit) & 1;

	red = i2c_smbus_read_byte_data(client, red_reg);
	if (red < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, red_reg);
		return -EINVAL;
	}
	red_bit = (red >> bit) & 1;

	/* green is bit 0 and red is bit 1 */
	index = (red_bit << 1) | green_bit;

	return sprintf(buf, "%s\n", led_fantray_colors[index]);
}

static ssize_t led_fantray_store(struct device *dev,
				 struct device_attribute *dattr,
				 const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int bit = attr->index;
	char raw[20];
	int i;
	int new_green;
	int new_red;
	int green_reg = LENOVO_NE2580_CPLD2_FAN_LED_CONTROL_1_REG;
	int red_reg = LENOVO_NE2580_CPLD2_FAN_LED_CONTROL_2_REG;
	s32 green;
	s32 red;
	int ret;

	if (sscanf(buf, "%19s", raw) <= 0)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(led_fantray_colors); i++) {
		if (strcmp(raw, led_fantray_colors[i]) == 0)
			break;
	}
	if (i == ARRAY_SIZE(led_fantray_colors))
		return -EINVAL;

	/* green is bit 0 and red is bit 1 */
	new_green = (i & 0x1);
	new_red = ((i & 0x2) >> 1);

	green = i2c_smbus_read_byte_data(client, green_reg);
	if (green < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, green_reg);
		return -EINVAL;
	}
	green &= ~(1 << bit);
	green |= (new_green << bit);
	ret = i2c_smbus_write_byte_data(client, green_reg, green);

	red = i2c_smbus_read_byte_data(client, red_reg);
	if (red < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, red_reg);
		return -EINVAL;
	}
	red &= ~(1 << bit);
	red |= (new_red << bit);
	ret = i2c_smbus_write_byte_data(client, red_reg, red);

	return count;
}

static SENSOR_DEVICE_ATTR_RW(led_fan1, led_fantray_show, led_fantray_store, 0);
static SENSOR_DEVICE_ATTR_RW(led_fan2, led_fantray_show, led_fantray_store, 1);
static SENSOR_DEVICE_ATTR_RW(led_fan3, led_fantray_show, led_fantray_store, 2);
static SENSOR_DEVICE_ATTR_RW(led_fan4, led_fantray_show, led_fantray_store, 3);
static SENSOR_DEVICE_ATTR_RW(led_fan5, led_fantray_show, led_fantray_store, 4);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_cpld2_system_airflow.attr,
	&cpld_cpld2_triton_model.attr,
	&cpld_cpld2_system_ready.attr,
	&cpld_led_power.attr,
	&cpld_led_fan.attr,
	&cpld_int_sfp_io5_b.attr,
	&cpld_int_sfp_io5_a.attr,
	&cpld_int_sfp_io4_b.attr,
	&cpld_int_sfp_io4_a.attr,
	&cpld_int_sfp_io3_b.attr,
	&cpld_int_sfp_io3_a.attr,
	&cpld_int_sfp_io2_b.attr,
	&cpld_int_sfp_io2_a.attr,
	&cpld_int_sfp_io1_b.attr,
	&cpld_int_sfp_io1_a.attr,
	&cpld_int_sfp_io0_c.attr,
	&cpld_int_sfp_io0_b.attr,
	&cpld_int_sfp_io0_a.attr,
	&cpld_mux_int_mode_en.attr,
	&cpld_psu_pwr2_type.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr2_all_ok.attr,
	&cpld_psu_pwr1_type.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_pgd_p5v.attr,
	&cpld_pgd_p3v3_stby.attr,
	&cpld_pgd_p1v8_a.attr,
	&cpld_pgd_p3v3_sys.attr,
	&cpld_pgd_p3v3_a.attr,
	&cpld_pgd_p3v3_b.attr,
	&cpld_pgd_p1v2.attr,
	&cpld_pgd_p0v8_a.attr,
	&cpld_pgd_p0v89_rov.attr,
	&cpld_sw_pwr_ready.attr,
	&cpld_pgd_p1v8_a_stby.attr,
	&cpld_pwm1.attr,
	&cpld_pwm2.attr,
	&cpld_pwm3.attr,
	&cpld_pwm4.attr,
	&cpld_pwm5.attr,
	&cpld_fan5_direction.attr,
	&cpld_fan4_direction.attr,
	&cpld_fan3_direction.attr,
	&cpld_fan2_direction.attr,
	&cpld_fan1_direction.attr,
	&cpld_fan5_present.attr,
	&cpld_fan4_present.attr,
	&cpld_fan3_present.attr,
	&cpld_fan2_present.attr,
	&cpld_fan1_present.attr,
	&cpld_set_max_fan_speed.attr,

	&sensor_dev_attr_cpld2_version.dev_attr.attr,
	&sensor_dev_attr_cpld2_date.dev_attr.attr,

	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,
	&sensor_dev_attr_fan9_input.dev_attr.attr,
	&sensor_dev_attr_fan10_input.dev_attr.attr,

	&sensor_dev_attr_led_fan1.dev_attr.attr,
	&sensor_dev_attr_led_fan2.dev_attr.attr,
	&sensor_dev_attr_led_fan3.dev_attr.attr,
	&sensor_dev_attr_led_fan4.dev_attr.attr,
	&sensor_dev_attr_led_fan5.dev_attr.attr,

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

	/* probe the hardware by reading the version register */
	temp = i2c_smbus_read_byte_data(client,
					LENOVO_NE2580_CPLD2_VERSION_REG);
	if (temp < 0) {
		pr_err(DRIVER_NAME ": read CPLD2 version register error: %d\n",
		       temp);
		ret = temp;
		goto err;
	}

	/* create sysfs node */
	ret = sysfs_create_group(&dev->kobj, &cpld_attr_group);
	if (ret) {
		pr_err(DRIVER_NAME ": failed to create sysfs group for cpld device\n");
		goto err;
	}

	/* all clear */
	pr_info(DRIVER_NAME ": device probed, CPLD2 rev: %d.%d\n",
		(int)GET_FIELD(temp, LENOVO_NE2580_CPLD2_MAJOR_VERSION),
		(int)GET_FIELD(temp, LENOVO_NE2580_CPLD2_MINOR_VERSION));

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

/* module init and exit */

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
MODULE_DESCRIPTION("Lenovo NE2580 CPLD2 Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
