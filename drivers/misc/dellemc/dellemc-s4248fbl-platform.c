/*
 * Copyright (c) 2018, 2020 Cumulus Networks, Inc.  All Rights Reserved.
 *
 * dellemc_s4248fbl_platform.c - Dell EMC S4248FBL-C2538 Platform Support.
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

/* This is modelled after DellEMC-S5048F driver */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/i2c-ismt.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/i2c-mux-gpio.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/at24.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"

#include "dellemc-s4248fbl.h"
#include "dellemc-s4248fbl-smf.h"

#define DRIVER_NAME	   "dellemc_s4248fbl_platform"
#define DRIVER_VERSION	   "1.2"

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
 * For eeproms, including ones in the sff transceivers,
 * we specify the label, i2c address, size, and some flags,
 * all done in mk*_eeprom() macros.  The label is the string
 * that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 *
 */

/*
 * Define a custom pca9548 macro that takes eight individual bus numbers
 * because why would you want the bus numbers to be in numerical order?
 *
 */

#define mk_custom_pca9548(_name, _bus0, _bus1, _bus2, _bus3, _bus4, _bus5, \
			  _bus6, _bus7, _deselect_on_exit) \
	static struct pca954x_platform_mode _name##_platform_modes[] = { \
	    mk_pca954x_platform_mode(_bus0, _deselect_on_exit), \
	    mk_pca954x_platform_mode(_bus1, _deselect_on_exit), \
	    mk_pca954x_platform_mode(_bus2, _deselect_on_exit), \
	    mk_pca954x_platform_mode(_bus3, _deselect_on_exit), \
	    mk_pca954x_platform_mode(_bus4, _deselect_on_exit), \
	    mk_pca954x_platform_mode(_bus5, _deselect_on_exit), \
	    mk_pca954x_platform_mode(_bus6, _deselect_on_exit), \
	    mk_pca954x_platform_mode(_bus7, _deselect_on_exit), \
	}; \
	mk_pca954x_platform_data(_name)

mk_pca9547(switch1, I2C_SWITCH1_BUS0, 1);
mk_pca9548(mux2, I2C_MUX2_BUS0, 1);

mk_custom_pca9548(mux4, I2C_MUX4_BUS3, I2C_MUX4_BUS1, I2C_MUX4_BUS4,
		  I2C_MUX4_BUS0, I2C_MUX4_BUS5, I2C_MUX4_BUS2, I2C_MUX4_BUS6,
		  I2C_MUX4_BUS7, 1);
mk_custom_pca9548(mux5, I2C_MUX5_BUS0, I2C_MUX5_BUS1, I2C_MUX5_BUS2,
		  I2C_MUX5_BUS3, I2C_MUX5_BUS7, I2C_MUX5_BUS6, I2C_MUX5_BUS5,
		  I2C_MUX5_BUS4, 1);
mk_custom_pca9548(mux6, I2C_MUX6_BUS4, I2C_MUX6_BUS5, I2C_MUX6_BUS6,
		  I2C_MUX6_BUS7, I2C_MUX6_BUS0, I2C_MUX6_BUS1, I2C_MUX6_BUS3,
		  I2C_MUX6_BUS2, 1);
mk_custom_pca9548(mux8, I2C_MUX8_BUS4, I2C_MUX8_BUS5, I2C_MUX8_BUS6,
		  I2C_MUX8_BUS7, I2C_MUX8_BUS0, I2C_MUX8_BUS1, I2C_MUX8_BUS2,
		  I2C_MUX8_BUS3, 1);
mk_custom_pca9548(mux9, I2C_MUX9_BUS3, I2C_MUX9_BUS2, I2C_MUX9_BUS1,
		  I2C_MUX9_BUS0, I2C_MUX9_BUS4, I2C_MUX9_BUS5, I2C_MUX9_BUS6,
		  I2C_MUX9_BUS7, 1);
mk_custom_pca9548(mux10, I2C_MUX10_BUS0, I2C_MUX10_BUS1, I2C_MUX10_BUS2,
		  I2C_MUX10_BUS3, I2C_MUX10_BUS7, I2C_MUX10_BUS6,
		  I2C_MUX10_BUS5, I2C_MUX10_BUS4, 1);

