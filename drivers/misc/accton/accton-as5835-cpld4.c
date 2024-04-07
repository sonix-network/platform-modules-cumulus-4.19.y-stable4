// SPDX-License-Identifier: GPL-2.0+
/*
 * Accton AS5835 CPLD4 Driver
 *
 * Copyright (c) 2019, 2020 Cumulus Networks, Inc.  All rights reserved.
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
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "accton-as5835.h"

#define DRIVER_NAME    AS5835_CPLD4_NAME
#define DRIVER_VERSION "1.0"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* cpld register bitfields with enum-like values */

static const char * const fan_direction_values[] = {
	"F2B",
	"B2F",
};

static const char * const led_fan_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_YELLOW,
};

/* CPLD registers */

cpld_bf_ro(cpld_version, ACCTON_AS5835_CPLD4_VERSION_REG,
	   ACCTON_AS5835_CPLD4_VERSION, NULL, 0);

cpld_bt_ro(fan5_present, ACCTON_AS5835_CPLD4_FAN_PRESENT_REG,
	   ACCTON_AS5835_CPLD4_FAN5_PRESENT, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan4_present, ACCTON_AS5835_CPLD4_FAN_PRESENT_REG,
	   ACCTON_AS5835_CPLD4_FAN4_PRESENT, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan3_present, ACCTON_AS5835_CPLD4_FAN_PRESENT_REG,
	   ACCTON_AS5835_CPLD4_FAN3_PRESENT, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan2_present, ACCTON_AS5835_CPLD4_FAN_PRESENT_REG,
	   ACCTON_AS5835_CPLD4_FAN2_PRESENT, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan1_present, ACCTON_AS5835_CPLD4_FAN_PRESENT_REG,
	   ACCTON_AS5835_CPLD4_FAN1_PRESENT, NULL, BF_COMPLEMENT);

cpld_bt_ro(fan5_direction, ACCTON_AS5835_CPLD4_FAN_DIRECTION_REG,
	   ACCTON_AS5835_CPLD4_FAN5_DIRECTION, fan_direction_values, 0);
cpld_bt_ro(fan4_direction, ACCTON_AS5835_CPLD4_FAN_DIRECTION_REG,
	   ACCTON_AS5835_CPLD4_FAN4_DIRECTION, fan_direction_values, 0);
cpld_bt_ro(fan3_direction, ACCTON_AS5835_CPLD4_FAN_DIRECTION_REG,
	   ACCTON_AS5835_CPLD4_FAN3_DIRECTION, fan_direction_values, 0);
cpld_bt_ro(fan2_direction, ACCTON_AS5835_CPLD4_FAN_DIRECTION_REG,
	   ACCTON_AS5835_CPLD4_FAN2_DIRECTION, fan_direction_values, 0);
cpld_bt_ro(fan1_direction, ACCTON_AS5835_CPLD4_FAN_DIRECTION_REG,
	   ACCTON_AS5835_CPLD4_FAN1_DIRECTION, fan_direction_values, 0);

cpld_bt_ro(fan5_fault, ACCTON_AS5835_CPLD4_FAN_FAULT_STATUS_REG,
	   ACCTON_AS5835_CPLD4_FAN5_FAULT, NULL, 0);
cpld_bt_ro(fan4_fault, ACCTON_AS5835_CPLD4_FAN_FAULT_STATUS_REG,
	   ACCTON_AS5835_CPLD4_FAN4_FAULT, NULL, 0);
cpld_bt_ro(fan3_fault, ACCTON_AS5835_CPLD4_FAN_FAULT_STATUS_REG,
	   ACCTON_AS5835_CPLD4_FAN3_FAULT, NULL, 0);
cpld_bt_ro(fan2_fault, ACCTON_AS5835_CPLD4_FAN_FAULT_STATUS_REG,
	   ACCTON_AS5835_CPLD4_FAN2_FAULT, NULL, 0);
cpld_bt_ro(fan1_fault, ACCTON_AS5835_CPLD4_FAN_FAULT_STATUS_REG,
	   ACCTON_AS5835_CPLD4_FAN1_FAULT, NULL, 0);

cpld_bt_ro(fanr5_fault, ACCTON_AS5835_CPLD4_FANR_FAULT_STATUS_REG,
	   ACCTON_AS5835_CPLD4_FANR5_FAULT, NULL, 0);
cpld_bt_ro(fanr4_fault, ACCTON_AS5835_CPLD4_FANR_FAULT_STATUS_REG,
	   ACCTON_AS5835_CPLD4_FANR4_FAULT, NULL, 0);
cpld_bt_ro(fanr3_fault, ACCTON_AS5835_CPLD4_FANR_FAULT_STATUS_REG,
	   ACCTON_AS5835_CPLD4_FANR3_FAULT, NULL, 0);
cpld_bt_ro(fanr2_fault, ACCTON_AS5835_CPLD4_FANR_FAULT_STATUS_REG,
	   ACCTON_AS5835_CPLD4_FANR2_FAULT, NULL, 0);
cpld_bt_ro(fanr1_fault, ACCTON_AS5835_CPLD4_FANR_FAULT_STATUS_REG,
	   ACCTON_AS5835_CPLD4_FANR1_FAULT, NULL, 0);

cpld_bf_rw(led_fan1, ACCTON_AS5835_CPLD4_FAN_4_1_LED_REG,
	   ACCTON_AS5835_CPLD4_FAN1_LED, led_fan_values, 0);
cpld_bf_rw(led_fan2, ACCTON_AS5835_CPLD4_FAN_4_1_LED_REG,
	   ACCTON_AS5835_CPLD4_FAN2_LED, led_fan_values, 0);
