/*
 * Smartfusion PSU module FRU driver for dell_s6100 platforms.
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

#include <linux/module.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "dell-s6100-platform.h"
#include "dell-s6100-smf.h"
#include "dell-s6100-smf-psu.h"

#define DRIVER_VERSION "1.0"

#define MAX_PSU_FRU_NAME_LEN  (10)

/**
 * struct s6100_smf_psu_drv_priv -- private driver data
 * @pdev:            Parent platform device
 * @smf_mb_map:      smf mailbox regmap
 * @name:            human readable name
 * @psu_attr_group:  attribute group for psu objects
 * @psu_attr_groups: array of attribute groups for psu objects
 *
 * Structure containing private data for the driver.
 */
struct s6100_smf_psu_drv_priv {
	struct platform_device         *pdev;
	struct regmap                  *smf_mb_map;
	char                            name[MAX_PSU_FRU_NAME_LEN];
};

/**
 * s6100_smf_psu_ids -- driver alias names
 */
static const struct platform_device_id s6100_smf_psu_ids[] = {
	{ S6100_SMF_PSU_DRIVER_NAME, 0 },
	{ /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(platform, s6100_smf_psu_ids);

enum s6100_smf_psu_label {
	PSU_LABEL_TEMP1 = 0,
	PSU_LABEL_FAN1,
	PSU_LABEL_IN1,
	PSU_LABEL_IN2,
	PSU_LABEL_CURR1,
	PSU_LABEL_CURR2,
	PSU_LABEL_POWER1,
	PSU_LABEL_POWER2,
	PSU_LABEL_MAX,
};

static char *s6100_smf_psu_labels[PSU_LABEL_MAX] = {
	[PSU_LABEL_TEMP1]  = "Temperature",
	[PSU_LABEL_FAN1]   = "Fan",
	[PSU_LABEL_IN1]	   = "Input Voltage",
	[PSU_LABEL_IN2]	   = "Output Voltage",
	[PSU_LABEL_CURR1]  = "Input Current",
	[PSU_LABEL_CURR2]  = "Output Current",
	[PSU_LABEL_POWER1] = "Input Power",
	[PSU_LABEL_POWER2] = "Output Power",
};

/**
 * smf_mb_psu_present_show
 *
 * PSU presence
 */
static ssize_t smf_mb_psu_present_show(struct device *dev,
				       struct device_attribute *dattr,
				       char *buf)
{
	struct s6100_smf_psu_drv_priv *priv = dev_get_drvdata(dev);
	int reg = S6100_SMF_MB_PSU1_STATUS;
	uint8_t psu_status;
	int rc;

	reg += S6100_SMF_MB_PSU_OFFSET * priv->pdev->id;
	rc = smf_mb_reg_rd(priv->smf_mb_map, reg, &psu_status);
	if (rc)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%d\n",
			(psu_status & PSU_STATUS_PRESENT_L) ? 0 : 1);
}

/**
 * smf_mb_psu_input_ok_show
 *
 * PSU input power is OK, i.e. the power cord is plugged in.
 */
static ssize_t smf_mb_psu_input_ok_show(struct device *dev,
					struct device_attribute *dattr,
					char *buf)
{
	struct s6100_smf_psu_drv_priv *priv = dev_get_drvdata(dev);
	int reg = S6100_SMF_MB_PSU1_STATUS;
	uint8_t psu_val;
	int input_bad = 0;
	int rc;

	/* Read input voltage */
	reg = S6100_SMF_MB_PSU1_INPUT_VOLT;
	reg += S6100_SMF_MB_PSU_OFFSET * priv->pdev->id;
	rc = smf_mb_reg_rd(priv->smf_mb_map, reg, &psu_val);
	if (rc)
		input_bad++;
	if (psu_val == 0x0)
		input_bad++;

	/* Read input power */
	reg = S6100_SMF_MB_PSU1_INPUT_POWER;
	reg += S6100_SMF_MB_PSU_OFFSET * priv->pdev->id;
	rc = smf_mb_reg_rd(priv->smf_mb_map, reg, &psu_val);
	if (rc)
		input_bad++;
	if (psu_val == 0x0)
		input_bad++;

	return snprintf(buf, PAGE_SIZE, "%d\n", (input_bad == 0) ? 1 : 0);
}

/**
 * smf_mb_psu_f2b_show
 *
 * PSU fan airflow, f2b or b2f
 *
 */
static ssize_t smf_mb_psu_f2b_show(struct device *dev,
				   struct device_attribute *dattr,
				   char *buf)
{
	struct s6100_smf_psu_drv_priv *priv = dev_get_drvdata(dev);
	int reg = S6100_SMF_MB_PSU1_FAN_STATUS;
	uint8_t psu_fan_status;
	int rc;

	reg += S6100_SMF_MB_PSU_OFFSET * priv->pdev->id;
	rc = smf_mb_reg_rd(priv->smf_mb_map, reg, &psu_fan_status);
	if (rc)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%d\n",
			(psu_fan_status & PSU_FAN_STATUS_F2B) ? 1 : 0);
}

