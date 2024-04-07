/*
 * accton-as5916-54xl-platform.c - Accton AS5916-54XL Platform Support.
 *
 * Copyright (C) 2019 Cumulus Networks, Inc.
 * Author: Alok Kumar <alok@cumulusnetworks.com>
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
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "accton-as5916-54xl-cpld.h"
#include <linux/cumulus-platform.h>

#define DRIVER_NAME	"accton_as5916_54xl_platform"
#define DRIVER_VERSION	"1.0"

/*
 * The platform has a intel i801 i2c SMBUS (Intel 82801
 * (ICH/PCH)). I801 has a PCA9548 mux.
 */

/* i2c bus adapter numbers for the 8 down stream PCA9548 i2c busses */
enum {
	AS5916_I2C_I801_BUS = 0,
	AS5916_I2C_MUX1_BUS0 = 10,
	AS5916_I2C_MUX1_BUS1,
	AS5916_I2C_MUX1_BUS2,
	AS5916_I2C_MUX1_BUS3,
	AS5916_I2C_MUX1_BUS4,
	AS5916_I2C_MUX1_BUS5,
	AS5916_I2C_MUX1_BUS6,
	AS5916_I2C_MUX1_BUS7,
	AS5916_I2C_MUX2_BUS0 = 20,
	AS5916_I2C_MUX2_BUS1,
	AS5916_I2C_MUX2_BUS2,
	AS5916_I2C_MUX2_BUS3,
	AS5916_I2C_MUX2_BUS4,
	AS5916_I2C_MUX2_BUS5,
	AS5916_I2C_MUX2_BUS6,
	AS5916_I2C_MUX2_BUS7,
	AS5916_I2C_MUX3_BUS0 = 30,
	AS5916_I2C_MUX3_BUS1,
	AS5916_I2C_MUX3_BUS2,
	AS5916_I2C_MUX3_BUS3,
	AS5916_I2C_MUX3_BUS4,
	AS5916_I2C_MUX3_BUS5,
	AS5916_I2C_MUX3_BUS6,
	AS5916_I2C_MUX3_BUS7,
	AS5916_I2C_MUX4_BUS0 = 40,
	AS5916_I2C_MUX4_BUS1,
	AS5916_I2C_MUX4_BUS2,
	AS5916_I2C_MUX4_BUS3,
	AS5916_I2C_MUX4_BUS4,
	AS5916_I2C_MUX4_BUS5,
	AS5916_I2C_MUX4_BUS6,
	AS5916_I2C_MUX4_BUS7,
	AS5916_I2C_MUX5_BUS0 = 50,
	AS5916_I2C_MUX5_BUS1,
	AS5916_I2C_MUX5_BUS2,
	AS5916_I2C_MUX5_BUS3,
	AS5916_I2C_MUX5_BUS4,
	AS5916_I2C_MUX5_BUS5,
	AS5916_I2C_MUX5_BUS6,
	AS5916_I2C_MUX5_BUS7,
	AS5916_I2C_MUX6_BUS0 = 60,
	AS5916_I2C_MUX6_BUS1,
	AS5916_I2C_MUX6_BUS2,
	AS5916_I2C_MUX6_BUS3,
	AS5916_I2C_MUX6_BUS4,
	AS5916_I2C_MUX6_BUS5,
	AS5916_I2C_MUX6_BUS6,
	AS5916_I2C_MUX6_BUS7,
	AS5916_I2C_MUX7_BUS0 = 70,
	AS5916_I2C_MUX7_BUS1,
	AS5916_I2C_MUX7_BUS2,
	AS5916_I2C_MUX7_BUS3,
	AS5916_I2C_MUX7_BUS4,
	AS5916_I2C_MUX7_BUS5,
	AS5916_I2C_MUX7_BUS6,
	AS5916_I2C_MUX7_BUS7,
	AS5916_I2C_MUX8_BUS0 = 80,
	AS5916_I2C_MUX8_BUS1,
	AS5916_I2C_MUX8_BUS2,
	AS5916_I2C_MUX8_BUS3,
	AS5916_I2C_MUX8_BUS4,
	AS5916_I2C_MUX8_BUS5,
	AS5916_I2C_MUX8_BUS6,
	AS5916_I2C_MUX8_BUS7,
	AS5916_I2C_MUX9_BUS0 = 90,
	AS5916_I2C_MUX9_BUS1,
	AS5916_I2C_MUX9_BUS2,
	AS5916_I2C_MUX9_BUS3,
	AS5916_I2C_MUX9_BUS4,
	AS5916_I2C_MUX9_BUS5,
	AS5916_I2C_MUX9_BUS6,
	AS5916_I2C_MUX9_BUS7,
	AS5916_I2C_MUX10_BUS0 = 100,
	AS5916_I2C_MUX10_BUS1,
	AS5916_I2C_MUX10_BUS2,
	AS5916_I2C_MUX10_BUS3,
	AS5916_I2C_MUX10_BUS4,
	AS5916_I2C_MUX10_BUS5,
	AS5916_I2C_MUX10_BUS6,
	AS5916_I2C_MUX10_BUS7,
	AS5916_I2C_MUX11_BUS0 = 110,
	AS5916_I2C_MUX11_BUS1,
	AS5916_I2C_MUX11_BUS2,
	AS5916_I2C_MUX11_BUS3,
	AS5916_I2C_MUX11_BUS4,
	AS5916_I2C_MUX11_BUS5,
	AS5916_I2C_MUX11_BUS6,
	AS5916_I2C_MUX11_BUS7,

};

