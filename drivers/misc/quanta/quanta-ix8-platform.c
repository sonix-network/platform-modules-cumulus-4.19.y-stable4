/*
 * quanta_ix8_platform.c - Quanta IX8 Platform Support.
 *
 * Copyright (C) 2018,2019 Cumulus Networks, Inc.  All Rights Reserved
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

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "quanta-ix8.h"

#define DRIVER_NAME	"quanta_ix8_platform"
#define DRIVER_VERSION	"1.0"

/*
 * The i801 bus is connected to the following devices on the CPU board:
 *
 *    pca9546 4-channel mux (0x71)
 *	 0 pca9555 io expander (0x20)
 *	 2 pca9554 io expander (0x21)
 *	 3 pca9555 io expander (0x22)
 *
 * The i801 bus is connected to the following devices on the main board:
 *    pca9546 4-channel mux (0x72)
 *	 0 cpld#1 io expander sfp28 1-16 (0x38)
 *	   cpld#4 port leds sfp28 and qsfp28 27-56 (0x3a)
 *	   cpld#6 port leds sfp28 and qsfp28 1-26 (0x39)
 *	 1 cpld#2 io expander sfp28 17-32 (0x38)
 *	 2 cpld#3 io expander sfp28 33-48 (0x38)
 *	   board eeprom (0x54)
 *	 3 pca9698 gpio expander qsfp28 49-56 (0x21)
 *	   pca9555 8-bit io expander for board id (0x23)
 *    pca9548 8-channel mux (0x77)
 *	 0 pca9548 8 channel mux sfp28 1-8 (0x73)
 *	 1 pca9548 8 channel mux sfp28 9-16 (0x73)
 *	 2 pca9548 8 channel mux sfp28 17-24 (0x73)
 *	 3 pca9548 8 channel mux sfp28 25-32 (0x73)
 *	 4 pca9548 8 channel mux sfp28 33-40 (0x73)
 *	 5 pca9548 8 channel mux sfp28 41-48 (0x73)
 *	 6 pca9548 8 channel mux qsfp28 49-56 (0x73)
 */

enum {
	I2C_ISMT_BUS = 0,
	I2C_I801_BUS = 1,

	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,