/**
 * smf_mb_psu_string_show
 *
 * retrieve PSU string data
 *
 * The base register offset is encoded in
 * sensor_device_attribute->index.
 */
static ssize_t smf_mb_psu_string_show(struct device *dev,
				      struct device_attribute *dattr,
				      char *buf)
{
	struct s6100_smf_psu_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int reg = attr->index;
	uint16_t len = 0;
	uint8_t str[S6100_SMF_MB_MAX_STRING_SIZE];
	int rc;

	switch (reg) {
	case S6100_SMF_MB_PSU1_MFG_DATE:
		len = S6100_SMF_MB_PSU_MFG_DATE_SIZE;
		break;
	case S6100_SMF_MB_PSU1_COUNTY_CODE:
		len = S6100_SMF_MB_PSU_COUNTY_CODE_SIZE;
		break;
	case S6100_SMF_MB_PSU1_PART_NUM:
		len = S6100_SMF_MB_PSU_PART_NUM_SIZE;
		break;
	case S6100_SMF_MB_PSU1_MFG_ID:
		len = S6100_SMF_MB_PSU_MFG_ID_SIZE;
		break;
	case S6100_SMF_MB_PSU1_SERIAL_NUM:
		len = S6100_SMF_MB_PSU_SERIAL_NUM_SIZE;
		break;
	case S6100_SMF_MB_PSU1_SERVICE_TAG:
		len = S6100_SMF_MB_PSU_SERVICE_TAG_SIZE;
		break;
	case S6100_SMF_MB_PSU1_LABEL_REV:
		len = S6100_SMF_MB_PSU_LABEL_REV_SIZE;
		break;
	default:
		WARN(1, "unexpected PSU string base register: 0x%x\n", reg);
		return -EINVAL;
	};

	reg += S6100_SMF_MB_PSU_OFFSET * priv->pdev->id;
	rc = smf_mb_reg_rd_array(priv->smf_mb_map, reg, str, len);
	if (rc)
		return rc;

	str[len] = '\0';

	return snprintf(buf, PAGE_SIZE, "%s\n", str);
}

static SENSOR_ATTR_DATA_RO(present,	smf_mb_psu_present_show,  0);
static SENSOR_ATTR_DATA_RO(input_ok,	smf_mb_psu_input_ok_show, 0);
static SENSOR_ATTR_DATA_RO(front2back,	smf_mb_psu_f2b_show,	  0);
static SENSOR_ATTR_DATA_RO(mfg_date,	smf_mb_psu_string_show,   S6100_SMF_MB_PSU1_MFG_DATE);
static SENSOR_ATTR_DATA_RO(country,	smf_mb_psu_string_show,   S6100_SMF_MB_PSU1_COUNTY_CODE);
static SENSOR_ATTR_DATA_RO(part_num,	smf_mb_psu_string_show,   S6100_SMF_MB_PSU1_PART_NUM);
static SENSOR_ATTR_DATA_RO(mfg_id,	smf_mb_psu_string_show,   S6100_SMF_MB_PSU1_MFG_ID);
static SENSOR_ATTR_DATA_RO(serial_num,	smf_mb_psu_string_show,   S6100_SMF_MB_PSU1_SERIAL_NUM);
static SENSOR_ATTR_DATA_RO(service_tag, smf_mb_psu_string_show,   S6100_SMF_MB_PSU1_SERVICE_TAG);
static SENSOR_ATTR_DATA_RO(label_rev,   smf_mb_psu_string_show,   S6100_SMF_MB_PSU1_LABEL_REV);

