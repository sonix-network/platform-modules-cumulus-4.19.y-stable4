// SPDX-License-Identifier: GPL-2.0+
/*
 * Lenovo NE2572 platform driver
 *
 * Copyright (C) 2018, 2019 Cumulus Networks, Inc.
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
#include "lenovo-ne2572.h"

#define DRIVER_NAME	"lenovo_ne2572_platform"
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
 *	    0 port cpld0 (0x5b)
 *	    1 pca9548#3 8-channel mux (0x74)
 *	      pca9548#4 8-channel mux (0x75)
 *	      pca9548#5 8-channel mux (0x76)
 *	    2 port cpld1 (0x5c)
 *	    3 pca9548#6 8-channel mux (0x74)
 *	      pca9548#7 8-channel mux (0x75)
 *	      pca9548#8 8-channel mux (0x76)
 *	    4 port cpld2 (0x5d)
 *	    5 pca9548#9 8-channel mux (0x74)
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

	/* 3rd-level pca9548#3 - pca9548#9 mux */
	I2C_MUX5_BUS0 = 51,
	I2C_MUX5_BUS1,
	I2C_MUX5_BUS2,
	I2C_MUX5_BUS3,
	I2C_MUX5_BUS4,
	I2C_MUX5_BUS5,
	I2C_MUX5_BUS6,
	I2C_MUX5_BUS7,

	I2C_MUX6_BUS0,
	I2C_MUX6_BUS1,
	I2C_MUX6_BUS2,
	I2C_MUX6_BUS3,
	I2C_MUX6_BUS4,
	I2C_MUX6_BUS5,
	I2C_MUX6_BUS6,
	I2C_MUX6_BUS7,

	I2C_MUX7_BUS0,
	I2C_MUX7_BUS1,
	I2C_MUX7_BUS2,
	I2C_MUX7_BUS3,
	I2C_MUX7_BUS4,
	I2C_MUX7_BUS5,
	I2C_MUX7_BUS6,
	I2C_MUX7_BUS7,

	I2C_MUX8_BUS0,
	I2C_MUX8_BUS1,
	I2C_MUX8_BUS2,
	I2C_MUX8_BUS3,
	I2C_MUX8_BUS4,
	I2C_MUX8_BUS5,
	I2C_MUX8_BUS6,
	I2C_MUX8_BUS7,

	I2C_MUX9_BUS0,
	I2C_MUX9_BUS1,
	I2C_MUX9_BUS2,
	I2C_MUX9_BUS3,
	I2C_MUX9_BUS4,
	I2C_MUX9_BUS5,
	I2C_MUX9_BUS6,
	I2C_MUX9_BUS7,

	I2C_MUX10_BUS0,
	I2C_MUX10_BUS1,
	I2C_MUX10_BUS2,
	I2C_MUX10_BUS3,
	I2C_MUX10_BUS4,
	I2C_MUX10_BUS5,
	I2C_MUX10_BUS6,
	I2C_MUX10_BUS7,

	I2C_MUX11_BUS0,
	I2C_MUX11_BUS1,
	I2C_MUX11_BUS2,
	I2C_MUX11_BUS3,
	I2C_MUX11_BUS4,
	I2C_MUX11_BUS5,
	I2C_MUX11_BUS6,
	I2C_MUX11_BUS7,
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
mk_pca9548(mux5, I2C_MUX5_BUS0, 1);
mk_pca9548(mux6, I2C_MUX6_BUS0, 1);
mk_pca9548(mux7, I2C_MUX7_BUS0, 1);
mk_pca9548(mux8, I2C_MUX8_BUS0, 1);
mk_pca9548(mux9, I2C_MUX9_BUS0, 1);
mk_pca9548(mux10, I2C_MUX10_BUS0, 1);
mk_pca9548(mux11, I2C_MUX11_BUS0, 1);

mk_eeprom(fruid, 54, 256, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);
mk_eeprom(board, 56, 256, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);
mk_eeprom(fru,	 51, 256, AT24_FLAG_IRUGO | AT24_FLAG_ADDR16);
mk_eeprom(mgmt,	 51, 256, AT24_FLAG_IRUGO);

