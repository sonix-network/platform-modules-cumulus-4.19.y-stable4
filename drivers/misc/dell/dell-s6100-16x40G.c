/*
 * IO module driver for dell_s6100 platforms.
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
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/i2c/sff-8436.h>
#include <linux/i2c/pca954x.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "dell-s6100-platform.h"
#include "dell-s6100-smf.h"
#include "dell-s6100-16x40G.h"

#define DRIVER_VERSION "1.0"

/**
 * s6100_smf_ids -- driver alias names
 */
static const struct platform_device_id s6100_16x40G_ids[] = {
	{ S6100_MOD_16X40G_DRIVER_NAME, 0 },
	{ S6100_MODULE_DRIVER_NAME, 0 },
	{ /* END OF LIST */ }
};
MODULE_DEVICE_TABLE(platform, s6100_16x40G_ids);

/**
 * S6100_16X40G_NUM_QSFPP
 *
 * Number of QSFP+ ports on this type of module
 */
#define S6100_16X40G_NUM_QSFPP  16

/**
 * S6100_16X40G_NUM_PCA9548
 *
 * Number oF PCA9548 i2c muxes on this type of module
 */
#define S6100_16X40G_NUM_PCA9548 2

/**
 * struct qsfp_info - qsfp i2c device info
 * @eeprom_label:  label of QSFP EEPROM device
 * @eeprom_pdata:  EEPROM class platform data, consumes @eeprom_label
 * @sff8436_pdata: sff8436 device platform data, consumes @eeprom_pdata
 * @sff8436_info:  i2c board info, consumes @sff8436_pdata
 * @client:        i2c client pointer for QSFP EEPROM
 *
 * Container for the QSFP port related i2c info and EEPROM info.  This
 * structure helps the driver maintain the lifecycle of the QSFP
 * EEPROM device.
 */

struct qsfp_info {
	char                          eeprom_label[S6100_MAX_EEPROM_NAME_LEN];
	struct eeprom_platform_data   eeprom_pdata;
	struct sff_8436_platform_data sff8436_pdata;
	struct i2c_board_info         sff8436_info;
	struct i2c_client            *client;
};

/**
 * struct pca9548_info - i2c mux related info
 * @pca954x_mode:  platform data for each bus of the mux
 * @pca954x_pdata: pca954x i2c device platform data, consumes @pca954x_mode
 * @pca954x_info:  i2c board info, consumes @pca954x_pdata
 * @client:        i2c client pointer for the i2c mux
 *
 * Container for PCA9548 i2c mux related i2c info.  This structure
 * helps the driver maintain the lifecycle of the PCA9548 mux devices.
 */

struct pca9548_info {
	struct pca954x_platform_mode pca954x_mode[8];
	struct pca954x_platform_data pca954x_pdata;
	struct i2c_board_info        pca954x_info;
	struct i2c_client           *client;
};

/**
 * struct s6100_16x40G_drv_priv -- private driver data
 * @pdev:        Parent platform device
 * @cpld_client: i2c client object for the CPLD device
 * @map:	 Module CPLD i2c register map
 * @gpio_ctrl:	 GPIO controller object
 * @gpio_lock:	 Mutex used to serialize read/modify/write GPIO operations
 * @qsfp:        Array of per-port QSFP info
 * @mux:         Array of per-mux PCA9548 info
 * Contains private data for the 16x40G IO module driver.
 */
struct s6100_16x40G_drv_priv {
	struct platform_device        *pdev;
	struct i2c_client             *cpld_client;
	struct regmap                 *map;
	struct gpio_chip               gpio_ctrl;
	struct mutex                   gpio_lock;
	struct qsfp_info               qsfp[S6100_16X40G_NUM_QSFPP];
	struct pca9548_info            mux[S6100_16X40G_NUM_PCA9548];
};

#define to_cpld_gpio(gc) \
	container_of(gc, struct s6100_16x40G_drv_priv, gpio_ctrl)

