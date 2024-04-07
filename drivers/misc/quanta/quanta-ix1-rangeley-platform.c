/*
 * quanta-ix1-rangeley-platform.c - Quanta IX1 Platform Support
 *
 * Copyright (C) 2016, 2017, 2018, 2019, 2020 Cumulus Networks, Inc.
 * Author: Vidya Sagar Ravipati (vidya@cumulusnetworks.com)
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

#include "platform-defs.h"
#include "quanta-ix-cpld.h"

#define DRIVER_NAME	"quanta_ix1_platform"
#define DRIVER_VERSION	"1.1"
#define SMB_I801_NAME "SMBus I801 adapter"
#define IX1_I2C_I801_BUS 0
#define PCA_9555_GPIO_COUNT 16
#define IX1_PORT_COUNT 32

static struct platform_driver ix1_platform_driver;

enum {
	PORT_TYPE_NONE = 0,
	PORT_TYPE_SFP28,
	PORT_TYPE_QSFP28,
};

mk_eeprom(spd1,  52, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(spd2,  53, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(cpu, 50, 256, AT24_FLAG_IRUGO);
mk_eeprom(board, 54, 256, AT24_FLAG_IRUGO);

struct ix1_device_info {
	int bus;
	struct i2c_board_info info;
	int has_port;  /* 1 for SFP, 2 for QSFP */
	int port_base;
	int port_bus;
	int num_ports;
};

struct ix1_gpio_pin_info {
	char *name;
	int direction; /* 1 = out */
	int value;
	int nc;
};

struct port_info {
	int type; /* 0 = SFP , 1 = QSFP */
	struct i2c_board_info *b;
	struct i2c_client *c;
};

static struct port_info ports_info[IX1_PORT_COUNT];

/*  I2C Muxes  */
#define ix1_pca9546(addr, busno)					\
	enum {								\
		IX1_PCA9546_##addr##_0 = busno,				\
		IX1_PCA9546_##addr##_1,					\
		IX1_PCA9546_##addr##_2,					\
		IX1_PCA9546_##addr##_3,					\
	};								\
	static struct pca954x_platform_mode ix1_mode_pca9546_##addr[] = { \
		{ .adap_id = IX1_PCA9546_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = IX1_PCA9546_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = IX1_PCA9546_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = IX1_PCA9546_##addr##_3, .deselect_on_exit = 1,}, \
	};								\
	static struct pca954x_platform_data ix1_data_pca9546_##addr = { \
		.modes = ix1_mode_pca9546_##addr,			\
		.num_modes = ARRAY_SIZE(ix1_mode_pca9546_##addr),	\
	}

