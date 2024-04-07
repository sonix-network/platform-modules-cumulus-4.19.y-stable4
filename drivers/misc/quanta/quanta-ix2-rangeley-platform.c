/*
 * quanta_ix2_rangeley_platform.c - Quanta IX2 Platform Support.
 *
 * Copyright (C) 2016, 2017, 2018, 2019 Cumulus Networks, Inc.
 * Author: Vidya Sagar Ravipati (vidya@cumulusnetworks.com)
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
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/at24.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "quanta-ix2-rangeley-platform.h"
#include "quanta-ix-cpld.h"

#define DRIVER_NAME     "quanta_ix2_rangeley"
#define DRIVER_VERSION  "1.0"

static struct platform_driver ix2_platform_driver;

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

mk_pca9545(mux1,  IX2_I2C_MUX1_BUS0,  1);
mk_pca9545(mux2,  IX2_I2C_MUX2_BUS0,  1);
mk_pca9545(mux3,  IX2_I2C_MUX3_BUS0,  1);

mk_pca9548(mux4,  IX2_I2C_MUX4_BUS0,  1);
mk_pca9548(mux5,  IX2_I2C_MUX5_BUS0,  1);
mk_pca9548(mux6,  IX2_I2C_MUX6_BUS0,  1);
mk_pca9548(mux7,  IX2_I2C_MUX7_BUS0,  1);
mk_pca9548(mux8,  IX2_I2C_MUX8_BUS0,  1);
mk_pca9548(mux9,  IX2_I2C_MUX9_BUS0,  1);
mk_pca9548(mux10, IX2_I2C_MUX10_BUS0, 1);
mk_pca9541(mux11, IX2_I2C_MUX11_BUS);

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
mk_qsfp_port_eeprom(port53,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port54,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port55,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port56,  50, 256,  SFF_8436_FLAG_IRUGO);

mk_eeprom(spd1,  52, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(spd2,  53, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(cpu,   50, 256, AT24_FLAG_IRUGO);
mk_eeprom(board, 54, 256, AT24_FLAG_IRUGO);

static struct quanta_ix2_rangeley_platform_data cpld1 = {
	.idx = IX2_IO_SFP28_33_48_CPLD_ID,
};

static struct quanta_ix2_rangeley_platform_data cpld2 = {
	.idx = IX2_IO_SFP28_1_16_CPLD_ID,
};

static struct quanta_ix2_rangeley_platform_data cpld3 = {
	.idx = IX2_LED_1_52_CPLD_ID,
};

static struct quanta_ix2_rangeley_platform_data cpld4 = {
	.idx = IX2_IO_SFP28_17_32_CPLD_ID,
};

static struct quanta_ix2_rangeley_platform_data cpld5 = {
	.idx = IX2_I2C_BUS_MNTR_CPLD_ID,
};

/*
 * This CPLD is defined under PCA9641 which is not
 * included in the code yet. Commenting out for now
 */

static struct quanta_ix2_rangeley_platform_data cpld6 = {
	.idx = IX2_UART_SWITCH_CPLD_ID,
};

/*
 * GPIO definitions
 */