/**
 * cpld_read_ranges
 *
 * Array of readable register ranges for the 16x40G CPLD registers.
 *
 * For this chip, the set is the union of the read-only register range
 * and the read-write register range.
 */
static const struct regmap_range cpld_read_ranges[] = {
	regmap_reg_range(S6100_16X40G_CPLD_VERSION,   S6100_16X40G_QSFP_COMBINE),
	regmap_reg_range(S6100_16X40G_QSFP_INT_MASK0, S6100_16X40G_PHY_INT_STA0),
	regmap_reg_range(S6100_16X40G_PHY_INT_MASK0,  S6100_16X40G_PHY_INT_MASK0),
};

/**
 * cpld_wr_ranges
 *
 * Array of writable register ranges for the 16x40G CPLD registers.
 */
static const struct regmap_range cpld_write_ranges[] = {
	regmap_reg_range(S6100_16X40G_SW_SCRATCH,     S6100_16X40G_SW_SCRATCH),
	regmap_reg_range(S6100_16X40G_CPLD_SEP_RST0,  S6100_16X40G_QSFP_LPMOD_CTRL1),
	regmap_reg_range(S6100_16X40G_QSFP_TRIG_MOD,  S6100_16X40G_QSFP_TRIG_MOD),
	regmap_reg_range(S6100_16X40G_QSFP_INT_MASK0, S6100_16X40G_PHY_TRIG_MOD),
	regmap_reg_range(S6100_16X40G_PHY_INT_MASK0,  S6100_16X40G_PHY_INT_MASK0),
};

/**
 * cpld_volatile_ranges
 *
 * Array of regmap volatile ranges for the 16x40G CPLD registers.
 */
static const struct regmap_range cpld_volatile_ranges[] = {
	regmap_reg_range(S6100_16X40G_QSFP_INT_INT0, S6100_16X40G_QSFP_ABS_INT1),
	regmap_reg_range(S6100_16X40G_PHY_INT_INT0,  S6100_16X40G_PHY_INT_INT0),
};

static struct regmap_access_table cpld_write_table = {
	.yes_ranges   = cpld_write_ranges,
	.n_yes_ranges = ARRAY_SIZE(cpld_write_ranges),
};

static struct regmap_access_table cpld_read_table = {
	.yes_ranges   = cpld_read_ranges,
	.n_yes_ranges = ARRAY_SIZE(cpld_read_ranges),
};

static struct regmap_access_table cpld_volatile_table = {
	.yes_ranges   = cpld_volatile_ranges,
	.n_yes_ranges = ARRAY_SIZE(cpld_volatile_ranges),
};

/**
 * cpld_regmap_reg_read()
 * @context: i2c_client for module CPLD registers
 * @reg: register offset
 * @val: pointer to hold register read value
 *
 * Regmap read register callback for the module i2c CPLD registers.
 * Returns 0 on success or negative errno codes from regmap
 * infrastructure.
 */
static int
cpld_regmap_reg_read(void *context, unsigned int reg, unsigned int *val)
{
	struct i2c_client *client = (struct i2c_client *)context;
	uint8_t val8 = 0;
	int rc = 0;

	rc = module_cpld_raw_reg_rd(client, reg, &val8);
	if (rc)
		return rc;
	*val = val8;

	return rc;
}

/**
 * cpld_regmap_reg_write()
 * @context: i2c_client for module CPLD registers
 * @reg: register offset
 * @val: value to write to register
 *
 * regmap write register callback for the module i2c CPLD registers.
 * Returns 0 on success or negative errno codes from regmap
 * infrastructure.
 */
static int
cpld_regmap_reg_write(void *context, unsigned int reg, unsigned int val)
{
	struct i2c_client *client = (struct i2c_client *)context;
	uint8_t offset = reg;
	uint8_t val8 = val;
	int rc = 0;

	/* Dell S6100 CPLD uses 16-bit addressing */
	rc = i2c_smbus_write_word_data(client, 0x0, ((val8 << 8) | offset));
	if (rc)
		dev_err(&client->dev,
			"CPLD i2c_smbus_write_word_data() failed, addr: 0x%02x, register: 0x%02x (%d)\n",
			client->addr, offset, rc);

	return rc;
}

