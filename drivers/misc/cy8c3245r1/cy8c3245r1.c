/*
 * A hwmon driver for the Cypress Semiconductor C3245
 * Copyright (C) 2014 Cumulus Networks
 *
 * Author: Shrijeet Mukherjee <shm@cumulusnetworks.com>
 * Author: Vidya Ravipati <vidya@cumulusnetworks.com>
 *
 * Based on the adt7470 driver
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
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/log2.h>
#include <linux/kthread.h>
#include <linux/slab.h>

/* cy8c3245r1 registers */
#define CY8C3245R1_REG_BASE_ADDR			0x00
#define CY8C3245R1_REG_DEV_ID			0x09
#define CY8C3245R1_REG_COMPANY_ID			0x05
#define CY8C3245R1_REG_FW_REV_MAJ			0x06
#define CY8C3245R1_REG_FW_REV_MIN			0x07
#define CY8C3245R1_REG_RESET			0x08

/*
 * Fan PWM / RPM Profile control registers
 *
 * These registers consist of two-bytes each
 */
#define CY8C3245R1_REG_FAN_PROFILE_BASE_ADDR	0x10
#define CY8C3245R1_REG_FAN_PROFILE(x) (CY8C3245R1_REG_FAN_PROFILE_BASE_ADDR + \
				       ((x) * 2))
enum {
	CY8C3245R1_FAN_PROFILE_LOW_DUTY = 0,
	CY8C3245R1_FAN_PROFILE_LOW_RPM,
	CY8C3245R1_FAN_PROFILE_HIGH_DUTY,
	CY8C3245R1_FAN_PROFILE_HIGH_RPM,
	CY8C3245R1_FAN_PROFILE_SPEED_0_DUTY,
	CY8C3245R1_FAN_PROFILE_SPEED_100_DUTY,
	CY8C3245R1_FAN_PROFILE_MAX
};

/* skipping over regs to set */

#define CY8C3245R1_REG_TEMP_BASE_ADDR		0x20

#define CY8C3245R1_REG_FAN_BASE_ADDR		0x40
#define CY8C3245R1_REG_FAN_TARGET_BASE_ADDR	0x3E

#define CY8C3245R1_REG_PWM_BASE_ADDR		0x3C

#define CY8C3245R1_REG_TEMP_LIMITS_BASE_ADDR	0x20
#define CY8C3245R1_REG_TEMP_LIMITS_MAX_ADDR	0x28

#define CY8C3245R1_REG_FAN_MAX_BASE_ADDR		0x16

#define CY8C3245R1_REG_PWM_CFG_BASE_ADDR		0x33

#define CY8C3245R1_TEMP_COUNT	8
#define CY8C3245R1_TEMP_REG(x)	(CY8C3245R1_REG_TEMP_BASE_ADDR + (x))
#define CY8C3245R1_TEMP_MAX_REG(x) (CY8C3245R1_REG_TEMP_LIMITS_MAX_ADDR + (x))

#define CY8C3245R1_FAN_COUNT	8
#define CY8C3245R1_REG_FAN(x)	(CY8C3245R1_REG_FAN_BASE_ADDR + ((x) * 2))

#define CY8C3245R1_REG_FAN_MIN(x)	(CY8C3245R1_REG_FAN_MIN_BASE_ADDR + \
					 ((x) * 2))
#define CY8C3245R1_REG_FAN_MAX(x)	(CY8C3245R1_REG_FAN_MAX_BASE_ADDR)
#define CY8C3245R1_REG_FAN_TARGET	(CY8C3245R1_REG_FAN_TARGET_BASE_ADDR)

#define CY8C3245R1_PWM_COUNT	1
#define CY8C3245R1_REG_PWM	(CY8C3245R1_REG_PWM_BASE_ADDR)

#define CY8C3245R1_COMPANY_ID	0xCC
#define CY8C3245R1_DEV_ID		0x09
#define CY8C3245R1_FW_REV_MAJ	0x02
#define CY8C3245R1_FW_REV_MIN	0x03

/* "all temps" according to hwmon sysfs interface spec */
#define CY8C3245R1_PWM_ALL_TEMPS	0x3FF

/* How often do we reread sensors values? (In jiffies) */
#define SENSOR_REFRESH_INTERVAL	(5 * HZ)

/* How often do we reread sensor limit values? (In jiffies) */
#define LIMIT_REFRESH_INTERVAL	(60 * HZ)

/* Wait at least 200ms per sensor for 10 sensors */
#define TEMP_COLLECTION_TIME	2000

/* auto update thing won't fire more than every 2s */
#define AUTO_UPDATE_INTERVAL	2000

/* datasheet says to divide this number by the fan reading to get fan rpm */
#define FAN_PERIOD_INVALID	65535
#define FAN_DATA_VALID(x)	((x) && (x) != FAN_PERIOD_INVALID)

struct cy8c3245r1_data {
	struct i2c_client	*client;
	struct mutex		lock;			/* Mutex Lock */
	char			sensors_valid;
	char			limits_valid;
	unsigned long		sensors_last_updated;	/* In jiffies */
	unsigned long		limits_last_updated;	/* In jiffies */

	int			num_temp_sensors;	/* -1 = probe */
	int			temperatures_probed;

