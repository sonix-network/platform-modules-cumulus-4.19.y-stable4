/*
 * Smartfusion fan tray FRU driver for dell_s6100 platforms.
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
#include "dell-s6100-smf-fan.h"

#define DRIVER_VERSION "1.0"

#define MAX_FAN_TRAY_FRU_NAME_LEN  (10)

/**
 * struct s6100_smf_fan_drv_priv -- private driver data
 * @pdev:            Parent platform device
 * @smf_mb_map:      smf mailbox regmap
 * @num_fans:        number of fans on this tray
 * @name:            human readable name
 * @fan_attr_group:  attribute group for fan objects
 * @fan_attr_groups: array of attribute groups for fan objects
 *
 * Structure containing private data for the driver.
 */
struct s6100_smf_fan_drv_priv {
	struct platform_device         *pdev;
	struct regmap                  *smf_mb_map;
	uint8_t                         num_fans;
	char                            name[MAX_FAN_TRAY_FRU_NAME_LEN];
	struct attribute_group          fan_attr_group;
	const struct attribute_group   *fan_attr_groups[2];
};

/**
 * s6100_smf_fan_ids -- driver alias names
 */
static const struct platform_device_id s6100_smf_fan_ids[] = {
	{ S6100_SMF_FAN_DRIVER_NAME, 0 },
	{ /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(platform, s6100_smf_fan_ids);

/**
 * smf_mb_fan_speed_show -- retrieve fan speed
 *
 * This attribute is per fan on a fan tray FRU.
 */
static ssize_t smf_mb_fan_speed_show(struct device *dev,
				     struct device_attribute *dattr,
				     char *buf)
{
	struct s6100_smf_fan_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	uint8_t fan_num = attr->index;
	uint16_t fan_speed;
	int rc;

	rc = smf_mb_reg_rd16(priv->smf_mb_map,
			     S6100_SMF_MB_FAN_TRAY_SPEED_BASE +
			     S6100_SMF_MB_FAN_TRAY_SPEED_OFFSET * priv->pdev->id +
			     2 * fan_num, &fan_speed);
	if (rc)
		return rc;

	if (fan_speed == S6100_SMF_MB_BAD_READ)
		return -EAGAIN;

	return snprintf(buf, PAGE_SIZE, "%d\n", fan_speed);
}

/**
 * smf_mb_fan_ok_show -- retrieve fan status
 *
 * This attribute is per fan on a fan tray FRU.
 */
static ssize_t smf_mb_fan_ok_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	struct s6100_smf_fan_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	uint8_t fan_num = attr->index;
	uint16_t fan_status;
	uint16_t fan_mask = BIT(priv->pdev->id * priv->num_fans + fan_num);
	int rc;

	rc = smf_mb_reg_rd16(priv->smf_mb_map,
			     S6100_SMF_MB_FAN_STATUS, &fan_status);
	if (rc)
		return rc;

	if (fan_status == S6100_SMF_MB_BAD_READ)
		return -EAGAIN;

	return snprintf(buf, PAGE_SIZE, "%d\n",
			(fan_status & fan_mask) ? 0 : 1);
}

/**
 * smf_mb_fan_label_show -- display a fan label
 *
 * This attribute is per fan on a fan tray.
 */
static ssize_t smf_mb_fan_label_show(struct device *dev,
				      struct device_attribute *dattr,
				      char *buf)
{
	struct s6100_smf_fan_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	uint8_t fan_num = attr->index;

	return snprintf(buf, PAGE_SIZE, "Fan Tray %d, Fan %d\n",
			priv->pdev->id + 1, fan_num + 1);
}

/**
 * smf_mb_fan_present_show -- retrieve fan presence
 *
 * This attribute is per fan tray.
 */
static ssize_t smf_mb_fan_present_show(struct device *dev,
				       struct device_attribute *dattr,
				       char *buf)
{
	struct s6100_smf_fan_drv_priv *priv = dev_get_drvdata(dev);
	uint8_t fan_present;
	uint8_t fan_mask = BIT(priv->pdev->id);
	int rc;

	rc = smf_mb_reg_rd(priv->smf_mb_map,
			   S6100_SMF_MB_FAN_TRAYS_PRESENT, &fan_present);
	if (rc)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%d\n",
			(fan_present & fan_mask) ? 0 : 1);
}

/**
 * smf_mb_fan_f2b_show -- retrieve fan airflow, f2b or b2f
 *
 * This attribute is per fan tray.
 */
static ssize_t smf_mb_fan_f2b_show(struct device *dev,
				   struct device_attribute *dattr,
				   char *buf)
{
	struct s6100_smf_fan_drv_priv *priv = dev_get_drvdata(dev);
	uint8_t fan_f2b;
	uint8_t fan_mask = BIT(priv->pdev->id);
	int rc;

	rc = smf_mb_reg_rd(priv->smf_mb_map, S6100_SMF_MB_FAN_TRAYS_F2B, &fan_f2b);
	if (rc)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%d\n", (fan_f2b & fan_mask) ? 1 : 0);
}

