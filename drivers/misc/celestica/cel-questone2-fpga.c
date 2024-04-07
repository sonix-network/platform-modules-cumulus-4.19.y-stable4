/*
 * Celestica Questone2 FPGA Support
 *
 * Copyright (c) 2019 Cumulus Networks, Inc.  All rights reserved.
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
#include <linux/interrupt.h>
#include <linux/stddef.h>
#include <linux/acpi.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "platform-bitfield.h"
#include "cel-fpga-i2c.h"
#include "cel-sea2que2.h"

#define DRIVER_NAME	"cel_questone2_fpga"
#define DRIVER_VERSION	"2.0"

#define NUM_FPGA_BUSSES		     10
#define NUM_CPLD_DEVICES	     2
#define CPLD1_ID		     0
#define CPLD2_ID		     1

#define CPLD_IDX_SHIFT		     8
#define CPLD_IDX_MASK		     BIT(CPLD_IDX_SHIFT)
#define CPLD1_IDX		     (CPLD1_ID << CPLD_IDX_SHIFT)
#define CPLD2_IDX		     (CPLD2_ID << CPLD_IDX_SHIFT)

#define GET_CPLD_ID(A)		     ((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT)
#define STRIP_CPLD_IDX(A)	     (A & ~CPLD_IDX_MASK)

#define FIRST_SFP_PORT		     1
#define NUM_SFP_PORTS		     48
#define FIRST_QSFP_PORT		     49
#define NUM_QSFP_PORTS		     8
#define CPLD1_MAX_PORT		     24
#define CPLD2_MAX_PORT		     56

/*
 * The FPGA on this platform has a number of I2C cores that connect to the
 * front panel ports and two CPLDs.
 */

enum {
	FPGA_I2C_CH1 = 10,
	FPGA_I2C_CH2,
	FPGA_I2C_CH3,
	FPGA_I2C_CH4,
	FPGA_I2C_CH5,
	FPGA_I2C_CH6,
	FPGA_I2C_CH7,
	FPGA_I2C_CH8,
	FPGA_I2C_CH9,
	FPGA_I2C_CH10,

	FPGA_I2C_CH10_MUX0_BUS0 = 21,
	FPGA_I2C_CH10_MUX0_BUS1,
	FPGA_I2C_CH10_MUX0_BUS2,
	FPGA_I2C_CH10_MUX0_BUS3,
	FPGA_I2C_CH10_MUX0_BUS4,
	FPGA_I2C_CH10_MUX0_BUS5,
	FPGA_I2C_CH10_MUX0_BUS6,
	FPGA_I2C_CH10_MUX0_BUS7,

	FPGA_I2C_CH10_MUX1_BUS0,
	FPGA_I2C_CH10_MUX1_BUS1,
	FPGA_I2C_CH10_MUX1_BUS2,
	FPGA_I2C_CH10_MUX1_BUS3,
	FPGA_I2C_CH10_MUX1_BUS4,
	FPGA_I2C_CH10_MUX1_BUS5,
	FPGA_I2C_CH10_MUX1_BUS6,
	FPGA_I2C_CH10_MUX1_BUS7,

	FPGA_I2C_CH10_MUX2_BUS0,
	FPGA_I2C_CH10_MUX2_BUS1,
	FPGA_I2C_CH10_MUX2_BUS2,
	FPGA_I2C_CH10_MUX2_BUS3,
	FPGA_I2C_CH10_MUX2_BUS4,
	FPGA_I2C_CH10_MUX2_BUS5,
	FPGA_I2C_CH10_MUX2_BUS6,
	FPGA_I2C_CH10_MUX2_BUS7,

	FPGA_I2C_CH10_MUX3_BUS0,
	FPGA_I2C_CH10_MUX3_BUS1,
	FPGA_I2C_CH10_MUX3_BUS2,
	FPGA_I2C_CH10_MUX3_BUS3,
	FPGA_I2C_CH10_MUX3_BUS4,
	FPGA_I2C_CH10_MUX3_BUS5,
	FPGA_I2C_CH10_MUX3_BUS6,
	FPGA_I2C_CH10_MUX3_BUS7,

	FPGA_I2C_CH10_MUX4_BUS0,
	FPGA_I2C_CH10_MUX4_BUS1,
	FPGA_I2C_CH10_MUX4_BUS2,
	FPGA_I2C_CH10_MUX4_BUS3,
	FPGA_I2C_CH10_MUX4_BUS4,
	FPGA_I2C_CH10_MUX4_BUS5,
	FPGA_I2C_CH10_MUX4_BUS6,
	FPGA_I2C_CH10_MUX4_BUS7,

	FPGA_I2C_CH10_MUX5_BUS0,
	FPGA_I2C_CH10_MUX5_BUS1,
	FPGA_I2C_CH10_MUX5_BUS2,
	FPGA_I2C_CH10_MUX5_BUS3,
	FPGA_I2C_CH10_MUX5_BUS4,
	FPGA_I2C_CH10_MUX5_BUS5,
	FPGA_I2C_CH10_MUX5_BUS6,
	FPGA_I2C_CH10_MUX5_BUS7,

