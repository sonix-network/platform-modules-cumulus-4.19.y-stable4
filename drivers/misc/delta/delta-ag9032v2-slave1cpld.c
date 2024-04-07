// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AG9032v2 Slave1 CPLD Driver
 *
 * Copyright (C) 2020 Cumulus Networks, Inc.  All Rights Reserved.
 * Author: dhyen <dhyen@cumulusnetworks.com>
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

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "delta-ag9032v2.h"

#define DRIVER_NAME    AG9032V2_SLAVE1CPLD_NAME
#define DRIVER_VERSION "1.1"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* CPLD register bitfields with enum-like values */

/* CPLD registers */

cpld_bf_ro(slave1_revision, DELTA_AG9032V2_SLAVE1_REVISION_REG,
	   DELTA_AG9032V2_SLAVE1_REVISION, NULL, 0);
cpld_bt_ro(sfp_p0_present, DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_REG,
	   DELTA_AG9032V2_SFP_P0_MOD_ABS, NULL, BF_COMPLEMENT);
cpld_bt_ro(sfp_p0_rxlos, DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_REG,
	   DELTA_AG9032V2_SFP_P0_RXLOS, NULL, BF_COMPLEMENT);
cpld_bt_rw(sfp_p0_tx_disable, DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_REG,
	   DELTA_AG9032V2_SFP_P0_TX_DIS, NULL, 0);
cpld_bt_ro(sfp_p0_tx_fault, DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_REG,
	   DELTA_AG9032V2_SFP_P0_TXFAULT, NULL, BF_COMPLEMENT);
cpld_bt_ro(sfp_p1_present, DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_REG,
	   DELTA_AG9032V2_SFP_P1_MOD_ABS, NULL, BF_COMPLEMENT);
cpld_bt_ro(sfp_p1_rxlos, DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_REG,
	   DELTA_AG9032V2_SFP_P1_RXLOS, NULL, BF_COMPLEMENT);
cpld_bt_rw(sfp_p1_tx_disable, DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_REG,
	   DELTA_AG9032V2_SFP_P1_TX_DIS, NULL, 0);
cpld_bt_ro(sfp_p1_tx_fault, DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_REG,
	   DELTA_AG9032V2_SFP_P1_TXFAULT, NULL, BF_COMPLEMENT);
cpld_bt_rw(sfp_p0_present_mask, DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_MASK_REG,
	   DELTA_AG9032V2_SFP_P0_MOD_ABS_MASK, NULL, BF_COMPLEMENT);
cpld_bt_rw(sfp_p1_present_mask, DELTA_AG9032V2_SLAVE1_MODULE_SIGNAL_MASK_REG,
	   DELTA_AG9032V2_SFP_P1_MOD_ABS_MASK, NULL, BF_COMPLEMENT);
cpld_bt_rw(sled_clk0_b2a_p2, DELTA_AG9032V2_SLAVE1_MAC_LED_REG,
	   DELTA_AG9032V2_SLED_CLK0_B2A_P2, NULL, 0);
cpld_bt_rw(sled_clk1_b2a_p2, DELTA_AG9032V2_SLAVE1_MAC_LED_REG,
	   DELTA_AG9032V2_SLED_CLK1_B2A_P2, NULL, 0);
cpld_bt_rw(sled_dat0_b2a_p2, DELTA_AG9032V2_SLAVE1_MAC_LED_REG,
	   DELTA_AG9032V2_SLED_DAT0_B2A_P2, NULL, 0);
cpld_bt_rw(sled_dat1_b2a_p2, DELTA_AG9032V2_SLAVE1_MAC_LED_REG,
	   DELTA_AG9032V2_SLED_DAT1_B2A_P2, NULL, 0);
cpld_bt_rw(tled4_b_p2, DELTA_AG9032V2_SLAVE1_MAC_LED_REG,
	   DELTA_AG9032V2_TLED4_B_P2, NULL, BF_COMPLEMENT);
cpld_bt_rw(tled4_g_p2, DELTA_AG9032V2_SLAVE1_MAC_LED_REG,
	   DELTA_AG9032V2_TLED4_G_P2, NULL, BF_COMPLEMENT);
cpld_bt_rw(tled4_r_p2, DELTA_AG9032V2_SLAVE1_MAC_LED_REG,
	   DELTA_AG9032V2_TLED4_R_P2, NULL, BF_COMPLEMENT);
cpld_bt_rw(tled5_p2, DELTA_AG9032V2_SLAVE1_MAC_LED_REG,
	   DELTA_AG9032V2_TLED5_P2, NULL, BF_COMPLEMENT);
cpld_bt_rw(sfp_p0_act_led, DELTA_AG9032V2_SLAVE1_LED_REG,
	   DELTA_AG9032V2_SFP_P0_ACT_LED, NULL, BF_COMPLEMENT);
cpld_bt_rw(sfp_p0_led_a, DELTA_AG9032V2_SLAVE1_LED_REG,
	   DELTA_AG9032V2_SFP_P0_LED_A, NULL, BF_COMPLEMENT);