#define ix1_pca9548(addr, busno)					\
	enum {								\
		IX1_PCA9548_##addr##_0 = busno,				\
		IX1_PCA9548_##addr##_1,					\
		IX1_PCA9548_##addr##_2,					\
		IX1_PCA9548_##addr##_3,					\
		IX1_PCA9548_##addr##_4,					\
		IX1_PCA9548_##addr##_5,					\
		IX1_PCA9548_##addr##_6,					\
		IX1_PCA9548_##addr##_7,					\
	};								\
	static struct pca954x_platform_mode ix1_mode_pca9548_##addr[] = { \
		{ .adap_id = IX1_PCA9548_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = IX1_PCA9548_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = IX1_PCA9548_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = IX1_PCA9548_##addr##_3, .deselect_on_exit = 1,}, \
		{ .adap_id = IX1_PCA9548_##addr##_4, .deselect_on_exit = 1,}, \
		{ .adap_id = IX1_PCA9548_##addr##_5, .deselect_on_exit = 1,}, \
		{ .adap_id = IX1_PCA9548_##addr##_6, .deselect_on_exit = 1,}, \
		{ .adap_id = IX1_PCA9548_##addr##_7, .deselect_on_exit = 1,}, \
	};								\
	static struct pca954x_platform_data ix1_data_pca9548_##addr = { \
		.modes = ix1_mode_pca9548_##addr,			\
		.num_modes = ARRAY_SIZE(ix1_mode_pca9548_##addr),	\
	}

/* I2C I801 -> PCA9546(0x71) */
ix1_pca9546(71, 10);
/* I2C I801 -> PCA9546_1(0x77) */
ix1_pca9546(77, 14);
/* I2C I801 -> PCA9546_2(0x72) */
ix1_pca9546(72, 18);
/* I2C I801 -> PCA9546_1_0(0x77) -> PCA9548_1_1 (0x73)*/
ix1_pca9548(0_73, 22);
/* I2C I801 -> PCA9546_1_0(0x77) -> PCA9548_1_2 (0x74)*/
ix1_pca9548(0_74, 30);
/* I2C I801 -> PCA9546_1_0(0x77) -> PCA9548_1_3 (0x75)*/
ix1_pca9548(0_75, 38);
/* I2C I801 -> PCA9546_1_1(0x77) -> PCA9548_2_4 (0x73)*/
ix1_pca9548(1_76, 46);

struct quanta_ix1_rangeley_platform_data {
	int idx;
};

static struct quanta_ix1_rangeley_platform_data cpld_1 = {
	.idx = IX1_LED_QSFP28_1_16_CPLD_ID,
};

static struct quanta_ix1_rangeley_platform_data cpld_2 = {
	.idx = IX1_IO_QSFP28_1_16_CPLD_ID,
};

static struct quanta_ix1_rangeley_platform_data cpld_3 = {
	.idx = IX1_LED_QSFP28_17_32_CPLD_ID,
};

static struct quanta_ix1_rangeley_platform_data cpld_4 = {
	.idx = IX1_IO_QSFP28_17_32_CPLD_ID,
};

static struct quanta_ix1_rangeley_platform_data cpld_5 = {
	.idx = IX1_I2C_BUS_MNTR_CPLD_ID,
};

/* GPIOs */
#define IX1_GPIO_BASE 100
#define IX1_GPIO_20_BASE IX1_GPIO_BASE
#define IX1_GPIO_22_BASE (IX1_GPIO_20_BASE + PCA_9555_GPIO_COUNT)
#define IX1_GPIO_26_BASE (IX1_GPIO_22_BASE + PCA_9555_GPIO_COUNT)
#define IX1_GPIO_23_BASE (IX1_GPIO_26_BASE + PCA_9555_GPIO_COUNT)
#define IX1_GPIO_25_BASE (IX1_GPIO_23_BASE + PCA_9555_GPIO_COUNT)

static struct ix1_gpio_pin_info ix1_gpio_20_info[PCA_9555_GPIO_COUNT] = {
	{.name = "mb_i2c_rst", .direction = 1, .value = 1,},
	{.name = "mb_led_rst", .direction = 1, .value = 1,},
	{.name = "mb_eth_rst", .direction = 1, .value = 1,},
	{.name = "mb_sw_rst", .direction = 1, .value = 1,},
	{.name = "mb_odd_phy_rst", .direction = 1, .value = 1,},
	{.name = "mb_even_phy_rst", .direction = 1, .value = 1,},
	{.name = "alta_rst", .direction = 1, .value = 1,},
	{.name = "usb_rst", .direction = 1, .value = 1,},

	{.name = "alta_power", .direction = 1, .value = 1,},
	{.name = "nc_10",},
	{.name = "boot_led", .direction = 1, .value = 0,},
	{.name = "sys_led", .direction = 1, .value = 1,},
	{.name = "board_pwr_led", .direction = 1, .value = 1,},
	{.name = "bmc_gpio_0",},
	{.name = "msata__rst", .direction = 1, .value = 1,},
	{.name = "bmc_gpio_1",},
};

static const char *ix1_gpio_20_names[PCA_9555_GPIO_COUNT];
static struct pca953x_platform_data ix1_gpio_20_data = {
	.gpio_base = IX1_GPIO_20_BASE,
	.names = ix1_gpio_20_names,
};

static struct ix1_gpio_pin_info ix1_gpio_22_info[PCA_9555_GPIO_COUNT] = {
	{.name = "master_reset_n",},
	{.name = "smb_pch_alert_fp_n",},
	{.name = "pch_ot1_n",},
	{.name = "sb_present",},
	{.name = "usb_oc_n",},
	{.name = "gpio_o_sb",},
	{.name = "bmc_gpio_2",},
	{.name = "bmc_gpio_3",},
	{.name = "sb_irq0_n",},
	{.name = "mb_pca9555_int_n",},
	{.name = "db_irq1_n",},
	{.name = "psoc_irq1_n",},
	{.name = "psu_9555_irq_n",},
	{.name = "nc_11",},
};

static const char *ix1_gpio_22_names[PCA_9555_GPIO_COUNT];
static struct pca953x_platform_data ix1_gpio_22_data = {
	.gpio_base = IX1_GPIO_22_BASE,
	.names = ix1_gpio_22_names,
};

static struct ix1_gpio_pin_info ix1_gpio_26_info[PCA_9555_GPIO_COUNT] = {
	{.name = "psu_pwr1_present",},
	{.name = "psu_pwr1_dc_ok",},
	{.name = "psu_pwr1_int",},
	{.name = "psu_pwr2_present",},
	{.name = "psu_pwr2_dc_ok",},
	{.name = "psu_pwr2_int",},
	{.name = "psu_pwr1_ac_ok",},
	{.name = "psu_pwr2_ac_ok",},
	{.name = "psu_pwr1_reset", .direction = 1, .value = 0,},
	{.name = "psu_pwr2_reset", .direction = 1, .value = 0,},
	{.name = "psu1_green_led", .direction = 1, .value = 1,},
	{.name = "psu1_red_led",   .direction = 1, .value = 0,},
	{.name = "psu2_green_led", .direction = 1, .value = 1,},
	{.name = "psu2_red_led",   .direction = 1, .value = 0,},
	{.name = "fan_green_led",  .direction = 1, .value = 1,},
	{.name = "fan_red_led",    .direction = 1, .value = 0,},
};

static const char *ix1_gpio_26_names[PCA_9555_GPIO_COUNT];

static struct pca953x_platform_data ix1_gpio_26_data = {
	.gpio_base = IX1_GPIO_26_BASE,
	.names = ix1_gpio_26_names,
};

static struct ix1_gpio_pin_info ix1_gpio_23_info[PCA_9555_GPIO_COUNT] = {
	{.name = "fan0_board",},
	{.name = "fan1_board",},
	{.name = "fan2_board",},
	{.name = "fan3_board",},
	{.name = "power_good",},
	{.name = "qsfp_power_enable", .direction = 1, .value = 1,},
	{.name = "mac_reset",     .direction = 1, .value = 1,},
	{.name = "usb_reset",        .direction = 1, .value = 1,},
	{.name = "pca9554_int",},
	{.name = "mgmt_present",},
	{.name = "board_id_0",},
	{.name = "board_id_1",},
	{.name = "board_id_2",},
	{.name = "board_id_3",},
	{.name = "board_id_4",},
	{.name = "board_id_5",},
};

static const char *ix1_gpio_23_names[PCA_9555_GPIO_COUNT];

static struct pca953x_platform_data ix1_gpio_23_data = {
	.gpio_base = IX1_GPIO_23_BASE,
	.names = ix1_gpio_23_names,
};

static struct ix1_gpio_pin_info ix1_gpio_25_info[PCA_9555_GPIO_COUNT] = {
	{.name = "cpld135_int",},
	{.name = "cpld2_int",},
	{.name = "cpld4_int",},
	{.name = "bcm5461s_int",},

	{.name = "fan1_present",}, /* 0 is present */
	{.name = "fan2_present",},
	{.name = "fan3_present",},
	{.name = "fan4_present",},

	{.name = "fan1_f2b",}, /* 1 is F2B, 0 is B2F */
	{.name = "fan2_f2b",},
	{.name = "fan3_f2b",},
	{.name = "fan4_f2b",},

	{.name = "fan1_fail_led", .direction = 1, .value = 0,}, /* 1 is fail */
	{.name = "fan2_fail_led", .direction = 1, .value = 0,},
	{.name = "fan3_fail_led", .direction = 1, .value = 0,},
	{.name = "fan4_fail_led", .direction = 1, .value = 0,},
};

static const char *ix1_gpio_25_names[PCA_9555_GPIO_COUNT];
static struct pca953x_platform_data ix1_gpio_25_data = {
	.gpio_base = IX1_GPIO_25_BASE,
	.names = ix1_gpio_25_names,
};

/* I2C Device Map */
static struct ix1_device_info i2c_devices_level1[] = {
	/* CPU Board  */
	{
		 /* SPD DIMM EEPROM */
		.bus = IX1_I2C_I801_BUS,
		{I2C_BOARD_INFO("spd", 0x52),
		 .platform_data = &spd1_52_at24,},
	},
	{
		/* SPD2 DIMM EEPROM */
		.bus = IX1_I2C_I801_BUS,
		{I2C_BOARD_INFO("spd", 0x53),
		 .platform_data = &spd2_53_at24,},
	},
	{
		/* CPLD */
		/* TODO: Vidya */
		.bus = IX1_I2C_I801_BUS,
		{I2C_BOARD_INFO("dummy", 0x40),},
	},
	{
		/* ISL90728:DDR3 VREF TUNING */
		/* TODO: Vidya */
		.bus = IX1_I2C_I801_BUS,
		{I2C_BOARD_INFO("dummy", 0x3e),},
	},
	{
		/* PCA9546 */
		.bus = IX1_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9546", 0x71),
		 .platform_data = &ix1_data_pca9546_71,},
	},
	/* Mother Board  */
	{
		/* P1VSW PWM Controller */
		.bus = IX1_I2C_I801_BUS,
		{I2C_BOARD_INFO("dummy", 0x7e),},
	},
	{
		/* P1V0A PWM Controller */
		.bus = IX1_I2C_I801_BUS,
		{I2C_BOARD_INFO("dummy", 0x6e),},
	},
	{
		/* Thermal & Fans Controller */
		.bus = IX1_I2C_I801_BUS,
		{I2C_BOARD_INFO("CY8C3245R1", 0x4e),},
	},
	{
		/* PCA9546 - 1 */
		.bus = IX1_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9546", 0x77),
		 .platform_data = &ix1_data_pca9546_77,},
	},
	{
		/* PCA9546 - 2 */
		.bus = IX1_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9546", 0x72),
		 .platform_data = &ix1_data_pca9546_72,},
	},
};

static struct ix1_device_info i2c_devices_level2[] = {
	/* Following devices are on CPU Board */
	{
		/* IO Expander */
		.bus = IX1_PCA9546_71_0,
		{I2C_BOARD_INFO("pca9555", 0x20),
		.platform_data = &ix1_gpio_20_data},
	},
	{
		/* Switch FRU Data */
		.bus = IX1_PCA9546_71_1,
		{I2C_BOARD_INFO("24c02", 0x50),
		 .platform_data = &cpu_50_at24,},
	},
	{
		/* Clock Buffer */
		.bus = IX1_PCA9546_71_1,
		{I2C_BOARD_INFO("dummy", 0x6a),},
	},
	{
		/* IO Expander */
		/* TODO: Vidya */
		.bus = IX1_PCA9546_71_2,
		{I2C_BOARD_INFO("dummy", 0x21),},
		/*
		 * .platform_data = &ix1_gpio_21_data},
		 */
	},
	{
		/* CPU ID Identification */
		.bus = IX1_PCA9546_71_3,
		{I2C_BOARD_INFO("pca9555", 0x22),
		.platform_data = &ix1_gpio_22_data},
	},
	{
		/* Clock Generator */
		.bus = IX1_PCA9546_71_3,
		{I2C_BOARD_INFO("dummy", 0x69),},
	},
	/* Following devices are on MB */
	/* PCA9546-1 Addr:77 */
	{
		.bus = IX1_PCA9546_77_0,
		{I2C_BOARD_INFO("pca9548", 0x73),
		 .platform_data = &ix1_data_pca9548_0_73,},
		 .has_port = PORT_TYPE_QSFP28,
		 .port_base = 1,
		 .port_bus = IX1_PCA9548_0_73_0,
		 .num_ports = 8,
	},
	{
		.bus = IX1_PCA9546_77_0,
		{I2C_BOARD_INFO("pca9548", 0x74),
		 .platform_data = &ix1_data_pca9548_0_74,},
		 .has_port = PORT_TYPE_QSFP28,
		 .port_base = 9,
		 .port_bus = IX1_PCA9548_0_74_0,
		 .num_ports = 8,
	},
	{
		.bus = IX1_PCA9546_77_0,
		{I2C_BOARD_INFO("pca9548", 0x75),
		 .platform_data = &ix1_data_pca9548_0_75,},
		 .has_port = PORT_TYPE_QSFP28,
		 .port_base = 17,
		 .port_bus = IX1_PCA9548_0_75_0,
		 .num_ports = 8,
	},
	{
		/* I/O CPLD for QSFP28 1-16 */
		.bus = IX1_PCA9546_77_0,
		{I2C_BOARD_INFO("ix_rangeley_cpld", 0x38),
		 .platform_data = &cpld_2},
	},
	{
		/* I/O CPLD for QSFP28 17-32 */
		.bus = IX1_PCA9546_77_0,
		{I2C_BOARD_INFO("ix_rangeley_cpld", 0x39),
		 .platform_data = &cpld_4},
	},
	{
		.bus = IX1_PCA9546_77_1,
		{I2C_BOARD_INFO("pca9548", 0x76),
		 .platform_data = &ix1_data_pca9548_1_76,},
		 .has_port = PORT_TYPE_QSFP28,
		 .port_base = 25,
		 .port_bus = IX1_PCA9548_1_76_0,
		 .num_ports = 8,
	},
	{
		/* LED CPLD for QSFP28 1-16 */
		.bus = IX1_PCA9546_77_3,
		{I2C_BOARD_INFO("ix_rangeley_cpld", 0x38),
		 .platform_data = &cpld_1},
	},
	{
		/* LED CPLD for QSFP28 17-32 */
		.bus = IX1_PCA9546_77_3,
		{I2C_BOARD_INFO("ix_rangeley_cpld", 0x39),
		 .platform_data = &cpld_3},
	},
	{
		/* I2C Bus monitor CPLD for QSFP28 1-32 */
		.bus = IX1_PCA9546_77_3,
		{I2C_BOARD_INFO("ix_rangeley_cpld", 0x3F),
		 .platform_data = &cpld_5},
	},
	{
		.bus = IX1_PCA9546_72_0, /* PSU1 PMBUS */
		{I2C_BOARD_INFO("pmbus", 0x5f),},
	},
	{
		.bus = IX1_PCA9546_72_1, /* PSU2 PMBUS */
		{I2C_BOARD_INFO("pmbus", 0x59),},
	},
	{
		.bus = IX1_PCA9546_72_2,
		{I2C_BOARD_INFO("pca9555", 0x26),
		 .platform_data = &ix1_gpio_26_data},
	},
	{
		.bus = IX1_PCA9546_72_2,
		{I2C_BOARD_INFO("pca9555", 0x23),
		 .platform_data = &ix1_gpio_23_data},
	},
	{
		.bus = IX1_PCA9546_72_2,   /* Board EEPROM  */
		{I2C_BOARD_INFO("24c02", 0x54),
		 .platform_data = &board_54_at24,},
	},
	{
		.bus = IX1_PCA9546_72_3,
		{I2C_BOARD_INFO("pca9555", 0x25),
		 .platform_data = &ix1_gpio_25_data},
	},
};

static struct i2c_client *i2c_clients_level1[ARRAY_SIZE(i2c_devices_level1)];
static struct i2c_client *i2c_clients_level2[ARRAY_SIZE(i2c_devices_level2)];

static struct i2c_client *add_i2c_client(int bus,
					 struct i2c_board_info *board_info)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = i2c_get_adapter(bus);
	if (!adapter) {
		pr_err(DRIVER_NAME "could not get adapter %u\n", bus);
		return ERR_PTR(-ENODEV);
	}
	client = i2c_new_device(adapter, board_info);
	if (!client) {
		pr_err(DRIVER_NAME "could not add device\n");
		client = ERR_PTR(-ENODEV);
	}
	i2c_put_adapter(adapter);
	return client;
}