	s8			temp[CY8C3245R1_TEMP_COUNT];
	s8			temp_max[CY8C3245R1_TEMP_COUNT];
	u16			fan[CY8C3245R1_FAN_COUNT];
	u16			fan_max[CY8C3245R1_FAN_COUNT];
	u16			fan_min[CY8C3245R1_FAN_COUNT];
	u16			fan_tgt;
	u16			fan_profile[CY8C3245R1_FAN_PROFILE_MAX];
	u8			fan_alarm;
	u8			temp_alarm;
	u8			force_pwm_max;
	u8			pwm;
	u8			pwm_automatic;
	struct task_struct	*auto_update;
	struct completion	auto_update_stop;
	unsigned int		auto_update_interval;
};

/*
 * 16-bit registers on the CY8C3245R1 are high-byte first.
 */
static inline int cy8c3245r1_read_word_data(struct i2c_client *client, u8 reg)
{
	s32 rc;
	u16 val;

	/* read high byte */
	rc = i2c_smbus_read_byte_data(client, reg);
	if (rc < 0) {
		dev_warn(&client->dev, "i2c read failed: 0x%02x, errno %d\n",
			 reg, -rc);
		return rc;
	}
	val = ((u16)rc & 0xFF) << 8;

	/* read low byte */
	rc = i2c_smbus_read_byte_data(client, reg + 1);
	if (rc < 0) {
		dev_warn(&client->dev, "i2c read failed: 0x%02x, errno %d\n",
			 reg + 1, -rc);
		return rc;
	}
	val |= (u16)rc & 0xFF;

	return val;
}

static inline int cy8c3245r1_write_word_data(struct i2c_client *client,
					     u8 reg,
					     u16 value)
{
	s32 rc;

	/* write high byte */
	rc = i2c_smbus_write_byte_data(client, reg, value >> 8);
	if (rc < 0) {
		dev_warn(&client->dev,
			 "i2c write failed: 0x%02x: 0x%02x, errno %d\n",
			 reg, value >> 8, -rc);
		return rc;
	}

	/* write low byte */
	rc = i2c_smbus_write_byte_data(client, reg + 1, value & 0xFF);
	if (rc < 0) {
		dev_warn(&client->dev,
			 "i2c write failed: 0x%02x: 0x%02x, errno %d\n",
			 reg + 1, value & 0xFF, -rc);
		return rc;
	}

	return rc;
}

/* Probe for temperature sensors.  Assumes lock is held */
static int cy8c3245r1_read_temperatures(struct i2c_client *client,
					struct cy8c3245r1_data *data)
{
	int i;

	/* Only count fans if we have to */
	if (data->num_temp_sensors >= 0)
		return 0;

	for (i = 0; i < CY8C3245R1_TEMP_COUNT; i++) {
		data->temp[i] = i2c_smbus_read_byte_data(client,
						CY8C3245R1_TEMP_REG(i));
		if (data->temp[i])
			data->num_temp_sensors = i + 1;
	}
	data->temperatures_probed = 1;
	return 0;
}

static int cy8c3245r1_update_thread(void *p)
{
	struct i2c_client *client = p;
	struct cy8c3245r1_data *data = i2c_get_clientdata(client);

	while (!kthread_should_stop()) {
		mutex_lock(&data->lock);
		cy8c3245r1_read_temperatures(client, data);
		mutex_unlock(&data->lock);
		if (kthread_should_stop())
			break;
		msleep_interruptible(data->auto_update_interval);
	}

	complete_all(&data->auto_update_stop);
	return 0;
}

static struct cy8c3245r1_data *cy8c3245r1_update_device(struct device *dev)
{
	struct cy8c3245r1_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	unsigned long local_jiffies = jiffies;
	int i;
	int need_sensors = 1;
	int need_limits = 1;

	/*
	 * Figure out if we need to update the shadow registers.
	 * Lockless means that we may occasionally report out of
	 * date data.
	 */
	if (time_before(local_jiffies, data->sensors_last_updated +
			SENSOR_REFRESH_INTERVAL) &&
	    data->sensors_valid)
		need_sensors = 0;

	if (time_before(local_jiffies, data->limits_last_updated +
			LIMIT_REFRESH_INTERVAL) &&
	    data->limits_valid)
		need_limits = 0;

	if (!need_sensors && !need_limits)
		return data;

	mutex_lock(&data->lock);
	if (!need_sensors)
		goto no_sensor_update;

	if (!data->temperatures_probed)
		cy8c3245r1_read_temperatures(client, data);
	else
		for (i = 0; i < CY8C3245R1_TEMP_COUNT; i++)
			data->temp[i] = i2c_smbus_read_byte_data(client,
						CY8C3245R1_TEMP_REG(i));

	for (i = 0; i < CY8C3245R1_FAN_COUNT; i++) {
		data->fan[i] = cy8c3245r1_read_word_data(client,
						      CY8C3245R1_REG_FAN(i));
	}

	data->pwm = i2c_smbus_read_byte_data(client,
					CY8C3245R1_REG_PWM);

	data->sensors_last_updated = local_jiffies;
	data->sensors_valid = 1;

no_sensor_update:
	if (!need_limits)
		goto out;

