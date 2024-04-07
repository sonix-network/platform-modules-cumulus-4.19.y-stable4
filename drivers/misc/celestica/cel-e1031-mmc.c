// SPDX-License-Identifier: GPL-2.0+
/*
 * Celestica Haliburton (E1031) MMC Driver
 *
 * Copyright (C) 2015, 2020 Cumulus Networks, Inc.  All rights reserved.
 * Authors: Puneet Shenoy <puneet@cumulusnetworks.com>
 *          David Yen <dhyen@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "cel-e1031.h"

#define DRIVER_NAME    E1031_MMC_NAME
#define DRIVER_VERSION "1.1"
#define IO_BASE        MMC_IO_BASE
#define IO_SIZE        MMC_IO_SIZE

/* bitfield accessor functions */

static u8 *cpld_regs;

static int lpc_read_reg(struct device *dev,
			int reg,
			int nregs,
			u32 *val)
{
	int nbits = nregs * 8;
	int bit;

	*val = 0;
	for (bit = 0; bit < nbits; bit += 8, reg++)
		*val |= ioread8(cpld_regs + reg - IO_BASE) << bit;
	return 0;
}

static int lpc_write_reg(struct device *dev,
			 int reg,
			 int nregs,
			 u32 val)
{
	for (; nregs > 0; nregs--, reg++, val >>= 8)
		iowrite8(val, cpld_regs + reg - IO_BASE);
	return 0;
}

#define cpld_read_reg  lpc_read_reg
#define cpld_write_reg lpc_write_reg

/* CPLD register bitfields with enum-like values */

static const char * const wid_values[] = {
	"200ms", /* 0 */
	"30s",   /* 1 */
	"60s",   /* 2 */
	"180s",  /* 3 */
};

static const char * const trigger_values[] = {
	"falling edge", /* 0 */
	"rising edge",  /* 1 */
	"both edges",   /* 2 */
	"low level",    /* 3 */
};

/* CPLD registers */

cpld_bf_ro(major_version, MMC_VERSION_REG,
	   MMC_MAJOR_VERSION, NULL, 0);
cpld_bf_ro(minor_version, MMC_VERSION_REG,
	   MMC_MINOR_VERSION, NULL, 0);
cpld_bf_rw(scratchpad, MMC_SW_SCRATCH_REG,
	   MMC_SCRATCHPAD, NULL, 0);