static int ix1_gpio_pins_init(struct ix1_gpio_pin_info *info,
			      int base, int count)
{
	struct ix1_gpio_pin_info *p;
	int i;
	int ret;
	int num;

	for (i = 0; i < count; i++) {
		p = &info[i];
		num = base + i;

		if (p->nc)
			continue;

		ret = gpio_request(num, NULL);
		if (ret) {
			pr_err(DRIVER_NAME "request for gpio pin %s (%u) failed\n",
			       p->name, num);
			return ret;
		}

		if (p->direction)
			ret = gpio_direction_output(num, p->value);
		else
			ret = gpio_direction_input(num);
		if (ret) {
			pr_err(DRIVER_NAME "unable to set direction on gpio pin %s (%u)\n",
			       p->name, num);
			return ret;
		}
		ret = gpio_export(num, false);
		if (ret) {
			pr_err(DRIVER_NAME "unable to export gpio pin %s (%u)\n",
			       p->name, num);
			return ret;
		}
	}
	return 0;
}

#define QSFP_LABEL_SIZE  8
static struct i2c_board_info *alloc_qsfp_board_info(int port)
{
	char *label;
	struct eeprom_platform_data *eeprom_data;
	struct sff_8436_platform_data *sff8436_data;
	struct i2c_board_info *board_info;