/**
 * psu_fru_attrs
 *
 * Array of PSU FRU attributes
 */
static struct attribute *psu_fru_attrs[] = {
	&sensor_dev_attr_present.dev_attr.attr,
	&sensor_dev_attr_input_ok.dev_attr.attr,
	&sensor_dev_attr_front2back.dev_attr.attr,
	&sensor_dev_attr_mfg_date.dev_attr.attr,
	&sensor_dev_attr_country.dev_attr.attr,
	&sensor_dev_attr_part_num.dev_attr.attr,
	&sensor_dev_attr_mfg_id.dev_attr.attr,
	&sensor_dev_attr_serial_num.dev_attr.attr,
	&sensor_dev_attr_service_tag.dev_attr.attr,
	&sensor_dev_attr_label_rev.dev_attr.attr,
	NULL,
};

ATTRIBUTE_GROUPS(psu_fru);

/**
 * smf_mb_psu_temp_show
 *
 * retrieve PSU temp
 */
static ssize_t smf_mb_psu_temp_show(struct device *dev,
				    struct device_attribute *dattr,
				    char *buf)
{
	struct s6100_smf_psu_drv_priv *priv = dev_get_drvdata(dev);
	int reg = S6100_SMF_MB_PSU1_TEMP;
	uint16_t temp;
	int rc;

	reg += S6100_SMF_MB_PSU_OFFSET * priv->pdev->id;
	rc = smf_mb_reg_rd16(priv->smf_mb_map, reg, &temp);
	if (rc)
		return rc;

	/*
	 * Register spec says 0xffff means the temperature has not
	 * been read temporarily.
	 */
	if (temp == S6100_SMF_MB_BAD_READ)
		return -EAGAIN;

	/*
	 * The temperature register is in units of centidegree
	 * Celsius.  The hwmon interface is in millidegree Celsius.
	 */
	return snprintf(buf, PAGE_SIZE, "%d\n", (int)temp * 100);
}

/**
 * smf_mb_psu_label_show
 *
 * return the PSU hwmon device label
 */
static ssize_t smf_mb_psu_label_show(struct device *dev,
				     struct device_attribute *dattr,
				     char *buf)
{
	struct s6100_smf_psu_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);

	if (attr->index >= PSU_LABEL_MAX) {
		WARN(1, "unexpected PSU label index: %d\n", attr->index);
		return -EINVAL;
	}

	return snprintf(buf, PAGE_SIZE, "PSU %d %s\n",
			priv->pdev->id + 1, s6100_smf_psu_labels[attr->index]);
}

/**
 * smf_mb_psu_fan_speed_show
 *
 * retrieve PSU fan speed
 */
static ssize_t smf_mb_psu_fan_speed_show(struct device *dev,
					 struct device_attribute *dattr,
					 char *buf)
{
	struct s6100_smf_psu_drv_priv *priv = dev_get_drvdata(dev);
	int reg = S6100_SMF_MB_PSU1_FAN_SPEED;
	uint16_t rpm;
	int rc;

	reg += S6100_SMF_MB_PSU_OFFSET * priv->pdev->id;
	rc = smf_mb_reg_rd16(priv->smf_mb_map, reg, &rpm);
	if (rc)
		return rc;

	if (rpm == S6100_SMF_MB_BAD_READ)
		return -EAGAIN;

	return snprintf(buf, PAGE_SIZE, "%d\n", rpm);
}

/**
 * smf_mb_psu_fan_ok_show
 *
 * retrieve PSU fan status
 */
static ssize_t smf_mb_psu_fan_ok_show(struct device *dev,
				      struct device_attribute *dattr,
				      char *buf)
{
	struct s6100_smf_psu_drv_priv *priv = dev_get_drvdata(dev);
	int reg = S6100_SMF_MB_PSU1_FAN_STATUS;
	uint8_t status;
	int rc;

	reg += S6100_SMF_MB_PSU_OFFSET * priv->pdev->id;
	rc = smf_mb_reg_rd(priv->smf_mb_map, reg, &status);
	if (rc)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%d\n", (status & PSU_FAN_STATUS_OK_L) ? 0 : 1);
}

