/*
 * quanta_ly6_platform.c - Quanta LY6 Platform Support
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

#define DRIVER_NAME	"quanta_ly6_platform"
#define DRIVER_VERSION	"1.1"

#define LY6_I2C_I801_BUS 0
#define LY6_PORT_COUNT 32

static struct platform_driver ly6_platform_driver;

mk_eeprom(spd1,  52, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(cpu, 53, 256, AT24_FLAG_IRUGO);
mk_eeprom(board, 54, 256, AT24_FLAG_IRUGO);

struct ly6_device_info {
	int bus;
	struct i2c_board_info info;
	int has_port;  /* 1 for SFP, 2 for QSFP */
	int port_base;
	int port_bus;
};

struct port_info {
	int type; /* 0 = SFP , 1 = QSFP */
	struct i2c_board_info *b;
	struct i2c_client *c;
};
static struct port_info ports_info[LY6_PORT_COUNT];

/*  I2C Muxes  */
#define ly6_pca9546(addr, busno)					\
	enum {								\
		LY6_PCA9546_##addr##_0 = busno,				\
		LY6_PCA9546_##addr##_1,					\
		LY6_PCA9546_##addr##_2,					\
		LY6_PCA9546_##addr##_3,					\
	};								\
	static struct pca954x_platform_mode ly6_mode_pca9546_##addr [] = { \
		{ .adap_id = LY6_PCA9546_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = LY6_PCA9546_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = LY6_PCA9546_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = LY6_PCA9546_##addr##_3, .deselect_on_exit = 1,},			\
	};								\
	static struct pca954x_platform_data ly6_data_pca9546_##addr = { \
		.modes = ly6_mode_pca9546_##addr,			\
		.num_modes = ARRAY_SIZE(ly6_mode_pca9546_##addr),	\
	};


