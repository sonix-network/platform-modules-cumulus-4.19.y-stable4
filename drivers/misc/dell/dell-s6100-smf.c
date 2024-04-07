/*
 * Smartfusion driver for dell_s6100 platforms.
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
#include <linux/platform_data/at24.h>
#include <linux/hwmon-sysfs.h>
#include <linux/gpio.h>
#include <linux/gpio/machine.h>
#include <linux/delay.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "dell-s6100-platform.h"
#include "dell-s6100-smf.h"
#include "dell-s6100-smf-fan.h"
#include "dell-s6100-smf-psu.h"

#define DRIVER_VERSION "1.0"

/**
 * s6100_smf_ids -- driver alias names
 */
static const struct platform_device_id s6100_smf_ids[] = {
	{ S6100_SMF_DRIVER_NAME, 0 },
	{ /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(platform, s6100_smf_ids);

#define MAX_SMF_DEV_NAME_LEN  (20)

/**
 * struct module_cpld_i2c_data
 * @eeprom_label: EEPROM class label for at24 device
 * @eeprom_pdata: EEPROM class platform data
 * @at24_pdata:   at24 device platform data
 * @at24_info:    i2c_board_info for the at24 device
 * @at24_client:  at24 device i2c_client
 * @cpld_client:  module CPLD device i2c_client
 *
 * This structure describes the i2c devices on an IO module connected
 * to the switch board PCA9548.  These i2c devices are common to all
 * IO module types.
 *
 * The driver maintains an instance of this structure for each IO
 * module.
 */
struct module_cpld_i2c_data {
	char eeprom_label[S6100_MAX_EEPROM_NAME_LEN];
	struct eeprom_platform_data eeprom_pdata;
	struct at24_platform_data at24_pdata;
	struct i2c_board_info at24_info;
	struct i2c_client *at24_client;
	struct i2c_client *cpld_client;
};

/**
 * struct s6100_smf_priv -- private driver data
 * @pdev:             Parent platform device
 * @name:             Device name
 * @smf_map:          SMF LPC register map
 * @smf_mb_map:       SMF mailbox register map
 * @fan_frus:         array of fan tray FRU devices
 * @num_fan_frus:     number of fan tray FRU devices
 * @psu_frus:         array of PSU FRU devices
 * @num_psu_frus:     number of PSU FRU devices
 * @num_temps:        number of temperature sensors
 * @temp_attr_group:  attribute group for temperature sensors objects
 * @temp_attr_groups: array of attribute groups for temperature sensors objects
 * @gpio_ctrl:	      GPIO controller object
 * @gpio_lock:        spinlock used to serialize read/modify/write GPIO
 *                    operations
 * @mod_frus:         array of platform devices for each IO module
 * @mod_i2c_data:     array of IO module i2c data for devices on CPLD bus
 *
 * Structure containing private data for the root s6100_smf driver.
 */
struct s6100_smf_drv_priv {
	struct platform_device        *pdev;
	char                           name[MAX_SMF_DEV_NAME_LEN];
	struct regmap                 *smf_map;
	struct regmap                 *smf_mb_map;
	struct platform_device       **fan_frus;
	uint8_t                        num_fan_frus;
	struct platform_device       **psu_frus;
	uint8_t                        num_psu_frus;
	uint8_t                        num_temps;
	struct attribute_group         temp_attr_group;
	const struct attribute_group  *temp_attr_groups[2];
	struct gpio_chip               gpio_ctrl;
	spinlock_t                     gpio_lock;
	struct platform_device        *mod_frus[NUM_IO_MODULES];
	struct module_cpld_i2c_data    mod_i2c_data[NUM_IO_MODULES];
};

#define to_smf_gpio(gc) container_of(gc, struct s6100_smf_drv_priv, gpio_ctrl)

/**
 * smf_ro_ranges
 *
 * Array of regmap read-only ranges for the SMF LPC registers.
 */
static const struct regmap_range smf_ro_ranges[] = {
	regmap_reg_range(S6100_SMF_VER,        S6100_SMF_BOARD_TYPE),
	regmap_reg_range(S6100_SMF_BOOT_OK,    S6100_SMF_MSS_STA),
	regmap_reg_range(S6100_SMF_POR_SOURCE, S6100_SMF_RST_SOURCE),
	regmap_reg_range(S6100_SMF_RAM_R_DATA, S6100_SMF_RAM_R_DATA),
	regmap_reg_range(S6100_SMF_TPM_STA_ID, S6100_SMF_TPM_STA_ID),
};

/**
 * smf_volatile_ranges
 *
 * Array of regmap volatile ranges for the SMF LPC registers.
 */
static const struct regmap_range smf_volatile_ranges[] = {
	regmap_reg_range(S6100_SMF_UART_STA,   S6100_SMF_MSS_STA),
	regmap_reg_range(S6100_SMF_RAM_R_DATA, S6100_SMF_RAM_R_DATA),
};

static struct regmap_access_table smf_write_table = {
	.no_ranges   = smf_ro_ranges,
	.n_no_ranges = ARRAY_SIZE(smf_ro_ranges),
};

static struct regmap_access_table smf_volatile_table = {
	.yes_ranges   = smf_volatile_ranges,
	.n_yes_ranges = ARRAY_SIZE(smf_volatile_ranges),
};

/**
 * smf_regmap_reg_read()
 * @context: IO-space base memory address
 * @reg: register offset
 * @val: pointer to hold register read value
 *
 * regmap read register callback for the IO-space LPC registers
 */
static int
smf_regmap_reg_read(void *context, unsigned int reg, unsigned int *val)
{
	void __iomem *iobase = context;

	*val = ioread8(iobase + reg);
	return 0;
}

/**
 * smf_regmap_reg_write()
 * @context: IO-space base memory address
 * @reg: register offset
 * @val: value to write to register
 *
 * regmap write register callback for the IO-space LPC registers
 */
static int
smf_regmap_reg_write(void *context, unsigned int reg, unsigned int val)
{
	void __iomem *iobase = context;

	iowrite8(val & 0xFF, iobase + reg);
	return 0;
}

/**
 * s6100_smf_regmap_config
 *
 * regmap configuration for the SMF LPC registers.
 */
static struct regmap_config s6100_smf_regmap_config = {
	.name            = "s6100-smf",
	.reg_bits	 = 8,
	.reg_stride	 = 1,
	.pad_bits	 = 0,
	.val_bits	 = 8,
	.fast_io	 = true,
	.max_register	 = S6100_SMF_TPM_STA_ID,
	.wr_table	 = &smf_write_table,
	.volatile_table	 = &smf_volatile_table,
	.use_single_rw	 = true,
	.can_multi_write = false,
	.reg_read        = smf_regmap_reg_read,
	.reg_write       = smf_regmap_reg_write,
};

/**
 * smf_mb_read_ranges
 *
 * Array of readable register ranges for the SMF mailbox registers.
 *
 * For this chip, the set is the union of the read-only register range
 * and the read-write register range.
 */
static const struct regmap_range smf_mb_read_ranges[] = {
	regmap_reg_range(S6100_SMF_MB_PROTO_VER, S6100_SMF_MB_SCAN_RESULT_4),
	regmap_reg_range(S6100_SMF_MB_TEMP_UPDATE_FLAG, S6100_SMF_MB_POWER_CYCLE_CTRL),
};

/**
 * smf_mb_wr_ranges
 *
 * Array of writable register ranges for the SMF mailbox registers.
 */
static const struct regmap_range smf_mb_write_ranges[] = {
	regmap_reg_range(S6100_SMF_MB_TEMP_UPDATE_FLAG, S6100_SMF_MB_POWER_CYCLE_CTRL),
};

/**
 * smf_mb_volatile_ranges
 *
 * Array of regmap volatile ranges for the SMF mailbox registers.
 */
static const struct regmap_range smf_mb_volatile_ranges[] = {
	regmap_reg_range(S6100_SMF_MB_TEMP01_SENSOR, S6100_SMF_MB_TEMP16_STATUS),
	regmap_reg_range(S6100_SMF_MB_FAN_MAX_SPEED,
			 S6100_SMF_MB_FAN_TRAY8_MFG_DATE + S6100_SMF_MB_FAN_TRAY_MFG_DATE_SIZE - 1),
	regmap_reg_range(S6100_SMF_MB_PSU_TOTAL_POWER_H, S6100_SMF_MB_SCAN_RESULT_4),
	regmap_reg_range(S6100_SMF_MB_TEMP_UPDATE_FLAG, S6100_SMF_MB_POWER_CYCLE_CTRL),
};

static struct regmap_access_table smf_mb_write_table = {
	.yes_ranges   = smf_mb_write_ranges,
	.n_yes_ranges = ARRAY_SIZE(smf_mb_write_ranges),
};

static struct regmap_access_table smf_mb_read_table = {
	.yes_ranges   = smf_mb_read_ranges,
	.n_yes_ranges = ARRAY_SIZE(smf_mb_read_ranges),
};

static struct regmap_access_table smf_mb_volatile_table = {
	.yes_ranges   = smf_mb_volatile_ranges,
	.n_yes_ranges = ARRAY_SIZE(smf_mb_volatile_ranges),
};


/**
 * smf_mb_regmap_addr_set() -- set mailbox register address
 * @map: regmap object for smf IO-space registers
 * @addr: mailbox register address
 *
 * Configures the smf mailbox address registers.  Returns 0 on success
 * or negative errno codes from regmap infrastructure.
 */
static int smf_mb_regmap_addr_set(struct regmap *map, unsigned int addr)
{
	int rc;

	/* program indirect addressing registers */
	rc = regmap_write(map, S6100_SMF_RAM_ADDR_H,
			  (addr >> 8) & 0xFF);
	if (WARN(rc, "regmap_write(map, 0x%x, 0x%x) failed: (%d)\n",
		 S6100_SMF_RAM_ADDR_H, (addr >> 8) & 0xFF, rc))
		return rc;

	rc = regmap_write(map, S6100_SMF_RAM_ADDR_L, addr & 0xFF);
	WARN(rc, "regmap_write(map, 0x%x, 0x%x) failed: (%d)\n",
	     S6100_SMF_RAM_ADDR_L, addr & 0xFF, rc);

	return rc;
}

/**
 * smf_mb_regmap_reg_read()
 * @context: regmap object for smf IO-space registers
 * @reg: register offset
 * @val: pointer to hold register read value
 *
 * Regmap read register callback for the SmartFusion mailbox
 * registers.  Returns 0 on success or negative errno codes from
 * regmap infrastructure.
 */
static int
smf_mb_regmap_reg_read(void *context, unsigned int reg, unsigned int *val)
{
	struct regmap *map = (struct regmap *)context;
	int rc = 0;

	rc = smf_mb_regmap_addr_set(map, reg);
	if (rc)
		return rc;

	rc = regmap_read(map, S6100_SMF_RAM_R_DATA, val);
	WARN(rc, "regmap_read(map, 0x%x, val) failed: (%d)\n",
	     S6100_SMF_RAM_R_DATA, rc);

	return rc;
}

/**
 * smf_mb_regmap_reg_write()
 * @context: regmap object for smf IO-space registers
 * @reg: register offset
 * @val: value to write to register
 *
 * regmap write register callback for the SmartFusion mailbox
 * registers.  Returns 0 on success or negative errno codes from
 * regmap infrastructure.
 */
static int
smf_mb_regmap_reg_write(void *context, unsigned int reg, unsigned int val)
{
	struct regmap *map = (struct regmap *)context;
	int rc = 0;

	rc = smf_mb_regmap_addr_set(map, reg);
	if (rc)
		return rc;

	rc = regmap_write(map, S6100_SMF_RAM_W_DATA, val);
	WARN(rc, "regmap_write(map, 0x%x, 0x%x) failed: (%d)\n",
	     S6100_SMF_RAM_W_DATA, val, rc);

	return rc;
}

/**
 * s6100_smf_mb_regmap_config
 *
 * regmap configuration for the SMF mailbox registers.
 */
static struct regmap_config s6100_smf_mb_regmap_config = {
	.name                 = "s6100-smf-mb",
	.reg_bits             = 16,
	.reg_stride           = 1,
	.pad_bits             = 0,
	.val_bits             = 8,
	.fast_io              = true,
	.max_register         = S6100_SMF_MB_POWER_CYCLE_CTRL,
	.wr_table             = &smf_mb_write_table,
	.rd_table             = &smf_mb_read_table,
	.volatile_table       = &smf_mb_volatile_table,
	.use_single_rw        = true,
	.can_multi_write      = false,
	.reg_read             = smf_mb_regmap_reg_read,
	.reg_write            = smf_mb_regmap_reg_write,
};

#define S6100_SMF_GPIO_CTRL_BASE 1100
#define S6100_SMF_GPIO_CHIP_NAME "dell-s6100-smf-gpio"

enum {
	SMF_REG_MAP = 0,
	SMF_MB_REG_MAP,
};

#define GPIO_DATA(_name, _reg_map, _reg, _bit, _flags, _val)		\
	{ .name = __stringify(_name), .reg_map = _reg_map, .reg = _reg,	\
	  .bit = _bit, .flags = _flags, .value = _val, }

#define GPIO_DATA_SMF(_name, _reg, _bit, _flags, _val) \
	GPIO_DATA(_name, SMF_REG_MAP, _reg, _bit, _flags, _val)

#define GPIO_DATA_SMF_MB(_name, _reg, _bit, _flags, _val) \
	GPIO_DATA(_name, SMF_MB_REG_MAP, _reg, _bit, _flags, _val)

static const struct s6100_gpio_data gpio_data[] = {
	GPIO_DATA_SMF_MB(mod1-present,    S6100_SMF_MB_IO_MODULE_PRESENT, 0, GPIOF_DIR_IN | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA_SMF_MB(mod2-present,    S6100_SMF_MB_IO_MODULE_PRESENT, 1, GPIOF_DIR_IN | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA_SMF_MB(mod3-present,    S6100_SMF_MB_IO_MODULE_PRESENT, 2, GPIOF_DIR_IN | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA_SMF_MB(mod4-present,    S6100_SMF_MB_IO_MODULE_PRESENT, 3, GPIOF_DIR_IN | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA_SMF_MB(mod1-pwr-status, S6100_SMF_MB_IO_MODULE_POWER_STATUS, 0, GPIOF_DIR_IN | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA_SMF_MB(mod2-pwr-status, S6100_SMF_MB_IO_MODULE_POWER_STATUS, 1, GPIOF_DIR_IN | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA_SMF_MB(mod3-pwr-status, S6100_SMF_MB_IO_MODULE_POWER_STATUS, 2, GPIOF_DIR_IN | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA_SMF_MB(mod4-pwr-status, S6100_SMF_MB_IO_MODULE_POWER_STATUS, 3, GPIOF_DIR_IN | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA_SMF(eeprom-wp-enable,   S6100_SMF_CPU_EEPROM_WP, CPU_EEPROM_WP_EN, GPIOF_DIR_OUT, 0),
	GPIO_DATA_SMF(primary-bios-boot,  S6100_SMF_BOOT_OK, 0, GPIOF_DIR_IN | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA_SMF(mss-alive,          S6100_SMF_MSS_STA, 0, GPIOF_DIR_IN, 0),
	GPIO_DATA_SMF_MB(master-led,      S6100_SMF_MB_SYS_STACKING_LED_CTRL, 1, GPIOF_DIR_OUT, 0),
	GPIO_DATA_SMF_MB(locator-led,     S6100_SMF_MB_SYS_BEACON_LED_CTRL, 1, GPIOF_DIR_OUT, 0),
};

/**
 * smf_gpio_get_direction()
 *
 * gpio_chip driver interface callback. Returns the current GPIO
 * signal direction.
 */
static int
smf_gpio_get_direction(struct gpio_chip *chip, unsigned offset)
{
	struct s6100_smf_drv_priv *priv = to_smf_gpio(chip);

	if (offset >= ARRAY_SIZE(gpio_data)) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d", offset);
		return -EINVAL;
	}

	return gpio_data[offset].flags & GPIOF_DIR_IN;
}

/**
 * smf_gpio_direction_input()
 *
 * gpio_chip driver interface callback. Configures the specified GPIO
 * as an input.
 */
static int
smf_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	struct s6100_smf_drv_priv *priv = to_smf_gpio(chip);

	if (offset >= ARRAY_SIZE(gpio_data)) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d", offset);
		return -EINVAL;
	}

	return gpio_data[offset].flags & GPIOF_DIR_IN ? 0 : -EINVAL;
}

/**
 * smf_gpio_direction_output()
 *
 * gpio_chip driver interface callback. Configures the specified GPIO
 * as an output.
 */
static int
smf_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
	struct s6100_smf_drv_priv *priv = to_smf_gpio(chip);

	if (offset >= ARRAY_SIZE(gpio_data)) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d", offset);
		return -EINVAL;
	}

	return gpio_data[offset].flags & GPIOF_DIR_IN ? -EINVAL : 0;
}

/**
 * smf_gpio_get()
 *
 * gpio_chip driver interface callback. Returns the current value of
 * the specified GPIO signal.
 */
static int
smf_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	int rc;
	struct s6100_smf_drv_priv *priv = to_smf_gpio(chip);
	uint8_t val;

	if (offset >= ARRAY_SIZE(gpio_data)) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d", offset);
		return -EINVAL;
	}

	if (gpio_data[offset].reg_map == SMF_REG_MAP)
		rc = smf_reg_rd(priv->smf_map, gpio_data[offset].reg, &val);
	else
		rc = smf_mb_reg_rd(priv->smf_mb_map, gpio_data[offset].reg, &val);
	if (rc)
		return rc;

	rc = val & BIT(gpio_data[offset].bit) ? 1 : 0;
	if (gpio_data[offset].flags & GPIOF_ACTIVE_LOW)
		rc = !rc;

	return rc;
}

/**
 * smf_gpio_get()
 *
 * gpio_chip driver interface callback. For output GPIO signals, sets
 * the current value of the specified GPIO signal.
 */
static void
smf_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	int rc;
	struct s6100_smf_drv_priv *priv = to_smf_gpio(chip);
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
	if (gpio_data[offset].reg_map == SMF_REG_MAP)
		rc = smf_reg_rd(priv->smf_map, gpio_data[offset].reg, &reg_val);
	else
		rc = smf_mb_reg_rd(priv->smf_mb_map, gpio_data[offset].reg, &reg_val);
	if (rc) {
		spin_unlock_irqrestore(&priv->gpio_lock, flags);
		return;
	}
	reg_val &= ~BIT(gpio_data[offset].bit);
	if (gpio_data[offset].flags & GPIOF_ACTIVE_LOW)
		value = !value;

	if ((gpio_data[offset].reg == S6100_SMF_MB_SYS_STACKING_LED_CTRL) ||
	    (gpio_data[offset].reg == S6100_SMF_MB_SYS_BEACON_LED_CTRL)) {
		/*
		 * These registers use an enumeration instead of a bitmask.
		 */
		reg_val = value ? 0x2 : 0x1;
	} else {
		if (value)
			reg_val |= BIT(gpio_data[offset].bit);
	}
	if (gpio_data[offset].reg_map == SMF_REG_MAP)
		rc = smf_reg_wr(priv->smf_map, gpio_data[offset].reg, reg_val);
	else
		rc = smf_mb_reg_wr(priv->smf_mb_map, gpio_data[offset].reg, reg_val);
	spin_unlock_irqrestore(&priv->gpio_lock, flags);
}

/**
 * s6100_smf_temp_labels
 *
 * Array of temperature sensor labels
 */
static char *s6100_smf_temp_labels[] = {
	"CPU Board Near CPU",  /* U2900 CPU on-board */
	"Switch Board Near Switch ASIC", /* U44 -BCM56960 on board */
	"Switch Board Right Rear", /* U4 -BCM56960 on board (rear right) */
	"Switch Board Center Middle", /* U2 -BCM56960 on board (center) */
	"IO Module 1", /* U32 - IO module 1 */
	"IO Module 2", /* U32 - IO module 2 */
	"IO Module 3", /* U32 - IO module 3 */
	"IO Module 4", /* U32 - IO module 4 */
	"Switch Board Left Rear", /* U2 - on switch board */
	"Front Panel GigEth", /* U2900 - front panel GE board */
	"Front Panel SFP+", /* U2900 - front panel SFP plus board */
};

/**
 * s6100_smf_temp_type
 *
 * Temperature sensor attribute types
 */
enum s6100_smf_temp_type {
	TEMP_TYPE_INPUT = 0,
	TEMP_TYPE_MAX,
	TEMP_TYPE_CRIT,
	TEMP_TYPE_EMERG,
};

/**
 * smf_mb_label_show
 *
 * return the SMF hwmon device label
 */
static ssize_t smf_mb_label_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	char *label;

	if (attr->index > priv->num_temps) {
		dev_err(&priv->pdev->dev,
			"unexpected sfm label index: %d\n", attr->index);
		return -EINVAL;
	}
	if (attr->index >= ARRAY_SIZE(s6100_smf_temp_labels))
		label = "unknown sensor";
	else
		label = s6100_smf_temp_labels[attr->index];

	return snprintf(buf, PAGE_SIZE, "%s\n", label);
}

/**
 * smf_mb_temp_show
 *
 * Return the SMF hwmon device temperature data. The type of data
 * returned depends on the .nr field of the struct
 * sensor_device_attribute_2 object.
 */
static ssize_t smf_mb_temp_show(struct device *dev,
				struct device_attribute *dattr,
				char *buf)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	int reg;
	uint16_t temp;
	int index_shift;
	int rc;

	if (attr->index > priv->num_temps) {
		dev_err(&priv->pdev->dev,
		     "unexpected sfm temperature device index: %d\n",
		     attr->index);
		return -EINVAL;
	}

	switch (attr->nr) {
	case TEMP_TYPE_INPUT:
		reg = S6100_SMF_MB_TEMP01_SENSOR;
		index_shift = 1;
		break;
	case TEMP_TYPE_MAX:
		reg = S6100_SMF_MB_TEMP01_MNR_ALARM_LIMIT;
		index_shift = 3;
		break;
	case TEMP_TYPE_CRIT:
		reg = S6100_SMF_MB_TEMP01_MJR_ALARM_LIMIT;
		index_shift = 3;
		break;
	case TEMP_TYPE_EMERG:
		reg = S6100_SMF_MB_TEMP01_HW_SHUT_LIMIT;
		index_shift = 3;
		break;
	default:
		WARN(1, "unexpected sfm temperature data type: %d\n", attr->nr);
		return -EINVAL;
	};

	reg += attr->index << index_shift;
	rc = smf_mb_reg_rd16(priv->smf_mb_map, reg, &temp);
	if (rc)
		return rc;

	if (temp == S6100_SMF_MB_BAD_READ)
		return -EAGAIN;

	/*
	 * The temperature register is in units of centidegree
	 * Celsius.  The hwmon interface is in millidegree Celsius.
	 */
	return snprintf(buf, PAGE_SIZE, "%d\n", (int)temp * 100);
}

/**
 * smf_mb_temp_show_default
 *
 * Return fixed default values for MAX and CRIT limits.
 */
#define SMF_DEFAULT_TMP_MAX  (80)
#define SMF_DEFAULT_TMP_CRIT (85)
static ssize_t smf_mb_temp_show_default(struct device *dev,
					struct device_attribute *dattr,
					char *buf)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	uint16_t temp;

	if (attr->index > priv->num_temps) {
		dev_err(&priv->pdev->dev,
		     "unexpected sfm temperature device index: %d\n",
		     attr->index);
		return -EINVAL;
	}

	switch (attr->nr) {
	case TEMP_TYPE_MAX:
		temp = SMF_DEFAULT_TMP_MAX;
		break;
	case TEMP_TYPE_CRIT:
		temp = SMF_DEFAULT_TMP_CRIT;
		break;
	default:
		WARN(1, "unexpected sfm temperature data type: %d\n", attr->nr);
		return -EINVAL;
	};

	/*
	 * The hwmon interface is in millidegree Celsius.
	 */
	return snprintf(buf, PAGE_SIZE, "%d\n", (int)temp * 1000);
}

