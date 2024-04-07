/*
 * Switch Board CPLD driver for dell_s6100 platforms.
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
#include <linux/hwmon-sysfs.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "dell-s6100-platform.h"
#include "dell-s6100-cpld.h"

#define DRIVER_VERSION "1.0"

/**
 * s6100_cpld_ids -- driver alias names
 */
static const struct platform_device_id s6100_cpld_ids[] = {
	{ "dell-s6100-cpld", 0 },
	{ /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(platform, s6100_cpld_ids);

#define MAX_CPLD_DEV_NAME_LEN  (10)

/**
 * struct s6100_cpld_priv -- private driver data
 * @pdev:      Parent platform device
 * @name:      Device name
 * @cpld_map:  CPLD LPC register map
 * @gpio_ctrl: GPIO controller object
 * @gpio_lock: spinlock used to serialize read/modify/write GPIO operations
 *
 * Structure containing private data for the s6100_cpld driver.
 */
struct s6100_cpld_drv_priv {
	struct platform_device        *pdev;
	char                           name[MAX_CPLD_DEV_NAME_LEN];
	struct regmap                 *cpld_map;
	struct gpio_chip               gpio_ctrl;
	spinlock_t                     gpio_lock;
};

#define to_cpld_gpio(gc) container_of(gc, struct s6100_cpld_drv_priv, gpio_ctrl)

/**
 * cpld_ro_ranges
 *
 * Array of regmap read-only ranges for the CPLD LPC registers.
 */
static const struct regmap_range cpld_ro_ranges[] = {
	regmap_reg_range(S6100_CPLD_VERSION,                  S6100_CPLD_BOARD_TYPE),
	regmap_reg_range(S6100_CPLD_ID,                       S6100_CPLD_BOARD_REV),
	regmap_reg_range(S6100_CPLD_MISC_INTR_STATUS_SUMMARY, S6100_CPLD_MISC_INTR),
	regmap_reg_range(S6100_CPLD_IO_MODULE_INTR_STATUS,    S6100_CPLD_IO_MODULE_PRESENT),
	regmap_reg_range(S6100_CPLD_SFPP_RXLOS_STATUS,        S6100_CPLD_SFPP_PRESENT_STATUS),
	regmap_reg_range(S6100_CPLD_SFPP_INTR_STATUS_SUMMARY, S6100_CPLD_SFPP_PRESENT_INTR),
};

/**
 * cpld_precious_ranges
 *
 * Array of regmap precious ranges for the CPLD LPC registers.  These
 * registers are read-clear.
 */
static const struct regmap_range cpld_precious_ranges[] = {
	regmap_reg_range(S6100_CPLD_MISC_INTR,       S6100_CPLD_MISC_INTR),
	regmap_reg_range(S6100_CPLD_SFPP_RXLOS_INTR, S6100_CPLD_SFPP_PRESENT_INTR),
};

/**
 * cpld_volatile_ranges
 *
 * Array of regmap volatile ranges for the CPLD LPC registers.
 */
static const struct regmap_range cpld_volatile_ranges[] = {
	regmap_reg_range(S6100_CPLD_MISC_INTR_STATUS_SUMMARY, S6100_CPLD_MISC_INTR),
	regmap_reg_range(S6100_CPLD_IO_MODULE_INTR_STATUS,    S6100_CPLD_IO_MODULE_PRESENT),
	regmap_reg_range(S6100_CPLD_SFPP_RXLOS_STATUS,        S6100_CPLD_SFPP_PRESENT_STATUS),
	regmap_reg_range(S6100_CPLD_SFPP_INTR_STATUS_SUMMARY, S6100_CPLD_SFPP_PRESENT_INTR),
};

static struct regmap_access_table cpld_write_table = {
	.no_ranges   = cpld_ro_ranges,
	.n_no_ranges = ARRAY_SIZE(cpld_ro_ranges),
};

static struct regmap_access_table cpld_precious_table = {
	.yes_ranges   = cpld_precious_ranges,
	.n_yes_ranges = ARRAY_SIZE(cpld_precious_ranges),
};

static struct regmap_access_table cpld_volatile_table = {
	.yes_ranges   = cpld_volatile_ranges,
	.n_yes_ranges = ARRAY_SIZE(cpld_volatile_ranges),
};

/**
 * cpld_regmap_reg_read()
 * @context: IO-space base memory address
 * @reg: register offset
 * @val: pointer to hold register read value
 *
 * regmap read register callback for the IO-space LPC registers
 */
static int
cpld_regmap_reg_read(void *context, unsigned int reg, unsigned int *val)
{
	void __iomem *iobase = context;

	*val = ioread8(iobase + reg);
	return 0;
}

/**
 * cpld_regmap_reg_write()
 * @context: IO-space base memory address
 * @reg: register offset
 * @val: value to write to register
 *
 * regmap write register callback for the IO-space LPC registers
 */
static int
cpld_regmap_reg_write(void *context, unsigned int reg, unsigned int val)
{
	void __iomem *iobase = context;

	iowrite8(val & 0xFF, iobase + reg);
	return 0;
}

/**
 * s6100_cpld_regmap_config
 *
 * regmap configuration for the CPLD LPC registers.
 */
static struct regmap_config s6100_cpld_regmap_config = {
	.name            = "s6100-cpld",
	.reg_bits	 = 8,
	.reg_stride	 = 1,
	.pad_bits	 = 0,
	.val_bits	 = 8,
	.fast_io	 = true,
	.max_register	 = S6100_CPLD_SFPP_PRESENT_INTR_MASK,
	.wr_table	 = &cpld_write_table,
	.precious_table	 = &cpld_precious_table,
	.volatile_table	 = &cpld_volatile_table,
	.use_single_rw	 = true,
	.can_multi_write = false,
	.reg_read        = cpld_regmap_reg_read,
	.reg_write       = cpld_regmap_reg_write,
};

/**
 * cpld_reg_rd(): Read cpld lpc register
 *
 * @cpld_map: regmap configured for the cpld lpc registers
 * @reg: register offset
 * @val: pointer to hold 8-bit register read value
 *
 * Read an 8-bit cpld lpc value from register @reg and return the
 * value in @val.
 *
 * A value of zero is returned on success, a negative errno is
 * returned in error cases.
 */
static inline int
cpld_reg_rd(struct regmap *cpld_map, unsigned int reg, uint8_t *val)
{
	int rc;
	unsigned int val32 = 0;

	rc = regmap_read(cpld_map, reg, &val32);
	WARN(rc, "regmap_read(cpld_map, 0x%x,...) failed: (%d)\n", reg, rc);

	*val = val32 & 0xFF;
	return rc;
}

/**
 * cpld_reg_wr(): Write cpld lpc register
 *
 * @cpld_map: regmap configured for the cpld lpc registers
 * @reg: register offset
 * @val: 8-bit value to write
 *
 * Writes @val to the 8-bit cpld lpc register specified by @reg.
 *
 * A value of zero is returned on success, a negative errno is
 * returned in error cases.
 */
static inline int
cpld_reg_wr(struct regmap *cpld_map, unsigned int reg, uint8_t val)
{
	int rc;

	rc = regmap_write(cpld_map, reg, val);
	WARN(rc, "regmap_write(cpld_map, 0x%x, 0x%x) failed: (%d)\n",
	     reg, val, rc);

	return rc;
}

/**
 * fw_version_show()
 *
 * Return the CPLD firmware version.
 */
static ssize_t fw_version_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	struct s6100_cpld_drv_priv *priv = dev_get_drvdata(dev);
	uint8_t val;
	int rc;

	rc = cpld_reg_rd(priv->cpld_map, S6100_CPLD_VERSION, &val);
	if (rc)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%d.%d\n",
			(val >> 4) & 0xF, val & 0xF);
}

