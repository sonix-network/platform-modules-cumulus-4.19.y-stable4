// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AG9032v1 CPU CPLD Driver
 *
 * Copyright (C) 2019 Cumulus Networks, Inc.  All Rights Reserved.
 * Author: David Yen <dhyen@cumulusnetworks.com>
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
#include "delta-ag9032v1.h"

#define DRIVER_NAME    AG9032V1_CPUPLD_NAME
#define DRIVER_VERSION "1.1"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* CPLD register bitfields with enum-like values */

/* CPLD registers */

cpld_bf_ro(cpu_cpld_version, DELTA_AG9032V1_CPUPLD_VERSION_REG,
	   DELTA_AG9032V1_CPUPLD_REV, NULL, 0);
cpld_bf_ro(cpu_board_version, DELTA_AG9032V1_CPUPLD_BOARD_ID_REG,
	   DELTA_AG9032V1_CPUPLD_BOARD_VERSION, NULL, 0);
cpld_bf_ro(cpu_board_id, DELTA_AG9032V1_CPUPLD_BOARD_ID_REG,
	   DELTA_AG9032V1_CPUPLD_BOARD_ID, NULL, 0);
cpld_bt_rw(reset_cpld, DELTA_AG9032V1_CPUPLD_SW_RESET_1_REG,
	   DELTA_AG9032V1_FORCE_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_main_board, DELTA_AG9032V1_CPUPLD_SW_RESET_1_REG,
	   DELTA_AG9032V1_MB_RST_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_i2c, DELTA_AG9032V1_CPUPLD_SW_RESET_1_REG,
	   DELTA_AG9032V1_I2C_SW_RST, NULL, BF_COMPLEMENT);
cpld_bt_ro(reset_req, DELTA_AG9032V1_CPUPLD_SW_RESET_2_REG,
	   DELTA_AG9032V1_RESET_REQ, NULL, BF_COMPLEMENT);
cpld_bt_ro(pg_3v3, DELTA_AG9032V1_CPUPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_PG_3V3, NULL, 0);
cpld_bt_ro(pg_1v1, DELTA_AG9032V1_CPUPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_PG_1V1, NULL, 0);
cpld_bt_ro(pgd_ddr_vtt, DELTA_AG9032V1_CPUPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_PGD_DDR_VTT, NULL, 0);
cpld_bt_ro(vr_pvddr_a_pwrgd, DELTA_AG9032V1_CPUPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_VR_PVDDR_A_PWRGD, NULL, 0);
cpld_bt_ro(pwrgd_pvccp_pvnn, DELTA_AG9032V1_CPUPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_PWRGD_PVCCP_PVNN, NULL, 0);
cpld_bt_ro(v1p0_pg, DELTA_AG9032V1_CPUPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_V1P0_PG, NULL, 0);
cpld_bt_ro(pg_5va, DELTA_AG9032V1_CPUPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_PG_5VA, NULL, 0);
cpld_bt_ro(pg_3v3a, DELTA_AG9032V1_CPUPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_PG_3V3A, NULL, 0);
cpld_bt_ro(mb_pwr_pgd, DELTA_AG9032V1_CPUPLD_POWER_STATUS_CONTROL_2_REG,
	   DELTA_AG9032V1_MB_PWR_PGD, NULL, 0);
cpld_bt_ro(reg_1v8_pg, DELTA_AG9032V1_CPUPLD_POWER_STATUS_CONTROL_2_REG,
	   DELTA_AG9032V1_REG_1V8_PG, NULL, 0);
cpld_bt_ro(reg_1v35_pg, DELTA_AG9032V1_CPUPLD_POWER_STATUS_CONTROL_2_REG,
	   DELTA_AG9032V1_REG_1V35_PG, NULL, 0);
cpld_bt_ro(irq_phy, DELTA_AG9032V1_CPUPLD_INTERRUPT_1_REG,
	   DELTA_AG9032V1_IRQ_PHY, NULL, BF_COMPLEMENT);
cpld_bt_ro(op_module_event, DELTA_AG9032V1_CPUPLD_INTERRUPT_1_REG,
	   DELTA_AG9032V1_OP_MODULE_EVENT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_fan_event, DELTA_AG9032V1_CPUPLD_INTERRUPT_1_REG,
	   DELTA_AG9032V1_PSU_FAN_EVENT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(spi_wp_gbe, DELTA_AG9032V1_CPUPLD_FLASH_PROTECTION_1_REG,
	   DELTA_AG9032V1_SPI_WP_GBE_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(eeprom_protect, DELTA_AG9032V1_CPUPLD_FLASH_PROTECTION_1_REG,
	   DELTA_AG9032V1_EEPROM_WP, NULL, 0);
cpld_bt_rw(cpld_mux_cpu, DELTA_AG9032V1_CPUPLD_FLASH_PROTECTION_2_REG,
	   DELTA_AG9032V1_CPLD_MUX_R, NULL, 0);

static struct attribute *cpld_attrs[] = {
	&cpld_cpu_cpld_version.attr,
	&cpld_cpu_board_version.attr,
	&cpld_cpu_board_id.attr,
	&cpld_reset_cpld.attr,
	&cpld_reset_main_board.attr,
	&cpld_reset_i2c.attr,
	&cpld_reset_req.attr,
	&cpld_pg_3v3.attr,
	&cpld_pg_1v1.attr,
	&cpld_pgd_ddr_vtt.attr,
	&cpld_vr_pvddr_a_pwrgd.attr,
	&cpld_pwrgd_pvccp_pvnn.attr,
	&cpld_v1p0_pg.attr,
	&cpld_pg_5va.attr,
	&cpld_pg_3v3a.attr,
	&cpld_mb_pwr_pgd.attr,
	&cpld_reg_1v8_pg.attr,
	&cpld_reg_1v35_pg.attr,
	&cpld_irq_phy.attr,
	&cpld_op_module_event.attr,
	&cpld_psu_fan_event.attr,
	&cpld_spi_wp_gbe.attr,
	&cpld_eeprom_protect.attr,
	&cpld_cpld_mux_cpu.attr,

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
					DELTA_AG9032V1_CPUPLD_VERSION_REG);
	if (temp < 0) {
		dev_err(dev, "read CPUPLD version register error: %d\n",
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
	dev_info(dev, "device probed, CPUPLD rev: %lu\n",
		 GET_FIELD(temp, DELTA_AG9032V1_CPUPLD_REV));

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
MODULE_DESCRIPTION("Delta AG9032v1 CPU CPLD Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