cpld_bf_rw(led_fan3, ACCTON_AS5835_CPLD4_FAN_4_1_LED_REG,
	   ACCTON_AS5835_CPLD4_FAN3_LED, led_fan_values, 0);
cpld_bf_rw(led_fan4, ACCTON_AS5835_CPLD4_FAN_4_1_LED_REG,
	   ACCTON_AS5835_CPLD4_FAN4_LED, led_fan_values, 0);
cpld_bf_rw(led_fan5, ACCTON_AS5835_CPLD4_FAN_5_LED_REG,
	   ACCTON_AS5835_CPLD4_FAN5_LED, led_fan_values, 0);

/* special case for fan speed registers */

static ssize_t fan_show(struct device *dev, struct device_attribute *dattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = to_i2c_client(dev);
	int idx = attr->index;
	int reg = ACCTON_AS5835_CPLD4_FAN1_SPEED_REG + idx;
	s32 tmp;

	tmp = i2c_smbus_read_byte_data(client, reg);
	if (tmp < 0) {
		pr_err(DRIVER_NAME ": CPLD4 read error - reg: 0x%02X\n",
		       reg);
		return -EINVAL;
	}
	return sprintf(buf, "%d\n", (tmp & 0xff) * 150);
}

static SENSOR_DEVICE_ATTR_RO(fan1_input,  fan_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan2_input,  fan_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan3_input,  fan_show, 2);
static SENSOR_DEVICE_ATTR_RO(fan4_input,  fan_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan5_input,  fan_show, 4);
static SENSOR_DEVICE_ATTR_RO(fanr1_input, fan_show, 5);
static SENSOR_DEVICE_ATTR_RO(fanr2_input, fan_show, 6);
static SENSOR_DEVICE_ATTR_RO(fanr3_input, fan_show, 7);
static SENSOR_DEVICE_ATTR_RO(fanr4_input, fan_show, 8);
static SENSOR_DEVICE_ATTR_RO(fanr5_input, fan_show, 9);

/* special case for pwm register */

static ssize_t pwm_show(struct device *dev, struct device_attribute *dattr,
			char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int reg = ACCTON_AS5835_CPLD4_FAN_PWM_CYCLE_STATUS_REG;
	s32 tmp;

	tmp = i2c_smbus_read_byte_data(client, reg);
	if (tmp < 0) {
		pr_err(DRIVER_NAME ": CPLD4 read error - reg: 0x%02X\n",
		       reg);
		return -EINVAL;
	}

	/* The PWM register contains a value between 0 and 20,
	 * representing the fan duty cycle in 5% increments.  A
	 * value of 20 is 100% duty cycle.
	 *
	 * For hwmon devices map the pwm value onto the range 0 to
	 * 255.
	 *
	 * hwmon = pwm * 255 / 20
	 */

	return sprintf(buf, "%d\n", ((tmp & 0x1f) * 255) / 20);
}

static ssize_t pwm_store(struct device *dev, struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	int ret;
	uint32_t pwm;
	int reg = ACCTON_AS5835_CPLD4_FAN_PWM_CYCLE_STATUS_REG;

	ret = kstrtou32(buf, 0, &pwm);
	if (ret != 0)
		return ret;

	pwm = clamp_val(pwm, 0, 255);

	/* See comments above in pwm_show for the mapping */
	pwm = (pwm * 20) / 255;

	ret = i2c_smbus_write_byte_data(client, reg, pwm);
	if (ret) {
		pr_err(DRIVER_NAME ": CPLD4 write error - reg: 0x%02X\n",
		       reg);
	}

	return count;
}

static SENSOR_DEVICE_ATTR_RW(fan_pwm, pwm_show, pwm_store, 0);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_cpld_version.attr,
	&cpld_fan5_present.attr,
	&cpld_fan4_present.attr,
	&cpld_fan3_present.attr,
	&cpld_fan2_present.attr,
	&cpld_fan1_present.attr,
	&cpld_fan5_direction.attr,
	&cpld_fan4_direction.attr,
	&cpld_fan3_direction.attr,
	&cpld_fan2_direction.attr,
	&cpld_fan1_direction.attr,
	&cpld_fan5_fault.attr,
	&cpld_fan4_fault.attr,
	&cpld_fan3_fault.attr,
	&cpld_fan2_fault.attr,
	&cpld_fan1_fault.attr,
	&cpld_fanr5_fault.attr,
	&cpld_fanr4_fault.attr,
	&cpld_fanr3_fault.attr,
	&cpld_fanr2_fault.attr,
	&cpld_fanr1_fault.attr,
	&sensor_dev_attr_fan_pwm.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fanr5_input.dev_attr.attr,
	&sensor_dev_attr_fanr4_input.dev_attr.attr,
	&sensor_dev_attr_fanr3_input.dev_attr.attr,
	&sensor_dev_attr_fanr2_input.dev_attr.attr,
	&sensor_dev_attr_fanr1_input.dev_attr.attr,
	&cpld_led_fan1.attr,
	&cpld_led_fan2.attr,
	&cpld_led_fan3.attr,
	&cpld_led_fan4.attr,
	&cpld_led_fan5.attr,

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
					ACCTON_AS5835_CPLD4_VERSION_REG);
	if (temp < 0) {
		dev_err(dev, "read CPLD4 version register error: %d\n",
			temp);
		ret = temp;
		goto err;
	}

	/* create sysfs node */
	ret = sysfs_create_group(&dev->kobj, &cpld_attr_group);
	if (ret) {
		dev_err(dev, "failed to create sysfs group for cpld device\n");
		goto err;
	}

	/* all clear */
	dev_info(dev, "device probed, CPLD4 rev: %lu\n",
		 GET_FIELD(temp, ACCTON_AS5835_CPLD4_VERSION));

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
MODULE_DESCRIPTION("Accton AS5835 CPLD4 Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