/**
 * cpld_u8_show()
 *
 * Return an 8-bit cpld register.  The desired decoding is specified
 * in the .nr field.
 */
static ssize_t cpld_u8_show(struct device *dev,
			    struct device_attribute *dattr,
			    char *buf)
{
	struct s6100_cpld_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	uint8_t val;
	char *fmt;
	int rc;

	rc = cpld_reg_rd(priv->cpld_map, attr->index, &val);
	if (rc)
		return rc;

	switch (attr->nr) {
	case 'd':
		fmt = "%d\n";
		break;
	case 'x':
		fmt = "0x%02x\n";
		break;
	default:
		return -EINVAL;
	};

	return snprintf(buf, PAGE_SIZE, fmt, val);
}

/**
 * gpio_map_show()
 *
 * Return an array of GPIO signal names seperated a newline character.
 * The GPIO names are ordered by the GPIO number.
 */
static ssize_t gpio_map_show(struct device *dev,
			     struct device_attribute *dattr,
			     char *buf)
{
	struct s6100_cpld_drv_priv *priv = dev_get_drvdata(dev);

	return cumulus_gpio_map_show(dev, &priv->gpio_ctrl, buf);
}

/**
 * struct attribute *misc_system_attrs[]
 *
 * Array of misc CPLD system level sysfs attributes.
 */