cpld_bt_ro(cpu_boot_ok, MMC_BOOT_OK_REG,
	   MMC_CPU_BOOT_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(cpu_bios, MMC_BOOT_OK_REG,
	   MMC_CPU_BIOS, NULL, 0);
cpld_bt_rw(spd1_wp, MMC_EEPROM_WP_REG,
	   MMC_SPD1_WP, NULL, 0);
cpld_bt_rw(system_eeprom_wp, MMC_EEPROM_WP_REG,
	   MMC_SYSTEM_EEPROM_WP, NULL, 0);
cpld_bt_ro(present_mb, MMC_PRESENT_MB_REG,
	   MMC_PRESENT_MB, NULL, BF_COMPLEMENT);
cpld_bf_rw(wd_width, MMC_WD_WID_REG,
	   MMC_WD_WID, wid_values, 0);
cpld_bt_rw(wd_en, MMC_WD_MASK_REG,
	   MMC_WD_MASK, NULL, BF_COMPLEMENT);
cpld_bf_ro(reset_source, MMC_RST_SOURCE_REG,
	   MMC_RST_SOURCE, NULL, 0);
cpld_bf_rw(reset_control, MMC_RST_CTRL_REG,
	   MMC_RST_CTRL, NULL, 0);
cpld_bt_rw(reset_smc, MMC_SEP_RST_REG,
	   MMC_SMC_RST, NULL, 0);
cpld_bt_rw(reset_BCM54616, MMC_SEP_RST_REG,
	   MMC_BCM54616_RST, NULL, 0);
cpld_bt_rw(cpu_thermal_poweroff, MMC_THERMAL_POWEROFF_CTRL_REG,
	   MMC_CPU_POWEROFF_CTRL, NULL, 0);
cpld_bf_rw(thermtrip_trig, MMC_SUS0_TRIG_MOD_REG,
	   MMC_THERMTRIP_TRIG, trigger_values, 0);
cpld_bf_rw(bcm54616_trig, MMC_SUS0_TRIG_MOD_REG,
	   MMC_BCM54616_TRIG, trigger_values, 0);
cpld_bf_rw(sensor_trig, MMC_SUS0_TRIG_MOD_REG,
	   MMC_SENSOR_TRIG, trigger_values, 0);
cpld_bt_ro(thermaltrip_combine, MMC_SUS0_COMBINE_REG,
	   MMC_THERMALTRIP_COMBINE, NULL, BF_COMPLEMENT);
cpld_bt_ro(bcm54616_combine, MMC_SUS0_COMBINE_REG,
	   MMC_BCM54616_COMBINE, NULL, BF_COMPLEMENT);
cpld_bt_ro(sensor_prochot_combine, MMC_SUS0_COMBINE_REG,
	   MMC_SENSOR_PROCHOT_COMBINE, NULL, BF_COMPLEMENT);
cpld_bt_ro(sensor_alert_combine, MMC_SUS0_COMBINE_REG,
	   MMC_SENSOR_ALERT_COMBINE, NULL, BF_COMPLEMENT);
cpld_bt_ro(thermaltrip_alert, MMC_SUS0_STA_REG,
	   MMC_THERMTRIP_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(bcm54616_alert, MMC_SUS0_STA_REG,
	   MMC_BCM54616_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(ts_prochot_alert, MMC_SUS0_STA_REG,
	   MMC_TS_PROCHOT_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(ts_alert, MMC_SUS0_STA_REG,
	   MMC_TS_ALERT_STA, NULL, BF_COMPLEMENT);
cpld_bt_ro(thermaltrip_interrupt, MMC_SUS0_INT_REG,
	   MMC_THERMTRIP_INT, NULL, 0);
cpld_bt_ro(bcm54616_interrupt, MMC_SUS0_INT_REG,
	   MMC_BCM54616_INT, NULL, 0);
cpld_bt_ro(ts_prochot_interrupt, MMC_SUS0_INT_REG,
	   MMC_TS_PROCHOT_INT, NULL, 0);
cpld_bt_ro(ts_alert_interrupt, MMC_SUS0_INT_REG,
	   MMC_TS_ALERT_INT, NULL, 0);
cpld_bt_rw(thermaltrip_mask, MMC_SUS0_MASK_REG,
	   MMC_THERMTRIP_MASK, NULL, 0);
cpld_bt_rw(bcm54616_mask, MMC_SUS0_MASK_REG,
	   MMC_BCM54616_MASK, NULL, 0);
cpld_bt_rw(ts_prochot_mask, MMC_SUS0_MASK_REG,
	   MMC_TS_PROCHOT_MASK, NULL, 0);
cpld_bt_rw(ts_alert_mask, MMC_SUS0_MASK_REG,
	   MMC_TS_ALERT_MASK, NULL, 0);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_major_version.attr,
	&cpld_minor_version.attr,
	&cpld_scratchpad.attr,
	&cpld_cpu_boot_ok.attr,
	&cpld_cpu_bios.attr,
	&cpld_spd1_wp.attr,
	&cpld_system_eeprom_wp.attr,
	&cpld_present_mb.attr,
	&cpld_wd_width.attr,
	&cpld_wd_en.attr,
	&cpld_reset_source.attr,
	&cpld_reset_control.attr,
	&cpld_reset_smc.attr,
	&cpld_reset_BCM54616.attr,
	&cpld_cpu_thermal_poweroff.attr,
	&cpld_thermtrip_trig.attr,
	&cpld_bcm54616_trig.attr,
	&cpld_sensor_trig.attr,
	&cpld_thermaltrip_combine.attr,
	&cpld_bcm54616_combine.attr,
	&cpld_sensor_prochot_combine.attr,
	&cpld_sensor_alert_combine.attr,
	&cpld_thermaltrip_alert.attr,
	&cpld_bcm54616_alert.attr,
	&cpld_ts_prochot_alert.attr,
	&cpld_ts_alert.attr,
	&cpld_thermaltrip_interrupt.attr,
	&cpld_bcm54616_interrupt.attr,
	&cpld_ts_prochot_interrupt.attr,
	&cpld_ts_alert_interrupt.attr,
	&cpld_thermaltrip_mask.attr,
	&cpld_bcm54616_mask.attr,
	&cpld_ts_prochot_mask.attr,
	&cpld_ts_alert_mask.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

/*------------------------------------------------------------------------------
 *
 * module interface
 *
 */
static struct platform_device *cpld_device;

static int cpld_probe(struct platform_device *dev)
{
	int ret;

	cpld_regs = ioport_map(IO_BASE, IO_SIZE);

	if (!cpld_regs) {
		pr_err("cpld: unable to map iomem\n");
		ret = -ENODEV;
		goto err_exit;
	}

	ret = sysfs_create_group(&dev->dev.kobj, &cpld_attr_group);
	if (ret) {
		pr_err("cpld: sysfs_create_group failed for cpld driver");
		goto err_unmap;
	}

	return ret;

err_unmap:
	iounmap(cpld_regs);

err_exit:
	return ret;
}

static int cpld_remove(struct platform_device *dev)
{
	iounmap(cpld_regs);
	return 0;
}

static struct platform_driver cpld_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.probe = cpld_probe,
	.remove = cpld_remove,
};

static int __init cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&cpld_driver);
	if (rv)
		goto err_exit;

	cpld_device = platform_device_alloc(DRIVER_NAME, 0);
	if (!cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device\n");
		rv = -ENOMEM;
		goto err_unregister;
	}

	rv = platform_device_add(cpld_device);
	if (rv) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_dealloc;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;

err_dealloc:
	platform_device_unregister(cpld_device);

err_unregister:
	platform_driver_unregister(&cpld_driver);

err_exit:
	pr_err("%s platform_driver_register failed (%i)\n",
	       DRIVER_NAME, rv);
	return rv;
}

static void __exit cpld_exit(void)
{
	platform_driver_unregister(&cpld_driver);
	platform_device_unregister(cpld_device);
}

module_init(cpld_init);
module_exit(cpld_exit);

MODULE_AUTHOR("David Yen <dhyen@cumulusnetworks.com>");
MODULE_DESCRIPTION("Celestica Haliburton (E1031) MMC Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