/**
 * smf_mb_fan_mfg_date_show -- decode the manufacturing date register
 *
 * This attribute is per fan tray.
 */
static ssize_t smf_mb_fan_mfg_date_show(struct device *dev,
					struct device_attribute *dattr,
					char *buf)
{
	struct s6100_smf_fan_drv_priv *priv = dev_get_drvdata(dev);
	uint8_t mfg_date[S6100_SMF_MB_FAN_TRAY_MFG_DATE_SIZE];
	uint16_t reg = S6100_SMF_MB_FAN_TRAY1_MFG_DATE;
	int rc;

	reg += priv->pdev->id * S6100_SMF_MB_FAN_TRAY_VPD_OFFSET;
	rc = smf_mb_reg_rd_array(priv->smf_mb_map, reg, mfg_date,
				 S6100_SMF_MB_FAN_TRAY_MFG_DATE_SIZE);
	if (rc)
		return rc;

	return snprintf(buf, PAGE_SIZE, "20%02u-%02u-%02uT%02u:%02u:%02u+0000\n",
			mfg_date[0], mfg_date[1], mfg_date[2],
			mfg_date[3], mfg_date[4], mfg_date[5]);
}

/**
 * smf_mb_fan_string_show -- retrieve fan string data
 *
 * This attribute is per fan tray.
 *
 * The base register offset is encoded in
 * sensor_device_attribute->index.
 */
static ssize_t smf_mb_fan_string_show(struct device *dev,
				      struct device_attribute *dattr,
				      char *buf)
{
	struct s6100_smf_fan_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	uint16_t base_reg = attr->index;
	uint16_t len = 0;
	uint8_t str[S6100_SMF_MB_MAX_STRING_SIZE];
	int rc;

	switch (base_reg) {
	case S6100_SMF_MB_FAN_TRAY1_SERIAL_NUM:
		len = S6100_SMF_MB_FAN_TRAY_SERIAL_NUM_SIZE;
		break;
	case S6100_SMF_MB_FAN_TRAY1_PART_NUM:
		len = S6100_SMF_MB_FAN_TRAY_PART_NUM_SIZE;
		break;
	case S6100_SMF_MB_FAN_TRAY1_LABEL_REV:
		len = S6100_SMF_MB_FAN_TRAY_LABEL_REV_SIZE;
		break;
	case S6100_SMF_MB_FAN_TRAY1_MFG_DATE:
		len = S6100_SMF_MB_FAN_TRAY_MFG_DATE_SIZE;
		break;
	default:
		WARN(1,
		     "unexpected fan string base register: 0x%x\n", base_reg);
		return -EINVAL;
	};

	base_reg += priv->pdev->id * S6100_SMF_MB_FAN_TRAY_VPD_OFFSET;
	rc = smf_mb_reg_rd_array(priv->smf_mb_map, base_reg, str, len);
	if (rc)
		return rc;

	str[len] = '\0';

	return snprintf(buf, PAGE_SIZE, "%s\n", str);
}

static SENSOR_ATTR_DATA_RO(present,    smf_mb_fan_present_show,	 0);
static SENSOR_ATTR_DATA_RO(front2back, smf_mb_fan_f2b_show,	 0);
static SENSOR_ATTR_DATA_RO(mfg_date,   smf_mb_fan_mfg_date_show, 0);
static SENSOR_ATTR_DATA_RO(serial_num, smf_mb_fan_string_show,	 S6100_SMF_MB_FAN_TRAY1_SERIAL_NUM);
static SENSOR_ATTR_DATA_RO(part_num,   smf_mb_fan_string_show,	 S6100_SMF_MB_FAN_TRAY1_PART_NUM);
static SENSOR_ATTR_DATA_RO(label_rev,  smf_mb_fan_string_show,	 S6100_SMF_MB_FAN_TRAY1_LABEL_REV);

/**
 * tray_fru_attr_data
 *
 * Array of fan tray attributes
 */
static struct attribute *tray_fru_attrs[] = {
	&sensor_dev_attr_present.dev_attr.attr,
	&sensor_dev_attr_front2back.dev_attr.attr,
	&sensor_dev_attr_mfg_date.dev_attr.attr,
	&sensor_dev_attr_serial_num.dev_attr.attr,
	&sensor_dev_attr_part_num.dev_attr.attr,
	&sensor_dev_attr_label_rev.dev_attr.attr,
	NULL,
};
ATTRIBUTE_GROUPS(tray_fru);

/**
 * fan_attr_data - Array of fan hwmon attributes
 *
 * These attributes are applied to each fan on a fan tray.
 *
 * This array is used as a template for creatng the actual fan
 * attributes at run time.
 *
 * At run time, the .name field is used as a printf-style format
 * string to generate the actual attribute name using the current fan
 * index.
 *
 * Also the .index field is filled in at run time.
 */
static struct sensor_device_attribute fan_attr_data[] = {
	ATTR_DATA_RO(fan%d_input,  smf_mb_fan_speed_show, 0),
	ATTR_DATA_RO(fan%d_all_ok, smf_mb_fan_ok_show,    0),
	ATTR_DATA_RO(fan%d_label,  smf_mb_fan_label_show, 0),
};

