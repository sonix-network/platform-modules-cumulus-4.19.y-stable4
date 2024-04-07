/*
 * accton_as7816_64x_platform.c - Accton AS7816_64X Platform Support.
 *
 * Copyright (C) 2018 Cumulus Networks, Inc.  All Rights Reserved
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
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/hwmon-sysfs.h>
#include <linux/platform_device.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "platform-bitfield.h"
#include "accton-as7816-64x.h"

#define DRIVER_NAME	"accton_as7816_64x_platform"
#define DRIVER_VERSION	"1.0"

/*
 * The i801 bus is connected to the following devices on the CPU board:
 *
 *    board eeprom (0x56)
 *    cpld (0x65)
 *    pca9548 8-channel mux (0x77)
 *	 0 pca9548 8-channel mux (0x71)
 *	      0 psu2 eeprom (0x50)
 *		psu2 pmbus (0x58)
 *	      1 psu1 eeprom (0x53)
 *		psu1 pmbus (0x5b)
 *	   pca9548 8-channel mux (0x76)
 *	      0 fan cpld (0x68)
 *		lm75 (0x4d)
 *		lm75 (0x4e)
 *	      1 lm75 (0x48)
 *		lm75 (0x49)
 *		lm75 (0x4a)
 *		lm75 (0x4b)
 *	      2 cpld1 (0x60)
 *	      3 cpld2(0x60)
 *	      4 cpld3(0x60)
 *	      5 cpld4(0x60)
 *	   pca9548 8-channel mux (0x73)
 *	      * qsfp28 eeprom (0x50)
 *	 1 pca9548 8 channel mux (0x70)
 *	      * qsfp28 eeprom (0x50)
 *	   pca9548 8 channel mux (0x71)
 *	      * qsfp28 eeprom (0x50)
 *	   pca9548 8 channel mux (0x72)
 *	      * qsfp28 eeprom (0x50)
 *	   pca9548 8 channel mux (0x73)
 *	      * qsfp28 eeprom (0x50)
 *	   pca9548 8 channel mux (0x74)
 *	      * qsfp28 eeprom (0x50)
 *	   pca9548 8 channel mux (0x75)
 *	      * qsfp28 eeprom (0x50)
 *	   pca9548 8 channel mux (0x76)
 *	      * qsfp28 eeprom (0x50)
 */

enum {
	I2C_ISMT_BUS = 0,
	I2C_I801_BUS = 1,

	I2C_MUX1_BUS0,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,
	I2C_MUX1_BUS4,
	I2C_MUX1_BUS5,
	I2C_MUX1_BUS6,
	I2C_MUX1_BUS7,

	I2C_MUX2_BUS0,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,
	I2C_MUX2_BUS4,
	I2C_MUX2_BUS5,
	I2C_MUX2_BUS6,
	I2C_MUX2_BUS7,

	I2C_MUX3_BUS0,
	I2C_MUX3_BUS1,
	I2C_MUX3_BUS2,
	I2C_MUX3_BUS3,
	I2C_MUX3_BUS4,
	I2C_MUX3_BUS5,
	I2C_MUX3_BUS6,
	I2C_MUX3_BUS7,

	I2C_MUX4_BUS0 = 31,
	I2C_MUX4_BUS1,
	I2C_MUX4_BUS2,
	I2C_MUX4_BUS3,
	I2C_MUX4_BUS4,
	I2C_MUX4_BUS5,
	I2C_MUX4_BUS6,
	I2C_MUX4_BUS7,