mk_eeprom(psu0, 38, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu1, 3b, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu0_3y, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu1_3y, 53, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(board, 55, 256, AT24_FLAG_IRUGO);
mk_port_eeprom(port0,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port1,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port2,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port3,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port4,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port5,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port6,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port7,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port8,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port9,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port10,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port11,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port12,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port13,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port14,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port15,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port16,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port17,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port18,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port19,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port20,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port21,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port22,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port23,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port24,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port25,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port26,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port27,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port28,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port29,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port30,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port31,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port32,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port33,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port34,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port35,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port36,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port37,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port38,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port39,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port40,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port41,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port42,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port43,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port44,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port45,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port46,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port47,	50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_qsfp_port_eeprom(port48,  50, 512,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port49,  50, 512,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port50,  50, 512,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port51,  50, 512,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port52,  50, 512,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port53,  50, 512,  SFF_8436_FLAG_IRUGO);

mk_pca9548(mux1,   AS5916_I2C_MUX1_BUS0,   1);
mk_pca9548(mux2,   AS5916_I2C_MUX2_BUS0,   1);
mk_pca9548(mux3,   AS5916_I2C_MUX3_BUS0,   1);
mk_pca9548(mux4,   AS5916_I2C_MUX4_BUS0,   1);
mk_pca9548(mux5,   AS5916_I2C_MUX5_BUS0,   1);
mk_pca9548(mux6,   AS5916_I2C_MUX6_BUS0,   1);
mk_pca9548(mux7,   AS5916_I2C_MUX7_BUS0,   1);
mk_pca9548(mux8,   AS5916_I2C_MUX8_BUS0,   1);
mk_pca9548(mux9,   AS5916_I2C_MUX9_BUS0,   1);
mk_pca9548(mux10,  AS5916_I2C_MUX10_BUS0,  1);
mk_pca9548(mux11,  AS5916_I2C_MUX11_BUS0,  1);

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(AS5916_I2C_I801_BUS,	"pca9548", 0x77, &mux1_platform_data),
	mk_i2cdev(AS5916_I2C_MUX1_BUS0,	"pca9548", 0x76, &mux2_platform_data),
	mk_i2cdev(AS5916_I2C_MUX1_BUS1,	"pca9548", 0x74, &mux3_platform_data),
	mk_i2cdev(AS5916_I2C_MUX1_BUS1,	"pca9548", 0x72, &mux4_platform_data),
	mk_i2cdev(AS5916_I2C_MUX2_BUS0,   "cpld", 0x66, NULL), /* fan cpld */
	mk_i2cdev(AS5916_I2C_MUX2_BUS1,   "lm75", 0x48, NULL),
	mk_i2cdev(AS5916_I2C_MUX2_BUS1,   "lm75", 0x49, NULL),
	mk_i2cdev(AS5916_I2C_MUX2_BUS1,   "lm75", 0x4a, NULL),
	mk_i2cdev(AS5916_I2C_MUX2_BUS1,   "lm75", 0x4b, NULL),
	mk_i2cdev(AS5916_I2C_MUX2_BUS2,   "cpld", 0x60, NULL), /* port cpld1 */
	mk_i2cdev(AS5916_I2C_MUX2_BUS3,   "cpld", 0x62, NULL), /* port cpld2 */
	mk_i2cdev(AS5916_I2C_MUX2_BUS5,	"24c02", 0x57, &board_55_at24),

	mk_i2cdev(AS5916_I2C_MUX3_BUS0, "24c02", 0x38, &psu0_38_at24),
	mk_i2cdev(AS5916_I2C_MUX3_BUS0, "cpr4011", 0x3c, NULL),
	mk_i2cdev(AS5916_I2C_MUX3_BUS1, "24c02", 0x3b, &psu1_3b_at24),
	mk_i2cdev(AS5916_I2C_MUX3_BUS1, "cpr4011", 0x3f, NULL),
	mk_i2cdev(AS5916_I2C_MUX3_BUS0, "24c02", 0x50, &psu0_3y_50_at24),
	mk_i2cdev(AS5916_I2C_MUX3_BUS0, "cpr4011", 0x58, NULL),
	mk_i2cdev(AS5916_I2C_MUX3_BUS1, "24c02", 0x53, &psu1_3y_53_at24),
	mk_i2cdev(AS5916_I2C_MUX3_BUS1, "cpr4011", 0x5b, NULL),

	mk_i2cdev(AS5916_I2C_MUX4_BUS0, "pca9548", 0x75, &mux5_platform_data),
	mk_i2cdev(AS5916_I2C_MUX4_BUS1, "pca9548", 0x75, &mux6_platform_data),
	mk_i2cdev(AS5916_I2C_MUX4_BUS2, "pca9548", 0x75, &mux7_platform_data),
	mk_i2cdev(AS5916_I2C_MUX4_BUS3, "pca9548", 0x75, &mux8_platform_data),
	mk_i2cdev(AS5916_I2C_MUX4_BUS4, "pca9548", 0x75, &mux9_platform_data),
	mk_i2cdev(AS5916_I2C_MUX4_BUS5, "pca9548", 0x75, &mux10_platform_data),
	mk_i2cdev(AS5916_I2C_MUX4_BUS6, "pca9548", 0x75, &mux11_platform_data),

	mk_i2cdev(AS5916_I2C_MUX5_BUS0, "24c04", 0x50, &port0_50_at24),
	mk_i2cdev(AS5916_I2C_MUX5_BUS1, "24c04", 0x50, &port1_50_at24),
	mk_i2cdev(AS5916_I2C_MUX5_BUS2, "24c04", 0x50, &port2_50_at24),
	mk_i2cdev(AS5916_I2C_MUX5_BUS3, "24c04", 0x50, &port3_50_at24),
	mk_i2cdev(AS5916_I2C_MUX5_BUS4, "24c04", 0x50, &port4_50_at24),
	mk_i2cdev(AS5916_I2C_MUX5_BUS5, "24c04", 0x50, &port5_50_at24),
	mk_i2cdev(AS5916_I2C_MUX5_BUS6, "24c04", 0x50, &port6_50_at24),
	mk_i2cdev(AS5916_I2C_MUX5_BUS7, "24c04", 0x50, &port7_50_at24),

	mk_i2cdev(AS5916_I2C_MUX6_BUS0, "24c04", 0x50, &port8_50_at24),
	mk_i2cdev(AS5916_I2C_MUX6_BUS1, "24c04", 0x50, &port9_50_at24),
	mk_i2cdev(AS5916_I2C_MUX6_BUS2, "24c04", 0x50, &port10_50_at24),
	mk_i2cdev(AS5916_I2C_MUX6_BUS3, "24c04", 0x50, &port11_50_at24),
	mk_i2cdev(AS5916_I2C_MUX6_BUS4, "24c04", 0x50, &port12_50_at24),
	mk_i2cdev(AS5916_I2C_MUX6_BUS5, "24c04", 0x50, &port13_50_at24),
	mk_i2cdev(AS5916_I2C_MUX6_BUS6, "24c04", 0x50, &port14_50_at24),
	mk_i2cdev(AS5916_I2C_MUX6_BUS7, "24c04", 0x50, &port15_50_at24),

	mk_i2cdev(AS5916_I2C_MUX7_BUS0, "24c04", 0x50, &port16_50_at24),
	mk_i2cdev(AS5916_I2C_MUX7_BUS1, "24c04", 0x50, &port17_50_at24),
	mk_i2cdev(AS5916_I2C_MUX7_BUS2, "24c04", 0x50, &port18_50_at24),
	mk_i2cdev(AS5916_I2C_MUX7_BUS3, "24c04", 0x50, &port19_50_at24),
	mk_i2cdev(AS5916_I2C_MUX7_BUS4, "24c04", 0x50, &port20_50_at24),
	mk_i2cdev(AS5916_I2C_MUX7_BUS5, "24c04", 0x50, &port21_50_at24),
	mk_i2cdev(AS5916_I2C_MUX7_BUS6, "24c04", 0x50, &port22_50_at24),
	mk_i2cdev(AS5916_I2C_MUX7_BUS7, "24c04", 0x50, &port23_50_at24),

	mk_i2cdev(AS5916_I2C_MUX8_BUS0, "24c04", 0x50, &port24_50_at24),
	mk_i2cdev(AS5916_I2C_MUX8_BUS1, "24c04", 0x50, &port25_50_at24),
	mk_i2cdev(AS5916_I2C_MUX8_BUS2, "24c04", 0x50, &port26_50_at24),
	mk_i2cdev(AS5916_I2C_MUX8_BUS3, "24c04", 0x50, &port27_50_at24),
	mk_i2cdev(AS5916_I2C_MUX8_BUS4, "24c04", 0x50, &port28_50_at24),
	mk_i2cdev(AS5916_I2C_MUX8_BUS5, "24c04", 0x50, &port29_50_at24),
	mk_i2cdev(AS5916_I2C_MUX8_BUS6, "24c04", 0x50, &port30_50_at24),
	mk_i2cdev(AS5916_I2C_MUX8_BUS7, "24c04", 0x50, &port31_50_at24),

	mk_i2cdev(AS5916_I2C_MUX9_BUS0, "24c04", 0x50, &port32_50_at24),
	mk_i2cdev(AS5916_I2C_MUX9_BUS1, "24c04", 0x50, &port33_50_at24),
	mk_i2cdev(AS5916_I2C_MUX9_BUS2, "24c04", 0x50, &port34_50_at24),
	mk_i2cdev(AS5916_I2C_MUX9_BUS3, "24c04", 0x50, &port35_50_at24),
	mk_i2cdev(AS5916_I2C_MUX9_BUS4, "24c04", 0x50, &port36_50_at24),
	mk_i2cdev(AS5916_I2C_MUX9_BUS5, "24c04", 0x50, &port37_50_at24),
	mk_i2cdev(AS5916_I2C_MUX9_BUS6, "24c04", 0x50, &port38_50_at24),
	mk_i2cdev(AS5916_I2C_MUX9_BUS7, "24c04", 0x50, &port39_50_at24),

	mk_i2cdev(AS5916_I2C_MUX10_BUS0, "24c04", 0x50, &port40_50_at24),
	mk_i2cdev(AS5916_I2C_MUX10_BUS1, "24c04", 0x50, &port41_50_at24),
	mk_i2cdev(AS5916_I2C_MUX10_BUS2, "24c04", 0x50, &port42_50_at24),
	mk_i2cdev(AS5916_I2C_MUX10_BUS3, "24c04", 0x50, &port43_50_at24),
	mk_i2cdev(AS5916_I2C_MUX10_BUS4, "24c04", 0x50, &port44_50_at24),
	mk_i2cdev(AS5916_I2C_MUX10_BUS5, "24c04", 0x50, &port45_50_at24),
	mk_i2cdev(AS5916_I2C_MUX10_BUS6, "24c04", 0x50, &port46_50_at24),
	mk_i2cdev(AS5916_I2C_MUX10_BUS7, "24c04", 0x50, &port47_50_at24),

	mk_i2cdev(AS5916_I2C_MUX11_BUS0, "sff8436", 0x50, &port48_50_sff8436),
	mk_i2cdev(AS5916_I2C_MUX11_BUS1, "sff8436", 0x50, &port49_50_sff8436),
	mk_i2cdev(AS5916_I2C_MUX11_BUS2, "sff8436", 0x50, &port50_50_sff8436),
	mk_i2cdev(AS5916_I2C_MUX11_BUS3, "sff8436", 0x50, &port51_50_sff8436),
	mk_i2cdev(AS5916_I2C_MUX11_BUS4, "sff8436", 0x50, &port52_50_sff8436),
	mk_i2cdev(AS5916_I2C_MUX11_BUS5, "sff8436", 0x50, &port53_50_sff8436),

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
		pr_err(DRIVER_NAME": could not find adapter %s\n", SMBUS_I801_NAME);
		ret = -ENODEV;
		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == AS5916_I2C_I801_BUS)
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
		pr_err(DRIVER_NAME ": Attempt to read invalid CPLD register: 0x%02X\n",
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
	int ret;
	int cpld_id = GET_CPLD_ID(reg);

	if (cpld_id < 0 || cpld_id >= NUM_CPLD_DEVICES) {
		pr_err(DRIVER_NAME ": Attempt to write to invalid CPLD register: 0x%02X\n",
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

static const char * const fan_direction_values[] = {
	"F2B",
	"B2F",
};

static const char * const led_locator_values[] = {
	PLATFORM_LED_ON,
	PLATFORM_LED_OFF,
};

static const char * const led_values[] = {
	PLATFORM_LED_HW_CTRL, /* both green and amber or on */
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_OFF,
};

static const char * const led_diag_values[] = {
	PLATFORM_LED_HW_CTRL, /* both green and amber or on */
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_OFF,
};

/* CPLD registers */
mk_bf_ro(cpld, fan_board_version,	   FAN_BOARD_INFO_REG, 5, 3,
	 NULL, 0);
mk_bf_ro(cpld, fan_board_id,		   FAN_BOARD_INFO_REG, 0, 2,
	 NULL, 0);
mk_bf_ro(cpld, fan_cpld_version,	   FAN_CPLD_VERSION_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, fan_reset_cpld,		   FAN_CPLD_RESET_REG, 7, 1,
	 NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, fantray0_present,	   FAN_MODULE_PRESENT_REG, 0, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fantray1_present,	   FAN_MODULE_PRESENT_REG, 1, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fantray2_present,	   FAN_MODULE_PRESENT_REG, 2, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fantray3_present,	   FAN_MODULE_PRESENT_REG, 3, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fantray4_present,	   FAN_MODULE_PRESENT_REG, 4, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fantray5_present,	   FAN_MODULE_PRESENT_REG, 5, 1,
	 NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, fantray0_direction,	 FAN_MODULE_DIRECTION_REG, 0, 1,
	 fan_direction_values, 0);
mk_bf_ro(cpld, fantray1_direction,	 FAN_MODULE_DIRECTION_REG, 1, 1,
	 fan_direction_values, 0);
mk_bf_ro(cpld, fantray2_direction,	 FAN_MODULE_DIRECTION_REG, 2, 1,
	 fan_direction_values, 0);
mk_bf_ro(cpld, fantray3_direction,	 FAN_MODULE_DIRECTION_REG, 3, 1,
	 fan_direction_values, 0);
mk_bf_ro(cpld, fantray4_direction,	 FAN_MODULE_DIRECTION_REG, 4, 1,
	 fan_direction_values, 0);
mk_bf_ro(cpld, fantray5_direction,	 FAN_MODULE_DIRECTION_REG, 5, 1,
	 fan_direction_values, 0);

mk_bf_rw(cpld, fan_pwm,		FAN_MODULE_PWM_REG, 0, 4,
	 NULL, 0);

mk_bf_rw(cpld, fantray0_led_red,	   FAN_LED_REG1, 7, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray0_led_green,	 FAN_LED_REG1, 6, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray1_led_red,	   FAN_LED_REG1, 5, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray1_led_green,	 FAN_LED_REG1, 4, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray2_led_red,	   FAN_LED_REG1, 3, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray2_led_green,	 FAN_LED_REG1, 2, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray3_led_red,	   FAN_LED_REG1, 1, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray3_led_green,	 FAN_LED_REG1, 0, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray4_led_red,	   FAN_LED_REG2, 3, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray4_led_green,	 FAN_LED_REG2, 2, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray5_led_red,	   FAN_LED_REG2, 1, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray5_led_green,	 FAN_LED_REG2, 0, 1,
	 NULL, 0);

mk_bf_rw(cpld, fantray0_enable,		FAN_MODULE_POWER_ENABLE_REG, 5, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray1_enable,		FAN_MODULE_POWER_ENABLE_REG, 4, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray2_enable,		FAN_MODULE_POWER_ENABLE_REG, 3, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray3_enable,		FAN_MODULE_POWER_ENABLE_REG, 2, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray4_enable,		FAN_MODULE_POWER_ENABLE_REG, 1, 1,
	 NULL, 0);
mk_bf_rw(cpld, fantray5_enable,		FAN_MODULE_POWER_ENABLE_REG, 0, 1,
	 NULL, 0);

mk_bf_ro(cpld, board_id,			   CPLD1_BOARD_INFO_REG, 3, 3,
	 NULL, 0);
mk_bf_ro(cpld, pcb_version,			   CPLD1_BOARD_INFO_REG, 0, 3,
	 NULL, 0);
mk_bf_ro(cpld, cpld1_version,		   CPLD1_CODE_VERSION_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, psu_pwr1_alarm,		   CPLD1_POWER_MODULE_STATUS_REG, 5, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr0_alarm,		   CPLD1_POWER_MODULE_STATUS_REG, 4, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr1_all_ok,		   CPLD1_POWER_MODULE_STATUS_REG, 3, 1,
	 NULL, 0);
mk_bf_ro(cpld, psu_pwr0_all_ok,		   CPLD1_POWER_MODULE_STATUS_REG, 2, 1,
	 NULL, 0);
mk_bf_ro(cpld, psu_pwr1_present,	   CPLD1_POWER_MODULE_STATUS_REG, 1, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr0_present,	   CPLD1_POWER_MODULE_STATUS_REG, 0, 1,
	 NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, sfp_7_0_present,		   CPLD1_MODULE_PRESENT_1_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, sfp_15_8_present,	   CPLD1_MODULE_PRESENT_2_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, sfp_23_16_present,	   CPLD1_MODULE_PRESENT_3_REG, 0, 8,
	 NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, sfp_7_0_tx_fault,	   CPLD1_MODULE_TX_FAULT_1_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp_15_8_tx_fault,	   CPLD1_MODULE_TX_FAULT_2_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp_23_16_tx_fault,	   CPLD1_MODULE_TX_FAULT_3_REG, 0, 8,
	 NULL, 0);

mk_bf_rw(cpld, sfp_7_0_tx_disable,	   CPLD1_MODULE_TX_DIS_1_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp_15_8_tx_disable,	   CPLD1_MODULE_TX_DIS_2_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp_23_16_tx_disable,   CPLD1_MODULE_TX_DIS_3_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, sfp_7_0_rx_loss,		   CPLD1_MODULE_RX_LOSS_1_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp_15_8_rx_loss,	   CPLD1_MODULE_RX_LOSS_2_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp_23_16_rx_loss,	   CPLD1_MODULE_RX_LOSS_3_REG, 0, 8,
	 NULL, 0);

mk_bf_rw(cpld, cpld_reset,             CPLD1_SYSTEM_RESET_5_REG, 2, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, mgmt_phy_reset,		   CPLD1_SYSTEM_RESET_5_REG, 0, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_11_reset,	   CPLD1_SYSTEM_RESET_8_REG, 6, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_10_reset,	   CPLD1_SYSTEM_RESET_8_REG, 5, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_9_reset,		   CPLD1_SYSTEM_RESET_8_REG, 4, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_8_reset,		   CPLD1_SYSTEM_RESET_8_REG, 3, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_7_reset,		   CPLD1_SYSTEM_RESET_8_REG, 2, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_6_reset,		   CPLD1_SYSTEM_RESET_8_REG, 1, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_5_reset,		   CPLD1_SYSTEM_RESET_8_REG, 0, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_4_reset,		   CPLD1_SYSTEM_RESET_7_REG, 1, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_3_reset,		   CPLD1_SYSTEM_RESET_7_REG, 0, 1,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, pca9548_2_reset,		   CPLD1_SYSTEM_RESET_7_REG, 5, 1,
	 NULL, BF_COMPLEMENT);

mk_bf_rw(cpld, led_locator,			   CPLD1_SYSTEM_LED_1_REG, 4, 1,
	 led_locator_values, 0);
mk_bf_rw(cpld, led_fan,				   CPLD1_SYSTEM_LED_1_REG, 0, 2,
	 led_values, 0);
mk_bf_rw(cpld, led_diag,			   CPLD1_SYSTEM_LED_1_REG, 2, 2,
	 led_diag_values, 0);
mk_bf_rw(cpld, led_psu1,			   CPLD1_SYSTEM_LED_2_REG, 3, 2,
	 led_values, 0);
mk_bf_rw(cpld, led_psu0,			   CPLD1_SYSTEM_LED_2_REG, 5, 2,
	 led_values, 0);


mk_bf_ro(cpld, cpld2_version,		   CPLD2_CODE_VERSION_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, sfp_31_24_present,	   CPLD2_MODULE_PRESENT_4_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, sfp_39_32_present,	   CPLD2_MODULE_PRESENT_5_REG, 0, 8,
	 NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, sfp_47_40_present,	   CPLD2_MODULE_PRESENT_6_REG, 0, 8,
	 NULL, BF_COMPLEMENT);

mk_bf_ro(cpld, sfp_31_24_tx_fault,	   CPLD2_MODULE_TX_FAULT_4_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp_39_32_tx_fault,	   CPLD2_MODULE_TX_FAULT_5_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp_47_40_tx_fault,	   CPLD2_MODULE_TX_FAULT_6_REG, 0, 8,
	 NULL, 0);

mk_bf_rw(cpld, sfp_31_24_tx_disable,   CPLD2_MODULE_TX_DIS_4_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp_39_32_tx_disable,   CPLD2_MODULE_TX_DIS_5_REG, 0, 8,
	 NULL, 0);
mk_bf_rw(cpld, sfp_47_40_tx_disable,   CPLD2_MODULE_TX_DIS_6_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, sfp_31_24_rx_loss,	   CPLD2_MODULE_RX_LOSS_4_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp_39_32_rx_loss,	   CPLD2_MODULE_RX_LOSS_5_REG, 0, 8,
	 NULL, 0);
mk_bf_ro(cpld, sfp_47_40_rx_loss,	   CPLD2_MODULE_RX_LOSS_6_REG, 0, 8,
	 NULL, 0);

mk_bf_ro(cpld, qsfp28_5_0_present,	   CPLD2_QSFP_PRESENT_REG, 0, 6,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_5_0_reset,	   CPLD2_QSFP_MOD_RST_REG, 0, 6,
	 NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_5_0_lpmode,	   CPLD2_QSFP_LPMODE_REG, 0, 6,
	 NULL, 0);


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
		pr_err(DRIVER_NAME ": CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
			   cpld_devices[cpld_id]->addr, base_reg);
		return -EINVAL;
	}
	/* From Accton hardware spec the RPM is 100 times the register value */
	return sprintf(buf, "%d\n", val*100);
}

static SENSOR_DEVICE_ATTR_RO(fan0_input, fan_show,
				 FAN_FRONT_FAN1_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan1_input, fan_show,
				 FAN_FRONT_FAN2_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan2_input, fan_show,
				 FAN_FRONT_FAN3_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan3_input, fan_show,
				 FAN_FRONT_FAN4_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan4_input, fan_show,
		 FAN_FRONT_FAN5_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan5_input, fan_show,
		 FAN_FRONT_FAN6_MODULE_SPEED_REG);

static SENSOR_DEVICE_ATTR_RO(fan6_input, fan_show,
				 FAN_REAR_FAN1_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan7_input, fan_show,
				 FAN_REAR_FAN2_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan8_input, fan_show,
				 FAN_REAR_FAN3_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan9_input, fan_show,
				 FAN_REAR_FAN4_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan10_input, fan_show,
		 FAN_REAR_FAN5_MODULE_SPEED_REG);
static SENSOR_DEVICE_ATTR_RO(fan11_input, fan_show,
		 FAN_REAR_FAN6_MODULE_SPEED_REG);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_fan_board_version.attr,
	&cpld_fan_board_id.attr,
	&cpld_fan_cpld_version.attr,
	&cpld_fan_reset_cpld.attr,
	&cpld_fantray0_present.attr,
	&cpld_fantray1_present.attr,
	&cpld_fantray2_present.attr,
	&cpld_fantray3_present.attr,
	&cpld_fantray4_present.attr,
	&cpld_fantray5_present.attr,
	&cpld_fantray0_direction.attr,
	&cpld_fantray1_direction.attr,
	&cpld_fantray2_direction.attr,
	&cpld_fantray3_direction.attr,
	&cpld_fantray4_direction.attr,
	&cpld_fantray5_direction.attr,
	&cpld_fan_pwm.attr,
	&cpld_fantray0_led_red.attr,
	&cpld_fantray0_led_green.attr,
	&cpld_fantray1_led_red.attr,
	&cpld_fantray1_led_green.attr,
	&cpld_fantray2_led_red.attr,
	&cpld_fantray2_led_green.attr,
	&cpld_fantray3_led_red.attr,
	&cpld_fantray3_led_green.attr,
	&cpld_fantray4_led_red.attr,
	&cpld_fantray4_led_green.attr,
	&cpld_fantray5_led_red.attr,
	&cpld_fantray5_led_green.attr,
	&cpld_fantray0_enable.attr,
	&cpld_fantray1_enable.attr,
	&cpld_fantray2_enable.attr,
	&cpld_fantray3_enable.attr,
	&cpld_fantray4_enable.attr,
	&cpld_fantray5_enable.attr,
	&cpld_board_id.attr,
	&cpld_pcb_version.attr,
	&cpld_cpld1_version.attr,
	&cpld_psu_pwr1_alarm.attr,
	&cpld_psu_pwr0_alarm.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_psu_pwr0_all_ok.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr0_present.attr,
	&cpld_sfp_7_0_present.attr,
	&cpld_sfp_15_8_present.attr,
	&cpld_sfp_23_16_present.attr,
	&cpld_sfp_7_0_tx_fault.attr,
	&cpld_sfp_15_8_tx_fault.attr,
	&cpld_sfp_23_16_tx_fault.attr,
	&cpld_sfp_7_0_tx_disable.attr,
	&cpld_sfp_15_8_tx_disable.attr,
	&cpld_sfp_23_16_tx_disable.attr,
	&cpld_sfp_7_0_rx_loss.attr,
	&cpld_sfp_15_8_rx_loss.attr,
	&cpld_sfp_23_16_rx_loss.attr,
	&cpld_cpld_reset.attr,
	&cpld_mgmt_phy_reset.attr,
	&cpld_pca9548_11_reset.attr,
	&cpld_pca9548_10_reset.attr,
	&cpld_pca9548_9_reset.attr,
	&cpld_pca9548_8_reset.attr,
	&cpld_pca9548_7_reset.attr,
	&cpld_pca9548_6_reset.attr,
	&cpld_pca9548_5_reset.attr,
	&cpld_pca9548_4_reset.attr,
	&cpld_pca9548_3_reset.attr,
	&cpld_pca9548_2_reset.attr,
	&cpld_led_locator.attr,
	&cpld_led_fan.attr,
	&cpld_led_diag.attr,
	&cpld_led_psu1.attr,
	&cpld_led_psu0.attr,
	&cpld_cpld2_version.attr,
	&cpld_sfp_31_24_present.attr,
	&cpld_sfp_39_32_present.attr,
	&cpld_sfp_47_40_present.attr,
	&cpld_sfp_31_24_tx_fault.attr,
	&cpld_sfp_39_32_tx_fault.attr,
	&cpld_sfp_47_40_tx_fault.attr,
	&cpld_sfp_31_24_tx_disable.attr,
	&cpld_sfp_39_32_tx_disable.attr,
	&cpld_sfp_47_40_tx_disable.attr,
	&cpld_sfp_31_24_rx_loss.attr,
	&cpld_sfp_39_32_rx_loss.attr,
	&cpld_sfp_47_40_rx_loss.attr,
	&cpld_qsfp28_5_0_present.attr,
	&cpld_qsfp28_5_0_reset.attr,
	&cpld_qsfp28_5_0_lpmode.attr,
	&sensor_dev_attr_fan0_input.dev_attr.attr,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,
	&sensor_dev_attr_fan9_input.dev_attr.attr,
	&sensor_dev_attr_fan10_input.dev_attr.attr,
	&sensor_dev_attr_fan11_input.dev_attr.attr,

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
		pr_err(DRIVER_NAME ": Failed to create sysfs group for cpld driver\n");

	return ret;
}

static int cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver cpld_driver = {
	.driver = {
		.name = "accton_as5916_54xl_cpld",
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
		pr_err(DRIVER_NAME ": platform_driver_register() failed for CPLD device\n");
		return ret;
	}

	cpld_device = platform_device_alloc("accton_as5916_54xl_cpld", 0);
	if (!cpld_device) {
		pr_err(DRIVER_NAME ": platform_device_alloc() failed for CPLD device\n");
		platform_driver_unregister(&cpld_driver);
		return -ENOMEM;
	}

	ret = platform_device_add(cpld_device);
	if (ret) {
		pr_err(DRIVER_NAME ": platform_device_add() failed for CPLD device\n");
		platform_device_put(cpld_device);
		return ret;
	}

	pr_info(DRIVER_NAME ": CPLD driver loaded\n");
	return 0;
}

static void cpld_exit(void)
{
	platform_driver_unregister(&cpld_driver);
	platform_device_unregister(cpld_device);
	pr_err(DRIVER_NAME ": CPLD driver unloaded\n");
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

static int __init as5916_54xl_platform_init(void)
{
	int ret;

	ret = i2c_init();
	if (ret) {
		pr_err(DRIVER_NAME ": I2C subsystem initialization failed\n");
		return ret;
	}

	ret = cpld_init();
	if (ret) {
		pr_err(DRIVER_NAME ": CPLD initialization failed\n");
		return ret;
	}

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;
}

static void __exit as5916_54xl_platform_exit(void)
{
	free_all();
	pr_info(DRIVER_NAME " driver unloaded\n");
}

module_init(as5916_54xl_platform_init);
module_exit(as5916_54xl_platform_exit);

MODULE_AUTHOR("Alok kumar (alok@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton AS5916_54XL Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