	FPGA_I2C_CH2_MUX0_BUS0,
	FPGA_I2C_CH2_MUX0_BUS1,
	FPGA_I2C_CH2_MUX0_BUS2,
	FPGA_I2C_CH2_MUX0_BUS3,
	FPGA_I2C_CH2_MUX0_BUS4,
	FPGA_I2C_CH2_MUX0_BUS5,
	FPGA_I2C_CH2_MUX0_BUS6,
	FPGA_I2C_CH2_MUX0_BUS7,
};

/*
 * The list of i2c devices and their bus connections for this platform.
 *
 * First we construct the necessary data struction for each device, using the
 * method specific to the device type.	Then we put them all together in a big
 * table (see i2c_devices below).
 *
 * For muxes, we specify the starting bus number for the block of ports, using
 * the magic mk_pca954*() macros.
 *
 * For eeproms, including ones in the sff transceivers, we specify the label,
 * i2c address, size, and some flags, all done in mk*_eeprom() macros.	The
 * label is the string that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at the user level.
 *
 */

mk_pca9548(fpga_ch10_0, FPGA_I2C_CH10_MUX0_BUS0, 1);
mk_pca9548(fpga_ch10_1, FPGA_I2C_CH10_MUX1_BUS0, 1);
mk_pca9548(fpga_ch10_2, FPGA_I2C_CH10_MUX2_BUS0, 1);
mk_pca9548(fpga_ch10_3, FPGA_I2C_CH10_MUX3_BUS0, 1);
mk_pca9548(fpga_ch10_4, FPGA_I2C_CH10_MUX4_BUS0, 1);
mk_pca9548(fpga_ch10_5, FPGA_I2C_CH10_MUX5_BUS0, 1);

mk_pca9548(fpga_ch2_0, FPGA_I2C_CH2_MUX0_BUS0, 1);

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
mk_qsfp_port_eeprom(port55, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port56, 50, 256, SFF_8436_FLAG_IRUGO);

/*
 * i2c device tables
 *
 * We use the magic mk_i2cdev() macro to construct the entries.	 Each entry is
 * a bus number and an i2c_board_info.	The i2c_board_info structure specifies
 * the device type, address, and platform data specific to the device type.
 *
 * The fpga_i2c_busses[] table contains just the stubs for the 10 fpga i2c
 * busses.  A separate structure, fpga_device_infotab[], is built and used to
 * communicate i2c device information to the fpga i2c driver in "slices" so
 * that it can manipulate the fpga registers as needed in order to access
 * devices on those busses.
 *
 * The fpga_i2c_devices[] table has all the devices exposed on the fpga i2c
 * busses.
 *
 */

static struct platform_i2c_device_info fpga_i2c_busses[] = {
	mk_i2cdev(FPGA_I2C_CH1,	 "", 0x0, NULL),
	mk_i2cdev(FPGA_I2C_CH2,	 "", 0x0, NULL),
	mk_i2cdev(FPGA_I2C_CH3,	 "", 0x0, NULL),
	mk_i2cdev(FPGA_I2C_CH4,	 "", 0x0, NULL),
	mk_i2cdev(FPGA_I2C_CH5,	 "", 0x0, NULL),
	mk_i2cdev(FPGA_I2C_CH6,	 "", 0x0, NULL),
	mk_i2cdev(FPGA_I2C_CH7,	 "", 0x0, NULL),
	mk_i2cdev(FPGA_I2C_CH8,	 "", 0x0, NULL),
	mk_i2cdev(FPGA_I2C_CH9,	 "", 0x0, NULL),
	mk_i2cdev(FPGA_I2C_CH10, "", 0x0, NULL),
};

static struct platform_i2c_device_info fpga_i2c_devices[] = {
	mk_i2cdev(FPGA_I2C_CH10, "pca9548", 0x72, &fpga_ch10_0_platform_data),
	mk_i2cdev(FPGA_I2C_CH10_MUX0_BUS0, "24c04", 0x50, &port1_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX0_BUS1, "24c04", 0x50, &port2_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX0_BUS2, "24c04", 0x50, &port3_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX0_BUS3, "24c04", 0x50, &port4_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX0_BUS4, "24c04", 0x50, &port5_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX0_BUS5, "24c04", 0x50, &port6_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX0_BUS6, "24c04", 0x50, &port7_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX0_BUS7, "24c04", 0x50, &port8_50_at24),

	mk_i2cdev(FPGA_I2C_CH10, "pca9548", 0x73, &fpga_ch10_1_platform_data),
	mk_i2cdev(FPGA_I2C_CH10_MUX1_BUS0, "24c04", 0x50, &port9_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX1_BUS1, "24c04", 0x50, &port10_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX1_BUS2, "24c04", 0x50, &port11_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX1_BUS3, "24c04", 0x50, &port12_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX1_BUS4, "24c04", 0x50, &port13_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX1_BUS5, "24c04", 0x50, &port14_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX1_BUS6, "24c04", 0x50, &port15_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX1_BUS7, "24c04", 0x50, &port16_50_at24),

	mk_i2cdev(FPGA_I2C_CH10, "pca9548", 0x74, &fpga_ch10_2_platform_data),
	mk_i2cdev(FPGA_I2C_CH10_MUX2_BUS0, "24c04", 0x50, &port17_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX2_BUS1, "24c04", 0x50, &port18_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX2_BUS2, "24c04", 0x50, &port19_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX2_BUS3, "24c04", 0x50, &port20_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX2_BUS4, "24c04", 0x50, &port21_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX2_BUS5, "24c04", 0x50, &port22_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX2_BUS6, "24c04", 0x50, &port23_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX2_BUS7, "24c04", 0x50, &port24_50_at24),

