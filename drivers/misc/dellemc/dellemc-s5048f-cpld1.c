// SPDX-License-Identifier: GPL-2.0+
/*
 * Dell EMC S5048F CPLD1 Driver
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
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "dellemc-s5048f.h"

#define DRIVER_NAME    S5048F_CPLD1_NAME
#define DRIVER_VERSION "1.0"
#define IO_BASE        CPLD1_IO_BASE
#define IO_SIZE        CPLD1_IO_SIZE

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
		*val |= ioread8(cpld_regs + reg) << bit;
	return 0;
}

static int lpc_write_reg(struct device *dev,
			 int reg,
			 int nregs,
			 u32 val)
{
	for (; nregs > 0; nregs--, reg++, val >>= 8)
		iowrite8(val, cpld_regs + reg);
	return 0;
}

#define cpld_read_reg  lpc_read_reg
#define cpld_write_reg lpc_write_reg

/* CPLD register bitfields with enum-like values */

static const char * const board_type_values[] = {
	"Reserved",     /* 0 */
	"Z9100",        /* 1 */
	"S6100",        /* 2 */
	"Z9100-SFP28",  /* 3 */
	"Z9100-Cavium", /* 4 */
	"S5048F",       /* 5 */
	"Reserved",     /* 6 */
	"Reserved",     /* 7 */
};

static const char * const cpld_id_values[] = {
	"Reserved", /* 0 */
	"CPLD1",    /* 1 */
	"CPLD2",    /* 2 */
	"CPLD3",    /* 3 */
	"CPLD4",    /* 4 */
	"Reserved", /* 5 */
	"Reserved", /* 6 */
	"Reserved", /* 7 */
};

static const char * const led_system_values[] = {
	PLATFORM_LED_GREEN,               /* 0 */
	PLATFORM_LED_GREEN_BLINKING,      /* 1 */
	PLATFORM_LED_AMBER,               /* 2 */
	PLATFORM_LED_AMBER_SLOW_BLINKING, /* 3 */
};

static const char * const led_power_values[] = {
	PLATFORM_LED_GREEN,          /* 0 */
	PLATFORM_LED_AMBER,          /* 1 */
	PLATFORM_LED_AMBER_BLINKING, /* 2 */
	PLATFORM_LED_OFF,            /* 3 */
};

static const char * const led_fan_values[] = {
	PLATFORM_LED_GREEN,          /* 0 */
	PLATFORM_LED_AMBER_BLINKING, /* 1 */
	PLATFORM_LED_OFF,            /* 2 */
	PLATFORM_LED_OFF,            /* 3 */
};

static const char * const led_beacon_values[] = {
	PLATFORM_LED_OFF,           /* 0 */
	PLATFORM_LED_BLUE_BLINKING, /* 1 */
};

static const char * const led_stack_values[] = {
	PLATFORM_LED_OFF,   /* 0 */
	PLATFORM_LED_GREEN, /* 1 */
};

static const char * const trigger_values[] = {
	"falling edge", /* 0 */
	"rising edge",	/* 1 */
	"both edges",	/* 2 */
	"low level",	/* 3 */
};

/* CPLD registers */

cpld_bf_ro(major_version, CPLD_VERSION_REG,
	   CPLD_MAJOR_VER, NULL, 0);
cpld_bf_ro(minor_version, CPLD_VERSION_REG,
	   CPLD_MINOR_VER, NULL, 0);
cpld_bf_ro(board_type, BOARD_TYPE_REG,
	   BOARD_TYPE, board_type_values, 0);
cpld_bf_rw(scratchpad, SW_SCRATCH_REG,
	   SW_SCRATCH, NULL, 0);
cpld_bf_ro(cpld_id, CPLD_ID_REG,
	   CPLD_ID, cpld_id_values, 0);
