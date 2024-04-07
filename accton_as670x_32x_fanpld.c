/*
 * sysfs driver for Accton AS670x_32x Fan CPLD (FANPLD)
 *
 * Copyright (C) 2014 Cumulus Networks, Inc.
 * Author: Dustin Byford <dustin@cumulusnetworks.com>
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
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/of_platform.h>
#include <asm/io.h>

#include "accton_as670x_32x_fanpld.h"
#include "platform_defs.h"

static const char driver_name[] = "accton_as670x_32x_fanpld";
#define DRIVER_VERSION "1.0"

static const struct i2c_device_id fanpld_id[] = {
        { "as670x_32x_fanpld", 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, fanpld_id);

struct fanpld_data
{
	struct device *hwmon_dev;
};

static const uint8_t pwm_values[] = { 0, 0, 0, 95, 127, 159, 191, 255 };

static ssize_t pwm_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	uint8_t pwm;
	uint8_t reg;

	reg = i2c_smbus_read_byte_data(client, CPLD_FAN_CONTROL);
	pwm = pwm_values[reg & CPLD_FAN_CONTROL_MASK];

	return sprintf(buf, "%u\n", pwm);
}

static ssize_t pwm_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	uint8_t reg = 0;
	int pwm;
	int val;
	int rv;

	pwm = simple_strtoul(buf, NULL, 10);
	pwm = clamp_val(pwm, 0, 255);

	/* Find the closest setting rounding up */
	reg = CPLD_FAN_CONTROL_100;
	for (val = ARRAY_SIZE(pwm_values) - 1; val != 0; val--) {
		if (pwm_values[val] < pwm) {
			break;
		}
		reg = val;
	}

	if (reg < 3) {
		/* values 1 and 2 aren't really valid */
		reg = 0;
	}

	reg |= i2c_smbus_read_byte_data(client, CPLD_FAN_CONTROL) & ~CPLD_FAN_CONTROL_MASK;
	rv = i2c_smbus_write_byte_data(client, CPLD_FAN_CONTROL, reg);
	if (rv < 0) {
		return rv;
	}

	return count;
}
static SYSFS_ATTR_RW(pwm1, pwm_show, pwm_store);

static ssize_t pwm_en_show(struct device *dev,
			   struct device_attribute *dattr,
			   char *buf)
{
	return sprintf(buf, "1\n");
}

static ssize_t pwm_en_store(struct device *dev,
			    struct device_attribute *dattr,
			    const char *buf, size_t count)
{
	if (count < 1) {
		return -EINVAL;
	}

	if (strcmp(buf, "1\n") != 0 && strcmp(buf, "1") != 0) {
		return -EINVAL;
	}

	return count;
}
static SYSFS_ATTR_RW(pwm1_enable, pwm_en_show, pwm_en_store);

static const int fan_rpm[] = { CPLD_FAN_SENSOR_0,
			       CPLD_FAN_SENSOR_0A,
			       CPLD_FAN_SENSOR_1,
			       CPLD_FAN_SENSOR_1A,
			       CPLD_FAN_SENSOR_2,
			       CPLD_FAN_SENSOR_2A,
			       CPLD_FAN_SENSOR_3,
			       CPLD_FAN_SENSOR_3A,
			       CPLD_FAN_SENSOR_4,
			       CPLD_FAN_SENSOR_4A,
			     };

static const int fan_dir[] = { CPLD_FAN_DIRECTION_0,
			       CPLD_FAN_DIRECTION_0,
			       CPLD_FAN_DIRECTION_1,
			       CPLD_FAN_DIRECTION_1,
			       CPLD_FAN_DIRECTION_2,
			       CPLD_FAN_DIRECTION_2,
			       CPLD_FAN_DIRECTION_3,
			       CPLD_FAN_DIRECTION_3,
			       CPLD_FAN_DIRECTION_4,
			       CPLD_FAN_DIRECTION_4,
			     };