mk_eeprom(spd1,	 50, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);

mk_eeprom(eeprom0, 50, 256, AT24_FLAG_IRUGO);

mk_eeprom(board,   50, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom2, 51, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom3, 52, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom4, 53, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom5, 54, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom6, 55, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom7, 56, 256, AT24_FLAG_IRUGO);
mk_eeprom(eeprom8, 57, 256, AT24_FLAG_IRUGO);

mk_port_eeprom(port1,  50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port2,  50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port3,  50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port4,  50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port5,  50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port6,  50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port7,  50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port8,  50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port9,  50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port10, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port11, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port12, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port13, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port14, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port15, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port16, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port17, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port18, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port19, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port20, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port21, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port22, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port23, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port24, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port25, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port26, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port27, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port28, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port29, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port30, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port31, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port32, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port33, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port34, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port35, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port36, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port37, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port38, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port39, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);
mk_port_eeprom(port40, 50, 512, AT24_FLAG_IRUGO|AT24_FLAG_HOTPLUG);

mk_qsfp_port_eeprom(port41, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port42, 50, 256, SFF_8436_FLAG_IRUGO);

mk_qsfp_port_eeprom(port43, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port44, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port45, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port46, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port47, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port48, 50, 256, SFF_8436_FLAG_IRUGO);

static struct platform_i2c_device_info i2c_devices[] = {
	/* SMBus_0 */
	mk_i2cdev(I2C_I801_BUS,	    "spd",     0x50, &spd1_50_at24),

	/* SMBus_1 */
	mk_i2cdev(I2C_ISMT_BUS,	    "pca9547", 0x70, &switch1_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x50, &board_50_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x51, &eeprom2_51_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x52, &eeprom3_52_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x53, &eeprom4_53_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x54, &eeprom5_54_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x55, &eeprom6_55_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x56, &eeprom7_56_at24),
	mk_i2cdev(I2C_SWITCH1_BUS0, "24c02",   0x57, &eeprom8_57_at24),

	mk_i2cdev(I2C_SWITCH1_BUS2, "pca9548", 0x71, &mux2_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS4, "pca9548", 0x71, &mux4_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS5, "pca9548", 0x71, &mux8_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS5, "pca9548", 0x72, &mux5_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS6, "pca9548", 0x71, &mux6_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS6, "pca9548", 0x72, &mux9_platform_data),
	mk_i2cdev(I2C_SWITCH1_BUS7, "pca9548", 0x72, &mux10_platform_data),

	mk_i2cdev(I2C_MUX2_BUS0,    "24c02",   0x50, &eeprom0_50_at24),
	mk_i2cdev(I2C_MUX2_BUS4,    "cpld",   0x3e, NULL), /* cpld2 */
	mk_i2cdev(I2C_MUX2_BUS5,    "cpld",   0x3e, NULL), /* cpld3 */
	mk_i2cdev(I2C_MUX2_BUS6,    "cpld",   0x3e, NULL), /* cpld4 */
	/* mk_i2cdev(I2C_MUX2_BUS7,    "cpld",	  0x3e, NULL), cpld1 */

	/* QSFP and QSFP28 ports */
	mk_i2cdev(I2C_MUX4_BUS0,    "sff8436", 0x50, &port44_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS1,    "sff8436", 0x50, &port43_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS2,    "sff8436", 0x50, &port46_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS3,    "sff8436", 0x50, &port45_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS4,    "sff8436", 0x50, &port47_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS5,    "sff8436", 0x50, &port48_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS6,    "sff8436", 0x50, &port41_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS7,    "sff8436", 0x50, &port42_50_sff8436),

