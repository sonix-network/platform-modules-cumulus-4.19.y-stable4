/*
 * quanta_ly8_platform.c - Quanta LY8 Platform Support
 *
 * Copyright (C) 2014, 2020 Cumulus Networks, Inc.
 * Author: Puneet Shenoy (puneet@cumulusnetworks.com)
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_device.h>
#include <linux/platform_data/sff-8436.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "quanta-utils.h"

#define DRIVER_NAME	"quanta_ly8_platform"
#define DRIVER_VERSION	"1.1"

#define LY8_I2C_I801_BUS 0
#define LY8_PORT_COUNT 54

static struct platform_driver ly8_platform_driver;

mk_eeprom(spd1,  52, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(cpu, 53, 256, AT24_FLAG_IRUGO);
mk_eeprom(board, 54, 256, AT24_FLAG_IRUGO);

struct ly8_device_info {
	int bus;
	struct i2c_board_info info;
	int has_port;  /* 1 for SFP, 2 for QSFP */
	int port_base;
	int port_bus;
	int num_ports;
};

struct port_info {
	int type; /* 0 = SFP , 1 = QSFP */
	struct i2c_board_info *b;
	struct i2c_client *c;
};
static struct port_info ports_info[LY8_PORT_COUNT];

/*  I2C Muxes  */
#define ly8_pca9546(addr, busno)					\
	enum {								\
		LY8_PCA9546_##addr##_0 = busno,				\
		LY8_PCA9546_##addr##_1,					\
		LY8_PCA9546_##addr##_2,					\
		LY8_PCA9546_##addr##_3,					\
	};								\
	static struct pca954x_platform_mode ly8_mode_pca9546_##addr [] = { \
		{ .adap_id = LY8_PCA9546_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = LY8_PCA9546_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = LY8_PCA9546_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = LY8_PCA9546_##addr##_3, .deselect_on_exit = 1,},			\
	};								\
	static struct pca954x_platform_data ly8_data_pca9546_##addr = { \
		.modes = ly8_mode_pca9546_##addr,			\
		.num_modes = ARRAY_SIZE(ly8_mode_pca9546_##addr),	\
	};


