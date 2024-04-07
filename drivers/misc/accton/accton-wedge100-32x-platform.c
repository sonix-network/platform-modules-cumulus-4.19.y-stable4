/*
 * accton_wedge100_platform.c - Accton Wedge100 Platform Support.
 *
 * Copyright (C) 2016, 2019 Cumulus Networks, Inc.
 * Author: Ellen Wang (ellen@cumulusnetworks.com)
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
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/at24.h>
#include <linux/gpio.h>

#include "platform-defs.h"
#include "platform-bitfield.h"

#define DRIVER_NAME	"accton_wedge100"
#define DRIVER_VERSION	"1.0"

/*
 * The platform has one CP2112 USB-to-I2C bridge, and possibly
 * other I2C buses that we don't care about.
 * The former has a CPLD and 5 pca9548 (8-port) I2C muxes
 * (OK they're really switches), 4 of which go to the 32 QSFP28
 * slots, and the last one goes to 6 pca9535 16-line IO expanders
 * and a 24c64 (64k-bit) EEPROM.
 *
 * Because the pca9548s are connected in parallel, and their
 * down-stream devices (the QSFP28s) all have the same address,
 * they need to disable all ports when not used (the deselect_on_exit
 * option).
 */

enum {
	W100_I2C_CP2112_BUS = -1,

	W100_I2C_MUX1_BUS0 = 10,
	W100_I2C_MUX1_BUS1,
	W100_I2C_MUX1_BUS2,
	W100_I2C_MUX1_BUS3,
	W100_I2C_MUX1_BUS4,
	W100_I2C_MUX1_BUS5,
	W100_I2C_MUX1_BUS6,
	W100_I2C_MUX1_BUS7,

	W100_I2C_MUX2_BUS0 = 20,
	W100_I2C_MUX2_BUS1,
	W100_I2C_MUX2_BUS2,
	W100_I2C_MUX2_BUS3,
	W100_I2C_MUX2_BUS4,
	W100_I2C_MUX2_BUS5,
	W100_I2C_MUX2_BUS6,
	W100_I2C_MUX2_BUS7,

	W100_I2C_MUX3_BUS0 = 30,
	W100_I2C_MUX3_BUS1,
	W100_I2C_MUX3_BUS2,
	W100_I2C_MUX3_BUS3,
	W100_I2C_MUX3_BUS4,
	W100_I2C_MUX3_BUS5,
	W100_I2C_MUX3_BUS6,
	W100_I2C_MUX3_BUS7,

	W100_I2C_MUX4_BUS0 = 40,
	W100_I2C_MUX4_BUS1,
	W100_I2C_MUX4_BUS2,
	W100_I2C_MUX4_BUS3,
	W100_I2C_MUX4_BUS4,
	W100_I2C_MUX4_BUS5,
	W100_I2C_MUX4_BUS6,
	W100_I2C_MUX4_BUS7,

	W100_I2C_MUX5_BUS0 = 50,
	W100_I2C_MUX5_BUS1,
	W100_I2C_MUX5_BUS2,
	W100_I2C_MUX5_BUS3,
	W100_I2C_MUX5_BUS4,
	W100_I2C_MUX5_BUS5,
	W100_I2C_MUX5_BUS6,
	W100_I2C_MUX5_BUS7,
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

mk_pca9548(mux1, W100_I2C_MUX1_BUS0, 1);
mk_pca9548(mux2, W100_I2C_MUX2_BUS0, 1);
mk_pca9548(mux3, W100_I2C_MUX3_BUS0, 1);
mk_pca9548(mux4, W100_I2C_MUX4_BUS0, 1);
mk_pca9548(mux5, W100_I2C_MUX5_BUS0, 1);

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

mk_eeprom(board, 50, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);

/*
 * GPIO definitions
 *
 * The GPIO infrastructure has an unfortunate interface for
 * initializing pins.  The pin names are specified in platform_data,
 * but everthing else has to be done by calls after pin creation.
 * So we consolidate all the information in one place (struct gpio_pin),
 * and try to make the best of it.  The pin name array is filled in
 * at runtime by init_gpio_platform_data(), before creating the I2C
 * device.  Then the pins are created by init_gpio_pins().
 */
struct gpio_pin {
	int pin;
	const char *label;
	unsigned long flags;
};

#define mk_gpio_pins(_name) \
	static struct gpio_pin _name##_pins[]
#define mk_gpio_pin(_num, _name, _flags) \
	{ \
		.pin = (_num), \
		.label = #_name, \
		.flags = (GPIOF_EXPORT_DIR_FIXED | (_flags)) \
	}

#define mk_gpio_platform_data(_name, _base, _numpins) \
	static char const *_name##_pinnames[_numpins]; \
	static struct pca953x_platform_data _name##_platform_data = { \
		.gpio_base = (_base), \
		.names = _name##_pinnames, \
	}