static SENSOR_ATTR_DATA_RO_2(cpld_version,   fw_version_show,  0, 0);
static SENSOR_ATTR_DATA_RO_2(board_revision, cpld_u8_show,     S6100_CPLD_BOARD_REV, 'd');
static SENSOR_ATTR_DATA_RO_2(gpio_map,       gpio_map_show,    0, 0);
static struct attribute *misc_system_attrs[] = {
	&sensor_dev_attr_cpld_version.dev_attr.attr,
	&sensor_dev_attr_board_revision.dev_attr.attr,
	&sensor_dev_attr_gpio_map.dev_attr.attr,
	NULL,
};
ATTRIBUTE_GROUPS(misc_system);

#define S6100_CPLD_GPIO_CTRL_BASE 1000
#define S6100_CPLD_GPIO_CHIP_NAME "dell-s6100-cpld-gpio"

#define GPIO_DATA(_name, _reg, _bit, _flags, _val) \
	{ .name = __stringify(_name), .reg = _reg, \
	  .bit = _bit, .flags = _flags, .value = _val, }

static const struct s6100_gpio_data gpio_data[] = {
	GPIO_DATA(mod1-reset,	     S6100_CPLD_IO_MODULE_RESET_CTRL,  0, GPIOF_DIR_OUT | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA(mod2-reset,	     S6100_CPLD_IO_MODULE_RESET_CTRL,  1, GPIOF_DIR_OUT | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA(mod3-reset,	     S6100_CPLD_IO_MODULE_RESET_CTRL,  2, GPIOF_DIR_OUT | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA(mod4-reset,	     S6100_CPLD_IO_MODULE_RESET_CTRL,  3, GPIOF_DIR_OUT | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA(swm5p1-tx-disable, S6100_CPLD_SFPP_TXDISABLE_CTRL,   0, GPIOF_DIR_OUT, 0),
	GPIO_DATA(swm5p2-tx-disable, S6100_CPLD_SFPP_TXDISABLE_CTRL,   1, GPIOF_DIR_OUT, 0),
	GPIO_DATA(swm5p1-rs,	     S6100_CPLD_SFPP_RATE_SELECT_CTRL, 0, GPIOF_DIR_OUT, 1),
	GPIO_DATA(swm5p2-rs,	     S6100_CPLD_SFPP_RATE_SELECT_CTRL, 1, GPIOF_DIR_OUT, 1),
	GPIO_DATA(swm5p1-rx-los,     S6100_CPLD_SFPP_RXLOS_STATUS,     0, GPIOF_DIR_IN,	0),
	GPIO_DATA(swm5p2-rx-los,     S6100_CPLD_SFPP_RXLOS_STATUS,     1, GPIOF_DIR_IN,	0),
	GPIO_DATA(swm5p1-tx-fault,   S6100_CPLD_SFPP_TXFAULT_STATUS,   0, GPIOF_DIR_IN,	0),
	GPIO_DATA(swm5p2-tx-fault,   S6100_CPLD_SFPP_TXFAULT_STATUS,   1, GPIOF_DIR_IN,	0),
	GPIO_DATA(swm5p1-mod-abs,    S6100_CPLD_SFPP_PRESENT_STATUS,   0, GPIOF_DIR_IN, 0),
	GPIO_DATA(swm5p2-mod-abs,    S6100_CPLD_SFPP_PRESENT_STATUS,   1, GPIOF_DIR_IN, 0),
};

/**
 * cpld_gpio_get_direction()
 *
 * gpio_chip driver interface callback. Returns the current GPIO
 * signal direction.
 */
static int
cpld_gpio_get_direction(struct gpio_chip *chip, unsigned offset)
{
	struct s6100_cpld_drv_priv *priv = to_cpld_gpio(chip);

	if (offset >= ARRAY_SIZE(gpio_data)) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d", offset);
		return -EINVAL;
	}

	return gpio_data[offset].flags & GPIOF_DIR_IN;
}

/**
 * cpld_gpio_direction_input()
 *
 * gpio_chip driver interface callback. Configures the specified GPIO
 * as an input.
 */
static int
cpld_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	struct s6100_cpld_drv_priv *priv = to_cpld_gpio(chip);

	if (offset >= ARRAY_SIZE(gpio_data)) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d", offset);
		return -EINVAL;
	}

	return gpio_data[offset].flags & GPIOF_DIR_IN ? 0 : -EINVAL;
}

/**
 * cpld_gpio_direction_output()
 *
 * gpio_chip driver interface callback. Configures the specified GPIO
 * as an output.
 */
static int
cpld_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
	struct s6100_cpld_drv_priv *priv = to_cpld_gpio(chip);

	if (offset >= ARRAY_SIZE(gpio_data)) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d", offset);
		return -EINVAL;
	}

	return gpio_data[offset].flags & GPIOF_DIR_IN ? -EINVAL : 0;
}