	/* SFP+ ports 1 - 8 */
	mk_i2cdev(I2C_MUX5_BUS0,    "24c04",   0x50, &port1_50_at24),
	mk_i2cdev(I2C_MUX5_BUS1,    "24c04",   0x50, &port2_50_at24),
	mk_i2cdev(I2C_MUX5_BUS2,    "24c04",   0x50, &port3_50_at24),
	mk_i2cdev(I2C_MUX5_BUS3,    "24c04",   0x50, &port4_50_at24),
	mk_i2cdev(I2C_MUX5_BUS4,    "24c04",   0x50, &port8_50_at24),
	mk_i2cdev(I2C_MUX5_BUS5,    "24c04",   0x50, &port7_50_at24),
	mk_i2cdev(I2C_MUX5_BUS6,    "24c04",   0x50, &port6_50_at24),
	mk_i2cdev(I2C_MUX5_BUS7,    "24c04",   0x50, &port5_50_at24),

	/* SFP+ ports 25 - 32 */
	mk_i2cdev(I2C_MUX6_BUS0,    "24c04",   0x50, &port25_50_at24),
	mk_i2cdev(I2C_MUX6_BUS1,    "24c04",   0x50, &port26_50_at24),
	mk_i2cdev(I2C_MUX6_BUS2,    "24c04",   0x50, &port27_50_at24),
	mk_i2cdev(I2C_MUX6_BUS3,    "24c04",   0x50, &port28_50_at24),
	mk_i2cdev(I2C_MUX6_BUS4,    "24c04",   0x50, &port32_50_at24),
	mk_i2cdev(I2C_MUX6_BUS5,    "24c04",   0x50, &port31_50_at24),
	mk_i2cdev(I2C_MUX6_BUS6,    "24c04",   0x50, &port30_50_at24),
	mk_i2cdev(I2C_MUX6_BUS7,    "24c04",   0x50, &port29_50_at24),

	/* SFP+ ports 9 - 16 */
	mk_i2cdev(I2C_MUX8_BUS0,    "24c04",   0x50, &port9_50_at24),
	mk_i2cdev(I2C_MUX8_BUS1,    "24c04",   0x50, &port10_50_at24),
	mk_i2cdev(I2C_MUX8_BUS2,    "24c04",   0x50, &port11_50_at24),
	mk_i2cdev(I2C_MUX8_BUS3,    "24c04",   0x50, &port12_50_at24),
	mk_i2cdev(I2C_MUX8_BUS4,    "24c04",   0x50, &port15_50_at24),
	mk_i2cdev(I2C_MUX8_BUS5,    "24c04",   0x50, &port16_50_at24),
	mk_i2cdev(I2C_MUX8_BUS6,    "24c04",   0x50, &port14_50_at24),
	mk_i2cdev(I2C_MUX8_BUS7,    "24c04",   0x50, &port13_50_at24),

	/* SFP+ ports  17 - 24 */
	mk_i2cdev(I2C_MUX9_BUS0,    "24c04",   0x50, &port17_50_at24),
	mk_i2cdev(I2C_MUX9_BUS1,    "24c04",   0x50, &port18_50_at24),
	mk_i2cdev(I2C_MUX9_BUS2,    "24c04",   0x50, &port19_50_at24),
	mk_i2cdev(I2C_MUX9_BUS3,    "24c04",   0x50, &port20_50_at24),
	mk_i2cdev(I2C_MUX9_BUS4,    "24c04",   0x50, &port24_50_at24),
	mk_i2cdev(I2C_MUX9_BUS5,    "24c04",   0x50, &port23_50_at24),
	mk_i2cdev(I2C_MUX9_BUS6,    "24c04",   0x50, &port22_50_at24),
	mk_i2cdev(I2C_MUX9_BUS7,    "24c04",   0x50, &port21_50_at24),

	/* SFP+ ports 33 - 40 */
	mk_i2cdev(I2C_MUX10_BUS0,    "24c04",	0x50, &port33_50_at24),
	mk_i2cdev(I2C_MUX10_BUS1,    "24c04",	0x50, &port34_50_at24),
	mk_i2cdev(I2C_MUX10_BUS2,    "24c04",	0x50, &port35_50_at24),
	mk_i2cdev(I2C_MUX10_BUS3,    "24c04",	0x50, &port36_50_at24),
	mk_i2cdev(I2C_MUX10_BUS4,    "24c04",	0x50, &port40_50_at24),
	mk_i2cdev(I2C_MUX10_BUS5,    "24c04",	0x50, &port39_50_at24),
	mk_i2cdev(I2C_MUX10_BUS6,    "24c04",	0x50, &port38_50_at24),
	mk_i2cdev(I2C_MUX10_BUS7,    "24c04",	0x50, &port37_50_at24),
};

