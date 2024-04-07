/*
 * cumulus_vx_platform.c - Cumulus VX Platform Support.
 *
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
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>

#include "platform-defs.h"

#define DRIVER_NAME	"cumulus_vx_platform"
#define DRIVER_VERSION	"0.1"

/* Temp sensors */
#define VX_TEMP_NUMS   7
#define VX_TEMP_TYPES  5
enum {
	TEMP_LCRIT = 0,
	TEMP_MIN,
	TEMP_INPUT,
	TEMP_MAX,
	TEMP_CRIT,
};

/* Fans */
#define VX_FAN_NUMS    8
#define VX_FAN_TYPES   6
enum {
	FAN_PRESENT = 0,
	FAN_OK,
	FAN_MIN,
	FAN_INPUT,
	FAN_MAX,
	FAN_TARGET,
};

/* PSUs */
#define VX_PSU_NUMS    2
#define VX_PSU_TYPES   2
enum {
	PSU_PRESENT = 0,
	PSU_OK,
};

/* LEDs */
#define VX_LED_NUMS    3
#define VX_LED_STR_LEN 8
#define VX_LED_GREEN  "green"
#define VX_LED_YELLOW "yellow"
enum {
	LED_PSU = 0,
	LED_FAN,
	LED_SYSTEM,
};

/* default values */
#define VX_TEMP_INPUT 25
#define VX_TEMP_MAX   80
#define VX_TEMP_CRIT  85
#define VX_TEMP_MIN   5
#define VX_TEMP_LCRIT 0

#define VX_FAN_INPUT   6000
#define VX_FAN_MAX     18000
#define VX_FAN_MIN     3000
#define VX_FAN_TARGET  6000
#define VX_FAN_PRESENT 1
#define VX_FAN_OK      1

#define VX_PSU_PRESENT 1
#define VX_PSU_OK      1

#define EEPROM_SIZE 256
static char eeprom_buf[EEPROM_SIZE];
static struct eeprom_device *eeprom_dev;

struct vx_sensors_t {
	int temp[VX_TEMP_TYPES][VX_TEMP_NUMS];
	int fan[VX_FAN_TYPES][VX_FAN_NUMS];
	int psu[VX_PSU_TYPES][VX_PSU_NUMS];
	char led[VX_LED_NUMS][VX_LED_STR_LEN];
};

static ssize_t vx_temp_show(struct device *dev,
			    struct device_attribute *dattr,
			    char *buf)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	struct vx_sensors_t *data = dev_get_drvdata(dev);
	int idx = attr->index - 1;
	int type = attr->nr;

	return sprintf(buf, "%d\n", data->temp[type][idx]);
}

static ssize_t vx_temp_store(struct device *dev,
			     struct device_attribute *dattr,
			     const char *buf, size_t count)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	struct vx_sensors_t *data = dev_get_drvdata(dev);
	int idx = attr->index - 1;
	int type = attr->nr;
	long tmp;

	if (kstrtol(buf, 10, &tmp))
		return -EINVAL;

	data->temp[type][idx] = tmp;
	return count;
}

static SENSOR_DEVICE_ATTR_2(temp1_input, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_INPUT, 1);
static SENSOR_DEVICE_ATTR_2(temp2_input, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_INPUT, 2);
static SENSOR_DEVICE_ATTR_2(temp1_min, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MIN, 1);
static SENSOR_DEVICE_ATTR_2(temp2_min, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MIN, 2);
static SENSOR_DEVICE_ATTR_2(temp1_max, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MAX, 1);
static SENSOR_DEVICE_ATTR_2(temp2_max, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MAX, 2);
static SENSOR_DEVICE_ATTR_2(temp1_crit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_CRIT, 1);
static SENSOR_DEVICE_ATTR_2(temp2_crit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_CRIT, 2);
static SENSOR_DEVICE_ATTR_2(temp1_lcrit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_LCRIT, 1);
static SENSOR_DEVICE_ATTR_2(temp2_lcrit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_LCRIT, 2);