#define ly6_pca9548(addr, busno)					\
	enum {								\
		LY6_PCA9548_##addr##_0 = busno,				\
		LY6_PCA9548_##addr##_1,					\
		LY6_PCA9548_##addr##_2,					\
		LY6_PCA9548_##addr##_3,					\
		LY6_PCA9548_##addr##_4,					\
		LY6_PCA9548_##addr##_5,					\
		LY6_PCA9548_##addr##_6,					\
		LY6_PCA9548_##addr##_7,					\
	};								\
	static struct pca954x_platform_mode ly6_mode_pca9548_##addr [] = { \
		{ .adap_id = LY6_PCA9548_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = LY6_PCA9548_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = LY6_PCA9548_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = LY6_PCA9548_##addr##_3, .deselect_on_exit = 1,}, \
		{ .adap_id = LY6_PCA9548_##addr##_4, .deselect_on_exit = 1,}, \
		{ .adap_id = LY6_PCA9548_##addr##_5, .deselect_on_exit = 1,}, \
		{ .adap_id = LY6_PCA9548_##addr##_6, .deselect_on_exit = 1,}, \
		{ .adap_id = LY6_PCA9548_##addr##_7, .deselect_on_exit = 1,}, \
	};								\
	static struct pca954x_platform_data ly6_data_pca9548_##addr = { \
		.modes = ly6_mode_pca9548_##addr,			\
		.num_modes = ARRAY_SIZE(ly6_mode_pca9548_##addr),	\
	};

/* I2C I801 -> PCA9546(0x71) */
ly6_pca9546(71, 10);
/* I2C I801 -> PCA9548_1(0x77) */
ly6_pca9548(77, 14)
/* I2C I801 -> PCA9548_1(0x77) -> PCA9548_2 (0x73) */
ly6_pca9548(73, 22)
/* I2C I801 -> PCA9548_1(0x77) -> PCA9548_2 (0x74) */
ly6_pca9548(74, 30)
/* I2C I801 -> PCA9548_1(0x77) -> PCA9548_3 (0x75) */
ly6_pca9548(75, 38)
/* I2C I801 -> PCA9548_1(0x77) -> PCA9548_3 (0x76) */
ly6_pca9548(76, 46)
/* I2C I801 -> PCA9546(0x71) - > PCA9546(0x72) */
ly6_pca9546(72, 54);

/* GPIOS */
#define LY6_GPIO_BASE 100
#define LY6_GPIO_20_BASE LY6_GPIO_BASE
#define LY6_GPIO_24_BASE (LY6_GPIO_20_BASE + PCA_9555_GPIO_COUNT)
#define LY6_GPIO_23_BASE (LY6_GPIO_24_BASE + PCA_9555_GPIO_COUNT)
#define LY6_GPIO_26_BASE (LY6_GPIO_23_BASE + PCA_9555_GPIO_COUNT)

mk_gpio_pins(gpio_20) = {
	mk_gpio_pin(0,    mb_i2c_rst,         GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(1,    mb_led_rst,         GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(2,    mb_eth_rst,         GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(3,    mb_sw_rst,          GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(4,    mb_odd_phy_rst,     GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(5,    mb_even_phy_rst,    GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(6,    alta_rst,           GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(7,    usb_rst,            GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,    alta_power,         GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(9,    i2c_mac_switch,     GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(10,    boot_led,          GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(11,    sys_led,           GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(12,    board_pwr_led,     GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(13,    nc_10,             GPIOF_DIR_IN),
	mk_gpio_pin(14,    nc_11,             GPIOF_DIR_IN),
	mk_gpio_pin(15,    nc_12,             GPIOF_DIR_IN),

};

mk_gpio_pins(gpio_24) = {
	mk_gpio_pin(0,      cpld2_int,        GPIOF_DIR_IN),
	mk_gpio_pin(1,      cpld3_int,        GPIOF_DIR_IN),
	mk_gpio_pin(2,      9555_4_int,       GPIOF_DIR_IN),
	mk_gpio_pin(3,      bcm5461s_int,     GPIOF_DIR_IN),
	mk_gpio_pin(4,      fan1_present,     GPIOF_DIR_IN),
	mk_gpio_pin(5,      fan2_present,     GPIOF_DIR_IN),
	mk_gpio_pin(6,      fan3_present,     GPIOF_DIR_IN),
	mk_gpio_pin(7,      fan4_present,     GPIOF_DIR_IN),
	mk_gpio_pin(8,      fan1_f2b,         GPIOF_DIR_IN),
	mk_gpio_pin(9,      fan2_f2b,         GPIOF_DIR_IN),
	mk_gpio_pin(10,     fan3_f2b,         GPIOF_DIR_IN),
	mk_gpio_pin(11,     fan4_f2b,         GPIOF_DIR_IN),
	mk_gpio_pin(12,     fan1_fail_led,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(13,     fan2_fail_led,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14,     fan3_fail_led,    GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(15,     fan4_fail_led,    GPIOF_OUT_INIT_LOW),

};

mk_gpio_pins(gpio_23) = {
	mk_gpio_pin(0,      fan0_board,        GPIOF_DIR_IN),
	mk_gpio_pin(1,      fan1_board,        GPIOF_DIR_IN),
	mk_gpio_pin(2,      fan2_board,        GPIOF_DIR_IN),
	mk_gpio_pin(3,      fan3_board,        GPIOF_DIR_IN),
	mk_gpio_pin(4,      qsfp_power_good,   GPIOF_DIR_IN),
	mk_gpio_pin(5,      nc_1,              GPIOF_DIR_IN),
	mk_gpio_pin(6,      nc_2,              GPIOF_DIR_IN),
	mk_gpio_pin(7,      mac_rst,           GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(8,      qsfp_pwr_enable,   GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(9,      usb_reset,         GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(10,     board_id_0,        GPIOF_DIR_IN),
	mk_gpio_pin(11,     board_id_1,        GPIOF_DIR_IN),
	mk_gpio_pin(12,     board_id_2,        GPIOF_DIR_IN),
	mk_gpio_pin(13,     board_id_3,        GPIOF_DIR_IN),
	mk_gpio_pin(14,     board_id_4,        GPIOF_DIR_IN),
	mk_gpio_pin(15,     board_id_5,        GPIOF_DIR_IN),
};

mk_gpio_pins(gpio_26) = {
	mk_gpio_pin(0,      psu_pwr1_present,      GPIOF_DIR_IN),
	mk_gpio_pin(1,      psu_pwr1_dc_ok,        GPIOF_DIR_IN),
	mk_gpio_pin(2,      psu_pwr1_int,          GPIOF_DIR_IN),
	mk_gpio_pin(3,      psu_pwr2_present,      GPIOF_DIR_IN),
	mk_gpio_pin(4,      psu_pwr2_dc_ok,        GPIOF_DIR_IN),
	mk_gpio_pin(5,      psu_pwr2_int,          GPIOF_DIR_IN),
	mk_gpio_pin(6,      psu_pwr1_ac_ok,        GPIOF_DIR_IN),
	mk_gpio_pin(7,      psu_pwr2_ac_ok,        GPIOF_DIR_IN),
	mk_gpio_pin(8,      psu_pwr1_reset,        GPIOF_DIR_IN),
	mk_gpio_pin(9,      psu_pwr2_reset,        GPIOF_DIR_IN),
	mk_gpio_pin(10,     psu1_green_led,        GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(11,     psu1_red_led,          GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(12,     psu2_green_led,        GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(13,     psu2_red_led,          GPIOF_OUT_INIT_LOW),
	mk_gpio_pin(14,     fan_green_led,         GPIOF_OUT_INIT_HIGH),
	mk_gpio_pin(15,     fan_red_led,           GPIOF_OUT_INIT_LOW),
};

mk_gpio_platform_data(gpio_20, LY6_GPIO_20_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_20_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_24, LY6_GPIO_24_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_24_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_23, LY6_GPIO_23_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_23_pins, init_gpio_pins, free_gpio_pins);
mk_gpio_platform_data(gpio_26, LY6_GPIO_26_BASE,   PCA_9555_GPIO_COUNT,
		      &gpio_26_pins, init_gpio_pins, free_gpio_pins);

struct quanta_ly6_rangeley_platform_data {
	int idx;
};

static struct quanta_ly6_rangeley_platform_data cpld_1 = {
	.idx = 1,
};

static struct quanta_ly6_rangeley_platform_data cpld_2 = {
	.idx = 2,
};

/* I2C Device Map */
static struct ly6_device_info i2c_devices_level1[] = {
	{
		.bus = LY6_I2C_I801_BUS,       /* SPD DIMM EEPROM */
		{I2C_BOARD_INFO("spd", 0x52),
		 .platform_data = &spd1_52_at24,},
	},
	{
		.bus = LY6_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9546", 0x71),
		 .platform_data = &ly6_data_pca9546_71,},
	},
	{
		.bus = LY6_I2C_I801_BUS,     /* Sensors Chip */
		{I2C_BOARD_INFO("CY8C3245R1", 0x4e),},
	},
	{
		.bus = LY6_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9548", 0x77),
		 .platform_data = &ly6_data_pca9548_77,},
	},
	{
		.bus = LY6_I2C_I801_BUS,
		{I2C_BOARD_INFO("pca9546", 0x72),
		 .platform_data = &ly6_data_pca9546_72,},
	},
};

static struct ly6_device_info i2c_devices_level2[] = {
	/* Following devices are on CPU Board */
	{
		.bus = LY6_PCA9546_71_0,
		{I2C_BOARD_INFO("pca9555", 0x20),
		.platform_data = &gpio_20_platform_data},
	},
	{
		.bus = LY6_PCA9546_71_1,
		{I2C_BOARD_INFO("24c02", 0x53),
		 .platform_data = &cpu_53_at24,},
	},
	{
		.bus = LY6_PCA9548_77_0,
		{I2C_BOARD_INFO("pca9548", 0x73),
		 .platform_data = &ly6_data_pca9548_73,},
		 .has_port = 2, .port_base = 1,
		 .port_bus = LY6_PCA9548_73_0,
	},
	{
		.bus = LY6_PCA9548_77_1,
		{I2C_BOARD_INFO("pca9548", 0x74),
		 .platform_data = &ly6_data_pca9548_74,},
		 .has_port = 2, .port_base = 9,
		 .port_bus = LY6_PCA9548_74_0,
	},
	{
		.bus = LY6_PCA9548_77_2,
		{I2C_BOARD_INFO("pca9548", 0x75),
		 .platform_data = &ly6_data_pca9548_75,},
		 .has_port = 2, .port_base = 17,
		 .port_bus = LY6_PCA9548_75_0,
	},
	{
		.bus = LY6_PCA9548_77_3,
		{I2C_BOARD_INFO("pca9548", 0x76),
		 .platform_data = &ly6_data_pca9548_76,},
		 .has_port = 2, .port_base = 25,
		 .port_bus = LY6_PCA9548_76_0,
	},
	{
	        .bus = LY6_PCA9548_77_4,
		{I2C_BOARD_INFO("ly6_rangeley_cpld", 0x3a),
		 .platform_data = &cpld_1},
	},
	{
	        .bus = LY6_PCA9548_77_5,
		{I2C_BOARD_INFO("ly6_rangeley_cpld", 0x3b),
		 .platform_data = &cpld_2},
	},
	{
		.bus = LY6_PCA9548_77_6,
		{I2C_BOARD_INFO("pca9555", 0x24),
		 .platform_data = &gpio_24_platform_data},
	},
	{
		.bus = LY6_PCA9548_77_7,
		{I2C_BOARD_INFO("pca9555", 0x23),
		 .platform_data = &gpio_23_platform_data},
	},
	{
		.bus = LY6_PCA9546_72_0, /* PSU1 PMBUS */
		{I2C_BOARD_INFO("ps2471", 0x58),},
	},
	{
		.bus = LY6_PCA9546_72_1, /* PSU2 PMBUS */
		{I2C_BOARD_INFO("ps2471", 0x59),},
	},
	{
		.bus = LY6_PCA9546_72_2,
		{I2C_BOARD_INFO("pca9555", 0x26),
		.platform_data = &gpio_26_platform_data},
	},
	{
		.bus = LY6_PCA9546_72_3,   /* Board EEPROM  */
		{I2C_BOARD_INFO("24c02", 0x54),
		 .platform_data = &board_54_at24,},
	},

};

static struct i2c_client *i2c_clients_level1[ARRAY_SIZE(i2c_devices_level1)];
static struct i2c_client *i2c_clients_level2[ARRAY_SIZE(i2c_devices_level2)];

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

static void free_qsfp_board_info(struct i2c_board_info *board_info)
{
	struct sff_8436_platform_data *sff8436_data = board_info->platform_data;
	struct eeprom_platform_data *eeprom_data = sff8436_data->eeprom_data;

	kfree(eeprom_data->label);
	kfree(eeprom_data);
	kfree(sff8436_data);
	kfree(board_info);
}

static int ly6_alloc_qsfp(int port, int bus) {
	struct i2c_board_info *b_info;
	struct i2c_client *c;
	int j;

	for (j = 0; j < 8; j++) {
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
	int i = 0;


	for(i = 0; i < LY6_PORT_COUNT; i++) {
		if (ports_info[i].b) {
			free_qsfp_board_info(ports_info[i].b);
			ports_info[i].b = NULL;
		}
		if (ports_info[i].c) {
			i2c_unregister_device(ports_info[i].c);
			ports_info[i].c = NULL;
		}
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
	int i;

	i801_bus = cumulus_i2c_find_adapter(SMB_I801_NAME);
	if (i801_bus < 0) {
		pr_err("Unable to find %s\n", SMB_I801_NAME);
		return -ENXIO;
	}

	/* Instantiate I2C devices */
	for (i = 0; i < ARRAY_SIZE(i2c_devices_level1); i++) {
		if (i2c_devices_level1[i].bus == LY6_I2C_I801_BUS)
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
			ly6_alloc_qsfp(i2c_devices_level2[i].port_base,
				       i2c_devices_level2[i].port_bus);
		}

	}

	pr_debug(DRIVER_NAME "LY6: I2C Init succeeded \n");
	return 0;
err_exit:
	 pr_err(DRIVER_NAME "LY6: I2C Init failed \n");
	free_i2c_devices();
	return ret;
}

static int ly6_i2c_init(void)
{
	int ret = 0;

	init_gpio_platform_data(gpio_20_pins, ARRAY_SIZE(gpio_20_pins),
				&gpio_20_platform_data);
	init_gpio_platform_data(gpio_24_pins, ARRAY_SIZE(gpio_24_pins),
				&gpio_24_platform_data);
	init_gpio_platform_data(gpio_23_pins, ARRAY_SIZE(gpio_23_pins),
				&gpio_23_platform_data);
	init_gpio_platform_data(gpio_26_pins, ARRAY_SIZE(gpio_26_pins),
				&gpio_26_platform_data);

	/*
	 * Error handling is already done as part of init_i2c_devices;
	 */
	ret = init_i2c_devices();

	return ret;

}

static void ly6_i2c_exit(void)
{
	free_i2c_devices();
}

static int ly6_platform_probe(struct platform_device *dev)
{
	int ret = 0;

	pr_info(DRIVER_NAME ": Probe begin \n");
	ret = ly6_i2c_init();
	if (ret) {
		goto err_exit;
	}

	pr_info(DRIVER_NAME ": Probe Succeeded \n");
	return 0;

err_exit:
	if (ret != -EPROBE_DEFER) {
		dev_info(&dev->dev, "error during probe, deleting clients\n");
		ly6_i2c_exit();
	}

	return ret;
}

static int ly6_platform_remove(struct platform_device *dev)
{
	ly6_i2c_exit();
	return 0;
}

static const struct platform_device_id ly6_platform_id[] = {
	{ DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(platform, ly6_platform_id);

static struct platform_driver ly6_platform_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = ly6_platform_probe,
	.remove = ly6_platform_remove,
};

static struct platform_device *ly6_plat_device = NULL;

static int __init ly6_platform_init(void)
{
	int ret = -1;

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" initializing\n");

	if (!driver_find(ly6_platform_driver.driver.name,
			 &platform_bus_type)) {
		ret = platform_driver_register(&ly6_platform_driver);
		if (ret) {
			pr_err(DRIVER_NAME ": %s driver registration failed"
			       " with %d  \n", ly6_platform_driver.driver.name,
			       ret);
			return ret;
		}
	}

	if (ly6_plat_device == NULL) {
		ly6_plat_device = platform_device_register_simple(DRIVER_NAME,
								  -1,
								  NULL,
								  0);
		if (IS_ERR(ly6_plat_device)) {
			ret = PTR_ERR(ly6_plat_device);
			ly6_plat_device = NULL;
			pr_err(DRIVER_NAME ": Platform device registration"
			       " failed \n");
			return ret;
		}
	}

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;
}

static void __exit ly6_platform_exit(void)
{
	platform_device_unregister(ly6_plat_device);
	ly6_plat_device = NULL;
	platform_driver_unregister(&ly6_platform_driver);
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " driver unloaded\n");
}
module_init(ly6_platform_init);
module_exit(ly6_platform_exit);

MODULE_AUTHOR("Puneet Shenoy (puneet@cumulusnetworks.com)");
MODULE_DESCRIPTION("Quanta LY6 Rangeley Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