/**
 * smf_mb_psu_voltage_show
 *
 * return the PSU hwmon voltages
 */
static ssize_t smf_mb_psu_voltage_show(struct device *dev,
				       struct device_attribute *dattr,
				       char *buf)
{
	struct s6100_smf_psu_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int reg = attr->index;
	uint16_t voltage;
	int rc;

	reg += S6100_SMF_MB_PSU_OFFSET * priv->pdev->id;
	rc = smf_mb_reg_rd16(priv->smf_mb_map, reg, &voltage);
	if (rc)
		return rc;

	if (voltage == S6100_SMF_MB_BAD_READ)
		return -EAGAIN;

	/*
	 * The voltage register is in units of 10 millivolts.  The
	 * hwmon interace is in millivolts.
	 */

	return snprintf(buf, PAGE_SIZE, "%d\n", (int)voltage * 10);
}

/**
 * smf_mb_psu_current_show
 *
 * return the PSU hwmon currents
 */
static ssize_t smf_mb_psu_current_show(struct device *dev,
				       struct device_attribute *dattr,
				       char *buf)
{
	struct s6100_smf_psu_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int reg = attr->index;
	uint16_t amp;
	int rc;

	reg += S6100_SMF_MB_PSU_OFFSET * priv->pdev->id;
	rc = smf_mb_reg_rd16(priv->smf_mb_map, reg, &amp);
	if (rc)
		return rc;

	if (amp == S6100_SMF_MB_BAD_READ)
		return -EAGAIN;

	/*
	 * The voltage register is in units of 10 milliamp.  The
	 * hwmon interace is also in milliamp.
	 */

	return snprintf(buf, PAGE_SIZE, "%d\n", (int)amp);
}

/**
 * smf_mb_psu_power_show
 *
 * return the PSU hwmon powers
 */
static ssize_t smf_mb_psu_power_show(struct device *dev,
				     struct device_attribute *dattr,
				     char *buf)
{
	struct s6100_smf_psu_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int reg = attr->index;
	uint16_t power;
	int rc;

	reg += S6100_SMF_MB_PSU_OFFSET * priv->pdev->id;
	rc = smf_mb_reg_rd16(priv->smf_mb_map, reg, &power);
	if (rc)
		return rc;

	if (power == S6100_SMF_MB_BAD_READ)
		return -EAGAIN;

	/*
	 * The power register is in units of centiWatts.  The
	 * hwmon interace is microWatts.
	 */

	return snprintf(buf, PAGE_SIZE, "%d\n", (int)power * 100 * 1000);
}

static SENSOR_ATTR_DATA_RO(temp1_input,  smf_mb_psu_temp_show,      0);
static SENSOR_ATTR_DATA_RO(temp1_label,  smf_mb_psu_label_show,     PSU_LABEL_TEMP1);
static SENSOR_ATTR_DATA_RO(fan1_input,   smf_mb_psu_fan_speed_show, 0);
static SENSOR_ATTR_DATA_RO(fan1_all_ok,  smf_mb_psu_fan_ok_show,    0);
static SENSOR_ATTR_DATA_RO(fan1_label,   smf_mb_psu_label_show,     PSU_LABEL_FAN1);
static SENSOR_ATTR_DATA_RO(in1_input,    smf_mb_psu_voltage_show,   S6100_SMF_MB_PSU1_INPUT_VOLT);
static SENSOR_ATTR_DATA_RO(in1_label,    smf_mb_psu_label_show,     PSU_LABEL_IN1);
static SENSOR_ATTR_DATA_RO(in2_input,    smf_mb_psu_voltage_show,   S6100_SMF_MB_PSU1_OUTPUT_VOLT);
static SENSOR_ATTR_DATA_RO(in2_label,    smf_mb_psu_label_show,     PSU_LABEL_IN2);
static SENSOR_ATTR_DATA_RO(curr1_input,  smf_mb_psu_current_show,   S6100_SMF_MB_PSU1_INPUT_CURRENT);
static SENSOR_ATTR_DATA_RO(curr1_label,  smf_mb_psu_label_show,     PSU_LABEL_CURR1);
static SENSOR_ATTR_DATA_RO(curr2_input,  smf_mb_psu_current_show,   S6100_SMF_MB_PSU1_OUTPUT_CURRENT);
static SENSOR_ATTR_DATA_RO(curr2_label,  smf_mb_psu_label_show,     PSU_LABEL_CURR2);
static SENSOR_ATTR_DATA_RO(power1_input, smf_mb_psu_power_show,     S6100_SMF_MB_PSU1_INPUT_POWER);
static SENSOR_ATTR_DATA_RO(power1_label, smf_mb_psu_label_show,     PSU_LABEL_POWER1);
static SENSOR_ATTR_DATA_RO(power2_input, smf_mb_psu_power_show,     S6100_SMF_MB_PSU1_OUTPUT_POWER);
static SENSOR_ATTR_DATA_RO(power2_label, smf_mb_psu_label_show,     PSU_LABEL_POWER2);