	for (i = 0; i < CY8C3245R1_TEMP_COUNT; i++) {
		data->temp_max[i] = i2c_smbus_read_byte_data(client,
						CY8C3245R1_TEMP_MAX_REG(i));
	}

	for (i = 0; i < CY8C3245R1_FAN_COUNT; i++) {
		data->fan_max[i] = cy8c3245r1_read_word_data(client,
						CY8C3245R1_REG_FAN_MAX(i));
	}
	data->fan_tgt = cy8c3245r1_read_word_data(client,
					CY8C3245R1_REG_FAN_TARGET);

	for (i = 0; i < CY8C3245R1_FAN_PROFILE_MAX; i++) {
		data->fan_profile[i] = cy8c3245r1_read_word_data(client,
					  CY8C3245R1_REG_FAN_PROFILE(i));
	}

	data->limits_last_updated = local_jiffies;
	data->limits_valid = 1;

out:
	mutex_unlock(&data->lock);
	return data;
}

static ssize_t show_auto_update_interval(struct device *dev,
					 struct device_attribute *devattr,
					 char *buf)
{
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	return sprintf(buf, "%d\n", data->auto_update_interval);
}

static ssize_t set_auto_update_interval(struct device *dev,
					struct device_attribute *devattr,
					const char *buf,
					size_t count)
{
	struct cy8c3245r1_data *data = dev_get_drvdata(dev);
	long temp;

	if (kstrtol(buf, 10, &temp))
		return -EINVAL;

	temp = clamp_val(temp, 0, 60000);

	mutex_lock(&data->lock);
	data->auto_update_interval = temp;
	mutex_unlock(&data->lock);

	return count;
}

static ssize_t show_num_temp_sensors(struct device *dev,
				     struct device_attribute *devattr,
				     char *buf)
{
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	return sprintf(buf, "%d\n", data->num_temp_sensors);
}

static ssize_t set_num_temp_sensors(struct device *dev,
				    struct device_attribute *devattr,
				    const char *buf,
				    size_t count)
{
	struct cy8c3245r1_data *data = dev_get_drvdata(dev);
	long temp;

	if (kstrtol(buf, 10, &temp))
		return -EINVAL;

	temp = clamp_val(temp, -1, 10);

	mutex_lock(&data->lock);
	data->num_temp_sensors = temp;
	if (temp < 0)
		data->temperatures_probed = 0;
	mutex_unlock(&data->lock);

	return count;
}

static ssize_t show_temp_max(struct device *dev,
			     struct device_attribute *devattr,
			     char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	return sprintf(buf, "%d\n", 1000 * data->temp_max[attr->index]);
}

static ssize_t set_temp_max(struct device *dev,
			    struct device_attribute *devattr,
			    const char *buf,
			    size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	long temp;

	if (kstrtol(buf, 10, &temp))
		return -EINVAL;

	temp = DIV_ROUND_CLOSEST(temp, 1000);
	temp = clamp_val(temp, -128, 127);

	mutex_lock(&data->lock);
	data->temp_max[attr->index] = temp;
	i2c_smbus_write_byte_data(client, CY8C3245R1_TEMP_MAX_REG(attr->index),
				  temp);
	mutex_unlock(&data->lock);

	return count;
}

static ssize_t show_temp(struct device *dev, struct device_attribute *devattr,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	return sprintf(buf, "%d\n", 1000 * data->temp[attr->index]);
}

static ssize_t show_fan_max(struct device *dev,
			    struct device_attribute *devattr,
			    char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	if (FAN_DATA_VALID(data->fan_max[attr->index]))
		return sprintf(buf, "%d\n",
			       data->fan_max[attr->index]);
	else
		return sprintf(buf, "0\n");
}

static ssize_t set_fan_max(struct device *dev,
			   struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	long rpm;

	if (kstrtol(buf, 10, &rpm) || !rpm)
		return -EINVAL;

	rpm = clamp_val(rpm, 1, 65534);

	mutex_lock(&data->lock);
	data->fan_max[attr->index] = rpm;
	cy8c3245r1_write_word_data(client,
				   CY8C3245R1_REG_FAN_MAX(attr->index), rpm);
	mutex_unlock(&data->lock);

	return count;
}

/*
 * fan_min is a pure software concept, not implemented by hardware.
 * It is used to compute the alarm status.
 */
static ssize_t show_fan_min(struct device *dev,
			    struct device_attribute *devattr,
			    char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	return sprintf(buf, "%d\n", data->fan_min[attr->index]);
}

static ssize_t set_fan_min(struct device *dev,
			   struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = dev_get_drvdata(dev);
	long rpm;

	if (kstrtol(buf, 10, &rpm) || !rpm)
		return -EINVAL;

	rpm = clamp_val(rpm, 1, 65534);

	mutex_lock(&data->lock);
	data->fan_min[attr->index] = rpm;
	mutex_unlock(&data->lock);

	return count;
}

static ssize_t show_fan_target(struct device *dev,
			       struct device_attribute *devattr,
			       char *buf)
{
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	if (FAN_DATA_VALID(data->fan_tgt))
		return sprintf(buf, "%d\n",
			       data->fan_tgt);
	else
		return sprintf(buf, "0\n");
}

