/*
 * Celestica Seastone2 and Questone2 Base Board CPLD Driver
 *
 * Copyright (c) 2019 Cumulus Networks, Inc.  All rights reserved.
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

#include <stddef.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include "platform-defs.h"
#include "platform-bitfield.h"

#define DRIVER_NAME "cel_sea2que2_base_board_cpld"
#define DRIVER_VERSION "0.4"

#define CPLD_IO_BASE 0xa000
#define CPLD_IO_SIZE 0x02ff

static uint8_t *cpld_regs;

/* bitfield accessor functions */

static int lpccpld_read_reg(struct device *dev, int reg, int nregs, u32 *val)
{
	int nbits = nregs * 8;
	int bit;

	*val = 0;
	for (bit = 0; bit < nbits; bit += 8, reg++)
		*val |= ioread8(cpld_regs + reg) << bit;
	return 0;
}

static int lpccpld_write_reg(struct device *dev, int reg, int nregs, u32 val)
{
	for (; nregs > 0; nregs--, reg++, val >>= 8)
		iowrite8(val, cpld_regs + reg);
	return 0;
}

/* CPLD register bitfields with enum-like values */

static const char * const uart_values[] = {
	"bmc",
	"com-e",
};

static const char * const direction_values[] = {
	"F2B",
	"B2F",
};

static const char * const psu_led_values[] = {
	"amber",
	"hw_control",
};

static const char * const sys_led_values[] = {
	"green_yellow",
	"green",
	"yellow",
	"off",
};

static const char * const alarm_led_values[] = {
	"off",
	"green",
	"yellow",
	"off",
};

static const char * const blink_values[] = {
	"off",
	"1Hz",
	"4Hz",
	"on",
};

/* CPLD registers */

mk_bf_ro(lpccpld, cpldb_major_version,	      0x100, 4, 4, NULL, 0);
mk_bf_ro(lpccpld, cpldb_minor_version,	      0x100, 0, 4, NULL, 0);
mk_bf_rw(lpccpld, cpldb_scratch,	      0x101, 0, 8, NULL, 0);