static const int fan_ok[] = { CPLD_FAN_NORMAL_L_0,
			      CPLD_FAN_NORMAL_L_0,
			      CPLD_FAN_NORMAL_L_1,
			      CPLD_FAN_NORMAL_L_1,
			      CPLD_FAN_NORMAL_L_2,
			      CPLD_FAN_NORMAL_L_2,
			      CPLD_FAN_NORMAL_L_3,
			      CPLD_FAN_NORMAL_L_3,
			      CPLD_FAN_NORMAL_L_4,
			      CPLD_FAN_NORMAL_L_4,
			     };

static ssize_t airflow_show(struct device *dev,
			    struct device_attribute *dattr,
			    char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int fan;
	int dir_mask;
	uint8_t reg;
	bool ftb;

	BUG_ON(strlen(dattr->attr.name) < 4);
	fan = simple_strtoul(&dattr->attr.name[3], NULL, 10) - 1;

	BUG_ON(fan > ARRAY_SIZE(fan_dir));
	dir_mask = fan_dir[fan];

	reg = i2c_smbus_read_byte_data(client, CPLD_FAN_DIRECTION);
	ftb = (reg & dir_mask) == CPLD_FAN_DIRECTION_FTB;

	return sprintf(buf, "%s\n", ftb ? "front-to-back" : "back-to-front");
}

static SYSFS_ATTR_RO(fan1_air_flow, airflow_show);
static SYSFS_ATTR_RO(fan2_air_flow, airflow_show);
static SYSFS_ATTR_RO(fan3_air_flow, airflow_show);
static SYSFS_ATTR_RO(fan4_air_flow, airflow_show);
static SYSFS_ATTR_RO(fan5_air_flow, airflow_show);
static SYSFS_ATTR_RO(fan6_air_flow, airflow_show);
static SYSFS_ATTR_RO(fan7_air_flow, airflow_show);
static SYSFS_ATTR_RO(fan8_air_flow, airflow_show);
static SYSFS_ATTR_RO(fan9_air_flow, airflow_show);
static SYSFS_ATTR_RO(fan10_air_flow, airflow_show);

static ssize_t rpm_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int fan;
	int rpmaddr;
	uint8_t reg;
	int rpm;

	BUG_ON(strlen(dattr->attr.name) < 4);
	fan = simple_strtoul(&dattr->attr.name[3], NULL, 10) - 1;

	BUG_ON(fan > ARRAY_SIZE(fan_rpm));
	rpmaddr = fan_rpm[fan];

	reg = i2c_smbus_read_byte_data(client, rpmaddr);

	/*
	 * From ES6622BT CPLD spec v10:
	 * RPM = Counter Clock * 60(sec->min) / 4 (two-pole fan) / Fan sensor value
	 *   Counter clock = 74.17KHz
	 */

	if (reg == 255) {
		rpm = 0;
	} else if (reg > 0) {
		rpm = 74170 * 60 / 4 / reg;
	} else {
		rpm = -1;
	}

	return sprintf(buf, "%u\n", rpm);
}
static SYSFS_ATTR_RO(fan1_input, rpm_show);
static SYSFS_ATTR_RO(fan2_input, rpm_show);
static SYSFS_ATTR_RO(fan3_input, rpm_show);
static SYSFS_ATTR_RO(fan4_input, rpm_show);
static SYSFS_ATTR_RO(fan5_input, rpm_show);
static SYSFS_ATTR_RO(fan6_input, rpm_show);
static SYSFS_ATTR_RO(fan7_input, rpm_show);
static SYSFS_ATTR_RO(fan8_input, rpm_show);
static SYSFS_ATTR_RO(fan9_input, rpm_show);
static SYSFS_ATTR_RO(fan10_input, rpm_show);