	label = kzalloc(QSFP_LABEL_SIZE, GFP_KERNEL);
	if (!label)
		goto err_exit;
	eeprom_data = kzalloc(sizeof(*eeprom_data), GFP_KERNEL);
	if (!eeprom_data)
		goto err_exit_eeprom;
	sff8436_data = kzalloc(sizeof(*sff8436_data), GFP_KERNEL);
	if (!sff8436_data)
		goto err_exit_sff8436;
	board_info = kzalloc(sizeof(*board_info), GFP_KERNEL);
	if (!board_info)
		goto err_exit_board;

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

static void free_qsfp_board_info(struct i2c_board_info *board_info)
{
	struct sff_8436_platform_data *sff8436_data = board_info->platform_data;
	struct eeprom_platform_data *eeprom_data = sff8436_data->eeprom_data;

	kfree(eeprom_data->label);
	kfree(eeprom_data);
	kfree(sff8436_data);
	kfree(board_info);
}

static int ix1_alloc_qsfp(int port, int bus, int num_ports)
{
	struct i2c_board_info *b_info;
	struct i2c_client *c;
	int j;

	for (j = 0; j < num_ports; j++) {
		b_info = alloc_qsfp_board_info(port + j);
		if (!b_info) {
			pr_err(DRIVER_NAME "could not allocate board info port: %d\n",
			       port + j);
			return -1;
		}
		c = add_i2c_client(bus + j, b_info);
		if (!c) {
			free_qsfp_board_info(b_info);
			pr_err(DRIVER_NAME "could not create i2c_client %s port: %d\n",
			       b_info->type, port + j);
			return -1;
		}
		ports_info[port + j - 1].type = 1;
		ports_info[port + j - 1].b = b_info;
		ports_info[port + j - 1].c = c;
	}
	return 0;
}

static void ix1_free_gpio(void)
{
	int i;

	for (i = ix1_gpio_20_data.gpio_base;
	    i < ix1_gpio_20_data.gpio_base + 16; i++)
		gpio_free(i);

	for (i = ix1_gpio_22_data.gpio_base;
	    i < ix1_gpio_22_data.gpio_base + 16; i++)
		gpio_free(i);

	for (i = ix1_gpio_25_data.gpio_base;
	    i < ix1_gpio_25_data.gpio_base + 16; i++)
		gpio_free(i);

	for (i = ix1_gpio_23_data.gpio_base;
	    i < ix1_gpio_23_data.gpio_base + 16; i++)
		gpio_free(i);

	for (i = ix1_gpio_26_data.gpio_base;
	    i < ix1_gpio_26_data.gpio_base + 16; i++)
		gpio_free(i);
}

static void ix1_free_ports(void)
{
	int i;

	for (i = 0; i < IX1_PORT_COUNT; i++) {
		if (ports_info[i].b)
			free_qsfp_board_info(ports_info[i].b);
		if (ports_info[i].c)
			i2c_unregister_device(ports_info[i].c);
	}
}

static void ix1_i2c_exit(void)
{
	int i;

	ix1_free_ports();

	for (i = 0; i < ARRAY_SIZE(i2c_devices_level2); i++)
		i2c_unregister_device(i2c_clients_level2[i]);
	for (i = 0; i < ARRAY_SIZE(i2c_devices_level1); i++)
		i2c_unregister_device(i2c_clients_level1[i]);
}

static int i2c_init(void)
{
	struct i2c_adapter *adapter;
	int ret = -1;
	int i801_bus = -1;
	int i, bus;

	/* Find the I801 bus */
	for (i = 0; i < 5; i++) {
		adapter = i2c_get_adapter(i);
		if (adapter) {
			if (!strncmp(adapter->name, SMB_I801_NAME,
				     strlen(SMB_I801_NAME)))
				i801_bus = i;
			i2c_put_adapter(adapter);
		}
		if (i801_bus >= 0)
			break;
	}
	if (i801_bus < 0) {
		pr_err(DRIVER_NAME "Unable to find %s\n", SMB_I801_NAME);
		return -ENXIO;
	}

	/* Instantiate I2C devices */
	for (i = 0; i < ARRAY_SIZE(i2c_devices_level1); i++) {
		if (i2c_devices_level1[i].bus == IX1_I2C_I801_BUS) {
			i2c_devices_level1[i].bus = i801_bus;
			i2c_clients_level1[i] = add_i2c_client(i801_bus,
						&i2c_devices_level1[i].info);
		}
		if (IS_ERR(i2c_clients_level1[i])) {
			ret = PTR_ERR(i2c_clients_level1[i]);
			goto err_exit;
		}
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices_level2); i++) {
		bus = i2c_devices_level2[i].bus;
		i2c_clients_level2[i] = add_i2c_client(bus,
					&i2c_devices_level2[i].info);
		if (IS_ERR(i2c_clients_level2[i])) {
			ret = PTR_ERR(i2c_clients_level2[i]);
			goto err_exit;
		}
		if (i2c_devices_level2[i].has_port == PORT_TYPE_QSFP28) {
			ix1_alloc_qsfp(i2c_devices_level2[i].port_base,
				       i2c_devices_level2[i].port_bus,
					     i2c_devices_level2[i].num_ports);
		}
	}

