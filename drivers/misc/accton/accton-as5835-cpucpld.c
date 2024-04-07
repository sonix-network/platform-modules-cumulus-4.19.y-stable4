// SPDX-License-Identifier: GPL-2.0+
/*
 * Accton AS5835 CPU CPLD Driver
 *
 * Copyright (c) 2019, 2020 Cumulus Networks, Inc.  All rights reserved.
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
#include <linux/hwmon.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "accton-as5835.h"

#define DRIVER_NAME    AS5835_CPUCPLD_NAME
#define DRIVER_VERSION "1.0"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* cpld register bitfields with enum-like values */

static const char * const cpu_id_values[] = {
	"C3958",
	"C3558",
};

/* CPLD registers */

cpld_bt_ro(cpu_id, ACCTON_AS5835_CPUCPLD_BOARD_INFO_REG,
	   ACCTON_AS5835_CPUCPLD_CPU_ID, cpu_id_values, 0);
cpld_bf_ro(pcb_version, ACCTON_AS5835_CPUCPLD_BOARD_INFO_REG,
	   ACCTON_AS5835_CPUCPLD_PCB_VERSION, NULL, 0);
cpld_bf_ro(cpld_version, ACCTON_AS5835_CPUCPLD_VERSION_REG,
	   ACCTON_AS5835_CPUCPLD_VERSION, NULL, 0);
cpld_bt_ro(spi_cs_select, ACCTON_AS5835_CPUCPLD_WATCHDOG_STATUS_1_REG,
	   ACCTON_AS5835_CPUCPLD_SPI_CS_SELECT, NULL, 0);
cpld_bt_rw(watchdog_en, ACCTON_AS5835_CPUCPLD_WATCHDOG_STATUS_2_REG,
	   ACCTON_AS5835_CPUCPLD_WATCHDOG_ENABLE, NULL, 0);
cpld_bt_rw(update_boot_dev_sel, ACCTON_AS5835_CPUCPLD_WATCHDOG_STATUS_2_REG,
	   ACCTON_AS5835_CPUCPLD_UPDATE_BOOT_DEV_SEL, NULL, 0);
cpld_bt_rw(boot_dev_sel, ACCTON_AS5835_CPUCPLD_WATCHDOG_STATUS_2_REG,
	   ACCTON_AS5835_CPUCPLD_BOOT_DEV_SEL, NULL, 0);
cpld_bt_rw(watchdog_trigger, ACCTON_AS5835_CPUCPLD_WATCHDOG_STATUS_2_REG,
	   ACCTON_AS5835_CPUCPLD_WATCHDOG_TRIGGER, NULL, 0);
cpld_bt_rw(test_mode, ACCTON_AS5835_CPUCPLD_WATCHDOG_STATUS_2_REG,
	   ACCTON_AS5835_CPUCPLD_TEST_MODE, NULL, 0);
cpld_bt_rw(system_sleep, ACCTON_AS5835_CPUCPLD_SYSTEM_STATE_CONTROL_REG,
	   ACCTON_AS5835_CPUCPLD_SYSTEM_SLEEP, NULL, BF_COMPLEMENT);