	I2C_MUX5_BUS0,
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
 * The list of I2C devices and their bus connections for this platform.
 *
 * Each entry is a bus number and a i2c_board_info.
 * The i2c_board_info specifies the device type, address,
 * and platform data depending on the device type.
 *
 * For muxes, we specify the bus number for each port,
 * and set the deselect_on_exit but (see comment above).
 *
 * For EEPROMs, including ones in the QSFP28 transceivers,
 * we specify the label, I2C address, size, and some flags.
 * All done in the magic mk*_eeprom() macros.  The label is
 * the string that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 */

mk_pca9548(mux1,  I2C_MUX1_BUS0,  1);
mk_pca9548(mux2,  I2C_MUX2_BUS0,  1);
mk_pca9548(mux3,  I2C_MUX3_BUS0,  1);
mk_pca9548(mux4,  I2C_MUX4_BUS0,  1);
mk_pca9548(mux5,  I2C_MUX5_BUS0,  1);
mk_pca9548(mux6,  I2C_MUX6_BUS0,  1);
mk_pca9548(mux7,  I2C_MUX7_BUS0,  1);
mk_pca9548(mux8,  I2C_MUX8_BUS0,  1);
mk_pca9548(mux9,  I2C_MUX9_BUS0,  1);
mk_pca9548(mux10, I2C_MUX10_BUS0, 1);
mk_pca9548(mux11, I2C_MUX11_BUS0, 1);

mk_eeprom(board, 56, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu2,	 50, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu1,	 53, 256, AT24_FLAG_IRUGO);

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
mk_qsfp_port_eeprom(port33, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port34, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port35, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port36, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port37, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port38, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port39, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port40, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port41, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port42, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port43, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port44, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port45, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port46, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port47, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port48, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port49, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port50, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port51, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port52, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port53, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port54, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port55, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port56, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port57, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port58, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port59, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port60, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port61, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port62, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port63, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port64, 50, 256, SFF_8436_FLAG_IRUGO);

static struct platform_i2c_device_info i2c_devices[] = {
	/* cpu board */
	mk_i2cdev(I2C_I801_BUS,	 "24c02",    0x56, &board_56_at24),
	mk_i2cdev(I2C_I801_BUS,	 "cpld",     0x65, NULL), /* cpu cpld */
	mk_i2cdev(I2C_I801_BUS,	 "pca9548",  0x77, &mux1_platform_data),

	/* main board */
	mk_i2cdev(I2C_MUX1_BUS0, "pca9548",  0x71, &mux2_platform_data),
	mk_i2cdev(I2C_MUX1_BUS0, "pca9548",  0x76, &mux3_platform_data),
	mk_i2cdev(I2C_MUX1_BUS0, "pca9548",  0x73, &mux4_platform_data),

	mk_i2cdev(I2C_MUX1_BUS1, "pca9548",  0x70, &mux5_platform_data),
	mk_i2cdev(I2C_MUX1_BUS1, "pca9548",  0x71, &mux6_platform_data),
	mk_i2cdev(I2C_MUX1_BUS1, "pca9548",  0x72, &mux7_platform_data),
	mk_i2cdev(I2C_MUX1_BUS1, "pca9548",  0x73, &mux8_platform_data),
	mk_i2cdev(I2C_MUX1_BUS1, "pca9548",  0x74, &mux9_platform_data),
	mk_i2cdev(I2C_MUX1_BUS1, "pca9548",  0x75, &mux10_platform_data),
	mk_i2cdev(I2C_MUX1_BUS1, "pca9548",  0x76, &mux11_platform_data),

	/* mux2 */
	mk_i2cdev(I2C_MUX2_BUS0, "24c02",    0x50, &psu2_50_at24),
	mk_i2cdev(I2C_MUX2_BUS1, "24c02",    0x53, &psu1_53_at24),

	/* mux3 */
	mk_i2cdev(I2C_MUX3_BUS0, "cpld",     0x68, NULL), /* fan cpld */
	mk_i2cdev(I2C_MUX3_BUS0, "lm75",     0x4d, NULL),
	mk_i2cdev(I2C_MUX3_BUS0, "lm75",     0x4e, NULL),
	mk_i2cdev(I2C_MUX3_BUS1, "lm75",     0x48, NULL),
	mk_i2cdev(I2C_MUX3_BUS1, "lm75",     0x49, NULL),
	mk_i2cdev(I2C_MUX3_BUS1, "lm75",     0x4a, NULL),
	mk_i2cdev(I2C_MUX3_BUS1, "lm75",     0x4b, NULL),
	mk_i2cdev(I2C_MUX3_BUS2, "cpld",     0x60, NULL), /* port cpld1 */
	mk_i2cdev(I2C_MUX3_BUS3, "cpld",     0x62, NULL), /* port cpld2 */
	mk_i2cdev(I2C_MUX3_BUS4, "cpld",     0x64, NULL), /* port cpld3 */
	mk_i2cdev(I2C_MUX3_BUS5, "cpld",     0x66, NULL), /* port cpld4 */

	/* mux4 (0x73) */
	mk_i2cdev(I2C_MUX4_BUS0, "sff8436",  0x50, &port61_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS1, "sff8436",  0x50, &port62_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS2, "sff8436",  0x50, &port63_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS3, "sff8436",  0x50, &port64_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS4, "sff8436",  0x50, &port55_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS5, "sff8436",  0x50, &port56_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS6, "sff8436",  0x50, &port53_50_sff8436),
	mk_i2cdev(I2C_MUX4_BUS7, "sff8436",  0x50, &port54_50_sff8436),

