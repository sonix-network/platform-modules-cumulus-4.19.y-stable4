/*
 * quanta_ly9_rangeley_cpld.c - Quanta LY9 CPLD Driver to access QSFP info.
 *
 * Copyright (C) 2014 Cumulus Networks, Inc.
 * Author: Puneet Shenoy (puneet@cumulusnetworks.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/device.h>

enum ly9_cpld_regs {
	LY9_IO_GRP = 1,
	LY9_LED_CTRL = 2,
};

enum ly9_qsfp_idx {
	LY9_QSFP50 = 0,
	LY9_QSFP49,
	LY9_QSFP52,
	LY9_QSFP51,
};
#define LY9_QSFP_MAX_IDX LY9_QSFP51

enum qsfp_field {
	LY9_LPMODE = 0,
	LY9_MOD_ABS,
	LY9_INTL,
	LY9_RST_N,
};
#define LY9_FIELD_MAX_IDX LY9_RST_N

/*
 * The QSFP IO GROUP Register has 16 bits, divided in groups of 4,
 * starting from QSFP 52 at LSB on to QSFP 49 at the MSB. Each group has
 * 4 bits for QSFP info, i.e., lpmode, present, interrupt and reset.
 *
 */

static ssize_t qsfp_show(struct device *dev,
			 struct device_attribute *da, char *buf)
{
	struct sensor_device_attribute_2 *attr2 = to_sensor_dev_attr_2(da);
	struct i2c_client *client = to_i2c_client(dev);
	u8 index = attr2->index;
	u8 field = attr2->nr;
	int val;

	if ((index > LY9_QSFP_MAX_IDX) || (field > LY9_FIELD_MAX_IDX))
		return -EINVAL;

	val = i2c_smbus_read_word_data(client, LY9_IO_GRP);
	if (val < 0)
		return val;
	
	if ((val & (1 << (index * 4 + field))) > 0)
		return sprintf(buf, "1\n");

	return sprintf(buf, "0\n");
}

static ssize_t qsfp_store(struct device *dev,
			  struct device_attribute *da,
			  const char *buf, size_t count)
{
	struct sensor_device_attribute_2 *attr2 = to_sensor_dev_attr_2(da);
	struct i2c_client *client = to_i2c_client(dev);
	u8 index = attr2->index;
	u8 field = attr2->nr;
	int ret;
	unsigned long value;


	if ((index > LY9_QSFP_MAX_IDX) || (field > LY9_FIELD_MAX_IDX))
		return -EINVAL;

	if (kstrtoul(buf, 0, &value) < 0)
		return -EINVAL;

	ret = i2c_smbus_read_word_data(client, LY9_IO_GRP);

	if (ret < 0)
		return ret;

	if (value > 0)
		value = ret  | (1 << (index * 4 + field));
	else
		value = ret & (~(1 << (index * 4 + field)));

	value &= 0xffff;
	ret = i2c_smbus_write_word_data(client, LY9_IO_GRP, value);

	if (ret < 0)
		return ret;

	return count;
}

static ssize_t led_mode_store(struct device *dev, struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);	
	int ret, val;

	if (sscanf(buf, "%d", &val) <= 0)
		return -EINVAL;

	ret = i2c_smbus_write_word_data(client, LY9_LED_CTRL, val);

	if (ret < 0)
		return ret;
	return count;
}

static ssize_t led_mode_show(struct device *dev,
			 struct device_attribute *da, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);	
	int ret;

	ret = i2c_smbus_read_word_data(client, LY9_LED_CTRL);

	if (ret < 0)
		return ret;
	return sprintf(buf, "%d\n", ret);
}

/* Needed for debugging purposes */
static ssize_t dump_io_grp(struct device *dev,
			   struct device_attribute *da, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int val;

	val = i2c_smbus_read_word_data(client, LY9_IO_GRP);
	if (val < 0)
		return val;
	return sprintf(buf, "0x%x\n", val);
}

static SENSOR_DEVICE_ATTR_2(qsfp49_lpmode, S_IWUSR | S_IRUGO, qsfp_show,
			    qsfp_store, LY9_LPMODE, LY9_QSFP49);
static SENSOR_DEVICE_ATTR_2(qsfp50_lpmode, S_IWUSR | S_IRUGO, qsfp_show,
			    qsfp_store, LY9_LPMODE, LY9_QSFP50);
static SENSOR_DEVICE_ATTR_2(qsfp51_lpmode, S_IWUSR | S_IRUGO, qsfp_show,
			    qsfp_store, LY9_LPMODE, LY9_QSFP51);
static SENSOR_DEVICE_ATTR_2(qsfp52_lpmode, S_IWUSR | S_IRUGO, qsfp_show,
			    qsfp_store, LY9_LPMODE, LY9_QSFP52);

static SENSOR_DEVICE_ATTR_2(qsfp49_present, S_IRUGO, qsfp_show, NULL,
			    LY9_MOD_ABS, LY9_QSFP49);
static SENSOR_DEVICE_ATTR_2(qsfp50_present, S_IRUGO, qsfp_show, NULL,
			    LY9_MOD_ABS, LY9_QSFP50);