/**
 * smf_mb_fault_show
 *
 * Return the SMF hwmon device fault status.
 */
static ssize_t smf_mb_fault_show(struct device *dev,
				struct device_attribute *dattr,
				char *buf)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	int reg = S6100_SMF_MB_TEMP01_STATUS;
	uint8_t status;
	int rc;

	if (attr->index > priv->num_temps) {
		dev_err(&priv->pdev->dev,
			"unexpected sfm temperature device index: %d\n",
			attr->index);
		return -EINVAL;
	}

	reg += attr->index;
	rc = smf_mb_reg_rd(priv->smf_mb_map, reg, &status);
	if (rc)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%d\n",
			(status == S6100_SMF_MB_TEMP_STATUS_FAULT) ? 1 : 0);
}

/**
 * dell_s6100_smf_hwmon_attrs
 *
 * Template to use for temperature sensors attributes
 */
static struct sensor_device_attribute_2 temp_attr_data[] __initdata = {
	ATTR_DATA_RO_2(temp%d_label,     smf_mb_label_show, 0,  0),
	ATTR_DATA_RO_2(temp%d_input,     smf_mb_temp_show,  0,  TEMP_TYPE_INPUT),
	ATTR_DATA_RO_2(temp%d_max,       smf_mb_temp_show,  0,  TEMP_TYPE_MAX),
	ATTR_DATA_RO_2(temp%d_crit,      smf_mb_temp_show,  0,  TEMP_TYPE_CRIT),
	ATTR_DATA_RO_2(temp%d_emergency, smf_mb_temp_show,  0,  TEMP_TYPE_EMERG),
	ATTR_DATA_RO_2(temp%d_fault,     smf_mb_fault_show, 0,  0),
};