mk_gpio_pins(gpio1) = {
	mk_gpio_pin(0,  qsfp2_lp_mode,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(1,  qsfp1_lp_mode,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(2,  qsfp4_lp_mode,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(3,  qsfp3_lp_mode,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(4,  qsfp6_lp_mode,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(5,  qsfp5_lp_mode,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(6,  qsfp8_lp_mode,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(7,  qsfp7_lp_mode,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(8,  qsfp10_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,  qsfp9_lp_mode,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, qsfp12_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, qsfp11_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, qsfp14_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(13, qsfp13_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14, qsfp16_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(15, qsfp15_lp_mode,   GPIOF_OUT_INIT_LOW),
};

mk_gpio_pins(gpio2) = {
	mk_gpio_pin(0,  qsfp18_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(1,  qsfp17_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(2,  qsfp20_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(3,  qsfp19_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(4,  qsfp22_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(5,  qsfp21_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(6,  qsfp24_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(7,  qsfp23_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(8,  qsfp26_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,  qsfp25_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, qsfp28_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11, qsfp27_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, qsfp30_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(13, qsfp29_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14, qsfp32_lp_mode,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(15, qsfp31_lp_mode,   GPIOF_OUT_INIT_LOW),
};

mk_gpio_pins(gpio3) = {
	mk_gpio_pin(0,  qsfp2_present,    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(1,  qsfp1_present,    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(2,  qsfp4_present,    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(3,  qsfp3_present,    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(4,  qsfp6_present,    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,  qsfp5_present,    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,  qsfp8_present,    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,  qsfp7_present,    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,  qsfp10_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(9,  qsfp9_present,    GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(10, qsfp12_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(11, qsfp11_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(12, qsfp14_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(13, qsfp13_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(14, qsfp16_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(15, qsfp15_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
};

mk_gpio_pins(gpio4) = {
	mk_gpio_pin(0,  qsfp18_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(1,  qsfp17_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(2,  qsfp20_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(3,  qsfp19_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(4,  qsfp22_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,  qsfp21_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,  qsfp24_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,  qsfp23_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,  qsfp26_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(9,  qsfp25_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(10, qsfp28_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(11, qsfp27_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(12, qsfp30_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(13, qsfp29_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(14, qsfp32_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(15, qsfp31_present,   GPIOF_IN | GPIOF_ACTIVE_LOW),
};

mk_gpio_pins(gpio5) = {
	mk_gpio_pin(0,  qsfp2_interrupt,  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(1,  qsfp1_interrupt,  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(2,  qsfp4_interrupt,  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(3,  qsfp3_interrupt,  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(4,  qsfp6_interrupt,  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,  qsfp5_interrupt,  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,  qsfp8_interrupt,  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,  qsfp7_interrupt,  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,  qsfp10_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(9,  qsfp9_interrupt,  GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(10, qsfp12_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(11, qsfp11_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(12, qsfp14_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(13, qsfp13_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(14, qsfp16_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(15, qsfp15_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
};

mk_gpio_pins(gpio6) = {
	mk_gpio_pin(0,  qsfp18_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(1,  qsfp17_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(2,  qsfp20_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(3,  qsfp19_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(4,  qsfp22_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(5,  qsfp21_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(6,  qsfp24_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(7,  qsfp23_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(8,  qsfp26_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(9,  qsfp25_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(10, qsfp28_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(11, qsfp27_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(12, qsfp30_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(13, qsfp29_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(14, qsfp32_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
	mk_gpio_pin(15, qsfp31_interrupt, GPIOF_IN | GPIOF_ACTIVE_LOW),
};

mk_gpio_platform_data(gpio1, 1, 16);
mk_gpio_platform_data(gpio2, 17, 16);
mk_gpio_platform_data(gpio3, 101, 16);
mk_gpio_platform_data(gpio4, 117, 16);
mk_gpio_platform_data(gpio5, 201, 16);
mk_gpio_platform_data(gpio6, 217, 16);

static struct platform_i2c_device_info i2c_devices[] = {
	/* devices on CP2112 bus */
	mk_i2cdev(W100_I2C_CP2112_BUS, "pca9548", 0x70, &mux1_platform_data),
	mk_i2cdev(W100_I2C_CP2112_BUS, "pca9548", 0x71, &mux2_platform_data),
	mk_i2cdev(W100_I2C_CP2112_BUS, "pca9548", 0x72, &mux3_platform_data),
	mk_i2cdev(W100_I2C_CP2112_BUS, "pca9548", 0x73, &mux4_platform_data),
	mk_i2cdev(W100_I2C_CP2112_BUS, "pca9548", 0x74, &mux5_platform_data),
	mk_i2cdev(W100_I2C_CP2112_BUS, "act_wedge100_cpld", 0x32, NULL),

	/*
	 * devices on pca9548 5
	 *
	 * Do this one first so the main EEPROM is eeprom0, and
	 * port EEPROMs are numbered exactly like the ports.
	 * Not required, just nice.
	 */
	mk_i2cdev(W100_I2C_MUX5_BUS0, "pca9535", 0x20, &gpio1_platform_data),
	mk_i2cdev(W100_I2C_MUX5_BUS1, "pca9535", 0x21, &gpio2_platform_data),
	mk_i2cdev(W100_I2C_MUX5_BUS2, "pca9535", 0x22, &gpio3_platform_data),
	mk_i2cdev(W100_I2C_MUX5_BUS3, "pca9535", 0x23, &gpio4_platform_data),
	mk_i2cdev(W100_I2C_MUX5_BUS4, "pca9535", 0x24, &gpio5_platform_data),
	mk_i2cdev(W100_I2C_MUX5_BUS5, "pca9535", 0x25, &gpio6_platform_data),
	mk_i2cdev(W100_I2C_MUX5_BUS6, "24c64",   0x50, &board_50_at24),

	/* devices on pca9548 1 */
	mk_i2cdev(W100_I2C_MUX1_BUS1, "sff8436", 0x50, &port1_50_sff8436),
	mk_i2cdev(W100_I2C_MUX1_BUS0, "sff8436", 0x50, &port2_50_sff8436),
	mk_i2cdev(W100_I2C_MUX1_BUS3, "sff8436", 0x50, &port3_50_sff8436),
	mk_i2cdev(W100_I2C_MUX1_BUS2, "sff8436", 0x50, &port4_50_sff8436),
	mk_i2cdev(W100_I2C_MUX1_BUS5, "sff8436", 0x50, &port5_50_sff8436),
	mk_i2cdev(W100_I2C_MUX1_BUS4, "sff8436", 0x50, &port6_50_sff8436),
	mk_i2cdev(W100_I2C_MUX1_BUS7, "sff8436", 0x50, &port7_50_sff8436),
	mk_i2cdev(W100_I2C_MUX1_BUS6, "sff8436", 0x50, &port8_50_sff8436),

	/* devices on pca9548 2 */
	mk_i2cdev(W100_I2C_MUX2_BUS1, "sff8436", 0x50, &port9_50_sff8436),
	mk_i2cdev(W100_I2C_MUX2_BUS0, "sff8436", 0x50, &port10_50_sff8436),
	mk_i2cdev(W100_I2C_MUX2_BUS3, "sff8436", 0x50, &port11_50_sff8436),
	mk_i2cdev(W100_I2C_MUX2_BUS2, "sff8436", 0x50, &port12_50_sff8436),
	mk_i2cdev(W100_I2C_MUX2_BUS5, "sff8436", 0x50, &port13_50_sff8436),
	mk_i2cdev(W100_I2C_MUX2_BUS4, "sff8436", 0x50, &port14_50_sff8436),
	mk_i2cdev(W100_I2C_MUX2_BUS7, "sff8436", 0x50, &port15_50_sff8436),
	mk_i2cdev(W100_I2C_MUX2_BUS6, "sff8436", 0x50, &port16_50_sff8436),

	/* devices on pca9548 3 */
	mk_i2cdev(W100_I2C_MUX3_BUS1, "sff8436", 0x50, &port17_50_sff8436),
	mk_i2cdev(W100_I2C_MUX3_BUS0, "sff8436", 0x50, &port18_50_sff8436),
	mk_i2cdev(W100_I2C_MUX3_BUS3, "sff8436", 0x50, &port19_50_sff8436),
	mk_i2cdev(W100_I2C_MUX3_BUS2, "sff8436", 0x50, &port20_50_sff8436),
	mk_i2cdev(W100_I2C_MUX3_BUS5, "sff8436", 0x50, &port21_50_sff8436),
	mk_i2cdev(W100_I2C_MUX3_BUS4, "sff8436", 0x50, &port22_50_sff8436),
	mk_i2cdev(W100_I2C_MUX3_BUS7, "sff8436", 0x50, &port23_50_sff8436),
	mk_i2cdev(W100_I2C_MUX3_BUS6, "sff8436", 0x50, &port24_50_sff8436),

	/* devices on pca9548 4 */
	mk_i2cdev(W100_I2C_MUX4_BUS1, "sff8436", 0x50, &port25_50_sff8436),
	mk_i2cdev(W100_I2C_MUX4_BUS0, "sff8436", 0x50, &port26_50_sff8436),
	mk_i2cdev(W100_I2C_MUX4_BUS3, "sff8436", 0x50, &port27_50_sff8436),
	mk_i2cdev(W100_I2C_MUX4_BUS2, "sff8436", 0x50, &port28_50_sff8436),
	mk_i2cdev(W100_I2C_MUX4_BUS5, "sff8436", 0x50, &port29_50_sff8436),
	mk_i2cdev(W100_I2C_MUX4_BUS4, "sff8436", 0x50, &port30_50_sff8436),
	mk_i2cdev(W100_I2C_MUX4_BUS7, "sff8436", 0x50, &port31_50_sff8436),
	mk_i2cdev(W100_I2C_MUX4_BUS6, "sff8436", 0x50, &port32_50_sff8436),
};


/*
 * Utility functions for GPIO
 */

static void free_gpio_pins(struct gpio_pin *pins, int num_pins, int gpio_base)
{
	while (num_pins--)
		gpio_free(gpio_base + (pins++)->pin);
}

static int init_gpio_pins(struct gpio_pin *pins, int num_pins, int gpio_base)
{
	int ret;
	int i;

	for (i = 0; i < num_pins; i++, pins++) {
		ret = gpio_request_one(gpio_base + pins->pin, pins->flags,
				       pins->label);
		if (ret) {
			if (ret != -EPROBE_DEFER) {
				pr_err(DRIVER_NAME
				       ": Failed to request %d GPIO pin"
				       " %s, err %d\n", gpio_base + pins->pin,
				       pins->label, ret);
			}
			goto err_exit;
		}
	}
	return 0;

err_exit:
	while (i--) {
		gpio_free(gpio_base + (--pins)->pin);
	}
	return ret;
}

static void gpio_free_all(void)
{
	free_gpio_pins(gpio6_pins, ARRAY_SIZE(gpio6_pins),
		       gpio6_platform_data.gpio_base);
	free_gpio_pins(gpio5_pins, ARRAY_SIZE(gpio5_pins),
		       gpio5_platform_data.gpio_base);
	free_gpio_pins(gpio4_pins, ARRAY_SIZE(gpio4_pins),
		       gpio4_platform_data.gpio_base);
	free_gpio_pins(gpio3_pins, ARRAY_SIZE(gpio3_pins),
		       gpio3_platform_data.gpio_base);
	free_gpio_pins(gpio2_pins, ARRAY_SIZE(gpio2_pins),
		       gpio2_platform_data.gpio_base);
	free_gpio_pins(gpio1_pins, ARRAY_SIZE(gpio1_pins),
		       gpio1_platform_data.gpio_base);
}

/*
 * Module init and exit for GPIOs
 */

static void accton_wedge100_gpio_exit(void)
{
	gpio_free_all();
}

static int accton_wedge100_gpio_init(void)
{
	int ret;

	ret = init_gpio_pins(gpio1_pins, ARRAY_SIZE(gpio1_pins),
			     gpio1_platform_data.gpio_base);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(gpio2_pins, ARRAY_SIZE(gpio2_pins),
			     gpio2_platform_data.gpio_base);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(gpio3_pins, ARRAY_SIZE(gpio3_pins),
			     gpio3_platform_data.gpio_base);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(gpio4_pins, ARRAY_SIZE(gpio4_pins),
			     gpio4_platform_data.gpio_base);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(gpio5_pins, ARRAY_SIZE(gpio5_pins),
			     gpio5_platform_data.gpio_base);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(gpio6_pins, ARRAY_SIZE(gpio6_pins),
			     gpio6_platform_data.gpio_base);
	if (ret)
		goto err_exit;

	return 0;

err_exit:
	gpio_free_all();
	return ret;
}


/*
 * Utility functions for I2C
 */

static struct i2c_adapter *get_adapter(int bus)
{
	int bail;
	struct i2c_adapter *adapter;

	for (bail = 50; --bail >= 0;) {
		adapter = i2c_get_adapter(bus);
		if (adapter)
			return adapter;
		msleep(100);
	}
	return NULL;
}

static int get_bus_by_name(char *name)
{
	struct i2c_adapter *adapter;
	int i;

	for (i = 0; i < W100_I2C_MUX1_BUS0; i++) {
		adapter = get_adapter(i);
		if (adapter) {
			if (strncmp(adapter->name, name, strlen(name)) == 0) {
				i2c_put_adapter(adapter);
				return i;
			}
			i2c_put_adapter(adapter);
		}
	}
	return -1;
}

static int check_i2c_match(struct device *dev, void *data)
{
	struct platform_i2c_device_info *plat_info = data;
	struct i2c_client *client;

	client = i2c_verify_client(dev);
	if (client) {
		if (client->addr == plat_info->board_info.addr) {
			plat_info->client = client;
			return 1;
		}
	}
	return 0;
}

static struct i2c_client *add_i2c_client(int bus,
				 struct platform_i2c_device_info *plat_info)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	struct i2c_board_info *board_info = &plat_info->board_info;

	adapter = get_adapter(bus);
	if (!adapter) {
		pr_err(DRIVER_NAME ": Could not get I2C adapter %d\n", bus);
		client = ERR_PTR(-ENODEV);
		goto exit;
	}

	if (!device_for_each_child(&adapter->dev, plat_info, check_i2c_match)) {
		client = i2c_new_device(adapter, board_info);
		if (!client) {
			pr_err(DRIVER_NAME ": Could not add I2C device at bus"
			       " %d type %s addr %#x\n",
			       bus, board_info->type, board_info->addr);
			client = ERR_PTR(-ENODEV);
		}
	}
	else {
		client = plat_info->client;
	}
	i2c_put_adapter(adapter);
exit:
	return client;
}

static int init_i2c_devices(void)
{
	int cp2112_bus;
	int i;
	int ret;

	cp2112_bus = get_bus_by_name("CP2112 SMBus Bridge");
	if (cp2112_bus < 0) {
		pr_err(DRIVER_NAME ": Could not find CP2112 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == W100_I2C_CP2112_BUS)
			bus = cp2112_bus;

		client = add_i2c_client(bus, &i2c_devices[i]);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_devices[i].client = client;
	}
	return 0;

err_exit:
	return ret;
}

static void free_i2c_devices(void)
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

static void init_gpio_platform_data(struct gpio_pin *pins,
				    int num_pins,
				    struct pca953x_platform_data *pdata)
{
	int i;
	/* pdata->names is *const*, we have to cast it */
	const char **names = (const char **)pdata->names;

	for (i = 0; i < num_pins; i++) {
		names[pins[i].pin] = pins[i].label;
	}
}

/*
 * Module init and exit for I2C devices
 */

static int accton_wedge100_i2c_init(void)
{
	int ret;

	init_gpio_platform_data(gpio1_pins, ARRAY_SIZE(gpio1_pins),
				&gpio1_platform_data);
	init_gpio_platform_data(gpio2_pins, ARRAY_SIZE(gpio2_pins),
				&gpio2_platform_data);
	init_gpio_platform_data(gpio3_pins, ARRAY_SIZE(gpio3_pins),
				&gpio3_platform_data);
	init_gpio_platform_data(gpio4_pins, ARRAY_SIZE(gpio4_pins),
				&gpio4_platform_data);
	init_gpio_platform_data(gpio5_pins, ARRAY_SIZE(gpio5_pins),
				&gpio5_platform_data);
	init_gpio_platform_data(gpio6_pins, ARRAY_SIZE(gpio6_pins),
				&gpio6_platform_data);

	ret = init_i2c_devices();
	if (ret)
		goto err_exit;

	return 0;

err_exit:
	free_i2c_devices();
	return ret;
}

static void accton_wedge100_i2c_exit(void)
{
	free_i2c_devices();
}

/*
 * CPLD driver
 */

#define CPLD_NREGS 0x40

#define cpld_read_reg cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/*
 * CPLD register definitions
 * Only the useful ones are exposed.
 */

static const char * const cpld_board_model_values[] = {
	"wdege100",
	"6-pack line card",
	"6-pack fabric card",
	"reserved",
};

mk_bf_ro(cpld, board_revision,  0x00, 0, 4, NULL, BF_DECIMAL);
mk_bf_ro(cpld, board_model,     0x00, 4, 2, cpld_board_model_values, 0);

mk_bf_ro(cpld, cpld_revision,   0x01, 0, 6, NULL, BF_DECIMAL);
mk_bf_ro(cpld, cpld_released,   0x01, 6, 1, NULL, 0);

mk_bf_ro(cpld, cpld_subversion, 0x02, 0, 8, NULL, BF_DECIMAL);

static const char * const cpld_rov_values[] = {
	"1.2000",
	"1.1750",
	"1.1500",
	"1.2500",
	"1.1000",
	"1.0750",
	"1.0500",
	"1.0250",
	"1.0000",
	"0.9250",
	"0.9500",
	"0.9250",
	"0.9000",
	"0.8750",
	"0.8500",
	"0.8250",
};

mk_bf_ro(cpld, rov, 0x0b, 0, 4, cpld_rov_values, 0);

mk_bf_ro(cpld, reset_reason,         0x0d, 0, 8, NULL, 0);
mk_bf_ro(cpld, reset_reason_source1, 0x0e, 0, 8, NULL, 0);
mk_bf_ro(cpld, reset_reason_source2, 0x0f, 0, 8, NULL, 0);

static const char * const cpld_led_button_values[] = {
	"top-bottom",
	"bottom",
	"top",
	"none",
};

mk_bf_ro(cpld, led_button, 0x1b, 0, 2, cpld_led_button_values, 0);

mk_bf_ro(cpld, psu_pwr1_present, 0x10, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr1_dc_ok,   0x10, 1, 1, NULL, 0);
mk_bf_ro(cpld, psu_pwr1_ac_ok,   0x10, 2, 1, NULL, 0);
mk_bf_ro(cpld, psu_pwr1_alarm,   0x10, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr2_present, 0x10, 4, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr2_dc_ok,   0x10, 5, 1, NULL, 0);
mk_bf_ro(cpld, psu_pwr2_ac_ok,   0x10, 6, 1, NULL, 0);
mk_bf_ro(cpld, psu_pwr2_alarm,   0x10, 7, 1, NULL, BF_COMPLEMENT);

mk_bf_rw(cpld, qsfp_reset, 0x34, 0, 32, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, cp2112_reset, 0x39, 1, 1, NULL, 0);

mk_bf_rw(cpld, led_bcm_reset,         0x3c, 0, 1, NULL, 0);
mk_bf_rw(cpld, led_bcm_enable,        0x3c, 1, 1, NULL, 0);
mk_bf_rw(cpld, led_act_enable,         0x3c, 2, 1, NULL, 0);
mk_bf_rw(cpld, led_walk_test_enable,  0x3c, 3, 1, NULL, 0);
mk_bf_rw(cpld, led_test_mode,         0x3c, 4, 2, NULL, 0);
mk_bf_rw(cpld, led_test_blink_enable, 0x3c, 6, 1, NULL, 0);
mk_bf_rw(cpld, led_test_mode_enable,  0x3c, 7, 1, NULL, 0);

mk_bf_rw(cpld, led_test_num,          0x3d, 0, 5, NULL, 0);
mk_bf_rw(cpld, led_test_green,        0x3d, 5, 1, NULL, 0);
mk_bf_rw(cpld, led_test_blue,         0x3d, 6, 1, NULL, 0);
mk_bf_rw(cpld, led_test_red,          0x3d, 7, 1, NULL, 0);

static const char * const cpld_led_system_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_RED,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_YELLOW,
	PLATFORM_LED_BLUE,
	"magenta",
	"cyan",
	"white",
	PLATFORM_LED_OFF,
	PLATFORM_LED_RED_BLINKING,
	PLATFORM_LED_GREEN_BLINKING,
	PLATFORM_LED_YELLOW_BLINKING,
	PLATFORM_LED_BLUE_BLINKING,
	"magenta_blinking",
	"cyan_blinking",
	"white",
};

mk_bf_rw(cpld, led_system1, 0x3e, 0, 4, cpld_led_system_values, 0);
mk_bf_rw(cpld, led_system2, 0x3f, 0, 4, cpld_led_system_values, 0);

struct attribute *cpld_attrs[] = {
	&cpld_board_revision.attr,
	&cpld_board_model.attr,
	&cpld_cpld_revision.attr,
	&cpld_cpld_released.attr,
	&cpld_cpld_subversion.attr,
	&cpld_rov.attr,
	&cpld_reset_reason.attr,
	&cpld_reset_reason_source1.attr,
	&cpld_reset_reason_source2.attr,
	&cpld_led_button.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr1_dc_ok.attr,
	&cpld_psu_pwr1_ac_ok.attr,
	&cpld_psu_pwr1_alarm.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr2_dc_ok.attr,
	&cpld_psu_pwr2_ac_ok.attr,
	&cpld_psu_pwr2_alarm.attr,
	&cpld_led_bcm_reset.attr,
	&cpld_led_bcm_enable.attr,
	&cpld_led_act_enable.attr,
	&cpld_led_walk_test_enable.attr,
	&cpld_led_test_mode.attr,
	&cpld_led_test_blink_enable.attr,
	&cpld_led_test_mode_enable.attr,
	&cpld_led_test_num.attr,
	&cpld_led_test_green.attr,
	&cpld_led_test_blue.attr,
	&cpld_led_test_red.attr,
	&cpld_led_system1.attr,
	&cpld_led_system2.attr,
	&cpld_qsfp_reset.attr,
	&cpld_cp2112_reset.attr,
	NULL
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

static int cpld_probe(struct i2c_client *client,
		      const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int boardrev;
	int cpldrev;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev,
			"adapter does not support I2C_FUNC_SMBUS_BYTE_DATA\n");
		ret = -EINVAL;
		goto err;
	}

	/*
	 * Probe the hardware by reading the revision numbers.
	 */
	ret = i2c_smbus_read_byte_data(client, 0);
	if (ret < 0) {
		dev_err(&client->dev,
			"read board revision register error %d\n", ret);
		goto err;
	}
	boardrev = ret;
	ret = i2c_smbus_read_byte_data(client, 1);
	if (ret < 0) {
		dev_err(&client->dev,
			"read CPLD revision register error %d\n", ret);
		goto err;
	}
	cpldrev = ret;

	/*
	 * Create sysfs nodes.
	 */
	ret = sysfs_create_group(&client->dev.kobj, &cpld_attr_group);
	if (ret) {
		dev_err(&client->dev, "sysfs_create_group failed\n");
		goto err;
	}

	/*
	 * All clear from this point on
	 */
	dev_info(&client->dev,
		 "device created, board model %d rev %d, CPLD rev %d %s\n",
		 (boardrev & 0x30) >> 4,
		 boardrev & 0x0f,
		 cpldrev & 0x3f,
		 cpldrev & 0x40 ? "released" : "unreleased");

#ifdef DEBUG
	for (i = 0; i < CPLD_NREGS; i++) {
		ret = i2c_smbus_read_byte_data(client, i);
		dev_dbg(&client->dev,
			ret < 0 ? "CPLD[%d] read error %d\n" :
				  "CPLD[%d] %#04x\n",
			i, ret);
	}
#endif
	return 0;

err:
	return ret;
}

static int cpld_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &cpld_attr_group);
	dev_info(&client->dev, "device removed\n");
	return 0;
}

static const struct i2c_device_id accton_wedge100_cpld_id[] = {
	{ "act_wedge100_cpld", 0 },	/* full name is too long, ick */
	{ }
};
MODULE_DEVICE_TABLE(i2c, accton_wedge100_cpld_id);

static struct i2c_driver cpld_driver = {
	.driver = {
		.name = "accton_wedge100_cpld",
		.owner = THIS_MODULE,
	},
	.probe = cpld_probe,
	.remove = cpld_remove,
	.id_table = accton_wedge100_cpld_id,
};


/*
 *  Platform driver
 */

static int platform_probe(struct platform_device *dev)
{
	int ret;

	ret = accton_wedge100_i2c_init();
	if (ret) {
		dev_err(&dev->dev, "I2C initialization failed (%d)\n", ret);
		return ret;
	}

	ret = accton_wedge100_gpio_init();
	if (ret) {
		if (ret != -EPROBE_DEFER) {
			accton_wedge100_i2c_exit();
			dev_err(&dev->dev, "GPIO initialization failed (%d)\n",
			        ret);
		}
		else {
			dev_info(&dev->dev, "GPIO initialization deferred\n");
		}
		return ret;
	}

	return 0;
}

static int platform_remove(struct platform_device *dev)
{
	accton_wedge100_gpio_exit();
	accton_wedge100_i2c_exit();
	return 0;
}

static const struct platform_device_id accton_wedge100_platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, accton_wedge100_platform_id);

static struct platform_driver platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = platform_probe,
	.remove = platform_remove,
	.id_table = accton_wedge100_platform_id
};
static struct platform_device *plat_device = NULL;


/*
 * Module init and exit
 */

static int __init accton_wedge100_init(void)
{
	int ret = 0;

	/*
	 * Do as little as possible in the module init function. Basically just
	 * register drivers. Those driver's probe functions will probe for
	 * hardware and create devices.
	 */
	pr_info(DRIVER_NAME": version "DRIVER_VERSION" initializing\n");

	/* Register the CPLD driver */
	if (!driver_find(cpld_driver.driver.name, &i2c_bus_type)) {
		ret = i2c_add_driver(&cpld_driver);
		if (ret) {
			pr_err(DRIVER_NAME ": %s driver registration failed."
			       "(%d)\n", cpld_driver.driver.name, ret);
			return ret;
		}
	}

	/* Register the platform driver */
	if (!driver_find(platform_driver.driver.name, &platform_bus_type)) {
		ret = platform_driver_register(&platform_driver);
		if (ret) {
			pr_err(DRIVER_NAME ": %s driver registration failed."
			       "(%d)\n", platform_driver.driver.name, ret);
			i2c_del_driver(&cpld_driver);
			return ret;
		}
	}

	/* Create the platform device */
	if (plat_device == NULL) {
		plat_device = platform_device_register_simple(DRIVER_NAME, -1,
							      NULL, 0);
		if (IS_ERR(plat_device)) {
			ret = PTR_ERR(plat_device);
			plat_device = NULL;
			pr_err(DRIVER_NAME": Platform device registration"
			       "failed. (%d)\n", ret);
			platform_driver_unregister(&platform_driver);
			i2c_del_driver(&cpld_driver);
			return ret;
		}
	}

	pr_info(DRIVER_NAME": version "DRIVER_VERSION
		" successfully initialized\n");
	return ret;
}

static void __exit accton_wedge100_exit(void)
{
	platform_device_unregister(plat_device);
	plat_device = NULL;
	platform_driver_unregister(&platform_driver);
	i2c_del_driver(&cpld_driver);
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

module_init(accton_wedge100_init);
module_exit(accton_wedge100_exit);

MODULE_AUTHOR("Ellen Wang (ellen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton Wedge100 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
