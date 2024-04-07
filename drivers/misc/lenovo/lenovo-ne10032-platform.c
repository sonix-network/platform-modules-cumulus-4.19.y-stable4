// SPDX-License-Identifier: GPL-2.0+
/*
 * Lenovo NE10032 platform driver
 *
 * Copyright (C) 2018, 2019 Cumulus Networks, Inc.  All Rights Reserved.
 * Author: David Yen (dhyen@cumulusnetworks.com)
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
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/sysfs.h>
#include <linux/hwmon-sysfs.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "platform-bitfield.h"
#include "lenovo-ne10032.h"

#define DRIVER_NAME	"lenovo_ne10032_platform"
#define DRIVER_VERSION	"2.0"

/*
 * The platform has one i801 adapter.
 *
 * The i801 is connected to the following on the CPU board:
 *
 *    so-dimm-0 eeprom (0x52)
 *    so-dimm-0 temp (0x1a)
 *    so-dimm-1 eeprom (0x53)
 *    so-dimm-1 temp (0x1b)
 *    fru id eeprom (0x54)
 *    g751 (lm75) temp (0x4f)
 *    6c49420d clock buffer (0x69)
 *    6v61036ndg clock buffer (0x6c)
 *    isl90727 (0x2e)
 *    isl90727 (0x3e)
 *    master cpld (0x6f)
 *
 * The i801 is connected to the following on the main switch board:
 *
 *    onie eeprom (0x56)
 *    power cpld (0x5e)
 *    pca9548#0 8-channel mux (0x70)
 *	 0 fru eeprom (0x51)
 *	 1 <none>
 *	 2 tmp75 hot spot (0x4d)
 *	 3 tmp75 ambient (0x4c)
 *	 4 pca9545#0 4-channel mux (0x71)
 *	    0 board eeprom (0x51)
 *	    1 psu#1 (0x59)
 *	      psu#1 eeprom (0x51)
 *	    2 psu#2 (0x59)
 *	      psu#2 eeprom (0x51)
 *	    3 <none>
 *	 5 <none>
 *	 6 pca9548#1 8-channel mux (0x72)
 *	    0 qsfp 1-8 eeprom (0x50)
 *	      qsfp 1-8 port (0x68)
 *	      port cpld0 (0x5f)
 *	    1 qsfp 9-16 eeprom (0x50)
 *	      qsfp 9-16 port (0x68)
 *	      port cpld1 (0x5f)
 *	    2 qsfp 17-24 eeprom (0x50)
 *	      qsfp 17-24 port (0x68)
 *	      port cpld2 (0x5f)
 *	    3 qsfp 25-32 eeprom (0x50)
 *	      qsfp 25-32 port (0x68)
 *	      port cpld3 (0x5f)
 *	    4 <none>
 *	    5 <none>
 *	    6 <none>
 *	    7 pca9548#2 8-channel mux (0x73)
 *	       0 fan#1 eeprom (0x57)
 *	       1 fan#2 eeprom (0x57)
 *	       2 fan#3 eeprom (0x57)
 *	       3 fan#4 eeprom (0x57)
 *	       4 fan#5 eeprom (0x57)
 *	       5 fan#6 eeprom (0x57)
 *	       6 <none>
 *	       7 <none>
 *	 7 system cpld (0x5f)
 */

enum {
	I2C_I801_BUS = -1,

	/* top-level pca9548#0 mux */
	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,
	I2C_MUX1_BUS4,
	I2C_MUX1_BUS5,
	I2C_MUX1_BUS6,
	I2C_MUX1_BUS7,

	/* 2nd-level pca9545#0 mux */
	I2C_MUX2_BUS0 = 20,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,

	/* 2nd-level pca9548#1 mux */
	I2C_MUX3_BUS0 = 30,
	I2C_MUX3_BUS1,
	I2C_MUX3_BUS2,
	I2C_MUX3_BUS3,
	I2C_MUX3_BUS4,
	I2C_MUX3_BUS5,
	I2C_MUX3_BUS6,
	I2C_MUX3_BUS7,

	/* 3rd-level pca9548#2 mux */
	I2C_MUX4_BUS0 = 40,
	I2C_MUX4_BUS1,
	I2C_MUX4_BUS2,
	I2C_MUX4_BUS3,
	I2C_MUX4_BUS4,
	I2C_MUX4_BUS5,
	I2C_MUX4_BUS6,
	I2C_MUX4_BUS7,

	/* CPLD muxes */
	I2C_CPLD_MUX0 = 50,
	I2C_CPLD_MUX1,
	I2C_CPLD_MUX2,
	I2C_CPLD_MUX3,

	CPLD_MUX_PORT1 = 61,
	CPLD_MUX_PORT2 = 62,
	CPLD_MUX_PORT3 = 63,
	CPLD_MUX_PORT4 = 64,
	CPLD_MUX_PORT5 = 65,
	CPLD_MUX_PORT6 = 66,
	CPLD_MUX_PORT7 = 67,
	CPLD_MUX_PORT8 = 68,
	CPLD_MUX_PORT9 = 69,
	CPLD_MUX_PORT10 = 70,
	CPLD_MUX_PORT11 = 71,
	CPLD_MUX_PORT12 = 72,
	CPLD_MUX_PORT13 = 73,
	CPLD_MUX_PORT14 = 74,
	CPLD_MUX_PORT15 = 75,
	CPLD_MUX_PORT16 = 76,
	CPLD_MUX_PORT17 = 77,
	CPLD_MUX_PORT18 = 78,
	CPLD_MUX_PORT19 = 79,
	CPLD_MUX_PORT20 = 80,
	CPLD_MUX_PORT21 = 81,
	CPLD_MUX_PORT22 = 82,
	CPLD_MUX_PORT23 = 83,
	CPLD_MUX_PORT24 = 84,
	CPLD_MUX_PORT25 = 85,
	CPLD_MUX_PORT26 = 86,
	CPLD_MUX_PORT27 = 87,
	CPLD_MUX_PORT28 = 88,
	CPLD_MUX_PORT29 = 89,
	CPLD_MUX_PORT30 = 90,
	CPLD_MUX_PORT31 = 91,
	CPLD_MUX_PORT32 = 92,
};