#define ly8_pca9548(addr, busno)					\
	enum {								\
		LY8_PCA9548_##addr##_0 = busno,				\
		LY8_PCA9548_##addr##_1,					\
		LY8_PCA9548_##addr##_2,					\
		LY8_PCA9548_##addr##_3,					\
		LY8_PCA9548_##addr##_4,					\
		LY8_PCA9548_##addr##_5,					\
		LY8_PCA9548_##addr##_6,					\
		LY8_PCA9548_##addr##_7,					\
	};								\
	static struct pca954x_platform_mode ly8_mode_pca9548_##addr [] = { \
		{ .adap_id = LY8_PCA9548_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = LY8_PCA9548_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = LY8_PCA9548_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = LY8_PCA9548_##addr##_3, .deselect_on_exit = 1,}, \
		{ .adap_id = LY8_PCA9548_##addr##_4, .deselect_on_exit = 1,}, \
		{ .adap_id = LY8_PCA9548_##addr##_5, .deselect_on_exit = 1,}, \
		{ .adap_id = LY8_PCA9548_##addr##_6, .deselect_on_exit = 1,}, \
		{ .adap_id = LY8_PCA9548_##addr##_7, .deselect_on_exit = 1,}, \
	};								\
	static struct pca954x_platform_data ly8_data_pca9548_##addr = { \
		.modes = ly8_mode_pca9548_##addr,			\
		.num_modes = ARRAY_SIZE(ly8_mode_pca9548_##addr),	\
	};

/* I2C I801 -> PCA9546(0x71) */
ly8_pca9546(71, 10);
/* I2C I801 -> PCA9546_1(0x72) */
ly8_pca9546(72, 14)
/* I2C I801 -> PCA9548_1(0x77) */
ly8_pca9548(77, 18)
/* I2C I801 -> PCA9548_1_1(0x77) -> PCA9548_1_2 (0x73)*/
ly8_pca9548(1_73, 26)
/* I2C I801 -> PCA9548_1_1(0x77) -> PCA9548_1_2 (0x74)*/
ly8_pca9548(1_74, 34)
/* I2C I801 -> PCA9548_1_1(0x77) -> PCA9548_1_2 (0x76)*/
ly8_pca9548(1_76, 42)
/* I2C I801 -> PCA9548_2_1(0x77) -> PCA9548_2_2 (0x73)*/
ly8_pca9548(2_73, 50)
/* I2C I801 -> PCA9548_2_1(0x77) -> PCA9548_2_2 (0x74)*/
ly8_pca9548(2_74, 58)
/* I2C I801 -> PCA9548_2_1(0x77) -> PCA9548_2_2 (0x75)*/
ly8_pca9548(2_75, 66)
/* I2C I801 -> PCA9548_2_1(0x77) -> CH5 -> PCA9548_8 (0x76)*/
ly8_pca9548(2_76, 74)
/* I2C I801 -> PCA9546_1(0x72)  -> PCA9546(0x76)*/
ly8_pca9546(76, 82)

/* GPIOs */
#define LY8_GPIO_BASE 100
#define LY8_GPIO_24_BASE LY8_GPIO_BASE
#define LY8_GPIO_23_BASE (LY8_GPIO_24_BASE + PCA_9555_GPIO_COUNT)
#define LY8_GPIO_26_BASE (LY8_GPIO_23_BASE + PCA_9555_GPIO_COUNT)

#define LY8_GPIO_23_2_BASE (LY8_GPIO_26_BASE + PCA_9555_GPIO_COUNT)
#define LY8_GPIO_24_2_BASE (LY8_GPIO_23_2_BASE + PCA_9555_GPIO_COUNT)
#define LY8_GPIO_23_3_BASE (LY8_GPIO_24_2_BASE + PCA_9555_GPIO_COUNT)
#define LY8_GPIO_20_BASE (LY8_GPIO_23_3_BASE + PCA_9555_GPIO_COUNT)
#define LY8_GPIO_SFP_1_8_BASE (LY8_GPIO_20_BASE + PCA_9555_GPIO_COUNT)
#define LY8_GPIO_SFP_9_16_BASE (LY8_GPIO_SFP_1_8_BASE + PCA_9698_GPIO_COUNT)
#define LY8_GPIO_SFP_17_24_BASE (LY8_GPIO_SFP_9_16_BASE + PCA_9698_GPIO_COUNT)
#define LY8_GPIO_SFP_25_32_BASE (LY8_GPIO_SFP_17_24_BASE + PCA_9698_GPIO_COUNT)
#define LY8_GPIO_SFP_33_40_BASE (LY8_GPIO_SFP_25_32_BASE + PCA_9698_GPIO_COUNT)
#define LY8_GPIO_SFP_41_48_BASE (LY8_GPIO_SFP_33_40_BASE + PCA_9698_GPIO_COUNT)

mk_gpio_pins(gpio_24) = {
	mk_gpio_pin(0,    qsfp_pwr_enable,    GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(1,    9555_5_int,         GPIOF_DIR_IN),
	mk_gpio_pin(2,    9555_4_int,         GPIOF_DIR_IN),
	mk_gpio_pin(3,    bcm5461s_int,       GPIOF_DIR_IN),
	mk_gpio_pin(4,    fan1_present,       GPIOF_DIR_IN),
	mk_gpio_pin(5,    fan2_present,       GPIOF_DIR_IN),
	mk_gpio_pin(6,    fan3_present,       GPIOF_DIR_IN),
	mk_gpio_pin(7,    m21441_rst,         GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,    fan1_f2b,           GPIOF_DIR_IN),
	mk_gpio_pin(9,    fan2_f2b,           GPIOF_DIR_IN),
	mk_gpio_pin(10,   fan3_f2b,           GPIOF_DIR_IN),
	mk_gpio_pin(11,   nc,                 GPIOF_DIR_IN),
	mk_gpio_pin(12,   fan1_led,           GPIOF_DIR_IN),
	mk_gpio_pin(13,   fan2_led,           GPIOF_DIR_IN),
	mk_gpio_pin(14,   fan3_led,           GPIOF_DIR_IN),
	mk_gpio_pin(15,   qsfp_int,           GPIOF_DIR_IN),
};

mk_gpio_pins(gpio_23) = {
	mk_gpio_pin(0,    fan0_board,        GPIOF_DIR_IN),
	mk_gpio_pin(1,    fan1_board,        GPIOF_DIR_IN),
	mk_gpio_pin(2,    fan2_board,        GPIOF_DIR_IN),
	mk_gpio_pin(3,    fan3_board,        GPIOF_DIR_IN),
	mk_gpio_pin(4,    power_good,        GPIOF_DIR_IN),
	mk_gpio_pin(5,    sfp_power_enable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(6,    qsfp_present,      GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,    usb_reset,         GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,    pca9554_int,       GPIOF_DIR_IN),
	mk_gpio_pin(9,    mgmt_present,      GPIOF_DIR_IN),
	mk_gpio_pin(10,   board_id_0,        GPIOF_DIR_IN),
	mk_gpio_pin(11,   board_id_1,        GPIOF_DIR_IN),
	mk_gpio_pin(12,   board_id_2,        GPIOF_DIR_IN),
	mk_gpio_pin(13,   board_id_3,        GPIOF_DIR_IN),
	mk_gpio_pin(14,   board_id_4,        GPIOF_DIR_IN),
	mk_gpio_pin(15,   board_id_5,        GPIOF_DIR_IN),
};

mk_gpio_pins(gpio_26) = {
	mk_gpio_pin(0,    psu_pwr1_present, GPIOF_DIR_IN),
	mk_gpio_pin(1,    psu_pwr1_dc_ok,   GPIOF_DIR_IN),
	mk_gpio_pin(2,    psu_pwr1_int,     GPIOF_DIR_IN),
	mk_gpio_pin(3,    psu_pwr2_present, GPIOF_DIR_IN),
	mk_gpio_pin(4,    psu_pwr2_dc_ok,   GPIOF_DIR_IN),
	mk_gpio_pin(5,    psu_pwr2_int,     GPIOF_DIR_IN),
	mk_gpio_pin(6,    psu_pwr1_ac_ok,   GPIOF_DIR_IN),
	mk_gpio_pin(7,    psu_pwr2_ac_ok,   GPIOF_DIR_IN),
	mk_gpio_pin(8,    psu_pwr1_reset,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,    psu_pwr2_reset,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10,   psu1_green_led,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,   psu1_red_led,     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12,   psu2_green_led,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(13,   psu2_red_led,     GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14,   fan_green_led,    GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(15,   fan_red_led,      GPIOF_OUT_INIT_LOW),
};

mk_gpio_pins(gpio_23_2) = {
	mk_gpio_pin(0,    qsfp49_reset,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(1,    qsfp49_intr,    GPIOF_DIR_IN),
	mk_gpio_pin(2,    qsfp49_present, GPIOF_DIR_IN),
	mk_gpio_pin(3,    qsfp49_lpmode,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(4,    qsfp50_reset,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(5,    qsfp50_intr,    GPIOF_DIR_IN),
	mk_gpio_pin(6,    qsfp50_present, GPIOF_DIR_IN),
	mk_gpio_pin(7,    qsfp50_lpmode,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,    qsfp51_reset,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(9,    qsfp51_intr,    GPIOF_DIR_IN),
	mk_gpio_pin(10,   qsfp51_present, GPIOF_DIR_IN),
	mk_gpio_pin(11,   qsfp51_lpmode,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(12,   qsfp52_reset,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(13,   qsfp52_intr,    GPIOF_DIR_IN),
	mk_gpio_pin(14,   qsfp52_present, GPIOF_DIR_IN),
	mk_gpio_pin(15,   qsfp52_lpmode,  GPIOF_OUT_INIT_HIGH),
};

mk_gpio_pins(gpio_24_2) = {
	mk_gpio_pin(0,    port49_40_led,  GPIOF_DIR_IN),
	mk_gpio_pin(1,    port49_10_led,  GPIOF_DIR_IN),
	mk_gpio_pin(2,    port50_40_led,  GPIOF_DIR_IN),
	mk_gpio_pin(3,    port50_10_led,  GPIOF_DIR_IN),
	mk_gpio_pin(4,    port51_40_led,  GPIOF_DIR_IN),
	mk_gpio_pin(5,    port51_10_led,  GPIOF_DIR_IN),
	mk_gpio_pin(6,    port52_40_led,  GPIOF_DIR_IN),
	mk_gpio_pin(7,    port52_10_led,  GPIOF_DIR_IN),
	mk_gpio_pin(8,    nc_1,           GPIOF_DIR_IN),
	mk_gpio_pin(9,    nc_2,           GPIOF_DIR_IN),
	mk_gpio_pin(10,   nc_3,           GPIOF_DIR_IN),
	mk_gpio_pin(11,   nc_4,           GPIOF_DIR_IN),
	mk_gpio_pin(12,   nc_5,           GPIOF_DIR_IN),
	mk_gpio_pin(13,   nc_6,           GPIOF_DIR_IN),
	mk_gpio_pin(14,   nc_7,           GPIOF_DIR_IN),
	mk_gpio_pin(15,   nc_8,           GPIOF_DIR_IN),
};
mk_gpio_pins(gpio_23_3) = {
	mk_gpio_pin(0,    qsfp53_reset,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(1,    qsfp53_intr,        GPIOF_DIR_IN),
	mk_gpio_pin(2,    qsfp53_present,     GPIOF_DIR_IN),
	mk_gpio_pin(3,    qsfp53_lpmode,      GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(4,    qsfp54_reset,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(5,    qsfp54_intr,        GPIOF_DIR_IN),
	mk_gpio_pin(6,    qsfp54_present,     GPIOF_DIR_IN),
	mk_gpio_pin(7,    qsfp54_lpmode,      GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,    qsfp_power_enable,  GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,    sys_reset,          GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(10,   9546_reset,         GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,   bcm84328_reset,     GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(12,   nc_9,               GPIOF_DIR_IN),
	mk_gpio_pin(13,   qdb_id_0,           GPIOF_DIR_IN),
	mk_gpio_pin(14,   qdb_id_1,           GPIOF_DIR_IN),
	mk_gpio_pin(15,   qdb_id_2,           GPIOF_OUT_INIT_HIGH),
};

mk_gpio_pins(gpio_20) = {
	mk_gpio_pin(0,    mb_i2c_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(1,    mb_led_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(2,    mb_eth_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(3,    mb_sw_rst,        GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(4,    mb_odd_phy_rst,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(5,    mb_even_phy_rst,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(6,    alta_rst,         GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,    usb_rst,          GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,    alta_power,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(9,    i2c_mac_switch,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(10,   boot_led,         GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11,   sys_led,          GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(12,   board_pwr_led,    GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(13,   nc_10,            GPIOF_DIR_IN),
	mk_gpio_pin(14,   nc_11,            GPIOF_DIR_IN),
	mk_gpio_pin(15,   nc_12,            GPIOF_DIR_IN),
};
mk_gpio_pins(gpio_sfp_1_8) = {
	mk_gpio_pin(0,    sfp1_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(1,    sfp1_los,         GPIOF_DIR_IN),
	mk_gpio_pin(2,    sfp1_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(3,    sfp1_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(4,    sfp2_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(5,    sfp2_los,         GPIOF_DIR_IN),
	mk_gpio_pin(6,    sfp2_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,    sfp2_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(8,    sfp3_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(9,    sfp3_los,         GPIOF_DIR_IN),
	mk_gpio_pin(10,   sfp3_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,   sfp3_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(12,   sfp4_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(13,   sfp4_los,         GPIOF_DIR_IN),
	mk_gpio_pin(14,   sfp4_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(15,   sfp4_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(16,   sfp5_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(17,   sfp5_los,         GPIOF_DIR_IN),
	mk_gpio_pin(18,   sfp5_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(19,   sfp5_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(20,   sfp6_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(21,   sfp6_los,         GPIOF_DIR_IN),
	mk_gpio_pin(22,   sfp6_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(23,   sfp6_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(24,   sfp7_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(25,   sfp7_los,         GPIOF_DIR_IN),
	mk_gpio_pin(26,   sfp7_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(27,   sfp7_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(28,   sfp8_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(29,   sfp8_los,         GPIOF_DIR_IN),
	mk_gpio_pin(30,   sfp8_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(31,   sfp8_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(32,   nc_sfp_1_0,       GPIOF_DIR_IN),
	mk_gpio_pin(33,   nc_sfp_1_1,       GPIOF_DIR_IN),
	mk_gpio_pin(34,   nc_sfp_1_2,       GPIOF_DIR_IN),
	mk_gpio_pin(35,   nc_sfp_1_3,       GPIOF_DIR_IN),
	mk_gpio_pin(36,   nc_sfp_1_4,       GPIOF_DIR_IN),
	mk_gpio_pin(37,   nc_sfp_1_5,       GPIOF_DIR_IN),
	mk_gpio_pin(38,   nc_sfp_1_6,       GPIOF_DIR_IN),
	mk_gpio_pin(39,   nc_sfp_1_7,       GPIOF_DIR_IN),
};

mk_gpio_pins(gpio_sfp_9_16) = {
	mk_gpio_pin(0,    sfp9_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(1,    sfp9_los,         GPIOF_DIR_IN),
	mk_gpio_pin(2,    sfp9_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(3,    sfp9_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(4,    sfp10_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(5,    sfp10_los,         GPIOF_DIR_IN),
	mk_gpio_pin(6,    sfp10_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,    sfp10_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(8,    sfp11_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(9,    sfp11_los,         GPIOF_DIR_IN),
	mk_gpio_pin(10,   sfp11_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,   sfp11_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(12,   sfp12_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(13,   sfp12_los,         GPIOF_DIR_IN),
	mk_gpio_pin(14,   sfp12_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(15,   sfp12_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(16,   sfp13_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(17,   sfp13_los,         GPIOF_DIR_IN),
	mk_gpio_pin(18,   sfp13_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(19,   sfp13_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(20,   sfp14_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(21,   sfp14_los,         GPIOF_DIR_IN),
	mk_gpio_pin(22,   sfp14_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(23,   sfp14_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(24,   sfp15_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(25,   sfp15_los,         GPIOF_DIR_IN),
	mk_gpio_pin(26,   sfp15_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(27,   sfp15_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(28,   sfp16_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(29,   sfp16_los,         GPIOF_DIR_IN),
	mk_gpio_pin(30,   sfp16_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(31,   sfp16_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(32,   nc_sfp_9_0,        GPIOF_DIR_IN),
	mk_gpio_pin(33,   nc_sfp_9_1,        GPIOF_DIR_IN),
	mk_gpio_pin(34,   nc_sfp_9_2,        GPIOF_DIR_IN),
	mk_gpio_pin(35,   nc_sfp_9_3,        GPIOF_DIR_IN),
	mk_gpio_pin(36,   nc_sfp_9_4,        GPIOF_DIR_IN),
	mk_gpio_pin(37,   nc_sfp_9_5,        GPIOF_DIR_IN),
	mk_gpio_pin(38,   nc_sfp_9_6,        GPIOF_DIR_IN),
	mk_gpio_pin(39,   nc_sfp_9_7,        GPIOF_DIR_IN),
};

mk_gpio_pins(gpio_sfp_17_24) = {
	mk_gpio_pin(0,    sfp17_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(1,    sfp17_los,         GPIOF_DIR_IN),
	mk_gpio_pin(2,    sfp17_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(3,    sfp17_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(4,    sfp18_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(5,    sfp18_los,         GPIOF_DIR_IN),
	mk_gpio_pin(6,    sfp18_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,    sfp18_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(8,    sfp19_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(9,    sfp19_los,         GPIOF_DIR_IN),
	mk_gpio_pin(10,   sfp19_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,   sfp19_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(12,   sfp20_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(13,   sfp20_los,         GPIOF_DIR_IN),
	mk_gpio_pin(14,   sfp20_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(15,   sfp20_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(16,   sfp21_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(17,   sfp21_los,         GPIOF_DIR_IN),
	mk_gpio_pin(18,   sfp21_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(19,   sfp21_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(20,   sfp22_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(21,   sfp22_los,         GPIOF_DIR_IN),
	mk_gpio_pin(22,   sfp22_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(23,   sfp22_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(24,   sfp23_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(25,   sfp23_los,         GPIOF_DIR_IN),
	mk_gpio_pin(26,   sfp23_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(27,   sfp23_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(28,   sfp24_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(29,   sfp24_los,         GPIOF_DIR_IN),
	mk_gpio_pin(30,   sfp24_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(31,   sfp24_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(32,   nc_sfp_17_0,       GPIOF_DIR_IN),
	mk_gpio_pin(33,   nc_sfp_17_1,       GPIOF_DIR_IN),
	mk_gpio_pin(34,   nc_sfp_17_2,       GPIOF_DIR_IN),
	mk_gpio_pin(35,   nc_sfp_17_3,       GPIOF_DIR_IN),
	mk_gpio_pin(36,   nc_sfp_17_4,       GPIOF_DIR_IN),
	mk_gpio_pin(37,   nc_sfp_17_5,       GPIOF_DIR_IN),
	mk_gpio_pin(38,   nc_sfp_17_6,       GPIOF_DIR_IN),
	mk_gpio_pin(39,   nc_sfp_17_7,       GPIOF_DIR_IN),
};

mk_gpio_pins(gpio_sfp_25_32) = {
	mk_gpio_pin(0,    sfp25_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(1,    sfp25_los,         GPIOF_DIR_IN),
	mk_gpio_pin(2,    sfp25_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(3,    sfp25_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(4,    sfp26_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(5,    sfp26_los,         GPIOF_DIR_IN),
	mk_gpio_pin(6,    sfp26_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,    sfp26_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(8,    sfp27_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(9,    sfp27_los,         GPIOF_DIR_IN),
	mk_gpio_pin(10,   sfp27_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,   sfp27_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(12,   sfp28_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(13,   sfp28_los,         GPIOF_DIR_IN),
	mk_gpio_pin(14,   sfp28_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(15,   sfp28_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(16,   sfp29_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(17,   sfp29_los,         GPIOF_DIR_IN),
	mk_gpio_pin(18,   sfp29_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(19,   sfp29_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(20,   sfp30_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(21,   sfp30_los,         GPIOF_DIR_IN),
	mk_gpio_pin(22,   sfp30_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(23,   sfp30_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(24,   sfp31_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(25,   sfp31_los,         GPIOF_DIR_IN),
	mk_gpio_pin(26,   sfp31_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(27,   sfp31_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(28,   sfp32_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(29,   sfp32_los,         GPIOF_DIR_IN),
	mk_gpio_pin(30,   sfp32_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(31,   sfp32_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(32,   nc_sfp_25_0,       GPIOF_DIR_IN),
	mk_gpio_pin(33,   nc_sfp_25_1,       GPIOF_DIR_IN),
	mk_gpio_pin(34,   nc_sfp_25_2,       GPIOF_DIR_IN),
	mk_gpio_pin(35,   nc_sfp_25_3,       GPIOF_DIR_IN),
	mk_gpio_pin(36,   nc_sfp_25_4,       GPIOF_DIR_IN),
	mk_gpio_pin(37,   nc_sfp_25_5,       GPIOF_DIR_IN),
	mk_gpio_pin(38,   nc_sfp_25_6,       GPIOF_DIR_IN),
	mk_gpio_pin(39,   nc_sfp_25_7,       GPIOF_DIR_IN),
};
mk_gpio_pins(gpio_sfp_33_40) = {
	mk_gpio_pin(0,    sfp33_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(1,    sfp33_los,         GPIOF_DIR_IN),
	mk_gpio_pin(2,    sfp33_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(3,    sfp33_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(4,    sfp34_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(5,    sfp34_los,         GPIOF_DIR_IN),
	mk_gpio_pin(6,    sfp34_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,    sfp34_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(8,    sfp35_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(9,    sfp35_los,         GPIOF_DIR_IN),
	mk_gpio_pin(10,   sfp35_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,   sfp35_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(12,   sfp36_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(13,   sfp36_los,         GPIOF_DIR_IN),
	mk_gpio_pin(14,   sfp36_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(15,   sfp36_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(16,   sfp37_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(17,   sfp37_los,         GPIOF_DIR_IN),
	mk_gpio_pin(18,   sfp37_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(19,   sfp37_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(20,   sfp38_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(21,   sfp38_los,         GPIOF_DIR_IN),
	mk_gpio_pin(22,   sfp38_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(23,   sfp38_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(24,   sfp39_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(25,   sfp39_los,         GPIOF_DIR_IN),
	mk_gpio_pin(26,   sfp39_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(27,   sfp39_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(28,   sfp40_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(29,   sfp40_los,         GPIOF_DIR_IN),
	mk_gpio_pin(30,   sfp40_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(31,   sfp40_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(32,   nc_sfp_33_0,       GPIOF_DIR_IN),
	mk_gpio_pin(33,   nc_sfp_33_1,       GPIOF_DIR_IN),
	mk_gpio_pin(34,   nc_sfp_33_2,       GPIOF_DIR_IN),
	mk_gpio_pin(35,   nc_sfp_33_3,       GPIOF_DIR_IN),
	mk_gpio_pin(36,   nc_sfp_33_4,       GPIOF_DIR_IN),
	mk_gpio_pin(37,   nc_sfp_33_5,       GPIOF_DIR_IN),
	mk_gpio_pin(38,   nc_sfp_33_6,       GPIOF_DIR_IN),
	mk_gpio_pin(39,   nc_sfp_33_7,       GPIOF_DIR_IN),
};
mk_gpio_pins(gpio_sfp_41_48) = {
	mk_gpio_pin(0,    sfp41_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(1,    sfp41_los,         GPIOF_DIR_IN),
	mk_gpio_pin(2,    sfp41_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(3,    sfp41_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(4,    sfp42_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(5,    sfp42_los,         GPIOF_DIR_IN),
	mk_gpio_pin(6,    sfp42_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,    sfp42_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(8,    sfp43_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(9,    sfp43_los,         GPIOF_DIR_IN),
	mk_gpio_pin(10,   sfp43_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,   sfp43_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(12,   sfp44_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(13,   sfp44_los,         GPIOF_DIR_IN),
	mk_gpio_pin(14,   sfp44_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(15,   sfp44_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(16,   sfp45_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(17,   sfp45_los,         GPIOF_DIR_IN),
	mk_gpio_pin(18,   sfp45_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(19,   sfp45_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(20,   sfp46_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(21,   sfp46_los,         GPIOF_DIR_IN),
	mk_gpio_pin(22,   sfp46_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(23,   sfp46_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(24,   sfp47_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(25,   sfp47_los,         GPIOF_DIR_IN),
	mk_gpio_pin(26,   sfp47_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(27,   sfp47_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(28,   sfp48_present_l,   GPIOF_DIR_IN),
	mk_gpio_pin(29,   sfp48_los,         GPIOF_DIR_IN),
	mk_gpio_pin(30,   sfp48_tx_disable,  GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(31,   sfp48_tx_fault,    GPIOF_DIR_IN),
	mk_gpio_pin(32,   nc_sfp_41_0,       GPIOF_DIR_IN),
	mk_gpio_pin(33,   nc_sfp_41_1,       GPIOF_DIR_IN),
	mk_gpio_pin(34,   nc_sfp_41_2,       GPIOF_DIR_IN),
	mk_gpio_pin(35,   nc_sfp_41_3,       GPIOF_DIR_IN),
	mk_gpio_pin(36,   nc_sfp_41_4,       GPIOF_DIR_IN),
	mk_gpio_pin(37,   nc_sfp_41_5,       GPIOF_DIR_IN),
	mk_gpio_pin(38,   nc_sfp_41_6,       GPIOF_DIR_IN),
	mk_gpio_pin(39,   nc_sfp_41_7,       GPIOF_DIR_IN),
};

mk_gpio_platform_data(gpio_24,     LY8_GPIO_24_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_24_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_23,     LY8_GPIO_23_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_23_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_26,     LY8_GPIO_26_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_26_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_23_2,   LY8_GPIO_23_2_BASE, PCA_9555_GPIO_COUNT,
		      &gpio_23_2_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_24_2,   LY8_GPIO_24_2_BASE, PCA_9555_GPIO_COUNT,
		      &gpio_24_2_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_23_3,   LY8_GPIO_23_3_BASE, PCA_9555_GPIO_COUNT,
		      &gpio_23_3_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_20,     LY8_GPIO_20_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_20_pins, init_gpio_pins, free_gpio_pins);

mk_gpio_platform_data(gpio_sfp_1_8,    LY8_GPIO_SFP_1_8_BASE,
		      PCA_9698_GPIO_COUNT,
		      &gpio_sfp_1_8_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_sfp_9_16,   LY8_GPIO_SFP_9_16_BASE,
		      PCA_9698_GPIO_COUNT,
		      &gpio_sfp_9_16_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_sfp_17_24,  LY8_GPIO_SFP_17_24_BASE,
		      PCA_9698_GPIO_COUNT,
		      &gpio_sfp_17_24_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_sfp_25_32,  LY8_GPIO_SFP_25_32_BASE,
		      PCA_9698_GPIO_COUNT,
		      &gpio_sfp_25_32_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_sfp_33_40,  LY8_GPIO_SFP_33_40_BASE,
		      PCA_9698_GPIO_COUNT,
		      &gpio_sfp_33_40_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_sfp_41_48,  LY8_GPIO_SFP_41_48_BASE,
		      PCA_9698_GPIO_COUNT,
		      &gpio_sfp_41_48_pins, init_gpio_pins, free_gpio_pins);

/* I2C Device Map */
static struct ly8_device_info i2c_devices_level1[] = {
	{
		.bus = LY8_I2C_I801_BUS,       /* SPD DIMM EEPROM */
		{I2C_BOARD_INFO("spd", 0x52),
		 .platform_data = &spd1_52_at24,},
	},
	{
		.bus = LY8_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9546", 0x71),
		 .platform_data = &ly8_data_pca9546_71,},
	},
	{
		.bus = LY8_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9546", 0x72),
		 .platform_data = &ly8_data_pca9546_72,},
	},
	{
		.bus = LY8_I2C_I801_BUS,     /* Sensors Chip */
		{I2C_BOARD_INFO("CY8C3245R1", 0x4e),},
	},
	{
		.bus = LY8_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9548", 0x77),
		 .platform_data = &ly8_data_pca9548_77,},
	},

};
static struct ly8_device_info i2c_devices_level3[] = {
	{
		.bus = LY8_PCA9548_2_76_4,
		{I2C_BOARD_INFO("dummy", 0x21),}, /* M21441 Chip for Port 53 */
	},
};
static struct ly8_device_info i2c_devices_level2[] = {
	/* Following devices are on CPU Board */
	{
		.bus = LY8_PCA9546_71_0,
		{I2C_BOARD_INFO("pca9555", 0x20),
		.platform_data = &gpio_20_platform_data},
	},
	{
		.bus = LY8_PCA9546_71_1,
		{I2C_BOARD_INFO("24c02", 0x53),
		 .platform_data = &cpu_53_at24,},
	},
	/* Following devices are on MB */
	{
		.bus = LY8_PCA9546_72_0, /* PSU1 PMBUS */
		{I2C_BOARD_INFO("ps2471", 0x6f),},
	},
	{
		.bus = LY8_PCA9546_72_1, /* PSU2 PMBUS */
		{I2C_BOARD_INFO("ps2471", 0x69),},
	},
	{
		.bus = LY8_PCA9546_72_2,
		{I2C_BOARD_INFO("pca9555", 0x26),
		.platform_data = &gpio_26_platform_data},
	},
	{
		.bus = LY8_PCA9546_72_3,   /* Board EEPROM  */
		{I2C_BOARD_INFO("24c02", 0x54),
		 .platform_data = &board_54_at24,},
	},
	{
		.bus = LY8_PCA9548_77_0,
		{I2C_BOARD_INFO("pca9555", 0x24),
		.platform_data = &gpio_24_platform_data},
	},
	{
		.bus = LY8_PCA9548_77_0,
		{I2C_BOARD_INFO("pca9555", 0x23),
		 .platform_data = &gpio_23_platform_data},
	},
	{
		.bus = LY8_PCA9548_77_1,
		{I2C_BOARD_INFO("pca9548", 0x73),
		 .platform_data = &ly8_data_pca9548_1_73,},
		 .has_port = 1, .port_base = 1,
		 .port_bus = LY8_PCA9548_1_73_0,
  		 .num_ports = 8,
	},
	{
		.bus = LY8_PCA9548_77_1,
		{I2C_BOARD_INFO("pca9548", 0x74),
		 .platform_data = &ly8_data_pca9548_1_74,},
		 .has_port = 1, .port_base = 9,
		 .port_bus = LY8_PCA9548_1_74_0,
  		 .num_ports = 8,
	},
	{
		.bus = LY8_PCA9548_77_1,
		{I2C_BOARD_INFO("pca9548", 0x76),
		 .platform_data = &ly8_data_pca9548_1_76,},
		 .has_port = 1, .port_base = 17,
		 .port_bus = LY8_PCA9548_1_76_0,
  		 .num_ports = 8,
	},
	{
		.bus = LY8_PCA9548_77_2,
		{I2C_BOARD_INFO("pca9548", 0x73),
		 .platform_data = &ly8_data_pca9548_2_73,},
		 .has_port = 1, .port_base = 25,
		 .port_bus = LY8_PCA9548_2_73_0,
  		 .num_ports = 8,
	},
	{
		.bus = LY8_PCA9548_77_2,
		{I2C_BOARD_INFO("pca9548", 0x74),
		 .platform_data = &ly8_data_pca9548_2_74,},
		 .has_port = 1, .port_base = 33,
		 .port_bus = LY8_PCA9548_2_74_0,
  		 .num_ports = 8,
	},
	{
		.bus = LY8_PCA9548_77_2,
		{I2C_BOARD_INFO("pca9548", 0x75),
		 .platform_data = &ly8_data_pca9548_2_75,},
		 .has_port = 1, .port_base = 41,
		 .port_bus = LY8_PCA9548_2_75_0,
  		 .num_ports = 8,
	},
	{
		.bus = LY8_PCA9548_77_3,
		{I2C_BOARD_INFO("pca9698", 0x23),
		 .platform_data = &gpio_sfp_1_8_platform_data},
	},
	{
		.bus = LY8_PCA9548_77_3,
		{I2C_BOARD_INFO("pca9698", 0x21),
		 .platform_data = &gpio_sfp_9_16_platform_data},
	},
	{
		.bus = LY8_PCA9548_77_3,
		{I2C_BOARD_INFO("pca9698", 0x22),
		 .platform_data = &gpio_sfp_17_24_platform_data},
	},
	{
		.bus = LY8_PCA9548_77_4,
		{I2C_BOARD_INFO("pca9698", 0x23),
		 .platform_data = &gpio_sfp_25_32_platform_data},
	},
	{
		.bus = LY8_PCA9548_77_4,
		{I2C_BOARD_INFO("pca9698", 0x24),
		 .platform_data = &gpio_sfp_33_40_platform_data},
	},
	{
		.bus = LY8_PCA9548_77_4,
		{I2C_BOARD_INFO("pca9698", 0x25),
		 .platform_data = &gpio_sfp_41_48_platform_data},
	},
	{
		.bus = LY8_PCA9548_77_5,
		{I2C_BOARD_INFO("pca9548", 0x76),
		 .platform_data = &ly8_data_pca9548_2_76,},
		 .has_port = 2, .port_base = 49,
		 .port_bus = LY8_PCA9548_2_76_0,
  		 .num_ports = 4,
	},
	{
		.bus = LY8_PCA9548_77_5,
		{I2C_BOARD_INFO("pca9555", 0x24),
		 .platform_data = &gpio_24_2_platform_data},
	},
	{
		.bus = LY8_PCA9548_77_6,
		{I2C_BOARD_INFO("pca9555", 0x23),
		 .platform_data = &gpio_23_2_platform_data},
	},
	{
		.bus = LY8_PCA9548_77_7,
		{I2C_BOARD_INFO("pca9555", 0x23),
		 .platform_data = &gpio_23_3_platform_data},
	},
	{
		.bus = LY8_PCA9548_77_7,
		{I2C_BOARD_INFO("pca9546", 0x76),
		 .platform_data = &ly8_data_pca9546_76,},
		 .has_port = 2, .port_base = 53,
  		 .num_ports = 2,
		 .port_bus = LY8_PCA9546_76_0,
	},

};

static struct i2c_client *i2c_clients_level1[ARRAY_SIZE(i2c_devices_level1)];
static struct i2c_client *i2c_clients_level2[ARRAY_SIZE(i2c_devices_level2)];
static struct i2c_client *i2c_clients_level3[ARRAY_SIZE(i2c_devices_level3)];

static int m21441_read(struct i2c_client *client, u8 reg)
{
	int value;

	value = i2c_smbus_read_byte_data(client, reg);
	if (value < 0) {
		dev_err(&client->dev, "Read to %d failed !\n", reg);
	}
	return value;
}

static int m21441_write(struct i2c_client *client, u8 reg, u8 value)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, reg, value);
	if (ret < 0) {
		dev_err(&client->dev,"Write to %d failed !\n", reg);
	}
	return ret;
}

static int m21441_init(struct i2c_client *client)
{
#define M21441_REG_PAGE_ADDRESS		0xFF
#define M21441_REG_MASTER_RESET		0xE0
#define M21441_REG_CHIP_ID              0xE1
#define M21441_REG_GEN_SW_CONFIG	0x01

#define M21441_REG_ASC_REG1_KR0	        0x10
#define M21441_REG_ASC_REG1_KR1	        0x11
#define M21441_REG_ASC_REG1_KR2	        0x12
#define M21441_REG_ASC_REG1_KR3	        0x13
#define M21441_REG_ASC_REG1_XFI0	0x14
#define M21441_REG_ASC_REG1_XFI1	0x15
#define M21441_REG_ASC_REG1_XFI2	0x16
#define M21441_REG_ASC_REG1_XFI3	0x17
#define M21441_REG_ASC_REG1_KR0r	0x18
#define M21441_REG_ASC_REG1_KR1r	0x19
#define M21441_REG_ASC_REG1_KR2r	0x1a
#define M21441_REG_ASC_REG1_KR3r	0x1b

#define M21441_REG_CONFIG_REG1_KR0	0x74
#define M21441_REG_CONFIG_REG1_KR1	0x79
#define M21441_REG_CONFIG_REG1_KR2	0x7e
#define M21441_REG_CONFIG_REG1_KR3	0x83
#define M21441_REG_CONFIG_REG1_XFI0	0x88
#define M21441_REG_CONFIG_REG1_XFI1	0x8d
#define M21441_REG_CONFIG_REG1_XFI2	0x92
#define M21441_REG_CONFIG_REG1_XFI3	0x97
#define M21441_REG_CONFIG_REG1_KR0r	0x9c
#define M21441_REG_CONFIG_REG1_KR1r	0xa1
#define M21441_REG_CONFIG_REG1_KR2r	0xa6
#define M21441_REG_CONFIG_REG1_KR3r	0xab

#define M21441_REG_CONFIG_REG3_XFI0	0x8a
#define M21441_REG_CONFIG_REG3_XFI1	0x8f
#define M21441_REG_CONFIG_REG3_XFI2	0x94
#define M21441_REG_CONFIG_REG3_XFI3	0x99

#define M21441_REG_CDR_REG1_KR0 	0x47
#define M21441_REG_CDR_REG1_KR1	        0x57
#define M21441_REG_CDR_REG1_KR2	        0x67
#define M21441_REG_CDR_REG1_KR3	        0x77
#define M21441_REG_CDR_REG1_XFI0	0x4b
#define M21441_REG_CDR_REG1_XFI1	0x5b
#define M21441_REG_CDR_REG1_XFI2	0x6b
#define M21441_REG_CDR_REG1_XFI3	0x7b
#define M21441_REG_CDR_REG1_KR0r	0x4f
#define M21441_REG_CDR_REG1_KR1r	0x5f
#define M21441_REG_CDR_REG1_KR2r	0x6f
#define M21441_REG_CDR_REG1_KR3r	0x7f

#define M21441_REG_PLL0_REG1            0x40
#define M21441_REG_PLL1_REG1            0x50
#define M21441_REG_PLL2_REG1            0x60
#define M21441_REG_PLL3_REG1            0x70

	static unsigned char out_signal_reg[12] = {
		M21441_REG_CONFIG_REG1_KR0,
		M21441_REG_CONFIG_REG1_KR1,
		M21441_REG_CONFIG_REG1_KR2,
		M21441_REG_CONFIG_REG1_KR3,
		M21441_REG_CONFIG_REG1_XFI0,
		M21441_REG_CONFIG_REG1_XFI1,
		M21441_REG_CONFIG_REG1_XFI2,
		M21441_REG_CONFIG_REG1_XFI3,
		M21441_REG_CONFIG_REG1_KR0r,
		M21441_REG_CONFIG_REG1_KR1r,
		M21441_REG_CONFIG_REG1_KR2r,
		M21441_REG_CONFIG_REG1_KR3r,
	};

	static unsigned char asc_val[12] = {
		0x06,
		0x0A,
		0x03,
		0x02,
		0x0B,
		0x07,
		0x05,
		0x09,
		0x01,
		0x00,
		0x08,
		0x04,
	};

	static unsigned char asc_reg[12] = {
		M21441_REG_ASC_REG1_KR0,
		M21441_REG_ASC_REG1_KR1,
		M21441_REG_ASC_REG1_KR2,
		M21441_REG_ASC_REG1_KR3,
		M21441_REG_ASC_REG1_XFI0,
		M21441_REG_ASC_REG1_XFI1,
		M21441_REG_ASC_REG1_XFI2,
		M21441_REG_ASC_REG1_XFI3,
		M21441_REG_ASC_REG1_KR0r,
		M21441_REG_ASC_REG1_KR1r,
		M21441_REG_ASC_REG1_KR2r,
		M21441_REG_ASC_REG1_KR3r
	};

	static unsigned char out_enable_reg[4] = {
		M21441_REG_CONFIG_REG3_XFI0,
		M21441_REG_CONFIG_REG3_XFI1,
		M21441_REG_CONFIG_REG3_XFI2,
		M21441_REG_CONFIG_REG3_XFI3,
	};

	static unsigned char cdr_reg[12] = {
		M21441_REG_CDR_REG1_KR0,
		M21441_REG_CDR_REG1_KR1,
		M21441_REG_CDR_REG1_KR2,
		M21441_REG_CDR_REG1_KR3,
		M21441_REG_CDR_REG1_XFI0,
		M21441_REG_CDR_REG1_XFI1,
		M21441_REG_CDR_REG1_XFI2,
		M21441_REG_CDR_REG1_XFI3,
		M21441_REG_CDR_REG1_KR0r,
		M21441_REG_CDR_REG1_KR1r,
		M21441_REG_CDR_REG1_KR2r,
		M21441_REG_CDR_REG1_KR3r,
	};

	u8 value;
	unsigned int index, ret = 0;

	/* Change to Page 0 */
	ret = m21441_write(client, M21441_REG_PAGE_ADDRESS, 0);
	if (ret < 0)
		return ret;

	/* Check Chip_id */
	value = m21441_read(client, M21441_REG_CHIP_ID);
	if (value == 0xE3) {
		dev_info(&client->dev, "Detected M21441 ID=0x%x\n", value);
	} else {
		dev_err(&client->dev, "Chip M2141 not detected: 0x%x\n", value);
		return value;
	}
	/* Software Reset On */
	ret = m21441_write(client, M21441_REG_MASTER_RESET, 0xaa);
	if (ret < 0)
		return ret;

	/* Software Reset Off */
	ret = m21441_write(client, M21441_REG_MASTER_RESET, 0x00);
	if (ret < 0)
		return ret;

	/* CDR scale and peak override */
	for (index = 0; index < 12; index++) {
		ret = m21441_write(client, cdr_reg[index], 0x60);
		if (ret < 0)
			return ret;
	}

	/* CDR Power Down */
	ret = m21441_write(client, M21441_REG_CDR_REG1_KR1, 0x80);
	if (ret < 0)
		return ret;

	ret = m21441_write(client, M21441_REG_CDR_REG1_XFI1, 0x80);
	if (ret < 0)
		return ret;

	ret = m21441_write(client, M21441_REG_CDR_REG1_KR3, 0x80);
	if (ret < 0)
		return ret;

	ret = m21441_write(client, M21441_REG_CDR_REG1_XFI3, 0x80);
	if (ret < 0)
		return ret;


	/* PLL Config */
	ret = m21441_write(client, M21441_REG_PLL1_REG1, 0x60);
	if (ret < 0)
		return ret;

	ret = m21441_write(client, M21441_REG_PLL3_REG1, 0x60);
	if (ret < 0)
		return ret;

	ret = m21441_write(client, M21441_REG_PLL0_REG1, 0x0);
	if (ret < 0)
		return ret;

	ret = m21441_write(client, M21441_REG_PLL2_REG1, 0x0);
	if (ret < 0)
		return ret;


	/* Change to Channel Mode */
	ret = m21441_write(client, M21441_REG_GEN_SW_CONFIG, 0x10);
	if (ret < 0)
		return ret;


	/* Active Software State */
	for (index = 0; index < 12; index++) {
		value = asc_val[index];
		ret = m21441_write(client, asc_reg[index], value);
		if (ret < 0)
			return ret;

		ret = m21441_write(client, asc_reg[index] + 10, value);
		if (ret < 0)
			return ret;
		ret = m21441_write(client, asc_reg[index] + 20, value);
		if (ret < 0)
			return ret;
	}

        /* Change to Page 1 */
	ret = m21441_write(client, M21441_REG_PAGE_ADDRESS, 1);
	if (ret < 0)
		return ret;

	/* Output channel enable */
	for (index = 0; index < 4; index++) {
		ret = m21441_write(client, out_enable_reg[index], 0x80);
		if (ret < 0)
			return ret;
	}
	/* Output channel signal*/
	for (index = 0; index <= 3; index++) {
		ret = m21441_write(client, out_signal_reg[index], 0x40);
		if (ret < 0)
			return ret;
	}

	for (index = 8; index < 12; index++) {
		ret = m21441_write(client, out_signal_reg[index], 0xa9);
		if (ret < 0)
			return ret;
	}

	dev_info(&client->dev, "M21441 Equalizer updated");
	return 0;
}

#define SFP_LABEL_SIZE  8
static struct i2c_board_info *alloc_sfp_board_info(int port) {
	char *label;
	struct eeprom_platform_data *eeprom_data;
	struct at24_platform_data *at24_data;
	struct i2c_board_info *board_info;

	label = kzalloc(SFP_LABEL_SIZE, GFP_KERNEL);
	if (!label) {
		goto err_exit;
	}
	eeprom_data = kzalloc(sizeof(struct eeprom_platform_data), GFP_KERNEL);
	if (!eeprom_data) {
		goto err_exit_eeprom;
	}
	at24_data = kzalloc(sizeof(struct at24_platform_data), GFP_KERNEL);
	if (!at24_data) {
		goto err_exit_at24;
	}
	board_info = kzalloc(sizeof(struct i2c_board_info), GFP_KERNEL);
	if (!board_info) {
		goto err_exit_board;
	}

	snprintf(label, SFP_LABEL_SIZE, "port%u", port);
	eeprom_data->label = label;

	at24_data->byte_len = 512;
	at24_data->flags = AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG;
	at24_data->page_size = 1;
	at24_data->eeprom_data = eeprom_data;

	strcpy(board_info->type, "24c04");
	board_info->addr = 0x50;
	board_info->platform_data = at24_data;

	return board_info;

err_exit_board:
	kfree(at24_data);
err_exit_at24:
	kfree(eeprom_data);
err_exit_eeprom:
	kfree(label);
err_exit:
	return NULL;
}

#define QSFP_LABEL_SIZE  8
static struct i2c_board_info *alloc_qsfp_board_info(int port) {
	char *label;
	struct eeprom_platform_data *eeprom_data;
	struct sff_8436_platform_data *sff8436_data;
	struct i2c_board_info *board_info;

	label = kzalloc(QSFP_LABEL_SIZE, GFP_KERNEL);
	if (!label) {
		goto err_exit;
	}
	eeprom_data = kzalloc(sizeof(struct eeprom_platform_data), GFP_KERNEL);
	if (!eeprom_data) {
		goto err_exit_eeprom;
	}
	sff8436_data = kzalloc(sizeof(struct sff_8436_platform_data), GFP_KERNEL);
	if (!sff8436_data) {
		goto err_exit_sff8436;
	}
	board_info = kzalloc(sizeof(struct i2c_board_info), GFP_KERNEL);
	if (!board_info) {
		goto err_exit_board;
	}

	snprintf(label, QSFP_LABEL_SIZE, "port%u", port);
	eeprom_data->label = label;

	sff8436_data->byte_len = 256;
	sff8436_data->flags = SFF_8436_FLAG_IRUGO;
	sff8436_data->page_size = 1;
	sff8436_data->eeprom_data = eeprom_data;

	strcpy(board_info->type, "sff8436");
	board_info->addr = 0x50;
	board_info->platform_data = sff8436_data;

	return board_info;

err_exit_board:
	kfree(sff8436_data);
err_exit_sff8436:
	kfree(eeprom_data);
err_exit_eeprom:
	kfree(label);
err_exit:
	return NULL;
}

static void free_sfp_board_info(struct i2c_board_info *board_info)
{
	struct at24_platform_data *at24_data = board_info->platform_data;
	struct eeprom_platform_data *eeprom_data = at24_data->eeprom_data;

	kfree(eeprom_data->label);
	kfree(eeprom_data);
	kfree(at24_data);
	kfree(board_info);
}

static void free_qsfp_board_info(struct i2c_board_info *board_info)
{
	struct sff_8436_platform_data *sff8436_data = board_info->platform_data;
	struct eeprom_platform_data *eeprom_data = sff8436_data->eeprom_data;

	kfree(eeprom_data->label);
	kfree(eeprom_data);
	kfree(sff8436_data);
	kfree(board_info);
}

static int ly8_alloc_sfp(int port, int bus, int num_ports) {
	struct i2c_board_info *b_info;
	struct i2c_client *c;
	int j;

	for (j = 0; j < num_ports; j++) {
		b_info = alloc_sfp_board_info(port + j);
		if (!b_info) {
			pr_err("could not allocate board info port: %d\n", port + j);
			return -1;
		}
		c = cumulus_i2c_add_client(bus + j, b_info);
		if (!c) {
			free_sfp_board_info(b_info);
			pr_err("could not create i2c_client %s port: %d\n", b_info->type, port + j);
			return -1;
		}
		ports_info[port + j - 1].b = b_info;
		ports_info[port + j - 1].c = c;
	}
	return 0;
}

static int ly8_alloc_qsfp(int port, int bus, int num_ports) {
	struct i2c_board_info *b_info;
	struct i2c_client *c;
	int j;

	for (j = 0; j < num_ports; j++) {
		b_info = alloc_qsfp_board_info(port + j);
		if (!b_info) {
			pr_err("could not allocate board info port: %d\n", port + j);
			return -1;
		}
		c = cumulus_i2c_add_client(bus + j, b_info);
		if (!c) {
			free_qsfp_board_info(b_info);
			pr_err("could not create i2c_client %s port: %d\n", b_info->type, port + j);
			return -1;
		}
		ports_info[port + j - 1].type = 1;
		ports_info[port + j - 1].b = b_info;
		ports_info[port + j - 1].c = c;
	}
	return 0;
}

static void free_i2c_devices(void)
{
	int i;

	for(i = 0; i < LY8_PORT_COUNT; i++) {
		if (ports_info[i].b) {
			if (ports_info[i].type == 1) {
				free_qsfp_board_info(ports_info[i].b);
			} else {
				free_sfp_board_info(ports_info[i].b);
			}
			ports_info[i].b = NULL;
		}
		if (ports_info[i].c) {
			i2c_unregister_device(ports_info[i].c);
			ports_info[i].c = NULL;
		}
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices_level3); i++) {
		i2c_unregister_device(i2c_clients_level3[i]);
		i2c_clients_level3[i] = NULL;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices_level2); i++) {
		i2c_unregister_device(i2c_clients_level2[i]);
		i2c_clients_level2[i] = NULL;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices_level1); i++) {
		i2c_unregister_device(i2c_clients_level1[i]);
		i2c_clients_level1[i] = NULL;
	}
}

static int init_i2c_devices(void)
{
	int ret = 0;
	int i801_bus = -1;
	struct i2c_client *m21441_client = NULL;
	int i;

	i801_bus = cumulus_i2c_find_adapter(SMB_I801_NAME);
	if (i801_bus < 0) {
		pr_err("Unable to find %s\n", SMB_I801_NAME);
		return -ENXIO;
	}

	/* Instantiate I2C devices */
	for (i = 0; i < ARRAY_SIZE(i2c_devices_level1); i++) {
		if (i2c_devices_level1[i].bus == LY8_I2C_I801_BUS)
			i2c_devices_level1[i].bus = i801_bus;
		i2c_clients_level1[i] = cumulus_i2c_add_client(i2c_devices_level1[i].bus,
						       &i2c_devices_level1[i].info);
		if (IS_ERR(i2c_clients_level1[i])) {
			ret = PTR_ERR(i2c_clients_level1[i]);
			goto err_exit;
		}
	}
	for (i = 0; i < ARRAY_SIZE(i2c_devices_level2); i++) {
		i2c_clients_level2[i] = cumulus_i2c_add_client(i2c_devices_level2[i].bus,
						       &i2c_devices_level2[i].info);
		if (IS_ERR(i2c_clients_level2[i])) {
			ret = PTR_ERR(i2c_clients_level2[i]);
			goto err_exit;
		}
		if (i2c_devices_level2[i].has_port == 1) {
			ly8_alloc_sfp(i2c_devices_level2[i].port_base,
				      i2c_devices_level2[i].port_bus,
				      i2c_devices_level2[i].num_ports);
		} else if (i2c_devices_level2[i].has_port == 2) {
			ly8_alloc_qsfp(i2c_devices_level2[i].port_base,
				       i2c_devices_level2[i].port_bus,
   				       i2c_devices_level2[i].num_ports);
		}
	}
	for (i = 0; i < ARRAY_SIZE(i2c_devices_level3); i++) {
		i2c_clients_level3[i] = cumulus_i2c_add_client(i2c_devices_level3[i].bus,
						       &i2c_devices_level3[i].info);
		if (IS_ERR(i2c_clients_level3[i])) {
			ret = PTR_ERR(i2c_clients_level3[i]);
			goto err_exit;
		}
		if (i2c_devices_level3[i].info.addr == 0x21)
			m21441_client = i2c_clients_level3[i];
	}
	if (m21441_client != NULL) {
		ret = m21441_init(m21441_client);
		if (ret < 0)
			pr_err("Initialization of M21441 failed\n");
	} else {
		pr_err("M21441 device not found\n");
	}

	pr_debug(DRIVER_NAME "LY8: I2C Init succeeded \n");
	return 0;
err_exit:
	pr_err(DRIVER_NAME "LY8: I2C Init failed \n");
	free_i2c_devices();
	return ret;
}

static int ly8_i2c_init(void)
{
	int ret = 0;

	init_gpio_platform_data(gpio_24_pins, ARRAY_SIZE(gpio_24_pins),
				&gpio_24_platform_data);
	init_gpio_platform_data(gpio_23_pins, ARRAY_SIZE(gpio_23_pins),
				&gpio_23_platform_data);
	init_gpio_platform_data(gpio_26_pins, ARRAY_SIZE(gpio_26_pins),
				&gpio_26_platform_data);
	init_gpio_platform_data(gpio_24_2_pins, ARRAY_SIZE(gpio_24_2_pins),
				&gpio_24_2_platform_data);
	init_gpio_platform_data(gpio_23_2_pins, ARRAY_SIZE(gpio_23_2_pins),
				&gpio_23_2_platform_data);
	init_gpio_platform_data(gpio_23_3_pins, ARRAY_SIZE(gpio_23_3_pins),
				&gpio_23_3_platform_data);
	init_gpio_platform_data(gpio_20_pins, ARRAY_SIZE(gpio_20_pins),
				&gpio_20_platform_data);
	init_gpio_platform_data(gpio_sfp_1_8_pins, ARRAY_SIZE(gpio_sfp_1_8_pins),
				&gpio_sfp_1_8_platform_data);
	init_gpio_platform_data(gpio_sfp_9_16_pins, ARRAY_SIZE(gpio_sfp_9_16_pins),
				&gpio_sfp_9_16_platform_data);
	init_gpio_platform_data(gpio_sfp_17_24_pins, ARRAY_SIZE(gpio_sfp_17_24_pins),
				&gpio_sfp_17_24_platform_data);
	init_gpio_platform_data(gpio_sfp_25_32_pins, ARRAY_SIZE(gpio_sfp_25_32_pins),
				&gpio_sfp_25_32_platform_data);
	init_gpio_platform_data(gpio_sfp_33_40_pins, ARRAY_SIZE(gpio_sfp_33_40_pins),
				&gpio_sfp_33_40_platform_data);
	init_gpio_platform_data(gpio_sfp_41_48_pins, ARRAY_SIZE(gpio_sfp_41_48_pins),
				&gpio_sfp_41_48_platform_data);

	ret = init_i2c_devices();

	return 0;
}

static void ly8_i2c_exit(void)
{
	free_i2c_devices();
}

static int ly8_platform_probe(struct platform_device *dev)
{
	int ret = 0;

	pr_info(DRIVER_NAME ": Probe begin \n");
	ret = ly8_i2c_init();
	if (ret) {
		goto err_exit;
	}

	return 0;

err_exit:
	if (ret != -EPROBE_DEFER) {
		dev_info(&dev->dev, "error during probe, deleting clients\n");
		ly8_i2c_exit();
	}

	return ret;
}

static int ly8_platform_remove(struct platform_device *dev)
{
	ly8_i2c_exit();
	return 0;
}

static const struct platform_device_id ly8_platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, ly8_platform_id);

static struct platform_driver ly8_platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = ly8_platform_probe,
	.remove = ly8_platform_remove,
};

static struct platform_device *ly8_plat_device = NULL;

static int __init ly8_platform_init(void)
{
	int ret = -1;

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" initializing\n");

	if (!driver_find(ly8_platform_driver.driver.name,
			 &platform_bus_type)) {
		ret = platform_driver_register(&ly8_platform_driver);
		if (ret) {
			pr_err(DRIVER_NAME ": %s driver registration failed"
			       " with %d  \n", ly8_platform_driver.driver.name,
				ret);
			return ret;
		}
	}

	if (ly8_plat_device == NULL) {
		ly8_plat_device = platform_device_register_simple(DRIVER_NAME,
								  -1,
								  NULL,
								  0);
		if (IS_ERR(ly8_plat_device)) {
			ret = PTR_ERR(ly8_plat_device);
			ly8_plat_device = NULL;
			pr_err(DRIVER_NAME ": Platform device registration"
			       " failed \n");
			return ret;
		}
	}

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;

}

static void __exit ly8_platform_exit(void)
{
	platform_device_unregister(ly8_plat_device);
	ly8_plat_device = NULL;
	platform_driver_unregister(&ly8_platform_driver);
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " driver unloaded\n");
}

module_init(ly8_platform_init);
module_exit(ly8_platform_exit);

MODULE_AUTHOR("Puneet Shenoy (puneet@cumulusnetworks.com)");
MODULE_DESCRIPTION("Quanta LY8 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