static SENSOR_DEVICE_ATTR_2(temp3_input, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_INPUT, 3);
static SENSOR_DEVICE_ATTR_2(temp4_input, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_INPUT, 4);
static SENSOR_DEVICE_ATTR_2(temp3_min, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MIN, 3);
static SENSOR_DEVICE_ATTR_2(temp4_min, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MIN, 4);
static SENSOR_DEVICE_ATTR_2(temp3_max, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MAX, 3);
static SENSOR_DEVICE_ATTR_2(temp4_max, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MAX, 4);
static SENSOR_DEVICE_ATTR_2(temp3_crit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_CRIT, 3);
static SENSOR_DEVICE_ATTR_2(temp4_crit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_CRIT, 4);
static SENSOR_DEVICE_ATTR_2(temp3_lcrit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_LCRIT, 3);
static SENSOR_DEVICE_ATTR_2(temp4_lcrit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_LCRIT, 4);

static SENSOR_DEVICE_ATTR_2(temp5_input, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_INPUT, 5);
static SENSOR_DEVICE_ATTR_2(temp6_input, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_INPUT, 6);
static SENSOR_DEVICE_ATTR_2(temp5_min, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MIN, 5);
static SENSOR_DEVICE_ATTR_2(temp6_min, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MIN, 6);
static SENSOR_DEVICE_ATTR_2(temp5_max, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MAX, 5);
static SENSOR_DEVICE_ATTR_2(temp6_max, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MAX, 6);
static SENSOR_DEVICE_ATTR_2(temp5_crit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_CRIT, 5);
static SENSOR_DEVICE_ATTR_2(temp6_crit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_CRIT, 6);
static SENSOR_DEVICE_ATTR_2(temp5_lcrit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_LCRIT, 5);
static SENSOR_DEVICE_ATTR_2(temp6_lcrit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_LCRIT, 6);

static SENSOR_DEVICE_ATTR_2(temp7_input, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_INPUT, 7);
static SENSOR_DEVICE_ATTR_2(temp7_min, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MIN, 7);
static SENSOR_DEVICE_ATTR_2(temp7_max, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_MAX, 7);
static SENSOR_DEVICE_ATTR_2(temp7_crit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_CRIT, 7);
static SENSOR_DEVICE_ATTR_2(temp7_lcrit, S_IRUGO | S_IWUSR, vx_temp_show,
			    vx_temp_store, TEMP_LCRIT, 7);

static ssize_t vx_fan_show(struct device *dev,
			   struct device_attribute *dattr,
			   char *buf)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	struct vx_sensors_t *data = dev_get_drvdata(dev);
	int idx = attr->index - 1;
	int type = attr->nr;

	return sprintf(buf, "%d\n", data->fan[type][idx]);
}

static ssize_t vx_fan_store(struct device *dev,
			    struct device_attribute *dattr,
			    const char *buf, size_t count)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	struct vx_sensors_t *data = dev_get_drvdata(dev);
	int idx = attr->index - 1;
	int type = attr->nr;
	long tmp;

	if (kstrtol(buf, 10, &tmp))
		return -EINVAL;
	data->fan[type][idx] = tmp;
	if (type == FAN_TARGET)
		data->fan[FAN_INPUT][idx] = tmp;
	return count;
}

static SENSOR_DEVICE_ATTR_2(fan1_input, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_INPUT, 1);
static SENSOR_DEVICE_ATTR_2(fan2_input, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_INPUT, 2);
static SENSOR_DEVICE_ATTR_2(fan1_min, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MIN, 1);
static SENSOR_DEVICE_ATTR_2(fan2_min, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MIN, 2);
static SENSOR_DEVICE_ATTR_2(fan1_max, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MAX, 1);
static SENSOR_DEVICE_ATTR_2(fan2_max, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MAX, 2);
static SENSOR_DEVICE_ATTR_2(fan1_present, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_PRESENT, 1);
static SENSOR_DEVICE_ATTR_2(fan2_present, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_PRESENT, 2);
static SENSOR_DEVICE_ATTR_2(fan1_ok, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_OK, 1);
static SENSOR_DEVICE_ATTR_2(fan2_ok, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_OK, 2);
static SENSOR_DEVICE_ATTR_2(fan1_target, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_TARGET, 1);
static SENSOR_DEVICE_ATTR_2(fan2_target, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_TARGET, 2);