cpld_bt_rw(reset_pca9548_10, CPLD_SET_RST0_REG,
	   PCA9548_RST10, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pca9548_9, CPLD_SET_RST0_REG,
	   PCA9548_RST9, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pca9548_8, CPLD_SET_RST0_REG,
	   PCA9548_RST8, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pca9548_7, CPLD_SET_RST0_REG,
	   PCA9548_RST7, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pca9548_6, CPLD_SET_RST0_REG,
	   PCA9548_RST6, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pca9548_5, CPLD_SET_RST0_REG,
	   PCA9548_RST5, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pca9548_4, CPLD_SET_RST0_REG,
	   PCA9548_RST4, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pca9548_2, CPLD_SET_RST0_REG,
	   PCA9548_RST2, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pcie, CPLD_SET_RST1_REG,
	   PCIE_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_dpll, CPLD_SET_RST1_REG,
	   DPLL_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_sw, CPLD_SET_RST1_REG,
	   SW_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_fpga, CPLD_SET_RST1_REG,
	   FPGA_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_cpld4, CPLD_SET_RST1_REG,
	   CPLD_RST4, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_cpld3, CPLD_SET_RST1_REG,
	   CPLD_RST3, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_cpld2, CPLD_SET_RST1_REG,
	   CPLD_RST2, NULL, BF_COMPLEMENT);
cpld_bt_rw(eeprom_wp, SW_EEPROM_WP_REG,
	   SW_EEPROM_WP, NULL, 0);
cpld_bf_rw(led_system, SYS_LED_CTRL_REG,
	   SYS_STA_LED, led_system_values, 0);
cpld_bf_rw(led_power, SYS_LED_CTRL_REG,
	   PWR_STA_LED, led_power_values, 0);
cpld_bf_rw(led_fan, SYS_LED_CTRL_REG,
	   FAN_STA_LED, led_fan_values, 0);
cpld_bt_rw(led_beacon, SYS_LED_CTRL_REG,
	   BEACON_LED, led_beacon_values, 0);
cpld_bt_rw(led_stack, SYS_LED_CTRL_REG,
	   STACKING_LED, led_stack_values, 0);
cpld_bf_rw(led_7_segment, SEG7_LED_CTRL_REG,
	   SEG7_LED_CTL, NULL, 0);
cpld_bf_rw(misc_int_trig, MISC_TRIG_MOD_REG,
	   MISC_INT_TRIG, trigger_values, 0);
cpld_bt_ro(misc_int_combine, MISC_COMBINE_REG,
	   MISC_INT_COMBINE, NULL, 0);
cpld_bt_ro(ir3581_alert_status, MISC_STA_REG,
	   IR3581_ALERT_N_STA, NULL, 0);
cpld_bt_ro(ir3584_1p0v_alert_status, MISC_STA_REG,
	   IR3584_1P0V_ALERT_N_STA, NULL, 0);
cpld_bt_ro(ir3584_3p3v_alert_status, MISC_STA_REG,
	   IR3584_3P3V_ALERT_N_STA, NULL, 0);
cpld_bt_ro(ir3584_3p3v_vrhot_status, MISC_STA_REG,
	   IR3584_3P3V_VRHOT_STA, NULL, 0);
cpld_bt_ro(bcm54616s_int_status, MISC_STA_REG,
	   BCM54616S_INT_STA, NULL, 0);
cpld_bt_ro(ir3581_alert_int, MISC_INT_REG,
	   IR3581_ALERT_N_INT, NULL, 0);
cpld_bt_ro(ir3584_1p0v_alert_int, MISC_INT_REG,
	   IR3584_1P0V_ALERT_N_INT, NULL, 0);
cpld_bt_ro(ir3584_3p3v_alert_int, MISC_INT_REG,
	   IR3584_3P3V_ALERT_N_INT, NULL, 0);
cpld_bt_ro(ir3584_3p3v_vrhot_int, MISC_INT_REG,
	   IR3584_3P3V_VRHOT_INT, NULL, 0);
cpld_bt_ro(bcm54616s_int_int, MISC_INT_REG,
	   BCM54616S_INT_INT, NULL, 0);
cpld_bt_rw(ir3581_alert_mask, MISC_INT_MASK_REG,
	   IR3581_ALERT_N_MASK, NULL, 0);
cpld_bt_rw(ir3584_1p0v_alert_mask, MISC_INT_MASK_REG,
	   IR3584_1P0V_ALERT_N_MASK, NULL, 0);
cpld_bt_rw(ir3584_3p3v_alert_mask, MISC_INT_MASK_REG,
	   IR3584_3P3V_ALERT_N_MASK, NULL, 0);
cpld_bt_rw(ir3584_3p3v_vrhot_mask, MISC_INT_MASK_REG,
	   IR3584_3P3V_VRHOT_MASK, NULL, 0);