cpld_bt_rw(sfp_p0_led_g, DELTA_AG9032V2_SLAVE1_LED_REG,
	   DELTA_AG9032V2_SFP_P0_LED_G, NULL, BF_COMPLEMENT);
cpld_bt_rw(sfp_p1_act_led, DELTA_AG9032V2_SLAVE1_LED_REG,
	   DELTA_AG9032V2_SFP_P1_ACT_LED, NULL, BF_COMPLEMENT);
cpld_bt_rw(sfp_p1_led_a, DELTA_AG9032V2_SLAVE1_LED_REG,
	   DELTA_AG9032V2_SFP_P1_LED_A, NULL, BF_COMPLEMENT);
cpld_bt_rw(sfp_p1_led_g, DELTA_AG9032V2_SLAVE1_LED_REG,
	   DELTA_AG9032V2_SFP_P1_LED_G, NULL, BF_COMPLEMENT);
cpld_bf_ro(board_type,
	   DELTA_AG9032V2_SLAVE1_RESET_AND_HARDWARE_REVISION_REG,
	   DELTA_AG9032V2_BOARD_STAGE_0_3, NULL, 0);
cpld_bt_ro(cpld_gpio_rst_en,
	   DELTA_AG9032V2_SLAVE1_RESET_AND_HARDWARE_REVISION_REG,
	   DELTA_AG9032V2_S1_CPLD_GPIO_RST_EN, NULL, 0);
cpld_bt_rw(global_reset,
	   DELTA_AG9032V2_SLAVE1_RESET_AND_HARDWARE_REVISION_REG,
	   DELTA_AG9032V2_GLOBAL_RESET, NULL, 0);
cpld_bt_rw(clk_156p25m_sel,
	   DELTA_AG9032V2_SLAVE1_RESET_AND_HARDWARE_REVISION_REG,
	   DELTA_AG9032V2_156P25M_CLK_SEL, NULL, 0);
cpld_bt_rw(cpld_ab_gpio4, DELTA_AG9032V2_SLAVE1_GPIO1_REG,
	   DELTA_AG9032V2_S1_CPLD_AB_GPIO4, NULL, 0);
cpld_bt_rw(cpld_ab_gpio3, DELTA_AG9032V2_SLAVE1_GPIO1_REG,
	   DELTA_AG9032V2_S1_CPLD_AB_GPIO3, NULL, 0);
cpld_bt_rw(pld1_pld2_a, DELTA_AG9032V2_SLAVE1_GPIO1_REG,
	   DELTA_AG9032V2_S1_PLD1_PLD2_A, NULL, 0);
cpld_bt_rw(gpio_p1_p2_a, DELTA_AG9032V2_SLAVE1_GPIO1_REG,
	   DELTA_AG9032V2_S1_GPIO_P1_P2_A, NULL, 0);
cpld_bt_rw(pld1_pld2_b, DELTA_AG9032V2_SLAVE1_GPIO1_REG,
	   DELTA_AG9032V2_S1_PLD1_PLD2_B, NULL, 0);
cpld_bt_rw(cpld_a_cpld_gpio2, DELTA_AG9032V2_SLAVE1_GPIO1_REG,
	   DELTA_AG9032V2_S1_CPLD_A_CPLD_GPIO2, NULL, 0);
cpld_bt_rw(cpu_cpld2_gpio2_p2, DELTA_AG9032V2_SLAVE1_GPIO1_REG,
	   DELTA_AG9032V2_S1_CPU_CPLD2_GPIO2_P2, NULL, 0);
cpld_bt_rw(cpld_a_cpld_gpio3, DELTA_AG9032V2_SLAVE1_GPIO1_REG,
	   DELTA_AG9032V2_S1_CPLD_A_CPLD_GPIO3, NULL, 0);
cpld_bt_rw(cpld_a_cpld_gpio1_p2, DELTA_AG9032V2_SLAVE1_GPIO2_REG,
	   DELTA_AG9032V2_S1_CPLD_A_CPLD_GPIO1_P2, NULL, 0);
cpld_bt_rw(cpld_ab_gpio2_p2, DELTA_AG9032V2_SLAVE1_GPIO2_REG,
	   DELTA_AG9032V2_S1_CPLD_AB_GPIO2_P2, NULL, 0);
cpld_bt_rw(cpld_ab_gpio1_p2, DELTA_AG9032V2_SLAVE1_GPIO2_REG,
	   DELTA_AG9032V2_S1_CPLD_AB_GPIO1_P2, NULL, 0);
cpld_bt_rw(gpioh4, DELTA_AG9032V2_SLAVE1_GPIO2_REG,
	   DELTA_AG9032V2_S1_GPIOH4, NULL, 0);
cpld_bt_rw(gpioh5, DELTA_AG9032V2_SLAVE1_GPIO2_REG,
	   DELTA_AG9032V2_S1_GPIOH5, NULL, 0);
