/*
 * quanta_ly9_rangeley_platform.c - Quanta LY9 Platform Support
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

#define DRIVER_NAME	"quanta_ly9_platform"
#define DRIVER_VERSION	"1.1"

#define LY9_I2C_I801_BUS 0
#define LY9_PORT_COUNT 54

static struct platform_driver ly9_platform_driver;

mk_eeprom(spd1,  52, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(cpu, 53, 256, AT24_FLAG_IRUGO);
mk_eeprom(board, 54, 256, AT24_FLAG_IRUGO);

struct ly9_device_info {
	int bus;
	struct i2c_board_info info;
	int has_port;  /* SFP = 1, QSFP = 2, QSFP backports = 3 */
	int port_base;
	int port_bus;
	int num_ports;
};

struct port_info {
	int type; /* SFP = 0, QSFP = 1 */
	struct i2c_board_info *b;
	struct i2c_client *c;
};
static struct port_info ports_info[LY9_PORT_COUNT];

/*  I2C Muxes  */
#define ly9_pca9546(addr, busno)					\
	enum {								\
		LY9_PCA9546_##addr##_0 = busno,				\
		LY9_PCA9546_##addr##_1,					\
		LY9_PCA9546_##addr##_2,					\
		LY9_PCA9546_##addr##_3,					\
	};								\
	static struct pca954x_platform_mode ly9_mode_pca9546_##addr [] = { \
		{ .adap_id = LY9_PCA9546_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = LY9_PCA9546_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = LY9_PCA9546_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = LY9_PCA9546_##addr##_3, .deselect_on_exit = 1,},			\
	};								\
	static struct pca954x_platform_data ly9_data_pca9546_##addr = { \
		.modes = ly9_mode_pca9546_##addr,			\
		.num_modes = ARRAY_SIZE(ly9_mode_pca9546_##addr),	\
	};