/*
 * The list of i2c devices and their bus connections for this platform.
 *
 * First we construct the necessary data struction for each device,
 * using the method specific to the device type.  Then we put them
 * all together in a big table (see i2c_devices below).
 *
 * For muxes, we specify the starting bus number for the block of ports,
 * using the magic mk_pca954*() macros.
 *
 * For eeproms, including ones in the qsfp+ transceivers,
 * we specify the label, i2c address, size, and some flags,
 * all done in mk*_eeprom() macros.  The label is the string
 * that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 *
 * See the comment below for gpio.
 */

mk_pca9548(mux1, I2C_MUX1_BUS0, 1);
mk_pca9545(mux2, I2C_MUX2_BUS0, 1);
mk_pca9548(mux3, I2C_MUX3_BUS0, 1);
mk_pca9548(mux4, I2C_MUX4_BUS0, 1);

mk_eeprom(fruid, 54, 256, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);
mk_eeprom(board, 56, 256, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);
mk_eeprom(fru,	 51, 256, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);
mk_eeprom(mgmt,	 51, 256, AT24_FLAG_IRUGO);

mk_eeprom(psu1,	 51, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu2,	 51, 256, AT24_FLAG_IRUGO);

mk_eeprom(fan1,	 57, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan2,	 57, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan3,	 57, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan4,	 57, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan5,	 57, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan6,	 57, 256, AT24_FLAG_IRUGO);

mk_qsfp_port_eeprom(port1,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port2,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port3,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port4,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port5,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port6,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port7,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port8,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port9,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port10, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port11, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port12, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port13, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port14, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port15, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port16, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port17, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port18, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port19, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port20, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port21, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port22, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port23, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port24, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port25, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port26, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port27, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port28, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port29, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port30, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port31, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port32, 50, 256, SFF_8436_FLAG_IRUGO);

/*
 * Main i2c device table
 *
 * We use the mk_i2cdev() macro to construct the entries.
 * Each entry is a bus number and a i2c_board_info.
 * The i2c_board_info structure specifies the device type, address,
 * and platform data specific to the device type.
 */

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_I801_BUS,	 "24c02",   0x54, &fruid_54_at24),
	mk_i2cdev(I2C_I801_BUS,	 "lm75",    0x4f, NULL),
	mk_i2cdev(I2C_I801_BUS,	 "cpld",    0x6f, NULL), /* Master CPLD */

	mk_i2cdev(I2C_I801_BUS,	 "24c02",   0x56, &board_56_at24),
	mk_i2cdev(I2C_I801_BUS,	 "cpld",    0x5e, NULL), /* Power CPLD */
	mk_i2cdev(I2C_I801_BUS,	 "pca9548", 0x70, &mux1_platform_data),

	/* devices on mux1 */
	mk_i2cdev(I2C_MUX1_BUS0, "24c02",   0x51, &fru_51_at24),
	mk_i2cdev(I2C_MUX1_BUS2, "tmp75",   0x4d, NULL),
	mk_i2cdev(I2C_MUX1_BUS3, "tmp75",   0x4c, NULL),
	mk_i2cdev(I2C_MUX1_BUS4, "pca9545", 0x71, &mux2_platform_data),
	mk_i2cdev(I2C_MUX1_BUS6, "pca9548", 0x72, &mux3_platform_data),
	mk_i2cdev(I2C_MUX1_BUS7, "cpld",    0x5f, NULL), /* System CPLD */

	/* devices on mux2 */
	mk_i2cdev(I2C_MUX2_BUS0, "24c02",   0x51, &mgmt_51_at24),
	mk_i2cdev(I2C_MUX2_BUS1, "24c02",   0x51, &psu1_51_at24),
	mk_i2cdev(I2C_MUX2_BUS2, "24c02",   0x51, &psu2_51_at24),

	/* devices on mux3 */
	mk_i2cdev(I2C_MUX3_BUS0, "cpld",    0x5f, NULL), /* Port CPLD0 */
	mk_i2cdev(I2C_MUX3_BUS1, "cpld",    0x5f, NULL), /* Port CPLD1 */
	mk_i2cdev(I2C_MUX3_BUS2, "cpld",    0x5f, NULL), /* Port CPLD2 */
	mk_i2cdev(I2C_MUX3_BUS3, "cpld",    0x5f, NULL), /* Port CPLD3 */
	mk_i2cdev(I2C_MUX3_BUS7, "pca9548", 0x73, &mux4_platform_data),

	/* devices on mux4 */
	mk_i2cdev(I2C_MUX4_BUS0, "24c02",   0x57, &fan1_57_at24),
	mk_i2cdev(I2C_MUX4_BUS1, "24c02",   0x57, &fan2_57_at24),
	mk_i2cdev(I2C_MUX4_BUS2, "24c02",   0x57, &fan3_57_at24),
	mk_i2cdev(I2C_MUX4_BUS3, "24c02",   0x57, &fan4_57_at24),
	mk_i2cdev(I2C_MUX4_BUS4, "24c02",   0x57, &fan5_57_at24),
	mk_i2cdev(I2C_MUX4_BUS5, "24c02",   0x57, &fan6_57_at24),
};

/* Additional i2c device table for the port EEPROMS */