static SENSOR_DEVICE_ATTR_2(qsfp51_present, S_IRUGO, qsfp_show, NULL,
			    LY9_MOD_ABS, LY9_QSFP51);
static SENSOR_DEVICE_ATTR_2(qsfp52_present, S_IRUGO, qsfp_show, NULL,
			    LY9_MOD_ABS, LY9_QSFP52);

static SENSOR_DEVICE_ATTR_2(qsfp49_reset, S_IWUSR | S_IRUGO, qsfp_show,
			    qsfp_store, LY9_RST_N, LY9_QSFP49);
static SENSOR_DEVICE_ATTR_2(qsfp50_reset, S_IWUSR | S_IRUGO, qsfp_show,
			    qsfp_store, LY9_RST_N, LY9_QSFP50);
static SENSOR_DEVICE_ATTR_2(qsfp51_reset, S_IWUSR | S_IRUGO, qsfp_show,
			    qsfp_store, LY9_RST_N, LY9_QSFP51);
static SENSOR_DEVICE_ATTR_2(qsfp52_reset, S_IWUSR | S_IRUGO, qsfp_show,
			    qsfp_store, LY9_RST_N, LY9_QSFP52);

static SENSOR_DEVICE_ATTR_2(qsfp49_int, S_IRUGO, qsfp_show, NULL, LY9_INTL,
			    LY9_QSFP49);
static SENSOR_DEVICE_ATTR_2(qsfp50_int, S_IRUGO, qsfp_show, NULL, LY9_INTL,
			    LY9_QSFP50);
static SENSOR_DEVICE_ATTR_2(qsfp51_int, S_IRUGO, qsfp_show, NULL, LY9_INTL,
			    LY9_QSFP51);
static SENSOR_DEVICE_ATTR_2(qsfp52_int, S_IRUGO, qsfp_show, NULL, LY9_INTL,
			    LY9_QSFP52);

static DEVICE_ATTR(qsfp_led_mode, S_IRUGO | S_IWUSR, led_mode_show,
		   led_mode_store);
static DEVICE_ATTR(io_dump, S_IRUGO, dump_io_grp, NULL);

static struct attribute *qsfp_attrs[] = {
	&sensor_dev_attr_qsfp49_lpmode.dev_attr.attr,
	&sensor_dev_attr_qsfp50_lpmode.dev_attr.attr,
	&sensor_dev_attr_qsfp51_lpmode.dev_attr.attr,
	&sensor_dev_attr_qsfp52_lpmode.dev_attr.attr,

	&sensor_dev_attr_qsfp49_present.dev_attr.attr,
	&sensor_dev_attr_qsfp50_present.dev_attr.attr,
	&sensor_dev_attr_qsfp51_present.dev_attr.attr,
	&sensor_dev_attr_qsfp52_present.dev_attr.attr,

	&sensor_dev_attr_qsfp49_reset.dev_attr.attr,
	&sensor_dev_attr_qsfp50_reset.dev_attr.attr,
	&sensor_dev_attr_qsfp51_reset.dev_attr.attr,
	&sensor_dev_attr_qsfp52_reset.dev_attr.attr,

	&sensor_dev_attr_qsfp49_int.dev_attr.attr,
	&sensor_dev_attr_qsfp50_int.dev_attr.attr,
	&sensor_dev_attr_qsfp51_int.dev_attr.attr,
	&sensor_dev_attr_qsfp52_int.dev_attr.attr,

	&dev_attr_qsfp_led_mode.attr,
	&dev_attr_io_dump.attr,
	NULL,
};

static const struct attribute_group qsfp_attr_group = {
	.attrs = qsfp_attrs,
};

static int quanta_ly9_rangeley_cpld_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WORD_DATA))
		return -ENODEV;
	return sysfs_create_group(&client->dev.kobj, &qsfp_attr_group);
}

static int quanta_ly9_rangeley_cpld_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &qsfp_attr_group);
	return 0;
}

static const struct i2c_device_id quanta_ly9_rangeley_cpld_id[] = {
	{ "ly9_rangeley_cpld", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, quanta_ly9_rangeley_cpld_id);

static struct i2c_driver quanta_ly9_rangeley_cpld_driver = {
	.driver = {
		.name = "ly9_rangeley_cpld",
	},
	.probe = quanta_ly9_rangeley_cpld_probe,
	.remove = quanta_ly9_rangeley_cpld_remove,
	.id_table = quanta_ly9_rangeley_cpld_id,
};

static int __init quanta_ly9_rangeley_cpld_init(void)
{
	return i2c_add_driver(&quanta_ly9_rangeley_cpld_driver);
}

static void __exit quanta_ly9_rangeley_cpld_exit(void)
{
	i2c_del_driver(&quanta_ly9_rangeley_cpld_driver);
}

MODULE_AUTHOR("Puneet Shenoy <puneet@cumulusnetworks.com>");
MODULE_DESCRIPTION("Quanta LY9 Rangeley CPLD driver");
MODULE_LICENSE("GPL");

module_init(quanta_ly9_rangeley_cpld_init);
module_exit(quanta_ly9_rangeley_cpld_exit);
