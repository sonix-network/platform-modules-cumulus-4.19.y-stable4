// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AGV848v1 CPUPLD Driver
 *
 * Copyright (C) 2020 Cumulus Networks, Inc.  All Rights Reserved.
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
#include <linux/hwmon-sysfs.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "delta-agv848v1.h"

#define DRIVER_NAME    AGV848V1_CPUPLD_NAME
#define DRIVER_VERSION "1.0"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* CPLD register bitfields with enum-like values */

/* CPLD registers */

cpld_bf_rw(scratch, DELTA_AGV848V1_SYS_CPLD_GPR_REG,
	   DELTA_AGV848V1_SCRTCH_REG, NULL, 0);
cpld_bf_ro(cpu_board_version, DELTA_AGV848V1_CPU_BRD_REV_TYPE_REG,
	   DELTA_AGV848V1_BRD_REV, NULL, 0);
cpld_bf_ro(cpu_board_type, DELTA_AGV848V1_CPU_BRD_REV_TYPE_REG,
	   DELTA_AGV848V1_BRD_TYPE, NULL, 0);
cpld_bt_rw(spi_cs_sel, DELTA_AGV848V1_SYS_SRR_REG,
	   DELTA_AGV848V1_SPI_CS_SEL, NULL, 0);
cpld_bt_rw(bios_boot_sel, DELTA_AGV848V1_SYS_SRR_REG,
	   DELTA_AGV848V1_RST_BIOS_SWITCH, NULL, 0);
cpld_bt_rw(eeprom_wp, DELTA_AGV848V1_SYS_EEPROM_WP_REG,
	   DELTA_AGV848V1_SYSTM_ID_EEPROM_WP, NULL, 0);
cpld_bt_rw(spi_gbe_wp, DELTA_AGV848V1_SYS_EEPROM_WP_REG,
	   DELTA_AGV848V1_SPI_WP_GBE, NULL, 0);
cpld_bt_rw(spi_bios_wp, DELTA_AGV848V1_SYS_EEPROM_WP_REG,
	   DELTA_AGV848V1_SPI_BIOS_WP, NULL, 0);
cpld_bt_rw(spi_backup_bios_wp, DELTA_AGV848V1_SYS_EEPROM_WP_REG,
	   DELTA_AGV848V1_SPI_BAK_BIOS_WP, NULL, 0);
cpld_bt_rw(lpc_clk_fail_irq_en, DELTA_AGV848V1_SYS_IRQ_REG,
	   DELTA_AGV848V1_LPC_CLK_FAIL_IRQ_EN, NULL, 0);
cpld_bt_rw(vrhot_vccp_irq_en, DELTA_AGV848V1_SYS_IRQ_REG,
	   DELTA_AGV848V1_VRHOT_VCCP_IRQ_EN, NULL, 0);
cpld_bt_rw(cpu_thermtrip_irq_en, DELTA_AGV848V1_SYS_IRQ_REG,
	   DELTA_AGV848V1_CPU_THERMTRIP_IRQ_EN, NULL, 0);
cpld_bt_rw(temp_alert_irq_en, DELTA_AGV848V1_SYS_IRQ_REG,
	   DELTA_AGV848V1_TEMP_ALERT_IRQ_EN, NULL, 0);
cpld_bt_ro(lpc_clk_fail_irq, DELTA_AGV848V1_SYS_IRQ_REG,
	   DELTA_AGV848V1_LPC_CLK_FAIL_IRQ, NULL, BF_COMPLEMENT);
cpld_bt_ro(vrhot_vccp_irq, DELTA_AGV848V1_SYS_IRQ_REG,
	   DELTA_AGV848V1_VRHOT_VCCP_IRQ, NULL, BF_COMPLEMENT);
cpld_bt_ro(cpu_thermtrip_irq, DELTA_AGV848V1_SYS_IRQ_REG,
	   DELTA_AGV848V1_CPU_THERMTRIP_IRQ, NULL, BF_COMPLEMENT);
cpld_bt_ro(temp_alert_irq, DELTA_AGV848V1_SYS_IRQ_REG,
	   DELTA_AGV848V1_TEMP_ALERT_IRQ, NULL, BF_COMPLEMENT);
cpld_bf_rw(wd_timer, DELTA_AGV848V1_SYS_WD_REG,
	   DELTA_AGV848V1_WD_TIMER, NULL, 0);