	mk_i2cdev(FPGA_I2C_CH10, "pca9548", 0x75, &fpga_ch10_3_platform_data),
	mk_i2cdev(FPGA_I2C_CH10_MUX3_BUS0, "24c04", 0x50, &port25_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX3_BUS1, "24c04", 0x50, &port26_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX3_BUS2, "24c04", 0x50, &port27_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX3_BUS3, "24c04", 0x50, &port28_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX3_BUS4, "24c04", 0x50, &port29_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX3_BUS5, "24c04", 0x50, &port30_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX3_BUS6, "24c04", 0x50, &port31_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX3_BUS7, "24c04", 0x50, &port32_50_at24),

	mk_i2cdev(FPGA_I2C_CH10, "pca9548", 0x76, &fpga_ch10_4_platform_data),
	mk_i2cdev(FPGA_I2C_CH10_MUX4_BUS0, "24c04", 0x50, &port33_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX4_BUS1, "24c04", 0x50, &port34_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX4_BUS2, "24c04", 0x50, &port35_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX4_BUS3, "24c04", 0x50, &port36_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX4_BUS4, "24c04", 0x50, &port37_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX4_BUS5, "24c04", 0x50, &port38_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX4_BUS6, "24c04", 0x50, &port39_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX4_BUS7, "24c04", 0x50, &port40_50_at24),

	mk_i2cdev(FPGA_I2C_CH10, "pca9548", 0x77, &fpga_ch10_5_platform_data),
	mk_i2cdev(FPGA_I2C_CH10_MUX5_BUS0, "24c04", 0x50, &port41_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX5_BUS1, "24c04", 0x50, &port42_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX5_BUS2, "24c04", 0x50, &port43_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX5_BUS3, "24c04", 0x50, &port44_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX5_BUS4, "24c04", 0x50, &port45_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX5_BUS5, "24c04", 0x50, &port46_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX5_BUS6, "24c04", 0x50, &port47_50_at24),
	mk_i2cdev(FPGA_I2C_CH10_MUX5_BUS7, "24c04", 0x50, &port48_50_at24),

	mk_i2cdev(FPGA_I2C_CH2, "pca9548", 0x74, &fpga_ch2_0_platform_data),
	mk_i2cdev(FPGA_I2C_CH2_MUX0_BUS0, "sff8436", 0x50, &port53_50_sff8436),
	mk_i2cdev(FPGA_I2C_CH2_MUX0_BUS1, "sff8436", 0x50, &port54_50_sff8436),
	mk_i2cdev(FPGA_I2C_CH2_MUX0_BUS2, "sff8436", 0x50, &port55_50_sff8436),
	mk_i2cdev(FPGA_I2C_CH2_MUX0_BUS3, "sff8436", 0x50, &port56_50_sff8436),
	mk_i2cdev(FPGA_I2C_CH2_MUX0_BUS4, "sff8436", 0x50, &port49_50_sff8436),
	mk_i2cdev(FPGA_I2C_CH2_MUX0_BUS5, "sff8436", 0x50, &port50_50_sff8436),
	mk_i2cdev(FPGA_I2C_CH2_MUX0_BUS6, "sff8436", 0x50, &port51_50_sff8436),
	mk_i2cdev(FPGA_I2C_CH2_MUX0_BUS7, "sff8436", 0x50, &port52_50_sff8436),

	mk_i2cdev(FPGA_I2C_CH3, "cel_questone2_cpld", 0x30, NULL),
	mk_i2cdev(FPGA_I2C_CH3, "cel_questone2_cpld", 0x31, NULL),
};

/*
 * Probe the FPGA device on the PCIe bus.  In the process, expose the relevant
 * FPGA Misc Control registers in the first partition.	Map the address ranges
 * of the I2C slices and pass the ranges to driver for each device.
 *
 */

/* fpga_id - PCI device ID supported by this driver */
static const struct pci_device_id fpga_id[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_XILINX, PCI_DEVICE_ID_XILINX_FPGA) },
	{ 0, }
};

static struct pci_driver fpga_driver;

/* FPGA Hardware Descriptor */
struct fpga_desc {
	u8 tgtaddr_rw;	/* target address & r/w bit */
	u8 wr_len_cmd;	/* write length in bytes or a command */
	u8 rd_len;	/* read length */
	u8 control;	/* control bits */
	u8 status;	/* status bits */
	u8 retry;	/* collision retry and retry count */
	u8 rxbytes;	/* received bytes */
	u8 txbytes;	/* transmitted bytes */
	u32 dptr_low;	/* lower 32 bit of the data pointer */
	u32 dptr_high;	/* upper 32 bit of the data pointer */
} __packed;

struct fpga_priv {
	u8 __iomem *misc_pbar;	 /* Misc registers PCI Base Address Register */
	u8 __iomem *fpga_pbar;	 /* Port registers PCI Base Address Register */
	struct pci_dev *pci_dev;
	struct fpga_desc *hw;	 /* descriptor virt base addr */
};

/* Accessor functions for reading and writing the Misc registers */