cpld_bt_rw(bcm54616s_int_mask, MISC_INT_MASK_REG,
	   BCM54616S_INT_MASK, NULL, 0);
cpld_bt_ro(cpld4_sfp28_rxlos_int, OPTICAL_INT_STA_REG,
	   CPLD4_SFP28_RXLOS_INT_STA, NULL, 0);
cpld_bt_ro(cpld4_sfp28_present_int, OPTICAL_INT_STA_REG,
	   CPLD4_SFP28_ABS_INT_STA, NULL, 0);
cpld_bt_ro(cpld4_qsfp28_interrupt_int, OPTICAL_INT_STA_REG,
	   CPLD4_INT_INT_STA, NULL, 0);
cpld_bt_ro(cpld4_qsfp28_present_int, OPTICAL_INT_STA_REG,
	   CPLD4_ABS_INT_STA, NULL, 0);
cpld_bt_ro(cpld3_sfp28_rxlos_int, OPTICAL_INT_STA_REG,
	   CPLD3_SFP28_RXLOS_INT_STA, NULL, 0);
cpld_bt_ro(cpld3_sfp28_present_int, OPTICAL_INT_STA_REG,
	   CPLD3_ABS_INT_STA, NULL, 0);
cpld_bt_ro(cpld2_sfp28_rxlos_int, OPTICAL_INT_STA_REG,
	   CPLD2_SFP28_RXLOS_INT_STA, NULL, 0);
cpld_bt_ro(cpld2_sfp28_present_int, OPTICAL_INT_STA_REG,
	   CPLD2_ABS_INT_STA, NULL, 0);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_major_version.attr,
	&cpld_minor_version.attr,
	&cpld_board_type.attr,
	&cpld_scratchpad.attr,
	&cpld_cpld_id.attr,
	&cpld_reset_pca9548_10.attr,
	&cpld_reset_pca9548_9.attr,
	&cpld_reset_pca9548_8.attr,
	&cpld_reset_pca9548_7.attr,
	&cpld_reset_pca9548_6.attr,
	&cpld_reset_pca9548_5.attr,
	&cpld_reset_pca9548_4.attr,
	&cpld_reset_pca9548_2.attr,
	&cpld_reset_pcie.attr,
	&cpld_reset_dpll.attr,
	&cpld_reset_sw.attr,
	&cpld_reset_fpga.attr,
	&cpld_reset_cpld4.attr,
	&cpld_reset_cpld3.attr,
	&cpld_reset_cpld2.attr,
	&cpld_eeprom_wp.attr,
	&cpld_led_system.attr,
	&cpld_led_power.attr,
	&cpld_led_fan.attr,
	&cpld_led_beacon.attr,
	&cpld_led_stack.attr,
	&cpld_led_7_segment.attr,
	&cpld_misc_int_trig.attr,
	&cpld_misc_int_combine.attr,
	&cpld_ir3581_alert_status.attr,
	&cpld_ir3584_1p0v_alert_status.attr,
	&cpld_ir3584_3p3v_alert_status.attr,
	&cpld_ir3584_3p3v_vrhot_status.attr,
	&cpld_bcm54616s_int_status.attr,
	&cpld_ir3581_alert_int.attr,
	&cpld_ir3584_1p0v_alert_int.attr,
	&cpld_ir3584_3p3v_alert_int.attr,
	&cpld_ir3584_3p3v_vrhot_int.attr,
	&cpld_bcm54616s_int_int.attr,
	&cpld_ir3581_alert_mask.attr,
	&cpld_ir3584_1p0v_alert_mask.attr,
	&cpld_ir3584_3p3v_alert_mask.attr,
	&cpld_ir3584_3p3v_vrhot_mask.attr,
	&cpld_bcm54616s_int_mask.attr,
	&cpld_cpld4_sfp28_rxlos_int.attr,
	&cpld_cpld4_sfp28_present_int.attr,
	&cpld_cpld4_qsfp28_interrupt_int.attr,
	&cpld_cpld4_qsfp28_present_int.attr,
	&cpld_cpld3_sfp28_rxlos_int.attr,
	&cpld_cpld3_sfp28_present_int.attr,
	&cpld_cpld2_sfp28_rxlos_int.attr,
	&cpld_cpld2_sfp28_present_int.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

/* module interface */

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

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC S5048F CPLD1 Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

