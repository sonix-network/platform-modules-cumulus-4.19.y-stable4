// SPDX-License-Identifier: GPL-2.0+
/*
 *  dellemc-s41xx-platform.c - Dell EMC S41xx Platform Support.
 *
 *  Copyright (C) 2017, 2019, 2020 Cumulus Networks, Inc.  All Rights Reserved
 *  Author: David Yen (dhyen@cumulusnetworks.com)
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
#include "dellemc-s41xx-cplds.h"

#define DRIVER_NAME	"dellemc_s41xx"
#define DRIVER_VERSION	"1.2"

/*
 * Required platform module parameter determines the type of platform:
 *   s4148f  - 48 SFP+, 2 QSFP+, 4 QSFP28
 *   s4128f  - 24 SFP+, 2 QSFP28
 *   s4148t  - 48 10GBT, 2 QSFP+, 4 QSFP28
 *   s4128t  - 24 10GBT, 2 QSFP28
 *   s4148fe - 48 LRM-capable SFP+, 2 QSFP+, 4 QSFP28
 *   s4148u  - 48 SFP+, 2 QSFP+, 4 QSFP28, first 24 SFP+ and QSFP28 support FC
 */
static char *platform;
module_param(platform, charp, 0);
MODULE_PARM_DESC(platform, "Platform type");

enum variant_enum {
	VAR_UNKNOWN = -1,
	VAR_S4148F,
	VAR_S4128F,
	VAR_S4148T,
	VAR_S4128T,
	VAR_S4148FE,
	VAR_S4148U,

	NUM_VARIANTS
};

static char *platform_names[NUM_VARIANTS + 1] = {
	[VAR_S4148F]   = "s4148f",
	[VAR_S4128F]   = "s4128f",
	[VAR_S4148T]   = "s4148t",
	[VAR_S4128T]   = "s4128t",
	[VAR_S4148FE]  = "s4148fe",
	[VAR_S4148U]   = "s4148u",
	[NUM_VARIANTS] = NULL /* end-of-list sentinel */
};

enum {
	/* I2C busses eminating from the Intel SOC */
	I2C_I801_BUS = -10,
	I2C_ISMT_BUS,

	/* top-level main board mux legs (actually resides on CPU module) */
	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,
	I2C_MUX1_BUS4,