	/* mux5 (0x70) */
	mk_i2cdev(I2C_MUX5_BUS0, "sff8436",  0x50, &port9_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS1, "sff8436",  0x50, &port10_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS2, "sff8436",  0x50, &port11_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS3, "sff8436",  0x50, &port12_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS4, "sff8436",  0x50, &port1_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS5, "sff8436",  0x50, &port2_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS6, "sff8436",  0x50, &port3_50_sff8436),
	mk_i2cdev(I2C_MUX5_BUS7, "sff8436",  0x50, &port4_50_sff8436),

	/* mux6 (0x71) */
	mk_i2cdev(I2C_MUX6_BUS0, "sff8436",  0x50, &port6_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS1, "sff8436",  0x50, &port5_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS2, "sff8436",  0x50, &port8_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS3, "sff8436",  0x50, &port7_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS4, "sff8436",  0x50, &port13_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS5, "sff8436",  0x50, &port14_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS6, "sff8436",  0x50, &port15_50_sff8436),
	mk_i2cdev(I2C_MUX6_BUS7, "sff8436",  0x50, &port16_50_sff8436),

	/* mux7 (0x72) */
	mk_i2cdev(I2C_MUX7_BUS0, "sff8436",  0x50, &port17_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS1, "sff8436",  0x50, &port18_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS2, "sff8436",  0x50, &port19_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS3, "sff8436",  0x50, &port20_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS4, "sff8436",  0x50, &port25_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS5, "sff8436",  0x50, &port26_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS6, "sff8436",  0x50, &port27_50_sff8436),
	mk_i2cdev(I2C_MUX7_BUS7, "sff8436",  0x50, &port28_50_sff8436),

	/* mux8 (0x73) */
	mk_i2cdev(I2C_MUX8_BUS0, "sff8436",  0x50, &port29_50_sff8436),
	mk_i2cdev(I2C_MUX8_BUS1, "sff8436",  0x50, &port30_50_sff8436),
	mk_i2cdev(I2C_MUX8_BUS2, "sff8436",  0x50, &port31_50_sff8436),
	mk_i2cdev(I2C_MUX8_BUS3, "sff8436",  0x50, &port32_50_sff8436),
	mk_i2cdev(I2C_MUX8_BUS4, "sff8436",  0x50, &port21_50_sff8436),
	mk_i2cdev(I2C_MUX8_BUS5, "sff8436",  0x50, &port22_50_sff8436),
	mk_i2cdev(I2C_MUX8_BUS6, "sff8436",  0x50, &port23_50_sff8436),
	mk_i2cdev(I2C_MUX8_BUS7, "sff8436",  0x50, &port24_50_sff8436),

	/* mux9 (0x74) */
	mk_i2cdev(I2C_MUX9_BUS0, "sff8436",  0x50, &port41_50_sff8436),
	mk_i2cdev(I2C_MUX9_BUS1, "sff8436",  0x50, &port42_50_sff8436),
	mk_i2cdev(I2C_MUX9_BUS2, "sff8436",  0x50, &port43_50_sff8436),
	mk_i2cdev(I2C_MUX9_BUS3, "sff8436",  0x50, &port44_50_sff8436),
	mk_i2cdev(I2C_MUX9_BUS4, "sff8436",  0x50, &port33_50_sff8436),
	mk_i2cdev(I2C_MUX9_BUS5, "sff8436",  0x50, &port34_50_sff8436),
	mk_i2cdev(I2C_MUX9_BUS6, "sff8436",  0x50, &port35_50_sff8436),
	mk_i2cdev(I2C_MUX9_BUS7, "sff8436",  0x50, &port36_50_sff8436),

	/* mux10 (0x75) */
	mk_i2cdev(I2C_MUX10_BUS0, "sff8436",  0x50, &port45_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS1, "sff8436",  0x50, &port46_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS2, "sff8436",  0x50, &port47_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS3, "sff8436",  0x50, &port48_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS4, "sff8436",  0x50, &port37_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS5, "sff8436",  0x50, &port38_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS6, "sff8436",  0x50, &port39_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS7, "sff8436",  0x50, &port40_50_sff8436),