static SENSOR_DEVICE_ATTR_2(fan3_input, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_INPUT, 3);
static SENSOR_DEVICE_ATTR_2(fan4_input, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_INPUT, 4);
static SENSOR_DEVICE_ATTR_2(fan3_min, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MIN, 3);
static SENSOR_DEVICE_ATTR_2(fan4_min, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MIN, 4);
static SENSOR_DEVICE_ATTR_2(fan3_max, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MAX, 3);
static SENSOR_DEVICE_ATTR_2(fan4_max, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MAX, 4);
static SENSOR_DEVICE_ATTR_2(fan3_present, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_PRESENT, 3);
static SENSOR_DEVICE_ATTR_2(fan4_present, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_PRESENT, 4);
static SENSOR_DEVICE_ATTR_2(fan3_ok, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_OK, 3);
static SENSOR_DEVICE_ATTR_2(fan4_ok, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_OK, 4);
static SENSOR_DEVICE_ATTR_2(fan3_target, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_TARGET, 3);
static SENSOR_DEVICE_ATTR_2(fan4_target, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_TARGET, 4);

static SENSOR_DEVICE_ATTR_2(fan5_input, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_INPUT, 5);
static SENSOR_DEVICE_ATTR_2(fan6_input, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_INPUT, 6);
static SENSOR_DEVICE_ATTR_2(fan5_min, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MIN, 5);
static SENSOR_DEVICE_ATTR_2(fan6_min, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MIN, 6);
static SENSOR_DEVICE_ATTR_2(fan5_max, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MAX, 5);
static SENSOR_DEVICE_ATTR_2(fan6_max, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MAX, 6);
static SENSOR_DEVICE_ATTR_2(fan5_present, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_PRESENT, 5);
static SENSOR_DEVICE_ATTR_2(fan6_present, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_PRESENT, 6);
static SENSOR_DEVICE_ATTR_2(fan5_ok, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_OK, 5);
static SENSOR_DEVICE_ATTR_2(fan6_ok, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_OK, 6);
static SENSOR_DEVICE_ATTR_2(fan5_target, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_TARGET, 5);
static SENSOR_DEVICE_ATTR_2(fan6_target, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_TARGET, 6);

static SENSOR_DEVICE_ATTR_2(fan7_input, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_INPUT, 7);
static SENSOR_DEVICE_ATTR_2(fan8_input, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_INPUT, 8);
static SENSOR_DEVICE_ATTR_2(fan7_min, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MIN, 7);
static SENSOR_DEVICE_ATTR_2(fan8_min, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MIN, 8);
static SENSOR_DEVICE_ATTR_2(fan7_max, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MAX, 7);
static SENSOR_DEVICE_ATTR_2(fan8_max, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_MAX, 8);
static SENSOR_DEVICE_ATTR_2(fan7_present, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_PRESENT, 7);
static SENSOR_DEVICE_ATTR_2(fan8_present, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_PRESENT, 8);
static SENSOR_DEVICE_ATTR_2(fan7_ok, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_OK, 7);
static SENSOR_DEVICE_ATTR_2(fan8_ok, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_OK, 8);
static SENSOR_DEVICE_ATTR_2(fan7_target, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_TARGET, 7);
static SENSOR_DEVICE_ATTR_2(fan8_target, S_IRUGO | S_IWUSR, vx_fan_show,
			    vx_fan_store, FAN_TARGET, 8);

static ssize_t vx_psu_show(struct device *dev,
			   struct device_attribute *dattr,
			   char *buf)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	struct vx_sensors_t *data = dev_get_drvdata(dev);
	int idx = attr->index - 1;
	int type = attr->nr;

	return sprintf(buf, "%d\n", data->psu[type][idx]);
}

static ssize_t vx_psu_store(struct device *dev,
			    struct device_attribute *dattr,
			    const char *buf, size_t count)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	struct vx_sensors_t *data = dev_get_drvdata(dev);
	int idx = attr->index - 1;
	int type = attr->nr;
	long tmp;

	if (kstrtol(buf, 10, &tmp))
		return -EINVAL;

	data->psu[type][idx] = tmp;
	return count;
}

static SENSOR_DEVICE_ATTR_2(psu1_present, S_IRUGO | S_IWUSR, vx_psu_show,
			    vx_psu_store, PSU_PRESENT, 1);
static SENSOR_DEVICE_ATTR_2(psu2_present, S_IRUGO | S_IWUSR, vx_psu_show,
			    vx_psu_store, PSU_PRESENT, 2);
static SENSOR_DEVICE_ATTR_2(psu1_ok, S_IRUGO | S_IWUSR, vx_psu_show,
			    vx_psu_store, PSU_OK, 1);