static int misc_read_reg(struct device *dev,
			 int reg,
			 int nregs,
			 u32 *val)
{
	struct fpga_priv *priv = dev_get_drvdata(dev);

	*val = readl(priv->misc_pbar + reg);
	return 0;
}

static int misc_write_reg(struct device *dev,
			  int reg,
			  int nregs,
			  u32 val)
{
	struct fpga_priv *priv = dev_get_drvdata(dev);

	writel(val, priv->misc_pbar + reg);
	return 0;
}

/* Define all the bitfields */

mk_bf_ro(misc, fpga_major_revision, 0x0000, 16, 16, NULL, 0);
mk_bf_ro(misc, fpga_minor_revision, 0x0000,  0, 16, NULL, 0);
mk_bf_rw(misc, fpga_scratch,	    0x0004,  0, 32, NULL, 0);
mk_bf_rw(misc, reset_sfp28_mux,	    0x0034,  5,	 1, NULL, BF_COMPLEMENT);
mk_bf_rw(misc, reset_qsfp28_mux,    0x0034,  4,	 1, NULL, BF_COMPLEMENT);

static struct attribute *misc_attrs[] = {
	&misc_fpga_major_revision.attr,
	&misc_fpga_minor_revision.attr,
	&misc_fpga_scratch.attr,
	&misc_reset_sfp28_mux.attr,
	&misc_reset_qsfp28_mux.attr,
	NULL,
};

static struct attribute_group misc_attr_group = {
	.attrs = misc_attrs,
};

/* Accessor functions for reading and writing the Port registers */

static int fpga_read_reg(struct device *dev,
			 int reg,
			 int nregs,
			 u32 *val)
{
	struct fpga_priv *priv = dev_get_drvdata(dev);

	*val = readl(priv->fpga_pbar + reg);
	return 0;
}

static int fpga_write_reg(struct device *dev,
			  int reg,
			  int nregs,
			  u32 val)
{
	struct fpga_priv *priv = dev_get_drvdata(dev);

	writel(val, priv->fpga_pbar + reg);
	return 0;
}

