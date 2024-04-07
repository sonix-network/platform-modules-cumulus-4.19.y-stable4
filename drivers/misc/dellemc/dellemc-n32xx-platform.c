// SPDX-License-Identifier: GPL-2.0+
/*
 * Dell N3248PXE Platform Support
 *
 * Copyright (c) 2019, 2020 Cumulus Networks, Inc.  All rights reserved.
 * Author: David Yen <dhyen@cumulusnetworks.com>
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

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c-ismt.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "dellemc-n32xx-n22xx-cplds.h"

#define DRIVER_NAME	"dellemc_n32xx"
#define DRIVER_VERSION	"1.2"

/*
 * Required platform module parameter determines the type of platform:
 *   n3248pxe - 48x10G Cu w/802.3bt type-4 99W PoE, 4 SFP28 w/MACSEC, 2 QSFP28
 *   n3248x   - 48x10G Cu, 4 SFP28, 2 QSFP28
 *   n3248p   - 48x1G Cu w/802.3at type-2 30W PoE, 4 SFP+, 2 QSFP28
 *   n3248te  - 48x1G Cu, 4 SFP+, 2 QSFP28
 *   n3248x   - 24 SFP+, 2 QSFP28
 *   n3224px  - 24x10G Cu w/802.3bt type-4 99W PoE, 4 SFP28, 2 QSFP28
 *   n3224p   - 24x1G Cu w/802.3at type-2 30W PoE, 4 SFP+, 2 QSFP28
 *   n3224t   - 24x1G Cu, 4 SFP+, 2 QSFP28
 *   n3224f   - 24x1G SFP, 4 SFP+, 2 QSFP28
 *   n3208px  - 4x1G Cu, 4x5G Cu w/802.3bt type-4 99W PoE, 2 SFP+
 */
static char *platform;
module_param(platform, charp, 0);
MODULE_PARM_DESC(platform, "Platform type");

enum variant_enum {
	VAR_UNKNOWN = -1,
	VAR_N3248PXE,
	VAR_N3248X,
	VAR_N3248P,
	VAR_N3248TE,
	VAR_N3224X,
	VAR_N3224PX,
	VAR_N3224P,
	VAR_N3224T,
	VAR_N3224F,
	VAR_N3208PX,

	NUM_VARIANTS
};

static char *platform_names[NUM_VARIANTS + 1] = {
	[VAR_N3248PXE] = "n3248pxe",
	[VAR_N3248X]   = "n3248x",
	[VAR_N3248P]   = "n3248p",
	[VAR_N3248TE]  = "n3248te",
	[VAR_N3224X]   = "n3224x",
	[VAR_N3224PX]  = "n3224px",
	[VAR_N3224P]   = "n3224p",
	[VAR_N3224T]   = "n3224t",
	[VAR_N3224F]   = "n3224f",
	[VAR_N3208PX]  = "n3208px",

	[NUM_VARIANTS] = NULL /* end-of-list sentinel */
};

enum {
	/* I2C busses eminating from the Intel SOC */
	I2C_I801_BUS = -10,
	I2C_ISMT_BUS,

	/* top-level mux legs */
	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,
	I2C_MUX1_BUS4,
	I2C_MUX1_BUS5,
	I2C_MUX1_BUS6,

	/* mux legs for the FAN eeproms */
	I2C_FAN_MUX_BUS0 = DELL_N32XX_N22XX_FAN_MUX_BUS_START,
	I2C_FAN_MUX_BUS1,
	I2C_FAN_MUX_BUS2,

	/* mux legs for the PSU eeproms/controllers */
	I2C_PSU_MUX_BUS0 = DELL_N32XX_N22XX_PSU_MUX_BUS_START,
	I2C_PSU_MUX_BUS1,
	I2C_PSU_MUX_BUS2,

	/* mux legs for the port eeproms */
	I2C_PORT_MUX_BUS0 = DELL_N32XX_N22XX_PORT_MUX_BUS_START,
	I2C_PORT_MUX_BUS1,
	I2C_PORT_MUX_BUS2,
	I2C_PORT_MUX_BUS3,
	I2C_PORT_MUX_BUS4,
	I2C_PORT_MUX_BUS5,