/**
 * cpld_gpio_get()
 *
 * gpio_chip driver interface callback. Returns the current value of
 * the specified GPIO signal.
 */
static int
cpld_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	int rc;
	struct s6100_cpld_drv_priv *priv = to_cpld_gpio(chip);
	uint8_t val;

	if (offset >= ARRAY_SIZE(gpio_data)) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d", offset);
		return -EINVAL;
	}

	rc = cpld_reg_rd(priv->cpld_map, gpio_data[offset].reg, &val);
	if (rc)
		return rc;

	rc = val & BIT(gpio_data[offset].bit) ? 1 : 0;
	if (gpio_data[offset].flags & GPIOF_ACTIVE_LOW)
		rc = !rc;

	return rc;
}

/**
 * cpld_gpio_set()
 *
 * gpio_chip driver interface callback. For output GPIO signals, sets
 * the current value of the specified GPIO signal.
 */
static void
cpld_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	int rc;
	struct s6100_cpld_drv_priv *priv = to_cpld_gpio(chip);
	uint8_t reg_val;
	unsigned long flags;

	if (offset >= ARRAY_SIZE(gpio_data)) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d", offset);
		return;
	}

	if (gpio_data[offset].flags & GPIOF_DIR_IN) {
		dev_warn(&priv->pdev->dev,
			 "Ignoring request to write read-only GPIO: %u",
			 offset);
		return;
	}

	spin_lock_irqsave(&priv->gpio_lock, flags);
	rc = cpld_reg_rd(priv->cpld_map, gpio_data[offset].reg, &reg_val);
	if (rc) {
		spin_unlock_irqrestore(&priv->gpio_lock, flags);
		return;
	}
	reg_val &= ~BIT(gpio_data[offset].bit);
	if (gpio_data[offset].flags & GPIOF_ACTIVE_LOW)
		value = !value;
	if (value)
		reg_val |= BIT(gpio_data[offset].bit);
	rc = cpld_reg_wr(priv->cpld_map, gpio_data[offset].reg, reg_val);
	spin_unlock_irqrestore(&priv->gpio_lock, flags);
}

/**
 * s6100_cpld_probe()
 *
 * Top level driver probe() callback.
 */
