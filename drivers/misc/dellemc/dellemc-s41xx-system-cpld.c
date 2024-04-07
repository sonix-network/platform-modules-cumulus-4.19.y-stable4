// SPDX-License-Identifier: GPL-2.0+
/*
 *  dellemc_s41xx-system-cpld.c - Dell EMC S41xx System CPLD support.
 *
 *  Copyright (C) 2017, 2019 Cumulus Networks, Inc.  All Rights Reserved
 *  Author: David Yen (dhyen@cumulusnetworks.com)
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

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c-ismt.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "dellemc-s41xx-cplds.h"

#define DRIVER_NAME     SYSTEM_CPLD_DRIVER_NAME
#define DRIVER_VERSION	"1.1"

#define cpld_read_reg cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

static const char * const cpu_board_type_values[] = {
	"Non-NEBS CPU Platform", /*  0 */
	"NEBS CPU Platform",     /*  1 */
	"reserved (2)",          /*  2 */
	"reserved (3)",          /*  3 */
	"reserved (4)",          /*  4 */
	"reserved (5)",          /*  5 */
	"reserved (6)",          /*  6 */
	"reserved (7)",          /*  7 */
	"reserved (8)",          /*  8 */
	"reserved (9)",          /*  9 */
	"reserved (10)",         /* 10 */
	"reserved (11)",         /* 11 */
	"reserved (12)",         /* 12 */
	"reserved (13)",         /* 13 */
	"reserved (14)",         /* 14 */
	"reserved (15)"          /* 15 */
};

static const char * const cpu_board_rev_values[] = {
	"X00",           /*  0 */
	"X01",           /*  1 */
	"X02",           /*  2 */
	"X03",           /*  3 */
	"reserved (4)",  /*  4 */
	"reserved (5)",  /*  5 */
	"reserved (6)",  /*  6 */
	"reserved (7)",  /*  7 */
	"reserved (8)",  /*  8 */
	"reserved (9)",  /*  9 */
	"reserved (10)", /* 10 */
	"reserved (11)", /* 11 */
	"reserved (12)", /* 12 */
	"reserved (13)", /* 13 */
	"reserved (14)", /* 14 */
	"reserved (15)"  /* 15 */
};

static const char * const wd_timer_values[] = {
	"15",       /* 0 */
	"20",       /* 1 */
	"30",       /* 2 */
	"40",       /* 3 */
	"50",       /* 4 */
	"60",       /* 5 */
	"65",       /* 6 */
	"70"        /* 7 */
};

cpld_bf_ro(system_cpld_major_version, DELL_S41XX_SYS_CPLD_REV_REG,
	   DELL_S41XX_SYS_CPLD_REV_MJR_REV, NULL, BF_DECIMAL);
cpld_bf_ro(system_cpld_minor_version, DELL_S41XX_SYS_CPLD_REV_REG,
	   DELL_S41XX_SYS_CPLD_REV_MNR_REV, NULL, BF_DECIMAL);
mk_bf_rw(cpld, scratch, DELL_S41XX_SYS_CPLD_GPR_REG, 0, 8, NULL, 0);
cpld_bf_ro(system_cpld_cpu_board_revision, DELL_S41XX_SYS_CPU_BRD_REV_TYPE_REG,
	   DELL_S41XX_SYS_CPU_BRD_REV_TYPE_BRD_REV, cpu_board_rev_values, 0);
cpld_bf_ro(system_cpld_cpu_board_type, DELL_S41XX_SYS_CPU_BRD_REV_TYPE_REG,
	   DELL_S41XX_SYS_CPU_BRD_REV_TYPE_BRD_TYPE, cpu_board_type_values, 0);
cpld_bt_ro(ssd_present, DELL_S41XX_SYS_SRR_REG, DELL_S41XX_SYS_SRR_SSD_PRESNT,
	   NULL, BF_COMPLEMENT);
cpld_bt_rw(boot_backup_bios, DELL_S41XX_SYS_SRR_REG,
	   DELL_S41XX_SYS_SRR_RST_BIOS_SWITCH, NULL, BF_COMPLEMENT);