mk_gpio_pins(cpu_gpio_20) = {
	mk_gpio_pin(0,  mb_i2c_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(1,  mb_led_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(2,  mb_eth_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(3,  mb_sw_rst,        GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(4,  mb_odd_phy_rst,   (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(5,  mb_even_phy_rst,  (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(6,  reset_require,    (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(7,  usb_rst,          GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,  power_off,        (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(9,  vr_i2c_switch,    GPIOF_DIR_OUT),
	mk_gpio_pin(10, boot_led,         GPIOF_DIR_OUT),
	mk_gpio_pin(11, sys_led,          GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(12, power_led,        GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(13, bcm_gpio_0,       GPIOF_DIR_IN),
	mk_gpio_pin(14, msata_rst,        (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(15, bmc_gpio_1,       GPIOF_DIR_IN),
};

mk_gpio_pins(cpu_gpio_21) = {
	mk_gpio_pin(0,  cpu_board_id0,    GPIOF_DIR_IN),
	mk_gpio_pin(1,  cpu_board_id1,    GPIOF_DIR_IN),
	mk_gpio_pin(2,  cpu_board_id2,    GPIOF_DIR_IN),
	mk_gpio_pin(3,  cpu_board_id3,    GPIOF_DIR_IN),
	mk_gpio_pin(4,  cpu_board_id4,    GPIOF_DIR_IN),
	mk_gpio_pin(5,  cpu_board_id5,    GPIOF_DIR_IN),
};

mk_gpio_pins(cpu_gpio_22) = {
	mk_gpio_pin(0,  master_reset,      (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(1,  max6695_int_alert, (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(2,  max6695_otp_alert, (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(3,  sb_present,        (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(4,  usb_overcurrent,   (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(5,  gpio_sb,           (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(6,  bmc_gpio_2,        GPIOF_DIR_IN),
	mk_gpio_pin(7,  bmc_gpio_3,        GPIOF_DIR_IN),
	mk_gpio_pin(8,  sb_irq0,           (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(9,  mb_pca9555_int,    (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(10, db_irq1,           (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(11, psoc_irq,          (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(12, psu_9555_irq,      (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(13, irq2_reserve,      GPIOF_DIR_IN),
	mk_gpio_pin(14, rtc_irq_reserved,  GPIOF_DIR_IN),
};

mk_gpio_pins(gpio_26) = {
	mk_gpio_pin(0,  psu_pwr1_present, GPIOF_DIR_IN),
	mk_gpio_pin(1,  psu_pwr1_dc_ok,   GPIOF_DIR_IN),
	mk_gpio_pin(2,  psu_pwr1_int,     GPIOF_DIR_IN),
	mk_gpio_pin(3,  psu_pwr2_present, GPIOF_DIR_IN),
	mk_gpio_pin(4,  psu_pwr2_dc_ok,   GPIOF_DIR_IN),
	mk_gpio_pin(5,  psu_pwr2_int,     GPIOF_DIR_IN),
	mk_gpio_pin(6,  psu_pwr1_ac_ok,   GPIOF_DIR_IN),
	mk_gpio_pin(7,  psu_pwr2_ac_ok,   GPIOF_DIR_IN),
	mk_gpio_pin(8,  psu_pwr1_reset,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,  psu_pwr2_reset,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10, psu1_green_led,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11, psu1_red_led,     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, psu2_green_led,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(13, psu2_red_led,     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14, fan_green_led,    GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(15, fan_red_led,      GPIOF_OUT_INIT_LOW),
};

mk_gpio_pins(gpio_23) = {
	mk_gpio_pin(0,  fan0_board,         GPIOF_DIR_IN),
	mk_gpio_pin(1,  fan1_board,         GPIOF_DIR_IN),
	mk_gpio_pin(2,  fan2_board,         GPIOF_DIR_IN),
	mk_gpio_pin(3,  fan3_board,         GPIOF_DIR_IN),
	mk_gpio_pin(4,  qsfp28_power_good,  GPIOF_DIR_IN),
	mk_gpio_pin(5,  qsfp_power_enable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(6,  mac_reset,          GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,  usb_reset,          GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,  mgmt_present,       (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(9,  pca9698_reset,      GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(10, board_id_0,         GPIOF_DIR_IN),
	mk_gpio_pin(11, board_id_1,         GPIOF_DIR_IN),
	mk_gpio_pin(12, board_id_2,         GPIOF_DIR_IN),
	mk_gpio_pin(13, board_id_3,         GPIOF_DIR_IN),
	mk_gpio_pin(14, board_id_4,         GPIOF_DIR_IN),
	mk_gpio_pin(15, board_id_5,         GPIOF_DIR_IN),
};

mk_gpio_pins(gpio_25) = {
	mk_gpio_pin(0,  cpld235_int,        (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(1,  cpld4_int,          (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(2,  9698_int,           (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(3,  bcm5461s_int,       (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(4,  fan1_present,       GPIOF_DIR_IN),
	mk_gpio_pin(5,  fan2_present,       GPIOF_DIR_IN),
	mk_gpio_pin(6,  fan3_present,       GPIOF_DIR_IN),
	mk_gpio_pin(7,  fan4_present,       GPIOF_DIR_IN),
	mk_gpio_pin(8,  fan1_f2b,           GPIOF_DIR_IN),
	mk_gpio_pin(9,  fan2_f2b,           GPIOF_DIR_IN),
	mk_gpio_pin(10, fan3_f2b,           GPIOF_DIR_IN),
	mk_gpio_pin(11, fan4_f2b,           GPIOF_DIR_IN),
	mk_gpio_pin(12, fan1_fail_led,      (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(13, fan2_fail_led,      (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(14, fan3_fail_led,      (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(15, fan4_fail_led,      (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
};

mk_gpio_pins(gpio_26_2) = {
	mk_gpio_pin(0,  phy88E6320_rst,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(1,  pca9641_rst,     GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(2,  enable_bmc,      (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(3,  bmc_present,     (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(4,  bmc_ready,       (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(5,  phy88E6320_int,  (GPIOF_DIR_IN | GPIOF_ACTIVE_LOW)),
	mk_gpio_pin(6,  bcm5461s_reset,  (GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW)),

};

mk_gpio_pins(gpio_21) = {
	mk_gpio_pin(0,  qsfp49_reset,       GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(1,  qsfp49_interrupt,   GPIOF_DIR_IN),
	mk_gpio_pin(2,  qsfp49_present,     GPIOF_DIR_IN),
	mk_gpio_pin(3,  qsfp49_lpmode,      GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(4,  qsfp50_reset,       GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(5,  qsfp50_interrupt,   GPIOF_DIR_IN),
	mk_gpio_pin(6,  qsfp50_present,     GPIOF_DIR_IN),
	mk_gpio_pin(7,  qsfp50_lpmode,      GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(8,  qsfp51_reset,       GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,  qsfp51_interrupt,   GPIOF_DIR_IN),
	mk_gpio_pin(10, qsfp51_present,     GPIOF_DIR_IN),
	mk_gpio_pin(11, qsfp51_lpmode,      GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12, qsfp52_reset,       GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(13, qsfp52_interrupt,   GPIOF_DIR_IN),
	mk_gpio_pin(14, qsfp52_present,     GPIOF_DIR_IN),
	mk_gpio_pin(15, qsfp52_lpmode,      GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(16, qsfp53_reset,       GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(17, qsfp53_interrupt,   GPIOF_DIR_IN),
	mk_gpio_pin(18, qsfp53_present,     GPIOF_DIR_IN),
	mk_gpio_pin(19, qsfp53_lpmode,      GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(20, qsfp54_reset,       GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(21, qsfp54_interrupt,   GPIOF_DIR_IN),
	mk_gpio_pin(22, qsfp54_present,     GPIOF_DIR_IN),
	mk_gpio_pin(23, qsfp54_lpmode,      GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(24, qsfp55_reset,       GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(25, qsfp55_interrupt,   GPIOF_DIR_IN),
	mk_gpio_pin(26, qsfp55_present,     GPIOF_DIR_IN),
	mk_gpio_pin(27, qsfp55_lpmode,      GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(28, qsfp56_reset,       GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(29, qsfp56_interrupt,   GPIOF_DIR_IN),
	mk_gpio_pin(30, qsfp56_present,     GPIOF_DIR_IN),
	mk_gpio_pin(31, qsfp56_lpmode,      GPIOF_OUT_INIT_LOW),
};

mk_gpio_platform_data(cpu_gpio_20, IX2_GPIO_20_BASE,   PCA_9555_GPIO_COUNT);
mk_gpio_platform_data(cpu_gpio_21, IX2_GPIO_21_BASE,   PCA_9555_GPIO_COUNT);
mk_gpio_platform_data(cpu_gpio_22, IX2_GPIO_22_BASE,   PCA_9555_GPIO_COUNT);
mk_gpio_platform_data(gpio_26,     IX2_GPIO_26_BASE,   PCA_9555_GPIO_COUNT);
mk_gpio_platform_data(gpio_23,     IX2_GPIO_23_BASE,   PCA_9555_GPIO_COUNT);
mk_gpio_platform_data(gpio_25,     IX2_GPIO_25_BASE,   PCA_9555_GPIO_COUNT);
mk_gpio_platform_data(gpio_26_2,   IX2_GPIO_26_2_BASE, PCA_9555_GPIO_COUNT);
mk_gpio_platform_data(gpio_21,     IX2_GPIO_21_2_BASE, PCA_9698_GPIO_COUNT);

static struct platform_i2c_device_info i2c_devices[] = {
	/* devices on I801 bus */
	/* CPU Board */
	/* DDR3 SPD */
	mk_i2cdev(IX2_I2C_I801_BUS, "spd",     0x52, &spd1_52_at24),
	/* DDR3 SPD */
	mk_i2cdev(IX2_I2C_I801_BUS, "spd",     0x53, &spd2_53_at24),
	/* CPLD */
	mk_i2cdev(IX2_I2C_I801_BUS, "dummy",   0x40, NULL),
	/* DDR3 VREF TUNING */
	mk_i2cdev(IX2_I2C_I801_BUS, "dummy",   0x3e, NULL),
	/* PCA9546 */
	mk_i2cdev(IX2_I2C_I801_BUS, "pca9546", 0x71, &mux1_platform_data),

	/* Todo:I2C Channel 1 Communication to BMC */

	/* CPU Board:Devices on MUXes */
	/* PCA9546:0x71 Devices */
	mk_i2cdev(IX2_I2C_MUX1_BUS0, "pca9555", 0x20,
		  &cpu_gpio_20_platform_data),
	/* Switch FRU Data */
	mk_i2cdev(IX2_I2C_MUX1_BUS1, "24c02",   0x50, &cpu_50_at24),
	/* Clock Buffer */
	mk_i2cdev(IX2_I2C_MUX1_BUS1, "dummy",   0x6a, NULL),
	/* PCA9554 expander */
	mk_i2cdev(IX2_I2C_MUX1_BUS2, "pca9554",   0x21,
		  &cpu_gpio_21_platform_data),
	/* CPU ID Identification */
	mk_i2cdev(IX2_I2C_MUX1_BUS3, "pca9555", 0x22,
		  &cpu_gpio_22_platform_data),
	/* Clock Generator */
	mk_i2cdev(IX2_I2C_MUX1_BUS3, "dummy",   0x69, NULL),

	/* Mother Board */
	/* P1VSW PWM Controller */
	mk_i2cdev(IX2_I2C_I801_BUS, "dummy",   0x7e, NULL),
	/* P1VSW PWM Controller */
	mk_i2cdev(IX2_I2C_I801_BUS, "dummy",   0x6e, NULL),
	/* 2 to 1 I2C  bidirectional master */

	mk_i2cdev(IX2_I2C_I801_BUS, "pca9641", 0x08, &mux11_platform_data),
	/* 9546-1 I2C 4-channel switch expander */
	mk_i2cdev(IX2_I2C_I801_BUS, "pca9546", 0x77, &mux2_platform_data),
	/* 9546-2 I2C 4-channel switch expander */
	mk_i2cdev(IX2_I2C_I801_BUS, "pca9546", 0x72, &mux3_platform_data),

	/* Devices on MUXes */
	/*
	 * devices on pca9546 2
	 *
	 * Do this one first so the main EEPROM is eeprom0, and
	 * port EEPROMs are numbered exactly like the ports.
	 * Not required, just nice.
	 */
	/* Redundant PSU module 1 */
	mk_i2cdev(IX2_I2C_MUX3_BUS0, "pmbus",   0x5f, NULL),
	/* Redundant PSU module 2 */
	mk_i2cdev(IX2_I2C_MUX3_BUS1, "pmbus",   0x59, NULL),
	/* PCA9555_2 - IO Expander for PSU and LED status */
	mk_i2cdev(IX2_I2C_MUX3_BUS2, "pca9555", 0x26, &gpio_26_platform_data),
	/* PCA9555_2 - Board ID and ZQSFP PW EN/PG */
	mk_i2cdev(IX2_I2C_MUX3_BUS2, "pca9555", 0x23, &gpio_23_platform_data),
	/* EEPROM - Board Information */
	mk_i2cdev(IX2_I2C_MUX3_BUS2, "24c02",   0x54, &board_54_at24),
	/* PCA9555_3 */
	mk_i2cdev(IX2_I2C_MUX3_BUS3, "pca9555", 0x25, &gpio_25_platform_data),
	/* PCA9555_6 */
	mk_i2cdev(IX2_I2C_MUX3_BUS3, "pca9555", 0x26, &gpio_26_2_platform_data),

	/*
	 * devices on pca9546 1
	 *
	 */
	/* PCA9548_1 - I2C expander of SFP28 1-8 */
	mk_i2cdev(IX2_I2C_MUX2_BUS0, "pca9548",   0x73, &mux4_platform_data),
	/* PCA9548_2 - I2C expander of SFP28 9-16 */
	mk_i2cdev(IX2_I2C_MUX2_BUS0, "pca9548",   0x74, &mux5_platform_data),
	/* PCA9548_3 - I2C expander of SFP28 17-24 */
	mk_i2cdev(IX2_I2C_MUX2_BUS0, "pca9548",   0x75, &mux6_platform_data),
	/* CPLD2: IO  expander of SFP28 1-16 */
	mk_i2cdev(IX2_I2C_MUX2_BUS0, "ix_rangeley_cpld",   0x38, &cpld2),
	/* CPLD4: IO  expander of SFP28 17-32 */
	mk_i2cdev(IX2_I2C_MUX2_BUS0, "ix_rangeley_cpld",   0x39, &cpld4),
	/* PCA9548_4 - I2C expander of SFP28 25-32 */
	mk_i2cdev(IX2_I2C_MUX2_BUS1, "pca9548",   0x73, &mux7_platform_data),
	/* PCA9548_5 - I2C expander of SFP28 33-40 */
	mk_i2cdev(IX2_I2C_MUX2_BUS1, "pca9548",   0x74, &mux8_platform_data),
	/* PCA9548_6 - I2C expander of SFP28 41-48 */
	mk_i2cdev(IX2_I2C_MUX2_BUS1, "pca9548",   0x75, &mux9_platform_data),
	/* CPLD1 - IO  expander of SFP28 33-48 */
	mk_i2cdev(IX2_I2C_MUX2_BUS1, "ix_rangeley_cpld", 0x38, &cpld1),
	/* PCA9698 - IO  expander of QSFP28 1-8 */
	mk_i2cdev(IX2_I2C_MUX2_BUS1, "pca9698",   0x21, &gpio_21_platform_data),
	/* PCA9548_7 - I2C expander of QSFP28 1-8 */
	mk_i2cdev(IX2_I2C_MUX2_BUS2, "pca9548",   0x73, &mux10_platform_data),
	/* CPLD3 -  LED funciton of SFP28 1-48 & QSFP28 1-8 */
	mk_i2cdev(IX2_I2C_MUX2_BUS3, "ix_rangeley_cpld",   0x38, &cpld3),
	/* CPLD5: Ii2C monitor function of PCA9548_1-7 */
	mk_i2cdev(IX2_I2C_MUX2_BUS3, "ix_rangeley_cpld",   0x3f, &cpld5),

	/* devices on pca9548 1 */
	mk_i2cdev(IX2_I2C_MUX4_BUS0, "24c04", 0x50, &port1_50_at24),
	mk_i2cdev(IX2_I2C_MUX4_BUS1, "24c04", 0x50, &port2_50_at24),
	mk_i2cdev(IX2_I2C_MUX4_BUS2, "24c04", 0x50, &port3_50_at24),
	mk_i2cdev(IX2_I2C_MUX4_BUS3, "24c04", 0x50, &port4_50_at24),
	mk_i2cdev(IX2_I2C_MUX4_BUS4, "24c04", 0x50, &port5_50_at24),
	mk_i2cdev(IX2_I2C_MUX4_BUS5, "24c04", 0x50, &port6_50_at24),
	mk_i2cdev(IX2_I2C_MUX4_BUS6, "24c04", 0x50, &port7_50_at24),
	mk_i2cdev(IX2_I2C_MUX4_BUS7, "24c04", 0x50, &port8_50_at24),

	/* devices on pca9548 2 */
	mk_i2cdev(IX2_I2C_MUX5_BUS0, "24c04", 0x50, &port9_50_at24),
	mk_i2cdev(IX2_I2C_MUX5_BUS1, "24c04", 0x50, &port10_50_at24),
	mk_i2cdev(IX2_I2C_MUX5_BUS2, "24c04", 0x50, &port11_50_at24),
	mk_i2cdev(IX2_I2C_MUX5_BUS3, "24c04", 0x50, &port12_50_at24),
	mk_i2cdev(IX2_I2C_MUX5_BUS4, "24c04", 0x50, &port13_50_at24),
	mk_i2cdev(IX2_I2C_MUX5_BUS5, "24c04", 0x50, &port14_50_at24),
	mk_i2cdev(IX2_I2C_MUX5_BUS6, "24c04", 0x50, &port15_50_at24),
	mk_i2cdev(IX2_I2C_MUX5_BUS7, "24c04", 0x50, &port16_50_at24),

	/* devices on pca9548 3 */
	mk_i2cdev(IX2_I2C_MUX6_BUS0, "24c04", 0x50, &port17_50_at24),
	mk_i2cdev(IX2_I2C_MUX6_BUS1, "24c04", 0x50, &port18_50_at24),
	mk_i2cdev(IX2_I2C_MUX6_BUS2, "24c04", 0x50, &port19_50_at24),
	mk_i2cdev(IX2_I2C_MUX6_BUS3, "24c04", 0x50, &port20_50_at24),
	mk_i2cdev(IX2_I2C_MUX6_BUS4, "24c04", 0x50, &port21_50_at24),
	mk_i2cdev(IX2_I2C_MUX6_BUS5, "24c04", 0x50, &port22_50_at24),
	mk_i2cdev(IX2_I2C_MUX6_BUS6, "24c04", 0x50, &port23_50_at24),
	mk_i2cdev(IX2_I2C_MUX6_BUS7, "24c04", 0x50, &port24_50_at24),

	/* devices on pca9548 4 */
	mk_i2cdev(IX2_I2C_MUX7_BUS0, "24c04", 0x50, &port25_50_at24),
	mk_i2cdev(IX2_I2C_MUX7_BUS1, "24c04", 0x50, &port26_50_at24),
	mk_i2cdev(IX2_I2C_MUX7_BUS2, "24c04", 0x50, &port27_50_at24),
	mk_i2cdev(IX2_I2C_MUX7_BUS3, "24c04", 0x50, &port28_50_at24),
	mk_i2cdev(IX2_I2C_MUX7_BUS4, "24c04", 0x50, &port29_50_at24),
	mk_i2cdev(IX2_I2C_MUX7_BUS5, "24c04", 0x50, &port30_50_at24),
	mk_i2cdev(IX2_I2C_MUX7_BUS6, "24c04", 0x50, &port31_50_at24),
	mk_i2cdev(IX2_I2C_MUX7_BUS7, "24c04", 0x50, &port32_50_at24),

	/* devices on pca9548 5 */
	mk_i2cdev(IX2_I2C_MUX8_BUS0, "24c04", 0x50, &port33_50_at24),
	mk_i2cdev(IX2_I2C_MUX8_BUS1, "24c04", 0x50, &port34_50_at24),
	mk_i2cdev(IX2_I2C_MUX8_BUS2, "24c04", 0x50, &port35_50_at24),
	mk_i2cdev(IX2_I2C_MUX8_BUS3, "24c04", 0x50, &port36_50_at24),
	mk_i2cdev(IX2_I2C_MUX8_BUS4, "24c04", 0x50, &port37_50_at24),
	mk_i2cdev(IX2_I2C_MUX8_BUS5, "24c04", 0x50, &port38_50_at24),
	mk_i2cdev(IX2_I2C_MUX8_BUS6, "24c04", 0x50, &port39_50_at24),
	mk_i2cdev(IX2_I2C_MUX8_BUS7, "24c04", 0x50, &port40_50_at24),

	/* devices on pca9548 6 */
	mk_i2cdev(IX2_I2C_MUX9_BUS0, "24c04", 0x50, &port41_50_at24),
	mk_i2cdev(IX2_I2C_MUX9_BUS1, "24c04", 0x50, &port42_50_at24),
	mk_i2cdev(IX2_I2C_MUX9_BUS2, "24c04", 0x50, &port43_50_at24),
	mk_i2cdev(IX2_I2C_MUX9_BUS3, "24c04", 0x50, &port44_50_at24),
	mk_i2cdev(IX2_I2C_MUX9_BUS4, "24c04", 0x50, &port45_50_at24),
	mk_i2cdev(IX2_I2C_MUX9_BUS5, "24c04", 0x50, &port46_50_at24),
	mk_i2cdev(IX2_I2C_MUX9_BUS6, "24c04", 0x50, &port47_50_at24),
	mk_i2cdev(IX2_I2C_MUX9_BUS7, "24c04", 0x50, &port48_50_at24),

	/* devices on pca9548 7 */
	mk_i2cdev(IX2_I2C_MUX10_BUS0, "sff8436", 0x50, &port49_50_sff8436),
	mk_i2cdev(IX2_I2C_MUX10_BUS1, "sff8436", 0x50, &port50_50_sff8436),
	mk_i2cdev(IX2_I2C_MUX10_BUS2, "sff8436", 0x50, &port51_50_sff8436),
	mk_i2cdev(IX2_I2C_MUX10_BUS3, "sff8436", 0x50, &port52_50_sff8436),
	mk_i2cdev(IX2_I2C_MUX10_BUS4, "sff8436", 0x50, &port53_50_sff8436),
	mk_i2cdev(IX2_I2C_MUX10_BUS5, "sff8436", 0x50, &port54_50_sff8436),
	mk_i2cdev(IX2_I2C_MUX10_BUS6, "sff8436", 0x50, &port55_50_sff8436),
	mk_i2cdev(IX2_I2C_MUX10_BUS7, "sff8436", 0x50, &port56_50_sff8436),

	mk_i2cdev(IX2_I2C_MUX11_BUS, "CY8C3245R1", 0x4e, NULL),
	mk_i2cdev(IX2_I2C_MUX11_BUS, "ix_rangeley_cpld", 0x38, &cpld6),

};

/*
 * Utility functions for I2C
 */

static struct i2c_adapter *get_adapter(int bus)
{
	int bail;
	struct i2c_adapter *adapter;

	for (bail = 20; --bail >= 0;) {
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

	for (i = 0; i < IX2_I2C_MUX1_BUS0; i++) {
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
		pr_err("could not get I2C adapter %d\n", bus);
		client = ERR_PTR(-ENODEV);
		goto exit;
	}

	if (!device_for_each_child(&adapter->dev, plat_info, check_i2c_match)) {
		client = i2c_new_device(adapter, board_info);
		if (!client) {
			pr_err(DRIVER_NAME "could not add I2C device at bus"
			                   "  %d type %s addr %#x\n",
					   bus, board_info->type,
					   board_info->addr);
			client = ERR_PTR(-ENODEV);
			goto exit;
		}
	} else {
		client = plat_info->client;
	}
	i2c_put_adapter(adapter);
exit:
	return client;
}

static int init_i2c_devices(void)
{
	int i801_bus = -1;
	int i;
	int ret;

	i801_bus = get_bus_by_name(SMB_I801_NAME);
	if (i801_bus < 0) {
		pr_err(DRIVER_NAME "could not find %s adapter bus\n",
				SMB_I801_NAME);
		ret = -ENODEV;
		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == IX2_I2C_I801_BUS)
			bus = i801_bus;

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

	for (i = 0; i < num_pins; i++) {
		names[pins[i].num] = pins[i].name;
	}
}

static int init_gpio_pins(struct gpio_pin *pins,
			  int num_pins, int gpio_base)
{
	int i;
	int ret;

	for (i = 0; i < num_pins; i++, pins++) {

	ret = gpio_request_one(gpio_base + pins->num, pins->flags, pins->name);
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

	return 0;

err_exit:
	while (i--) {
		gpio_free(gpio_base + (--pins)->num);
	}

	return ret;
}

static void free_gpio_pins(struct gpio_pin *pins,
			   int num_pins, int gpio_base)
{
	while (num_pins--) {
		gpio_free(gpio_base + (pins++)->num);
	}
}

static void gpio_free_all(void)
{
	free_gpio_pins(gpio_21_pins, ARRAY_SIZE(gpio_21_pins), IX2_GPIO_21_2_BASE);
	free_gpio_pins(gpio_26_2_pins, ARRAY_SIZE(gpio_26_2_pins),
			IX2_GPIO_26_2_BASE);
	free_gpio_pins(gpio_25_pins, ARRAY_SIZE(gpio_25_pins), IX2_GPIO_25_BASE);
	free_gpio_pins(gpio_23_pins, ARRAY_SIZE(gpio_23_pins), IX2_GPIO_23_BASE);
	free_gpio_pins(gpio_26_pins, ARRAY_SIZE(gpio_26_pins), IX2_GPIO_26_BASE);
	free_gpio_pins(cpu_gpio_22_pins, ARRAY_SIZE(cpu_gpio_22_pins),
			IX2_GPIO_22_BASE);
	free_gpio_pins(cpu_gpio_21_pins, ARRAY_SIZE(cpu_gpio_21_pins),
			IX2_GPIO_21_BASE);
	free_gpio_pins(cpu_gpio_20_pins, ARRAY_SIZE(cpu_gpio_20_pins),
			IX2_GPIO_20_BASE);
}
/*
 * Module init and exit for all I2C devices, including GPIO
 */

static int ix2_i2c_init(void)
{
	int ret;

	pr_debug(DRIVER_NAME " ix2 I2C init \n");
	init_gpio_platform_data(cpu_gpio_20_pins, ARRAY_SIZE(cpu_gpio_20_pins),
				&cpu_gpio_20_platform_data);
	init_gpio_platform_data(cpu_gpio_21_pins, ARRAY_SIZE(cpu_gpio_21_pins),
				&cpu_gpio_21_platform_data);
	init_gpio_platform_data(cpu_gpio_22_pins, ARRAY_SIZE(cpu_gpio_22_pins),
				&cpu_gpio_22_platform_data);
	init_gpio_platform_data(gpio_26_pins, ARRAY_SIZE(gpio_26_pins),
				&gpio_26_platform_data);
	init_gpio_platform_data(gpio_23_pins, ARRAY_SIZE(gpio_23_pins),
				&gpio_23_platform_data);
	init_gpio_platform_data(gpio_25_pins, ARRAY_SIZE(gpio_25_pins),
				&gpio_25_platform_data);
	init_gpio_platform_data(gpio_26_2_pins, ARRAY_SIZE(gpio_26_2_pins),
				&gpio_26_2_platform_data);
	init_gpio_platform_data(gpio_21_pins, ARRAY_SIZE(gpio_21_pins),
				&gpio_21_platform_data);

	ret = init_i2c_devices();
	if (ret)
		goto err_exit;

	pr_debug(DRIVER_NAME " ix2 I2C init succeeded \n");
	return 0;

err_exit:
	pr_debug(DRIVER_NAME "ix2 free i2c devices \n");
	free_i2c_devices();
	return ret;
}

static void ix2_i2c_exit(void)
{
	free_i2c_devices();
}

static void ix2_gpio_exit(void)
{
	gpio_free_all();
}

static int ix2_gpio_init(void)
{
	int ret;

	pr_debug(DRIVER_NAME " ix2 GPIO init \n");
	ret = init_gpio_pins(cpu_gpio_20_pins, ARRAY_SIZE(cpu_gpio_20_pins),
				IX2_GPIO_20_BASE);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(cpu_gpio_21_pins, ARRAY_SIZE(cpu_gpio_21_pins),
				IX2_GPIO_21_BASE);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(cpu_gpio_22_pins, ARRAY_SIZE(cpu_gpio_22_pins),
				IX2_GPIO_22_BASE);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(gpio_26_pins, ARRAY_SIZE(gpio_26_pins),
				IX2_GPIO_26_BASE);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(gpio_23_pins, ARRAY_SIZE(gpio_23_pins),
				IX2_GPIO_23_BASE);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(gpio_25_pins, ARRAY_SIZE(gpio_25_pins),
				IX2_GPIO_25_BASE);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(gpio_26_2_pins, ARRAY_SIZE(gpio_26_2_pins),
				IX2_GPIO_26_2_BASE);
	if (ret)
		goto err_exit;
	ret = init_gpio_pins(gpio_21_pins, ARRAY_SIZE(gpio_21_pins),
				IX2_GPIO_21_2_BASE);
	if (ret)
		goto err_exit;

	pr_debug(DRIVER_NAME "Succeeeded in gpio init \n");
	return 0;

err_exit:
	pr_err(DRIVER_NAME "gpio init failed freeup and BAIL\n");
	gpio_free_all();
	return ret;
}


static int ix2_platform_probe(struct platform_device *dev)
{
	int ret;

	dev_err(&dev->dev, "ix2 probe begin \n");

	ret = ix2_i2c_init();
	if (ret) {
		pr_err("I2C initialization failed\n");
		return ret;
	}

	ret = ix2_gpio_init();
	if (ret) {
		if (ret != -EPROBE_DEFER) {
			ix2_i2c_exit();
			dev_err(&dev->dev, "GPIO initialization failed (%d)\n",
					ret);
		} else {
			dev_info(&dev->dev, "GPIO initialization deferred\n");
		}

		return ret;
	}

	dev_err(&dev->dev, "ix2 probe succeeded \n");
	return 0;
}

static int ix2_platform_remove(struct platform_device *dev)
{
	ix2_gpio_exit();
	ix2_i2c_exit();
	return 0;
}

static const struct platform_device_id ix2_platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, ix2_platform_id);

static struct platform_driver ix2_platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = ix2_platform_probe,
	.remove = ix2_platform_remove,
};

static struct platform_device *plat_device = NULL;

/*
 * Module init and exit
 */

static int __init ix2_platform_init(void)
{
	int ret;

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" initializing\n");

	if (!driver_find(ix2_platform_driver.driver.name,
			 &platform_bus_type)) {
		ret = platform_driver_register(&ix2_platform_driver);
		if (ret) {
			pr_err(DRIVER_NAME ": %s driver registration failed."
				"(%d)\n", ix2_platform_driver.driver.name, ret);
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
			platform_driver_unregister(&ix2_platform_driver);
			return ret;
		}
	}
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
			" successfully loaded \n");
	return 0;
}

static void __exit ix2_platform_exit(void)
{
	platform_device_unregister(plat_device);
	plat_device =NULL;
	platform_driver_unregister(&ix2_platform_driver);
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " driver unloaded \n");
}

module_init(ix2_platform_init);
module_exit(ix2_platform_exit);

MODULE_AUTHOR("Vidya Sagar Ravipati (vidya@cumulusnetworks.com)");
MODULE_DESCRIPTION("Quanta IX2 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