/**
 * s6100_16x40G_cpld_regmap_config
 *
 * regmap configuration for the 16x40G CPLD i2c registers.
 */
static struct regmap_config s6100_16x40G_cpld_regmap_config = {
	.name                 = S6100_MOD_16X40G_DRIVER_NAME,
	.reg_bits             = 8,
	.reg_stride           = 1,
	.pad_bits             = 0,
	.val_bits             = 8,
	.fast_io              = false,
	.max_register         = S6100_16X40G_PHY_INT_MASK0,
	.wr_table             = &cpld_write_table,
	.rd_table             = &cpld_read_table,
	.volatile_table       = &cpld_volatile_table,
	.use_single_rw        = true,
	.can_multi_write      = false,
	.reg_read             = cpld_regmap_reg_read,
	.reg_write            = cpld_regmap_reg_write,
};

#define S6100_16X40G_GPIO_CHIP_LABEL "dell-s6100-16x40G-gpio-%d"
#define S6100_16X40G_GPIOS_PER_REG 8

#define GPIO_DATA(_name, _reg, _bit, _flags, _val) \
	{ .name = __stringify(_name), .reg = _reg, .bit = _bit, \
	  .flags = _flags, .value = _val, }

static const struct s6100_gpio_data gpio_data[] = {
	GPIO_DATA(swm%dp%d-reset,   S6100_16X40G_QSFP_RESET_CTRL0, 0, GPIOF_DIR_OUT | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA(swm%dp%d-lp-mode, S6100_16X40G_QSFP_LPMOD_CTRL0, 0, GPIOF_DIR_OUT, 1),
	GPIO_DATA(swm%dp%d-int,	    S6100_16X40G_QSFP_INT_STA0,	   0, GPIOF_DIR_IN | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA(swm%dp%d-mod-prs, S6100_16X40G_QSFP_ABS_STA0,	   0, GPIOF_DIR_IN | GPIOF_ACTIVE_LOW, 0),
	GPIO_DATA(mod%d-beacon,     S6100_16X40G_MODULE_LED_CTRL,  0, GPIOF_DIR_OUT, 0),
};

#define S6100_16X40G_GPIOS_PER_QSFPP 4
#define S6100_16X40G_NUM_GPIOS ((S6100_16X40G_GPIOS_PER_QSFPP * S6100_16X40G_NUM_QSFPP) + 1)
#define MODULE_BEACON_GPIO (S6100_16X40G_NUM_GPIOS - 1)
#define MODULE_BEACON_GPIO_TYPE 4

/**
 * get_gpio_type()
 * @offset: GPIO number
 *
 * Given a GPIO return the GPIO 'type', which is simply the index into
 * the 'gpio_data' array.
 */
static inline int
get_gpio_type(int offset) {
	if (offset == MODULE_BEACON_GPIO)
		return MODULE_BEACON_GPIO_TYPE;
	else
		return offset / S6100_16X40G_NUM_QSFPP;
}

/**
 * cpld_gpio_get_direction()
 *
 * gpio_chip driver interface callback. Returns the current GPIO
 * signal direction.
 */
static int
cpld_gpio_get_direction(struct gpio_chip *chip, unsigned offset)
{
	struct s6100_16x40G_drv_priv *priv = to_cpld_gpio(chip);
	int gpio_type;

	if (offset >= S6100_16X40G_NUM_GPIOS) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d\n", offset);
		return -EINVAL;
	}

	gpio_type = get_gpio_type(offset);

	return gpio_data[gpio_type].flags & GPIOF_DIR_IN;
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
	struct s6100_16x40G_drv_priv *priv = to_cpld_gpio(chip);
	int gpio_type;

	if (offset >= S6100_16X40G_NUM_GPIOS) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d\n", offset);
		return -EINVAL;
	}

	gpio_type = get_gpio_type(offset);

	return gpio_data[gpio_type].flags & GPIOF_DIR_IN ? 0 : -EINVAL;
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
	struct s6100_16x40G_drv_priv *priv = to_cpld_gpio(chip);
	int gpio_type;

	if (offset >= S6100_16X40G_NUM_GPIOS) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d\n", offset);
		return -EINVAL;
	}

	gpio_type = get_gpio_type(offset);

	return gpio_data[gpio_type].flags & GPIOF_DIR_IN ? -EINVAL : 0;
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
	struct s6100_16x40G_drv_priv *priv = to_cpld_gpio(chip);
	int gpio_type;
	int gpio_index = 0;
	int reg_offset;
	int bit_offset;
	uint8_t val;

	if (offset >= S6100_16X40G_NUM_GPIOS) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d\n", offset);
		return -EINVAL;
	}

	gpio_type = get_gpio_type(offset);
	reg_offset = gpio_data[gpio_type].reg;
	if (gpio_type == MODULE_BEACON_GPIO_TYPE) {
		bit_offset = gpio_data[gpio_type].bit;
	} else {
		gpio_index = offset % S6100_16X40G_NUM_QSFPP;
		reg_offset += gpio_index / S6100_16X40G_GPIOS_PER_REG;
		bit_offset = gpio_index % S6100_16X40G_GPIOS_PER_REG;
	}

	rc = cpld_reg_rd(priv->map, reg_offset, &val);
	if (rc)
		return rc;

	dev_dbg(&priv->pdev->dev,
		"%s(): type: %d, index: %d, reg_offset: %d, bit_offset: %d, reg_val: 0x%x\n",
		__func__, gpio_type, gpio_index, reg_offset, bit_offset, val);

	rc = val & BIT(bit_offset) ? 1 : 0;
	if (gpio_data[gpio_type].flags & GPIOF_ACTIVE_LOW)
		rc = !rc;

	return rc;
}