static ssize_t set_fan_target(struct device *dev,
			      struct device_attribute *devattr,
			      const char *buf, size_t count)
{
	struct cy8c3245r1_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	long rpm;

	if (kstrtol(buf, 10, &rpm) || !rpm)
		return -EINVAL;

	rpm = clamp_val(rpm, 1, 65534);

	mutex_lock(&data->lock);
	data->fan_tgt = rpm;
	cy8c3245r1_write_word_data(client, CY8C3245R1_REG_FAN_TARGET, rpm);
	mutex_unlock(&data->lock);

	return count;
}

/*
 * Show Fan Profile Settings
 */
static ssize_t show_fan_profile(struct device *dev,
				struct device_attribute *devattr,
				char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	return sprintf(buf, "%u\n",
		       data->fan_profile[attr->index]);
}

/*
 * Set Fan Profile Settings
 */
static ssize_t set_fan_profile(struct device *dev,
			       struct device_attribute *devattr,
			       const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	long parm;

	if (kstrtoul(buf, 10, &parm))
		return -EINVAL;

	parm = clamp_val(parm, 1, 65534);

	mutex_lock(&data->lock);
	data->fan_profile[attr->index] = parm;
	cy8c3245r1_write_word_data(client,
				   CY8C3245R1_REG_FAN_PROFILE(attr->index),
				   parm);
	mutex_unlock(&data->lock);

	return count;
}

static ssize_t show_fan(struct device *dev, struct device_attribute *devattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	if (FAN_DATA_VALID(data->fan[attr->index]))
		return sprintf(buf, "%d\n", data->fan[attr->index]);
	else
		return sprintf(buf, "0\n");
}

static ssize_t show_pwm(struct device *dev, struct device_attribute *devattr,
			char *buf)
{
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	return sprintf(buf, "%d\n", data->pwm);
}

static ssize_t set_pwm(struct device *dev, struct device_attribute *devattr,
		       const char *buf, size_t count)
{
	struct cy8c3245r1_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	long temp;

	if (kstrtol(buf, 10, &temp))
		return -EINVAL;

	temp = clamp_val(temp, 0, 255);

	mutex_lock(&data->lock);
	data->pwm = temp;
	i2c_smbus_write_byte_data(client, CY8C3245R1_REG_PWM, temp);
	mutex_unlock(&data->lock);

	return count;
}

static ssize_t show_fan_alarm_mask(struct device *dev,
				   struct device_attribute *devattr,
				   char *buf)
{
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);
	int i;
	u32 alarm_mask = 0;
	u16 min = 0;
	u16 max = 0;

	for (i = 0; i < CY8C3245R1_FAN_COUNT; i++) {
		/*
		 * 15% Variance need to be applied for
		 * generating alarms
		 */
		min = (uint16_t)((85 * data->fan_min[i]) / 100);
		max = (uint16_t)((115 * data->fan_max[i]) / 100);

		if ((data->fan[i] < min) ||
		    (data->fan[i] >= max))
			alarm_mask |= 0x1 << i;
	}

	return sprintf(buf, "%x\n", alarm_mask);
}

static ssize_t show_temp_alarm_mask(struct device *dev,
				    struct device_attribute *devattr,
				    char *buf)
{
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);
	int i;
	u32 alarm_mask = 0;

	for (i = 0; i < CY8C3245R1_TEMP_COUNT; i++)
		if (data->temp[i] >= data->temp_max[i])
			alarm_mask |= 0x1 << i;

	return sprintf(buf, "%x\n", alarm_mask);
}

static ssize_t show_fan_alarm(struct device *dev,
			      struct device_attribute *devattr,
			      char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);
	u16 min = 0;
	u16 max = 0;

	/*
	 * 15% Variance need to be applied for
	 * generating alarms
	 */
	min = (uint16_t)((85 * data->fan_min[attr->index]) / 100);
	max = (uint16_t)((115 * data->fan_max[attr->index]) / 100);

	if ((data->fan[attr->index] < min) ||
	    (data->fan[attr->index] >= max))
		return sprintf(buf, "1\n");
	else
		return sprintf(buf, "0\n");
}

static ssize_t show_temp_alarm(struct device *dev,
			       struct device_attribute *devattr,
			       char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	if (data->temp[attr->index] >= data->temp_max[attr->index])
		return sprintf(buf, "1\n");
	else
		return sprintf(buf, "0\n");
}

static ssize_t set_pwm_auto(struct device *dev,
			    struct device_attribute *devattr,
			    const char *buf,
			    size_t count)
{
	long temp;

	if (kstrtol(buf, 10, &temp))
		return -EINVAL;

	if (!(temp >= 0 && temp <= 3))
		return -EINVAL;

	return count;
}

static ssize_t show_pwm_auto(struct device *dev,
			     struct device_attribute *devattr,
			     char *buf)
{
	struct cy8c3245r1_data *data = cy8c3245r1_update_device(dev);

	return sprintf(buf, "%d\n", data->pwm_automatic);
}

#define CY8C3245R1_REG_MIN 0x00
#define CY8C3245R1_REG_MAX 0xe0