	return 0;
err_exit:
	ix1_i2c_exit();
	return -1;
}

static int init_gpio(void)
{
	int i = 0;

	/* Fill in the GPIO names */
	for (i = 0; i < PCA_9555_GPIO_COUNT; i++) {
		ix1_gpio_20_names[i] = ix1_gpio_20_info[i].name;
		ix1_gpio_22_names[i] = ix1_gpio_22_info[i].name;
		ix1_gpio_26_names[i] = ix1_gpio_26_info[i].name;
		ix1_gpio_23_names[i] = ix1_gpio_23_info[i].name;
		ix1_gpio_25_names[i] = ix1_gpio_25_info[i].name;
	}

	/* Instantiate GPIOs */
	ix1_gpio_pins_init(ix1_gpio_20_info, ix1_gpio_20_data.gpio_base,
			   PCA_9555_GPIO_COUNT);
	ix1_gpio_pins_init(ix1_gpio_22_info, ix1_gpio_22_data.gpio_base,
			   PCA_9555_GPIO_COUNT);
	ix1_gpio_pins_init(ix1_gpio_26_info, ix1_gpio_26_data.gpio_base,
			   PCA_9555_GPIO_COUNT);
	ix1_gpio_pins_init(ix1_gpio_23_info, ix1_gpio_23_data.gpio_base,
			   PCA_9555_GPIO_COUNT);
	ix1_gpio_pins_init(ix1_gpio_25_info, ix1_gpio_25_data.gpio_base,
			   PCA_9555_GPIO_COUNT);

	return 0;
}