cpld_bt_rw(system_reset, ACCTON_AS5835_CPUCPLD_SYSTEM_STATE_CONTROL_REG,
	   ACCTON_AS5835_CPUCPLD_SYSTEM_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(system_power_off, ACCTON_AS5835_CPUCPLD_SYSTEM_STATE_CONTROL_REG,
	   ACCTON_AS5835_CPUCPLD_SYSTEM_POWER_OFF, NULL, BF_COMPLEMENT);
cpld_bt_rw(system_power_reset, ACCTON_AS5835_CPUCPLD_SYSTEM_STATE_CONTROL_REG,
	   ACCTON_AS5835_CPUCPLD_SYSTEM_POWER_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_bmc_lpc_debug, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_1_REG,
	   ACCTON_AS5835_CPUCPLD_BMC_LPC_DEBUG_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_tpm, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_1_REG,
	   ACCTON_AS5835_CPUCPLD_TPM_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_ltb, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_1_REG,
	   ACCTON_AS5835_CPUCPLD_LTB_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_m_2_ssd2, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_1_REG,
	   ACCTON_AS5835_CPUCPLD_M_2_SSD_2_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_m_2_ssd1, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_1_REG,
	   ACCTON_AS5835_CPUCPLD_M_2_SSD_1_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_main_board_jtag, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_1_REG,
	   ACCTON_AS5835_CPUCPLD_MAIN_BOARD_JTAG_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pca9548, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_1_REG,
	   ACCTON_AS5835_CPUCPLD_PCA9548_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_main_board_syscpld, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_2_REG,
	   ACCTON_AS5835_CPUCPLD_MAIN_BOARD_SYS_CPLD_RESET, NULL,
	   BF_COMPLEMENT);
cpld_bt_rw(reset_main_board_mac, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_2_REG,
	   ACCTON_AS5835_CPUCPLD_MAIN_BOARD_MAX_RESET, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_main_board_p1014, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_2_REG,
	   ACCTON_AS5835_CPUCPLD_MAIN_BOARD_P1014_RESET, NULL, BF_COMPLEMENT);
cpld_bt_ro(power_button, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_3_REG,
	   ACCTON_AS5835_CPUCPLD_POWER_BUTTON, NULL, BF_COMPLEMENT);
cpld_bt_ro(reset_button, ACCTON_AS5835_CPUCPLD_RESET_DEVICE_3_REG,
	   ACCTON_AS5835_CPUCPLD_RESET_BUTTON, NULL, BF_COMPLEMENT);
cpld_bt_ro(interrupt_to_bmc, ACCTON_AS5835_CPUCPLD_INTERRUPT_STATE_REG,
	   ACCTON_AS5835_CPUCPLD_INTERRUPT_TO_BMC, NULL, BF_COMPLEMENT);
cpld_bt_ro(interrupt_to_cpu, ACCTON_AS5835_CPUCPLD_INTERRUPT_STATE_REG,
	   ACCTON_AS5835_CPUCPLD_INTERRUPT_TO_CPU, NULL, BF_COMPLEMENT);
cpld_bt_ro(nmi_event_from_bmc, ACCTON_AS5835_CPUCPLD_INTERRUPT_STATE_REG,
	   ACCTON_AS5835_CPUCPLD_NMI_EVENT_FROM_BMC, NULL, BF_COMPLEMENT);
cpld_bt_ro(main_board_cpld23_interrupt,
	   ACCTON_AS5835_CPUCPLD_INTERRUPT_STATE_REG,
	   ACCTON_AS5835_CPUCPLD_MAIN_BOARD_CPLD23_INTERRUPT, NULL,
	   BF_COMPLEMENT);
cpld_bt_ro(main_board_syscpld_interrupt,
	   ACCTON_AS5835_CPUCPLD_INTERRUPT_STATE_REG,
	   ACCTON_AS5835_CPUCPLD_MAIN_BOARD_SYS_CPLD_INTERRUPT, NULL,
	   BF_COMPLEMENT);
cpld_bt_rw(interrupt_flag_to_bmc, ACCTON_AS5835_CPUCPLD_INTERRUPT_IF_REG,
	   ACCTON_AS5835_CPUCPLD_INTERRUPT_TO_BMC, NULL, BF_COMPLEMENT);
cpld_bt_rw(interrupt_flag_to_cpu, ACCTON_AS5835_CPUCPLD_INTERRUPT_IF_REG,
	   ACCTON_AS5835_CPUCPLD_INTERRUPT_TO_CPU, NULL, BF_COMPLEMENT);
cpld_bf_rw(interrupt_to_bmc_or_cpu_mask,
	   ACCTON_AS5835_CPUCPLD_INTERRUPT_MASK_REG,
	   ACCTON_AS5835_CPUCPLD_INTERRUPT_TO_BMC_OR_CPU_MASK, NULL,
	   BF_COMPLEMENT);
cpld_bt_rw(nmi_event_from_bmc_mask, ACCTON_AS5835_CPUCPLD_INTERRUPT_MASK_REG,
	   ACCTON_AS5835_CPUCPLD_NMI_EVENT_FROM_BMC_MASK, NULL, BF_COMPLEMENT);
cpld_bt_rw(main_board_cpld23_interrupt_mask,
	   ACCTON_AS5835_CPUCPLD_INTERRUPT_MASK_REG,
	   ACCTON_AS5835_CPUCPLD_MAIN_BOARD_CPLD23_INTERRUPT_MASK, NULL,
	   BF_COMPLEMENT);
cpld_bt_rw(main_board_syscpld_interrupt_mask,
	   ACCTON_AS5835_CPUCPLD_INTERRUPT_MASK_REG,
	   ACCTON_AS5835_CPUCPLD_MAIN_BOARD_SYS_CPLD_INTERRUPT_MASK, NULL,
	   BF_COMPLEMENT);
cpld_bt_rw(uart1_selection, ACCTON_AS5835_CPUCPLD_UART_SELECTION_REG,
	   ACCTON_AS5835_CPUCPLD_UART1_SELECTION, NULL, 0);
cpld_bt_rw(uart2_selectiong, ACCTON_AS5835_CPUCPLD_UART_SELECTION_REG,
	   ACCTON_AS5835_CPUCPLD_UART0_SELECTION, NULL, 0);
cpld_bt_ro(tps53622_hot_pvnn, ACCTON_AS5835_CPUCPLD_THERMAL_STATUS_REG,
	   ACCTON_AS5835_CPUCPLD_TPS53622_HOT_PVNN, NULL, BF_COMPLEMENT);
cpld_bt_ro(tps53622_hot_pvccp, ACCTON_AS5835_CPUCPLD_THERMAL_STATUS_REG,
	   ACCTON_AS5835_CPUCPLD_TPS53622_HOT_PVCCP, NULL, BF_COMPLEMENT);
cpld_bt_ro(tps53622_hot_pvccram, ACCTON_AS5835_CPUCPLD_THERMAL_STATUS_REG,
	   ACCTON_AS5835_CPUCPLD_TPS53622_HOT_PVCCRAM, NULL, BF_COMPLEMENT);
cpld_bt_ro(cpu_prochot, ACCTON_AS5835_CPUCPLD_THERMAL_STATUS_REG,
	   ACCTON_AS5835_CPUCPLD_CPU_PROC_HOT, NULL, BF_COMPLEMENT);
cpld_bt_ro(cpu_thermtrip, ACCTON_AS5835_CPUCPLD_THERMAL_STATUS_REG,
	   ACCTON_AS5835_CPUCPLD_CPU_THERMAL_TRIP, NULL, BF_COMPLEMENT);
cpld_bt_rw(bmc_usb1_pwrfault, ACCTON_AS5835_CPUCPLD_BMC_STATUS_REG,
	   ACCTON_AS5835_CPUCPLD_BMC_USB1_PWRFAULT, NULL, BF_COMPLEMENT);
cpld_bt_ro(bmc_usb1_drvvmus, ACCTON_AS5835_CPUCPLD_BMC_STATUS_REG,
	   ACCTON_AS5835_CPUCPLD_BMC_USB1_DRVVBUS, NULL, BF_COMPLEMENT);
cpld_bt_rw(bmc_control_spi_source_mask,
	   ACCTON_AS5835_CPUCPLD_BMC_CONTROL_SPI_FLASH_MASK_REG,
	   ACCTON_AS5835_CPUCPLD_BMC_CONTROL_SPI_SOURCE_MASK, NULL,
	   BF_COMPLEMENT);
cpld_bt_ro(usb3_oc, ACCTON_AS5835_CPUCPLD_USB_PROTECT_REG,
	   ACCTON_AS5835_CPUCPLD_USB3_OC, NULL, BF_COMPLEMENT);
cpld_bt_ro(usb1_pwrfault, ACCTON_AS5835_CPUCPLD_USB_PROTECT_REG,
	   ACCTON_AS5835_CPUCPLD_USB1_PWRFAULT, NULL, BF_COMPLEMENT);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_cpu_id.attr,
	&cpld_pcb_version.attr,
	&cpld_cpld_version.attr,
	&cpld_spi_cs_select.attr,
	&cpld_watchdog_en.attr,
	&cpld_update_boot_dev_sel.attr,
	&cpld_boot_dev_sel.attr,
	&cpld_watchdog_trigger.attr,
	&cpld_test_mode.attr,
	&cpld_system_sleep.attr,
	&cpld_system_reset.attr,
	&cpld_system_power_off.attr,
	&cpld_system_power_reset.attr,
	&cpld_reset_bmc_lpc_debug.attr,
	&cpld_reset_tpm.attr,
	&cpld_reset_ltb.attr,
	&cpld_reset_m_2_ssd2.attr,
	&cpld_reset_m_2_ssd1.attr,
	&cpld_reset_main_board_jtag.attr,
	&cpld_reset_pca9548.attr,
	&cpld_reset_main_board_syscpld.attr,
	&cpld_reset_main_board_mac.attr,
	&cpld_reset_main_board_p1014.attr,
	&cpld_power_button.attr,
	&cpld_reset_button.attr,
	&cpld_interrupt_to_bmc.attr,
	&cpld_interrupt_to_cpu.attr,
	&cpld_nmi_event_from_bmc.attr,
	&cpld_main_board_cpld23_interrupt.attr,
	&cpld_main_board_syscpld_interrupt.attr,
	&cpld_interrupt_flag_to_bmc.attr,
	&cpld_interrupt_flag_to_cpu.attr,
	&cpld_interrupt_to_bmc_or_cpu_mask.attr,
	&cpld_nmi_event_from_bmc_mask.attr,
	&cpld_main_board_cpld23_interrupt_mask.attr,
	&cpld_main_board_syscpld_interrupt_mask.attr,
	&cpld_uart1_selection.attr,
	&cpld_uart2_selectiong.attr,
	&cpld_tps53622_hot_pvnn.attr,
	&cpld_tps53622_hot_pvccp.attr,
	&cpld_tps53622_hot_pvccram.attr,
	&cpld_cpu_prochot.attr,
	&cpld_cpu_thermtrip.attr,
	&cpld_bmc_usb1_pwrfault.attr,
	&cpld_bmc_usb1_drvvmus.attr,
	&cpld_bmc_control_spi_source_mask.attr,
	&cpld_usb3_oc.attr,
	&cpld_usb1_pwrfault.attr,

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
					ACCTON_AS5835_CPUCPLD_VERSION_REG);
	if (temp < 0) {
		dev_err(dev, "read CPU CPLD version register error: %d\n",
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
	dev_info(dev, "device probed, CPU CPLD rev: %lu\n",
		 GET_FIELD(temp, ACCTON_AS5835_CPUCPLD_VERSION));

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
MODULE_DESCRIPTION("Accton AS5835 CPU CPLD Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