static ssize_t show_debug(struct device *dev,
			  struct device_attribute *devattr,
			  char *buf)
{
	struct cy8c3245r1_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int len = 0, i, j;
	u8  val;
	int reg_count = (CY8C3245R1_REG_MAX - CY8C3245R1_REG_MIN) / 16;

	for (i = 0; i < reg_count; i++) {
		len += sprintf(buf + len, "0x%02x: ",
			       CY8C3245R1_REG_MIN + (i * 16));
		for (j = 0; j < 16; j++) {
			val = i2c_smbus_read_byte_data(client,
						       CY8C3245R1_REG_MIN
						       + (i * 16) + j);
			len += sprintf(buf + len, "%02x ", val);
		}
		len += sprintf(buf + len, "\n");
	}
	return len;
}

static DEVICE_ATTR(fan_alarm_mask, S_IRUGO, show_fan_alarm_mask, NULL);
static DEVICE_ATTR(temp_alarm_mask, S_IRUGO, show_temp_alarm_mask, NULL);
static DEVICE_ATTR(num_temp_sensors, S_IWUSR | S_IRUGO, show_num_temp_sensors,
		   set_num_temp_sensors);
static DEVICE_ATTR(auto_update_interval, S_IWUSR | S_IRUGO,
		   show_auto_update_interval, set_auto_update_interval);

static SENSOR_DEVICE_ATTR(temp1_max, S_IWUSR | S_IRUGO, show_temp_max,
		    set_temp_max, 0);
static SENSOR_DEVICE_ATTR(temp2_max, S_IWUSR | S_IRUGO, show_temp_max,
		    set_temp_max, 1);
static SENSOR_DEVICE_ATTR(temp3_max, S_IWUSR | S_IRUGO, show_temp_max,
		    set_temp_max, 2);
static SENSOR_DEVICE_ATTR(temp4_max, S_IWUSR | S_IRUGO, show_temp_max,
		    set_temp_max, 3);
static SENSOR_DEVICE_ATTR(temp5_max, S_IWUSR | S_IRUGO, show_temp_max,
		    set_temp_max, 4);
static SENSOR_DEVICE_ATTR(temp6_max, S_IWUSR | S_IRUGO, show_temp_max,
		    set_temp_max, 5);
static SENSOR_DEVICE_ATTR(temp7_max, S_IWUSR | S_IRUGO, show_temp_max,
		    set_temp_max, 6);
static SENSOR_DEVICE_ATTR(temp8_max, S_IWUSR | S_IRUGO, show_temp_max,
		    set_temp_max, 7);

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, show_temp, NULL, 0);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO, show_temp, NULL, 1);
static SENSOR_DEVICE_ATTR(temp3_input, S_IRUGO, show_temp, NULL, 2);
static SENSOR_DEVICE_ATTR(temp4_input, S_IRUGO, show_temp, NULL, 3);
static SENSOR_DEVICE_ATTR(temp5_input, S_IRUGO, show_temp, NULL, 4);
static SENSOR_DEVICE_ATTR(temp6_input, S_IRUGO, show_temp, NULL, 5);
static SENSOR_DEVICE_ATTR(temp7_input, S_IRUGO, show_temp, NULL, 6);
static SENSOR_DEVICE_ATTR(temp8_input, S_IRUGO, show_temp, NULL, 7);

static SENSOR_DEVICE_ATTR(temp1_alarm, S_IRUGO, show_temp_alarm, NULL, 0);
static SENSOR_DEVICE_ATTR(temp2_alarm, S_IRUGO, show_temp_alarm, NULL, 1);
static SENSOR_DEVICE_ATTR(temp3_alarm, S_IRUGO, show_temp_alarm, NULL, 2);
static SENSOR_DEVICE_ATTR(temp4_alarm, S_IRUGO, show_temp_alarm, NULL, 3);
static SENSOR_DEVICE_ATTR(temp5_alarm, S_IRUGO, show_temp_alarm, NULL, 4);
static SENSOR_DEVICE_ATTR(temp6_alarm, S_IRUGO, show_temp_alarm, NULL, 4);
static SENSOR_DEVICE_ATTR(temp7_alarm, S_IRUGO, show_temp_alarm, NULL, 4);
static SENSOR_DEVICE_ATTR(temp8_alarm, S_IRUGO, show_temp_alarm, NULL, 4);

static SENSOR_DEVICE_ATTR(fan1_max, S_IWUSR | S_IRUGO, show_fan_max,
		    set_fan_max, 0);
static SENSOR_DEVICE_ATTR(fan2_max, S_IWUSR | S_IRUGO, show_fan_max,
		    set_fan_max, 1);
static SENSOR_DEVICE_ATTR(fan3_max, S_IWUSR | S_IRUGO, show_fan_max,
		    set_fan_max, 2);
static SENSOR_DEVICE_ATTR(fan4_max, S_IWUSR | S_IRUGO, show_fan_max,
		    set_fan_max, 3);
static SENSOR_DEVICE_ATTR(fan5_max, S_IWUSR | S_IRUGO, show_fan_max,
		    set_fan_max, 4);
static SENSOR_DEVICE_ATTR(fan6_max, S_IWUSR | S_IRUGO, show_fan_max,
		    set_fan_max, 5);
static SENSOR_DEVICE_ATTR(fan7_max, S_IWUSR | S_IRUGO, show_fan_max,
		    set_fan_max, 6);
static SENSOR_DEVICE_ATTR(fan8_max, S_IWUSR | S_IRUGO, show_fan_max,
		    set_fan_max, 7);