static int ix1_platform_probe(struct platform_device *dev)
{
	int ret;

	ret = i2c_init();
	if (ret) {
		pr_err(DRIVER_NAME "I2C initialization failed\n");
		return ret;
	}

	ret = init_gpio();
	if (ret) {
		if (ret != -EPROBE_DEFER) {
			ix1_i2c_exit();
			dev_err(&dev->dev, "GPIO initialization failed (%d)\n",
				ret);
		} else {
			dev_info(&dev->dev, "GPIO initialization deferred\n");
		}

		return ret;
	}

	dev_info(&dev->dev, "ix1 probe succeeded \n");
	return 0;
}

static int ix1_platform_remove(struct platform_device *dev)
{
	ix1_free_gpio();
	ix1_i2c_exit();
	return 0;
}

static const struct platform_device_id ix1_platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, ix1_platform_id);

static struct platform_driver ix1_platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = ix1_platform_probe,
	.remove = ix1_platform_remove,
};

static struct platform_device *ix1_plat_device = NULL;

static int __init ix1_platform_init(void)
{
	int ret = 0;

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" initializing\n");

	if (!driver_find(ix1_platform_driver.driver.name, &platform_bus_type)) {
		ret = platform_driver_register(&ix1_platform_driver);
		if (ret) {
		    pr_err(DRIVER_NAME ": %s driver registration failed."
		       "(%d)\n", ix1_platform_driver.driver.name, ret);
		    return ret;
		}
	}

	if (ix1_plat_device == NULL) {
		ix1_plat_device = platform_device_register_simple(DRIVER_NAME, -1,
								  NULL, 0);
		if (IS_ERR(ix1_plat_device)) {
			ret = PTR_ERR(ix1_plat_device);
			ix1_plat_device = NULL;
			pr_err(DRIVER_NAME": Platform device registration"
					"failed. (%d)\n", ret);
			platform_driver_unregister(&ix1_platform_driver);
			return ret;
		}
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;
}

static void __exit ix1_platform_exit(void)
{
	platform_device_unregister(ix1_plat_device);
	ix1_plat_device = NULL;
	platform_driver_unregister(&ix1_platform_driver);
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " driver unloaded\n");
}


module_init(ix1_platform_init);
module_exit(ix1_platform_exit);

MODULE_AUTHOR("Vidya Sagar Ravipati (vidya@cumulusnetworks.com)");
MODULE_DESCRIPTION("Quanta IX1 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