static struct platform_i2c_device_info cpld_i2c_devices[] = {
	mk_i2cdev(CPLD_MUX_PORT1,  "sff8436", 0x50, &port1_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT2,  "sff8436", 0x50, &port2_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT3,  "sff8436", 0x50, &port3_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT4,  "sff8436", 0x50, &port4_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT5,  "sff8436", 0x50, &port5_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT6,  "sff8436", 0x50, &port6_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT7,  "sff8436", 0x50, &port7_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT8,  "sff8436", 0x50, &port8_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT9,  "sff8436", 0x50, &port9_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT10, "sff8436", 0x50, &port10_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT11, "sff8436", 0x50, &port11_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT12, "sff8436", 0x50, &port12_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT13, "sff8436", 0x50, &port13_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT14, "sff8436", 0x50, &port14_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT15, "sff8436", 0x50, &port15_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT16, "sff8436", 0x50, &port16_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT17, "sff8436", 0x50, &port17_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT18, "sff8436", 0x50, &port18_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT19, "sff8436", 0x50, &port19_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT20, "sff8436", 0x50, &port20_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT21, "sff8436", 0x50, &port21_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT22, "sff8436", 0x50, &port22_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT23, "sff8436", 0x50, &port23_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT24, "sff8436", 0x50, &port24_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT25, "sff8436", 0x50, &port25_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT26, "sff8436", 0x50, &port26_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT27, "sff8436", 0x50, &port27_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT28, "sff8436", 0x50, &port28_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT29, "sff8436", 0x50, &port29_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT30, "sff8436", 0x50, &port30_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT31, "sff8436", 0x50, &port31_50_sff8436),
	mk_i2cdev(CPLD_MUX_PORT32, "sff8436", 0x50, &port32_50_sff8436),
};

/* Utility functions for I2C */

static struct i2c_client *cpld_devices[NUM_CPLD_DEVICES];
static int num_cpld_devices;

static void i2c_exit(void)
{
	int i;

	for (i = ARRAY_SIZE(i2c_devices); --i >= 0;) {
		struct i2c_client *c = i2c_devices[i].client;

		if (c) {
			i2c_devices[i].client = NULL;
			i2c_unregister_device(c);
		}
	}
}

static int i801_bus;

static int i2c_init(void)
{
	int i;
	int ret;

	i801_bus = cumulus_i2c_find_adapter("SMBus I801 adapter");
	if (i801_bus < 0) {
		pr_err(DRIVER_NAME ": could not find the i801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	num_cpld_devices = 0;
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == I2C_I801_BUS)
			bus = i801_bus;

		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_devices[i].client = client;
		if (strcmp(i2c_devices[i].board_info.type, "cpld") == 0)
			cpld_devices[num_cpld_devices++] = client;
	}
	return 0;

err_exit:
	i2c_exit();
	return ret;
}

/* bitfield accessor functions */