static SENSOR_DEVICE_ATTR(fan1_min, S_IWUSR | S_IRUGO, show_fan_min,
		    set_fan_min, 0);
static SENSOR_DEVICE_ATTR(fan2_min, S_IWUSR | S_IRUGO, show_fan_min,
		    set_fan_min, 1);
static SENSOR_DEVICE_ATTR(fan3_min, S_IWUSR | S_IRUGO, show_fan_min,
		    set_fan_min, 2);
static SENSOR_DEVICE_ATTR(fan4_min, S_IWUSR | S_IRUGO, show_fan_min,
		    set_fan_min, 3);
static SENSOR_DEVICE_ATTR(fan5_min, S_IWUSR | S_IRUGO, show_fan_min,
		    set_fan_min, 4);
static SENSOR_DEVICE_ATTR(fan6_min, S_IWUSR | S_IRUGO, show_fan_min,
		    set_fan_min, 5);
static SENSOR_DEVICE_ATTR(fan7_min, S_IWUSR | S_IRUGO, show_fan_min,
		    set_fan_min, 6);
static SENSOR_DEVICE_ATTR(fan8_min, S_IWUSR | S_IRUGO, show_fan_min,
		    set_fan_min, 7);

static SENSOR_DEVICE_ATTR(fan1_target, S_IWUSR | S_IRUGO, show_fan_target,
		    set_fan_target, 0);
static SENSOR_DEVICE_ATTR(fan2_target, S_IWUSR | S_IRUGO, show_fan_target,
		    set_fan_target, 1);
static SENSOR_DEVICE_ATTR(fan3_target, S_IWUSR | S_IRUGO, show_fan_target,
		    set_fan_target, 2);
static SENSOR_DEVICE_ATTR(fan4_target, S_IWUSR | S_IRUGO, show_fan_target,
		    set_fan_target, 3);
static SENSOR_DEVICE_ATTR(fan5_target, S_IWUSR | S_IRUGO, show_fan_target,
		    set_fan_target, 4);
static SENSOR_DEVICE_ATTR(fan6_target, S_IWUSR | S_IRUGO, show_fan_target,
		    set_fan_target, 5);
static SENSOR_DEVICE_ATTR(fan7_target, S_IWUSR | S_IRUGO, show_fan_target,
		    set_fan_target, 6);
static SENSOR_DEVICE_ATTR(fan8_target, S_IWUSR | S_IRUGO, show_fan_target,
		    set_fan_target, 7);

#define FAN_PROFILE_ATTR(_name, _index)					\
	SENSOR_DEVICE_ATTR(_name, S_IWUSR | S_IRUGO,		\
				  show_fan_profile, set_fan_profile, _index)

static FAN_PROFILE_ATTR(fan_low_duty,      CY8C3245R1_FAN_PROFILE_LOW_DUTY);
static FAN_PROFILE_ATTR(fan_low_rpm,       CY8C3245R1_FAN_PROFILE_LOW_RPM);
static FAN_PROFILE_ATTR(fan_high_duty,     CY8C3245R1_FAN_PROFILE_HIGH_DUTY);
static FAN_PROFILE_ATTR(fan_high_rpm,      CY8C3245R1_FAN_PROFILE_HIGH_RPM);
static FAN_PROFILE_ATTR(fan_speed_0_duty,  CY8C3245R1_FAN_PROFILE_SPEED_0_DUTY);
static FAN_PROFILE_ATTR(fan_speed_100_duty,
					CY8C3245R1_FAN_PROFILE_SPEED_100_DUTY);

static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, show_fan, NULL, 0);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO, show_fan, NULL, 1);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO, show_fan, NULL, 2);
static SENSOR_DEVICE_ATTR(fan4_input, S_IRUGO, show_fan, NULL, 3);
static SENSOR_DEVICE_ATTR(fan5_input, S_IRUGO, show_fan, NULL, 4);
static SENSOR_DEVICE_ATTR(fan6_input, S_IRUGO, show_fan, NULL, 5);
static SENSOR_DEVICE_ATTR(fan7_input, S_IRUGO, show_fan, NULL, 6);
static SENSOR_DEVICE_ATTR(fan8_input, S_IRUGO, show_fan, NULL, 7);

static SENSOR_DEVICE_ATTR(fan1_alarm, S_IRUGO, show_fan_alarm, NULL, 0);
static SENSOR_DEVICE_ATTR(fan2_alarm, S_IRUGO, show_fan_alarm, NULL, 1);
static SENSOR_DEVICE_ATTR(fan3_alarm, S_IRUGO, show_fan_alarm, NULL, 2);
static SENSOR_DEVICE_ATTR(fan4_alarm, S_IRUGO, show_fan_alarm, NULL, 3);
static SENSOR_DEVICE_ATTR(fan5_alarm, S_IRUGO, show_fan_alarm, NULL, 4);
static SENSOR_DEVICE_ATTR(fan6_alarm, S_IRUGO, show_fan_alarm, NULL, 5);
static SENSOR_DEVICE_ATTR(fan7_alarm, S_IRUGO, show_fan_alarm, NULL, 6);
static SENSOR_DEVICE_ATTR(fan8_alarm, S_IRUGO, show_fan_alarm, NULL, 7);