	/* mux11 (0x76)*/
	mk_i2cdev(I2C_MUX11_BUS0, "sff8436",  0x50, &port57_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS1, "sff8436",  0x50, &port58_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS2, "sff8436",  0x50, &port59_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS3, "sff8436",  0x50, &port60_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS4, "sff8436",  0x50, &port49_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS5, "sff8436",  0x50, &port50_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS6, "sff8436",  0x50, &port51_50_sff8436),
	mk_i2cdev(I2C_MUX11_BUS7, "sff8436",  0x50, &port52_50_sff8436),
};

/*
 * Utility functions for I2C
 */

static struct i2c_client *cpld_devices[NUM_CPLD_DEVICES];

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
	int i801_bus = -1;
	int i;
	int ret;
	int num_cpld_devices = 0;

	ret = -1;
	i801_bus = cumulus_i2c_find_adapter(SMBUS_I801_NAME);
	if (i801_bus < 0) {
		pr_err("could not find i801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

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
	int ret;
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

static const char * const fan_direction_values[] = {
	"F2B",
	"B2F",
};

static const char * const led_locator_values[] = {
	"on",
	"off",
};

static const char * const led_values[] = {
	"hw_control",
	"red",
	"green",
	"off",
};

static const char * const led_diag_values[] = {
	"yellow",
	"red",
	"green",
	"off",
};

/* CPLD registers */
mk_bf_ro(cpld, fan_board_version,      FAN_BOARD_INFO_REG, 4, 4,
	 NULL, 0);
mk_bf_ro(cpld, fan_board_id,	       FAN_BOARD_INFO_REG, 0, 4,
	 NULL, 0);
mk_bf_ro(cpld, fan_cpld_version,       FAN_CPLD_VERSION_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, fan_reset_cpld,	       FAN_CPLD_RESET_REG, 7, 1,
	 NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, fantray1_present,       FAN_MODULE_PRESENT_REG, 0, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fantray2_present,       FAN_MODULE_PRESENT_REG, 1, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fantray3_present,       FAN_MODULE_PRESENT_REG, 2, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fantray4_present,       FAN_MODULE_PRESENT_REG, 3, 1,
	 NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, fantray1_direction,     FAN_MODULE_DIRECTION_REG, 0, 1,
	 fan_direction_values, 0);
mk_bf_ro(cpld, fantray2_direction,     FAN_MODULE_DIRECTION_REG, 1, 1,
	 fan_direction_values, 0);
mk_bf_ro(cpld, fantray3_direction,     FAN_MODULE_DIRECTION_REG, 2, 1,
	 fan_direction_values, 0);
mk_bf_ro(cpld, fantray4_direction,     FAN_MODULE_DIRECTION_REG, 3, 1,
	 fan_direction_values, 0);

mk_bf_rw(cpld, fantray1_led_green,     FAN_LED_G_REG, 0, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray2_led_green,     FAN_LED_G_REG, 1, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray3_led_green,     FAN_LED_G_REG, 2, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray4_led_green,     FAN_LED_G_REG, 3, 1,
	 NULL, 0);

mk_bf_rw(cpld, fantray1_led_red,       FAN_LED_R_REG, 0, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray2_led_red,       FAN_LED_R_REG, 1, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray3_led_red,       FAN_LED_R_REG, 2, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray4_led_red,       FAN_LED_R_REG, 3, 1,
	 NULL, 0);

mk_bf_rw(cpld, fantray1_enable,	       FAN_MODULE_POWER_ENABLE_REG, 0, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray2_enable,	       FAN_MODULE_POWER_ENABLE_REG, 1, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray3_enable,	       FAN_MODULE_POWER_ENABLE_REG, 2, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray4_enable,	       FAN_MODULE_POWER_ENABLE_REG, 3, 1,
	 NULL, 0);

mk_bf_rw(cpld, fan_pwm,		       FAN_MODULE_PWM_REG, 0, 4,
	 NULL, 0);
mk_bf_rw(cpld, fan_clk_pwm,	       FAN_CLK_DIVIDER_PWM_REG, 0, 5,
	 NULL, 0);
mk_bf_rw(cpld, fan_clk_tach,	       FAN_CLK_DIVIDER_TACH_REG, 0, 5,
	 NULL, 0);

mk_bf_rw(cpld, fan_front_min,	       FAN_FRONT_MIN_SPEED_SET_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, fan_front_max,	       FAN_FRONT_MAX_SPEED_SET_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, fan_rear_min,	       FAN_REAR_MIN_SPEED_SET_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, fan_rear_max,	       FAN_REAR_MAX_SPEED_SET_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, board_id,	       CPLD1_BOARD_INFO_REG, 4, 2,
	 NULL, 0);
mk_bf_ro(cpld, pcb_version,	       CPLD1_BOARD_INFO_REG, 0, 2,
	 NULL, 0);
mk_bf_ro(cpld, cpld1_version,	       CPLD1_CODE_VERSION_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, bcm56970_rov,	       CPLD1_BCM56970_ROV_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, psu_pwr2_alarm,	       CPLD1_POWER_MODULE_STATUS_REG, 5, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr1_alarm,	       CPLD1_POWER_MODULE_STATUS_REG, 4, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr2_all_ok,	       CPLD1_POWER_MODULE_STATUS_REG, 3, 1,
	 NULL, 0);
mk_bf_ro(cpld, psu_pwr1_all_ok,	       CPLD1_POWER_MODULE_STATUS_REG, 2, 1,
	 NULL, 0);
mk_bf_ro(cpld, psu_pwr2_present,       CPLD1_POWER_MODULE_STATUS_REG, 1, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr1_present,       CPLD1_POWER_MODULE_STATUS_REG, 0, 1,
	 NULL, BF_COMPLEMENT);

mk_bf_rw(cpld, cpld_reset,	       CPLD1_SYSTEM_RESET_1_REG, 2, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, mgmt_phy_reset,	       CPLD1_SYSTEM_RESET_1_REG, 0, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_9_reset,	       CPLD1_SYSTEM_RESET_2_REG, 2, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_8_reset,	       CPLD1_SYSTEM_RESET_2_REG, 1, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_7_reset,	       CPLD1_SYSTEM_RESET_3_REG, 7, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_6_reset,	       CPLD1_SYSTEM_RESET_3_REG, 6, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_5_reset,	       CPLD1_SYSTEM_RESET_3_REG, 5, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_4_reset,	       CPLD1_SYSTEM_RESET_3_REG, 4, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_3_reset,	       CPLD1_SYSTEM_RESET_3_REG, 3, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_2_reset,	       CPLD1_SYSTEM_RESET_3_REG, 2, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_1_reset,	       CPLD1_SYSTEM_RESET_3_REG, 1, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_0_reset,	       CPLD1_SYSTEM_RESET_3_REG, 0, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, system_reset_lock,      CPLD1_SYSTEM_RESET_LOCK_REG, 0, 1,
	 NULL, 0);

mk_bf_rw(cpld, led_locator,	       CPLD1_SYSTEM_LED_1_REG, 4, 1,
	 led_locator_values, 0);
mk_bf_rw(cpld, led_fan,		       CPLD1_SYSTEM_LED_1_REG, 2, 2,
	 led_values, 0);
mk_bf_rw(cpld, led_diag,	       CPLD1_SYSTEM_LED_1_REG, 0, 2,
	 led_diag_values, 0);
mk_bf_rw(cpld, led_psu2,	       CPLD1_SYSTEM_LED_2_REG, 2, 2,
	 led_values, 0);
mk_bf_rw(cpld, led_psu1,	       CPLD1_SYSTEM_LED_2_REG, 0, 2,
	 led_values, 0);

/* Note: the CPLD will automatically clear the reset after 500 ms. */
mk_bf_rw(cpld, qsfp28_8_1_reset,       CPLD1_MODULE_RESET_1_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_16_9_reset,      CPLD1_MODULE_RESET_2_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_24_17_reset,     CPLD1_MODULE_RESET_3_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_32_25_reset,     CPLD1_MODULE_RESET_4_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_40_33_reset,     CPLD1_MODULE_RESET_5_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_48_41_reset,     CPLD1_MODULE_RESET_6_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_56_49_reset,     CPLD1_MODULE_RESET_7_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_64_57_reset,     CPLD1_MODULE_RESET_8_REG, 0, 8,
	 NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, qsfp28_8_1_interrupt,   CPLD1_MODULE_INTERRUPT_1_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_16_9_interrupt,  CPLD1_MODULE_INTERRUPT_2_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_24_17_interrupt, CPLD1_MODULE_INTERRUPT_3_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_32_25_interrupt, CPLD1_MODULE_INTERRUPT_4_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_40_33_interrupt, CPLD1_MODULE_INTERRUPT_5_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_48_41_interrupt, CPLD1_MODULE_INTERRUPT_6_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_56_49_interrupt, CPLD1_MODULE_INTERRUPT_7_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_64_57_interrupt, CPLD1_MODULE_INTERRUPT_8_REG, 0, 8,
	 NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, qsfp28_8_1_present,     CPLD1_MODULE_PRESENT_1_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_16_9_present,    CPLD1_MODULE_PRESENT_2_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_24_17_present,   CPLD1_MODULE_PRESENT_3_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_32_25_present,   CPLD1_MODULE_PRESENT_4_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_40_33_present,   CPLD1_MODULE_PRESENT_5_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_48_41_present,   CPLD1_MODULE_PRESENT_6_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_56_49_present,   CPLD1_MODULE_PRESENT_7_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_64_57_present,   CPLD1_MODULE_PRESENT_8_REG, 0, 8,
	 NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, cpld2_version,	       CPLD2_CODE_VERSION_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, cpld3_version,	       CPLD3_CODE_VERSION_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, cpld4_version,	       CPLD4_CODE_VERSION_REG, 0, 8,
	 NULL, 0);

/* special case for port LED RGB color */

static ssize_t rgb_show(struct device *dev, struct device_attribute *dattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int idx = attr->index;
	int cpld_id;
	int base_reg;
	int red, green, blue;

	cpld_id = GET_CPLD_ID(idx);
	base_reg = STRIP_CPLD_IDX(idx);

	red = i2c_smbus_read_byte_data(cpld_devices[cpld_id], base_reg + 1);
	if (red < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, base_reg + 1);
		return -EINVAL;
	}

	green = i2c_smbus_read_byte_data(cpld_devices[cpld_id], base_reg);
	if (green < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, base_reg);
		return -EINVAL;
	}

	blue = i2c_smbus_read_byte_data(cpld_devices[cpld_id], base_reg + 2);
	if (blue < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, base_reg + 2);
		return -EINVAL;
	}