/**
 * psu_hwmon_attrs
 *
 * Array of PSU hwmon attributes
 */
static struct attribute *psu_hwmon_attrs[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp1_label.dev_attr.attr,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan1_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan1_label.dev_attr.attr,
	&sensor_dev_attr_in1_input.dev_attr.attr,
	&sensor_dev_attr_in1_label.dev_attr.attr,
	&sensor_dev_attr_in2_input.dev_attr.attr,
	&sensor_dev_attr_in2_label.dev_attr.attr,
	&sensor_dev_attr_curr1_input.dev_attr.attr,
	&sensor_dev_attr_curr1_label.dev_attr.attr,
	&sensor_dev_attr_curr2_input.dev_attr.attr,
	&sensor_dev_attr_curr2_label.dev_attr.attr,
	&sensor_dev_attr_power1_input.dev_attr.attr,
	&sensor_dev_attr_power1_label.dev_attr.attr,
	&sensor_dev_attr_power2_input.dev_attr.attr,
	&sensor_dev_attr_power2_label.dev_attr.attr,
	NULL,
};

ATTRIBUTE_GROUPS(psu_hwmon);

/**
 * s6100_smf_psu_probe() -- psu tray driver probe
 */
static int s6100_smf_psu_probe(struct platform_device *pdev)
{
	int rc = 0;
	struct s6100_smf_psu_platform_data *pdata;
	struct device *hwmon_dev;
	struct s6100_smf_psu_drv_priv *priv;

	pdata = dev_get_platdata(&pdev->dev);
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data\n");
		return -EINVAL;
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->pdev = pdev;
	priv->smf_mb_map = pdata->smf_mb_map;
	platform_set_drvdata(pdev, (void *)priv);

	/* Human readable name */
	snprintf(priv->name, sizeof(priv->name), "psu%d", pdev->id + 1);

	/* Create per PSU FRU sysfs nodes */
	rc = sysfs_create_groups(&pdev->dev.kobj, psu_fru_groups);
	if (rc)
		return rc;

	hwmon_dev = devm_hwmon_device_register_with_groups(&pdev->dev,
							   priv->name,
							   (void *)priv,
							   psu_hwmon_groups);
	if (IS_ERR(hwmon_dev)) {
		rc = PTR_ERR(hwmon_dev);
		dev_err(&pdev->dev,
			"unable to create smf_psu hwmon device: %d\n", rc);
		goto error_psu_fru_attr;
	}

	dev_info(&pdev->dev, "device probed ok\n");
	return rc;

error_psu_fru_attr:
	sysfs_remove_groups(&pdev->dev.kobj, psu_fru_groups);
	return rc;
}

/**
 * s6100_smf_psu_remove() -- PSU FRU driver remove
 */
static int s6100_smf_psu_remove(struct platform_device *pdev)
{
	sysfs_remove_groups(&pdev->dev.kobj, psu_fru_groups);

	dev_info(&pdev->dev, "device removed\n");
	return 0;
}

static struct platform_driver s6100_smf_psu_driver = {
	.driver = {
		.name  = S6100_SMF_PSU_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = s6100_smf_psu_probe,
	.remove = s6100_smf_psu_remove,
	.id_table = s6100_smf_psu_ids,
};

module_platform_driver(s6100_smf_psu_driver);

MODULE_AUTHOR("Curt Brune <curt@cumulusnetworks.com");
MODULE_DESCRIPTION("Smartfusion PSU FRU Driver for DELL S6100");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