/**
 * make_fan_hwmon_attrs()
 *
 * @pdev:      platform device object
 * @attrs:     array of attributes pointers [out]
 * @pattr:     array of struct sensor_device_attribute entries [in]
 * @num_attrs: length of @pattr [in]
 * @inst:      fan instance
 *
 * Given an array of struct sensor_device_attribute entries in @pattr,
 * generate an array of attributes.  The array is returned in
 * @attrs.
 *
 * The @pattr array is used as a template for creatng the actual fan
 * attributes at run time, parameterized by @inst.
 *
 * Returns 0 on success or other negative errno.
 */
static int
make_fan_hwmon_attrs(struct platform_device *pdev,
		     struct attribute **attrs,
		     struct sensor_device_attribute *pattr,
		     int num_attrs,
		     int inst)
{
	int i;
	struct attribute *attr;
	struct sensor_device_attribute attr_data;
	char *name;

	for (i = 0; i < num_attrs; i++) {

		name = devm_kasprintf(&pdev->dev, GFP_KERNEL,
				      pattr->dev_attr.attr.name, inst + 1);
		if (!name)
			return -ENOMEM;
		attr_data.dev_attr.attr.name = name;
		attr_data.dev_attr.attr.mode = pattr->dev_attr.attr.mode;
		attr_data.dev_attr.show = pattr->dev_attr.show;
		attr_data.dev_attr.store = pattr->dev_attr.store;
		attr_data.index = inst;
		attr = gen_device_attr(pdev, &attr_data);
		if (IS_ERR(attr))
			return PTR_ERR(attr);
		*attrs++ = attr;
		pattr++;
	}
	return 0;
}

/**
 * s6100_smf_fan_probe() -- fan tray driver probe
 */
static int s6100_smf_fan_probe(struct platform_device *pdev)
{
	int rc = 0;
	struct s6100_smf_fan_platform_data *pdata;
	int i, n_attr = 0;
	struct device *hwmon_dev;
	struct s6100_smf_fan_drv_priv *priv;
	struct attribute **attrs;

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
	snprintf(priv->name, sizeof(priv->name), "fan_tray%d", pdev->id + 1);

	/* Create per fan tray sysfs nodes */
	rc = sysfs_create_groups(&pdev->dev.kobj, tray_fru_groups);
	if (rc)
		return rc;

	/* Create per fan hwmon sysfs nodes */
	rc = smf_mb_reg_rd(priv->smf_mb_map,
			   S6100_SMF_MB_FANS_PER_TRAY, &priv->num_fans);
	if (rc)
		goto error_tray_fru_attr;

	/*
	 * Number of per fan hwmon attributes, plus one for the NULL
	 * entry of the attr array.
	 */
	n_attr = priv->num_fans * ARRAY_SIZE(fan_attr_data) + 1;
	attrs = devm_kzalloc(&pdev->dev,
			     n_attr * sizeof(*attrs), GFP_KERNEL);
	if (!attrs) {
		rc = -ENOMEM;
		goto error_tray_fru_attr;
	}
	priv->fan_attr_group.attrs = attrs;
	priv->fan_attr_groups[0] = &priv->fan_attr_group;
	priv->fan_attr_groups[1] = NULL;

	for (i = 0; i < priv->num_fans; i++) {
		rc = make_fan_hwmon_attrs(pdev, attrs, fan_attr_data,
					  ARRAY_SIZE(fan_attr_data), i);
		if (rc)
			goto error_tray_fru_attr;
		attrs += ARRAY_SIZE(fan_attr_data);
	}

	hwmon_dev = devm_hwmon_device_register_with_groups(&pdev->dev,
							   priv->name,
							   (void *)priv,
							   priv->fan_attr_groups);

	if (IS_ERR(hwmon_dev)) {
		rc = PTR_ERR(hwmon_dev);
		dev_err(&pdev->dev,
			"unable to create smf_fan hwmon device: %d\n", rc);
		goto error_tray_fru_attr;
	}

	dev_info(&pdev->dev, "device probed ok\n");
	return rc;

error_tray_fru_attr:
	sysfs_remove_groups(&pdev->dev.kobj, tray_fru_groups);
	return rc;
}

/**
 * s6100_smf_fan_remove() -- fan tray driver remove
 */
static int s6100_smf_fan_remove(struct platform_device *pdev)
{
	sysfs_remove_groups(&pdev->dev.kobj, tray_fru_groups);

	dev_info(&pdev->dev, "device removed\n");
	return 0;
}

static struct platform_driver s6100_smf_fan_driver = {
	.driver = {
		.name  = S6100_SMF_FAN_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = s6100_smf_fan_probe,
	.remove = s6100_smf_fan_remove,
	.id_table = s6100_smf_fan_ids,
};

module_platform_driver(s6100_smf_fan_driver);

MODULE_AUTHOR("Curt Brune <curt@cumulusnetworks.com");
MODULE_DESCRIPTION("Smartfusion Fan Driver for DELL S6100");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