mk_bf_rw(lpccpld, bmc_reset,		      0x104, 4, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, pca9548_reset,	      0x104, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, i210_reset,		      0x104, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, switch_board_reset,	      0x104, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_rw(lpccpld, rear_panel_card_present,    0x108, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, bmc_card_present,	      0x108, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_rw(lpccpld, rear_panel_uart,	      0x10c, 0, 1, uart_values, 0);

mk_bf_rw(lpccpld, com_e_power_enable,	      0x120, 0, 1, NULL, 0);
mk_bf_rw(lpccpld, com_e_reset,		      0x121, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, com_e_type,		      0x125, 0, 3, NULL, 0);

mk_bf_rw(lpccpld, system_eeprom_wp,	      0x131, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, tlv_eeprom_wp,	      0x131, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, bmc_bios_spi1_wp,	      0x131, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, bmc_bios_spi0_wp,	      0x131, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_rw(lpccpld, fan_watchdog_enable,	      0x134, 0, 1, NULL, 0);

mk_bf_rw(lpccpld, fan1_pwm,		      0x140, 0, 8, NULL, 0);
mk_bf_ro(lpccpld, fan1_direction,	      0x141, 3, 1, direction_values, 0);
mk_bf_ro(lpccpld, fan1_present,		      0x141, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, led_fan1_green,	      0x141, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, led_fan1_red,		      0x141, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, fan1r_input,		      0x142, 0, 8, NULL, 0);
mk_bf_ro(lpccpld, fan1_input,		      0x143, 0, 8, NULL, 0);

mk_bf_rw(lpccpld, fan2_pwm,		      0x144, 0, 8, NULL, 0);
mk_bf_ro(lpccpld, fan2_direction,	      0x145, 3, 1, direction_values, 0);
mk_bf_ro(lpccpld, fan2_present,		      0x145, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, led_fan2_green,	      0x145, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, led_fan2_red,		      0x145, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, fan2r_input,		      0x146, 0, 8, NULL, 0);
mk_bf_ro(lpccpld, fan2_input,		      0x147, 0, 8, NULL, 0);

mk_bf_rw(lpccpld, fan3_pwm,		      0x148, 0, 8, NULL, 0);
mk_bf_ro(lpccpld, fan3_direction,	      0x149, 3, 1, direction_values, 0);
mk_bf_ro(lpccpld, fan3_present,		      0x149, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, led_fan3_green,	      0x149, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, led_fan3_red,		      0x149, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, fan3r_input,		      0x14a, 0, 8, NULL, 0);
mk_bf_ro(lpccpld, fan3_input,		      0x14b, 0, 8, NULL, 0);

mk_bf_rw(lpccpld, fan4_pwm,		      0x14c, 0, 8, NULL, 0);
mk_bf_ro(lpccpld, fan4_direction,	      0x14d, 3, 1, direction_values, 0);
mk_bf_ro(lpccpld, fan4_present,		      0x14d, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, led_fan4_green,	      0x14d, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, led_fan4_red,		      0x14d, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, fan4r_input,		      0x14e, 0, 8, NULL, 0);
mk_bf_ro(lpccpld, fan4_input,		      0x14f, 0, 8, NULL, 0);

mk_bf_rw(lpccpld, fan5_pwm,		      0x150, 0, 8, NULL, 0);
mk_bf_ro(lpccpld, fan5_direction,	      0x151, 3, 1, direction_values, 0);
mk_bf_ro(lpccpld, fan5_present,		      0x151, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, led_fan5_green,	      0x151, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, led_fan5_red,		      0x151, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, fan5r_input,		      0x152, 0, 8, NULL, 0);
mk_bf_ro(lpccpld, fan5_input,		      0x153, 0, 8, NULL, 0);

mk_bf_ro(lpccpld, psu_pwr1_alert,	      0x160, 7, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, psu_pwr2_alert,	      0x160, 6, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, psu_pwr1_present,	      0x160, 5, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, psu_pwr2_present,	      0x160, 4, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, psu_pwr1_all_ok,	      0x160, 3, 1, NULL, 0);
mk_bf_ro(lpccpld, psu_pwr2_all_ok,	      0x160, 2, 1, NULL, 0);
mk_bf_rw(lpccpld, psu_pwr1_enable,	      0x160, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, psu_pwr2_enable,	      0x160, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_rw(lpccpld, psu_pwr2_led_control,	      0x161, 1, 1, psu_led_values, 0);
mk_bf_rw(lpccpld, psu_pwr1_led_control,	      0x161, 0, 1, psu_led_values, 0);

mk_bf_rw(lpccpld, system_led_select,	      0x162, 4, 2, sys_led_values, 0);
mk_bf_rw(lpccpld, system_led_control,	      0x162, 0, 2, blink_values, 0);

mk_bf_rw(lpccpld, alarm_led_select,	      0x163, 4, 2, alarm_led_values, 0);
mk_bf_rw(lpccpld, alarm_led_control,	      0x163, 0, 2, blink_values, 0);

mk_bf_rw(lpccpld, cpu_power_cycle,	      0x164, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, boot_status,		      0x170, 0, 1, NULL, 0);
mk_bf_rw(lpccpld, clear_boot_counter,	      0x171, 0, 8, NULL, 0);
mk_bf_rw(lpccpld, thermal_shutdown_enable,    0x175, 0, 1, NULL, 0);

mk_bf_ro(lpccpld, thermal_shutdown_event,     0x176, 4, 1, NULL, 0);
mk_bf_ro(lpccpld, switch_board_lm75_status,   0x176, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, base_board_lm75_2_status,   0x176, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, base_board_lm75_1_status,   0x176, 0, 1, NULL, BF_COMPLEMENT);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&lpccpld_cpldb_major_version.attr,
	&lpccpld_cpldb_minor_version.attr,
	&lpccpld_cpldb_scratch.attr,
	&lpccpld_bmc_reset.attr,
	&lpccpld_pca9548_reset.attr,
	&lpccpld_i210_reset.attr,
	&lpccpld_switch_board_reset.attr,
	&lpccpld_rear_panel_card_present.attr,
	&lpccpld_bmc_card_present.attr,
	&lpccpld_rear_panel_uart.attr,
	&lpccpld_com_e_power_enable.attr,
	&lpccpld_com_e_reset.attr,
	&lpccpld_com_e_type.attr,
	&lpccpld_system_eeprom_wp.attr,
	&lpccpld_tlv_eeprom_wp.attr,
	&lpccpld_bmc_bios_spi1_wp.attr,
	&lpccpld_bmc_bios_spi0_wp.attr,
	&lpccpld_fan_watchdog_enable.attr,
	&lpccpld_fan1_pwm.attr,
	&lpccpld_fan1_direction.attr,
	&lpccpld_fan1_present.attr,
	&lpccpld_led_fan1_green.attr,
	&lpccpld_led_fan1_red.attr,
	&lpccpld_fan1r_input.attr,
	&lpccpld_fan1_input.attr,
	&lpccpld_fan2_pwm.attr,
	&lpccpld_fan2_direction.attr,
	&lpccpld_fan2_present.attr,
	&lpccpld_led_fan2_green.attr,
	&lpccpld_led_fan2_red.attr,
	&lpccpld_fan2r_input.attr,
	&lpccpld_fan2_input.attr,
	&lpccpld_fan3_pwm.attr,
	&lpccpld_fan3_direction.attr,
	&lpccpld_fan3_present.attr,
	&lpccpld_led_fan3_green.attr,
	&lpccpld_led_fan3_red.attr,
	&lpccpld_fan3r_input.attr,
	&lpccpld_fan3_input.attr,
	&lpccpld_fan4_pwm.attr,
	&lpccpld_fan4_direction.attr,
	&lpccpld_fan4_present.attr,
	&lpccpld_led_fan4_green.attr,
	&lpccpld_led_fan4_red.attr,
	&lpccpld_fan4r_input.attr,
	&lpccpld_fan4_input.attr,
	&lpccpld_fan5_pwm.attr,
	&lpccpld_fan5_direction.attr,
	&lpccpld_fan5_present.attr,
	&lpccpld_led_fan5_green.attr,
	&lpccpld_led_fan5_red.attr,
	&lpccpld_fan5r_input.attr,
	&lpccpld_fan5_input.attr,
	&lpccpld_psu_pwr1_alert.attr,
	&lpccpld_psu_pwr2_alert.attr,
	&lpccpld_psu_pwr1_present.attr,
	&lpccpld_psu_pwr2_present.attr,
	&lpccpld_psu_pwr1_all_ok.attr,
	&lpccpld_psu_pwr2_all_ok.attr,
	&lpccpld_psu_pwr1_enable.attr,
	&lpccpld_psu_pwr2_enable.attr,
	&lpccpld_psu_pwr2_led_control.attr,
	&lpccpld_psu_pwr1_led_control.attr,
	&lpccpld_system_led_select.attr,
	&lpccpld_system_led_control.attr,
	&lpccpld_alarm_led_select.attr,
	&lpccpld_alarm_led_control.attr,
	&lpccpld_cpu_power_cycle.attr,
	&lpccpld_boot_status.attr,
	&lpccpld_clear_boot_counter.attr,
	&lpccpld_thermal_shutdown_enable.attr,
	&lpccpld_thermal_shutdown_event.attr,
	&lpccpld_switch_board_lm75_status.attr,
	&lpccpld_base_board_lm75_2_status.attr,
	&lpccpld_base_board_lm75_1_status.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

/* driver probe */

static int cpld_probe(struct platform_device *dev)
{
	int ret = 0;

	cpld_regs = ioport_map(CPLD_IO_BASE, CPLD_IO_SIZE);

	if (!cpld_regs) {
		pr_err(DRIVER_NAME ": failed to map iomem\n");
		ret = -ENODEV;
		goto err_exit;
	}

	ret = sysfs_create_group(&dev->dev.kobj, &cpld_attr_group);
	if (ret) {
		pr_err(DRIVER_NAME ": sysfs_create_group failed\n");
		goto err_unmap;
	}
	return ret;

err_unmap:
	iounmap(cpld_regs);

err_exit:
	return ret;
}

static int cpld_remove(struct platform_device *ofdev)
{
	struct kobject *kobj = &ofdev->dev.kobj;

	iounmap(cpld_regs);
	sysfs_remove_group(kobj, &cpld_attr_group);

	platform_set_drvdata(ofdev, NULL);
	dev_info(&ofdev->dev, "removed\n");
	return 0;
}

static struct platform_driver cpld_driver = {
	.probe = cpld_probe,
	.remove = cpld_remove,
	.driver = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

/* device init */

static struct platform_device *cpld_device;

static int __init cel_sea2que2_base_board_cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&cpld_driver);
	if (rv)
		goto err_exit;

	cpld_device = platform_device_alloc(DRIVER_NAME, 0);
	if (!cpld_device) {
		pr_err(DRIVER_NAME ": platform_device_alloc() failed: %i\n",
		       rv);
		rv = -ENOMEM;
		goto err_unregister;
	}

	rv = platform_device_add(cpld_device);
	if (rv) {
		pr_err(DRIVER_NAME ": platform_device_add() failed: %i\n", rv);
	     goto err_dealloc;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;

err_dealloc:
	platform_device_unregister(cpld_device);
	platform_driver_unregister(&cpld_driver);

err_unregister:
	platform_driver_unregister(&cpld_driver);

err_exit:
	pr_err(DRIVER_NAME ": platform_driver_register failed: %i\n", rv);

	return rv;
}

static void __exit cel_sea2que2_base_board_cpld_exit(void)
{
	platform_device_unregister(cpld_device);
	platform_driver_unregister(&cpld_driver);
}

module_init(cel_sea2que2_base_board_cpld_init);
module_exit(cel_sea2que2_base_board_cpld_exit);

MODULE_AUTHOR("David Yen <dhyen@cumulusnetworks.com>");
MODULE_DESCRIPTION("Celestica Seastone2 and Questone2 Base Board CPLD Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