static SENSOR_DEVICE_ATTR_2(psu2_ok, S_IRUGO | S_IWUSR, vx_psu_show,
			    vx_psu_store, PSU_OK, 2);

static ssize_t vx_led_show(struct device *dev,
			   struct device_attribute *dattr,
			   char *buf)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	struct vx_sensors_t *data = dev_get_drvdata(dev);
	int type = attr->nr;

	return sprintf(buf, "%s\n", data->led[type]);
}

static ssize_t vx_led_store(struct device *dev,
			    struct device_attribute *dattr,
			    const char *buf, size_t count)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	struct vx_sensors_t *data = dev_get_drvdata(dev);
	int type = attr->nr;
	char tmp[VX_LED_STR_LEN];
	int ret;

	ret = sscanf(buf, "%s", tmp);
	if (ret <= 0)
		return -EINVAL;
	if ((strcmp(VX_LED_GREEN, tmp)) && (strcmp(VX_LED_YELLOW, tmp)))
		return -EINVAL;
	strcpy(data->led[type], tmp);
	return count;
}

static SENSOR_DEVICE_ATTR_2(led_psu, S_IRUGO | S_IWUSR, vx_led_show,
			    vx_led_store, LED_PSU, 0);
static SENSOR_DEVICE_ATTR_2(led_fan, S_IRUGO | S_IWUSR, vx_led_show,
			    vx_led_store, LED_FAN, 0);
static SENSOR_DEVICE_ATTR_2(led_system, S_IRUGO | S_IWUSR, vx_led_show,
			    vx_led_store, LED_SYSTEM, 0);