/**
 * cpld_gpio_set()
 *
 * gpio_chip driver interface callback. For output GPIO signals, sets
 * the current value of the specified GPIO signal.
 */
static void cpld_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	int rc;
	struct s6100_16x40G_drv_priv *priv = to_cpld_gpio(chip);
	int gpio_type;
	int gpio_index = 0;
	int reg_offset;
	int bit_offset;
	uint8_t reg_val;

	if (offset >= S6100_16X40G_NUM_GPIOS) {
		dev_err(&priv->pdev->dev,
			"GPIO index out of bounds: %d\n", offset);
		return;
	}

	gpio_type  = offset / S6100_16X40G_NUM_QSFPP;
	reg_offset = gpio_data[gpio_type].reg;

	if (gpio_type == MODULE_BEACON_GPIO_TYPE) {
		bit_offset = gpio_data[gpio_type].bit;
	} else {
		gpio_index = offset % S6100_16X40G_NUM_QSFPP;
		reg_offset += gpio_index / S6100_16X40G_GPIOS_PER_REG;
		bit_offset = gpio_index % S6100_16X40G_GPIOS_PER_REG;
	}

	if (gpio_data[gpio_type].flags & GPIOF_DIR_IN) {
		dev_warn(&priv->pdev->dev,
			 "Ignoring request to write read-only GPIO: %u", offset);
		return;
	}

	mutex_lock(&priv->gpio_lock);
	rc = cpld_reg_rd(priv->map, reg_offset, &reg_val);
	if (rc) {
		mutex_unlock(&priv->gpio_lock);
		return;
	}

	dev_dbg(&priv->pdev->dev,
		"%s(): type: %d, index: %d, reg_offset: %d, bit_offset: %d, reg_rd: 0x%x\n",
		__func__, gpio_type, gpio_index, reg_offset, bit_offset, reg_val);

	reg_val &= ~BIT(bit_offset);
	if (gpio_data[gpio_type].flags & GPIOF_ACTIVE_LOW)
		value = !value;
	if (value)
		reg_val |= BIT(bit_offset);
	rc = cpld_reg_wr(priv->map, reg_offset, reg_val);
	mutex_unlock(&priv->gpio_lock);

	dev_info(&priv->pdev->dev,
		 "set GPIO: type: %d, index: %d, reg_offset: %d, bit_offset: %d, reg_wr: 0x%x\n",
		 gpio_type, gpio_index, reg_offset, bit_offset, reg_val);
}