mk_eeprom(psu1,	 51, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2,	 51, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

mk_eeprom(fan1,	 57, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan2,	 57, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan3,	 57, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan4,	 57, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan5,	 57, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan6,	 57, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

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
mk_port_eeprom(port29, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port30, 50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
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

mk_qsfp_port_eeprom(port49, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port50, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port51, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port52, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port53, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port54, 50, 256, SFF_8436_FLAG_IRUGO);

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
	mk_i2cdev(I2C_MUX3_BUS0, "cpld",    0x5b, NULL), /* Port CPLD0 */
	mk_i2cdev(I2C_MUX3_BUS1, "pca9548", 0x74, &mux5_platform_data),
	mk_i2cdev(I2C_MUX3_BUS1, "pca9548", 0x75, &mux6_platform_data),
	mk_i2cdev(I2C_MUX3_BUS1, "pca9548", 0x76, &mux7_platform_data),
	mk_i2cdev(I2C_MUX3_BUS2, "cpld",    0x5c, NULL), /* Port CPLD1 */
	mk_i2cdev(I2C_MUX3_BUS3, "pca9548", 0x74, &mux8_platform_data),
	mk_i2cdev(I2C_MUX3_BUS3, "pca9548", 0x75, &mux9_platform_data),
	mk_i2cdev(I2C_MUX3_BUS3, "pca9548", 0x76, &mux10_platform_data),
	mk_i2cdev(I2C_MUX3_BUS4, "cpld",    0x5d, NULL), /* Port CPLD2 */
	mk_i2cdev(I2C_MUX3_BUS5, "pca9548", 0x74, &mux11_platform_data),
	mk_i2cdev(I2C_MUX3_BUS7, "pca9548", 0x73, &mux4_platform_data),

	/* devices on mux4 */
	mk_i2cdev(I2C_MUX4_BUS0, "24c02",   0x57, &fan1_57_at24),
	mk_i2cdev(I2C_MUX4_BUS1, "24c02",   0x57, &fan2_57_at24),
	mk_i2cdev(I2C_MUX4_BUS2, "24c02",   0x57, &fan3_57_at24),
	mk_i2cdev(I2C_MUX4_BUS3, "24c02",   0x57, &fan4_57_at24),
	mk_i2cdev(I2C_MUX4_BUS4, "24c02",   0x57, &fan5_57_at24),
	mk_i2cdev(I2C_MUX4_BUS5, "24c02",   0x57, &fan6_57_at24),

	/* devices on mux5 */
	mk_i2cdev(I2C_MUX5_BUS0, "24c02",   0x50, &port1_50_at24),
	mk_i2cdev(I2C_MUX5_BUS1, "24c02",   0x50, &port2_50_at24),
	mk_i2cdev(I2C_MUX5_BUS2, "24c02",   0x50, &port3_50_at24),
	mk_i2cdev(I2C_MUX5_BUS3, "24c02",   0x50, &port4_50_at24),
	mk_i2cdev(I2C_MUX5_BUS4, "24c02",   0x50, &port5_50_at24),
	mk_i2cdev(I2C_MUX5_BUS5, "24c02",   0x50, &port6_50_at24),
	mk_i2cdev(I2C_MUX5_BUS6, "24c02",   0x50, &port7_50_at24),
	mk_i2cdev(I2C_MUX5_BUS7, "24c02",   0x50, &port8_50_at24),

	/* devices on mux6 */
	mk_i2cdev(I2C_MUX6_BUS0, "24c02",   0x50, &port9_50_at24),
	mk_i2cdev(I2C_MUX6_BUS1, "24c02",   0x50, &port10_50_at24),
	mk_i2cdev(I2C_MUX6_BUS2, "24c02",   0x50, &port11_50_at24),
	mk_i2cdev(I2C_MUX6_BUS3, "24c02",   0x50, &port12_50_at24),
	mk_i2cdev(I2C_MUX6_BUS4, "24c02",   0x50, &port13_50_at24),
	mk_i2cdev(I2C_MUX6_BUS5, "24c02",   0x50, &port14_50_at24),
	mk_i2cdev(I2C_MUX6_BUS6, "24c02",   0x50, &port15_50_at24),
	mk_i2cdev(I2C_MUX6_BUS7, "24c02",   0x50, &port16_50_at24),

	/* devices on mux7 */
	mk_i2cdev(I2C_MUX7_BUS0, "24c02",   0x50, &port17_50_at24),
	mk_i2cdev(I2C_MUX7_BUS1, "24c02",   0x50, &port18_50_at24),
	mk_i2cdev(I2C_MUX7_BUS2, "24c02",   0x50, &port19_50_at24),
	mk_i2cdev(I2C_MUX7_BUS3, "24c02",   0x50, &port20_50_at24),
	mk_i2cdev(I2C_MUX7_BUS4, "24c02",   0x50, &port21_50_at24),
	mk_i2cdev(I2C_MUX7_BUS5, "24c02",   0x50, &port22_50_at24),
	mk_i2cdev(I2C_MUX7_BUS6, "24c02",   0x50, &port23_50_at24),
	mk_i2cdev(I2C_MUX7_BUS7, "24c02",   0x50, &port24_50_at24),

	/* devices on mux8 */
	mk_i2cdev(I2C_MUX8_BUS0, "24c02",   0x50, &port25_50_at24),
	mk_i2cdev(I2C_MUX8_BUS1, "24c02",   0x50, &port26_50_at24),
	mk_i2cdev(I2C_MUX8_BUS2, "24c02",   0x50, &port27_50_at24),
	mk_i2cdev(I2C_MUX8_BUS3, "24c02",   0x50, &port28_50_at24),
	mk_i2cdev(I2C_MUX8_BUS4, "24c02",   0x50, &port29_50_at24),
	mk_i2cdev(I2C_MUX8_BUS5, "24c02",   0x50, &port30_50_at24),
	mk_i2cdev(I2C_MUX8_BUS6, "24c02",   0x50, &port31_50_at24),
	mk_i2cdev(I2C_MUX8_BUS7, "24c02",   0x50, &port32_50_at24),

	/* devices on mux9 */
	mk_i2cdev(I2C_MUX9_BUS0, "24c02",   0x50, &port33_50_at24),
	mk_i2cdev(I2C_MUX9_BUS1, "24c02",   0x50, &port34_50_at24),
	mk_i2cdev(I2C_MUX9_BUS2, "24c02",   0x50, &port35_50_at24),
	mk_i2cdev(I2C_MUX9_BUS3, "24c02",   0x50, &port36_50_at24),
	mk_i2cdev(I2C_MUX9_BUS4, "24c02",   0x50, &port37_50_at24),
	mk_i2cdev(I2C_MUX9_BUS5, "24c02",   0x50, &port38_50_at24),
	mk_i2cdev(I2C_MUX9_BUS6, "24c02",   0x50, &port39_50_at24),
	mk_i2cdev(I2C_MUX9_BUS7, "24c02",   0x50, &port40_50_at24),

	/* devices on mux10 */
	mk_i2cdev(I2C_MUX10_BUS0, "24c02",   0x50, &port41_50_at24),
	mk_i2cdev(I2C_MUX10_BUS1, "24c02",   0x50, &port42_50_at24),
	mk_i2cdev(I2C_MUX10_BUS2, "24c02",   0x50, &port43_50_at24),
	mk_i2cdev(I2C_MUX10_BUS3, "24c02",   0x50, &port44_50_at24),
	mk_i2cdev(I2C_MUX10_BUS4, "24c02",   0x50, &port45_50_at24),
	mk_i2cdev(I2C_MUX10_BUS5, "24c02",   0x50, &port46_50_at24),
	mk_i2cdev(I2C_MUX10_BUS6, "24c02",   0x50, &port47_50_at24),
	mk_i2cdev(I2C_MUX10_BUS7, "24c02",   0x50, &port48_50_at24),

	/* devices on mux11 */
	mk_i2cdev(I2C_MUX11_BUS0, "sff8436", 0x50, &port49_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS1, "sff8436", 0x50, &port50_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS2, "sff8436", 0x50, &port51_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS3, "sff8436", 0x50, &port52_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS4, "sff8436", 0x50, &port53_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS5, "sff8436", 0x50, &port54_50_sff8436),

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
		pr_err("could not find i801 adapter bus\n");
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
		pr_err("Attempt to read invalid CPLD register: 0x%02X\n",
		       reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	ret = i2c_smbus_read_byte_data(cpld_devices[cpld_id], reg);
	if (ret < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
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
		pr_err("Attempt to write to invalid CPLD register: 0x%02X\n",
		       reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	ret = i2c_smbus_write_byte_data(cpld_devices[cpld_id], reg, val);
	if (ret) {
		pr_err("CPLD write error - addr: 0x%02X, offset: 0x%02X\n",
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
mk_bf_ro(cpld, port_cpld0_rev,	   PORT_SFP28_CPLD0_REVISION_REG, 0, 8, NULL,
	 0);
mk_bf_ro(cpld, port_cpld1_rev,	   PORT_SFP28_CPLD1_REVISION_REG, 0, 8, NULL,
	 0);
mk_bf_ro(cpld, port_cpld2_rev,	   PORT_QSFP28_CPLD_REVISION_REG, 0, 8, NULL,
	 0);

mk_bf_ro(cpld, cpld_tpm_pp,	   MASTER_TPM_CTRL_REG, 2, 1, NULL,
	 BF_COMPLEMENT);
mk_bf_ro(cpld, tpm_present,	   MASTER_TPM_CTRL_REG, 1, 1, NULL,
	 BF_COMPLEMENT);
mk_bf_ro(cpld, tpm_reset,	   MASTER_TPM_CTRL_REG, 0, 1, NULL, 0);
mk_bf_ro(cpld, reset_button,	   SYSTEM_RESET_BUTTON_REG, 0, 2,
	 reset_button_values, 0);

mk_bf_rw(cpld, cpu_reset,	   POWER_RESET_CONTROL_REG, 1, 1, NULL, 0);
mk_bf_rw(cpld, pca9548_0_reset,	   POWER_RESET_CONTROL_REG, 0, 1, NULL, 0);
mk_bf_rw(cpld, pca9548_3_9_reset,  SYSTEM_RESET_0_REG, 6, 1, NULL, 0);
mk_bf_rw(cpld, bcm56967_reset,	   SYSTEM_RESET_0_REG, 5, 1, NULL, 0);
mk_bf_rw(cpld, bcm53115M_reset,	   SYSTEM_RESET_0_REG, 4, 1, NULL, 0);
mk_bf_rw(cpld, 88e1112_reset,	   SYSTEM_RESET_0_REG, 3, 1, NULL, 0);
mk_bf_rw(cpld, pca9548_2_reset,	   SYSTEM_RESET_0_REG, 2, 1, NULL, 0);
mk_bf_rw(cpld, pca9548_1_reset,	   SYSTEM_RESET_0_REG, 1, 1, NULL, 0);
mk_bf_rw(cpld, pca9545_0_reset,	   SYSTEM_RESET_0_REG, 0, 1, NULL, 0);
mk_bf_rw(cpld, bcm82381_reset,     SYSTEM_RESET_1_REG, 3, 1, NULL, 0);
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
mk_bf_rw(cpld, led_fan,	           SYSTEM_FAN_0_LED_REG, 0, 3,
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

mk_bf_ro(cpld, sfp28_1_8_tx_fault,     PORT_SFP28_1_8_TX_FAULT_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp28_1_8_rx_los,       PORT_SFP28_1_8_RX_LOS_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_1_8_present,      PORT_SFP28_1_8_PRESENT_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_1_8_tx_disable,   PORT_SFP28_1_8_TX_DISABLE_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, sfp28_9_16_tx_fault,    PORT_SFP28_9_16_TX_FAULT_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp28_9_16_rx_los,      PORT_SFP28_9_16_RX_LOS_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_9_16_present,     PORT_SFP28_9_16_PRESENT_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_9_16_tx_disable,  PORT_SFP28_9_16_TX_DISABLE_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, sfp28_17_24_tx_fault,   PORT_SFP28_17_24_TX_FAULT_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp28_17_24_rx_los,     PORT_SFP28_17_24_RX_LOS_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_17_24_present,    PORT_SFP28_17_24_PRESENT_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_17_24_tx_disable, PORT_SFP28_17_24_TX_DISABLE_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, sfp28_25_32_tx_fault,   PORT_SFP28_25_32_TX_FAULT_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp28_25_32_rx_los,     PORT_SFP28_25_32_RX_LOS_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_25_32_present,    PORT_SFP28_25_32_PRESENT_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_25_32_tx_disable, PORT_SFP28_25_32_TX_DISABLE_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, sfp28_33_40_tx_fault,   PORT_SFP28_33_40_TX_FAULT_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp28_33_40_rx_los,     PORT_SFP28_33_40_RX_LOS_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_33_40_present,    PORT_SFP28_33_40_PRESENT_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_33_40_tx_disable, PORT_SFP28_33_40_TX_DISABLE_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, sfp28_41_48_tx_fault,   PORT_SFP28_41_48_TX_FAULT_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp28_41_48_rx_los,     PORT_SFP28_41_48_RX_LOS_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_41_48_present,    PORT_SFP28_41_48_PRESENT_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp28_41_48_tx_disable, PORT_SFP28_41_48_TX_DISABLE_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, qsfp28_49_54_interrupt, PORT_QSFP28_49_54_INTERRUPT_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, qsfp28_49_54_present,   PORT_QSFP28_49_54_PRESENT_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_49_54_reset,     PORT_QSFP28_49_54_RESET_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_49_54_lpmode,    PORT_QSFP28_49_54_LPMODE_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, qsfp28_49_54_modsel,    PORT_QSFP28_49_54_MODSEL_REG, 0, 8,
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
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       cpld_devices[cpld_id]->addr, reg);
		return -EINVAL;
	}

	return sprintf(buf, "%d\n", ret * 150);
}

static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, fan_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO, fan_show, NULL, 2);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO, fan_show, NULL, 3);
static SENSOR_DEVICE_ATTR(fan4_input, S_IRUGO, fan_show, NULL, 4);
static SENSOR_DEVICE_ATTR(fan5_input, S_IRUGO, fan_show, NULL, 5);
static SENSOR_DEVICE_ATTR(fan6_input, S_IRUGO, fan_show, NULL, 6);
static SENSOR_DEVICE_ATTR(fan7_input, S_IRUGO, fan_show, NULL, 7);
static SENSOR_DEVICE_ATTR(fan8_input, S_IRUGO, fan_show, NULL, 8);
static SENSOR_DEVICE_ATTR(fan9_input, S_IRUGO, fan_show, NULL, 9);
static SENSOR_DEVICE_ATTR(fan10_input, S_IRUGO, fan_show, NULL, 10);
static SENSOR_DEVICE_ATTR(fan11_input, S_IRUGO, fan_show, NULL, 11);
static SENSOR_DEVICE_ATTR(fan12_input, S_IRUGO, fan_show, NULL, 12);
static SENSOR_DEVICE_ATTR(fan_min, S_IRUGO, fan_show, NULL, 0);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_master_cpld_rev.attr,
	&cpld_power_cpld_rev.attr,
	&cpld_system_cpld_rev.attr,
	&cpld_port_cpld0_rev.attr,
	&cpld_port_cpld1_rev.attr,
	&cpld_port_cpld2_rev.attr,

	&cpld_cpld_tpm_pp.attr,
	&cpld_tpm_present.attr,
	&cpld_tpm_reset.attr,
	&cpld_reset_button.attr,

	&cpld_cpu_reset.attr,
	&cpld_pca9548_0_reset.attr,
	&cpld_pca9548_3_9_reset.attr,
	&cpld_bcm56967_reset.attr,
	&cpld_bcm53115M_reset.attr,
	&cpld_88e1112_reset.attr,
	&cpld_pca9548_2_reset.attr,
	&cpld_pca9548_1_reset.attr,
	&cpld_pca9545_0_reset.attr,
	&cpld_bcm82381_reset.attr,
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
	&cpld_led_fan.attr,
	&cpld_led_fan2_rear.attr,
	&cpld_led_fan1_rear.attr,
	&cpld_led_fan4_rear.attr,
	&cpld_led_fan3_rear.attr,
	&cpld_led_fan6_rear.attr,
	&cpld_led_fan5_rear.attr,
	&cpld_led_system.attr,

	&cpld_sfp28_1_8_tx_fault.attr,
	&cpld_sfp28_1_8_rx_los.attr,
	&cpld_sfp28_1_8_present.attr,
	&cpld_sfp28_1_8_tx_disable.attr,

	&cpld_sfp28_9_16_tx_fault.attr,
	&cpld_sfp28_9_16_rx_los.attr,
	&cpld_sfp28_9_16_present.attr,
	&cpld_sfp28_9_16_tx_disable.attr,

	&cpld_sfp28_17_24_tx_fault.attr,
	&cpld_sfp28_17_24_rx_los.attr,
	&cpld_sfp28_17_24_present.attr,
	&cpld_sfp28_17_24_tx_disable.attr,

	&cpld_sfp28_25_32_tx_fault.attr,
	&cpld_sfp28_25_32_rx_los.attr,
	&cpld_sfp28_25_32_present.attr,
	&cpld_sfp28_25_32_tx_disable.attr,

	&cpld_sfp28_33_40_tx_fault.attr,
	&cpld_sfp28_33_40_rx_los.attr,
	&cpld_sfp28_33_40_present.attr,
	&cpld_sfp28_33_40_tx_disable.attr,

	&cpld_sfp28_41_48_tx_fault.attr,
	&cpld_sfp28_41_48_rx_los.attr,
	&cpld_sfp28_41_48_present.attr,
	&cpld_sfp28_41_48_tx_disable.attr,

	&cpld_qsfp28_49_54_interrupt.attr,
	&cpld_qsfp28_49_54_present.attr,
	&cpld_qsfp28_49_54_reset.attr,
	&cpld_qsfp28_49_54_lpmode.attr,
	&cpld_qsfp28_49_54_modsel.attr,

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
		pr_err("sysfs_cpld_driver_group failed for cpld driver");

	return ret;
}

static int cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver cpld_driver = {
	.driver = {
		.name = "lenovo_ne2572_cpld",
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
		pr_err("Error: number of CPLD devices: %d; expected: %d\n",
		       num_cpld_devices, NUM_CPLD_DEVICES);
		return -ENODEV;
	}

	ret = platform_driver_register(&cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for CPLD device\n");
		return ret;
	}

	cpld_device = platform_device_alloc("lenovo_ne2572_cpld", 0);
	if (!cpld_device) {
		pr_err("platform_device_alloc() failed for CPLD device\n");
		platform_driver_unregister(&cpld_driver);
		return -ENOMEM;
	}

	ret = platform_device_add(cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for CPLD device.\n");
		platform_device_put(cpld_device);
		return ret;
	}

	pr_info("CPLD driver loaded\n");
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
	pr_err("CPLD driver unloaded\n");
}

/*
 * Module init and exit
 */

static int __init lenovo_ne2572_init(void)
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

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit lenovo_ne2572_exit(void)
{
	cpld_exit();
	i2c_exit();
	pr_info(DRIVER_NAME " driver unloaded\n");
}

module_init(lenovo_ne2572_init);
module_exit(lenovo_ne2572_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Lenovo NE2572 platform driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