/*-----------------------------------------------------------------------------
 *
 * Initialization routines
 *
 */

/* I2C Initialization */
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

static int i2c_init(void)
{
	int ismt_bus;
	int i801_bus;
	int ret;
	int i;

	ismt_bus = cumulus_i2c_find_adapter(ISMT_ADAPTER_NAME);
	if (ismt_bus < 0) {
		pr_err(DRIVER_NAME "could not find iSMT adapter bus\n");
		 ret = -ENODEV;
		 goto err_exit;
	}

	i801_bus = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (i801_bus < 0) {
		pr_err(DRIVER_NAME "could not find i801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	num_cpld_devices = 0;
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == I2C_ISMT_BUS)
			bus = ismt_bus;
		else if (bus == I2C_I801_BUS)
			bus = i801_bus;
		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_devices[i].client = client;
		if (strcmp(i2c_devices[i].board_info.type, "cpld") == 0) {
			cpld_devices[num_cpld_devices] = client;
			num_cpld_devices++;
		}
	}
	return 0;

err_exit:
	i2c_exit();
	return ret;
}

/*-----------------------------------------------------------------------------
 *
 * CPLD Driver
 */

/* Generic CPLD read/write routines */
static int cpld_read(struct i2c_client *client, int addr)
{
	int ret;

	/* CPLD uses 16-bit addressing */
	ret = i2c_smbus_write_byte_data(client, 0x0, addr);
	if (ret < 0) {
		dev_err(&client->dev,
			"16-bit addr write failed for addr: 0x%x\n", addr);
		return ret;
	}
	ret = i2c_smbus_read_byte(client);
	if (ret < 0)
		dev_err(&client->dev, "Read failed for addr: 0x%x\n", addr);
	return ret;
}

static int cpld_write(struct i2c_client *client, int addr, u8 val)
{
	int ret;

	/* CPLD uses 16-bit addressing */
	ret = i2c_smbus_write_word_data(client, 0x0, ((val << 8) | addr));
	if (ret < 0)
		dev_err(&client->dev, "Write failed for addr: 0x%x\n", addr);

	return ret;
}