	/* more port mux legs, but only on the N3224F */
	I2C_N3224F_PORT_MUX_BUS0 = DELL_N3224F_PORT_MUX_BUS_START,
	I2C_N3224F_PORT_MUX_BUS1,
	I2C_N3224F_PORT_MUX_BUS2,
	I2C_N3224F_PORT_MUX_BUS3,
	I2C_N3224F_PORT_MUX_BUS4,
	I2C_N3224F_PORT_MUX_BUS5,
	I2C_N3224F_PORT_MUX_BUS6,
	I2C_N3224F_PORT_MUX_BUS7,
	I2C_N3224F_PORT_MUX_BUS8,
	I2C_N3224F_PORT_MUX_BUS9,
	I2C_N3224F_PORT_MUX_BUS10,
	I2C_N3224F_PORT_MUX_BUS11,
	I2C_N3224F_PORT_MUX_BUS12,
	I2C_N3224F_PORT_MUX_BUS13,
	I2C_N3224F_PORT_MUX_BUS14,
	I2C_N3224F_PORT_MUX_BUS15,
	I2C_N3224F_PORT_MUX_BUS16,
	I2C_N3224F_PORT_MUX_BUS17,
	I2C_N3224F_PORT_MUX_BUS18,
	I2C_N3224F_PORT_MUX_BUS19,
	I2C_N3224F_PORT_MUX_BUS20,
	I2C_N3224F_PORT_MUX_BUS21,
	I2C_N3224F_PORT_MUX_BUS22,
	I2C_N3224F_PORT_MUX_BUS23
};

/*
 * First we construct the necessary data structures for each device, using the
 * method specific to the device type.  Then we put them all together in a big
 * table (see i2c_devices below).
 *
 * For muxes, we specify the starting bus number for the block of ports,
 * using the magic mk_pca954*() macros.
 *
 * For eeproms, including ones in the qsfp+ transceivers, we specify the label,
 * i2c address, size, and some flags, all done in mk*_eeprom() macros.  The
 * label is the string that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 */

mk_pca9548(mux1, I2C_MUX1_BUS0, 1);

mk_eeprom(board,  50, 256, AT24_FLAG_IRUGO);