static SENSOR_DEVICE_ATTR(pwm1, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 0);
static SENSOR_DEVICE_ATTR(pwm2, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 1);
static SENSOR_DEVICE_ATTR(pwm3, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 2);
static SENSOR_DEVICE_ATTR(pwm4, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 3);
static SENSOR_DEVICE_ATTR(pwm5, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 4);
static SENSOR_DEVICE_ATTR(pwm6, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 5);
static SENSOR_DEVICE_ATTR(pwm7, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 6);
static SENSOR_DEVICE_ATTR(pwm8, S_IWUSR | S_IRUGO, show_pwm, set_pwm, 7);

static SENSOR_DEVICE_ATTR(pwm1_enable, S_IWUSR | S_IRUGO, show_pwm_auto,
		    set_pwm_auto, 0);
static SENSOR_DEVICE_ATTR(pwm2_enable, S_IWUSR | S_IRUGO, show_pwm_auto,
		    set_pwm_auto, 0);
static SENSOR_DEVICE_ATTR(pwm3_enable, S_IWUSR | S_IRUGO, show_pwm_auto,
		    set_pwm_auto, 0);
static SENSOR_DEVICE_ATTR(pwm4_enable, S_IWUSR | S_IRUGO, show_pwm_auto,
		    set_pwm_auto, 0);
static SENSOR_DEVICE_ATTR(pwm5_enable, S_IWUSR | S_IRUGO, show_pwm_auto,
		    set_pwm_auto, 0);
static SENSOR_DEVICE_ATTR(pwm6_enable, S_IWUSR | S_IRUGO, show_pwm_auto,
		    set_pwm_auto, 0);
static SENSOR_DEVICE_ATTR(pwm7_enable, S_IWUSR | S_IRUGO, show_pwm_auto,
		    set_pwm_auto, 0);
static SENSOR_DEVICE_ATTR(pwm8_enable, S_IWUSR | S_IRUGO, show_pwm_auto,
		    set_pwm_auto, 0);

static SENSOR_DEVICE_ATTR(debug, S_IRUGO, show_debug, NULL, 0);

static struct attribute *cy8c3245r1_attrs[] = {
	&dev_attr_fan_alarm_mask.attr,
	&dev_attr_temp_alarm_mask.attr,
	&dev_attr_num_temp_sensors.attr,
	&dev_attr_auto_update_interval.attr,
	&sensor_dev_attr_temp1_max.dev_attr.attr,
	&sensor_dev_attr_temp2_max.dev_attr.attr,
	&sensor_dev_attr_temp3_max.dev_attr.attr,
	&sensor_dev_attr_temp4_max.dev_attr.attr,
	&sensor_dev_attr_temp5_max.dev_attr.attr,
	&sensor_dev_attr_temp6_max.dev_attr.attr,
	&sensor_dev_attr_temp7_max.dev_attr.attr,
	&sensor_dev_attr_temp8_max.dev_attr.attr,
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp4_input.dev_attr.attr,
	&sensor_dev_attr_temp5_input.dev_attr.attr,
	&sensor_dev_attr_temp6_input.dev_attr.attr,
	&sensor_dev_attr_temp7_input.dev_attr.attr,
	&sensor_dev_attr_temp8_input.dev_attr.attr,
	&sensor_dev_attr_temp1_alarm.dev_attr.attr,
	&sensor_dev_attr_temp2_alarm.dev_attr.attr,
	&sensor_dev_attr_temp3_alarm.dev_attr.attr,
	&sensor_dev_attr_temp4_alarm.dev_attr.attr,
	&sensor_dev_attr_temp5_alarm.dev_attr.attr,
	&sensor_dev_attr_temp6_alarm.dev_attr.attr,
	&sensor_dev_attr_temp7_alarm.dev_attr.attr,
	&sensor_dev_attr_temp8_alarm.dev_attr.attr,
	&sensor_dev_attr_fan1_max.dev_attr.attr,
	&sensor_dev_attr_fan2_max.dev_attr.attr,
	&sensor_dev_attr_fan3_max.dev_attr.attr,
	&sensor_dev_attr_fan4_max.dev_attr.attr,
	&sensor_dev_attr_fan5_max.dev_attr.attr,
	&sensor_dev_attr_fan6_max.dev_attr.attr,
	&sensor_dev_attr_fan7_max.dev_attr.attr,
	&sensor_dev_attr_fan8_max.dev_attr.attr,
	&sensor_dev_attr_fan1_min.dev_attr.attr,
	&sensor_dev_attr_fan2_min.dev_attr.attr,
	&sensor_dev_attr_fan3_min.dev_attr.attr,
	&sensor_dev_attr_fan4_min.dev_attr.attr,
	&sensor_dev_attr_fan5_min.dev_attr.attr,
	&sensor_dev_attr_fan6_min.dev_attr.attr,
	&sensor_dev_attr_fan7_min.dev_attr.attr,
	&sensor_dev_attr_fan8_min.dev_attr.attr,
	&sensor_dev_attr_fan1_target.dev_attr.attr,
	&sensor_dev_attr_fan2_target.dev_attr.attr,
	&sensor_dev_attr_fan3_target.dev_attr.attr,
	&sensor_dev_attr_fan4_target.dev_attr.attr,
	&sensor_dev_attr_fan5_target.dev_attr.attr,
	&sensor_dev_attr_fan6_target.dev_attr.attr,
	&sensor_dev_attr_fan7_target.dev_attr.attr,
	&sensor_dev_attr_fan8_target.dev_attr.attr,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,
	&sensor_dev_attr_fan1_alarm.dev_attr.attr,
	&sensor_dev_attr_fan2_alarm.dev_attr.attr,
	&sensor_dev_attr_fan3_alarm.dev_attr.attr,
	&sensor_dev_attr_fan4_alarm.dev_attr.attr,
	&sensor_dev_attr_fan5_alarm.dev_attr.attr,
	&sensor_dev_attr_fan6_alarm.dev_attr.attr,
	&sensor_dev_attr_fan7_alarm.dev_attr.attr,
	&sensor_dev_attr_fan8_alarm.dev_attr.attr,
	&sensor_dev_attr_pwm1.dev_attr.attr,
	&sensor_dev_attr_pwm2.dev_attr.attr,
	&sensor_dev_attr_pwm3.dev_attr.attr,
	&sensor_dev_attr_pwm4.dev_attr.attr,
	&sensor_dev_attr_pwm5.dev_attr.attr,
	&sensor_dev_attr_pwm6.dev_attr.attr,
	&sensor_dev_attr_pwm7.dev_attr.attr,
	&sensor_dev_attr_pwm8.dev_attr.attr,
	&sensor_dev_attr_pwm1_enable.dev_attr.attr,
	&sensor_dev_attr_pwm2_enable.dev_attr.attr,
	&sensor_dev_attr_pwm3_enable.dev_attr.attr,
	&sensor_dev_attr_pwm4_enable.dev_attr.attr,
	&sensor_dev_attr_pwm5_enable.dev_attr.attr,
	&sensor_dev_attr_pwm6_enable.dev_attr.attr,
	&sensor_dev_attr_pwm7_enable.dev_attr.attr,
	&sensor_dev_attr_pwm8_enable.dev_attr.attr,
	&sensor_dev_attr_fan_low_duty.dev_attr.attr,
	&sensor_dev_attr_fan_low_rpm.dev_attr.attr,
	&sensor_dev_attr_fan_high_duty.dev_attr.attr,
	&sensor_dev_attr_fan_high_rpm.dev_attr.attr,
	&sensor_dev_attr_fan_speed_0_duty.dev_attr.attr,
	&sensor_dev_attr_fan_speed_100_duty.dev_attr.attr,
	&sensor_dev_attr_debug.dev_attr.attr,
	NULL
};