cpld_bt_rw(wd_en, DELTA_AGV848V1_SYS_WD_REG,
	   DELTA_AGV848V1_WD_EN, NULL, 0);
cpld_bt_rw(wd_punch, DELTA_AGV848V1_SYS_WD_REG,
	   DELTA_AGV848V1_WD_PUNCH, NULL, 0);
cpld_bt_ro(reboot_reason_cold_reset, DELTA_AGV848V1_REBOOT_CAUSE_REG,
	   DELTA_AGV848V1_COLD_RESET, NULL, 0);
cpld_bt_ro(reboot_reason_warm_reset, DELTA_AGV848V1_REBOOT_CAUSE_REG,
	   DELTA_AGV848V1_WARM_RESET, NULL, 0);
cpld_bt_ro(reboot_reason_wd_fail, DELTA_AGV848V1_REBOOT_CAUSE_REG,
	   DELTA_AGV848V1_WD_FAIL, NULL, 0);
cpld_bt_ro(reboot_reason_bios_switchover, DELTA_AGV848V1_REBOOT_CAUSE_REG,
	   DELTA_AGV848V1_BIOS_SWITCHOVER, NULL, 0);
cpld_bt_ro(reboot_reason_boot_fail, DELTA_AGV848V1_REBOOT_CAUSE_REG,
	   DELTA_AGV848V1_BOOT_FAIL, NULL, 0);
cpld_bt_ro(reboot_reason_cpu_pwr_error, DELTA_AGV848V1_REBOOT_CAUSE_REG,
	   DELTA_AGV848V1_CPU_PWR_ERR, NULL, 0);
cpld_bt_rw(v1p5_en, DELTA_AGV848V1_CPU_PWR_EN_STATUS_REG,
	   DELTA_AGV848V1_V1P5_EN, NULL, 0);
cpld_bt_rw(pwr_vddr_en, DELTA_AGV848V1_CPU_PWR_EN_STATUS_REG,
	   DELTA_AGV848V1_PWR_VDDR_EN, NULL, 0);
cpld_bt_rw(pwr_core_en, DELTA_AGV848V1_CPU_PWR_EN_STATUS_REG,
	   DELTA_AGV848V1_PWR_CORE_EN, NULL, 0);
cpld_bt_rw(v1p1_en, DELTA_AGV848V1_CPU_PWR_EN_STATUS_REG,
	   DELTA_AGV848V1_V1P1_EN, NULL, 0);
cpld_bt_rw(v1p0_en, DELTA_AGV848V1_CPU_PWR_EN_STATUS_REG,
	   DELTA_AGV848V1_V1P0_EN, NULL, 0);
cpld_bt_rw(v3p3_en, DELTA_AGV848V1_CPU_PWR_EN_STATUS_REG,
	   DELTA_AGV848V1_V3P3_EN, NULL, 0);
cpld_bt_rw(reg_1v8_en, DELTA_AGV848V1_CPU_PWR_EN_STATUS_REG,
	   DELTA_AGV848V1_REG_1V8_EN, NULL, 0);
cpld_bt_rw(reg_1v35_en, DELTA_AGV848V1_CPU_PWR_EN_STATUS_REG,
	   DELTA_AGV848V1_REG_1V35_EN, NULL, 0);
cpld_bt_ro(pg_ddr_vtt, DELTA_AGV848V1_CPU_PWR_STATUS_1_REG,
	   DELTA_AGV848V1_PG_DDR_VTT, NULL, 0);
cpld_bt_ro(pg_pvddr, DELTA_AGV848V1_CPU_PWR_STATUS_1_REG,
	   DELTA_AGV848V1_PG_PVDDR, NULL, 0);
cpld_bt_ro(pg_pwr_core, DELTA_AGV848V1_CPU_PWR_STATUS_1_REG,
	   DELTA_AGV848V1_PG_PWR_CORE, NULL, 0);
cpld_bt_ro(pg_v1p1, DELTA_AGV848V1_CPU_PWR_STATUS_1_REG,
	   DELTA_AGV848V1_PG_V1P1, NULL, 0);
cpld_bt_ro(pg_v1p0, DELTA_AGV848V1_CPU_PWR_STATUS_1_REG,
	   DELTA_AGV848V1_PG_V1P0, NULL, 0);