#define ly9_pca9548(addr, busno)					\
	enum {								\
		LY9_PCA9548_##addr##_0 = busno,				\
		LY9_PCA9548_##addr##_1,					\
		LY9_PCA9548_##addr##_2,					\
		LY9_PCA9548_##addr##_3,					\
		LY9_PCA9548_##addr##_4,					\
		LY9_PCA9548_##addr##_5,					\
		LY9_PCA9548_##addr##_6,					\
		LY9_PCA9548_##addr##_7,					\
	};								\
	static struct pca954x_platform_mode ly9_mode_pca9548_##addr [] = { \
		{ .adap_id = LY9_PCA9548_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = LY9_PCA9548_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = LY9_PCA9548_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = LY9_PCA9548_##addr##_3, .deselect_on_exit = 1,}, \
		{ .adap_id = LY9_PCA9548_##addr##_4, .deselect_on_exit = 1,}, \
		{ .adap_id = LY9_PCA9548_##addr##_5, .deselect_on_exit = 1,}, \
		{ .adap_id = LY9_PCA9548_##addr##_6, .deselect_on_exit = 1,}, \
		{ .adap_id = LY9_PCA9548_##addr##_7, .deselect_on_exit = 1,}, \
	};								\
	static struct pca954x_platform_data ly9_data_pca9548_##addr = { \
		.modes = ly9_mode_pca9548_##addr,			\
		.num_modes = ARRAY_SIZE(ly9_mode_pca9548_##addr),	\
	};

/* I2C I801 -> PCA9546(0x71) */
ly9_pca9546(71, 10);
/* I2C I801 -> PCA9546_1(0x72) */
ly9_pca9546(72, 14)
/* I2C I801 -> PCA9548_1(0x77) */
ly9_pca9548(77, 18)
/* I2C I801 -> PCA9548_2_1(0x77) -> CH5 -> PCA9548_8 (0x76)*/
ly9_pca9548(2_76, 26)
/* I2C I801 -> PCA9546_1(0x72)  -> PCA9546(0x76)*/
ly9_pca9546(76, 34)

/* GPIOs */
#define LY9_GPIO_BASE 100
#define LY9_GPIO_20_BASE LY9_GPIO_BASE
#define LY9_GPIO_23_BASE (LY9_GPIO_20_BASE + PCA_9555_GPIO_COUNT)
#define LY9_GPIO_24_BASE (LY9_GPIO_23_BASE + PCA_9555_GPIO_COUNT)
#define LY9_GPIO_26_BASE (LY9_GPIO_24_BASE + PCA_9555_GPIO_COUNT)
#define LY9_GPIO_23_2_BASE (LY9_GPIO_26_BASE + PCA_9555_GPIO_COUNT)
#define LY9_GPIO_23_3_BASE (LY9_GPIO_23_2_BASE + PCA_9555_GPIO_COUNT)

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

mk_gpio_pins(gpio_23) = {
	mk_gpio_pin(0,    fan0_board,            GPIOF_DIR_IN),
	mk_gpio_pin(1,    fan1_board,            GPIOF_DIR_IN),
	mk_gpio_pin(2,    fan2_board,            GPIOF_DIR_IN),
	mk_gpio_pin(3,    fan3_board,            GPIOF_DIR_IN),
	mk_gpio_pin(4,    qsfp_power_good,       GPIOF_DIR_IN),
	mk_gpio_pin(5,    qsfp_3v3_pwr_enable,   GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(6,    qsfp_present,          GPIOF_DIR_IN),
	mk_gpio_pin(7,    usb_reset,             GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,    cpld_int,              GPIOF_DIR_IN),
	mk_gpio_pin(9,    mgmt_present,          GPIOF_DIR_IN),
	mk_gpio_pin(10,   board_id_0,            GPIOF_DIR_IN),
	mk_gpio_pin(11,   board_id_1,            GPIOF_DIR_IN),
	mk_gpio_pin(12,   board_id_2,            GPIOF_DIR_IN),
	mk_gpio_pin(13,   board_id_3,            GPIOF_DIR_IN),
	mk_gpio_pin(14,   board_id_4,            GPIOF_DIR_IN),
	mk_gpio_pin(15,   board_id_5,            GPIOF_DIR_IN),

};

mk_gpio_pins(gpio_24) = {
	mk_gpio_pin(0,    qsfp_12v_pwr_enable,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(1,    9555_5_int,            GPIOF_DIR_IN),
	mk_gpio_pin(2,    9555_4_int,            GPIOF_DIR_IN),
	mk_gpio_pin(3,    bcm5461s_int,          GPIOF_DIR_IN),
	mk_gpio_pin(4,    fan1_present,          GPIOF_DIR_IN),
	mk_gpio_pin(5,    fan2_present,          GPIOF_DIR_IN),
	mk_gpio_pin(6,    fan3_present,          GPIOF_DIR_IN),
	mk_gpio_pin(7,    nc,                    GPIOF_DIR_IN),
	mk_gpio_pin(8,    fan1_f2b,              GPIOF_DIR_IN),
	mk_gpio_pin(9,    fan2_f2b,              GPIOF_DIR_IN),
	mk_gpio_pin(10,   fan3_f2b,              GPIOF_DIR_IN),
	mk_gpio_pin(11,   nc_1,                  GPIOF_DIR_IN),
	mk_gpio_pin(12,   fan1_led,              GPIOF_DIR_IN),
	mk_gpio_pin(13,   fan2_led,              GPIOF_DIR_IN),
	mk_gpio_pin(14,   fan3_led,              GPIOF_DIR_IN),
	mk_gpio_pin(15,   qsfp_int,              GPIOF_DIR_IN),
};

mk_gpio_pins(gpio_26) = {
	mk_gpio_pin(0,    psu_pwr1_present,     GPIOF_DIR_IN),
	mk_gpio_pin(1,    psu_pwr1_dc_ok,       GPIOF_DIR_IN),
	mk_gpio_pin(2,    psu_pwr1_int,         GPIOF_DIR_IN),
	mk_gpio_pin(3,    psu_pwr2_present,     GPIOF_DIR_IN),
	mk_gpio_pin(4,    psu_pwr2_dc_ok,       GPIOF_DIR_IN),
	mk_gpio_pin(5,    psu_pwr2_int,         GPIOF_DIR_IN),
	mk_gpio_pin(6,    psu_pwr1_ac_ok,       GPIOF_DIR_IN),
	mk_gpio_pin(7,    psu_pwr2_ac_ok,       GPIOF_DIR_IN),
	mk_gpio_pin(8,    psu_pwr1_reset,       GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,    psu_pwr2_reset,       GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(10,   psu2_green_led,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,   psu2_red_led,         GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12,   psu1_green_led,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(13,   psu1_red_led,         GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14,   fan_green_led,        GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(15,   fan_red_led,          GPIOF_OUT_INIT_LOW),
};

mk_gpio_pins(gpio_23_2) = {
	mk_gpio_pin(0,    bmc56854_rst,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(1,    phy1_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(2,    phy2_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(3,    phy3_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(4,    phy4_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(5,    phy5_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(6,    phy6_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,    phy7_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,    phy8_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(9,    phy9_rst,       GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(10,   phy10_rst,      GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,   phy11_rst,      GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(12,   phy12_rst,      GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(13,   m21441_rst,     GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(14,   phy_iso,        GPIOF_OUT_INIT_HIGH),
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
	mk_gpio_pin(8,    qsfpd_power_enable, GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(9,    sys_reset,          GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(10,   9546_reset,         GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,   bcm84328_reset,     GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(12,   nc_9,               GPIOF_DIR_IN),
	mk_gpio_pin(13,   qdb_id_0,           GPIOF_DIR_IN),
	mk_gpio_pin(14,   qdb_id_1,           GPIOF_DIR_IN),
	mk_gpio_pin(15,   qdb_id_2,           GPIOF_DIR_IN),
};

mk_gpio_platform_data(gpio_20,     LY9_GPIO_20_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_20_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_23,     LY9_GPIO_23_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_23_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_24,     LY9_GPIO_24_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_24_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_26,     LY9_GPIO_26_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_26_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_23_2,   LY9_GPIO_23_2_BASE, PCA_9555_GPIO_COUNT,
		      &gpio_23_2_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_23_3,   LY9_GPIO_23_3_BASE, PCA_9555_GPIO_COUNT,
		      &gpio_23_3_pins, init_gpio_pins, free_gpio_pins);

/* I2C Device Map */
static struct ly9_device_info i2c_devices_level1[] = {
	{
		.bus = LY9_I2C_I801_BUS,       /* SPD DIMM EEPROM */
		{I2C_BOARD_INFO("spd", 0x52),
		 .platform_data = &spd1_52_at24,},
	},
	{
		.bus = LY9_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9546", 0x71),
		 .platform_data = &ly9_data_pca9546_71,},
	},
	{
		.bus = LY9_I2C_I801_BUS,
		{I2C_BOARD_INFO("ly9_rangeley_cpld", 0x3a),},
	},
	{
		.bus = LY9_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9546", 0x72),
		 .platform_data = &ly9_data_pca9546_72,},
	},
	{
		.bus = LY9_I2C_I801_BUS,     /* Sensors Chip */
		{I2C_BOARD_INFO("CY8C3245R1", 0x4e),},
	},
	{
		.bus = LY9_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9548", 0x77),
		 .platform_data = &ly9_data_pca9548_77,},
	},

};

static struct ly9_device_info i2c_devices_level2[] = {
	/* Following devices are on CPU Board */
	{
		.bus = LY9_PCA9546_71_0,
		{I2C_BOARD_INFO("pca9555", 0x20),
		.platform_data = &gpio_20_platform_data},
	},
	{
		.bus = LY9_PCA9546_71_1,
		{I2C_BOARD_INFO("24c02", 0x53),
		 .platform_data = &cpu_53_at24,},
	},
	/* Following devices are on MB */
	{
		.bus = LY9_PCA9546_72_0, /* PSU1 PMBUS */
		{I2C_BOARD_INFO("ps2471", 0x6f),},
	},
	{
		.bus = LY9_PCA9546_72_1, /* PSU2 PMBUS */
		{I2C_BOARD_INFO("ps2471", 0x69),},
	},
	{
		.bus = LY9_PCA9546_72_2,
		{I2C_BOARD_INFO("pca9555", 0x26),
		.platform_data = &gpio_26_platform_data},
	},
	{
		.bus = LY9_PCA9546_72_3,   /* Board EEPROM  */
		{I2C_BOARD_INFO("24c02", 0x54),
		 .platform_data = &board_54_at24,},
	},
	{
		.bus = LY9_PCA9548_77_0,
		{I2C_BOARD_INFO("pca9555", 0x24),
		.platform_data = &gpio_24_platform_data},
	},
	{
		.bus = LY9_PCA9548_77_0,
		{I2C_BOARD_INFO("pca9555", 0x23),
		 .platform_data = &gpio_23_platform_data},
	},
	{
		.bus = LY9_PCA9548_77_5,
		{I2C_BOARD_INFO("pca9548", 0x76),
		 .platform_data = &ly9_data_pca9548_2_76,},
		 .has_port = 2, .port_base = 49,
		 .port_bus = LY9_PCA9548_2_76_0,
 		 .num_ports = 4,
	},
	{
		.bus = LY9_PCA9548_77_6,
		{I2C_BOARD_INFO("pca9555", 0x23),
		 .platform_data = &gpio_23_2_platform_data},
	},
	{
		.bus = LY9_PCA9548_77_7,
		{I2C_BOARD_INFO("pca9555", 0x23),
		 .platform_data = &gpio_23_3_platform_data},
	},
	{
		.bus = LY9_PCA9548_77_7,
		{I2C_BOARD_INFO("pca9546", 0x76),
		 .platform_data = &ly9_data_pca9546_76,},
		 .has_port = 3, .port_base = 53,
		 .port_bus = LY9_PCA9546_76_0,
 		 .num_ports = 2,
	},

};

static struct ly9_device_info i2c_devices_level3[] = {
	{
		.bus = LY9_PCA9548_2_76_4,
		{I2C_BOARD_INFO("dummy", 0x21),}, /* M21441 Chip for Port 53 */
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

#define QSFP_LABEL_SIZE  8
static struct i2c_board_info *alloc_qsfp_board_info(int port) {
	char *label = NULL;
	struct eeprom_platform_data *eeprom_data = NULL;
	struct sff_8436_platform_data *sff8436_data = NULL;
	struct i2c_board_info *board_info = NULL;

	label = kzalloc(QSFP_LABEL_SIZE, GFP_KERNEL);
	if (!label)
		goto err_kzalloc;

	eeprom_data = kzalloc(sizeof(struct eeprom_platform_data), GFP_KERNEL);
	if (!eeprom_data)
		goto err_kzalloc;

	sff8436_data = kzalloc(sizeof(struct sff_8436_platform_data), GFP_KERNEL);
	if (!sff8436_data)
		goto err_kzalloc;

	board_info = kzalloc(sizeof(struct i2c_board_info), GFP_KERNEL);
	if (!board_info)
		goto err_kzalloc;

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

err_kzalloc:
	kfree(sff8436_data);
	kfree(eeprom_data);
	kfree(label);

	return NULL;
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

static int ly9_alloc_qsfp_rev(int port, int bus, int num_ports) {
	struct i2c_board_info *b_info;
	struct i2c_client *c;
	int j;

	for (j = 0; j < num_ports; j++) {
		b_info = alloc_qsfp_board_info(port + num_ports - (j + 1));
		if (!b_info) {
			pr_err("could not allocate board info port: %d\n", port + j);
			return -ENOMEM;
		}
		c = cumulus_i2c_add_client(bus + j, b_info);
		if (!c) {
			free_qsfp_board_info(b_info);
			pr_err("could not create i2c_client %s port: %d\n", b_info->type, port + num_ports - (j + 1));
			return -ENOMEM;
		}
		/* port numbers start from 1, not 0 */
		ports_info[port + num_ports - (j + 1)].type = 1;
		ports_info[port + num_ports - (j + 1)].b = b_info;
		ports_info[port + num_ports - (j + 1)].c = c;
	}
	return 0;
}

static int ly9_alloc_qsfp(int port, int bus, int num_ports) {
	struct i2c_board_info *b_info;
	struct i2c_client *c;
	int j;

	for (j = 0; j < num_ports; j++) {
		b_info = alloc_qsfp_board_info(port + j);
		if (!b_info) {
			pr_err("could not allocate board info port: %d\n", port + j);
			return -ENOMEM;
		}
		c = cumulus_i2c_add_client(bus + j, b_info);
		if (!c) {
			free_qsfp_board_info(b_info);
			pr_err("could not create i2c_client %s port: %d\n", b_info->type, port + j);
			return -ENOMEM;
		}
		/* port numbers start from 1, not 0 */
		ports_info[port + j - 1].type = 1;
		ports_info[port + j - 1].b = b_info;
		ports_info[port + j - 1].c = c;
	}
	return 0;
}

static void free_i2c_devices(void)
{
	int i;

	for(i = 0; i < LY9_PORT_COUNT; i++) {
		if (ports_info[i].b) {
			free_qsfp_board_info(ports_info[i].b);
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
		pr_err("Unable to find %s\n", I801_ADAPTER_NAME);
		return -ENXIO;
	}

	/* Instantiate I2C devices */
	for (i = 0; i < ARRAY_SIZE(i2c_devices_level1); i++) {
		if (i2c_devices_level1[i].bus == LY9_I2C_I801_BUS)
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
		if (i2c_devices_level2[i].has_port == 2) {
			ret = ly9_alloc_qsfp(i2c_devices_level2[i].port_base,
					     i2c_devices_level2[i].port_bus,
					     i2c_devices_level2[i].num_ports);
			if (ret)
				goto err_exit;
		}
		/* QSFP 53-54 EEPROMs are in reverse order */
		if (i2c_devices_level2[i].has_port == 3) {
			ret = ly9_alloc_qsfp_rev(i2c_devices_level2[i].port_base,
					     i2c_devices_level2[i].port_bus,
					     i2c_devices_level2[i].num_ports);
			if (ret)
				goto err_exit;
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

	pr_debug(DRIVER_NAME "LY9: I2C Init succeeded \n");
	return 0;
err_exit:
	pr_err(DRIVER_NAME "LY9: I2C Init failed \n");
	free_i2c_devices();
	return ret;

}

static int ly9_i2c_init(void)
{
	int ret = 0;

	init_gpio_platform_data(gpio_20_pins, ARRAY_SIZE(gpio_20_pins),
				&gpio_20_platform_data);
	init_gpio_platform_data(gpio_23_pins, ARRAY_SIZE(gpio_23_pins),
				&gpio_23_platform_data);
	init_gpio_platform_data(gpio_24_pins, ARRAY_SIZE(gpio_24_pins),
				&gpio_24_platform_data);
	init_gpio_platform_data(gpio_26_pins, ARRAY_SIZE(gpio_26_pins),
				&gpio_26_platform_data);
	init_gpio_platform_data(gpio_23_2_pins, ARRAY_SIZE(gpio_23_2_pins),
				&gpio_23_2_platform_data);
	init_gpio_platform_data(gpio_23_3_pins, ARRAY_SIZE(gpio_23_3_pins),
				&gpio_23_3_platform_data);

	/*
	 * Error handling is already done as part of init_i2c_devices;
	 */
	ret = init_i2c_devices();

	return ret;

}

static void ly9_i2c_exit(void)
{
	free_i2c_devices();
}

static int ly9_platform_probe(struct platform_device *dev)
{
	int ret = 0;

	pr_info(DRIVER_NAME ": Probe begin \n");
	ret = ly9_i2c_init();
	if (ret) {
		goto err_exit;
	}

	/*
	 * GPIO init is done through mk_gpio_platform_data setup/
	 * teardown
	 */

	pr_info(DRIVER_NAME ": Probe Succeeded \n");
	return 0;

err_exit:
	if (ret != -EPROBE_DEFER) {
		dev_info(&dev->dev, "error during probe, deleting clients\n");
		ly9_i2c_exit();
	}

	return ret;
}

static int ly9_platform_remove(struct platform_device *dev)
{
	ly9_i2c_exit();
	return 0;
}

static const struct platform_device_id ly9_platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, ly9_platform_id);

static struct platform_driver ly9_platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = ly9_platform_probe,
	.remove = ly9_platform_remove,
};

static struct platform_device *ly9_plat_device = NULL;

static int __init ly9_platform_init(void)
{
	int ret = -1;

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" initializing\n");

	if (!driver_find(ly9_platform_driver.driver.name,
			 &platform_bus_type)) {
		ret = platform_driver_register(&ly9_platform_driver);
		if (ret) {
			pr_err(DRIVER_NAME ": %s driver registration failed"
			       " with %d  \n", ly9_platform_driver.driver.name,
				ret);
			return ret;
		}
	}

	if (ly9_plat_device == NULL) {
		ly9_plat_device = platform_device_register_simple(DRIVER_NAME,
								  -1,
								  NULL,
								  0);
		if (IS_ERR(ly9_plat_device)) {
			ret = PTR_ERR(ly9_plat_device);
			ly9_plat_device = NULL;
			pr_err(DRIVER_NAME ": Platform device registration"
			       " failed \n");
			return ret;
		}
	}

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;
}

static void __exit ly9_platform_exit(void)
{
	platform_device_unregister(ly9_plat_device);
	ly9_plat_device = NULL;
	platform_driver_unregister(&ly9_platform_driver);
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " driver unloaded\n");
}

module_init(ly9_platform_init);
module_exit(ly9_platform_exit);

MODULE_AUTHOR("Puneet Shenoy (puneet@cumulusnetworks.com)");
MODULE_DESCRIPTION("Quanta LY9 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