cpld_bt_rw(pld2_pld3_a, DELTA_AG9032V2_SLAVE1_GPIO2_REG,
	   DELTA_AG9032V2_S1_PLD2_PLD3_A, NULL, 0);
cpld_bt_rw(pld2_pld3_b, DELTA_AG9032V2_SLAVE1_GPIO2_REG,
	   DELTA_AG9032V2_S1_PLD2_PLD3_B, NULL, 0);
cpld_bt_rw(pld2_pld3_c, DELTA_AG9032V2_SLAVE1_GPIO2_REG,
	   DELTA_AG9032V2_S1_PLD2_PLD3_C, NULL, 0);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_slave1_revision.attr,
	&cpld_sfp_p0_present.attr,
	&cpld_sfp_p0_rxlos.attr,
	&cpld_sfp_p0_tx_disable.attr,
	&cpld_sfp_p0_tx_fault.attr,
	&cpld_sfp_p1_present.attr,
	&cpld_sfp_p1_rxlos.attr,
	&cpld_sfp_p1_tx_disable.attr,
	&cpld_sfp_p1_tx_fault.attr,
	&cpld_sfp_p0_present_mask.attr,
	&cpld_sfp_p1_present_mask.attr,
	&cpld_sled_clk0_b2a_p2.attr,
	&cpld_sled_clk1_b2a_p2.attr,
	&cpld_sled_dat0_b2a_p2.attr,
	&cpld_sled_dat1_b2a_p2.attr,
	&cpld_tled4_b_p2.attr,
	&cpld_tled4_g_p2.attr,
	&cpld_tled4_r_p2.attr,
	&cpld_tled5_p2.attr,
	&cpld_sfp_p0_act_led.attr,
	&cpld_sfp_p0_led_a.attr,
	&cpld_sfp_p0_led_g.attr,
	&cpld_sfp_p1_act_led.attr,
	&cpld_sfp_p1_led_a.attr,
	&cpld_sfp_p1_led_g.attr,
	&cpld_board_type.attr,
	&cpld_cpld_gpio_rst_en.attr,
	&cpld_global_reset.attr,
	&cpld_clk_156p25m_sel.attr,
	&cpld_cpld_ab_gpio4.attr,
	&cpld_cpld_ab_gpio3.attr,
	&cpld_pld1_pld2_a.attr,
	&cpld_gpio_p1_p2_a.attr,
	&cpld_pld1_pld2_b.attr,
	&cpld_cpld_a_cpld_gpio2.attr,
	&cpld_cpu_cpld2_gpio2_p2.attr,
	&cpld_cpld_a_cpld_gpio3.attr,
	&cpld_cpld_a_cpld_gpio1_p2.attr,
	&cpld_cpld_ab_gpio2_p2.attr,
	&cpld_cpld_ab_gpio1_p2.attr,
	&cpld_gpioh4.attr,
	&cpld_gpioh5.attr,
	&cpld_pld2_pld3_a.attr,
	&cpld_pld2_pld3_b.attr,
	&cpld_pld2_pld3_c.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

static int cpld_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	s32 temp;
	int ret = 0;

	/* make sure the adpater supports i2c smbus reads */
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(dev, "adapter does not support I2C_FUNC_SMBUS_BYTE_DATA\n");
		ret = -EINVAL;
		goto err;
	}

	/* probe the hardware by reading the version register */
	temp = i2c_smbus_read_byte_data(client,
					DELTA_AG9032V2_CPLD_REVISION_REG);
	if (temp < 0) {
		dev_err(dev, "read Master CPLD version register error: %d\n",
			temp);
		ret = temp;
		goto err;
	}

	/* create sysfs node */
	ret = sysfs_create_group(&dev->kobj, &cpld_attr_group);
	if (ret) {
		dev_err(dev, "failed to create sysfs group for cpld device\n");
		goto err;
	}

	/* all clear */
	dev_info(dev, "device probed, Master CPLD rev: %lu\n",
		 GET_FIELD(temp, DELTA_AG9032V2_CPLD_REVISION));

err:
	return ret;
}

static int cpld_remove(struct i2c_client *client)
{
	struct device *dev = &client->dev;

	sysfs_remove_group(&dev->kobj, &cpld_attr_group);
	return 0;
}

static const struct i2c_device_id cpld_id[] = {
	{ DRIVER_NAME, 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, cpld_id);

static struct i2c_driver cpld_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe_new = cpld_probe,
	.remove = cpld_remove,
	.id_table = cpld_id,
};

/* module init/exit */

static int __init cpld_init(void)
{
	int ret;

	ret = i2c_add_driver(&cpld_driver);
	if (ret) {
		pr_err(DRIVER_NAME ": driver failed to load\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;
}

static void __exit cpld_exit(void)
{
	i2c_del_driver(&cpld_driver);
	pr_info(DRIVER_NAME ": unloaded\n");
}

module_init(cpld_init);
module_exit(cpld_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Delta AG9032v2 Slave2 CPLD Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