/**
 * init_optical_i2c()
 * @priv: 16x40G private driver data
 * @bus:  parent i2c bus
 *
 * Completes the i2c bus hierarchy for accessing the 16 QSFP+ sff8436
 * EEPROMs.
 *
 * The bus hierarchy looks like:
 *
 *
 *                   parent bus
 *                        |
 *                        |
 *         +--------------+-----------+
 *         |                          |
 *         |                          |
 * +-------+---------+        +-------+---------+
 * |  PCA9548-1 0x71 |        |  PCA9548-2 0x72 |
 * +-----------------+        +-----------------+
 *   |  |  ......  |            |  |  ......  |
 *  p1  p2         p8          p9 p10        p16
 *
 *
 * Where p<N> are the QSFP+ ports.  Each QSFP+ has a sff8436
 * EEPROM available at i2c address 0x50.
 *
 */
static int init_optical_i2c(struct s6100_16x40G_drv_priv *priv, int bus)
{
	int rc = 0;
	int i, j;
	struct pca954x_platform_mode *mux_mode;
	struct qsfp_info *qsfp;
	int mux_bus_base = S6100_MODULE_I2C_BUS_BASE +
		(priv->pdev->id * S6100_I2C_BUS_PER_MODULE);

	/* Create i2c MUXes */
	for (i = 0; i < ARRAY_SIZE(priv->mux); i++) {
		for (j = 0; j < ARRAY_SIZE(priv->mux[0].pca954x_mode); j++) {
			mux_mode = priv->mux[i].pca954x_mode + j;
			mux_mode->adap_id = mux_bus_base++;
			mux_mode->deselect_on_exit = 1;
		}
		priv->mux[i].pca954x_pdata.modes = priv->mux[i].pca954x_mode;
		priv->mux[i].pca954x_pdata.num_modes = ARRAY_SIZE(priv->mux[0].pca954x_mode);
		strncpy(priv->mux[i].pca954x_info.type, "pca9548",
			sizeof(priv->mux[i].pca954x_info.type) - 1);
		priv->mux[i].pca954x_info.addr = 0x71 + i;
		priv->mux[i].pca954x_info.platform_data = &priv->mux[i].pca954x_pdata;
		priv->mux[i].client = cumulus_i2c_add_client(bus,
							     &priv->mux[i].pca954x_info);
		if (IS_ERR(priv->mux[i].client)) {
			rc = PTR_ERR(priv->mux[i].client);
			dev_err(&priv->pdev->dev,
				"Problems adding i2c mux device, bus: %d, addr: 0x%02x (%d)\n",
				bus, priv->mux[i].pca954x_info.addr, rc);
			while (--i >= 0)
				i2c_unregister_device(priv->mux[i].client);
			return rc;
		}
	}

	/* Attach a sff8436 device to each mux adapter */
	mux_bus_base = S6100_MODULE_I2C_BUS_BASE + (priv->pdev->id * S6100_I2C_BUS_PER_MODULE);
	for (i = 0; i < ARRAY_SIZE(priv->qsfp); i++) {
		qsfp = priv->qsfp + i;
		snprintf(qsfp->eeprom_label, sizeof(qsfp->eeprom_label) - 1,
			 "portm%dp%d", priv->pdev->id + 1, i + 1);
		qsfp->eeprom_pdata.label = qsfp->eeprom_label;
		qsfp->sff8436_pdata.byte_len = 256;
		qsfp->sff8436_pdata.flags = SFF_8436_FLAG_IRUGO;
		qsfp->sff8436_pdata.page_size = 1;
		qsfp->sff8436_pdata.eeprom_data = &qsfp->eeprom_pdata;
		strncpy(qsfp->sff8436_info.type, "sff8436",
			sizeof(qsfp->sff8436_info.type) - 1);
		qsfp->sff8436_info.addr = 0x50;
		qsfp->sff8436_info.platform_data = &qsfp->sff8436_pdata;
		qsfp->client = cumulus_i2c_add_client(mux_bus_base + i,
						      &qsfp->sff8436_info);
		if (IS_ERR(qsfp->client)) {
			rc = PTR_ERR(qsfp->client);
			dev_err(&priv->pdev->dev,
				"Problems adding sff8436 i2c device, bus: %d, addr: 0x%02x (%d)\n",
				mux_bus_base + i, qsfp->sff8436_info.addr, rc);
			goto error_sff8436;
		}
	}
	return rc;

error_sff8436:
	while (--i >= 0)
		i2c_unregister_device(priv->qsfp[i].client);
	for (i = 0; i < ARRAY_SIZE(priv->mux); i++)
		i2c_unregister_device(priv->mux[i].client);

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
	struct s6100_16x40G_drv_priv *priv = dev_get_drvdata(dev);
	uint8_t val;
	int rc;

	rc = cpld_reg_rd(priv->map, S6100_16X40G_CPLD_VERSION, &val);
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
	struct s6100_16x40G_drv_priv *priv = dev_get_drvdata(dev);
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	uint8_t val;
	char *fmt;
	int rc;

	rc = cpld_reg_rd(priv->map, attr->index, &val);
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
 * status_led_colors[]
 *
 * Simple look-up table, mapping S6100_16X40G_MODULE_LED_CTRL register
 * fields to human readable strings.
 */
static char *status_led_colors[] = {
	PLATFORM_LED_GREEN,
	PLATFORM_LED_GREEN_BLINKING,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_AMBER_BLINKING,
};

/**
 * led_status_show()
 *
 * Return a human readable string representing the current setting of
 * the module status LED.
 */
static ssize_t led_status_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	struct s6100_16x40G_drv_priv *priv = dev_get_drvdata(dev);
	uint8_t val;
	int rc;

	rc = cpld_reg_rd(priv->map, S6100_16X40G_MODULE_LED_CTRL, &val);
	if (rc)
		return rc;

	val &= MODULE_LED_STATUS_MASK;
	val >>= MODULE_LED_STATUS_SHIFT;
	if (val >= ARRAY_SIZE(status_led_colors)) {
		dev_err(&priv->pdev->dev,
			"LED status value out of bounds: %d\n", val);
		return -EINVAL;
	}

	return snprintf(buf, PAGE_SIZE, "%s\n", status_led_colors[val]);
}