/**
 * make_temp_hwmon_attrs()
 *
 * @priv:      private driver data
 * @attrs:     array of attributes pointers [out]
 * @n_created: number of attributes created [out]
 * @pattr:     array of struct sensor_device_attribute_2 entries [in]
 * @num_attrs: length of @pattr [in]
 * @inst:      temp sensor instance
 *
 * Given an array of struct sensor_device_attribute_2 entries in
 * @pattr, generate an array of attributes for the sensor instance.
 * The array is returned in @attrs along with its length in
 * @n_created.
 *
 * The @pattr array is used as a template for creatng the actual temp
 * attributes at run time, parameterized by @inst.
 *
 * Every temp sensors does not implement all possible attributes, so
 * @n_created does not necessarily equal @num_attrs.
 *
 * Returns 0 on success or other negative errno.
 */
static int __init
make_temp_hwmon_attrs(struct s6100_smf_drv_priv *priv, struct attribute **attrs,
		      int *n_created, struct sensor_device_attribute_2 *pattr,
		      int num_attrs, int inst)
{
	int rc = 0, i;
	int index_shift;
	int reg;
	uint16_t temp = 0;
	bool attribute_absent;
	struct sensor_device_attribute_2 *sensor_attr;
	struct attribute **new_attrs = attrs;
	char *name;

	*n_created = 0;
	for (i = 0; i < num_attrs; i++, pattr++) {
		/*
		 * check if sensor attribute is implemented in the hardware
		 */
		attribute_absent = false;
		if (pattr->nr >= TEMP_TYPE_MAX) {
			switch (pattr->nr) {
			case TEMP_TYPE_MAX:
				reg = S6100_SMF_MB_TEMP01_MNR_ALARM_LIMIT;
				index_shift = 3;
				break;
			case TEMP_TYPE_CRIT:
				reg = S6100_SMF_MB_TEMP01_MJR_ALARM_LIMIT;
				index_shift = 3;
				break;
			case TEMP_TYPE_EMERG:
				reg = S6100_SMF_MB_TEMP01_HW_SHUT_LIMIT;
				index_shift = 3;
				break;
			default:
				WARN(1, "unexpected sfm temperature data type: %d\n",
				     pattr->nr);
				return -EINVAL;
			}
			reg += inst << index_shift;
			rc = smf_mb_reg_rd16(priv->smf_mb_map, reg, &temp);
			if (rc)
				return rc;
			if (temp == S6100_SMF_MB_BAD_READ)
				attribute_absent = true;

			if (attribute_absent && (pattr->nr == TEMP_TYPE_EMERG))
				/* attribute is not supported */
				continue;
		}

		sensor_attr = devm_kzalloc(&priv->pdev->dev,
					   sizeof(*sensor_attr), GFP_KERNEL);
		if (!sensor_attr)
			return -ENOMEM;
		sysfs_attr_init(&sensor_attr->dev_attr.attr);
		name = devm_kasprintf(&priv->pdev->dev, GFP_KERNEL,
				      pattr->dev_attr.attr.name, inst + 1);
		if (!name)
			return -ENOMEM;
		sensor_attr->dev_attr.attr.name = name;
		sensor_attr->dev_attr.attr.mode = pattr->dev_attr.attr.mode;
		if (attribute_absent) {
			sensor_attr->dev_attr.show      = smf_mb_temp_show_default;
		} else {
			sensor_attr->dev_attr.show      = pattr->dev_attr.show;
		}
		sensor_attr->dev_attr.store     = pattr->dev_attr.store;
		sensor_attr->index              = inst;
		sensor_attr->nr                 = pattr->nr;

		*new_attrs++ = &sensor_attr->dev_attr.attr;
	}

	*n_created = new_attrs - attrs;

	return 0;
}