ATTRIBUTE_GROUPS(cy8c3245r1);

static void cy8c3245r1_init_client(struct i2c_client *client)
{
	int reg;

	reg = i2c_smbus_read_byte_data(client,
				       CY8C3245R1_REG_PWM_CFG_BASE_ADDR);

	if (reg < 0)
		dev_err(&client->dev, "cannot read configuration register\n");
	else
		i2c_smbus_write_byte_data(client,
					  CY8C3245R1_REG_PWM_CFG_BASE_ADDR, 0);
}

static int cy8c3245r1_probe(struct i2c_client *client,
			    const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct cy8c3245r1_data *data;
	struct device *hwmon_dev;

	data = devm_kzalloc(dev, sizeof(struct cy8c3245r1_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->num_temp_sensors = -1;
	data->auto_update_interval = AUTO_UPDATE_INTERVAL;

	i2c_set_clientdata(client, data);
	data->client = client;
	mutex_init(&data->lock);

	dev_info(&client->dev, "%s chip found\n", client->name);

	/* Initialize the CY8C3245R1 chip */
	cy8c3245r1_init_client(client);

	/* Register sysfs hooks */
	hwmon_dev = devm_hwmon_device_register_with_groups(dev, client->name,
							   data,
							   cy8c3245r1_groups);

	if (IS_ERR(hwmon_dev))
		return PTR_ERR(hwmon_dev);

	init_completion(&data->auto_update_stop);
	data->auto_update = kthread_run(cy8c3245r1_update_thread, client, "%s",
					dev_name(hwmon_dev));
	if (IS_ERR(data->auto_update))
		return PTR_ERR(data->auto_update);

	return 0;
}

static int cy8c3245r1_remove(struct i2c_client *client)
{
	struct cy8c3245r1_data *data = i2c_get_clientdata(client);

	kthread_stop(data->auto_update);
	wait_for_completion(&data->auto_update_stop);
	return 0;
}

static const struct i2c_device_id cy8c3245r1_id[] = {
	{ "CY8C3245R1", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, cy8c3245r1_id);

static struct i2c_driver cy8c3245r1_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "cy8c3245r1",
	},
	.probe		= cy8c3245r1_probe,
	.remove		= cy8c3245r1_remove,
	.id_table	= cy8c3245r1_id,
};

module_i2c_driver(cy8c3245r1_driver);

MODULE_AUTHOR("Shrijeet Mukherjee <shm@cumulusnetworks.com>");
MODULE_AUTHOR("Vidya Ravipati <vidya@cumulusnetworks.com>");
MODULE_DESCRIPTION("CY8C3245R1 driver");
MODULE_LICENSE("GPL");