/**
 * led_status_store()
 *
 * Set the module status LED from a human readable string.
 */
static ssize_t led_status_store(struct device *dev,
				struct device_attribute *dattr,
				const char *buf, size_t count)
{
	struct s6100_16x40G_drv_priv *priv = dev_get_drvdata(dev);
	uint8_t val;
	int rc;
	int i;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];

	if (sscanf(buf, "%19s", raw) <= 0)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(status_led_colors); i++)
		if (strncmp(raw, status_led_colors[i], count) == 0)
			break;

	if (i == ARRAY_SIZE(status_led_colors))
		return -EINVAL;

	rc = cpld_reg_rd(priv->map, S6100_16X40G_MODULE_LED_CTRL, &val);
	if (rc)
		return rc;

	val &= ~MODULE_LED_STATUS_MASK;
	val |= i << MODULE_LED_STATUS_SHIFT;

	rc = cpld_reg_wr(priv->map, S6100_16X40G_MODULE_LED_CTRL, val);
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
	struct s6100_16x40G_drv_priv *priv = dev_get_drvdata(dev);

	return cumulus_gpio_map_show(dev, &priv->gpio_ctrl, buf);
}

/**
 * struct attribute *misc_system_attrs[]
 *
 * Array of misc CPLD system level sysfs attributes.
 */