/**
 * probe_temperature_system()
 *
 * Device probe routine, handling all SMF temperature sensors
 */
static int __init probe_temperature_system(struct s6100_smf_drv_priv *priv)
{
	int rc = 0;
	int i, max_attr = 0;
	int n_created;
	struct attribute **temp_attrs;
	struct device *hwmon_dev;

	rc = smf_mb_reg_rd(priv->smf_mb_map,
			   S6100_SMF_MB_NUM_TEMP_SENSORS, &priv->num_temps);
	if (rc)
		return rc;

	if (priv->num_temps != ARRAY_SIZE(s6100_smf_temp_labels))
		dev_warn(&priv->pdev->dev,
			 "unexpected number of temp sensors: %d.  Expected: %lu\n",
			 priv->num_temps, ARRAY_SIZE(s6100_smf_temp_labels));

	/*
	 * Maximum possible number of temp sensors attributes, plus
	 * one for the NULL entry of the attr array.  Some temp
	 * sensors do not have all possible attributes.
	 */
	max_attr = priv->num_temps * ARRAY_SIZE(temp_attr_data) + 1;
	temp_attrs = devm_kzalloc(&priv->pdev->dev,
				  max_attr * sizeof(*temp_attrs), GFP_KERNEL);
	if (!temp_attrs)
		return -ENOMEM;

	priv->temp_attr_group.attrs = temp_attrs;
	priv->temp_attr_groups[0] = &priv->temp_attr_group;
	priv->temp_attr_groups[1] = NULL;

	for (i = 0; i < priv->num_temps; i++) {
		rc = make_temp_hwmon_attrs(priv, temp_attrs, &n_created,
					   temp_attr_data,
					   ARRAY_SIZE(temp_attr_data),
					   i);
		if (rc)
			return rc;
		temp_attrs += n_created;
	}

	hwmon_dev = devm_hwmon_device_register_with_groups(&priv->pdev->dev,
							   priv->name,
							   (void *)priv,
							   priv->temp_attr_groups);
	if (IS_ERR(hwmon_dev)) {
		rc = PTR_ERR(hwmon_dev);
		dev_err(&priv->pdev->dev,
			"unable to create smf hwmon device: %d\n", rc);
	}

	return rc;
}