static int cpld_read_regs(struct device *dev,
			  int reg,
			  u32 *val)
{
	int ret;
	int cpld_id;

	cpld_id = GET_CPLD_ID(reg);
	reg = STRIP_CPLD_IDX(reg);

	if (cpld_id < 0 || cpld_id >= NUM_CPLD_DEVICES) {
		pr_err(DRIVER_NAME "attempt to read invalid CPLD register - cpld: %d, reg: 0x%02X\n",
		       cpld_id, reg);
		return -EINVAL;
	}

	ret = cpld_read(cpld_devices[cpld_id], reg);
	if (ret < 0) {
		pr_err(DRIVER_NAME "I2C read error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, reg);
		return -EINVAL;
	}
	*val = ret;
	return 0;
}

static int cpld_write_regs(struct device *dev,
			   int reg,
			   u32 val)
{
	int ret = 0;
	int cpld_id;

	cpld_id = GET_CPLD_ID(reg);
	reg = STRIP_CPLD_IDX(reg);

	if (cpld_id < 0 || cpld_id >= NUM_CPLD_DEVICES) {
		pr_err(DRIVER_NAME "attempt to write to invalid CPLD register - cpld: %d, reg: 0x%02X\n",
		       cpld_id, reg);
		return -EINVAL;
	}

	ret = cpld_write(cpld_devices[cpld_id], reg, val);

	if (ret) {
		pr_err(DRIVER_NAME "could not write to i2c device addr: 0x%02X, reg: 0x%02X, val: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, reg, val);
	}
	return ret;
}

/* Pre-defined CPLD read/write routines */
static ssize_t cpld_version_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	int ret;
	int major_ver, minor_ver;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = cpld_devices[attr->index];

	ret = cpld_read(client, CPLD_VERSION_REG);
	if (ret < 0)
		return ret;
	major_ver = (ret & CPLD_MAJOR_VER_MASK) >> CPLD_MAJOR_VER_SHIFT;
	minor_ver = (ret & CPLD_MINOR_VER_MASK) >> CPLD_MINOR_VER_SHIFT;
	return sprintf(buf, "%d.%d\n", major_ver, minor_ver);
}

static SENSOR_DEVICE_ATTR_RO(cpld2_version, cpld_version_show, CPLD2_ID);
static SENSOR_DEVICE_ATTR_RO(cpld3_version, cpld_version_show, CPLD3_ID);
static SENSOR_DEVICE_ATTR_RO(cpld4_version, cpld_version_show, CPLD4_ID);

/* CPLD SFP28 read/write routines (these span more than one CPLD register) */
struct reg_offset {
	uint8_t mask;
	uint8_t shift;
};

static struct reg_offset sfp28_offset[] = {
	{ 0xff,	 0 },
	{ 0xff,	 8 },
	{ 0x03, 16 },
	{ 0xff, 18 },
	{ 0xff, 26 },
	{ 0x03, 34 },
	{ 0x0f, 36 },
};

struct sfp28_port_status {
	char name[CPLD_STRING_NAME_SIZE];
	u8 index;
	u8 active_low;
	u8 num_bits;
	u8 num_regs;
	int reg[MAX_REGS];
};

static struct sfp28_port_status sfp28_status[] = {
	{
		.name = "tx_disable",
		.index = 0,
		.active_low = 0,
		.num_bits = 40,
		.num_regs = 7,
		.reg = { CPLD2_SFP28_TXDISABLE_CTRL0_REG,
			 CPLD2_SFP28_TXDISABLE_CTRL1_REG,
			 CPLD2_SFP28_TXDISABLE_CTRL2_REG,
			 CPLD3_SFP28_TXDISABLE_CTRL0_REG,
			 CPLD3_SFP28_TXDISABLE_CTRL1_REG,
			 CPLD3_SFP28_TXDISABLE_CTRL2_REG,
			 CPLD4_SFP28_TXDISABLE_CTRL0_REG, },
	},
	{
		.name = "rs",
		.index = 1,
		.active_low = 0,
		.num_bits = 40,
		.num_regs = 7,
		.reg = { CPLD2_SFP28_RS_CTRL0_REG,
			 CPLD2_SFP28_RS_CTRL1_REG,
			 CPLD2_SFP28_RS_CTRL2_REG,
			 CPLD3_SFP28_RS_CTRL0_REG,
			 CPLD3_SFP28_RS_CTRL1_REG,
			 CPLD3_SFP28_RS_CTRL2_REG,
			 CPLD4_SFP28_RS_CTRL0_REG, },
	},
	{
		.name = "rx_los",
		.index = 2,
		.active_low = 0,
		.num_bits = 40,
		.num_regs = 7,
		.reg = { CPLD2_SFP28_RXLOS_STA0_REG,
			 CPLD2_SFP28_RXLOS_STA1_REG,
			 CPLD2_SFP28_RXLOS_STA2_REG,
			 CPLD3_SFP28_RXLOS_STA0_REG,
			 CPLD3_SFP28_RXLOS_STA1_REG,
			 CPLD3_SFP28_RXLOS_STA2_REG,
			 CPLD4_SFP28_RXLOS_STA0_REG, },
	},
	{
		.name = "tx_fault",
		.index = 3,
		.active_low = 0,
		.num_bits = 40,
		.num_regs = 7,
		.reg = { CPLD2_SFP28_TXFAULT_STA0_REG,
			 CPLD2_SFP28_TXFAULT_STA1_REG,
			 CPLD2_SFP28_TXFAULT_STA2_REG,
			 CPLD3_SFP28_TXFAULT_STA0_REG,
			 CPLD3_SFP28_TXFAULT_STA1_REG,
			 CPLD3_SFP28_TXFAULT_STA2_REG,
			 CPLD4_SFP28_TXFAULT_STA0_REG, },
	},
	{
		.name = "present",
		.index = 4,
		.active_low = 1,
		.num_bits = 40,
		.num_regs = 7,
		.reg = { CPLD2_SFP28_ABS_STA0_REG,
			 CPLD2_SFP28_ABS_STA1_REG,
			 CPLD2_SFP28_ABS_STA2_REG,
			 CPLD3_SFP28_ABS_STA0_REG,
			 CPLD3_SFP28_ABS_STA1_REG,
			 CPLD3_SFP28_ABS_STA2_REG,
			 CPLD4_SFP28_ABS_STA0_REG, },
	},
};

static ssize_t sfp28_show(struct device *dev,
			  struct device_attribute *dattr,
			  char *buf)
{
	int i;
	int res;
	u32 reg_val = 0;
	u64 val = 0;
	struct sfp28_port_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &sfp28_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	for (i = 0; i < target->num_regs; i++) {
		res = cpld_read_regs(dev, target->reg[i], &reg_val);
		val |= (reg_val & (uint64_t)sfp28_offset[i].mask) <<
			(uint64_t)sfp28_offset[i].shift;
	}
	if (target->active_low)
		val = ~val;

	return sprintf(buf, "0x%012llx\n", val & 0xffffffffff);
}

static ssize_t sfp28_store(struct device *dev,
			   struct device_attribute *dattr,
			   const char *buf,
			   size_t count)
{
	int i;
	int res;
	uint64_t val;
	struct sfp28_port_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);

	if (kstrtoll(buf, 0, &val) != 0)
		return -EINVAL;

	/* find the target register */
	target = &sfp28_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	for (i = 0; i < target->num_regs; i++) {
		res = cpld_write_regs(dev, target->reg[i],
				      target->active_low ?
				      ~(val >> sfp28_offset[i].shift)
				      & sfp28_offset[i].mask :
				      (val >> sfp28_offset[i].shift)
				      & sfp28_offset[i].mask);

		if (res < 0) {
			pr_err(DRIVER_NAME "CPLD sfp28 register (%s) write failed\n",
			       target->name);
			return res;
		}
	}
	return count;
}