	/* mux legs for the SFP+ and QSFP28 ports */
	I2C_SFF_PORT_MUX1  = DELL_S41XX_MUX_BUS_START,
	I2C_SFF_PORT_MUX2,
	I2C_SFF_PORT_MUX3,
	I2C_SFF_PORT_MUX4,
	I2C_SFF_PORT_MUX5,
	I2C_SFF_PORT_MUX6,
	I2C_SFF_PORT_MUX7,
	I2C_SFF_PORT_MUX8,
	I2C_SFF_PORT_MUX9,
	I2C_SFF_PORT_MUX10,
	I2C_SFF_PORT_MUX11,
	I2C_SFF_PORT_MUX12,
	I2C_SFF_PORT_MUX13,
	I2C_SFF_PORT_MUX14,
	I2C_SFF_PORT_MUX15,
	I2C_SFF_PORT_MUX16,
	I2C_SFF_PORT_MUX17,
	I2C_SFF_PORT_MUX18,
	I2C_SFF_PORT_MUX19,
	I2C_SFF_PORT_MUX20,
	I2C_SFF_PORT_MUX21,
	I2C_SFF_PORT_MUX22,
	I2C_SFF_PORT_MUX23,
	I2C_SFF_PORT_MUX24,
	I2C_SFF_PORT_MUX25,
	I2C_SFF_PORT_MUX26,
	I2C_SFF_PORT_MUX27,
	I2C_SFF_PORT_MUX28,
	I2C_SFF_PORT_MUX29,
	I2C_SFF_PORT_MUX30,
	I2C_SFF_PORT_MUX31,
	I2C_SFF_PORT_MUX32,
	I2C_SFF_PORT_MUX33,
	I2C_SFF_PORT_MUX34,
	I2C_SFF_PORT_MUX35,
	I2C_SFF_PORT_MUX36,
	I2C_SFF_PORT_MUX37,
	I2C_SFF_PORT_MUX38,
	I2C_SFF_PORT_MUX39,
	I2C_SFF_PORT_MUX40,
	I2C_SFF_PORT_MUX41,
	I2C_SFF_PORT_MUX42,
	I2C_SFF_PORT_MUX43,
	I2C_SFF_PORT_MUX44,
	I2C_SFF_PORT_MUX45,
	I2C_SFF_PORT_MUX46,
	I2C_SFF_PORT_MUX47,
	I2C_SFF_PORT_MUX48,
	I2C_SFF_PORT_MUX49,
	I2C_SFF_PORT_MUX50,
	I2C_SFF_PORT_MUX51,
	I2C_SFF_PORT_MUX52,
	I2C_SFF_PORT_MUX53,
	I2C_SFF_PORT_MUX54
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

mk_eeprom(spd,    52, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board,  53, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan1,   51, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan2,   52, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan3,   53, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan4,   54, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu1,   50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2,   51, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

/* These module EEPROMs are on all fiber switches */
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

/* These module EEPROMs are on s4128f switches */
mk_port_eeprom(port27, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port28, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port29, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port30, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

/* These module EEPROMs are on all fiber 4148 switches */
mk_port_eeprom(port31, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port32, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port33, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port34, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port35, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port36, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port37, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port38, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port39, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port40, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port41, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port42, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port43, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port44, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port45, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port46, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port47, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port48, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port49, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port50, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port51, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port52, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port53, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port54, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

/* These module EEPROMs are on all switches */
mk_qsfp_port_eeprom(port25, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port26, 50, 256, SFF_8436_FLAG_IRUGO);

/* These module EEPROMs are on all 4148 switches */
mk_qsfp_port_eeprom(port27, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port28, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port29, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port30, 50, 256, SFF_8436_FLAG_IRUGO);

/*
 * Common I2C devices - These I2C devices are on all 41xx platforms.
 */
static struct platform_i2c_device_info common_i2c_devices[] = {
	/* devices on i801 bus */
	mk_i2cdev(I2C_I801_BUS,  "spd",     0x52, &spd_52_at24),

	/* devices on iSMT bus */
	mk_i2cdev(I2C_ISMT_BUS,  "pca9548", 0x70, &mux1_platform_data),

	/* devices on mux0 */
	mk_i2cdev(I2C_MUX1_BUS0, SYSTEM_CPLD_DRIVER_NAME, 0x31, NULL),
	mk_i2cdev(I2C_MUX1_BUS0, MASTER_CPLD_DRIVER_NAME, 0x32, NULL),
	mk_i2cdev(I2C_MUX1_BUS0, "tmp75",          0x4d, NULL),
	mk_i2cdev(I2C_MUX1_BUS0, "24c02",          0x53, &board_53_at24),

	/* devices on mux1 */
	mk_i2cdev(I2C_MUX1_BUS1, "lm25066",        0x40, NULL),/* PSU 1 PMBus */
	mk_i2cdev(I2C_MUX1_BUS1, "lm25066",        0x42, NULL),/* PSU 2 PMBus */
	mk_i2cdev(I2C_MUX1_BUS1, "24c02",          0x51, &fan1_51_at24),
	mk_i2cdev(I2C_MUX1_BUS1, "24c02",          0x52, &fan2_52_at24),
	mk_i2cdev(I2C_MUX1_BUS1, "24c02",          0x53, &fan3_53_at24),
	mk_i2cdev(I2C_MUX1_BUS1, "24c02",          0x54, &fan4_54_at24),
	mk_i2cdev(I2C_MUX1_BUS1, "emc2305",        0x4d, NULL), /* fan ctl #1 */

	/* devices on mux2, MUST come after the Master CPLD in this array */
	mk_i2cdev(I2C_MUX1_BUS2, MUX_DRIVER_NAME,  0x7f, NULL),

	/* devices on mux3 */
	mk_i2cdev(I2C_MUX1_BUS3, "tmp75",          0x4a, NULL),
	mk_i2cdev(I2C_MUX1_BUS3, "tmp75",          0x4b, NULL),
	mk_i2cdev(I2C_MUX1_BUS3, "tmp75",          0x4c, NULL),
	mk_i2cdev(I2C_MUX1_BUS3, "tmp75",          0x4e, NULL),
	mk_i2cdev(I2C_MUX1_BUS3, "tmp75",          0x4f, NULL),

	/* devices on mux4 */
	mk_i2cdev(I2C_MUX1_BUS4, "24c02",          0x50, &psu1_50_at24),
	mk_i2cdev(I2C_MUX1_BUS4, "dps460",         0x58, NULL),/* PSU1 fan ctl*/
	mk_i2cdev(I2C_MUX1_BUS4, "24c02",          0x51, &psu2_51_at24),
	mk_i2cdev(I2C_MUX1_BUS4, "dps460",         0x59, NULL),/* PSU2 fan ctl*/

	/* devices on SN74CBTLV3251 port muxes */
	mk_i2cdev(I2C_SFF_PORT_MUX25, "sff8436",   0x50, &port25_50_sff8436),
	mk_i2cdev(I2C_SFF_PORT_MUX26, "sff8436",   0x50, &port26_50_sff8436)
};

#define NUM_COMMON_I2C_DEVICES ARRAY_SIZE(common_i2c_devices)

/*
 * F and U I2C devices - These devices are on the F and U 41xx platforms.
 */
static struct platform_i2c_device_info f_u_i2c_devices[] = {
	/* devices on mux0 */
	mk_i2cdev(I2C_MUX1_BUS0, SLAVE_CPLD_DRIVER_NAME,  0x33, NULL),

	/* devices on SN74CBTLV3251 port muxes */
	mk_i2cdev(I2C_SFF_PORT_MUX1,  "24c02",     0x50, &port1_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX2,  "24c02",     0x50, &port2_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX3,  "24c02",     0x50, &port3_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX4,  "24c02",     0x50, &port4_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX5,  "24c02",     0x50, &port5_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX6,  "24c02",     0x50, &port6_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX7,  "24c02",     0x50, &port7_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX8,  "24c02",     0x50, &port8_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX9,  "24c02",     0x50, &port9_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX10, "24c02",     0x50, &port10_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX11, "24c02",     0x50, &port11_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX12, "24c02",     0x50, &port12_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX13, "24c02",     0x50, &port13_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX14, "24c02",     0x50, &port14_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX15, "24c02",     0x50, &port15_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX16, "24c02",     0x50, &port16_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX17, "24c02",     0x50, &port17_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX18, "24c02",     0x50, &port18_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX19, "24c02",     0x50, &port19_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX20, "24c02",     0x50, &port20_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX21, "24c02",     0x50, &port21_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX22, "24c02",     0x50, &port22_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX23, "24c02",     0x50, &port23_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX24, "24c02",     0x50, &port24_50_at24)
};

#define NUM_F_U_I2C_DEVICES ARRAY_SIZE(f_u_i2c_devices)

/*
 * T I2C devices - These devices are on the T 41xx platforms.
 */
static struct platform_i2c_device_info t_i2c_devices[] = {
	/* devices on mux3 */
	mk_i2cdev(I2C_MUX1_BUS3, "tmp75",          0x49, NULL)
};

#define NUM_T_I2C_DEVICES ARRAY_SIZE(t_i2c_devices)

/*
 * U and 48T I2C devices - These devices are on the U and 48T platforms.
 */
static struct platform_i2c_device_info u_48t_i2c_devices[] = {
	/* devices on mux3 */
	mk_i2cdev(I2C_MUX1_BUS3, "emc2305",        0x4d, NULL) /* fan ctl #2 */
};

#define NUM_U_48T_I2C_DEVICES ARRAY_SIZE(u_48t_i2c_devices)

/*
 * 4128f port I2C devices - These devices are on the 4128f platform
 */
static struct platform_i2c_device_info s4128f_i2c_devices[] = {
	/* devices on SN74CBTLV3251 port muxes */
	mk_i2cdev(I2C_SFF_PORT_MUX27, "24c02",     0x50, &port27_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX28, "24c02",     0x50, &port28_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX29, "24c02",     0x50, &port29_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX30, "24c02",     0x50, &port30_50_at24)
};

#define NUM_S4128F_I2C_DEVICES ARRAY_SIZE(s4128f_i2c_devices)

/*
 * 4148x port I2C devices - These devices are on the 48 port platforms
 */
static struct platform_i2c_device_info s4148x_i2c_devices[] = {
	mk_i2cdev(I2C_SFF_PORT_MUX27, "sff8436",   0x50, &port27_50_sff8436),
	mk_i2cdev(I2C_SFF_PORT_MUX28, "sff8436",   0x50, &port28_50_sff8436),
	mk_i2cdev(I2C_SFF_PORT_MUX29, "sff8436",   0x50, &port29_50_sff8436),
	mk_i2cdev(I2C_SFF_PORT_MUX30, "sff8436",   0x50, &port30_50_sff8436),
};

#define NUM_S4148X_I2C_DEVICES ARRAY_SIZE(s4148x_i2c_devices)

/*
 * 4148f port I2C devices - These devices are on the 4148f and u platforms
 */
static struct platform_i2c_device_info s4148f_i2c_devices[] = {
	mk_i2cdev(I2C_SFF_PORT_MUX31, "24c02",     0x50, &port31_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX32, "24c02",     0x50, &port32_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX33, "24c02",     0x50, &port33_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX34, "24c02",     0x50, &port34_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX35, "24c02",     0x50, &port35_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX36, "24c02",     0x50, &port36_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX37, "24c02",     0x50, &port37_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX38, "24c02",     0x50, &port38_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX39, "24c02",     0x50, &port39_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX40, "24c02",     0x50, &port40_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX41, "24c02",     0x50, &port41_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX42, "24c02",     0x50, &port42_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX43, "24c02",     0x50, &port43_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX44, "24c02",     0x50, &port44_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX45, "24c02",     0x50, &port45_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX46, "24c02",     0x50, &port46_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX47, "24c02",     0x50, &port47_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX48, "24c02",     0x50, &port48_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX49, "24c02",     0x50, &port49_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX50, "24c02",     0x50, &port50_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX51, "24c02",     0x50, &port51_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX52, "24c02",     0x50, &port52_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX53, "24c02",     0x50, &port53_50_at24),
	mk_i2cdev(I2C_SFF_PORT_MUX54, "24c02",     0x50, &port54_50_at24)
};

#define NUM_S4148F_I2C_DEVICES ARRAY_SIZE(s4148f_i2c_devices)

static void del_i2c_clients(void)
{
	int i;

	for (i = NUM_S4148F_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(s4148f_i2c_devices[i].client);
		s4148f_i2c_devices[i].client = NULL;
	}

	for (i = NUM_S4148X_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(s4148x_i2c_devices[i].client);
		s4148x_i2c_devices[i].client = NULL;
	}

	for (i = NUM_S4128F_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(s4128f_i2c_devices[i].client);
		s4128f_i2c_devices[i].client = NULL;
	}

	for (i = NUM_U_48T_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(u_48t_i2c_devices[i].client);
		u_48t_i2c_devices[i].client = NULL;
	}

	for (i = NUM_T_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(t_i2c_devices[i].client);
		t_i2c_devices[i].client = NULL;
	}

	for (i = NUM_F_U_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(f_u_i2c_devices[i].client);
		f_u_i2c_devices[i].client = NULL;
	}

	for (i = NUM_COMMON_I2C_DEVICES - 1; i >= 0; i--) {
		i2c_unregister_device(common_i2c_devices[i].client);
		common_i2c_devices[i].client = NULL;

		if (!strcmp(common_i2c_devices[i].board_info.type,
			    MUX_DRIVER_NAME)) {
			common_i2c_devices[i].board_info.platform_data = NULL;
		}
	}
}

static int add_i2c_clients(struct platform_i2c_device_info *pdi, int num_dev,
			   int ISMT_bus, int I801_bus)
{
	struct i2c_client *master_client = NULL;
	int ret;

	while (num_dev--) {
		if (pdi->bus == I2C_ISMT_BUS)
			pdi->bus = ISMT_bus;
		else if (pdi->bus == I2C_I801_BUS)
			pdi->bus = I801_bus;

		/* Put master cpld client in MUX platform data */
		if (!strcmp(pdi->board_info.type, MUX_DRIVER_NAME))
			pdi->board_info.platform_data = master_client;

		if (!pdi->client) {
			pdi->client = cumulus_i2c_add_client(pdi->bus,
							     &pdi->board_info);
			if (IS_ERR(pdi->client)) {
				ret = PTR_ERR(pdi->client);
				pdi->client = NULL;
				return ret;
			}
		}

		/* Save the client of the master CPLD. */
		if (!strcmp(pdi->board_info.type,
			    MASTER_CPLD_DRIVER_NAME) &&
		    pdi->board_info.addr == 0x32)
			master_client = pdi->client;
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

	/* I2C devices common to all 41xx platforms */
	ret = add_i2c_clients(&common_i2c_devices[0], NUM_COMMON_I2C_DEVICES,
			      ISMT_bus, I801_bus);
	if (ret)
		goto err_exit;

	/* I2C devices on all 41xx fiber platforms */
	if (*pv == VAR_S4148F || *pv == VAR_S4128F || *pv == VAR_S4148FE ||
	    *pv == VAR_S4148U) {
		ret = add_i2c_clients(&f_u_i2c_devices[0], NUM_F_U_I2C_DEVICES,
				      ISMT_bus, I801_bus);
		if (ret)
			goto err_exit;
	}

	/* I2C devices on all 41xx twisted pair platforms */
	if (*pv == VAR_S4148T || *pv == VAR_S4128T) {
		ret = add_i2c_clients(&t_i2c_devices[0], NUM_T_I2C_DEVICES,
				      ISMT_bus, I801_bus);
		if (ret)
			goto err_exit;
	}

	/* I2C devices on s4148t and s4148u platforms */
	if (*pv == VAR_S4148T || *pv == VAR_S4148U) {
		ret = add_i2c_clients(&u_48t_i2c_devices[0],
				      NUM_U_48T_I2C_DEVICES,
				      ISMT_bus, I801_bus);
		if (ret)
			goto err_exit;
	}

	/* I2C devices on s4128f platform */
	if (*pv == VAR_S4128F) {
		ret = add_i2c_clients(&s4128f_i2c_devices[0],
				      NUM_S4128F_I2C_DEVICES, ISMT_bus,
				      I801_bus);
		if (ret)
			goto err_exit;
	}

	/* I2C devices on 48 port platforms */
	if (*pv == VAR_S4148T || *pv == VAR_S4148F || *pv == VAR_S4148FE ||
	    *pv == VAR_S4148U) {
		ret = add_i2c_clients(&s4148x_i2c_devices[0],
				      NUM_S4148X_I2C_DEVICES, ISMT_bus,
				      I801_bus);
		if (ret)
			goto err_exit;
	}

	/* I2C devices on 48 port fiber platforms */
	if (*pv == VAR_S4148F || *pv == VAR_S4148FE || *pv == VAR_S4148U) {
		ret = add_i2c_clients(&s4148f_i2c_devices[0],
				      NUM_S4148F_I2C_DEVICES, ISMT_bus,
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
MODULE_DESCRIPTION("Dell EMC S41xx platform support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