static int cpld_read_reg(struct device *dev, int reg, int nregs, u32 *val)
{
	int ret;
	int cpld_id = GET_CPLD_ID(reg);

	if (cpld_id < 0 || cpld_id >= NUM_CPLD_DEVICES) {
		pr_err(DRIVER_NAME ": attempt to read invalid CPLD register: 0x%02X\n",
		       reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	ret = i2c_smbus_read_byte_data(cpld_devices[cpld_id], reg);
	if (ret < 0) {
		pr_err(DRIVER_NAME ": CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, reg);
		return -EINVAL;
	}
	*val = ret;
	return 0;
}

static int cpld_write_reg(struct device *dev, int reg, int nregs, u32 val)
{
	int ret = 0;

	int cpld_id = GET_CPLD_ID(reg);

	if (cpld_id < 0 || cpld_id >= NUM_CPLD_DEVICES) {
		pr_err(DRIVER_NAME ": attempt to write to invalid CPLD register: 0x%02X\n",
		       reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	ret = i2c_smbus_write_byte_data(cpld_devices[cpld_id], reg, val);
	if (ret) {
		pr_err(DRIVER_NAME ": CPLD write error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, reg);
	}
	return ret;
}

/* CPLD register bitfields with enum-like values */

#define PLATFORM_LED_AMBER_SLOW_BLINKING "amber_slow_blinking"
#define PLATFORM_LED_BLUE_SLOW_BLINKING	 "blue_slow_blinking"

static const char * const led_color_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_AMBER_SLOW_BLINKING,
	PLATFORM_LED_AMBER_BLINKING,
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_GREEN_SLOW_BLINKING,
	PLATFORM_LED_GREEN_BLINKING,
};

static const char * const rear_led_color_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_GREEN_SLOW_BLINKING,
	PLATFORM_LED_GREEN_BLINKING,
};

static const char * const system_led_color_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_BLUE,
	PLATFORM_LED_BLUE_SLOW_BLINKING,
	PLATFORM_LED_BLUE_BLINKING,
};

static const char * const reset_button_values[] = {
	"not_pressed",
	"reserved",
	"pressed_and_released",
	"pressed_and_held",
};

/* CPLD registers */

mk_bf_ro(cpld, master_cpld_rev,	   MASTER_CPLD_REVISION_REG, 0, 8, NULL, 0);
mk_bf_ro(cpld, power_cpld_rev,	   POWER_CPLD_REVISION_REG, 0, 8, NULL, 0);
mk_bf_ro(cpld, system_cpld_rev,	   SYSTEM_CPLD_REVISION_REG, 0, 8, NULL, 0);
mk_bf_ro(cpld, port_cpld0_rev,	   PORT_CPLD0_REVISION_REG, 0, 8, NULL, 0);
mk_bf_ro(cpld, port_cpld1_rev,	   PORT_CPLD1_REVISION_REG, 0, 8, NULL, 0);
mk_bf_ro(cpld, port_cpld2_rev,	   PORT_CPLD2_REVISION_REG, 0, 8, NULL, 0);
mk_bf_ro(cpld, port_cpld3_rev,	   PORT_CPLD3_REVISION_REG, 0, 8, NULL, 0);

mk_bf_ro(cpld, cpld_tpm_pp,	   MASTER_TPM_CTRL_REG, 2, 1, NULL,
	 BF_COMPLEMENT);
mk_bf_ro(cpld, tpm_present,	   MASTER_TPM_CTRL_REG, 1, 1, NULL,
	 BF_COMPLEMENT);
mk_bf_ro(cpld, tpm_reset,	   MASTER_TPM_CTRL_REG, 0, 1, NULL, 0);
mk_bf_ro(cpld, reset_button,	   SYSTEM_RESET_BUTTON_REG, 0, 2,
	 reset_button_values, 0);

mk_bf_rw(cpld, cpu_reset,	   POWER_RESET_CONTROL_REG, 1, 1, NULL, 0);
mk_bf_rw(cpld, pca9548_0_reset,	   POWER_RESET_CONTROL_REG, 0, 1, NULL, 0);
mk_bf_rw(cpld, bcm56960_reset,	   SYSTEM_RESET_0_REG, 5, 1, NULL, 0);
mk_bf_rw(cpld, bcm53115M_reset,	   SYSTEM_RESET_0_REG, 4, 1, NULL, 0);
mk_bf_rw(cpld, 88e1112_reset,	   SYSTEM_RESET_0_REG, 3, 1, NULL, 0);
mk_bf_rw(cpld, pca9548_2_reset,	   SYSTEM_RESET_0_REG, 2, 1, NULL, 0);
mk_bf_rw(cpld, pca9548_1_reset,	   SYSTEM_RESET_0_REG, 1, 1, NULL, 0);
mk_bf_rw(cpld, pca9545_0_reset,	   SYSTEM_RESET_0_REG, 0, 1, NULL, 0);
mk_bf_rw(cpld, port_cpld3_reset,   SYSTEM_RESET_1_REG, 3, 1, NULL, 0);
mk_bf_rw(cpld, port_cpld2_reset,   SYSTEM_RESET_1_REG, 2, 1, NULL, 0);
mk_bf_rw(cpld, port_cpld1_reset,   SYSTEM_RESET_1_REG, 1, 1, NULL, 0);
mk_bf_rw(cpld, port_cpld0_reset,   SYSTEM_RESET_1_REG, 0, 1, NULL, 0);

mk_bf_ro(cpld, psu_pwr2_present,   POWER_PSU2_STATUS_REG, 2, 1, NULL, 0);
mk_bf_ro(cpld, psu_pwr2_all_ok,	   POWER_PSU2_STATUS_REG, 1, 1, NULL, 0);
mk_bf_rw(cpld, psu_pwr2_power_on,  POWER_PSU2_STATUS_REG, 0, 1, NULL, 0);
mk_bf_ro(cpld, psu_pwr1_present,   POWER_PSU1_STATUS_REG, 2, 1, NULL, 0);
mk_bf_ro(cpld, psu_pwr1_all_ok,	   POWER_PSU1_STATUS_REG, 1, 1, NULL, 0);
mk_bf_rw(cpld, psu_pwr1_power_on,  POWER_PSU1_STATUS_REG, 0, 1, NULL, 0);

mk_bf_ro(cpld, fantray6_present,   POWER_FAN6_STATUS_REG, 1, 1, NULL, 0);
mk_bf_ro(cpld, fantray6_b2f,	   POWER_FAN6_STATUS_REG, 0, 1, NULL, 0);
mk_bf_ro(cpld, fan11_fault,	   POWER_FAN6_STATUS_REG, 2, 1, NULL, 0);
mk_bf_ro(cpld, fan12_fault,	   POWER_FAN6_STATUS_REG, 3, 1, NULL, 0);

mk_bf_ro(cpld, fantray5_present,   POWER_FAN5_STATUS_REG, 1, 1, NULL, 0);
mk_bf_ro(cpld, fantray5_b2f,	   POWER_FAN5_STATUS_REG, 0, 1, NULL, 0);
mk_bf_ro(cpld, fan9_fault,	   POWER_FAN5_STATUS_REG, 2, 1, NULL, 0);
mk_bf_ro(cpld, fan10_fault,	   POWER_FAN5_STATUS_REG, 3, 1, NULL, 0);

mk_bf_ro(cpld, fantray4_present,   POWER_FAN4_STATUS_REG, 1, 1, NULL, 0);
mk_bf_ro(cpld, fantray4_b2f,	   POWER_FAN4_STATUS_REG, 0, 1, NULL, 0);
mk_bf_ro(cpld, fan7_fault,	   POWER_FAN4_STATUS_REG, 2, 1, NULL, 0);
mk_bf_ro(cpld, fan8_fault,	   POWER_FAN4_STATUS_REG, 3, 1, NULL, 0);

mk_bf_ro(cpld, fantray3_present,   POWER_FAN3_STATUS_REG, 1, 1, NULL, 0);
mk_bf_ro(cpld, fantray3_b2f,	   POWER_FAN3_STATUS_REG, 0, 1, NULL, 0);
mk_bf_ro(cpld, fan5_fault,	   POWER_FAN3_STATUS_REG, 2, 1, NULL, 0);
mk_bf_ro(cpld, fan6_fault,	   POWER_FAN3_STATUS_REG, 3, 1, NULL, 0);

mk_bf_ro(cpld, fantray2_present,   POWER_FAN2_STATUS_REG, 1, 1, NULL, 0);
mk_bf_ro(cpld, fantray2_b2f,	   POWER_FAN2_STATUS_REG, 0, 1, NULL, 0);
mk_bf_ro(cpld, fan3_fault,	   POWER_FAN2_STATUS_REG, 2, 1, NULL, 0);
mk_bf_ro(cpld, fan4_fault,	   POWER_FAN2_STATUS_REG, 3, 1, NULL, 0);

mk_bf_ro(cpld, fantray1_present,   POWER_FAN1_STATUS_REG, 1, 1, NULL, 0);
mk_bf_ro(cpld, fantray1_b2f,	   POWER_FAN1_STATUS_REG, 0, 1, NULL, 0);
mk_bf_ro(cpld, fan1_fault,	   POWER_FAN1_STATUS_REG, 2, 1, NULL, 0);
mk_bf_ro(cpld, fan2_fault,	   POWER_FAN1_STATUS_REG, 3, 1, NULL, 0);

mk_bf_rw(cpld, fan_pwm,		   POWER_FAN_PWM_REG, 0, 8, NULL, 0);

mk_bf_rw(cpld, led_stacking,	   SYSTEM_SYSTEM_LED_REG, 4, 3,
	 led_color_values, 0);
mk_bf_rw(cpld, led_psu,		   SYSTEM_SYSTEM_LED_REG, 0, 3,
	 led_color_values, 0);
mk_bf_rw(cpld, led_fan6,	   SYSTEM_FAN_2_LED_REG, 4, 3,
	 led_color_values, 0);
mk_bf_rw(cpld, led_fan5,	   SYSTEM_FAN_2_LED_REG, 0, 3,
	 led_color_values, 0);
mk_bf_rw(cpld, led_fan4,	   SYSTEM_FAN_1_LED_REG, 4, 3,
	 led_color_values, 0);
mk_bf_rw(cpld, led_fan3,	   SYSTEM_FAN_1_LED_REG, 0, 3,
	 led_color_values, 0);
mk_bf_rw(cpld, led_fan2,	   SYSTEM_FAN_0_LED_REG, 4, 3,
	 led_color_values, 0);
mk_bf_rw(cpld, led_fan1,	   SYSTEM_FAN_0_LED_REG, 0, 3,
	 led_color_values, 0);
mk_bf_rw(cpld, led_fan6_rear,	   SYSTEM_FAN_REAR_2_LED_REG, 4, 2,
	 rear_led_color_values, 0);
mk_bf_rw(cpld, led_fan5_rear,	   SYSTEM_FAN_REAR_2_LED_REG, 0, 2,
	 rear_led_color_values, 0);
mk_bf_rw(cpld, led_fan4_rear,	   SYSTEM_FAN_REAR_1_LED_REG, 4, 2,
	 rear_led_color_values, 0);
mk_bf_rw(cpld, led_fan3_rear,	   SYSTEM_FAN_REAR_1_LED_REG, 0, 2,
	 rear_led_color_values, 0);
mk_bf_rw(cpld, led_fan2_rear,	   SYSTEM_FAN_REAR_0_LED_REG, 4, 2,
	 rear_led_color_values, 0);
mk_bf_rw(cpld, led_fan1_rear,	   SYSTEM_FAN_REAR_0_LED_REG, 0, 2,
	 rear_led_color_values, 0);
mk_bf_rw(cpld, led_system,	   SYSTEM_SERVICE_BLUE_LED_REG, 0, 2,
	 system_led_color_values, 0);

mk_bf_ro(cpld, qsfp28_1_8_interrupt,   PORT_QSFP28_1_8_INTERRUPT_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, qsfp28_1_8_present,     PORT_QSFP28_1_8_PRESENT_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_1_8_reset,       PORT_QSFP28_1_8_RESET_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_1_8_lpmode,      PORT_QSFP28_1_8_LPMODE_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_1_8_modsel,      PORT_QSFP28_1_8_MODSEL_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, qsfp28_9_16_interrupt,  PORT_QSFP28_9_16_INTERRUPT_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, qsfp28_9_16_present,    PORT_QSFP28_9_16_PRESENT_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_9_16_reset,      PORT_QSFP28_9_16_RESET_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_9_16_lpmode,     PORT_QSFP28_9_16_LPMODE_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_9_16_modsel,     PORT_QSFP28_9_16_MODSEL_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, qsfp28_17_24_interrupt, PORT_QSFP28_17_24_INTERRUPT_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, qsfp28_17_24_present,   PORT_QSFP28_17_24_PRESENT_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_17_24_reset,     PORT_QSFP28_17_24_RESET_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_17_24_lpmode,    PORT_QSFP28_17_24_LPMODE_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_17_24_modsel,    PORT_QSFP28_17_24_MODSEL_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, qsfp28_25_32_interrupt, PORT_QSFP28_25_32_INTERRUPT_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, qsfp28_25_32_present,   PORT_QSFP28_25_32_PRESENT_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_25_32_reset,     PORT_QSFP28_25_32_RESET_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_25_32_lpmode,    PORT_QSFP28_25_32_LPMODE_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_25_32_modsel,    PORT_QSFP28_25_32_MODSEL_REG, 0, 8,
	 NULL, 0);

/* special case for fan speeds -- need to be multipled by 150 to get rpms */

static int fan_speed_reg[] = {
	POWER_FAN_MIN_SPEED_REG,
	POWER_FAN1_0_SPEED_REG,
	POWER_FAN1_1_SPEED_REG,
	POWER_FAN2_0_SPEED_REG,
	POWER_FAN2_1_SPEED_REG,
	POWER_FAN3_0_SPEED_REG,
	POWER_FAN3_1_SPEED_REG,
	POWER_FAN4_0_SPEED_REG,
	POWER_FAN4_1_SPEED_REG,
	POWER_FAN5_0_SPEED_REG,
	POWER_FAN5_1_SPEED_REG,
	POWER_FAN6_0_SPEED_REG,
	POWER_FAN6_1_SPEED_REG,
};

#define NUM_FAN_SPEED_REGS ARRAY_SIZE(fan_speed_reg)

static ssize_t fan_show(struct device *dev, struct device_attribute *dattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int idx = attr->index;
	int reg;
	int cpld_id;
	int ret;

	if (idx < 0 || idx >= NUM_FAN_SPEED_REGS)
		return sprintf(buf, "Invalid fan speed register index: %d\n",
			       idx);
	reg = fan_speed_reg[idx];
	cpld_id = GET_CPLD_ID(reg);
	reg = STRIP_CPLD_IDX(reg);
	ret = i2c_smbus_read_byte_data(cpld_devices[cpld_id], reg);
	if (ret < 0) {
		pr_err(DRIVER_NAME ": CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, reg);
		return -EINVAL;
	}

	return sprintf(buf, "%d\n", ret * 150);
}

static SENSOR_DEVICE_ATTR(fan_min,     0444, fan_show, NULL, 0);
static SENSOR_DEVICE_ATTR(fan1_input,  0444, fan_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan2_input,  0444, fan_show, NULL, 2);
static SENSOR_DEVICE_ATTR(fan3_input,  0444, fan_show, NULL, 3);
static SENSOR_DEVICE_ATTR(fan4_input,  0444, fan_show, NULL, 4);
static SENSOR_DEVICE_ATTR(fan5_input,  0444, fan_show, NULL, 5);
static SENSOR_DEVICE_ATTR(fan6_input,  0444, fan_show, NULL, 6);
static SENSOR_DEVICE_ATTR(fan7_input,  0444, fan_show, NULL, 7);
static SENSOR_DEVICE_ATTR(fan8_input,  0444, fan_show, NULL, 8);
static SENSOR_DEVICE_ATTR(fan9_input,  0444, fan_show, NULL, 9);
static SENSOR_DEVICE_ATTR(fan10_input, 0444, fan_show, NULL, 10);
static SENSOR_DEVICE_ATTR(fan11_input, 0444, fan_show, NULL, 11);
static SENSOR_DEVICE_ATTR(fan12_input, 0444, fan_show, NULL, 12);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_master_cpld_rev.attr,
	&cpld_power_cpld_rev.attr,
	&cpld_system_cpld_rev.attr,
	&cpld_port_cpld0_rev.attr,
	&cpld_port_cpld1_rev.attr,
	&cpld_port_cpld2_rev.attr,
	&cpld_port_cpld3_rev.attr,

	&cpld_cpld_tpm_pp.attr,
	&cpld_tpm_present.attr,
	&cpld_tpm_reset.attr,
	&cpld_reset_button.attr,

	&cpld_cpu_reset.attr,
	&cpld_pca9548_0_reset.attr,
	&cpld_bcm56960_reset.attr,
	&cpld_bcm53115M_reset.attr,
	&cpld_88e1112_reset.attr,
	&cpld_pca9548_2_reset.attr,
	&cpld_pca9548_1_reset.attr,
	&cpld_pca9545_0_reset.attr,
	&cpld_port_cpld3_reset.attr,
	&cpld_port_cpld2_reset.attr,
	&cpld_port_cpld1_reset.attr,
	&cpld_port_cpld0_reset.attr,

	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr2_all_ok.attr,
	&cpld_psu_pwr2_power_on.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_psu_pwr1_power_on.attr,

	&cpld_fantray6_present.attr,
	&cpld_fantray6_b2f.attr,
	&sensor_dev_attr_fan11_input.dev_attr.attr,
	&cpld_fan11_fault.attr,
	&sensor_dev_attr_fan12_input.dev_attr.attr,
	&cpld_fan12_fault.attr,

	&cpld_fantray5_present.attr,
	&cpld_fantray5_b2f.attr,
	&sensor_dev_attr_fan9_input.dev_attr.attr,
	&cpld_fan9_fault.attr,
	&sensor_dev_attr_fan10_input.dev_attr.attr,
	&cpld_fan10_fault.attr,

	&cpld_fantray4_present.attr,
	&cpld_fantray4_b2f.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&cpld_fan7_fault.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,
	&cpld_fan8_fault.attr,

	&cpld_fantray3_present.attr,
	&cpld_fantray3_b2f.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&cpld_fan5_fault.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&cpld_fan6_fault.attr,

	&cpld_fantray2_present.attr,
	&cpld_fantray2_b2f.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&cpld_fan3_fault.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&cpld_fan4_fault.attr,

	&cpld_fantray1_present.attr,
	&cpld_fantray1_b2f.attr,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&cpld_fan1_fault.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&cpld_fan2_fault.attr,

	&sensor_dev_attr_fan_min.dev_attr.attr,
	&cpld_fan_pwm.attr,

	&cpld_led_stacking.attr,
	&cpld_led_psu.attr,
	&cpld_led_fan2.attr,
	&cpld_led_fan1.attr,
	&cpld_led_fan4.attr,
	&cpld_led_fan3.attr,
	&cpld_led_fan6.attr,
	&cpld_led_fan5.attr,
	&cpld_led_fan2_rear.attr,
	&cpld_led_fan1_rear.attr,
	&cpld_led_fan4_rear.attr,
	&cpld_led_fan3_rear.attr,
	&cpld_led_fan6_rear.attr,
	&cpld_led_fan5_rear.attr,
	&cpld_led_system.attr,

	&cpld_qsfp28_1_8_interrupt.attr,
	&cpld_qsfp28_1_8_present.attr,
	&cpld_qsfp28_1_8_reset.attr,
	&cpld_qsfp28_1_8_lpmode.attr,
	&cpld_qsfp28_1_8_modsel.attr,

	&cpld_qsfp28_9_16_interrupt.attr,
	&cpld_qsfp28_9_16_present.attr,
	&cpld_qsfp28_9_16_reset.attr,
	&cpld_qsfp28_9_16_lpmode.attr,
	&cpld_qsfp28_9_16_modsel.attr,

	&cpld_qsfp28_17_24_interrupt.attr,
	&cpld_qsfp28_17_24_present.attr,
	&cpld_qsfp28_17_24_reset.attr,
	&cpld_qsfp28_17_24_lpmode.attr,
	&cpld_qsfp28_17_24_modsel.attr,

	&cpld_qsfp28_25_32_interrupt.attr,
	&cpld_qsfp28_25_32_present.attr,
	&cpld_qsfp28_25_32_reset.attr,
	&cpld_qsfp28_25_32_lpmode.attr,
	&cpld_qsfp28_25_32_modsel.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

/* CPLD initialization */

static int cpld_probe(struct platform_device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->dev.kobj, &cpld_attr_group);
	if (ret)
		pr_err(DRIVER_NAME ": failed to create sysfs group for cpld driver\n");

	return ret;
}

static int cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver cpld_driver = {
	.driver = {
		.name = "lenovo_ne10032_cpld",
		.owner = THIS_MODULE,
	},
	.probe = cpld_probe,
	.remove = cpld_remove,
};

static struct platform_device *cpld_device;

static int cpld_init(void)
{
	int ret;

	if (num_cpld_devices != NUM_CPLD_DEVICES) {
		pr_err(DRIVER_NAME ": error: number of CPLD devices: %d; expected: %d\n",
		       num_cpld_devices, NUM_CPLD_DEVICES);
		return -ENODEV;
	}

	ret = platform_driver_register(&cpld_driver);
	if (ret) {
		pr_err(DRIVER_NAME ": failed to register CPLD platform driver\n");
		return ret;
	}

	cpld_device = platform_device_alloc("lenovo_ne10032_cpld", 0);
	if (!cpld_device) {
		pr_err(DRIVER_NAME ": failed to allocate CPLD device\n");
		platform_driver_unregister(&cpld_driver);
		return -ENOMEM;
	}

	ret = platform_device_add(cpld_device);
	if (ret) {
		pr_err(DRIVER_NAME ": failed to add CPLD device\n");
		platform_device_put(cpld_device);
		return ret;
	}

	pr_info(DRIVER_NAME ": CPLD driver loaded\n");
	return 0;
}

static int cpld_reset(void)
{
	int cpld_id;
	int reg;

	cpld_id = GET_CPLD_ID(SYSTEM_RESET_1_REG);
	reg = STRIP_CPLD_IDX(SYSTEM_RESET_1_REG);
	return i2c_smbus_write_byte_data(cpld_devices[cpld_id], reg, 0);
}

static void cpld_exit(void)
{
	platform_driver_unregister(&cpld_driver);
	platform_device_unregister(cpld_device);
	pr_err(DRIVER_NAME ": CPLD driver unloaded\n");
}

/* Port mux */

struct mux_data_item {
	int num_clients;
	struct platform_device *mux_dev;
	struct i2c_mux_core *muxc;
	struct i2c_client **mux_client;
};

struct mux_data_struct {
	struct mux_data_item mux_item[NUM_PORT_MUXES];
	int num_port_muxes;
};

static struct mux_data_struct *port_mux_dev;

static int port_mux_select_chan(struct i2c_mux_core *muxc, u32 chan)
{
	u32 reg;
	int val;
	int ret = 0;
	int cpld_id;
	struct i2c_client *my_client;
	struct i2c_adapter *adap;

	chan -= CPLD_MUX_PORT1;

	if (chan < PORT_1_8_SELECT_RANGE) {
		adap = i2c_get_adapter(I2C_MUX3_BUS0);
		reg = PORT_QSFP28_1_8_MODSEL_REG;
		val = 1 << chan;
	} else if (chan < PORT_9_16_SELECT_RANGE) {
		adap = i2c_get_adapter(I2C_MUX3_BUS1);
		reg = PORT_QSFP28_9_16_MODSEL_REG;
		val = 1 << (chan - PORT_1_8_SELECT_RANGE);
	} else if (chan < PORT_17_24_SELECT_RANGE) {
		adap = i2c_get_adapter(I2C_MUX3_BUS2);
		reg = PORT_QSFP28_17_24_MODSEL_REG;
		val = 1 << (chan - PORT_9_16_SELECT_RANGE);
	} else if (chan < PORT_25_32_SELECT_RANGE) {
		adap = i2c_get_adapter(I2C_MUX3_BUS3);
		reg = PORT_QSFP28_25_32_MODSEL_REG;
		val = 1 << (chan - PORT_17_24_SELECT_RANGE);
	} else {
		pr_err(DRIVER_NAME ": invalid mux channel number %u\n", chan);
		return -EINVAL;
	}

	cpld_id = GET_CPLD_ID(reg);
	my_client = cpld_devices[cpld_id];
	reg = STRIP_CPLD_IDX(reg);

	if (adap->algo->master_xfer) {
		struct i2c_msg msg = { 0 };
		char buf[2];

		msg.addr = my_client->addr;
		msg.flags = 0;
		msg.len = 2;
		buf[0] = reg;
		buf[1] = val;
		msg.buf = buf;
		ret = adap->algo->master_xfer(adap, &msg, 1);
	} else {
		union i2c_smbus_data data;

		data.byte = val;
		ret = adap->algo->smbus_xfer(adap, my_client->addr,
					     my_client->flags,
					     I2C_SMBUS_WRITE,
					     reg, I2C_SMBUS_BYTE_DATA, &data);
	}

	i2c_put_adapter(adap);
	return ret;
}

/*
 * All of the port EEPROMs are muxed onto the I2C_I801 bus at address 0x50.
 * The active port is selected by the CPLD_PORT1_16_I2C_SELECT_REG and
 * CPLD_PORT17_32_I2C_SELECT_REG registers.
 *
 * This routine creates I2C mux devices to service these EEPROMs.
 */

static int create_port_mux(int bus, int port_mux_num, int num_ports,
			   int first_port_num, int first_bus_num,
			   const char *mux_name,
			   int (select_function(struct i2c_mux_core *, u32)))

{
	struct i2c_adapter *parent;
	struct mux_data_item *mux_data;
	int ret;
	int i;

	parent = i2c_get_adapter(bus);
	if (!parent) {
		pr_err(DRIVER_NAME ": i2c adapter for %s bus %d not found\n",
		       mux_name, bus);
		return -ENODEV;
	}

	mux_data = &port_mux_dev->mux_item[port_mux_num];

	mux_data->mux_client = kcalloc(num_ports,
				       sizeof(struct i2c_client *),
				       GFP_KERNEL);
	if (!mux_data->mux_client) {
		ret = -ENOMEM;
		goto err_client;
	}

	mux_data->num_clients = num_ports;

	mux_data->mux_dev = platform_device_alloc(mux_name, 0);
	if (!mux_data->mux_dev) {
		pr_err(DRIVER_NAME ": failed to allocate %s platform device\n",
		       mux_name);
		ret = -ENOMEM;
		goto err_device_alloc;
	}

	ret = platform_device_add(mux_data->mux_dev);
	if (ret) {
		pr_err(DRIVER_NAME ": failed to add %s platform device\n",
		       mux_name);
		goto err_device_add;
	}

	mux_data->muxc = i2c_mux_alloc(parent, &mux_data->mux_dev->dev,
				       num_ports, 0, 0,
				       port_mux_select_chan, NULL);
	if (!mux_data->muxc) {
		pr_err(DRIVER_NAME ": failed to allocate mux %s\n", mux_name);
		ret = -ENOMEM;
		goto err_mux_alloc;
	}

	for (i = 0; i < num_ports; i++) {
		int mux_num;
		int port_num;
		int bus_num;
		int cpld_bus;
		struct i2c_board_info board_info;
		struct i2c_client *client;

		mux_num = first_port_num + i - 1;
		port_num = first_port_num + i;
		bus_num = first_bus_num + i;

		ret = i2c_mux_add_adapter(mux_data->muxc, bus_num, bus_num, 0);
		if (ret) {
			pr_err(DRIVER_NAME ": failed to add adapter for bus %u\n",
			       bus_num);
			goto err_add_adapter;
		}

		cpld_bus = cpld_i2c_devices[mux_num].bus;
		board_info = cpld_i2c_devices[mux_num].board_info;
		client = cumulus_i2c_add_client(cpld_bus, &board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			pr_err(DRIVER_NAME ": add CPLD I2C client failed for bus %d: %d\n",
			       bus, ret);
			goto err_exit;
		}
		mux_data->mux_client[i] = client;
	}

	i2c_put_adapter(parent);
	return 0;

err_exit:
	while (--i >= 0)
		if (mux_data->mux_client[i])
			i2c_unregister_device(mux_data->mux_client[i]);
err_add_adapter:
	if (mux_data->muxc)
		i2c_mux_del_adapters(mux_data->muxc);
err_mux_alloc:
err_device_add:
	if (mux_data->mux_dev)
		platform_device_unregister(mux_data->mux_dev);
err_device_alloc:
	kfree(mux_data->mux_client);
err_client:
	return ret;
}

static int port_mux_init(void)
{
	int ret;

	port_mux_dev = kzalloc(sizeof(*port_mux_dev), GFP_KERNEL);
	if (!port_mux_dev) {
		ret = -ENOMEM;
		goto init_err;
	}

	ret = create_port_mux(I2C_MUX3_BUS0, 0, 8, 1, CPLD_MUX_PORT1,
			      "lenovo_ne10032_mux0",
			      port_mux_select_chan);
	if (ret)
		goto init_err;

	ret = create_port_mux(I2C_MUX3_BUS1, 1, 8, 9, CPLD_MUX_PORT9,
			      "lenovo_ne10032_mux1",
			      port_mux_select_chan);
	if (ret)
		goto init_err;

	ret = create_port_mux(I2C_MUX3_BUS2, 2, 8, 17, CPLD_MUX_PORT17,
			      "lenovo_ne10032_mux2",
			      port_mux_select_chan);
	if (ret)
		goto init_err;

	ret = create_port_mux(I2C_MUX3_BUS3, 3, 8, 25, CPLD_MUX_PORT25,
			      "lenovo_ne10032_mux3",
			      port_mux_select_chan);
	if (ret)
		goto init_err;

	return 0;

init_err:
	kfree(port_mux_dev);
	return ret;
}

/* Free all the resources related to the port mux */

static void port_mux_exit(void)
{
	int i, j;
	struct i2c_client *client;
	struct platform_device *dev;

	for (i = 0; i < NUM_PORT_MUXES; i++) {
		for (j = 0; j < port_mux_dev->mux_item[i].num_clients; j++) {
			client = port_mux_dev->mux_item[i].mux_client[j];
			if (client)
				i2c_unregister_device(client);
		}

		if (port_mux_dev->mux_item[i].muxc)
			i2c_mux_del_adapters(port_mux_dev->mux_item[i].muxc);

		dev = port_mux_dev->mux_item[i].mux_dev;
		if (dev)
			platform_device_unregister(dev);

		kfree(port_mux_dev->mux_item[i].mux_client);
	}

	kfree(port_mux_dev);

	pr_err(DRIVER_NAME ": port muxes unloaded\n");
}

/*
 * Module init and exit
 */

static int __init lenovo_ne10032_init(void)
{
	int ret;

	ret = i2c_init();
	if (ret) {
		pr_err("I2C subsystem initialization failed\n");
		return ret;
	}

	ret = cpld_init();
	if (ret) {
		pr_err("CPLD initialization failed\n");
		return ret;
	}

	/* Port CPLDs are reset by default.  Take them out of reset */
	ret = cpld_reset();
	if (ret) {
		pr_err("Unable to reset Port CPLDs\n");
		return ret;
	}

	ret = port_mux_init();
	if (ret) {
		pr_err("Port EEPROM Mux initialization failed\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit lenovo_ne10032_exit(void)
{
	port_mux_exit();
	cpld_exit();
	i2c_exit();
	pr_info(DRIVER_NAME " driver unloaded\n");
}

module_init(lenovo_ne10032_init);
module_exit(lenovo_ne10032_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Lenovo NE10032 platform driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