mk_eeprom(fan1,   50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan2,   50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan3,   50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu1,   56, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2,   56, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

mk_port_eeprom(port1,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port2,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port3,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port4,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port5,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port6,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port7,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port8,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port9,  50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port10, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port11, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port12, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port13, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port14, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port15, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port16, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port17, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port18, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port19, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port20, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port21, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port22, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port23, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port24, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

mk_port_eeprom(port25, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port26, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port27, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port28, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

mk_qsfp_port_eeprom(port29,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port30,  50, 256, SFF_8436_FLAG_IRUGO);

mk_port_eeprom(port49, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port50, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port51, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port52, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

mk_qsfp_port_eeprom(port53,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port54,  50, 256, SFF_8436_FLAG_IRUGO);

static struct system_cpld_mux_platform_data port_mux_platform_data = {
	.cpld = NULL,
	.dev_name = PORT_MUX_DEVICE_NAME
};

static struct system_cpld_mux_platform_data fan_mux_platform_data = {
	.cpld = NULL,
	.dev_name = FAN_MUX_DEVICE_NAME
};

static struct system_cpld_mux_platform_data psu_mux_platform_data = {
	.cpld = NULL,
	.dev_name = PSU_MUX_DEVICE_NAME
};

/*
 * Common I2C devices - These I2C devices are on all n32xx platforms.
 */
static struct platform_i2c_device_info common_i2c_devices[] = {
	mk_i2cdev(I2C_ISMT_BUS, CPU_CPLD_DRIVER_NAME,    0x31, NULL),
	mk_i2cdev(I2C_ISMT_BUS, SYSTEM_CPLD_DRIVER_NAME, 0x32, NULL),
	mk_i2cdev(I2C_ISMT_BUS, "pca9548", 0x71, &mux1_platform_data),

	mk_i2cdev(I2C_MUX1_BUS0, "24c32", 0x50, &board_50_at24),

	mk_i2cdev(I2C_MUX1_BUS5, "tmp75",         0x48, NULL),
	mk_i2cdev(I2C_MUX1_BUS5, "tmp75",         0x49, NULL),
	mk_i2cdev(I2C_MUX1_BUS5, "tmp75",         0x4a, NULL),
	mk_i2cdev(I2C_MUX1_BUS5, "tmp75",         0x4b, NULL),
	mk_i2cdev(I2C_MUX1_BUS5, "tmp75",         0x4c, NULL),

	mk_i2cdev(I2C_MUX1_BUS6, SYS_CPLD_MUX_DRIVER_NAME, 0x7f,
		  &port_mux_platform_data)
};

#define NUM_COMMON_I2C_DEVICES ARRAY_SIZE(common_i2c_devices)

/*
 * I2C devices which are only on the n3224f
 */
static struct platform_i2c_device_info n3224f_i2c_devices[] = {
	mk_i2cdev(I2C_MUX1_BUS0, PORT_CPLD_DRIVER_NAME, 0x34, NULL),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS0,  "24c02", 0x50, &port1_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS1,  "24c02", 0x50, &port2_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS2,  "24c02", 0x50, &port3_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS3,  "24c02", 0x50, &port4_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS4,  "24c02", 0x50, &port5_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS5,  "24c02", 0x50, &port6_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS6,  "24c02", 0x50, &port7_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS7,  "24c02", 0x50, &port8_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS8,  "24c02", 0x50, &port9_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS9,  "24c02", 0x50, &port10_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS10, "24c02", 0x50, &port11_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS11, "24c02", 0x50, &port12_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS12, "24c02", 0x50, &port13_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS13, "24c02", 0x50, &port14_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS14, "24c02", 0x50, &port15_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS15, "24c02", 0x50, &port16_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS16, "24c02", 0x50, &port17_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS17, "24c02", 0x50, &port18_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS18, "24c02", 0x50, &port19_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS19, "24c02", 0x50, &port20_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS20, "24c02", 0x50, &port21_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS21, "24c02", 0x50, &port22_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS22, "24c02", 0x50, &port23_50_at24),
	mk_i2cdev(I2C_N3224F_PORT_MUX_BUS23, "24c02", 0x50, &port24_50_at24)
};

#define NUM_N3224F_I2C_DEVICES ARRAY_SIZE(n3224f_i2c_devices)

/*
 * I2C devices which are on all platforms except the n3208px
 */
static struct platform_i2c_device_info not_n3208px_i2c_devices[] = {
	mk_i2cdev(I2C_MUX1_BUS2, SYS_CPLD_MUX_DRIVER_NAME,  0x7f,
		  &fan_mux_platform_data),
	mk_i2cdev(I2C_FAN_MUX_BUS0, "24c02", 0x50, &fan1_50_at24),
	mk_i2cdev(I2C_FAN_MUX_BUS1, "24c02", 0x50, &fan2_50_at24),
	mk_i2cdev(I2C_FAN_MUX_BUS2, "24c02", 0x50, &fan3_50_at24),

	mk_i2cdev(I2C_MUX1_BUS3, SYS_CPLD_MUX_DRIVER_NAME,  0x7f,
		  &psu_mux_platform_data),
	mk_i2cdev(I2C_PSU_MUX_BUS0, "24c02", 0x56, &psu1_56_at24),
	mk_i2cdev(I2C_PSU_MUX_BUS0, "pmbus", 0x5e, NULL),
	mk_i2cdev(I2C_PSU_MUX_BUS1, "24c02", 0x56, &psu2_56_at24),
	mk_i2cdev(I2C_PSU_MUX_BUS1, "pmbus", 0x5e, NULL),

	mk_i2cdev(I2C_MUX1_BUS5, "emc2305",       0x2c, NULL),
	mk_i2cdev(I2C_MUX1_BUS5, "tmp75",         0x4f, NULL)
};

#define NUM_NOT_N3208PX_I2C_DEVICES ARRAY_SIZE(not_n3208px_i2c_devices)

/*
 * I2C devices which are only on the n3208px
 */
static struct platform_i2c_device_info n3208px_i2c_devices[] = {
	mk_i2cdev(I2C_MUX1_BUS5, "emc2302",       0x2e, NULL),

	mk_i2cdev(I2C_PORT_MUX_BUS0, "24c02", 0x50, &port9_50_at24),
	mk_i2cdev(I2C_PORT_MUX_BUS1, "24c02", 0x50, &port10_50_at24)
};

#define NUM_N3208PX_I2C_DEVICES ARRAY_SIZE(n3208px_i2c_devices)

/*
 * I2C devices which are only on 24 port platforms
 */
static struct platform_i2c_device_info ports_24_i2c_devices[] = {
	mk_i2cdev(I2C_PORT_MUX_BUS0, "24c02", 0x50, &port25_50_at24),
	mk_i2cdev(I2C_PORT_MUX_BUS1, "24c02", 0x50, &port26_50_at24),
	mk_i2cdev(I2C_PORT_MUX_BUS2, "24c02", 0x50, &port27_50_at24),
	mk_i2cdev(I2C_PORT_MUX_BUS3, "24c02", 0x50, &port28_50_at24),

	mk_i2cdev(I2C_PORT_MUX_BUS4, "sff8436", 0x50, &port29_50_sff8436),
	mk_i2cdev(I2C_PORT_MUX_BUS5, "sff8436", 0x50, &port30_50_sff8436)
};

#define NUM_PORTS_24_I2C_DEVICES ARRAY_SIZE(ports_24_i2c_devices)

/*
 * I2C devices which are only on 48 port platforms
 */
static struct platform_i2c_device_info ports_48_i2c_devices[] = {
	mk_i2cdev(I2C_PORT_MUX_BUS0, "24c02", 0x50, &port49_50_at24),
	mk_i2cdev(I2C_PORT_MUX_BUS1, "24c02", 0x50, &port50_50_at24),
	mk_i2cdev(I2C_PORT_MUX_BUS2, "24c02", 0x50, &port51_50_at24),
	mk_i2cdev(I2C_PORT_MUX_BUS3, "24c02", 0x50, &port52_50_at24),

	mk_i2cdev(I2C_PORT_MUX_BUS4, "sff8436", 0x50, &port53_50_sff8436),
	mk_i2cdev(I2C_PORT_MUX_BUS5, "sff8436", 0x50, &port54_50_sff8436)
};

#define NUM_PORTS_48_I2C_DEVICES ARRAY_SIZE(ports_48_i2c_devices)

static void del_i2c_clients(void)
{
	int i;
	struct i2c_board_info *bi;
	struct system_cpld_mux_platform_data *pd;

	for (i = NUM_PORTS_48_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(ports_48_i2c_devices[i].client);
		ports_48_i2c_devices[i].client = NULL;
	}

	for (i = NUM_PORTS_24_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(ports_24_i2c_devices[i].client);
		ports_24_i2c_devices[i].client = NULL;
	}

	for (i = NUM_N3208PX_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(n3208px_i2c_devices[i].client);
		n3208px_i2c_devices[i].client = NULL;
	}

	for (i = NUM_NOT_N3208PX_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(not_n3208px_i2c_devices[i].client);
		not_n3208px_i2c_devices[i].client = NULL;

		bi = &not_n3208px_i2c_devices[i].board_info;
		if (!strcmp(bi->type, SYS_CPLD_MUX_DRIVER_NAME)) {
			pd = bi->platform_data;
			pd->cpld = NULL;
		}
	}

	for (i = NUM_N3224F_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(n3224f_i2c_devices[i].client);
		n3224f_i2c_devices[i].client = NULL;
	}

	for (i = NUM_COMMON_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(common_i2c_devices[i].client);
		common_i2c_devices[i].client = NULL;

		bi = &common_i2c_devices[i].board_info;
		if (!strcmp(bi->type, SYS_CPLD_MUX_DRIVER_NAME)) {
			pd = bi->platform_data;
			pd->cpld = NULL;
		}
	}
}

static int add_i2c_clients(struct platform_i2c_device_info *pdi, int num_dev,
			   int ISMT_bus, int I801_bus)
{
	static struct i2c_client *sys_cpld_client;
	struct system_cpld_mux_platform_data *pd;
	int ret;

	while (num_dev--) {
		if (pdi->bus == I2C_ISMT_BUS)
			pdi->bus = ISMT_bus;
		else if (pdi->bus == I2C_I801_BUS)
			pdi->bus = I801_bus;

		/* Put system cpld client in MUX platform data */
		if (!strcmp(pdi->board_info.type, SYS_CPLD_MUX_DRIVER_NAME)) {
			pd = pdi->board_info.platform_data;
			pd->cpld = sys_cpld_client;
		}

		if (!pdi->client) {
			pdi->client = cumulus_i2c_add_client(pdi->bus,
							     &pdi->board_info);
			if (IS_ERR(pdi->client)) {
				ret = PTR_ERR(pdi->client);
				pdi->client = NULL;
				return ret;
			}
		}

		/* Save the client of the system CPLD. */
		if (!strcmp(pdi->board_info.type,
			    SYSTEM_CPLD_DRIVER_NAME))
			sys_cpld_client = pdi->client;
		pdi++;
	}
	return 0;
}

static int platform_probe(struct platform_device *dev)
{
	int ISMT_bus;
	int I801_bus;
	int ret;
	enum variant_enum *pv = dev_get_platdata(&dev->dev);

	ret = -ENODEV;
	ISMT_bus = cumulus_i2c_find_adapter(ISMT_ADAPTER_NAME);
	if (ISMT_bus < 0) {
		dev_err(&dev->dev, "Could not find iSMT adapter bus\n");
		goto err_exit;
	}
	I801_bus = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (I801_bus < 0) {
		dev_err(&dev->dev, "Could not find i801 adapter bus\n");
		goto err_exit;
	}

	/* I2C devices common to all n32xx platforms */
	ret = add_i2c_clients(&common_i2c_devices[0], NUM_COMMON_I2C_DEVICES,
			      ISMT_bus, I801_bus);
	if (ret)
		goto err_exit;

	/* I2C devices on n3224f platforms */
	if (*pv == VAR_N3224F) {
		ret = add_i2c_clients(&n3224f_i2c_devices[0],
				      NUM_N3224F_I2C_DEVICES,
				      ISMT_bus, I801_bus);
		if (ret)
			goto err_exit;
	}

	/* I2C devices on all n32xx platforms except n3208px */
	if (*pv != VAR_N3208PX) {
		ret = add_i2c_clients(&not_n3208px_i2c_devices[0],
				      NUM_NOT_N3208PX_I2C_DEVICES,
				      ISMT_bus, I801_bus);
		if (ret)
			goto err_exit;
	}

	/* I2C devices on n3208px platforms */
	if (*pv == VAR_N3208PX) {
		ret = add_i2c_clients(&n3208px_i2c_devices[0],
				      NUM_N3208PX_I2C_DEVICES,
				      ISMT_bus, I801_bus);
		if (ret)
			goto err_exit;
	}

	/* I2C devices on 24 port platforms */
	if (*pv == VAR_N3224PX || *pv == VAR_N3224P || *pv == VAR_N3224T ||
	    *pv == VAR_N3224F || *pv == VAR_N3224X) {
		ret = add_i2c_clients(&ports_24_i2c_devices[0],
				      NUM_PORTS_24_I2C_DEVICES, ISMT_bus,
				      I801_bus);
		if (ret)
			goto err_exit;
	}

	/* I2C devices on 48 port platforms */
	if (*pv == VAR_N3248PXE || *pv == VAR_N3248X || *pv == VAR_N3248P ||
	    *pv == VAR_N3248TE) {
		ret = add_i2c_clients(&ports_48_i2c_devices[0],
				      NUM_PORTS_48_I2C_DEVICES, ISMT_bus,
				      I801_bus);
		if (ret)
			goto err_exit;
	}

	return 0;

err_exit:
	if (ret != -EPROBE_DEFER)
		del_i2c_clients();
	return ret;
}

static int platform_remove(struct platform_device *dev)
{
	del_i2c_clients();
	return 0;
}

static const struct platform_device_id platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(platform, platform_id);

static struct platform_driver plat_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = platform_probe,
	.remove = platform_remove,
	.id_table = platform_id
};

static struct platform_device *plat_device;

/*
 * Module init/exit
 */
static int __init platform_init(void)
{
	int ret = 0;
	char **p;
	enum variant_enum pv;

	/*
	 * Do as little as possible in the module init function. Basically just
	 * register drivers. Those driver's probe functions will probe for
	 * hardware and create devices.
	 */
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " initializing\n");

	/*
	 * Process module parameter(s)
	 *
	 * There is currently only one: the platform type.
	 * We match that to the known ones to compute the platform
	 * variant we are running on.
	 */
	if (!platform) {
		pr_err(DRIVER_NAME
		       ": \"platform\" parameter must be specified.\n");
		return -EINVAL;
	}
	for (p = platform_names; *p; p++)
		if (!strcmp(*p, platform))
			break;
	if (!*p) {
		pr_err(DRIVER_NAME
		       ": \"platform\" parameter \"%s\" not recognized.\n",
		       platform);
		return -EINVAL;
	}
	pv = p - platform_names;

	/* Register the platform driver */
	ret = platform_driver_register(&plat_driver);
	if (ret) {
		pr_err(DRIVER_NAME
		       ": %s driver registration failed. (%d)\n",
		       plat_driver.driver.name, ret);
		goto err_plat_driver;
	}

	/* Create the platform device */
	plat_device = platform_device_register_data(NULL, DRIVER_NAME, -1,
						    &pv, sizeof(pv));
	if (IS_ERR(plat_device)) {
		ret = PTR_ERR(plat_device);
		pr_err(DRIVER_NAME
		       ": Platform device registration failed. (%d)\n", ret);
		goto err_plat_device;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully initialized\n");
	return ret;

err_plat_device:
	platform_driver_unregister(&plat_driver);
err_plat_driver:
	return ret;
}

static void __exit platform_exit(void)
{
	platform_device_unregister(plat_device);
	platform_driver_unregister(&plat_driver);
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

module_init(platform_init);
module_exit(platform_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell N3248PXE Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