cpld_bt_ro(pg_v3p3, DELTA_AGV848V1_CPU_PWR_STATUS_1_REG,
	   DELTA_AGV848V1_PG_3V3, NULL, 0);
cpld_bt_ro(pg_v1p8, DELTA_AGV848V1_CPU_PWR_STATUS_1_REG,
	   DELTA_AGV848V1_PG_1V8, NULL, 0);
cpld_bt_ro(pg_v1p35, DELTA_AGV848V1_CPU_PWR_STATUS_1_REG,
	   DELTA_AGV848V1_PG_1V35, NULL, 0);
cpld_bf_rw(cpu_pwr_margin, DELTA_AGV848V1_CPU_PWR_RAIL_MARGINS_CONTROL_REG,
	   DELTA_AGV848V1_CPU_PWR_MARGIN, NULL, 0);

/* special case for cpld version register */

static ssize_t version_show(struct device *dev,
			    struct device_attribute *dattr,
			    char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	s32 temp;
	int reg = DELTA_AGV848V1_SYS_CPLD_REV_REG;

	temp = i2c_smbus_read_byte_data(client, reg);
	if (temp < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, reg);
		return -EINVAL;
	}

	return sprintf(buf, "%d.%d\n",
		       (int)GET_FIELD(temp, DELTA_AGV848V1_MJR_REV),
		       (int)GET_FIELD(temp, DELTA_AGV848V1_MNR_REV));
}

static SENSOR_DEVICE_ATTR_RO(cpucpld_version, version_show, 0);

static struct attribute *cpld_attrs[] = {
	&cpld_scratch.attr,
	&cpld_cpu_board_version.attr,
	&cpld_cpu_board_type.attr,
	&cpld_spi_cs_sel.attr,
	&cpld_bios_boot_sel.attr,
	&cpld_eeprom_wp.attr,
	&cpld_spi_gbe_wp.attr,
	&cpld_spi_bios_wp.attr,
	&cpld_spi_backup_bios_wp.attr,
	&cpld_lpc_clk_fail_irq_en.attr,
	&cpld_vrhot_vccp_irq_en.attr,
	&cpld_cpu_thermtrip_irq_en.attr,
	&cpld_temp_alert_irq_en.attr,
	&cpld_lpc_clk_fail_irq.attr,
	&cpld_vrhot_vccp_irq.attr,
	&cpld_cpu_thermtrip_irq.attr,
	&cpld_temp_alert_irq.attr,
	&cpld_wd_timer.attr,
	&cpld_wd_en.attr,
	&cpld_wd_punch.attr,
	&cpld_reboot_reason_cold_reset.attr,
	&cpld_reboot_reason_warm_reset.attr,
	&cpld_reboot_reason_wd_fail.attr,
	&cpld_reboot_reason_bios_switchover.attr,
	&cpld_reboot_reason_boot_fail.attr,
	&cpld_reboot_reason_cpu_pwr_error.attr,
	&cpld_v1p5_en.attr,
	&cpld_pwr_vddr_en.attr,
	&cpld_pwr_core_en.attr,
	&cpld_v1p1_en.attr,
	&cpld_v1p0_en.attr,
	&cpld_v3p3_en.attr,
	&cpld_reg_1v8_en.attr,
	&cpld_reg_1v35_en.attr,
	&cpld_pg_ddr_vtt.attr,
	&cpld_pg_pvddr.attr,
	&cpld_pg_pwr_core.attr,
	&cpld_pg_v1p1.attr,
	&cpld_pg_v1p0.attr,
	&cpld_pg_v3p3.attr,
	&cpld_pg_v1p8.attr,
	&cpld_pg_v1p35.attr,
	&cpld_cpu_pwr_margin.attr,

	&sensor_dev_attr_cpucpld_version.dev_attr.attr,

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
					DELTA_AGV848V1_SYS_CPLD_REV_REG);
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
	dev_info(dev, "device probed, CPUPLD rev: %lu.%lu\n",
		 GET_FIELD(temp, DELTA_AGV848V1_MJR_REV),
		 GET_FIELD(temp, DELTA_AGV848V1_MNR_REV));

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
MODULE_DESCRIPTION("Delta AGV848v1 CPUPLD Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