/**
 * smf_mb_u8_show()
 *
 * Return an 8-bit smf mailbox register, decoded as a simple integer.
 */
static ssize_t smf_mb_u8_show(struct device *dev,
			      struct device_attribute *dattr,
			      char *buf)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	uint8_t val;
	int rc;

	rc = smf_mb_reg_rd(priv->smf_mb_map, attr->index, &val);
	if (rc)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

/**
 * struct attribute *fan_system_attrs[]
 *
 * array of system level fan sysfs attributes.  These fan attributes
 * have system level scope, as opposed to fan tray (FRU) scope.
 */
static SENSOR_ATTR_DATA_RO(fan_tray_fru_cnt, smf_mb_u8_show, S6100_SMF_MB_FAN_TRAY_CNT);
static struct attribute *fan_system_attrs[] = {
	&sensor_dev_attr_fan_tray_fru_cnt.dev_attr.attr,
	NULL,
};

static struct attribute_group fan_system_attr_group = {
	.attrs = fan_system_attrs,
};

/**
 * probe_fan_system()
 *
 * Device probe routine, handling all fan related operations.
 */
static int __init probe_fan_system(struct s6100_smf_drv_priv *priv)
{
	int rc = 0;
	int i;
	struct platform_device *fan_dev;
	struct s6100_smf_fan_platform_data fan_pdata;
	struct platform_device_info s6100_smf_fan_info = {
		.parent    = &priv->pdev->dev,
		.name      = S6100_SMF_FAN_DRIVER_NAME,
		.data      = &fan_pdata,
		.size_data = sizeof(fan_pdata),
	};

	/* Allocate storage for fan fru device objects */
	rc = smf_mb_reg_rd(priv->smf_mb_map,
			   S6100_SMF_MB_FAN_TRAY_CNT, &priv->num_fan_frus);
	if (rc)
		return rc;
	priv->fan_frus = devm_kzalloc(&priv->pdev->dev,
				      priv->num_fan_frus * sizeof(*priv->fan_frus),
				      GFP_KERNEL);
	if (!priv->fan_frus)
		return -ENOMEM;

	/* Pass the smf mailbox regmap to the children */
	fan_pdata.smf_mb_map = priv->smf_mb_map;

	for (i = 0; i < priv->num_fan_frus; i++) {
		s6100_smf_fan_info.id = i;
		fan_dev = platform_device_register_full(&s6100_smf_fan_info);
		if (IS_ERR(fan_dev)) {
			rc = PTR_ERR(fan_dev);
			goto error_fan_add;
		}
		priv->fan_frus[i] = fan_dev;
	}
	return rc;

error_fan_add:
	while (--i >= 0)
		platform_device_unregister(priv->fan_frus[i]);

	return rc;
}

/**
 * smf_mb_power_show()
 *
 * Return the total PSU power in microWatts
 */
static ssize_t smf_mb_power_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	uint16_t val;
	int rc;

	rc = smf_mb_reg_rd16(priv->smf_mb_map, attr->index, &val);
	if (rc)
		return rc;

	/*
	 * The register value is in deciWatts.  Convert to microWatts
	 * for hwmon/sysfs interface.
	 */
	return snprintf(buf, PAGE_SIZE, "%u\n", ((uint32_t)val) * 100 * 1000);
}

/**
 * struct attribute *psu_system_attrs[]
 *
 * array of system level PSU sysfs attributes.  These PSU attributes
 * have system level scope, as opposed to PSU module (FRU) scope.
 */
static SENSOR_ATTR_DATA_RO(psu_fru_cnt,     smf_mb_u8_show,    S6100_SMF_MB_PSU_CNT);
static SENSOR_ATTR_DATA_RO(psu_total_power, smf_mb_power_show, S6100_SMF_MB_PSU_TOTAL_POWER_H);
static struct attribute *psu_system_attrs[] = {
	&sensor_dev_attr_psu_fru_cnt.dev_attr.attr,
	&sensor_dev_attr_psu_total_power.dev_attr.attr,
	NULL,
};

static struct attribute_group psu_system_attr_group = {
	.attrs = psu_system_attrs,
};

/**
 * probe_psu_system()
 *
 * Device probe routine, handling all PSU related operations.
 */
static int __init probe_psu_system(struct s6100_smf_drv_priv *priv)
{
	int rc = 0;
	int i;
	struct platform_device *psu_dev;
	struct s6100_smf_psu_platform_data psu_pdata;
	struct platform_device_info s6100_smf_psu_info = {
		.parent    = &priv->pdev->dev,
		.name      = S6100_SMF_PSU_DRIVER_NAME,
		.data      = &psu_pdata,
		.size_data = sizeof(psu_pdata),
	};

	/* Allocate storage for PSU device objects */
	rc = smf_mb_reg_rd(priv->smf_mb_map, S6100_SMF_MB_PSU_CNT,
			   &priv->num_psu_frus);
	if (rc)
		return rc;
	priv->psu_frus = devm_kzalloc(&priv->pdev->dev,
				      priv->num_psu_frus * sizeof(*priv->psu_frus),
				      GFP_KERNEL);
	if (!priv->psu_frus)
		return -ENOMEM;

	/* Pass the smf mailbox regmap to the children */
	psu_pdata.smf_mb_map = priv->smf_mb_map;

	for (i = 0; i < priv->num_psu_frus; i++) {
		s6100_smf_psu_info.id = i;
		psu_dev = platform_device_register_full(&s6100_smf_psu_info);
		if (IS_ERR(psu_dev)) {
			rc = PTR_ERR(psu_dev);
			goto error_psu_add;
		}
		priv->psu_frus[i] = psu_dev;
	}
	return rc;

error_psu_add:
	while (--i >= 0)
		platform_device_unregister(priv->psu_frus[i]);

	return rc;
}