#define fpga_bt_ro(_name, _reg, _field, _values, _flags) \
	mk_bf_ro(fpga, _name, _reg, _field##_BIT, 1, _values, _flags)

#define fpga_bt_rw(_name, _reg, _field, _values, _flags) \
	mk_bf_rw(fpga, _name, _reg, _field##_BIT, 1, _values, _flags)

#define fpga_sfp_port(_port) \
	fpga_bt_rw(port##_port##_tx_disable, \
	   CEL_SEA2QUE2_PORT_CTRL_REG + (16 * ((_port) - 1)), \
	   CEL_SEA2QUE2_PORT_CTRL_TX_DIS, NULL, 0); \
	fpga_bt_ro(port##_port##_tx_fault, \
	   CEL_SEA2QUE2_PORT_STAT_REG + (16 * ((_port) - 1)), \
	   CEL_SEA2QUE2_PORT_STAT_TXFAULT, NULL, 0); \
	fpga_bt_ro(port##_port##_rx_los, \
	   CEL_SEA2QUE2_PORT_STAT_REG + (16 * ((_port) - 1)), \
	   CEL_SEA2QUE2_PORT_STAT_RXLOS, NULL, 0); \
	fpga_bt_ro(port##_port##_present, \
	   CEL_SEA2QUE2_PORT_STAT_REG + (16 * ((_port) - 1)), \
	   CEL_SEA2QUE2_PORT_STAT_MODABS, NULL, BF_COMPLEMENT)

#define fpga_qsfp_port(_port) \
	fpga_bt_rw(port##_port##_lpmode, \
	   CEL_SEA2QUE2_PORT_CTRL_REG + (16 * ((_port) - 1)), \
	   CEL_SEA2QUE2_PORT_CTRL_LPMOD, NULL, 0); \
	fpga_bt_ro(port##_port##_interrupt, \
	   CEL_SEA2QUE2_PORT_STAT_REG + (16 * ((_port) - 1)), \
	   CEL_SEA2QUE2_PORT_STAT_IRQ, NULL, BF_COMPLEMENT); \
	fpga_bt_rw(port##_port##_reset, \
	   CEL_SEA2QUE2_PORT_CTRL_REG + (16 * ((_port) - 1)), \
	   CEL_SEA2QUE2_PORT_CTRL_RST, NULL, BF_COMPLEMENT); \
	fpga_bt_ro(port##_port##_present, \
	   CEL_SEA2QUE2_PORT_STAT_REG + (16 * ((_port) - 1)), \
	   CEL_SEA2QUE2_PORT_STAT_PRESENT, NULL, BF_COMPLEMENT)

#define fpga_sfp_port_attrs(_num) \
	&fpga_port##_num##_tx_disable.attr, \
	&fpga_port##_num##_tx_fault.attr, \
	&fpga_port##_num##_rx_los.attr, \
	&fpga_port##_num##_present.attr

#define fpga_qsfp_port_attrs(_num) \
	&fpga_port##_num##_lpmode.attr, \
	&fpga_port##_num##_interrupt.attr, \
	&fpga_port##_num##_reset.attr, \
	&fpga_port##_num##_present.attr

/* plumb all of the front panel port signals */

fpga_sfp_port(1);
fpga_sfp_port(2);
fpga_sfp_port(3);
fpga_sfp_port(4);
fpga_sfp_port(5);
fpga_sfp_port(6);
fpga_sfp_port(7);
fpga_sfp_port(8);
fpga_sfp_port(9);
fpga_sfp_port(10);
fpga_sfp_port(11);
fpga_sfp_port(12);
fpga_sfp_port(13);
fpga_sfp_port(14);
fpga_sfp_port(15);
fpga_sfp_port(16);
fpga_sfp_port(17);
fpga_sfp_port(18);
fpga_sfp_port(19);
fpga_sfp_port(20);
fpga_sfp_port(21);
fpga_sfp_port(22);
fpga_sfp_port(23);
fpga_sfp_port(24);
fpga_sfp_port(25);
fpga_sfp_port(26);
fpga_sfp_port(27);
fpga_sfp_port(28);
fpga_sfp_port(29);
fpga_sfp_port(30);
fpga_sfp_port(31);
fpga_sfp_port(32);
fpga_sfp_port(33);
fpga_sfp_port(34);
fpga_sfp_port(35);
fpga_sfp_port(36);
fpga_sfp_port(37);
fpga_sfp_port(38);
fpga_sfp_port(39);
fpga_sfp_port(40);
fpga_sfp_port(41);
fpga_sfp_port(42);
fpga_sfp_port(43);
fpga_sfp_port(44);
fpga_sfp_port(45);
fpga_sfp_port(46);
fpga_sfp_port(47);
fpga_sfp_port(48);

fpga_qsfp_port(49);
fpga_qsfp_port(50);
fpga_qsfp_port(51);
fpga_qsfp_port(52);
fpga_qsfp_port(53);
fpga_qsfp_port(54);
fpga_qsfp_port(55);
fpga_qsfp_port(56);

static struct attribute *fpga_attrs[] = {
	fpga_sfp_port_attrs(1),
	fpga_sfp_port_attrs(2),
	fpga_sfp_port_attrs(3),
	fpga_sfp_port_attrs(4),
	fpga_sfp_port_attrs(5),
	fpga_sfp_port_attrs(6),
	fpga_sfp_port_attrs(7),
	fpga_sfp_port_attrs(8),
	fpga_sfp_port_attrs(9),
	fpga_sfp_port_attrs(10),
	fpga_sfp_port_attrs(11),
	fpga_sfp_port_attrs(12),
	fpga_sfp_port_attrs(13),
	fpga_sfp_port_attrs(14),
	fpga_sfp_port_attrs(15),
	fpga_sfp_port_attrs(16),
	fpga_sfp_port_attrs(17),
	fpga_sfp_port_attrs(18),
	fpga_sfp_port_attrs(19),
	fpga_sfp_port_attrs(20),
	fpga_sfp_port_attrs(21),
	fpga_sfp_port_attrs(22),
	fpga_sfp_port_attrs(23),
	fpga_sfp_port_attrs(24),
	fpga_sfp_port_attrs(25),
	fpga_sfp_port_attrs(26),
	fpga_sfp_port_attrs(27),
	fpga_sfp_port_attrs(28),
	fpga_sfp_port_attrs(29),
	fpga_sfp_port_attrs(30),
	fpga_sfp_port_attrs(31),
	fpga_sfp_port_attrs(32),
	fpga_sfp_port_attrs(33),
	fpga_sfp_port_attrs(34),
	fpga_sfp_port_attrs(35),
	fpga_sfp_port_attrs(36),
	fpga_sfp_port_attrs(37),
	fpga_sfp_port_attrs(38),
	fpga_sfp_port_attrs(39),
	fpga_sfp_port_attrs(40),
	fpga_sfp_port_attrs(41),
	fpga_sfp_port_attrs(42),
	fpga_sfp_port_attrs(43),
	fpga_sfp_port_attrs(44),
	fpga_sfp_port_attrs(45),
	fpga_sfp_port_attrs(46),
	fpga_sfp_port_attrs(47),
	fpga_sfp_port_attrs(48),

	fpga_qsfp_port_attrs(49),
	fpga_qsfp_port_attrs(50),
	fpga_qsfp_port_attrs(51),
	fpga_qsfp_port_attrs(52),
	fpga_qsfp_port_attrs(53),
	fpga_qsfp_port_attrs(54),
	fpga_qsfp_port_attrs(55),
	fpga_qsfp_port_attrs(56),

	NULL,
};

static struct attribute_group fpga_attr_group = {
	.attrs = fpga_attrs,
};

/* FPGA Init */

static int fpga_dev_init(struct fpga_priv *priv)
{
	size_t size;

	size = FPGA_DESC_ENTRIES * sizeof(*priv->hw);

	/* allocate memory for the FPGA descriptor */
	priv->hw = devm_kzalloc(&priv->pci_dev->dev, size, GFP_KERNEL);
	if (!priv->hw)
		return -ENOMEM;

	memset(priv->hw, 0, size);
	return 0;
}

static void fpga_dev_release(struct fpga_priv *priv)
{
	devm_kfree(&priv->pci_dev->dev, priv->hw);
}

struct fpga_i2c_device_info fpga_device_infotab[NUM_FPGA_BUSSES];
static struct fpga_i2c_platform_data fpga_i2c_data[NUM_FPGA_BUSSES];

static struct resource fpga_resources[NUM_FPGA_BUSSES];
static struct resource ctrl_resource;

static struct i2c_client *cpld_devices[NUM_CPLD_DEVICES];

static int fpga_probe(struct pci_dev *pdev, const struct pci_device_id *devid)
{
	struct fpga_priv *priv;
	struct resource *cres;
	struct fpga_i2c_platform_data *fipd;
	struct platform_device *platdev;
	unsigned long start, len;
	int i, ch, index;
	int err;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	pci_set_drvdata(pdev, priv);
	priv->pci_dev = pdev;

	err = pcim_enable_device(pdev);
	if (err) {
		dev_err(&pdev->dev, "Failed to enable FPGA PCI device (%d)\n",
			err);
		devm_kfree(&pdev->dev, priv);
		return err;
	}

	/* Determine the address of the FPGA */
	start = pci_resource_start(pdev, FPGA_BAR);
	len = pci_resource_len(pdev, FPGA_BAR);
	if (!start || !len) {
		dev_err(&pdev->dev,
			"FPGA base address uninitialized, upgrade BIOS\n");
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		return -ENODEV;
	}

	err = fpga_dev_init(priv);
	if (err) {
		dev_err(&pdev->dev, "init failed");
		err = -ENOMEM;
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		goto fail;
	}

	/* Initialize a resource for the misc registers defined in the FPGA */
	cres = &ctrl_resource;
	cres->start = start;
	cres->end   = cres->start + 0x00FF;
	cres->flags = IORESOURCE_MEM;

	priv->misc_pbar = devm_ioremap_resource(&pdev->dev, cres);
	if (!priv->misc_pbar) {
		pr_err(DRIVER_NAME ": devm_ioremap_resource failed for misc registers\n");
		err = -ENOMEM;
		devm_kfree(&pdev->dev, priv->hw);
		fpga_dev_release(priv);
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		goto fail;
	}

	/* Create sysfs group for the misc registers in the FPGA */
	err = sysfs_create_group(&pdev->dev.kobj, &misc_attr_group);
	if (err) {
		pr_err(DRIVER_NAME ": sysfs_misc_attr_group failed for FPGA misc registers\n");
		devm_iounmap(&pdev->dev, priv->misc_pbar);
		fpga_dev_release(priv);
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		goto fail;
	}

	/* Initialize a resource for the port registers defined in the FPGA */
	cres = &ctrl_resource;
	cres->start = start + 0x4000;
	cres->end   = cres->start + 0x4FFF;
	cres->flags = IORESOURCE_MEM;

	priv->fpga_pbar = devm_ioremap_resource(&pdev->dev, cres);
	if (!priv->fpga_pbar) {
		pr_err(DRIVER_NAME ": devm_ioremap_resource failed for port registers\n");
		err = -ENOMEM;
		devm_kfree(&pdev->dev, priv->hw);
		fpga_dev_release(priv);
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		goto fail;
	}

	/* Create sysfs group for the port registers in the FPGA */
	err = sysfs_create_group(&pdev->dev.kobj, &fpga_attr_group);
	if (err) {
		pr_err(DRIVER_NAME ": sysfs_fpga_attr_group failed for FPGA port registers\n");
		devm_iounmap(&pdev->dev, priv->misc_pbar);
		devm_iounmap(&pdev->dev, priv->fpga_pbar);
		fpga_dev_release(priv);
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		goto fail;
	}

	/*
	 * Initialize fpga_resources[] for all FPGA busses.
	 * The device slice loop will index into this array to
	 * get platform device resources.
	 *
	 * FPGA I2C_CH1	 gets start 0x100, end 0x1ff
	 * FPGA I2C_CH10  gets start 0x200, end 0x2ff
	 * ...
	 * FPGA I2C_CH10 gets start 0xa00, end 0xaff
	 */
	for (i = FPGA_I2C_CH1; i <= FPGA_I2C_CH10; i++) {
		index = i - FPGA_I2C_CH1;
		fpga_resources[index].start = start + 0x0100 +
			((i - FPGA_I2C_CH1) * 0x0100);
		fpga_resources[index].end   = fpga_resources[index].start +
			0xff;
		fpga_resources[index].flags = IORESOURCE_MEM;
	}

	/*
	 * The device slice loop.  Each of the devices in
	 * fpga_device_infotab[] carves out its own name, resources,
	 * and private data and is initialized here.  At the end, each
	 * infotab device registers with the i2c-fpga driver to share
	 * its idea of its resources.  We therefore get one i2c-fpga
	 * driver instance for each device in fpga_device_infotab[].
	 *
	 * fpga_device_infotab[], fpga_resources[], fpga_devtab[],
	 * and fpga_i2c_data[] had better all have the same array size as
	 * fpga_i2c_mux_devices[]!
	 */
	for (i = FPGA_I2C_CH1; i <= FPGA_I2C_CH10; i++) {
		ch = i - FPGA_I2C_CH1 + 1; /* I2C channel number */
		index = i - FPGA_I2C_CH1; /* array index */

		/* Define the fpga_i2c_resource for each FPGA I2C bus.
		 *
		 * FPGA_I2C_CH1	 gets start + 0x100 to 0x1ff
		 * FPGA_I2C_CH2	 gets start + 0x200 to 0x2ff
		 * ...
		 * FPGA I2C_CH10 gets start + 0xa00 to 0xaff
		 */

		cres = &ctrl_resource;
		cres->start = start + 0x0100 + ((i - FPGA_I2C_CH1) * 0x0100);
		cres->end   = cres->start + 0x00FF;
		cres->flags = IORESOURCE_MEM;

		platdev = platform_device_alloc(CEL_FPGA_I2C_DRIVER_NAME,
						index);
		if (!platdev) {
			pr_err(DRIVER_NAME ": device allocation failed for fpga i2c ch%d\n",
			       ch);
			err = -ENOMEM;
			sysfs_remove_group(&pdev->dev.kobj, &misc_attr_group);
			sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
			devm_iounmap(&pdev->dev, priv->misc_pbar);
			devm_iounmap(&pdev->dev, priv->fpga_pbar);
			fpga_dev_release(priv);
			pci_disable_device(pdev);
			devm_kfree(&pdev->dev, priv);
			goto fail;
		}

		err = platform_device_add_resources(platdev, cres, 1);
		if (err) {
			pr_err(DRIVER_NAME ": failed to add resources for fpga i2c ch%d\n",
			       ch);
			sysfs_remove_group(&pdev->dev.kobj, &misc_attr_group);
			sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
			devm_iounmap(&pdev->dev, priv->misc_pbar);
			devm_iounmap(&pdev->dev, priv->fpga_pbar);
			fpga_dev_release(priv);
			pci_disable_device(pdev);
			devm_kfree(&pdev->dev, priv);
			goto fail;
		}
		fpga_device_infotab[index].bus = fpga_i2c_busses[index].bus;
		fpga_device_infotab[index].info =
			&fpga_i2c_busses[index].board_info;

		fipd = &fpga_i2c_data[index];
		fipd->clock_khz		= 100000;
		fipd->devices		= &fpga_device_infotab[index];
		fipd->num_devices	= 1;

		err = platform_device_add_data(platdev, fipd, sizeof(*fipd));
		if (err) {
			pr_err(DRIVER_NAME ": add data failed for fpga i2c ch%d\n",
			       ch);
			sysfs_remove_group(&pdev->dev.kobj, &misc_attr_group);
			sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
			devm_iounmap(&pdev->dev, priv->misc_pbar);
			devm_iounmap(&pdev->dev, priv->fpga_pbar);
			fpga_dev_release(priv);
			pci_disable_device(pdev);
			devm_kfree(&pdev->dev, priv);
			goto fail;
		}

		err = platform_device_add(platdev);
		if (err) {
			pr_err(DRIVER_NAME ": failed to add device for fpga i2c ch%d\n",
			       ch);
			sysfs_remove_group(&pdev->dev.kobj, &misc_attr_group);
			sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
			devm_iounmap(&pdev->dev, priv->misc_pbar);
			devm_iounmap(&pdev->dev, priv->fpga_pbar);
			fpga_dev_release(priv);
			pci_disable_device(pdev);
			devm_kfree(&pdev->dev, priv);
			goto fail;
		}
	}
	pr_debug(DRIVER_NAME ": fpga driver loaded\n");

fail:
	return err;
}

static void fpga_remove(struct pci_dev *pdev)
{
	struct fpga_priv *priv;

	priv = dev_get_drvdata(&pdev->dev);
	sysfs_remove_group(&pdev->dev.kobj, &misc_attr_group);
	sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
	devm_iounmap(&pdev->dev, priv->misc_pbar);
	devm_iounmap(&pdev->dev, priv->fpga_pbar);
	fpga_dev_release(priv);
	pci_disable_device(pdev);
	devm_kfree(&pdev->dev, priv);

	pr_info(DRIVER_NAME ": FPGA driver unloaded\n");
}

#define fpga_suspend NULL
#define fpga_resume NULL

static struct pci_driver fpga_driver = {
	.name = "cel_questone2_fpga",
	.id_table = fpga_id,
	.probe = fpga_probe,
	.remove = fpga_remove,
	.suspend = fpga_suspend,
	.resume = fpga_resume,
};

static int fpga_init(void)
{
	struct i2c_client *client;
	int i;
	int ret;
	int count = 0;

	ret = pci_register_driver(&fpga_driver);
	if (ret) {
		pr_err(DRIVER_NAME ": failed to register fpga device\n");
		return ret;
	}

	/*
	 * Allocate all the I2C devices on the FPGA I2C busses.	 The I2C
	 * adapters should have been created already in the probe function.
	 */
	for (i = 0; i < ARRAY_SIZE(fpga_i2c_devices); i++) {
		int bus;

		bus = fpga_i2c_devices[i].bus;

		client =
			cumulus_i2c_add_client(bus,
					       &fpga_i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			pr_err(DRIVER_NAME ": add fpga i2c client failed for bus %d: %d\n",
			       bus, ret);
			goto err_exit;
		}
		fpga_i2c_devices[i].client = client;
		if (strcmp(fpga_i2c_devices[i].board_info.type,
			   "cel_questone2_cpld") == 0)
			cpld_devices[count++] = client;
	}
	pr_info(DRIVER_NAME ": FPGA driver registered\n");
	return 0;

err_exit:
	pr_err(DRIVER_NAME ": FPGA driver failed to register\n");
	return -1;
}

static void fpga_exit(void)
{
	int i;
	struct i2c_client *c;

	/* unregister the FPGA i2c clients */
	for (i = ARRAY_SIZE(fpga_i2c_devices); --i >= 0;) {
		c = fpga_i2c_devices[i].client;
		if (c)
			i2c_unregister_device(c);
	}

	pci_unregister_driver(&fpga_driver);
	pr_info(DRIVER_NAME ": FPGA driver unloaded\n");
}

/* CPLD */

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
	int ret = 0;

	int cpld_id = GET_CPLD_ID(reg);

	if (cpld_id < 0 || cpld_id >= NUM_CPLD_DEVICES) {
		pr_err(DRIVER_NAME ": Attempt to write to invalid CPLD register: 0x%02X\n",
		       reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	ret = i2c_smbus_write_byte_data(cpld_devices[cpld_id], reg, val);
	if (ret < 0) {
		pr_err(DRIVER_NAME ": CPLD write error - addr: 0x%02X, offset: 0x%02X\n",
		       cpld_devices[cpld_id]->addr, reg);
	}
	return ret;
}

/* CPLD register bitfields with enum-like values */

static const char * const slot_id_values[] = {
	"CPLD1",
	"CPLD2",
};

/* CPLD registers */

mk_bf_ro(cpld, cpld1_major_revision, (0x00 | CPLD1_IDX), 4, 4, NULL, 0);
mk_bf_ro(cpld, cpld1_minor_revision, (0x00 | CPLD1_IDX), 0, 4, NULL, 0);
mk_bf_rw(cpld, cpld1_scratch,	     (0x01 | CPLD1_IDX), 0, 8, NULL, 0);
mk_bf_ro(cpld, cpld1_board_type,     (0x02 | CPLD1_IDX), 0, 5, NULL, 0);
mk_bf_ro(cpld, cpld1_board_revision, (0x03 | CPLD1_IDX), 0, 3, NULL, 0);
mk_bf_ro(cpld, cpld1_slot_id,	     (0x04 | CPLD1_IDX), 0, 3, slot_id_values,
	 0);
mk_bf_rw(cpld, cpld1_led_mode,	     (0x09 | CPLD1_IDX), 0, 1, NULL, 0);
mk_bf_rw(cpld, cpld1_test_amber,     (0x0a | CPLD1_IDX), 1, 1, NULL,
	 BF_COMPLEMENT);
mk_bf_rw(cpld, cpld1_test_green,     (0x0a | CPLD1_IDX), 0, 1, NULL,
	 BF_COMPLEMENT);

mk_bf_ro(cpld, cpld2_major_revision, (0x00 | CPLD2_IDX), 4, 4, NULL, 0);
mk_bf_ro(cpld, cpld2_minor_revision, (0x00 | CPLD2_IDX), 0, 4, NULL, 0);
mk_bf_rw(cpld, cpld2_scratch,	     (0x01 | CPLD2_IDX), 0, 8, NULL, 0);
mk_bf_ro(cpld, cpld2_board_type,     (0x02 | CPLD2_IDX), 0, 5, NULL, 0);
mk_bf_ro(cpld, cpld2_board_revision, (0x03 | CPLD2_IDX), 0, 3, NULL, 0);
mk_bf_ro(cpld, cpld2_slot_id,	     (0x04 | CPLD2_IDX), 0, 3, slot_id_values,
	 0);
mk_bf_rw(cpld, cpld2_led_mode,	     (0x09 | CPLD2_IDX), 0, 1, NULL, 0);
mk_bf_rw(cpld, cpld2_test_amber,     (0x0a | CPLD2_IDX), 1, 1, NULL,
	 BF_COMPLEMENT);
mk_bf_rw(cpld, cpld2_test_green,     (0x0a | CPLD2_IDX), 0, 1, NULL,
	 BF_COMPLEMENT);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_cpld1_major_revision.attr,
	&cpld_cpld1_minor_revision.attr,
	&cpld_cpld1_scratch.attr,
	&cpld_cpld1_board_type.attr,
	&cpld_cpld1_board_revision.attr,
	&cpld_cpld1_slot_id.attr,
	&cpld_cpld1_led_mode.attr,
	&cpld_cpld1_test_amber.attr,
	&cpld_cpld1_test_green.attr,
	&cpld_cpld2_major_revision.attr,
	&cpld_cpld2_minor_revision.attr,
	&cpld_cpld2_scratch.attr,
	&cpld_cpld2_board_type.attr,
	&cpld_cpld2_board_revision.attr,
	&cpld_cpld2_slot_id.attr,
	&cpld_cpld2_led_mode.attr,
	&cpld_cpld2_test_amber.attr,
	&cpld_cpld2_test_green.attr,

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
		.name = "cel_que2_switch_board_cpld",
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

	cpld_device = platform_device_alloc("cel_que2_switch_board_cpld", 0);
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

/* Module init and exit */

static int __init cel_questone2_init(void)
{
	int ret;

	ret = fpga_init();
	if (ret) {
		pr_err(DRIVER_NAME ": FPGA initialization failed\n");
		fpga_exit();
		return ret;
	}

	ret = cpld_init();
	if (ret) {
		pr_err(DRIVER_NAME ": CPLD initialization failed\n");
		cpld_exit();
		fpga_exit();
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit cel_questone2_exit(void)
{
	cpld_exit();
	fpga_exit();
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

module_init(cel_questone2_init);
module_exit(cel_questone2_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica Questone2 FPGA Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
