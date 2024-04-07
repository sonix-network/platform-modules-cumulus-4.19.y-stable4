/*
 * quanta_ly7_platform.c - Quanta LY7 Platform Support.
 *
 * Copyright (C) 2017, 2018, 2019, 2020 Cumulus Networks, Inc.  All Rights Reserved
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
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/gpio.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "quanta-ly7.h"
#include "quanta-utils.h"

#define DRIVER_NAME     "quanta_ly7_platform"
#define DRIVER_VERSION  "1.0"

static struct platform_driver ly7_platform_driver;

/*
 * The i801 bus is connected to the following devices:
 *
 * On the CPU board:
 *    cpu eeprom (0x52)
 *    pca9554 8-bit IO expander (0x20)
 *
 * On the main board:
 *    pca9546 4-channel mux (0x72)
 *       0 cpld#1 io expander for sfp+ 1-16 (0x38)
 *         cpld#4 port leds (0x39)
 *       1 cpld#2 io expander for sfp+ 17-32 (0x38)
 *       2 cpld#3 io expander for sfp+ 33-48 (0x38)
 *         board eeprom (0x54)
 *       3 pca9698 40-bit io expander for qsfp28 49-54 (0x21)
 *         pca9555 8-bit io expander for board id (0x23)
 *    pca9548 8-channel mux (0x77)
 *       0 pca9548 8 channel mux for sfp+ 1-8 (0x73)
 *       1 pca9548 8 channel mux for sfp+ 9-16 (0x73)
 *       2 pca9548 8 channel mux for sfp+ 17-24 (0x73)
 *       3 pca9548 8 channel mux for sfp+ 25-32 (0x73)
 *       4 pca9548 8 channel mux for sfp+ 33-40 (0x73)
 *       5 pca9548 8 channel mux for sfp+ 41-48 (0x73)
 *       6 pca9548 8 channel mux for qsfp28 49-52 (0x73)
 */

enum {
	I2C_I801_BUS = -10,

	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,

	I2C_MUX2_BUS0 = 20,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,
	I2C_MUX2_BUS4,
	I2C_MUX2_BUS5,
	I2C_MUX2_BUS6,
	I2C_MUX2_BUS7,

	I2C_MUX3_BUS0 = 31,
	I2C_MUX3_BUS1,
	I2C_MUX3_BUS2,
	I2C_MUX3_BUS3,
	I2C_MUX3_BUS4,
	I2C_MUX3_BUS5,
	I2C_MUX3_BUS6,
	I2C_MUX3_BUS7,

