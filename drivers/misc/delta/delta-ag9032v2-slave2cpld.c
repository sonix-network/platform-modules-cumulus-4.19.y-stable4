// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AG9032v2 Slave2 CPLD Driver
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

#define DRIVER_NAME    AG9032V2_SLAVE2CPLD_NAME
#define DRIVER_VERSION "1.1"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* CPLD register bitfields with enum-like values */

/* CPLD registers */

cpld_bf_ro(slave2_revision, DELTA_AG9032V2_SLAVE2_REVISION_REG,
	   DELTA_AG9032V2_SLAVE2_REVISION, NULL, 0);
cpld_bt_rw(54616s_led1, DELTA_AG9032V2_SLAVE2_PHY_AND_DEBUG_LED_REG,
	   DELTA_AG9032V2_54616S_LED1, NULL, BF_COMPLEMENT);
cpld_bt_rw(54616s_led2, DELTA_AG9032V2_SLAVE2_PHY_AND_DEBUG_LED_REG,
	   DELTA_AG9032V2_54616S_LED2, NULL, BF_COMPLEMENT);
cpld_bt_rw(54616s_led3, DELTA_AG9032V2_SLAVE2_PHY_AND_DEBUG_LED_REG,
	   DELTA_AG9032V2_54616S_LED3, NULL, BF_COMPLEMENT);
cpld_bt_rw(tled0_r_p3, DELTA_AG9032V2_SLAVE2_PHY_AND_DEBUG_LED_REG,
	   DELTA_AG9032V2_TLED0_R_P3, NULL, BF_COMPLEMENT);
cpld_bt_rw(tled0_g_p3, DELTA_AG9032V2_SLAVE2_PHY_AND_DEBUG_LED_REG,
	   DELTA_AG9032V2_TLED0_G_P3, NULL, BF_COMPLEMENT);
cpld_bt_rw(tled0_b_p3, DELTA_AG9032V2_SLAVE2_PHY_AND_DEBUG_LED_REG,
	   DELTA_AG9032V2_TLED0_B_P3, NULL, BF_COMPLEMENT);
cpld_bt_rw(tled2_p3, DELTA_AG9032V2_SLAVE2_PHY_AND_DEBUG_LED_REG,
	   DELTA_AG9032V2_TLED2_P3, NULL, BF_COMPLEMENT);
cpld_bt_rw(1pps_out, DELTA_AG9032V2_SLAVE2_SYNC_E_SIGNAL_REG,
	   DELTA_AG9032V2_1PPS_OUT, NULL, 0);
cpld_bt_rw(82p33731_int, DELTA_AG9032V2_SLAVE2_SYNC_E_SIGNAL_REG,
	   DELTA_AG9032V2_82P33731_INT, NULL, 0);
cpld_bt_rw(bs_clk, DELTA_AG9032V2_SLAVE2_SYNC_E_SIGNAL_REG,
	   DELTA_AG9032V2_BS_CLK, NULL, 0);