/**
 * io_mod_pwr_show()
 *
 * Return the IO module power status
 */
static ssize_t io_mod_pwr_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	uint8_t val;
	int rc;

	rc = smf_mb_reg_rd(priv->smf_mb_map,
			   S6100_SMF_MB_IO_MODULE1_POWER_CTRL + attr->index,
			   &val);
	if (rc)
		return rc;

	/* Only the lower 2-bits are valid */
	val &= 0x3;
	return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

/**
 * io_mod_pwr_store()
 *
 * Set the IO module power state
 */
static ssize_t io_mod_pwr_store(struct device *dev,
				struct device_attribute *dattr,
				const char *buf, size_t count)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	uint8_t val;
	int rc;

	rc = kstrtou8(buf, 0, &val);
	if (rc)
		return rc;

	/*
	 * Only two values are valid:
	 *
	 *   1 -- power on the module
	 *   2 -- power off the module
	 */
	if ((val != 1) && (val != 2))
		return -EINVAL;

	rc = smf_mb_reg_wr(priv->smf_mb_map,
			   S6100_SMF_MB_IO_MODULE1_POWER_CTRL + attr->index,
			   val);
	if (rc)
		return rc;

	return count;
}

/**
 * fw_version_show()
 *
 * Return the SMF firmware version.
 */
static ssize_t fw_version_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	uint8_t val;
	uint8_t ver_shift;
	int rc;

	if (attr->index == S6100_SMF_VER) {
		ver_shift = 4;
		rc = smf_reg_rd(priv->smf_map,
				attr->index, &val);
	} else if (attr->index == S6100_SMF_MB_FIRMWARE_VER) {
		ver_shift = 6;
		rc = smf_mb_reg_rd(priv->smf_mb_map,
				   attr->index, &val);
	} else {
		dev_err(dev, "unexpected firmware version register: 0x%x\n",
			attr->index);
		return -EINVAL;
	}

	if (rc)
		return rc;

	return snprintf(buf, PAGE_SIZE, "%lu.%lu\n",
			(val >> ver_shift) & GENMASK(8 - ver_shift - 1, 0),
			val & GENMASK(ver_shift - 1, 0));
}

/**
 * smf_reg_show()
 *
 * Return the one byte SMF register formatted as hex.
 */
static ssize_t smf_reg_show(struct device *dev,
			    struct device_attribute *dattr,
			    char *buf)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	uint8_t val;
	int rc;

	rc = smf_reg_rd(priv->smf_map, attr->index, &val);
	if (rc)
		return rc;

	return snprintf(buf, PAGE_SIZE, "0x%02x\n", val);
}

/**
 * status_led_colors[]
 *
 * Simple look-up table, mapping S6100_SMF_MB_SYS_STATUS_LED_CTRL
 * register fields to human readable strings.
 */
struct led_map {
	char*   color;
	uint8_t value;
};

static struct led_map status_leds[] = {
	{
		.color = PLATFORM_LED_GREEN,
		.value = SYS_LED_STATUS_GREEN,
	},
	{
		.color = PLATFORM_LED_GREEN_BLINKING,
		.value = SYS_LED_STATUS_GREEN_BLINKING,
	},
	{
		.color = PLATFORM_LED_AMBER,
		.value = SYS_LED_STATUS_AMBER,
	},
	{
		.color = PLATFORM_LED_AMBER_BLINKING,
		.value = SYS_LED_STATUS_AMBER_BLINKING,
	},
};

/**
 * led_status_show()
 *
 * Return a human readable string representing the current setting of
 * the system status LED.
 */
static ssize_t led_status_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	uint8_t val;
	char *color = "unknown";
	int i;
	int rc;

	rc = smf_mb_reg_rd(priv->smf_mb_map,
			   S6100_SMF_MB_SYS_STATUS_LED_CTRL, &val);
	if (rc)
		return rc;

	for (i = 0; i < ARRAY_SIZE(status_leds); i++)
		if (status_leds[i].value == val) {
			color = status_leds[i].color;
			break;
		}

	return snprintf(buf, PAGE_SIZE, "%s\n", color);
}

/**
 * led_status_store()
 *
 * Set the system status LED from a human readable string.
 */
static ssize_t led_status_store(struct device *dev,
				struct device_attribute *dattr,
				const char *buf, size_t count)
{
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);
	int i;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];
	int rc;

	if (sscanf(buf, "%19s", raw) <= 0)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(status_leds); i++)
		if (strncmp(raw, status_leds[i].color, count) == 0)
			break;

	if (i == ARRAY_SIZE(status_leds))
		return -EINVAL;

	rc = smf_mb_reg_wr(priv->smf_mb_map,
			   S6100_SMF_MB_SYS_STATUS_LED_CTRL,
			   status_leds[i].value);
	if (rc)
		return rc;

	return count;
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
	struct s6100_smf_drv_priv *priv = dev_get_drvdata(dev);

	return cumulus_gpio_map_show(dev, &priv->gpio_ctrl, buf);
}

/**
 * struct attribute *misc_system_attrs[]
 *
 * Array of misc SMF system level sysfs attributes.
 */
static SENSOR_DEVICE_ATTR(mod1_pwr_ctrl, S_IRUGO | S_IWUSR, io_mod_pwr_show, io_mod_pwr_store, 0);
static SENSOR_DEVICE_ATTR(mod2_pwr_ctrl, S_IRUGO | S_IWUSR, io_mod_pwr_show, io_mod_pwr_store, 1);
static SENSOR_DEVICE_ATTR(mod3_pwr_ctrl, S_IRUGO | S_IWUSR, io_mod_pwr_show, io_mod_pwr_store, 2);
static SENSOR_DEVICE_ATTR(mod4_pwr_ctrl, S_IRUGO | S_IWUSR, io_mod_pwr_show, io_mod_pwr_store, 3);
static SENSOR_ATTR_DATA_RO(cpld_version,    fw_version_show, S6100_SMF_VER);
static SENSOR_ATTR_DATA_RO(mailbox_version, fw_version_show, S6100_SMF_MB_FIRMWARE_VER);
static SENSOR_ATTR_DATA_RO(uart_status,     smf_reg_show,    S6100_SMF_UART_STA);
static SENSOR_ATTR_DATA_RO(por_source,      smf_reg_show,    S6100_SMF_POR_SOURCE);
static SENSOR_ATTR_DATA_RO(reset_source,    smf_reg_show,    S6100_SMF_RST_SOURCE);
static SENSOR_ATTR_DATA_RW_2(led_status,    led_status_show, led_status_store, 0, 0);
static SENSOR_ATTR_DATA_RO(gpio_map,        gpio_map_show,   0);
static struct attribute *misc_system_attrs[] = {
	&sensor_dev_attr_mod1_pwr_ctrl.dev_attr.attr,
	&sensor_dev_attr_mod2_pwr_ctrl.dev_attr.attr,
	&sensor_dev_attr_mod3_pwr_ctrl.dev_attr.attr,
	&sensor_dev_attr_mod4_pwr_ctrl.dev_attr.attr,
	&sensor_dev_attr_cpld_version.dev_attr.attr,
	&sensor_dev_attr_mailbox_version.dev_attr.attr,
	&sensor_dev_attr_uart_status.dev_attr.attr,
	&sensor_dev_attr_por_source.dev_attr.attr,
	&sensor_dev_attr_reset_source.dev_attr.attr,
	&sensor_dev_attr_led_status.dev_attr.attr,
	&sensor_dev_attr_gpio_map.dev_attr.attr,
	NULL,
};