	I2C_MUX4_BUS0,
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
mk_pca9548(mux2, I2C_MUX2_BUS0, 1);

mk_pca9548(mux3, I2C_MUX3_BUS0, 1);
mk_pca9548(mux4, I2C_MUX4_BUS0, 1);
mk_pca9548(mux5, I2C_MUX5_BUS0, 1);
mk_pca9548(mux6, I2C_MUX6_BUS0, 1);
mk_pca9548(mux7, I2C_MUX7_BUS0, 1);
mk_pca9548(mux8, I2C_MUX8_BUS0, 1);
mk_pca9548(mux9, I2C_MUX9_BUS0, 1);

mk_eeprom(cpu,   52, 256, AT24_FLAG_IRUGO);
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
mk_qsfp_port_eeprom(port49,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port50,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port51,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port52,  50, 256,  SFF_8436_FLAG_IRUGO);

static struct quanta_ly7_platform_data cpld1 = {
	.idx = LY7_IO_SFP28_1_16_CPLD_ID,
};

static struct quanta_ly7_platform_data cpld2 = {
	.idx = LY7_IO_SFP28_17_32_CPLD_ID,
};

static struct quanta_ly7_platform_data cpld3 = {
	.idx = LY7_IO_SFP28_33_48_CPLD_ID,
};

static struct quanta_ly7_platform_data cpld4 = {
	.idx = LY7_LED_CPLD_ID,
};

/*
 * GPIO definitions
 * num, name, output, active_low, value
 */

mk_gpio_pins(cpu_gpio_23) = {
	mk_gpio_pin(0,  cpld123_int,         (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(1,  cpld4_int,           (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(2,  9698_int,            (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(3,  os_boot_ready,       (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(4,  qsfp28_power_good,   GPIOF_DIR_IN),
	mk_gpio_pin(5,  qsfp28_power_enable, GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(6,  mac_reset,           GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,  usb_reset,           GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,  mgmt_present,        (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(9,  pca9698_reset,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(10, board_id_0,          GPIOF_DIR_IN),
	mk_gpio_pin(11, board_id_1,          GPIOF_DIR_IN),
	mk_gpio_pin(12, board_id_2,          GPIOF_DIR_IN),
	mk_gpio_pin(13, board_id_3,          GPIOF_DIR_IN),
	mk_gpio_pin(14, board_id_4,          GPIOF_DIR_IN),
	mk_gpio_pin(15, board_id_5,          GPIOF_DIR_IN),
};

mk_gpio_pins(cpu_gpio_21) = {
	mk_gpio_pin(0,  qsfp49_reset,        (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(1,  qsfp49_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(2,  qsfp49_present,      (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(3,  qsfp49_lpmode,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(4,  qsfp50_reset,        (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(5,  qsfp50_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(6,  qsfp50_present,      (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(7,  qsfp50_lpmode,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,  qsfp51_reset,        (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(9,  qsfp51_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(10, qsfp51_present,      (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(11, qsfp51_lpmode,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(12, qsfp52_reset,        (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(13, qsfp52_interrupt,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(14, qsfp52_present,      (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(15, qsfp52_lpmode,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(16, nc_16,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(17, nc_17,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(18, nc_18,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(19, nc_19,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(20, nc_20,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(21, nc_21,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(22, nc_22,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(23, nc_23,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(24, nc_24,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(25, nc_25,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(26, nc_26,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(27, nc_27,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(28, nc_28,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(29, nc_29,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(30, nc_30,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(31, nc_31,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(32, nc_32,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(33, nc_33,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(34, nc_34,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(35, nc_35,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(36, nc_36,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(37, nc_37,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(38, nc_38,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(39, nc_39,       GPIOF_OUT_INIT_HIGH),
};

mk_gpio_platform_data(cpu_gpio_23, LY7_GPIO_23_BASE,   PCA_9555_GPIO_COUNT,
		      &cpu_gpio_23_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(cpu_gpio_21, LY7_GPIO_21_BASE,   PCA_9698_GPIO_COUNT,
		      &cpu_gpio_21_pins, init_gpio_pins, free_gpio_pins);

static struct platform_i2c_device_info i2c_devices[] = {
	/* devices on I801 bus */

	/* main board */
	mk_i2cdev(I2C_I801_BUS,  "pca9546",  0x72, &mux1_platform_data),
	mk_i2cdev(I2C_I801_BUS,  "pca9548",  0x77, &mux2_platform_data),

	/* mux1 */
	mk_i2cdev(I2C_MUX1_BUS0, "quanta_ly7_cpld", 0x38, &cpld1),
	mk_i2cdev(I2C_MUX1_BUS0, "quanta_ly7_cpld", 0x39, &cpld4),
	mk_i2cdev(I2C_MUX1_BUS1, "quanta_ly7_cpld", 0x38, &cpld2),
	mk_i2cdev(I2C_MUX1_BUS2, "quanta_ly7_cpld", 0x38, &cpld3),
	mk_i2cdev(I2C_MUX1_BUS2, "24c02",    0x54, &board_54_at24),
	mk_i2cdev(I2C_MUX1_BUS3, "pca9698",  0x21, &cpu_gpio_21_platform_data),
	mk_i2cdev(I2C_MUX1_BUS3, "pca9555",  0x23, &cpu_gpio_23_platform_data),

	/* mux2 */
	mk_i2cdev(I2C_MUX2_BUS0, "pca9548",  0x73, &mux3_platform_data),
	mk_i2cdev(I2C_MUX2_BUS1, "pca9548",  0x73, &mux4_platform_data),
	mk_i2cdev(I2C_MUX2_BUS2, "pca9548",  0x73, &mux5_platform_data),
	mk_i2cdev(I2C_MUX2_BUS3, "pca9548",  0x73, &mux6_platform_data),
	mk_i2cdev(I2C_MUX2_BUS4, "pca9548",  0x73, &mux7_platform_data),
	mk_i2cdev(I2C_MUX2_BUS5, "pca9548",  0x73, &mux8_platform_data),
	mk_i2cdev(I2C_MUX2_BUS6, "pca9548",  0x73, &mux9_platform_data),

	/* mux 3 */
	mk_i2cdev(I2C_MUX3_BUS0, "24c04",    0x50, &port1_50_at24),
	mk_i2cdev(I2C_MUX3_BUS1, "24c04",    0x50, &port2_50_at24),
	mk_i2cdev(I2C_MUX3_BUS2, "24c04",    0x50, &port3_50_at24),
	mk_i2cdev(I2C_MUX3_BUS3, "24c04",    0x50, &port4_50_at24),
	mk_i2cdev(I2C_MUX3_BUS4, "24c04",    0x50, &port5_50_at24),
	mk_i2cdev(I2C_MUX3_BUS5, "24c04",    0x50, &port6_50_at24),
	mk_i2cdev(I2C_MUX3_BUS6, "24c04",    0x50, &port7_50_at24),
	mk_i2cdev(I2C_MUX3_BUS7, "24c04",    0x50, &port8_50_at24),

	/* mux 4 */
	mk_i2cdev(I2C_MUX4_BUS0, "24c04",    0x50, &port9_50_at24),
	mk_i2cdev(I2C_MUX4_BUS1, "24c04",    0x50, &port10_50_at24),
	mk_i2cdev(I2C_MUX4_BUS2, "24c04",    0x50, &port11_50_at24),
	mk_i2cdev(I2C_MUX4_BUS3, "24c04",    0x50, &port12_50_at24),
	mk_i2cdev(I2C_MUX4_BUS4, "24c04",    0x50, &port13_50_at24),
	mk_i2cdev(I2C_MUX4_BUS5, "24c04",    0x50, &port14_50_at24),
	mk_i2cdev(I2C_MUX4_BUS6, "24c04",    0x50, &port15_50_at24),
	mk_i2cdev(I2C_MUX4_BUS7, "24c04",    0x50, &port16_50_at24),

	/* mux 5 */
	mk_i2cdev(I2C_MUX5_BUS0, "24c04",    0x50, &port17_50_at24),
	mk_i2cdev(I2C_MUX5_BUS1, "24c04",    0x50, &port18_50_at24),
	mk_i2cdev(I2C_MUX5_BUS2, "24c04",    0x50, &port19_50_at24),
	mk_i2cdev(I2C_MUX5_BUS3, "24c04",    0x50, &port20_50_at24),
	mk_i2cdev(I2C_MUX5_BUS4, "24c04",    0x50, &port21_50_at24),
	mk_i2cdev(I2C_MUX5_BUS5, "24c04",    0x50, &port22_50_at24),
	mk_i2cdev(I2C_MUX5_BUS6, "24c04",    0x50, &port23_50_at24),
	mk_i2cdev(I2C_MUX5_BUS7, "24c04",    0x50, &port24_50_at24),

	/* mux 6 */
	mk_i2cdev(I2C_MUX6_BUS0, "24c04",    0x50, &port25_50_at24),
	mk_i2cdev(I2C_MUX6_BUS1, "24c04",    0x50, &port26_50_at24),
	mk_i2cdev(I2C_MUX6_BUS2, "24c04",    0x50, &port27_50_at24),
	mk_i2cdev(I2C_MUX6_BUS3, "24c04",    0x50, &port28_50_at24),
	mk_i2cdev(I2C_MUX6_BUS4, "24c04",    0x50, &port29_50_at24),
	mk_i2cdev(I2C_MUX6_BUS5, "24c04",    0x50, &port30_50_at24),
	mk_i2cdev(I2C_MUX6_BUS6, "24c04",    0x50, &port31_50_at24),
	mk_i2cdev(I2C_MUX6_BUS7, "24c04",    0x50, &port32_50_at24),

	/* mux 7 */
	mk_i2cdev(I2C_MUX7_BUS0, "24c04",    0x50, &port33_50_at24),
	mk_i2cdev(I2C_MUX7_BUS1, "24c04",    0x50, &port34_50_at24),
	mk_i2cdev(I2C_MUX7_BUS2, "24c04",    0x50, &port35_50_at24),
	mk_i2cdev(I2C_MUX7_BUS3, "24c04",    0x50, &port36_50_at24),
	mk_i2cdev(I2C_MUX7_BUS4, "24c04",    0x50, &port37_50_at24),
	mk_i2cdev(I2C_MUX7_BUS5, "24c04",    0x50, &port38_50_at24),
	mk_i2cdev(I2C_MUX7_BUS6, "24c04",    0x50, &port39_50_at24),
	mk_i2cdev(I2C_MUX7_BUS7, "24c04",    0x50, &port40_50_at24),

	/* mux 8 */
	mk_i2cdev(I2C_MUX8_BUS0, "24c04",    0x50, &port41_50_at24),
	mk_i2cdev(I2C_MUX8_BUS1, "24c04",    0x50, &port42_50_at24),
	mk_i2cdev(I2C_MUX8_BUS2, "24c04",    0x50, &port43_50_at24),
	mk_i2cdev(I2C_MUX8_BUS3, "24c04",    0x50, &port44_50_at24),
	mk_i2cdev(I2C_MUX8_BUS4, "24c04",    0x50, &port45_50_at24),
	mk_i2cdev(I2C_MUX8_BUS5, "24c04",    0x50, &port46_50_at24),
	mk_i2cdev(I2C_MUX8_BUS6, "24c04",    0x50, &port47_50_at24),
	mk_i2cdev(I2C_MUX8_BUS7, "24c04",    0x50, &port48_50_at24),

	/* mux 9 */
	mk_i2cdev(I2C_MUX9_BUS0, "sff8436", 0x50, &port49_50_sff8436),
	mk_i2cdev(I2C_MUX9_BUS1, "sff8436", 0x50, &port50_50_sff8436),
	mk_i2cdev(I2C_MUX9_BUS2, "sff8436", 0x50, &port51_50_sff8436),
	mk_i2cdev(I2C_MUX9_BUS3, "sff8436", 0x50, &port52_50_sff8436),
};

/*
 * Utility functions for I2C
 */

static int init_i2c_devices(void)
{
	int i801_bus = -1;
	int i;
	int ret;

	i801_bus = cumulus_i2c_find_adapter(SMB_I801_NAME);
	if (i801_bus < 0) {
		pr_err("could not find %s adapter bus\n", SMB_I801_NAME);
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
			i2c_unregister_device(c);
			i2c_devices[i].client = NULL;
		}
	}
}

/*
 * Module init and exit for all I2C devices, including GPIO
 */

static int ly7_i2c_init(void)
{
	int ret;

	init_gpio_platform_data(cpu_gpio_23_pins, ARRAY_SIZE(cpu_gpio_23_pins),
				&cpu_gpio_23_platform_data);
	init_gpio_platform_data(cpu_gpio_21_pins, ARRAY_SIZE(cpu_gpio_21_pins),
				&cpu_gpio_21_platform_data);

	/*
	 * Error handling is already done as part of init_i2c_devices;
	 */
	ret = init_i2c_devices();

	return ret;
}

static void ly7_i2c_exit(void)
{
	free_i2c_devices();
}

static int ly7_platform_probe(struct platform_device *dev)
{
	int ret = 0;

	pr_info(DRIVER_NAME ": Probe begin \n");
	ret = ly7_i2c_init();
	if (ret) {
		goto err_exit;
	}

	pr_info(DRIVER_NAME ": Probe Succeeded \n");
	return 0;

err_exit:
	if (ret != -EPROBE_DEFER) {
		dev_info(&dev->dev, "error during probe, deleting clients\n");
		ly7_i2c_exit();
	}

	return ret;
}

static int ly7_platform_remove(struct platform_device *dev)
{
	ly7_i2c_exit();
	return 0;
}

static const struct platform_device_id ly7_platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, ly7_platform_id);

static struct platform_driver ly7_platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = ly7_platform_probe,
	.remove = ly7_platform_remove,
};

static struct platform_device *ly7_plat_device = NULL;

/*
 * Module init and exit
 */

static int __init ly7_platform_init(void)
{
	int ret = 0;

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" initializing\n");

	if (!driver_find(ly7_platform_driver.driver.name,
			 &platform_bus_type)) {
		ret = platform_driver_register(&ly7_platform_driver);
		if (ret) {
			pr_err(DRIVER_NAME ": %s driver registration failed"
			       " with %d  \n", ly7_platform_driver.driver.name,
			       ret);
			return ret;
		}
	}

	if (ly7_plat_device == NULL) {
		ly7_plat_device = platform_device_register_simple(DRIVER_NAME,
								  -1,
								  NULL,
								  0);
		if (IS_ERR(ly7_plat_device)) {
			ret = PTR_ERR(ly7_plat_device);
			ly7_plat_device = NULL;
			pr_err(DRIVER_NAME ": Platform device registration"
			       " failed \n");
			platform_driver_unregister(&ly7_platform_driver);
			return ret;
		}
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit ly7_platform_exit(void)
{
	platform_device_unregister(ly7_plat_device);
	ly7_plat_device = NULL;
	platform_driver_unregister(&ly7_platform_driver);
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " driver unloaded\n");
}

module_init(ly7_platform_init);
module_exit(ly7_platform_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Quanta LY7 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