static SENSOR_DEVICE_ATTR_RW(sfp28_tx_disable, sfp28_show, sfp28_store, 0);
static SENSOR_DEVICE_ATTR_RW(sfp28_rs,	       sfp28_show, sfp28_store, 1);
static SENSOR_DEVICE_ATTR_RO(sfp28_rx_los,     sfp28_show, 2);
static SENSOR_DEVICE_ATTR_RO(sfp28_tx_fault,   sfp28_show, 3);
static SENSOR_DEVICE_ATTR_RO(sfp28_present,    sfp28_show, 4);

/* CPLD QSFP28 read/write routines */
struct qsfp28_port_status {
	char name[CPLD_STRING_NAME_SIZE];
	u8 index;
	u8 active_low;
	int reg;
};

static struct qsfp28_port_status qsfp28_status[] = {
	{
		.name = "reset",
		.index = 0,
		.active_low = 1,
		.reg = CPLD4_QSFP28_RESET_CTRL0_REG,
	},
	{
		.name = "lpmod",
		.index = 1,
		.active_low = 0,
		.reg = CPLD4_QSFP28_LPMOD_CTRL0_REG,
	},
	{
		.name = "interrupt",
		.index = 2,
		.active_low = 1,
		.reg = CPLD4_QSFP28_INT_STA0_REG,
	},
	{
		.name = "present",
		.index = 3,
		.active_low = 1,
		.reg = CPLD4_QSFP28_ABS_STA0_REG,
	},
};

static ssize_t qsfp28_show(struct device *dev,
			   struct device_attribute *dattr,
			   char *buf)
{
	int res;
	int reg_val = 0;
	struct qsfp28_port_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &qsfp28_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	res = cpld_read_regs(dev, target->reg, &reg_val);

	if (target->active_low)
		reg_val = ~reg_val;

	return sprintf(buf, "0x%02x\n", reg_val & 0xff);
}