static struct attribute_group misc_system_attr_group = {
	.attrs = misc_system_attrs,
};

static const struct attribute_group *smf_attr_groups[] = {
	&fan_system_attr_group,
	&psu_system_attr_group,
	&misc_system_attr_group,
	NULL,
};

static const char *smf_module_type_names[] = {
	"16x40G QSFP+",
	"8x100G QSFP28",
	"8x100G CXP",
};

static const char smf_module_supported_types[] = {
	S6100_MODULE_ID_16X40G_QSFP,
	/* S6100_MODULE_ID_8X100G_QSFP28, */
	/* S6100_MODULE_ID_8X100G_CXP, */
};

/**
 * probe_io_modules()
 *
 * Probe for present IO modules.  For every present IO module:
 *
 * -power it up
 * -create a IO module platform device.
 */
static int __init probe_io_modules(struct s6100_smf_drv_priv *priv)
{
	int rc = 0;
	int i, j;
	uint8_t mod_present, mod_power_ok;
	int msec;
	uint8_t val;
	int bus_offset;
	struct resource *mod_cpld_res;
	struct resource *mod_optical_res;
	struct module_cpld_i2c_data *i2c_data;
	struct i2c_adapter *cpld_bus_adapter = NULL;
	struct platform_device *module_dev;
	struct s6100_module_platform_data module_pdata;
	struct platform_device_info s6100_smf_module_info = {
		.parent    = &priv->pdev->dev,
		.name      = S6100_MODULE_DRIVER_NAME,
		.data      = &module_pdata,
		.size_data = sizeof(module_pdata),
	};

	mod_optical_res = platform_get_resource(priv->pdev, IORESOURCE_BUS, 0);
	if (mod_optical_res == NULL) {
		dev_err(&priv->pdev->dev,
			"unable to find module Optical I2C Bus resource\n");
		return -ENODEV;
	}

	mod_cpld_res = platform_get_resource(priv->pdev, IORESOURCE_BUS, 1);
	if (mod_cpld_res == NULL) {
		dev_err(&priv->pdev->dev,
			"unable to find module CPLD I2C Bus resource\n");
		return -ENODEV;
	}

	rc = smf_mb_reg_rd(priv->smf_mb_map,
			   S6100_SMF_MB_IO_MODULE_PRESENT, &mod_present);
	if (rc)
		return rc;

	/* present register is active low */
	mod_present = ~mod_present & 0xF;
	mod_power_ok = 0;

	/* Power off all modules */
	for (i = 0; i < NUM_IO_MODULES; i++) {
		rc = smf_mb_reg_wr(priv->smf_mb_map,
				   S6100_SMF_MB_IO_MODULE1_POWER_CTRL + i,
				   S6100_IO_MODULE_POWER_OFF);
		if (rc)
			return rc;
	}
	/* wait with modules off for a bit */
	msleep(1000);

	/* Power up modules in parallel */
	for (i = 0; i < NUM_IO_MODULES; i++) {
		if (mod_present & BIT(i)) {
			dev_info(&priv->pdev->dev, "mod %d: Powering up\n", i + 1);
			rc = smf_mb_reg_wr(priv->smf_mb_map,
					   S6100_SMF_MB_IO_MODULE1_POWER_CTRL + i,
					   S6100_IO_MODULE_POWER_ON);
			if (rc)
				return rc;
		}
	}

	/* check the power up status for up to 10000ms */
	msec = 10000;
	do {
		for (i = 0; i < NUM_IO_MODULES; i++) {
			if ((mod_present & BIT(i)) &&
			    !(mod_power_ok & BIT(i))) {
				rc = smf_mb_reg_rd(priv->smf_mb_map,
						   S6100_SMF_MB_IO_MODULE1_POWER_CTRL + i,
						   &val);
				if (rc)
					return rc;
				if ((val & S6100_IO_MODULE_POWER_MASK) == S6100_IO_MODULE_POWER_OK) {
					mod_power_ok |= BIT(i);
					dev_dbg(&priv->pdev->dev,
						"mod %d: power up ok, msec remaining: %d\n",
						i + 1, msec);
				}
			}
		}
		/* Are all present modules powered up? */
		if (mod_present == mod_power_ok)
			break;
		msleep(100);
		msec -= 100;
	} while (msec);
	dev_info(&priv->pdev->dev, "power up msec remaining: %d\n", msec);

	for (i = 0; i < NUM_IO_MODULES; i++) {
		if (mod_present & BIT(i)) {
			if (!(mod_power_ok & BIT(i))) {
				/* timed out waiting for POWER_OK */
				dev_warn(&priv->pdev->dev,
					 "mod %d: Unable to power up\n", i + 1);
			} else {
				dev_info(&priv->pdev->dev,
					 "mod %d: Powered up OK\n", i + 1);
			}
		}
	}

	for (i = 0; i < NUM_IO_MODULES; i++) {
		if (!(mod_power_ok & BIT(i)))
			continue;

		if (i == 1)
			bus_offset = 2;
		else if (i == 2)
			bus_offset = 1;
		else
			bus_offset = i;

		/* create common i2c devices on the CPLD i2c bus
		 *
		 * - 0x50 -- EEPROM
		 * - 0x3e -- CPLD
		 */

		i2c_data = priv->mod_i2c_data + i;

		/* setup EEPROM device */
		snprintf(i2c_data->eeprom_label, sizeof(i2c_data->eeprom_label),
			 "module%d_eeprom", i + 1);
		i2c_data->eeprom_pdata.label = i2c_data->eeprom_label;
		i2c_data->at24_pdata.byte_len = 8192;
		i2c_data->at24_pdata.flags = AT24_FLAG_IRUGO | AT24_FLAG_ADDR16;
		i2c_data->at24_pdata.page_size = 1;
		i2c_data->at24_pdata.eeprom_data = &i2c_data->eeprom_pdata;
		strncpy(i2c_data->at24_info.type, "24c64",
			sizeof(i2c_data->at24_info.type) - 1);
		i2c_data->at24_info.addr = 0x50;
		i2c_data->at24_info.platform_data = &i2c_data->at24_pdata;

		cpld_bus_adapter = i2c_get_adapter(mod_cpld_res->start + bus_offset);
		if (!cpld_bus_adapter) {
			dev_err(&priv->pdev->dev,
				"unable to find cpld I2C adapter: %llu\n",
				mod_cpld_res->start + bus_offset);
			goto error_module_clean_up;
		}
		i2c_data->at24_client = i2c_new_device(cpld_bus_adapter,
						       &i2c_data->at24_info);
		if (IS_ERR(i2c_data->at24_client)) {
			rc = PTR_ERR(i2c_data->at24_client);
			i2c_data->at24_client = NULL;
			dev_err(&priv->pdev->dev,
				"mod %d: problems creating EEPROM i2c device: %d\n",
				i + 1, rc);
			goto error_module_clean_up;
		}

		/* setup CPLD device using a dummy device */
		i2c_data->cpld_client = i2c_new_dummy(cpld_bus_adapter, 0x3e);
		if (IS_ERR(i2c_data->cpld_client)) {
			rc = PTR_ERR(i2c_data->cpld_client);
			i2c_data->cpld_client = NULL;
			dev_err(&priv->pdev->dev,
				"mod %d: problems creating CPLD i2c device: %d\n",
				i + 1, rc);
			goto error_module_clean_up;
		}
		i2c_put_adapter(cpld_bus_adapter);
		cpld_bus_adapter = NULL;

		/* Verify module type is supported */
		rc = module_cpld_raw_reg_rd(i2c_data->cpld_client, S6100_MOD_MODULE_ID, &val);
		for (j = 0; j < ARRAY_SIZE(smf_module_supported_types); j++)
			if (val == smf_module_supported_types[j])
				break;
		if (j == ARRAY_SIZE(smf_module_supported_types))
			dev_warn(&priv->pdev->dev,
				 "mod %d: warning: unsupported module type: %s(%d).\n",
				 i + 1,
				 val < ARRAY_SIZE(smf_module_type_names) ? smf_module_type_names[val] : "Unknown",
				 val);

		/*
		 * create platform device, passing it the cpld client
		 * and optical i2c adapter info.
		 */
		module_pdata.cpld_client = i2c_data->cpld_client;
		module_pdata.optical_i2c_bus = mod_optical_res->start + bus_offset;
		s6100_smf_module_info.id = i;
		module_dev = platform_device_register_full(&s6100_smf_module_info);
		if (IS_ERR(module_dev)) {
			rc = PTR_ERR(module_dev);
			dev_err(&priv->pdev->dev,
				"mod %d: platform_device_register_full() failed: %d\n",
				i + 1, rc);
			goto error_module_clean_up;
		}
		priv->mod_frus[i] = module_dev;
	}

	return rc;

error_module_clean_up:
	if (cpld_bus_adapter)
		i2c_put_adapter(cpld_bus_adapter);
	for (i = 0; i < NUM_IO_MODULES; i++) {
		if (priv->mod_frus[i])
			platform_device_unregister(priv->mod_frus[i]);
		if (priv->mod_i2c_data[i].at24_client)
			i2c_unregister_device(priv->mod_i2c_data[i].at24_client);
		if (priv->mod_i2c_data[i].cpld_client)
			i2c_unregister_device(priv->mod_i2c_data[i].cpld_client);
	}
	return rc;
}