static int __init s6100_cpld_probe(struct platform_device *pdev)
{
	int rc = 0;
	int i;
	struct s6100_cpld_drv_priv *priv;
	struct resource *res;
	void __iomem *iobase;
	char const **names;
	uint8_t version;

	res = platform_get_resource(pdev, IORESOURCE_IO, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "unable to find IO resource\n");
		return -ENODEV;
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->pdev = pdev;

	/* Human readable name */
	snprintf(priv->name, sizeof(priv->name), "cpld");

	/* Initialize LPC register regmap */
	iobase = ioport_map(res->start, resource_size(res));
	priv->cpld_map = devm_regmap_init(&pdev->dev, NULL,
					  (void *)iobase,
					  &s6100_cpld_regmap_config);
	if (IS_ERR(priv->cpld_map)) {
		rc = PTR_ERR(priv->cpld_map);
		dev_err(&pdev->dev, "unable to create cpld regmap: %d\n", rc);
		return rc;
	}

	platform_set_drvdata(pdev, (void *)priv);

	/* Initialize GPIO controller for I/O signals */
	spin_lock_init(&priv->gpio_lock);
	priv->gpio_ctrl.label = S6100_CPLD_GPIO_CHIP_NAME;
	priv->gpio_ctrl.dev   = &pdev->dev;
	priv->gpio_ctrl.owner = THIS_MODULE;
	priv->gpio_ctrl.get_direction = cpld_gpio_get_direction;
	priv->gpio_ctrl.direction_input = cpld_gpio_direction_input;
	priv->gpio_ctrl.direction_output = cpld_gpio_direction_output;
	priv->gpio_ctrl.get = cpld_gpio_get;
	priv->gpio_ctrl.set = cpld_gpio_set;
	priv->gpio_ctrl.base = S6100_CPLD_GPIO_CTRL_BASE;
	priv->gpio_ctrl.ngpio = ARRAY_SIZE(gpio_data);
	priv->gpio_ctrl.can_sleep = false;
	names = devm_kmalloc(&pdev->dev,
			     sizeof(*priv->gpio_ctrl.names) * ARRAY_SIZE(gpio_data),
			     GFP_KERNEL);
	if (!names)
		return -ENOMEM;
	for (i = 0; i < ARRAY_SIZE(gpio_data); i++)
		names[i] = gpio_data[i].name;
	priv->gpio_ctrl.names = names;

	rc = gpiochip_add(&priv->gpio_ctrl);
	if (rc) {
		dev_err(&pdev->dev, "gpiochip_add() failed: %d\n", rc);
		return rc;
	}

	rc = cpld_reg_rd(priv->cpld_map, S6100_CPLD_VERSION, &version);
	if (rc)
		return rc;
	dev_info(&pdev->dev, "cpld firmware version: %d.%d\n",
		 (version >> 4) & 0xF,
		 version & 0xF);

	rc = sysfs_create_groups(&priv->pdev->dev.kobj, misc_system_groups);
	if (rc) {
		dev_err(&priv->pdev->dev,
			"problems creating misc_system sysfs groups\n");
		return rc;
	}

	if (rc == 0)
		dev_info(&pdev->dev, "device probed ok\n");

	return rc;
}

/**
 * s6100_cpld_remove()
 *
 * Top level driver remove() callback.
 */
static int s6100_cpld_remove(struct platform_device *pdev)
{
	struct s6100_cpld_drv_priv *priv = platform_get_drvdata(pdev);

	gpiochip_remove(&priv->gpio_ctrl);
	sysfs_remove_groups(&pdev->dev.kobj, misc_system_groups);

	dev_info(&pdev->dev, "device removed\n");
	return 0;
}

static struct platform_driver s6100_cpld_driver = {
	.driver = {
		.name  = S6100_CPLD_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.remove = s6100_cpld_remove,
	.id_table = s6100_cpld_ids,
};

module_platform_driver_probe(s6100_cpld_driver, s6100_cpld_probe);

MODULE_AUTHOR("Curt Brune <curt@cumulusnetworks.com");
MODULE_DESCRIPTION("DELL S6100 Switch Board CPLD Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