	I2C_MUX2_BUS0,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,

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

mk_pca9545(mux1, I2C_MUX1_BUS0, 1);
mk_pca9545(mux2, I2C_MUX2_BUS0, 1);
mk_pca9548(mux3, I2C_MUX3_BUS0, 1);

mk_pca9548(mux4, I2C_MUX4_BUS0, 1);
mk_pca9548(mux5, I2C_MUX5_BUS0, 1);
mk_pca9548(mux6, I2C_MUX6_BUS0, 1);
mk_pca9548(mux7, I2C_MUX7_BUS0, 1);
mk_pca9548(mux8, I2C_MUX8_BUS0, 1);
mk_pca9548(mux9, I2C_MUX9_BUS0, 1);
mk_pca9548(mux10, I2C_MUX10_BUS0, 1);

mk_eeprom(board, 54, 256, AT24_FLAG_IRUGO);

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

static struct quanta_ix8_platform_data cpld1 = {
	.idx = IX8_IO_SFP28_1_16_CPLD_ID,
};

static struct quanta_ix8_platform_data cpld2 = {
	.idx = IX8_IO_SFP28_17_32_CPLD_ID,
};

static struct quanta_ix8_platform_data cpld3 = {
	.idx = IX8_IO_SFP28_33_48_CPLD_ID,
};

static struct quanta_ix8_platform_data cpld4 = {
	.idx = IX8_LED_SFP28_QSFP28_27_56_CPLD_ID,
};

static struct quanta_ix8_platform_data cpld6 = {
	.idx = IX8_LED_SFP28_1_26_CPLD_ID,
};

/*
 * GPIO definitions
 * num, name, output, active_low, value
 */

mk_gpio_pins(cpu_gpio_20) = {
	mk_gpio_pin(0,	mb_i2c_rst,	     (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(1,	mb_led_rst,	     (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(2,	mb_eth_rst,	     (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)), 
	mk_gpio_pin(3,	mb_sw_rst,	     (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(7,	usb_rst,	     (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),

	mk_gpio_pin(9,	i2c_switch,	     GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(10, boot_led,	     (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(11, sys_led,	     (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(12, board_pwr_led,	     GPIOF_OUT_INIT_HIGH),
};

mk_gpio_pins(cpu_gpio_21) = {
	mk_gpio_pin(0,	sb_id0,		     GPIOF_DIR_IN),
	mk_gpio_pin(1,	sb_id1,		     GPIOF_DIR_IN),
	mk_gpio_pin(2,	sb_id2,		     GPIOF_DIR_IN),
	mk_gpio_pin(3,	sb_id3,		     GPIOF_DIR_IN),
	mk_gpio_pin(4,	sb_id4,		     GPIOF_DIR_IN),
	mk_gpio_pin(5,	sb_id5,		     GPIOF_DIR_IN),
};

mk_gpio_pins(cpu_gpio_22) = {
	mk_gpio_pin(3,	sb_present,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(4,	usb_overcurrent,     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(9,	mb_pca9555_int,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(11, psoc_irq,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(12, psu_9555_irq,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(15, bios_wp,	     (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
};

mk_gpio_pins(mb_gpio_23) = {
	mk_gpio_pin(0,	cpld123_int,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(1,	cpld4_int,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(2,	qsfpio_int,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(4,	sfp28_power_good,    GPIOF_DIR_IN),
	mk_gpio_pin(5,	sfp28_power_enable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(6,	mac_reset,	     (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(7,	usb_reset,	     (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(8,	mgmt_present,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(9,	qsfp_reset,	     (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(10, board_id_0,	     GPIOF_DIR_IN),
	mk_gpio_pin(11, board_id_1,	     GPIOF_DIR_IN),
	mk_gpio_pin(12, board_id_2,	     GPIOF_DIR_IN),
	mk_gpio_pin(13, board_id_3,	     GPIOF_DIR_IN),
	mk_gpio_pin(14, board_id_4,	     GPIOF_DIR_IN),
	mk_gpio_pin(15, board_id_5,	     GPIOF_DIR_IN),
};

mk_gpio_pins(mb_gpio_21) = {
	mk_gpio_pin(0,	qsfp49_reset,	     (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(1,	qsfp49_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(2,	qsfp49_present,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(3,	qsfp49_lpmode,	     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(4,	qsfp50_reset,	     (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(5,	qsfp50_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(6,	qsfp50_present,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(7,	qsfp50_lpmode,	     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(8,	qsfp51_reset,	     (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(9,	qsfp51_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(10, qsfp51_present,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(11, qsfp51_lpmode,	     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, qsfp52_reset,	     (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(13, qsfp52_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(14, qsfp52_present,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(15, qsfp52_lpmode,	     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(16, qsfp53_reset,	     (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(17, qsfp53_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(18, qsfp53_present,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(19, qsfp53_lpmode,	     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(20, qsfp54_reset,	     (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(21, qsfp54_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(22, qsfp54_present,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(23, qsfp54_lpmode,	     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(24, qsfp55_reset,	     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(25, qsfp55_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(26, qsfp55_present,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(27, qsfp55_lpmode,	     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(28, qsfp56_reset,	     (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(29, qsfp56_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(30, qsfp56_present,	     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(31, qsfp56_lpmode,	     GPIOF_OUT_INIT_LOW),
};

mk_gpio_platform_data(cpu_gpio_20, IX8_CPU_GPIO_20_BASE, PCA_9555_GPIO_COUNT);
mk_gpio_platform_data(cpu_gpio_21, IX8_CPU_GPIO_21_BASE, PCA_9554_GPIO_COUNT);
mk_gpio_platform_data(cpu_gpio_22, IX8_CPU_GPIO_22_BASE, PCA_9555_GPIO_COUNT);

mk_gpio_platform_data(mb_gpio_21,  IX8_MB_GPIO_21_BASE,	 PCA_9698_GPIO_COUNT);
mk_gpio_platform_data(mb_gpio_23,  IX8_MB_GPIO_23_BASE,	 PCA_9555_GPIO_COUNT);

static struct platform_i2c_device_info i2c_devices[] = {
	/* cpu board */
	mk_i2cdev(I2C_I801_BUS,	 "pca9546",  0x71, &mux1_platform_data),
	mk_i2cdev(I2C_MUX1_BUS0, "pca9555",  0x20, &cpu_gpio_20_platform_data),
	mk_i2cdev(I2C_MUX1_BUS2, "pca9554",  0x21, &cpu_gpio_21_platform_data),
	mk_i2cdev(I2C_MUX1_BUS3, "pca9555",  0x22, &cpu_gpio_22_platform_data),

	/* main board */
	mk_i2cdev(I2C_I801_BUS,	 "pca9546",  0x72, &mux2_platform_data),
	mk_i2cdev(I2C_I801_BUS,	 "pca9548",  0x77, &mux3_platform_data),

	/* mux2 */
	mk_i2cdev(I2C_MUX2_BUS0, "quanta_ix8_cpld", 0x38, &cpld1),
	mk_i2cdev(I2C_MUX2_BUS0, "quanta_ix8_cpld", 0x3a, &cpld4),
	mk_i2cdev(I2C_MUX2_BUS0, "quanta_ix8_cpld", 0x39, &cpld6),
	mk_i2cdev(I2C_MUX2_BUS1, "quanta_ix8_cpld", 0x38, &cpld2),
	mk_i2cdev(I2C_MUX2_BUS2, "quanta_ix8_cpld", 0x38, &cpld3),
	mk_i2cdev(I2C_MUX2_BUS2, "24c02",    0x54, &board_54_at24),
	mk_i2cdev(I2C_MUX2_BUS3, "pca9698",  0x21, &mb_gpio_21_platform_data),
	mk_i2cdev(I2C_MUX2_BUS3, "pca9555",  0x23, &mb_gpio_23_platform_data),

	/* mux3 */
	mk_i2cdev(I2C_MUX3_BUS0, "pca9548",  0x73, &mux4_platform_data),
	mk_i2cdev(I2C_MUX3_BUS1, "pca9548",  0x73, &mux5_platform_data),
	mk_i2cdev(I2C_MUX3_BUS2, "pca9548",  0x73, &mux6_platform_data),
	mk_i2cdev(I2C_MUX3_BUS3, "pca9548",  0x73, &mux7_platform_data),
	mk_i2cdev(I2C_MUX3_BUS4, "pca9548",  0x73, &mux8_platform_data),
	mk_i2cdev(I2C_MUX3_BUS5, "pca9548",  0x73, &mux9_platform_data),
	mk_i2cdev(I2C_MUX3_BUS6, "pca9548",  0x73, &mux10_platform_data),

	/* mux4 */
	mk_i2cdev(I2C_MUX4_BUS0, "24c04",    0x50, &port1_50_at24),
	mk_i2cdev(I2C_MUX4_BUS1, "24c04",    0x50, &port2_50_at24),
	mk_i2cdev(I2C_MUX4_BUS2, "24c04",    0x50, &port3_50_at24),
	mk_i2cdev(I2C_MUX4_BUS3, "24c04",    0x50, &port4_50_at24),
	mk_i2cdev(I2C_MUX4_BUS4, "24c04",    0x50, &port5_50_at24),
	mk_i2cdev(I2C_MUX4_BUS5, "24c04",    0x50, &port6_50_at24),
	mk_i2cdev(I2C_MUX4_BUS6, "24c04",    0x50, &port7_50_at24),
	mk_i2cdev(I2C_MUX4_BUS7, "24c04",    0x50, &port8_50_at24),

	/* mux5 */
	mk_i2cdev(I2C_MUX5_BUS0, "24c04",    0x50, &port9_50_at24),
	mk_i2cdev(I2C_MUX5_BUS1, "24c04",    0x50, &port10_50_at24),
	mk_i2cdev(I2C_MUX5_BUS2, "24c04",    0x50, &port11_50_at24),
	mk_i2cdev(I2C_MUX5_BUS3, "24c04",    0x50, &port12_50_at24),
	mk_i2cdev(I2C_MUX5_BUS4, "24c04",    0x50, &port13_50_at24),
	mk_i2cdev(I2C_MUX5_BUS5, "24c04",    0x50, &port14_50_at24),
	mk_i2cdev(I2C_MUX5_BUS6, "24c04",    0x50, &port15_50_at24),
	mk_i2cdev(I2C_MUX5_BUS7, "24c04",    0x50, &port16_50_at24),

	/* mux6 */
	mk_i2cdev(I2C_MUX6_BUS0, "24c04",    0x50, &port17_50_at24),
	mk_i2cdev(I2C_MUX6_BUS1, "24c04",    0x50, &port18_50_at24),
	mk_i2cdev(I2C_MUX6_BUS2, "24c04",    0x50, &port19_50_at24),
	mk_i2cdev(I2C_MUX6_BUS3, "24c04",    0x50, &port20_50_at24),
	mk_i2cdev(I2C_MUX6_BUS4, "24c04",    0x50, &port21_50_at24),
	mk_i2cdev(I2C_MUX6_BUS5, "24c04",    0x50, &port22_50_at24),
	mk_i2cdev(I2C_MUX6_BUS6, "24c04",    0x50, &port23_50_at24),
	mk_i2cdev(I2C_MUX6_BUS7, "24c04",    0x50, &port24_50_at24),

	/* mux7 */
	mk_i2cdev(I2C_MUX7_BUS0, "24c04",    0x50, &port25_50_at24),
	mk_i2cdev(I2C_MUX7_BUS1, "24c04",    0x50, &port26_50_at24),
	mk_i2cdev(I2C_MUX7_BUS2, "24c04",    0x50, &port27_50_at24),
	mk_i2cdev(I2C_MUX7_BUS3, "24c04",    0x50, &port28_50_at24),
	mk_i2cdev(I2C_MUX7_BUS4, "24c04",    0x50, &port29_50_at24),
	mk_i2cdev(I2C_MUX7_BUS5, "24c04",    0x50, &port30_50_at24),
	mk_i2cdev(I2C_MUX7_BUS6, "24c04",    0x50, &port31_50_at24),
	mk_i2cdev(I2C_MUX7_BUS7, "24c04",    0x50, &port32_50_at24),

	/* mux8 */
	mk_i2cdev(I2C_MUX8_BUS0, "24c04",    0x50, &port33_50_at24),
	mk_i2cdev(I2C_MUX8_BUS1, "24c04",    0x50, &port34_50_at24),
	mk_i2cdev(I2C_MUX8_BUS2, "24c04",    0x50, &port35_50_at24),
	mk_i2cdev(I2C_MUX8_BUS3, "24c04",    0x50, &port36_50_at24),
	mk_i2cdev(I2C_MUX8_BUS4, "24c04",    0x50, &port37_50_at24),
	mk_i2cdev(I2C_MUX8_BUS5, "24c04",    0x50, &port38_50_at24),
	mk_i2cdev(I2C_MUX8_BUS6, "24c04",    0x50, &port39_50_at24),
	mk_i2cdev(I2C_MUX8_BUS7, "24c04",    0x50, &port40_50_at24),

	/* mux9 */
	mk_i2cdev(I2C_MUX9_BUS0, "24c04",    0x50, &port41_50_at24),
	mk_i2cdev(I2C_MUX9_BUS1, "24c04",    0x50, &port42_50_at24),
	mk_i2cdev(I2C_MUX9_BUS2, "24c04",    0x50, &port43_50_at24),
	mk_i2cdev(I2C_MUX9_BUS3, "24c04",    0x50, &port44_50_at24),
	mk_i2cdev(I2C_MUX9_BUS4, "24c04",    0x50, &port45_50_at24),
	mk_i2cdev(I2C_MUX9_BUS5, "24c04",    0x50, &port46_50_at24),
	mk_i2cdev(I2C_MUX9_BUS6, "24c04",    0x50, &port47_50_at24),
	mk_i2cdev(I2C_MUX9_BUS7, "24c04",    0x50, &port48_50_at24),

	/* mux10 */
	mk_i2cdev(I2C_MUX10_BUS0, "sff8436", 0x50, &port49_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS1, "sff8436", 0x50, &port50_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS2, "sff8436", 0x50, &port51_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS3, "sff8436", 0x50, &port52_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS4, "sff8436", 0x50, &port53_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS5, "sff8436", 0x50, &port54_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS6, "sff8436", 0x50, &port55_50_sff8436),
	mk_i2cdev(I2C_MUX10_BUS7, "sff8436", 0x50, &port56_50_sff8436),
};

/*
 * Utility functions for I2C
 */

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
	int ismt_bus = -1;
	int i801_bus = -1;
	int i;
	int ret;

	ret = -1;
	ismt_bus = cumulus_i2c_find_adapter(SMBUS_ISMT_NAME);
	if (ismt_bus < 0) {
		pr_err("could not find iSMT adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}
	i801_bus = cumulus_i2c_find_adapter(SMBUS_I801_NAME);
	if (i801_bus < 0) {
		pr_err("could not find i801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == I2C_ISMT_BUS)
			bus = ismt_bus;
		if (bus == I2C_I801_BUS)
			bus = i801_bus;
		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
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

/*
 * Utility functions for GPIO
 */

static void init_gpio_platform_data(struct gpio_pin *pins,
				    int num_pins,
				    struct pca953x_platform_data *pdata)
{
	int i;
	/* pdata->names is *const*, we have to cast it */
	const char **names = (const char **)pdata->names;

	for (i = 0; i < num_pins; i++)
		names[pins[i].num] = pins[i].name;
}

static int init_gpio_pins(struct gpio_pin *pins,
                          int num_pins,
                          int gpio_base)
{
        int i;
        int ret;

        pr_debug("Registering %d GPIO pins\n", num_pins);
        for (i = 0; i < num_pins; i++, pins++) {
                ret = gpio_request_one(gpio_base + pins->num,
                                       pins->flags,
                                       pins->name);
                if (ret) {
                        if (ret !=  -EPROBE_DEFER) {
                                pr_err(DRIVER_NAME
                                        "Failed to request %d GPIO pin"
                                        " %s, err %d\n", gpio_base + pins->num,
                                        pins->name, ret);
                        }
                        goto err_exit;
                }
        }
        pr_debug(DRIVER_NAME "Succeeeded in gpio pins create \n");
        return 0;

err_exit:
        while (i--) {
                gpio_free(gpio_base + (--pins)->num);
        }

        return ret;
}

static void free_gpio_pins(struct gpio_pin *pins,
			   int num_pins,
                           int gpio_base)
{
        while (num_pins--) {
                gpio_free(gpio_base + (pins++)->num);
        }
}

/*
 * Module init and exit for all I2C devices, including GPIO
 */

static void free_all(void)
{
	free_gpio_pins(mb_gpio_23_pins, ARRAY_SIZE(mb_gpio_23_pins),
                       IX8_MB_GPIO_23_BASE);
	free_gpio_pins(mb_gpio_21_pins, ARRAY_SIZE(mb_gpio_21_pins),
                       IX8_MB_GPIO_21_BASE);
	free_gpio_pins(cpu_gpio_22_pins, ARRAY_SIZE(cpu_gpio_22_pins),  
                       IX8_CPU_GPIO_22_BASE);
	free_gpio_pins(cpu_gpio_21_pins, ARRAY_SIZE(cpu_gpio_21_pins),  
                       IX8_CPU_GPIO_21_BASE);
	free_gpio_pins(cpu_gpio_20_pins, ARRAY_SIZE(cpu_gpio_20_pins),  
                       IX8_CPU_GPIO_20_BASE);

	i2c_exit();
}

static int __init ix8_platform_init(void)
{
	int ret;

	init_gpio_platform_data(cpu_gpio_20_pins, ARRAY_SIZE(cpu_gpio_20_pins),
				&cpu_gpio_20_platform_data);
	init_gpio_platform_data(cpu_gpio_21_pins, ARRAY_SIZE(cpu_gpio_21_pins),
				&cpu_gpio_21_platform_data);
	init_gpio_platform_data(cpu_gpio_22_pins, ARRAY_SIZE(cpu_gpio_22_pins),
				&cpu_gpio_22_platform_data);
	init_gpio_platform_data(mb_gpio_21_pins, ARRAY_SIZE(mb_gpio_21_pins),
				&mb_gpio_21_platform_data);
	init_gpio_platform_data(mb_gpio_23_pins, ARRAY_SIZE(mb_gpio_23_pins),
				&mb_gpio_23_platform_data);

	ret = i2c_init();
	if (ret)
		goto err_exit;

	ret = init_gpio_pins(cpu_gpio_20_pins, 
                             ARRAY_SIZE(cpu_gpio_20_pins),
                             IX8_CPU_GPIO_20_BASE);
	if (ret)
		goto err_exit;

	ret = init_gpio_pins(cpu_gpio_21_pins, 
                             ARRAY_SIZE(cpu_gpio_21_pins),
                             IX8_CPU_GPIO_21_BASE);
	if (ret)
		goto err_exit;

	ret = init_gpio_pins(cpu_gpio_22_pins, 
                             ARRAY_SIZE(cpu_gpio_22_pins),
                             IX8_CPU_GPIO_22_BASE);
	if (ret)
		goto err_exit;

	ret = init_gpio_pins(mb_gpio_21_pins, 
                             ARRAY_SIZE(mb_gpio_21_pins),
                             IX8_MB_GPIO_21_BASE);
	if (ret)
		goto err_exit;

	ret = init_gpio_pins(mb_gpio_23_pins, 
                             ARRAY_SIZE(mb_gpio_23_pins),
                             IX8_MB_GPIO_23_BASE);
	if (ret)
		goto err_exit;

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;

err_exit:
	pr_err("I2C initialization failed\n");
	free_all();
	return ret;
}

static void __exit ix8_platform_exit(void)
{
	free_all();
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " driver unloaded\n");
}

module_init(ix8_platform_init);
module_exit(ix8_platform_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Quanta IX8 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