static ssize_t qsfp28_store(struct device *dev,
			    struct device_attribute *dattr,
			    const char *buf,
			    size_t count)
{
	int res;
	uint64_t val;
	struct qsfp28_port_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);

	if (kstrtoll(buf, 0, &val) != 0)
		return -EINVAL;

	/* find the target register */
	target = &qsfp28_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	res = cpld_write_regs(dev, target->reg, target->active_low ?
			      ~val & 0xff : val & 0xff);

	if (res < 0) {
		pr_err(DRIVER_NAME "CPLD qsfp28 register (%s) write failed\n",
		       target->name);
		return res;
	}
	return count;
}

static SENSOR_DEVICE_ATTR_RW(qsfp28_reset,     qsfp28_show, qsfp28_store, 0);
static SENSOR_DEVICE_ATTR_RW(qsfp28_lpmod,     qsfp28_show, qsfp28_store, 1);
static SENSOR_DEVICE_ATTR_RO(qsfp28_interrupt, qsfp28_show,		  2);
static SENSOR_DEVICE_ATTR_RO(qsfp28_present,   qsfp28_show,		  3);

/* sysfs registration */
static struct attribute *cpld_attrs[] = {
	&sensor_dev_attr_cpld2_version.dev_attr.attr,
	&sensor_dev_attr_cpld3_version.dev_attr.attr,
	&sensor_dev_attr_cpld4_version.dev_attr.attr,

	&sensor_dev_attr_sfp28_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp28_rs.dev_attr.attr,
	&sensor_dev_attr_sfp28_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp28_tx_fault.dev_attr.attr,
	&sensor_dev_attr_sfp28_present.dev_attr.attr,

	&sensor_dev_attr_qsfp28_reset.dev_attr.attr,
	&sensor_dev_attr_qsfp28_lpmod.dev_attr.attr,
	&sensor_dev_attr_qsfp28_interrupt.dev_attr.attr,
	&sensor_dev_attr_qsfp28_present.dev_attr.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

/*-----------------------------------------------------------------------------
 *
 * CPLD Initialization
 */

static int cpld_probe(struct platform_device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->dev.kobj, &cpld_attr_group);
	if (ret)
		pr_err(DRIVER_NAME "sysfs_cpld_driver_group failed for cpld driver\n");

	return ret;
}

static int cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver cpld_driver = {
	.driver = {
		.name = "dellemc_s4248fbl_cpld",
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
		pr_err(DRIVER_NAME "incorrect number of CPLD devices (%d); expected: %d\n",
		       num_cpld_devices, NUM_CPLD_DEVICES);
		return -ENODEV;
	}

	ret = platform_driver_register(&cpld_driver);
	if (ret) {
		pr_err(DRIVER_NAME "platform_driver_register() failed for cpld device\n");
		return ret;
	}

	cpld_device = platform_device_alloc("dellemc_s4248fbl_cpld", 0);
	if (!cpld_device) {
		pr_err(DRIVER_NAME "platform_device_alloc failed for cpld device\n");
		platform_driver_unregister(&cpld_driver);
		return -ENOMEM;
	}

	ret = platform_device_add(cpld_device);
	if (ret) {
		pr_err(DRIVER_NAME "platform_device_add failed for cpld device.\n");
		platform_device_put(cpld_device);
		return ret;
	}

	pr_info(DRIVER_NAME " loaded\n");
	return 0;
}

static void cpld_exit(void)
{
	platform_driver_unregister(&cpld_driver);
	platform_device_unregister(cpld_device);
	pr_info(DRIVER_NAME " driver unloaded\n");
}

/*-----------------------------------------------------------------------------
 *
 * Module init/exit
 */

static int __init dellemc_s4248fbl_init(void)
{
	int ret = 0;

	ret = i2c_init();
	if (ret) {
		pr_err(DRIVER_NAME "I2C subsystem initialization failed\n");
		return ret;
	}

	ret = cpld_init();
	if (ret) {
		pr_err(DRIVER_NAME "CPLD initialization failed\n");
		i2c_exit();
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit dellemc_s4248fbl_exit(void)
{
	cpld_exit();
	i2c_exit();
	pr_err(DRIVER_NAME " driver unloaded\n");
}

module_init(dellemc_s4248fbl_init);
module_exit(dellemc_s4248fbl_exit);

MODULE_AUTHOR("Manohar K C (manu@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC S4248FBL Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