static struct attribute *cumulus_vx_cpld_attrs[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp1_crit.dev_attr.attr,
	&sensor_dev_attr_temp2_crit.dev_attr.attr,
	&sensor_dev_attr_temp1_lcrit.dev_attr.attr,
	&sensor_dev_attr_temp2_lcrit.dev_attr.attr,
	&sensor_dev_attr_temp1_max.dev_attr.attr,
	&sensor_dev_attr_temp2_max.dev_attr.attr,
	&sensor_dev_attr_temp1_min.dev_attr.attr,
	&sensor_dev_attr_temp2_min.dev_attr.attr,

	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp4_input.dev_attr.attr,
	&sensor_dev_attr_temp3_crit.dev_attr.attr,
	&sensor_dev_attr_temp4_crit.dev_attr.attr,
	&sensor_dev_attr_temp3_lcrit.dev_attr.attr,
	&sensor_dev_attr_temp4_lcrit.dev_attr.attr,
	&sensor_dev_attr_temp3_max.dev_attr.attr,
	&sensor_dev_attr_temp4_max.dev_attr.attr,
	&sensor_dev_attr_temp3_min.dev_attr.attr,
	&sensor_dev_attr_temp4_min.dev_attr.attr,

	&sensor_dev_attr_temp5_input.dev_attr.attr,
	&sensor_dev_attr_temp6_input.dev_attr.attr,
	&sensor_dev_attr_temp5_crit.dev_attr.attr,
	&sensor_dev_attr_temp6_crit.dev_attr.attr,
	&sensor_dev_attr_temp5_lcrit.dev_attr.attr,
	&sensor_dev_attr_temp6_lcrit.dev_attr.attr,
	&sensor_dev_attr_temp5_max.dev_attr.attr,
	&sensor_dev_attr_temp6_max.dev_attr.attr,
	&sensor_dev_attr_temp5_min.dev_attr.attr,
	&sensor_dev_attr_temp6_min.dev_attr.attr,

	&sensor_dev_attr_temp7_input.dev_attr.attr,
	&sensor_dev_attr_temp7_crit.dev_attr.attr,
	&sensor_dev_attr_temp7_lcrit.dev_attr.attr,
	&sensor_dev_attr_temp7_max.dev_attr.attr,
	&sensor_dev_attr_temp7_min.dev_attr.attr,

	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan1_present.dev_attr.attr,
	&sensor_dev_attr_fan2_present.dev_attr.attr,
	&sensor_dev_attr_fan1_ok.dev_attr.attr,
	&sensor_dev_attr_fan2_ok.dev_attr.attr,
	&sensor_dev_attr_fan1_max.dev_attr.attr,
	&sensor_dev_attr_fan2_max.dev_attr.attr,
	&sensor_dev_attr_fan1_min.dev_attr.attr,
	&sensor_dev_attr_fan2_min.dev_attr.attr,
	&sensor_dev_attr_fan1_target.dev_attr.attr,
	&sensor_dev_attr_fan2_target.dev_attr.attr,

	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan3_present.dev_attr.attr,
	&sensor_dev_attr_fan4_present.dev_attr.attr,
	&sensor_dev_attr_fan3_ok.dev_attr.attr,
	&sensor_dev_attr_fan4_ok.dev_attr.attr,
	&sensor_dev_attr_fan3_max.dev_attr.attr,
	&sensor_dev_attr_fan4_max.dev_attr.attr,
	&sensor_dev_attr_fan3_min.dev_attr.attr,
	&sensor_dev_attr_fan4_min.dev_attr.attr,
	&sensor_dev_attr_fan3_target.dev_attr.attr,
	&sensor_dev_attr_fan4_target.dev_attr.attr,

	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan5_present.dev_attr.attr,
	&sensor_dev_attr_fan6_present.dev_attr.attr,
	&sensor_dev_attr_fan5_ok.dev_attr.attr,
	&sensor_dev_attr_fan6_ok.dev_attr.attr,
	&sensor_dev_attr_fan5_max.dev_attr.attr,
	&sensor_dev_attr_fan6_max.dev_attr.attr,
	&sensor_dev_attr_fan5_min.dev_attr.attr,
	&sensor_dev_attr_fan6_min.dev_attr.attr,
	&sensor_dev_attr_fan5_target.dev_attr.attr,
	&sensor_dev_attr_fan6_target.dev_attr.attr,

	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,
	&sensor_dev_attr_fan7_present.dev_attr.attr,
	&sensor_dev_attr_fan8_present.dev_attr.attr,
	&sensor_dev_attr_fan7_ok.dev_attr.attr,
	&sensor_dev_attr_fan8_ok.dev_attr.attr,
	&sensor_dev_attr_fan7_max.dev_attr.attr,
	&sensor_dev_attr_fan8_max.dev_attr.attr,
	&sensor_dev_attr_fan7_min.dev_attr.attr,
	&sensor_dev_attr_fan8_min.dev_attr.attr,
	&sensor_dev_attr_fan7_target.dev_attr.attr,
	&sensor_dev_attr_fan8_target.dev_attr.attr,

	&sensor_dev_attr_psu1_present.dev_attr.attr,
	&sensor_dev_attr_psu2_present.dev_attr.attr,
	&sensor_dev_attr_psu1_ok.dev_attr.attr,
	&sensor_dev_attr_psu2_ok.dev_attr.attr,

	&sensor_dev_attr_led_psu.dev_attr.attr,
	&sensor_dev_attr_led_fan.dev_attr.attr,
	&sensor_dev_attr_led_system.dev_attr.attr,

	NULL,
};
ATTRIBUTE_GROUPS(cumulus_vx_cpld);

static ssize_t eeprom_read(struct file *filp, struct kobject *kobj,
			   struct bin_attribute *bin_attr,
			   char *buf, loff_t off, size_t count)
{
	if (off > EEPROM_SIZE)
		return -EINVAL;
	if (off + count > EEPROM_SIZE)
		return -EINVAL;

	memcpy(buf, eeprom_buf+off, count);
	return count;
}

static ssize_t eeprom_write(struct file *file, struct kobject *kobj,
			    struct bin_attribute *bin_attr,
			    char *buf, loff_t off, size_t count)
{
	if (off > EEPROM_SIZE)
		return -EINVAL;
	if (off + count > EEPROM_SIZE)
		return -EINVAL;

	memcpy(eeprom_buf+off, buf, count);
	return count;
}

static struct bin_attribute eeprom_attr = {
	.attr = {
		.name = "eeprom",
		.mode = S_IRUGO | S_IWUSR,
	},
	.size = EEPROM_SIZE,
	.read = eeprom_read,
	.write = eeprom_write,
};

struct eeprom_platform_data eeprom_label = {
	.label = "board_eeprom",
};