static SENSOR_ATTR_DATA_RO_2(cpld_version,   fw_version_show, 0, 0);
static SENSOR_ATTR_DATA_RO_2(module_type,    cpld_u8_show,    S6100_16X40G_MODULE_ID, 'd');
static SENSOR_ATTR_DATA_RW_2(led_mod_status, led_status_show, led_status_store, 0, 0);
static SENSOR_ATTR_DATA_RO_2(gpio_map,       gpio_map_show,   0, 0);
static struct attribute *misc_system_attrs[] = {
	&sensor_dev_attr_cpld_version.dev_attr.attr,
	&sensor_dev_attr_module_type.dev_attr.attr,
	&sensor_dev_attr_led_mod_status.dev_attr.attr,
	&sensor_dev_attr_gpio_map.dev_attr.attr,
	NULL,
};
ATTRIBUTE_GROUPS(misc_system);

/**
 * s6100_16x40G_probe()
 *
 * Top level s6100_16x40G device probe routine.
 */
static int s6100_16x40G_probe(struct platform_device *pdev)
{
	int rc = 0;
	int i, j;
	uint8_t val;
	struct s6100_16x40G_drv_priv *priv;
	struct s6100_module_platform_data *pdata = dev_get_platdata(&pdev->dev);
	char const **names;
	char const *name;

	if (!pdata) {
		dev_err(&pdev->dev, "unable to find module platform data\n");
		return -ENODEV;
	}

	if (!pdata->cpld_client) {
		dev_err(&pdev->dev, "unable to find cpld_client in platform data\n");
		return -ENODEV;
	}

	/* Verify module type matches this driver */
	rc = module_cpld_raw_reg_rd(pdata->cpld_client, S6100_16X40G_MODULE_ID, &val);
	if (rc)
		return rc;
	if (val != S6100_MODULE_ID_16X40G_QSFP)
		return -ENODEV;

	/* Verify slot number */
	rc = module_cpld_raw_reg_rd(pdata->cpld_client, S6100_16X40G_SLOT_ID, &val);
	if (rc)
		return rc;
	if (val != pdev->id) {
		dev_err(&pdev->dev, "device id (%d) does not match slot number: %d\n",
			pdev->id, val);
		return -ENXIO;
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->pdev = pdev;
	platform_set_drvdata(pdev, (void *)priv);

	priv->cpld_client = pdata->cpld_client;

	/* setup regmap */
	priv->map = devm_regmap_init(&pdev->dev, NULL,
				     (void *)priv->cpld_client,
				     &s6100_16x40G_cpld_regmap_config);
	if (IS_ERR(priv->map)) {
		rc = PTR_ERR(priv->map);
		dev_err(&pdev->dev, "unable to create 16x40G cpld regmap: %d\n", rc);
		return rc;
	}

	/* Initialize GPIO controller for I/O signals */
	mutex_init(&priv->gpio_lock);
	priv->gpio_ctrl.label = devm_kasprintf(&pdev->dev, GFP_KERNEL,
					       S6100_16X40G_GPIO_CHIP_LABEL,
					       pdev->id + 1);
	priv->gpio_ctrl.dev   = &pdev->dev;
	priv->gpio_ctrl.owner = THIS_MODULE;
	priv->gpio_ctrl.get_direction = cpld_gpio_get_direction;
	priv->gpio_ctrl.direction_input = cpld_gpio_direction_input;
	priv->gpio_ctrl.direction_output = cpld_gpio_direction_output;
	priv->gpio_ctrl.get = cpld_gpio_get;
	priv->gpio_ctrl.set = cpld_gpio_set;
	priv->gpio_ctrl.base = S6100_MODULE_GPIO_CTRL_BASE +
		(S6100_GPIO_PER_MODULE * pdev->id);
	priv->gpio_ctrl.ngpio = S6100_16X40G_NUM_GPIOS;
	priv->gpio_ctrl.can_sleep = true;
	names = devm_kmalloc(&pdev->dev,
			     sizeof(*priv->gpio_ctrl.names) * priv->gpio_ctrl.ngpio,
			     GFP_KERNEL);
	if (!names)
		return -ENOMEM;
	/* Create the QSFPP GPIO names */
	for (i = 0; i < ARRAY_SIZE(gpio_data); i++) {
		for (j = 0; j < S6100_16X40G_NUM_QSFPP; j++) {
			name = devm_kasprintf(&pdev->dev, GFP_KERNEL,
					      gpio_data[i].name, pdev->id + 1,
					      j + 1);
			if (!name)
				return -ENOMEM;
			names[(i*S6100_16X40G_NUM_QSFPP) + j] = name;
		}
	}

	/* Include the module beacon GPIO name */
	name = devm_kasprintf(&pdev->dev, GFP_KERNEL,
			      gpio_data[MODULE_BEACON_GPIO_TYPE].name, pdev->id + 1);
	if (!name)
		return -ENOMEM;
	names[MODULE_BEACON_GPIO] = name;
	priv->gpio_ctrl.names = names;

	rc = gpiochip_add(&priv->gpio_ctrl);
	if (rc) {
		dev_err(&pdev->dev, "gpiochip_add() failed: %d\n", rc);
		return rc;
	}

	rc = sysfs_create_groups(&priv->pdev->dev.kobj, misc_system_groups);
	if (rc)
		dev_err(&priv->pdev->dev,
			"problems creating misc_system sysfs groups\n");

	/* Flesh out optical i2c hierarchy for module EEPROM access */
	rc = init_optical_i2c(priv, pdata->optical_i2c_bus);
	if (rc) {
		dev_err(&pdev->dev, "probe_optical_i2c() failed: %d\n", rc);
		goto error_init_i2c;
	}

	rc = cpld_reg_rd(priv->map, S6100_16X40G_CPLD_VERSION, &val);
	if (rc)
		goto error_reg_rd;

	dev_info(&pdev->dev, "cpld firmware version: %d.%d\n",
		 (val >> 4) & 0xF, val & 0xF);

	if (rc == 0)
		dev_info(&pdev->dev, "device probed ok\n");

	return rc;

error_reg_rd:
	for (i = 0; i < ARRAY_SIZE(priv->qsfp); i++)
		i2c_unregister_device(priv->qsfp[i].client);

	for (i = 0; i < ARRAY_SIZE(priv->mux); i++)
		i2c_unregister_device(priv->mux[i].client);
error_init_i2c:
	gpiochip_remove(&priv->gpio_ctrl);
	sysfs_remove_groups(&priv->pdev->dev.kobj, misc_system_groups);

	return rc;
}

/**
 * s6100_16x40G_remove()
 *
 * s6100_16x40G device clean-up routine.
 */
static int s6100_16x40G_remove(struct platform_device *pdev)
{
	struct s6100_16x40G_drv_priv *priv = platform_get_drvdata(pdev);
	int i;

	gpiochip_remove(&priv->gpio_ctrl);
	sysfs_remove_groups(&priv->pdev->dev.kobj, misc_system_groups);

	for (i = 0; i < ARRAY_SIZE(priv->qsfp); i++)
		i2c_unregister_device(priv->qsfp[i].client);

	for (i = 0; i < ARRAY_SIZE(priv->mux); i++)
		i2c_unregister_device(priv->mux[i].client);

	dev_info(&pdev->dev, "device removed\n");
	return 0;
}

static struct platform_driver s6100_16x40G_driver = {
	.driver = {
		.name  = S6100_MOD_16X40G_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = s6100_16x40G_probe,
	.remove = s6100_16x40G_remove,
	.id_table = s6100_16x40G_ids,
};

module_platform_driver(s6100_16x40G_driver);

MODULE_AUTHOR("Curt Brune <curt@cumulusnetworks.com");
MODULE_DESCRIPTION("16x40G QSFP+ IO Module Driver for DELL S6100");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