cpld_bt_rw(jer1_sync_e_rst_n, DELTA_AG9032V2_SLAVE2_SYNC_E_SIGNAL_REG,
	   DELTA_AG9032V2_JER1_SYNC_E_RST_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(ja_clk_sel, DELTA_AG9032V2_SLAVE2_SYNC_E_SIGNAL_REG,
	   DELTA_AG9032V2_JA_CLK_SEL, NULL, 0);
cpld_bt_ro(sync_e_present, DELTA_AG9032V2_SLAVE2_SYNC_E_SIGNAL_REG,
	   DELTA_AG9032V2_SYNC_E_PRESENT, NULL, BF_COMPLEMENT);
cpld_bt_rw(jer1_sync_e_eeprom_wp, DELTA_AG9032V2_SLAVE2_SYNC_E_SIGNAL_REG,
	   DELTA_AG9032V2_JER1_SYNC_E_EEPROM_WP, NULL, BF_COMPLEMENT);
cpld_bt_rw(t_i210_p1_activity_n, DELTA_AG9032V2_SLAVE2_PHY_I210_SIGNAL_REG,
	   DELTA_AG9032V2_T_I210_P1_ACTIVITY_N, NULL, 0);
cpld_bt_rw(t_i210_p1_link100_n, DELTA_AG9032V2_SLAVE2_PHY_I210_SIGNAL_REG,
	   DELTA_AG9032V2_T_I210_P1_LINK100_N, NULL, 0);
cpld_bt_rw(t_i210_p1_link1000_n, DELTA_AG9032V2_SLAVE2_PHY_I210_SIGNAL_REG,
	   DELTA_AG9032V2_T_I210_P1_LINK1000_N, NULL, 0);
cpld_bt_rw(t_c_fm_cpld_pcie_reset_n, DELTA_AG9032V2_SLAVE2_PHY_I210_SIGNAL_REG,
	   DELTA_AG9032V2_T_C_FM_CPLD_PCIE_RESET_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(t_c_i210_rst, DELTA_AG9032V2_SLAVE2_PHY_I210_SIGNAL_REG,
	   DELTA_AG9032V2_T_C_I210_RSTN, NULL, 0);
cpld_bt_rw(t_c_lanwakeb, DELTA_AG9032V2_SLAVE2_PHY_I210_SIGNAL_REG,
	   DELTA_AG9032V2_T_C_LANWAKEB, NULL, 0);
cpld_bt_ro(cpu_rst_mb_oob, DELTA_AG9032V2_SLAVE2_PHY_I210_SIGNAL_REG,
	   DELTA_AG9032V2_CPU_RST_MB_OOB, NULL, 0);
cpld_bt_rw(u86_clk_sel, DELTA_AG9032V2_SLAVE2_PHY_I210_SIGNAL_REG,
	   DELTA_AG9032V2_U86_CLK_SEL, NULL, 0);
cpld_bt_rw(mb_cpld_upgrade_rst, DELTA_AG9032V2_SLAVE2_RESET_SIGNAL_REG,
	   DELTA_AG9032V2_MB_CPLD_UPGRADE_RST, NULL, 0);
cpld_bt_rw(cpld_upgrade_rst, DELTA_AG9032V2_SLAVE2_RESET_SIGNAL_REG,
	   DELTA_AG9032V2_S2_CPLD_UPGRADE_RST, NULL, 0);
cpld_bt_rw(cpld_gpio_rst_done, DELTA_AG9032V2_SLAVE2_RESET_SIGNAL_REG,
	   DELTA_AG9032V2_CPLD_GPIO_RST_DONE, NULL, 0);
cpld_bt_rw(mb_b_cpld_rstn_p3, DELTA_AG9032V2_SLAVE2_RESET_SIGNAL_REG,
	   DELTA_AG9032V2_MB_B_CPLD_RSTN_P3, NULL, 0);
cpld_bt_rw(cpld_mb_ldrq0_n, DELTA_AG9032V2_SLAVE2_RESET_SIGNAL_REG,
	   DELTA_AG9032V2_CPLD_MB_LDRQ0_N, NULL, 0);
cpld_bt_rw(cpld_gpio_pwr_en, DELTA_AG9032V2_SLAVE2_RESET_SIGNAL_REG,
	   DELTA_AG9032V2_CPLD_GPIO_PWR_EN, NULL, 0);
cpld_bt_rw(cpld_gpio_pwr_pg, DELTA_AG9032V2_SLAVE2_RESET_SIGNAL_REG,
	   DELTA_AG9032V2_CPLD_GPIO_PWR_PG, NULL, 0);
cpld_bt_rw(cpld_gpio_rst_en, DELTA_AG9032V2_SLAVE2_RESET_SIGNAL_REG,
	   DELTA_AG9032V2_S2_CPLD_GPIO_RST_EN, NULL, 0);
cpld_bt_rw(sled_clk0_b2a_p3, DELTA_AG9032V2_SLAVE2_MAC_LED_REG,
	   DELTA_AG9032V2_SLED_CLK0_B2A_P3, NULL, 0);
cpld_bt_rw(sled_clk1_b2a_p3, DELTA_AG9032V2_SLAVE2_MAC_LED_REG,
	   DELTA_AG9032V2_SLED_CLK1_B2A_P3, NULL, 0);
cpld_bt_rw(sled_dat0_b2a_p3, DELTA_AG9032V2_SLAVE2_MAC_LED_REG,
	   DELTA_AG9032V2_SLED_DAT0_B2A_P3, NULL, 0);
cpld_bt_rw(sled_dat1_b2a_p3, DELTA_AG9032V2_SLAVE2_MAC_LED_REG,
	   DELTA_AG9032V2_SLED_DAT1_B2A_P3, NULL, 0);
cpld_bt_rw(p3_mpld_1, DELTA_AG9032V2_SLAVE2_MAC_LED_REG,
	   DELTA_AG9032V2_P3_MPLD_1, NULL, 0);
cpld_bt_rw(pld1_pld3_b, DELTA_AG9032V2_SLAVE2_GPIO1_REG,
	   DELTA_AG9032V2_S2_PLD1_PLD3_B, NULL, 0);
cpld_bt_rw(pld1_pld3_a, DELTA_AG9032V2_SLAVE2_GPIO1_REG,
	   DELTA_AG9032V2_S2_PLD1_PLD3_A, NULL, 0);
cpld_bt_rw(cpld_ab_gpio4_p3, DELTA_AG9032V2_SLAVE2_GPIO1_REG,
	   DELTA_AG9032V2_S2_CPLD_AB_GPIO4_P3, NULL, 0);
cpld_bt_rw(cpld_ab_gpio3_p3, DELTA_AG9032V2_SLAVE2_GPIO1_REG,
	   DELTA_AG9032V2_S2_CPLD_AB_GPIO3_P3, NULL, 0);
cpld_bt_rw(gpio_p1_p3_a, DELTA_AG9032V2_SLAVE2_GPIO1_REG,
	   DELTA_AG9032V2_S2_GPIO_P1_P3_A, NULL, 0);
cpld_bt_rw(cpld_b_cpld_gpio2, DELTA_AG9032V2_SLAVE2_GPIO1_REG,
	   DELTA_AG9032V2_S2_CPLD_B_CPLD_GPIO2, NULL, 0);
cpld_bt_rw(cpu_cpld3_gpio1, DELTA_AG9032V2_SLAVE2_GPIO1_REG,
	   DELTA_AG9032V2_S2_CPU_CPLD3_GPIO1, NULL, 0);
cpld_bt_rw(cpld_b_cpld_gpio3, DELTA_AG9032V2_SLAVE2_GPIO1_REG,
	   DELTA_AG9032V2_S2_CPLD_B_CPLD_GPIO3, NULL, 0);
cpld_bt_rw(cpld_ab_gpio1, DELTA_AG9032V2_SLAVE2_GPIO2_REG,
	   DELTA_AG9032V2_S2_CPLD_AB_GPIO1, NULL, 0);
cpld_bt_rw(cpld_b_cpld_gpio1_p3, DELTA_AG9032V2_SLAVE2_GPIO2_REG,
	   DELTA_AG9032V2_S2_CPLD_B_CPLD_GPIO1_P3, NULL, 0);
cpld_bt_rw(cpld_ab_gpio2, DELTA_AG9032V2_SLAVE2_GPIO2_REG,
	   DELTA_AG9032V2_S2_CPLD_AB_GPIO2, NULL, 0);
cpld_bt_rw(pld2_pld3_a, DELTA_AG9032V2_SLAVE2_GPIO2_REG,
	   DELTA_AG9032V2_S2_PLD2_PLD3_A, NULL, 0);
cpld_bt_rw(pld2_pld3_b, DELTA_AG9032V2_SLAVE2_GPIO2_REG,
	   DELTA_AG9032V2_S2_PLD2_PLD3_B, NULL, 0);
cpld_bt_rw(pld2_pld3_c, DELTA_AG9032V2_SLAVE2_GPIO2_REG,
	   DELTA_AG9032V2_S2_PLD2_PLD3_C, NULL, 0);
cpld_bt_rw(gpioh6, DELTA_AG9032V2_SLAVE2_GPIO2_REG,
	   DELTA_AG9032V2_S2_GPIOH6, NULL, 0);
cpld_bt_rw(gpioh7, DELTA_AG9032V2_SLAVE2_GPIO2_REG,
	   DELTA_AG9032V2_S2_GPIOH7, NULL, 0);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_slave2_revision.attr,
	&cpld_54616s_led1.attr,
	&cpld_54616s_led2.attr,
	&cpld_54616s_led3.attr,
	&cpld_tled0_r_p3.attr,
	&cpld_tled0_g_p3.attr,
	&cpld_tled0_b_p3.attr,
	&cpld_tled2_p3.attr,
	&cpld_1pps_out.attr,
	&cpld_82p33731_int.attr,
	&cpld_bs_clk.attr,
	&cpld_jer1_sync_e_rst_n.attr,
	&cpld_ja_clk_sel.attr,
	&cpld_sync_e_present.attr,
	&cpld_jer1_sync_e_eeprom_wp.attr,
	&cpld_t_i210_p1_activity_n.attr,
	&cpld_t_i210_p1_link100_n.attr,
	&cpld_t_i210_p1_link1000_n.attr,
	&cpld_t_c_fm_cpld_pcie_reset_n.attr,
	&cpld_t_c_i210_rst.attr,
	&cpld_t_c_lanwakeb.attr,
	&cpld_cpu_rst_mb_oob.attr,
	&cpld_u86_clk_sel.attr,
	&cpld_mb_cpld_upgrade_rst.attr,
	&cpld_cpld_upgrade_rst.attr,
	&cpld_cpld_gpio_rst_done.attr,
	&cpld_mb_b_cpld_rstn_p3.attr,
	&cpld_cpld_mb_ldrq0_n.attr,
	&cpld_cpld_gpio_pwr_en.attr,
	&cpld_cpld_gpio_pwr_pg.attr,
	&cpld_cpld_gpio_rst_en.attr,
	&cpld_sled_clk0_b2a_p3.attr,
	&cpld_sled_clk1_b2a_p3.attr,
	&cpld_sled_dat0_b2a_p3.attr,
	&cpld_sled_dat1_b2a_p3.attr,
	&cpld_p3_mpld_1.attr,
	&cpld_pld1_pld3_b.attr,
	&cpld_pld1_pld3_a.attr,
	&cpld_cpld_ab_gpio4_p3.attr,
	&cpld_cpld_ab_gpio3_p3.attr,
	&cpld_gpio_p1_p3_a.attr,
	&cpld_cpld_b_cpld_gpio2.attr,
	&cpld_cpu_cpld3_gpio1.attr,
	&cpld_cpld_b_cpld_gpio3.attr,
	&cpld_cpld_ab_gpio1.attr,
	&cpld_cpld_b_cpld_gpio1_p3.attr,
	&cpld_cpld_ab_gpio2.attr,
	&cpld_pld2_pld3_a.attr,
	&cpld_pld2_pld3_b.attr,
	&cpld_pld2_pld3_c.attr,
	&cpld_gpioh6.attr,
	&cpld_gpioh7.attr,

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