static int cumulus_vx_cpld_probe(struct platform_device *pdev)
{
	struct vx_sensors_t *data;
	struct device *hwmon_dev;
	int i, ret;
	struct device *dev = &pdev->dev;

	data = devm_kzalloc(dev, sizeof(struct vx_sensors_t), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	for (i = 0; i < VX_TEMP_NUMS; i++) {
		data->temp[TEMP_INPUT][i] = VX_TEMP_INPUT * 1000;
		data->temp[TEMP_CRIT][i] = VX_TEMP_CRIT * 1000;
		data->temp[TEMP_MAX][i] = VX_TEMP_MAX * 1000;
		data->temp[TEMP_MIN][i] = VX_TEMP_MIN * 1000;
		data->temp[TEMP_LCRIT][i] = VX_TEMP_LCRIT * 1000;
	}

	for (i = 0; i < VX_FAN_NUMS; i++) {
		data->fan[FAN_INPUT][i] = VX_FAN_INPUT;
		data->fan[FAN_MIN][i] = VX_FAN_MIN;
		data->fan[FAN_PRESENT][i] = VX_FAN_PRESENT;
		data->fan[FAN_OK][i] = VX_FAN_OK;
		data->fan[FAN_MAX][i] = VX_FAN_MAX;
		data->fan[FAN_TARGET][i] = VX_FAN_TARGET;
	}
	for (i = 0; i < VX_PSU_NUMS; i++) {
		data->psu[PSU_PRESENT][i] = VX_PSU_PRESENT;
		data->psu[PSU_OK][i] = VX_PSU_OK;
	}
	for (i = 0; i < VX_LED_NUMS; i++) {
		strcpy(data->led[LED_PSU], VX_LED_GREEN);
		strcpy(data->led[LED_FAN], VX_LED_GREEN);
		strcpy(data->led[LED_SYSTEM], VX_LED_GREEN);
	}

	hwmon_dev = devm_hwmon_device_register_with_groups(dev, pdev->name,
							   data,
						   cumulus_vx_cpld_groups);
	if (IS_ERR(hwmon_dev))
		return PTR_ERR(hwmon_dev);

	/* Board EEPROM */
	ret = sysfs_create_bin_file(&dev->kobj, &eeprom_attr);
	if (ret < 0) {
		dev_err(dev, "error creating binary file\n");
		return ret;
	}
	eeprom_dev = eeprom_device_register(dev, &eeprom_label);
	if (IS_ERR(eeprom_dev)) {
		dev_err(dev, "error registering eeprom device.\n");
		ret = PTR_ERR(eeprom_dev);
		return ret;
	}
	return 0;
}

static int cumulus_vx_cpld_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	/* Remove Board EEPROM */
	eeprom_device_unregister(eeprom_dev);
	sysfs_remove_bin_file(&dev->kobj, &eeprom_attr);
	return 0;
}

static struct platform_driver cumulus_vx_cpld_driver = {
	.driver = {
		.name = "cumulus_vx_cpld",
		.owner = THIS_MODULE,
	},
	.probe = cumulus_vx_cpld_probe,
	.remove = cumulus_vx_cpld_remove,
};

static struct platform_device *cumulus_vx_cpld_device;

static int __init cumulus_vx_cpld_init(void)
{
	int ret = platform_driver_register(&cumulus_vx_cpld_driver);

	if (ret) {
		pr_err("platform_driver_register() failed for cpld device");
		goto err_drvr;
	}

	cumulus_vx_cpld_device = platform_device_alloc("cumulus_vx_cpld", 0);
	if (!cumulus_vx_cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}

	ret = platform_device_add(cumulus_vx_cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_dev_add;
	}
	return 0;

err_dev_add:
	platform_device_put(cumulus_vx_cpld_device);

err_dev_alloc:
	platform_driver_unregister(&cumulus_vx_cpld_driver);

err_drvr:
	return ret;
}

static void cumulus_vx_cpld_exit(void)
{
	platform_driver_unregister(&cumulus_vx_cpld_driver);
	platform_device_unregister(cumulus_vx_cpld_device);
}

static int __init cumulus_vx_platform_init(void)
{
	int ret = 0;

	ret = cumulus_vx_cpld_init();
	if (ret) {
		pr_err("Registering CPLD driver failed.\n");
		cumulus_vx_cpld_exit();
		return ret;
	}
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;
}

static void __exit cumulus_vx_platform_exit(void)
{
	cumulus_vx_cpld_exit();
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " unloaded\n");
}

module_init(cumulus_vx_platform_init);
module_exit(cumulus_vx_platform_exit);

MODULE_AUTHOR("Puneet Shenoy (puneet@cumulusnetworks.com)");
MODULE_DESCRIPTION("Cumulus VX Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