static ssize_t fan_ok_show(struct device *dev,
			   struct device_attribute *dattr,
			   char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	uint8_t reg;
	int ok_mask;
	bool ok;
	int fan;

	BUG_ON(strlen(dattr->attr.name) < 4);
	fan = simple_strtoul(&dattr->attr.name[3], NULL, 10) - 1;

	BUG_ON(fan > ARRAY_SIZE(fan_ok));
	ok_mask = fan_ok[fan];

	reg = i2c_smbus_read_byte_data(client, CPLD_FAN_LED_NORMAL);
	ok = (reg & ok_mask) == 0;

	return sprintf(buf, "%u\n", ok);
}
static SYSFS_ATTR_RO(fan1_all_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan2_all_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan3_all_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan4_all_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan5_all_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan6_all_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan7_all_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan8_all_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan9_all_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan10_all_ok, fan_ok_show);


static struct attribute *fanpld_attrs[] = {
	&dev_attr_pwm1.attr,
	&dev_attr_pwm1_enable.attr,
	&dev_attr_fan1_air_flow.attr,
	&dev_attr_fan2_air_flow.attr,
	&dev_attr_fan3_air_flow.attr,
	&dev_attr_fan4_air_flow.attr,
	&dev_attr_fan5_air_flow.attr,
	&dev_attr_fan6_air_flow.attr,
	&dev_attr_fan7_air_flow.attr,
	&dev_attr_fan8_air_flow.attr,
	&dev_attr_fan9_air_flow.attr,
	&dev_attr_fan10_air_flow.attr,
	&dev_attr_fan1_input.attr,
	&dev_attr_fan2_input.attr,
	&dev_attr_fan3_input.attr,
	&dev_attr_fan4_input.attr,
	&dev_attr_fan5_input.attr,
	&dev_attr_fan6_input.attr,
	&dev_attr_fan7_input.attr,
	&dev_attr_fan8_input.attr,
	&dev_attr_fan9_input.attr,
	&dev_attr_fan10_input.attr,
	&dev_attr_fan1_all_ok.attr,
	&dev_attr_fan2_all_ok.attr,
	&dev_attr_fan3_all_ok.attr,
	&dev_attr_fan4_all_ok.attr,
	&dev_attr_fan5_all_ok.attr,
	&dev_attr_fan6_all_ok.attr,
	&dev_attr_fan7_all_ok.attr,
	&dev_attr_fan8_all_ok.attr,
	&dev_attr_fan9_all_ok.attr,
	&dev_attr_fan10_all_ok.attr,
	NULL
};

static struct attribute_group fanpld_attr_group = {
	.attrs = fanpld_attrs,
};

static int fanpld_remove(struct i2c_client *client)
{
	struct fanpld_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &fanpld_attr_group);

	return 0;
}

static int fanpld_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
	struct fanpld_data *data;
	int rv;

	if (!(data = kzalloc(sizeof(struct fanpld_data), GFP_KERNEL))) {
		dev_err(&client->dev, "out of memory.\n");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, data);

	rv = sysfs_create_group(&client->dev.kobj, &fanpld_attr_group);
	if (rv < 0) {
		dev_err(&client->dev, "failed to create sysfs group\n");
		goto done;
	}

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (data->hwmon_dev < 0) {
		rv = -ENODEV;
		goto done;
	}

done:
	if (rv < 0) {
		kfree(data);
	}

	return rv;
}

static struct i2c_driver fanpld_driver = {
        .driver = {
                .name   = "as670x_32x_fanpld",
                .owner  = THIS_MODULE,
        },
        .probe          = fanpld_probe,
        .remove         = fanpld_remove,
        .id_table       = fanpld_id,
};

static int __init fanpld_init(void)
{
	return i2c_add_driver(&fanpld_driver);
}

static void __exit fanpld_exit(void)
{
	i2c_del_driver(&fanpld_driver);
}

MODULE_AUTHOR("Dustin Byford <dustin@cumulusnetworks.com>");
MODULE_DESCRIPTION("FANPLD driver for Accton Technology Corporation AS670x-32X");
MODULE_LICENSE("GPL");

module_init(fanpld_init);
module_exit(fanpld_exit);