/**
 * s6100_smf_probe()
 *
 * Top level s6100_smf device probe routine.
 */
static int __init s6100_smf_probe(struct platform_device *pdev)
{
	int rc = 0;
	int i;
	struct s6100_smf_drv_priv *priv;
	struct resource *res;
	char const **names;
	void __iomem *iobase;
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
	snprintf(priv->name, sizeof(priv->name), "smf");

	/* Initialize LPC register regmap */
	iobase = ioport_map(res->start, resource_size(res));
	priv->smf_map = devm_regmap_init(&pdev->dev, NULL,
					 (void *)iobase,
					 &s6100_smf_regmap_config);
	if (IS_ERR(priv->smf_map)) {
		rc = PTR_ERR(priv->smf_map);
		dev_err(&pdev->dev, "unable to create smf regmap: %d\n", rc);
		return rc;
	}

	rc = smf_reg_rd(priv->smf_map, S6100_SMF_VER, &version);
	if (rc)
		return rc;
	dev_info(&pdev->dev, "smf firmware version: %d.%d\n",
		 (version >> 4) & 0xF,
		 version & 0xF);

	/* Initialize SMF mailbox register regmap */
	priv->smf_mb_map = devm_regmap_init(&pdev->dev, NULL,
					    (void *)priv->smf_map,
					    &s6100_smf_mb_regmap_config);
	if (IS_ERR(priv->smf_mb_map)) {
		rc = PTR_ERR(priv->smf_mb_map);
		dev_err(&pdev->dev, "unable to create smf_mb regmap: %d\n", rc);
		return rc;
	}

	platform_set_drvdata(pdev, (void *)priv);

	/* Initialize GPIO controller for I/O signals */
	spin_lock_init(&priv->gpio_lock);
	priv->gpio_ctrl.label = S6100_SMF_GPIO_CHIP_NAME;
	priv->gpio_ctrl.dev   = &pdev->dev;
	priv->gpio_ctrl.owner = THIS_MODULE;
	priv->gpio_ctrl.get_direction = smf_gpio_get_direction;
	priv->gpio_ctrl.direction_input = smf_gpio_direction_input;
	priv->gpio_ctrl.direction_output = smf_gpio_direction_output;
	priv->gpio_ctrl.get = smf_gpio_get;
	priv->gpio_ctrl.set = smf_gpio_set;
	priv->gpio_ctrl.base = S6100_SMF_GPIO_CTRL_BASE;
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

	rc = smf_mb_reg_rd(priv->smf_mb_map, S6100_SMF_MB_PROTO_VER, &version);
	if (rc)
		return rc;
	dev_info(&pdev->dev, "smf mb protocol version: 0x%x\n", version);

	rc = smf_mb_reg_rd(priv->smf_mb_map, S6100_SMF_MB_FIRMWARE_VER, &version);
	if (rc)
		return rc;
	dev_info(&pdev->dev, "smf mb firmware version: %d.%d\n",
		 (version >> 6) & 0x3,
		 version & 0x3F);

	rc = sysfs_create_groups(&pdev->dev.kobj, smf_attr_groups);
	if (rc != 0) {
		dev_err(&pdev->dev,
			"problems setting up smf sysfs attr groups (%d)\n", rc);
		return rc;
	}

	rc = probe_temperature_system(priv);
	if (rc != 0) {
		dev_err(&pdev->dev,
			"problems setting up temperature sensors (%d)\n", rc);
		return rc;
	}

	rc = probe_fan_system(priv);
	if (rc != 0) {
		dev_err(&pdev->dev,
			"problems setting up fan FRU devices (%d)\n", rc);
		return rc;
	}

	rc = probe_psu_system(priv);
	if (rc != 0) {
		dev_err(&pdev->dev,
			"problems setting up PSU FRU devices (%d)\n", rc);
		return rc;
	}

	rc = probe_io_modules(priv);
	if (rc != 0) {
		dev_err(&pdev->dev,
			"problems setting up IO module devices (%d)\n", rc);
		return rc;
	}

	if (rc == 0)
		dev_info(&pdev->dev, "device probed ok\n");

	return rc;
}

/**
 * s6100_smf_remove()
 *
 * s6100_smf device clean-up routine.
 */
static int s6100_smf_remove(struct platform_device *pdev)
{
	struct s6100_smf_drv_priv *priv = platform_get_drvdata(pdev);
	int i;

	gpiochip_remove(&priv->gpio_ctrl);

	for (i = 0; i < priv->num_fan_frus; i++)
		if (priv->fan_frus[i])
			platform_device_unregister(priv->fan_frus[i]);

	for (i = 0; i < priv->num_psu_frus; i++)
		if (priv->psu_frus[i])
			platform_device_unregister(priv->psu_frus[i]);

	sysfs_remove_groups(&pdev->dev.kobj, smf_attr_groups);

	for (i = 0; i < NUM_IO_MODULES; i++) {
		if (priv->mod_frus[i])
			platform_device_unregister(priv->mod_frus[i]);
		if (priv->mod_i2c_data[i].at24_client)
			i2c_unregister_device(priv->mod_i2c_data[i].at24_client);
		if (priv->mod_i2c_data[i].cpld_client)
			i2c_unregister_device(priv->mod_i2c_data[i].cpld_client);
	}

	dev_info(&pdev->dev, "device removed\n");
	return 0;
}

static struct platform_driver s6100_smf_driver = {
	.driver = {
		.name  = S6100_SMF_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.remove = s6100_smf_remove,
	.id_table = s6100_smf_ids,
};

module_platform_driver_probe(s6100_smf_driver, s6100_smf_probe);

MODULE_AUTHOR("Curt Brune <curt@cumulusnetworks.com");
MODULE_DESCRIPTION("Smartfusion Driver for DELL S6100");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