cpld_bt_rw(power_cycle, DELL_S41XX_SYS_SRR_REG,
	   DELL_S41XX_SYS_SRR_CPLD_UPGRADE_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(system_eeprom_wp, DELL_S41XX_SYS_EEPROM_WP_REG,
	   DELL_S41XX_SYS_EEPROM_WP_SYSTM_ID_EEPROM_WP, NULL, 0);
cpld_bt_rw(gbe_spi_wp, DELL_S41XX_SYS_EEPROM_WP_REG,
	   DELL_S41XX_SYS_EEPROM_WP_SPI_WP_GBE, NULL, 0);
cpld_bt_rw(primary_bios_wp, DELL_S41XX_SYS_EEPROM_WP_REG,
	   DELL_S41XX_SYS_EEPROM_WP_SPI_BIOS_WP, NULL, 0);
cpld_bt_rw(backup_bios_wp, DELL_S41XX_SYS_EEPROM_WP_REG,
	   DELL_S41XX_SYS_EEPROM_WP_SPI_BAK_BIOS_WP, NULL, 0);
cpld_bf_rw(watchdog_timer, DELL_S41XX_SYS_WD_REG,
	   DELL_S41XX_SYS_WD_WD_TIMER, wd_timer_values, 0);
cpld_bt_rw(watchdog_enable, DELL_S41XX_SYS_WD_REG,
	   DELL_S41XX_SYS_WD_WD_EN, NULL, 0);
cpld_bt_rw(watchdog_punch, DELL_S41XX_SYS_WD_REG,
	   DELL_S41XX_SYS_WD_WD_PUNCH, NULL, BF_COMPLEMENT);
cpld_bt_rw(warm_reset_npu, DELL_S41XX_SYS_MB_RST_EN_REG,
	   DELL_S41XX_SYS_MB_RST_EN, NULL, 0);
mk_bf_ro(cpld, reboot_cause, DELL_S41XX_SYS_REBOOT_CAUSE_REG, 0, 8, NULL, 0);
cpld_bt_ro(power_v1p5_enabled, DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_EN_STATUS_V1P5_EN, NULL, 0);
cpld_bt_ro(power_vddr_enabled, DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_EN_STATUS_PWR_VDDR_EN, NULL, 0);
cpld_bt_ro(power_core_enabled, DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_EN_STATUS_PWR_CORE_EN, NULL, 0);
cpld_bt_ro(power_v1p1_enabled, DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_EN_STATUS_V1P1_EN, NULL, 0);
cpld_bt_ro(power_v1p0_enabled, DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_EN_STATUS_V1P0_EN, NULL, 0);
cpld_bt_ro(power_v3p3_enabled, DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_EN_STATUS_V3P3_EN, NULL, 0);
cpld_bt_ro(power_v1p8_enabled, DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG_1V8_EN, NULL, 0);
cpld_bt_ro(power_v1p35_enabled, DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG_1V35_EN, NULL, 0);
cpld_bt_ro(power_ddr_vtt_good, DELL_S41XX_SYS_CPU_PWR_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_STATUS_PG_DDR_VTT, NULL, 0);
cpld_bt_ro(power_vddr_good, DELL_S41XX_SYS_CPU_PWR_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_STATUS_PG_PVDDR, NULL, 0);
cpld_bt_ro(power_core_good, DELL_S41XX_SYS_CPU_PWR_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_STATUS_PG_PWR_CORE, NULL, 0);
cpld_bt_ro(power_v1p1_good, DELL_S41XX_SYS_CPU_PWR_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_STATUS_PG_V1P1, NULL, 0);
cpld_bt_ro(power_v1p0_good, DELL_S41XX_SYS_CPU_PWR_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_STATUS_PG_V1P0, NULL, 0);
cpld_bt_ro(power_v3p3_good, DELL_S41XX_SYS_CPU_PWR_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_STATUS_PG_3V3, NULL, 0);
cpld_bt_ro(power_v1p8_good, DELL_S41XX_SYS_CPU_PWR_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_STATUS_PG_1V8, NULL, 0);
cpld_bt_ro(power_v1p35_good, DELL_S41XX_SYS_CPU_PWR_STATUS_REG,
	   DELL_S41XX_SYS_CPU_PWR_STATUS_PG_1V35, NULL, 0);

/*
 * SYSFS attributes
 */
static struct attribute *cpld_attrs[] = {
	&cpld_system_cpld_major_version.attr,
	&cpld_system_cpld_minor_version.attr,
	&cpld_scratch.attr,
	&cpld_system_cpld_cpu_board_revision.attr,
	&cpld_system_cpld_cpu_board_type.attr,
	&cpld_ssd_present.attr,
	&cpld_boot_backup_bios.attr,
	&cpld_power_cycle.attr,
	&cpld_system_eeprom_wp.attr,
	&cpld_gbe_spi_wp.attr,
	&cpld_primary_bios_wp.attr,
	&cpld_backup_bios_wp.attr,
	&cpld_watchdog_timer.attr,
	&cpld_watchdog_enable.attr,
	&cpld_watchdog_punch.attr,
	&cpld_warm_reset_npu.attr,
	&cpld_reboot_cause.attr,
	&cpld_power_v1p5_enabled.attr,
	&cpld_power_vddr_enabled.attr,
	&cpld_power_core_enabled.attr,
	&cpld_power_v1p1_enabled.attr,
	&cpld_power_v1p0_enabled.attr,
	&cpld_power_v3p3_enabled.attr,
	&cpld_power_v1p8_enabled.attr,
	&cpld_power_v1p35_enabled.attr,
	&cpld_power_ddr_vtt_good.attr,
	&cpld_power_vddr_good.attr,
	&cpld_power_core_good.attr,
	&cpld_power_v1p1_good.attr,
	&cpld_power_v1p0_good.attr,
	&cpld_power_v3p3_good.attr,
	&cpld_power_v1p8_good.attr,
	&cpld_power_v1p35_good.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

static int cpld_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int boardrev;
	int cpldrev;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(dev,
			"adapter does not support I2C_FUNC_SMBUS_BYTE_DATA\n");
		ret = -EINVAL;
		goto err;
	}

	/*
	 * Probe the hardware by reading the revision numbers.
	 */
	ret = i2c_smbus_read_byte_data(client, DELL_S41XX_SYS_CPLD_REV_REG);
	if (ret < 0) {
		dev_err(dev, "read cpld revision register error %d\n", ret);
		goto err;
	}
	cpldrev = ret;
	ret = i2c_smbus_read_byte_data(client,
				       DELL_S41XX_SYS_CPU_BRD_REV_TYPE_REG);
	if (ret < 0) {
		dev_err(dev, "read board revision register error %d\n", ret);
		goto err;
	}
	boardrev = ret;

	/*
	 * Create sysfs nodes.
	 */
	ret = sysfs_create_group(&dev->kobj, &cpld_attr_group);
	if (ret) {
		dev_err(dev, "sysfs_create_group failed\n");
		goto err;
	}

	/*
	 * All clear from this point on
	 */
	dev_info(dev,
		 "device created, board type %ld rev X%ld, CPLD rev %ld.%ld\n",
		 GET_FIELD(boardrev, DELL_S41XX_SYS_CPU_BRD_REV_TYPE_BRD_TYPE),
		 GET_FIELD(boardrev, DELL_S41XX_SYS_CPU_BRD_REV_TYPE_BRD_REV),
		 GET_FIELD(cpldrev, DELL_S41XX_SYS_CPLD_REV_MJR_REV),
		 GET_FIELD(cpldrev, DELL_S41XX_SYS_CPLD_REV_MNR_REV));

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
	{ }
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

/*
 * Module init/exit
 */
static int __init cpld_init(void)
{
	int ret = 0;

	/*
	 * Do as little as possible in the module init function. Basically just
	 * register drivers. Those driver's probe functions will probe for
	 * hardware and create devices.
	 */
	pr_info(DRIVER_NAME
		": version " DRIVER_VERSION " initializing\n");

	/* Register the I2C CPLD driver for the CPLD */
	ret = i2c_add_driver(&cpld_driver);
	if (ret) {
		pr_err(DRIVER_NAME
		       ": %s driver registration failed. (%d)\n",
		       cpld_driver.driver.name, ret);
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully initialized\n");
	return ret;
}

static void __exit cpld_exit(void)
{
	i2c_del_driver(&cpld_driver);
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

module_init(cpld_init);
module_exit(cpld_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC S41xx system cpld support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