	return sprintf(buf, "0x%02X%02X%02X\n", red, green, blue);
}

static ssize_t rgb_store(struct device *dev, struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int idx = attr->index;
	int cpld_id;
	int base_reg;
	int ret;
	uint32_t rgb;
	int red, green, blue;

	cpld_id = GET_CPLD_ID(idx);
	base_reg = STRIP_CPLD_IDX(idx);

	ret = kstrtou32(buf, 0, &rgb);
	if (ret != 0)
		return ret;

	red = (rgb >> 16) & 0xff;
	pr_info("red = 0x%02X\n", red);
	ret = i2c_smbus_write_byte_data(cpld_devices[cpld_id], base_reg + 1,
					red);
	if (ret) {
		pr_err("CPLD write error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, base_reg + 1);
	}

	green = (rgb >> 8) & 0xff;
	pr_info("green = 0x%02X\n", green);
	ret = i2c_smbus_write_byte_data(cpld_devices[cpld_id], base_reg,
					green);
	if (ret) {
		pr_err("CPLD write error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, base_reg);
	}

	blue = rgb & 0xff;
	pr_info("blue = 0x%02X\n", blue);
	ret = i2c_smbus_write_byte_data(cpld_devices[cpld_id], base_reg + 2,
					blue);
	if (ret) {
		pr_err("CPLD write error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, base_reg + 2);
	}

	return count;
}

static SENSOR_DEVICE_ATTR_RW(sysled_rgb,      rgb_show, rgb_store,
			     CPLD1_SYSTEM_LED_3_REG);

static SENSOR_DEVICE_ATTR_RW(cpld2_4x25g_rgb, rgb_show, rgb_store,
			     CPLD2_4X25G_RGB_LED_G_REG);
static SENSOR_DEVICE_ATTR_RW(cpld2_4x10g_rgb, rgb_show, rgb_store,
			     CPLD2_4X10G_RGB_LED_G_REG);
static SENSOR_DEVICE_ATTR_RW(cpld2_1x25g_rgb, rgb_show, rgb_store,
			     CPLD2_1X25G_RGB_LED_G_REG);
static SENSOR_DEVICE_ATTR_RW(cpld2_1x10g_rgb, rgb_show, rgb_store,
			     CPLD2_1X10G_RGB_LED_G_REG);

static SENSOR_DEVICE_ATTR_RW(cpld3_4x25g_rgb, rgb_show, rgb_store,
			     CPLD3_4X25G_RGB_LED_G_REG);
static SENSOR_DEVICE_ATTR_RW(cpld3_4x10g_rgb, rgb_show, rgb_store,
			     CPLD3_4X10G_RGB_LED_G_REG);
static SENSOR_DEVICE_ATTR_RW(cpld3_1x25g_rgb, rgb_show, rgb_store,
			     CPLD3_1X25G_RGB_LED_G_REG);
static SENSOR_DEVICE_ATTR_RW(cpld3_1x10g_rgb, rgb_show, rgb_store,
			     CPLD3_1X10G_RGB_LED_G_REG);

static SENSOR_DEVICE_ATTR_RW(cpld4_4x25g_rgb, rgb_show, rgb_store,
			     CPLD4_4X25G_RGB_LED_G_REG);
static SENSOR_DEVICE_ATTR_RW(cpld4_4x10g_rgb, rgb_show, rgb_store,
			     CPLD4_4X10G_RGB_LED_G_REG);
static SENSOR_DEVICE_ATTR_RW(cpld4_1x25g_rgb, rgb_show, rgb_store,
			     CPLD4_1X25G_RGB_LED_G_REG);
static SENSOR_DEVICE_ATTR_RW(cpld4_1x10g_rgb, rgb_show, rgb_store,
			     CPLD4_1X10G_RGB_LED_G_REG);

/* special case for fan speeds */

static ssize_t fan_show(struct device *dev, struct device_attribute *dattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int idx = attr->index;
	int cpld_id;
	int base_reg;
	int val;

	cpld_id = GET_CPLD_ID(idx);
	base_reg = STRIP_CPLD_IDX(idx);

	val = i2c_smbus_read_byte_data(cpld_devices[cpld_id], base_reg);
	if (val < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, base_reg);
		return -EINVAL;
	}
	/* RPM = 24.414 kHz * 60 / (2 * 2 * (255 - val)) */
	return sprintf(buf, "%d\n", 366210 / (255 - val));
}

static SENSOR_DEVICE_ATTR_RO(fan1_input, fan_show,
			     FAN_FRONT_FAN1_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan2_input, fan_show,
			     FAN_FRONT_FAN2_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan3_input, fan_show,
			     FAN_FRONT_FAN3_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan4_input, fan_show,
			     FAN_FRONT_FAN4_MODULE_SPEED_REG);

static SENSOR_DEVICE_ATTR_RO(fan5_input, fan_show,
			     FAN_REAR_FAN1_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan6_input, fan_show,
			     FAN_REAR_FAN2_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan7_input, fan_show,
			     FAN_REAR_FAN3_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan8_input, fan_show,
			     FAN_REAR_FAN4_MODULE_SPEED_REG);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_fan_board_version.attr,
	&cpld_fan_board_id.attr,
	&cpld_fan_cpld_version.attr,
	&cpld_fan_reset_cpld.attr,

	&cpld_fantray1_present.attr,
	&cpld_fantray2_present.attr,
	&cpld_fantray3_present.attr,
	&cpld_fantray4_present.attr,

	&cpld_fantray1_direction.attr,
	&cpld_fantray2_direction.attr,
	&cpld_fantray3_direction.attr,
	&cpld_fantray4_direction.attr,

	&cpld_fantray1_led_green.attr,
	&cpld_fantray2_led_green.attr,
	&cpld_fantray3_led_green.attr,
	&cpld_fantray4_led_green.attr,

	&cpld_fantray1_led_red.attr,
	&cpld_fantray2_led_red.attr,
	&cpld_fantray3_led_red.attr,
	&cpld_fantray4_led_red.attr,

	&cpld_fantray1_enable.attr,
	&cpld_fantray2_enable.attr,
	&cpld_fantray3_enable.attr,
	&cpld_fantray4_enable.attr,

	&cpld_fan_pwm.attr,
	&cpld_fan_clk_pwm.attr,
	&cpld_fan_clk_tach.attr,

	&cpld_fan_front_min.attr,
	&cpld_fan_front_max.attr,
	&cpld_fan_rear_min.attr,
	&cpld_fan_rear_max.attr,

	&cpld_board_id.attr,
	&cpld_pcb_version.attr,
	&cpld_cpld1_version.attr,
	&cpld_bcm56970_rov.attr,

	&cpld_psu_pwr2_alarm.attr,
	&cpld_psu_pwr1_alarm.attr,
	&cpld_psu_pwr2_all_ok.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr1_present.attr,

	&cpld_cpld_reset.attr,
	&cpld_mgmt_phy_reset.attr,
	&cpld_pca9548_9_reset.attr,
	&cpld_pca9548_8_reset.attr,
	&cpld_pca9548_7_reset.attr,
	&cpld_pca9548_6_reset.attr,
	&cpld_pca9548_5_reset.attr,
	&cpld_pca9548_4_reset.attr,
	&cpld_pca9548_3_reset.attr,
	&cpld_pca9548_2_reset.attr,
	&cpld_pca9548_1_reset.attr,
	&cpld_pca9548_0_reset.attr,
	&cpld_system_reset_lock.attr,

	&cpld_led_locator.attr,
	&cpld_led_fan.attr,
	&cpld_led_diag.attr,
	&cpld_led_psu2.attr,
	&cpld_led_psu1.attr,

	&cpld_qsfp28_8_1_reset.attr,
	&cpld_qsfp28_16_9_reset.attr,
	&cpld_qsfp28_24_17_reset.attr,
	&cpld_qsfp28_32_25_reset.attr,
	&cpld_qsfp28_40_33_reset.attr,
	&cpld_qsfp28_48_41_reset.attr,
	&cpld_qsfp28_56_49_reset.attr,
	&cpld_qsfp28_64_57_reset.attr,

	&cpld_qsfp28_8_1_interrupt.attr,
	&cpld_qsfp28_16_9_interrupt.attr,
	&cpld_qsfp28_24_17_interrupt.attr,
	&cpld_qsfp28_32_25_interrupt.attr,
	&cpld_qsfp28_40_33_interrupt.attr,
	&cpld_qsfp28_48_41_interrupt.attr,
	&cpld_qsfp28_56_49_interrupt.attr,
	&cpld_qsfp28_64_57_interrupt.attr,

	&cpld_qsfp28_8_1_present.attr,
	&cpld_qsfp28_16_9_present.attr,
	&cpld_qsfp28_24_17_present.attr,
	&cpld_qsfp28_32_25_present.attr,
	&cpld_qsfp28_40_33_present.attr,
	&cpld_qsfp28_48_41_present.attr,
	&cpld_qsfp28_56_49_present.attr,
	&cpld_qsfp28_64_57_present.attr,

	&cpld_cpld2_version.attr,
	&cpld_cpld3_version.attr,
	&cpld_cpld4_version.attr,

	&sensor_dev_attr_sysled_rgb.dev_attr.attr,

	&sensor_dev_attr_cpld2_4x25g_rgb.dev_attr.attr,
	&sensor_dev_attr_cpld2_4x10g_rgb.dev_attr.attr,
	&sensor_dev_attr_cpld2_1x25g_rgb.dev_attr.attr,
	&sensor_dev_attr_cpld2_1x10g_rgb.dev_attr.attr,

	&sensor_dev_attr_cpld3_4x25g_rgb.dev_attr.attr,
	&sensor_dev_attr_cpld3_4x10g_rgb.dev_attr.attr,
	&sensor_dev_attr_cpld3_1x25g_rgb.dev_attr.attr,
	&sensor_dev_attr_cpld3_1x10g_rgb.dev_attr.attr,

	&sensor_dev_attr_cpld4_4x25g_rgb.dev_attr.attr,
	&sensor_dev_attr_cpld4_4x10g_rgb.dev_attr.attr,
	&sensor_dev_attr_cpld4_1x25g_rgb.dev_attr.attr,
	&sensor_dev_attr_cpld4_1x10g_rgb.dev_attr.attr,

	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,

	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,

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
		pr_err("Failed to create sysfs group for cpld driver\n");

	return ret;
}

static int cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver cpld_driver = {
	.driver = {
		.name = "accton_as7816_64x_cpld",
		.owner = THIS_MODULE,
	},
	.probe = cpld_probe,
	.remove = cpld_remove,
};

static struct platform_device *cpld_device;

static int cpld_init(void)
{
	int ret;

	ret = platform_driver_register(&cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for CPLD device\n");
		return ret;
	}

	cpld_device = platform_device_alloc("accton_as7816_64x_cpld", 0);
	if (!cpld_device) {
		pr_err("platform_device_alloc() failed for CPLD device\n");
		platform_driver_unregister(&cpld_driver);
		return -ENOMEM;
	}

	ret = platform_device_add(cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for CPLD device\n");
		platform_device_put(cpld_device);
		return ret;
	}

	pr_info("CPLD driver loaded\n");
	return 0;
}

static void cpld_exit(void)
{
	platform_driver_unregister(&cpld_driver);
	platform_device_unregister(cpld_device);
	pr_err("CPLD driver unloaded\n");
}

/*
 * Module init and exit for all I2C devices, including GPIO
 */

static void free_all(void)
{
	cpld_exit();
	i2c_exit();
	pr_info(DRIVER_NAME " driver unloaded\n");
}

static int __init as7816_64x_platform_init(void)
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

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit as7816_64x_platform_exit(void)
{
	free_all();
	pr_info(DRIVER_NAME " driver unloaded\n");
}

module_init(as7816_64x_platform_init);
module_exit(as7816_64x_platform_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton AS7816_64X Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
