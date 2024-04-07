// SPDX-License-Identifier: GPL-2.0+
/*
 * Lenovo NE2580 CPLD1 Driver
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

#include <linux/module.h>
#include <linux/hwmon-sysfs.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "lenovo-ne2580.h"

#define DRIVER_NAME	   NE2580_CPLD1_NAME
#define DRIVER_VERSION	   "1.1"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* cpld register bitfields with enum-like values */

static const char * const green_led_values[] = {
	PLATFORM_LED_OFF,                 /* 0 */
	PLATFORM_LED_GREEN,               /* 1 */
	PLATFORM_LED_GREEN_SLOW_BLINKING, /* 2 */
	PLATFORM_LED_GREEN_BLINKING,      /* 3 */
};

static const char * const blue_led_values[] = {
	PLATFORM_LED_OFF,                /* 0 */
	PLATFORM_LED_BLUE,               /* 1 */
	PLATFORM_LED_BLUE_SLOW_BLINKING, /* 2 */
	PLATFORM_LED_BLUE_BLINKING,      /* 3 */
};

static const char * const system_direction_values[] = {
	"B2F", /* 0 */
	"F2B", /* 1 */
};

static const char * const diag_mode_color_values[] = {
	PLATFORM_LED_GREEN, /* 0 */
	PLATFORM_LED_RED,   /* 1 */
};

static const char * const reset_button_values[] = {
	"not pressed",     /* 0 */
	"reserved",        /* 1 */
	"pressed, < 5 sec" /* 2 */
	"held, > 5 sec",   /* 3 */
};

/* CPLD registers */

cpld_bt_ro(cpld1_system_airflow, LENOVO_NE2580_CPLD1_PCB_VERSION_REG,
	   LENOVO_NE2580_CPLD1_SYS_AIR_DIR, system_direction_values, 0);
cpld_bf_ro(cpld1_triton_model, LENOVO_NE2580_CPLD1_PCB_VERSION_REG,
	   LENOVO_NE2580_CPLD1_TRITON_MODEL_SEL, NULL, 0);
cpld_bf_ro(hw_version, LENOVO_NE2580_CPLD1_PCB_VERSION_REG,
	   LENOVO_NE2580_CPLD1_HW_VERSION, NULL, 0);

cpld_bt_rw(cpu_spi_cs, LENOVO_NE2580_CPLD1_BIOS_CHIP_SELECT_REG,
	   LENOVO_NE2580_CPLD1_CPU_SPI_CS, NULL, 0);

cpld_bt_rw(cpld1_system_ready, LENOVO_NE2580_CPLD1_SWITCH_READY_REG,
	   LENOVO_NE2580_CPLD1_SYS_RDY, NULL, 0);

cpld_bt_rw(diag_mode_sfp28, LENOVO_NE2580_CPLD1_I2C_DIAG_MODE_SETTING_REG,
	   LENOVO_NE2580_CPLD1_SFP28_DIAG_MODE, NULL, 0);
cpld_bt_rw(diag_mode_sfp28_color,
	   LENOVO_NE2580_CPLD1_I2C_DIAG_MODE_SETTING_REG,
	   LENOVO_NE2580_CPLD1_SFP28_DIAG_MODE_COLOR,
	   diag_mode_color_values, 0);
cpld_bt_rw(diag_mode_qsfp28,
	   LENOVO_NE2580_CPLD1_I2C_DIAG_MODE_SETTING_REG,
	   LENOVO_NE2580_CPLD1_QSFP28_DIAG_MODE, NULL, 0);
cpld_bt_rw(diag_mode_qsfp28_color,
	   LENOVO_NE2580_CPLD1_I2C_DIAG_MODE_SETTING_REG,
	   LENOVO_NE2580_CPLD1_QSFP28_DIAG_MODE_COLOR,
	   diag_mode_color_values, 0);
cpld_bt_rw(diag_mode_qsfp28_sfp28_amber,
	   LENOVO_NE2580_CPLD1_I2C_DIAG_MODE_SETTING_REG,
	   LENOVO_NE2580_CPLD1_QSFP28_SFP28_DIAG_MODE_AMBER, NULL, 0);

cpld_rg_rw(diag_mode_sfp28_8_1_led,
	   LENOVO_NE2580_CPLD1_SFP28_8_1_LED_DIAG_MODE_REG, NULL, 0);
cpld_rg_rw(diag_mode_sfp28_16_9_led,
	   LENOVO_NE2580_CPLD1_SFP28_16_9_LED_DIAG_MODE_REG, NULL, 0);
cpld_rg_rw(diag_mode_sfp28_24_17_led,
	   LENOVO_NE2580_CPLD1_SFP28_24_17_LED_DIAG_MODE_REG, NULL, 0);
cpld_rg_rw(diag_mode_sfp28_32_25_led,
	   LENOVO_NE2580_CPLD1_SFP28_32_25_LED_DIAG_MODE_REG, NULL, 0);
cpld_rg_rw(diag_mode_sfp28_40_33_led,
	   LENOVO_NE2580_CPLD1_SFP28_40_33_LED_DIAG_MODE_REG, NULL, 0);
cpld_rg_rw(diag_mode_sfp28_48_41_led,
	   LENOVO_NE2580_CPLD1_SFP28_48_41_LED_DIAG_MODE_REG, NULL, 0);

cpld_rg_rw(diag_mode_qsfp28_50_49_led,
	   LENOVO_NE2580_CPLD1_QSFP28_2_1_LED_DIAG_MODE_REG, NULL, 0);
cpld_rg_rw(diag_mode_qsfp28_52_51_led,
	   LENOVO_NE2580_CPLD1_QSFP28_4_3_LED_DIAG_MODE_REG, NULL, 0);
cpld_rg_rw(diag_mode_qsfp28_54_53_led,
	   LENOVO_NE2580_CPLD1_QSFP28_6_5_LED_DIAG_MODE_REG, NULL, 0);
cpld_rg_rw(diag_mode_qsfp28_56_55_led,
	   LENOVO_NE2580_CPLD1_QSFP28_8_7_LED_DIAG_MODE_REG, NULL, 0);

cpld_bf_rw(led_system, LENOVO_NE2580_CPLD1_SYSTEM_STATUS_LED_CONTROL_REG,
	   LENOVO_NE2580_CPLD1_SVR_LED, blue_led_values, 0);
cpld_bf_rw(led_psu, LENOVO_NE2580_CPLD1_SYSTEM_STATUS_LED_CONTROL_REG,
	   LENOVO_NE2580_CPLD1_PWR_LED, green_led_values, 0);
cpld_bf_rw(led_fan, LENOVO_NE2580_CPLD1_SYSTEM_STATUS_LED_CONTROL_REG,
	   LENOVO_NE2580_CPLD1_FAN_LED, green_led_values, 0);
cpld_bf_rw(led_stack, LENOVO_NE2580_CPLD1_SYSTEM_STATUS_LED_CONTROL_REG,
	   LENOVO_NE2580_CPLD1_STK_LED, green_led_values, 0);

cpld_bt_ro(int_ucd90160_temp, LENOVO_NE2580_CPLD1_INTERRUPT_INDICATE_REG,
	   LENOVO_NE2580_CPLD1_UCD90160_TEMP_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(int_pld_sen3, LENOVO_NE2580_CPLD1_INTERRUPT_INDICATE_REG,
	   LENOVO_NE2580_CPLD1_PLD_SEN3_ALERT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(int_pld_sen4, LENOVO_NE2580_CPLD1_INTERRUPT_INDICATE_REG,
	   LENOVO_NE2580_CPLD1_PLD_SEN4_ALERT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(int_pld_sen5, LENOVO_NE2580_CPLD1_INTERRUPT_INDICATE_REG,
	   LENOVO_NE2580_CPLD1_PLD_SEN5_ALERT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(int_ext_usb_oc, LENOVO_NE2580_CPLD1_INTERRUPT_INDICATE_REG,
	   LENOVO_NE2580_CPLD1_EXT_USB_OC_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(int_cpu_sen, LENOVO_NE2580_CPLD1_INTERRUPT_INDICATE_REG,
	   LENOVO_NE2580_CPLD1_CPU_SEN_ALERT_N, NULL, BF_COMPLEMENT);

cpld_bt_ro(int_rstbtn_5s, LENOVO_NE2580_CPLD1_INTERRUPT_2_INDICATE_REG,
	   LENOVO_NE2580_CPLD1_RSTBTN_5S_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(int_wdt_irq, LENOVO_NE2580_CPLD1_INTERRUPT_2_INDICATE_REG,
	   LENOVO_NE2580_CPLD1_WDT_IRQ_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(int_rstbtn, LENOVO_NE2580_CPLD1_INTERRUPT_2_INDICATE_REG,
	   LENOVO_NE2580_CPLD1_RSTBTN_INT_N, NULL, BF_COMPLEMENT);

cpld_bt_rw(int_mask_rstbtn_5s, LENOVO_NE2580_CPLD1_INTERRUPT_2_MASK_REG,
	   LENOVO_NE2580_CPLD1_RSTBTN_5S_INT_N_MASK, NULL, 0);
cpld_bt_rw(int_mask_wdt_irq, LENOVO_NE2580_CPLD1_INTERRUPT_2_MASK_REG,
	   LENOVO_NE2580_CPLD1_WDT_IRQ_N_MASK, NULL, 0);
cpld_bt_rw(int_mask_rstbtn, LENOVO_NE2580_CPLD1_INTERRUPT_2_MASK_REG,
	   LENOVO_NE2580_CPLD1_RSTBTN_INT_N_MASK, NULL, 0);

cpld_bt_rw(factory_default_bios, LENOVO_NE2580_CPLD1_SYSTEM_CONTROL_REG,
	   LENOVO_NE2580_CPLD1_BDX_RTCRST_R0_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(bios_spi_wp1, LENOVO_NE2580_CPLD1_SYSTEM_CONTROL_REG,
	   LENOVO_NE2580_CPLD1_BIOS_SPI_WP1_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(bios_spi_wp0, LENOVO_NE2580_CPLD1_SYSTEM_CONTROL_REG,
	   LENOVO_NE2580_CPLD1_BIOS_SPI_WP0_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(halt_test_en, LENOVO_NE2580_CPLD1_SYSTEM_CONTROL_REG,
	   LENOVO_NE2580_CPLD1_HALT_TEST_EN_N, NULL, BF_COMPLEMENT);

cpld_bt_rw(psu_12v_ctrl, LENOVO_NE2580_CPLD1_SYSTEM_POWER_CONTROL_REG,
	   LENOVO_NE2580_CPLD1_PSU_12V_CTRL, NULL, 0);

cpld_bt_rw(reset_i2c_mux7, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_1_REG,
	   LENOVO_NE2580_CPLD1_RST_I2C_MUX7_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_i2c_mux6, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_1_REG,
	   LENOVO_NE2580_CPLD1_RST_I2C_MUX6_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_i2c_mux5, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_1_REG,
	   LENOVO_NE2580_CPLD1_RST_I2C_MUX5_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_i2c_mux4, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_1_REG,
	   LENOVO_NE2580_CPLD1_RST_I2C_MUX4_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_i2c_mux3, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_1_REG,
	   LENOVO_NE2580_CPLD1_RST_I2C_MUX3_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_i2c_mux2, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_1_REG,
	   LENOVO_NE2580_CPLD1_RST_I2C_MUX2_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_i2c_mux1, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_1_REG,
	   LENOVO_NE2580_CPLD1_RST_I2C_MUX1_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_i2c_mux0, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_1_REG,
	   LENOVO_NE2580_CPLD1_RST_I2C_MUX0_N, NULL, BF_COMPLEMENT);

cpld_bt_rw(reset_ucd90160, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_2_REG,
	   LENOVO_NE2580_CPLD1_RST_UCD90160_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_usb_hub, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_2_REG,
	   LENOVO_NE2580_CPLD1_RST_USB_HUB_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_i21x, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_2_REG,
	   LENOVO_NE2580_CPLD1_RST_I21X_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_bcm56873, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_2_REG,
	   LENOVO_NE2580_CPLD1_PLTRST_BCM56873_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pcie, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_2_REG,
	   LENOVO_NE2580_CPLD1_PCIE_RESET_L, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_usb1_hub, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_2_REG,
	   LENOVO_NE2580_CPLD1_RST_USB1_HUB_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_cpu, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_2_REG,
	   LENOVO_NE2580_CPLD1_RST_CPU_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pca9543, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_2_REG,
	   LENOVO_NE2580_CPLD1_PCA9543_RESET_N, NULL, BF_COMPLEMENT);

cpld_bf_ro(reset_button_status, LENOVO_NE2580_CPLD1_RESET_BUTTON_STATUS_REG,
	   LENOVO_NE2580_CPLD1_RB_ST, reset_button_values, 0);

cpld_bt_rw(reason_temp, LENOVO_NE2580_CPLD1_RESET_CAUSE_REG,
	   LENOVO_NE2580_CPLD1_SYS_THERMAL_TRIP, NULL, 0);
cpld_bt_rw(reason_panic, LENOVO_NE2580_CPLD1_RESET_CAUSE_REG,
	   LENOVO_NE2580_CPLD1_PANIC, NULL, 0);
cpld_bt_rw(reason_button, LENOVO_NE2580_CPLD1_RESET_CAUSE_REG,
	   LENOVO_NE2580_CPLD1_RB_RST, NULL, 0);
cpld_bt_rw(reason_software, LENOVO_NE2580_CPLD1_RESET_CAUSE_REG,
	   LENOVO_NE2580_CPLD1_SW_RST, NULL, 0);
cpld_bt_rw(reason_watchdog, LENOVO_NE2580_CPLD1_RESET_CAUSE_REG,
	   LENOVO_NE2580_CPLD1_WD_RST, NULL, 0);
cpld_bt_rw(reason_poweroff, LENOVO_NE2580_CPLD1_RESET_CAUSE_REG,
	   LENOVO_NE2580_CPLD1_PO_RST, NULL, 0);

cpld_bf_ro(wd_counter, LENOVO_NE2580_CPLD1_WATCHDOG_CURRENT_COUNTER_REG,
	   LENOVO_NE2580_CPLD1_WATCHDOG_CURRENT_COUNTER, NULL, 0);
cpld_bf_rw(wd_timer, LENOVO_NE2580_CPLD1_WATCHDOG_TIMER_REG,
	   LENOVO_NE2580_CPLD1_WATCHDOG_TIMER, NULL, 0);
cpld_bt_rw(wd_clr, LENOVO_NE2580_CPLD1_WATCHDOG_ENABLE_REG,
	   LENOVO_NE2580_CPLD1_WD_CLR, NULL, 0);
cpld_bt_rw(wd_en, LENOVO_NE2580_CPLD1_WATCHDOG_ENABLE_REG,
	   LENOVO_NE2580_CPLD1_WD_EN, NULL, 0);

cpld_bt_rw(reset_clk_at, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_6_REG,
	   LENOVO_NE2580_CPLD1_RST_CLK_AT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(system_reset_sel, LENOVO_NE2580_CPLD1_SYSTEM_RESET_CONTROL_6_REG,
	   LENOVO_NE2580_CPLD1_SYSTEM_RST_SEL, NULL, BF_COMPLEMENT);

cpld_bf_rw(panic_code, LENOVO_NE2580_CPLD1_PANIC_REG,
	   LENOVO_NE2580_CPLD1_PANIC_CODE, NULL, 0);

/* special case for cpld version register */

static ssize_t cpld1_version_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	s32 temp;
	int reg = LENOVO_NE2580_CPLD1_VERSION_REG;

	temp = i2c_smbus_read_byte_data(client, reg);
	if (temp < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, reg);
		return -EINVAL;
	}

	return sprintf(buf, "%d.%d\n",
		       (int)GET_FIELD(temp, LENOVO_NE2580_CPLD1_MAJOR_VERSION),
		       (int)GET_FIELD(temp, LENOVO_NE2580_CPLD1_MINOR_VERSION));
}

static SENSOR_DEVICE_ATTR_RO(cpld1_version, cpld1_version_show, 0);

/* special case for cpld date registers */

static ssize_t cpld1_date_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	s32 temp1;
	int reg1 = LENOVO_NE2580_CPLD1_RELEASE_DATE_1_REG;
	s32 temp2;
	int reg2 = LENOVO_NE2580_CPLD1_RELEASE_DATE_2_REG;

	temp1 = i2c_smbus_read_byte_data(client, reg1);
	if (temp1 < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, reg1);
		return -EINVAL;
	}
	temp2 = i2c_smbus_read_byte_data(client, reg2);
	if (temp2 < 0) {
		pr_err("CPLD read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, reg2);
		return -EINVAL;
	}

	return sprintf(buf, "%d/%d/%d\n",
		       (int)GET_FIELD(temp1, LENOVO_NE2580_CPLD1_RELEASE_MONTH),
		       (int)GET_FIELD(temp2, LENOVO_NE2580_CPLD1_RELEASE_DATE),
		       (int)GET_FIELD(temp1, LENOVO_NE2580_CPLD1_RELEASE_YEAR) +
		       2014);
}

static SENSOR_DEVICE_ATTR_RO(cpld1_date, cpld1_date_show, 0);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_cpld1_system_airflow.attr,
	&cpld_cpld1_triton_model.attr,
	&cpld_hw_version.attr,
	&cpld_cpu_spi_cs.attr,
	&cpld_cpld1_system_ready.attr,
	&cpld_led_system.attr,
	&cpld_led_psu.attr,
	&cpld_led_fan.attr,
	&cpld_led_stack.attr,
	&cpld_diag_mode_sfp28.attr,
	&cpld_diag_mode_sfp28_color.attr,
	&cpld_diag_mode_qsfp28.attr,
	&cpld_diag_mode_qsfp28_color.attr,
	&cpld_diag_mode_qsfp28_sfp28_amber.attr,
	&cpld_diag_mode_sfp28_8_1_led.attr,
	&cpld_diag_mode_sfp28_16_9_led.attr,
	&cpld_diag_mode_sfp28_24_17_led.attr,
	&cpld_diag_mode_sfp28_32_25_led.attr,
	&cpld_diag_mode_sfp28_40_33_led.attr,
	&cpld_diag_mode_sfp28_48_41_led.attr,
	&cpld_diag_mode_qsfp28_50_49_led.attr,
	&cpld_diag_mode_qsfp28_52_51_led.attr,
	&cpld_diag_mode_qsfp28_54_53_led.attr,
	&cpld_diag_mode_qsfp28_56_55_led.attr,
	&cpld_int_ucd90160_temp.attr,
	&cpld_int_pld_sen3.attr,
	&cpld_int_pld_sen4.attr,
	&cpld_int_pld_sen5.attr,
	&cpld_int_ext_usb_oc.attr,
	&cpld_int_cpu_sen.attr,
	&cpld_int_rstbtn_5s.attr,
	&cpld_int_wdt_irq.attr,
	&cpld_int_rstbtn.attr,
	&cpld_int_mask_rstbtn_5s.attr,
	&cpld_int_mask_wdt_irq.attr,
	&cpld_int_mask_rstbtn.attr,
	&cpld_factory_default_bios.attr,
	&cpld_bios_spi_wp1.attr,
	&cpld_bios_spi_wp0.attr,
	&cpld_halt_test_en.attr,
	&cpld_psu_12v_ctrl.attr,
	&cpld_reset_i2c_mux7.attr,
	&cpld_reset_i2c_mux6.attr,
	&cpld_reset_i2c_mux5.attr,
	&cpld_reset_i2c_mux4.attr,
	&cpld_reset_i2c_mux3.attr,
	&cpld_reset_i2c_mux2.attr,
	&cpld_reset_i2c_mux1.attr,
	&cpld_reset_i2c_mux0.attr,
	&cpld_reset_ucd90160.attr,
	&cpld_reset_usb_hub.attr,
	&cpld_reset_i21x.attr,
	&cpld_reset_bcm56873.attr,
	&cpld_reset_pcie.attr,
	&cpld_reset_usb1_hub.attr,
	&cpld_reset_cpu.attr,
	&cpld_reset_pca9543.attr,
	&cpld_reset_button_status.attr,
	&cpld_reason_temp.attr,
	&cpld_reason_panic.attr,
	&cpld_reason_button.attr,
	&cpld_reason_software.attr,
	&cpld_reason_watchdog.attr,
	&cpld_reason_poweroff.attr,
	&cpld_wd_counter.attr,
	&cpld_wd_timer.attr,
	&cpld_wd_en.attr,
	&cpld_wd_clr.attr,
	&cpld_reset_clk_at.attr,
	&cpld_system_reset_sel.attr,
	&cpld_panic_code.attr,

	&sensor_dev_attr_cpld1_version.dev_attr.attr,
	&sensor_dev_attr_cpld1_date.dev_attr.attr,

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
					LENOVO_NE2580_CPLD1_VERSION_REG);
	if (temp < 0) {
		pr_err(DRIVER_NAME ": read CPLD1 version register error: %d\n",
		       temp);
		ret = temp;
		goto err;
	}

	/* create sysfs node */
	ret = sysfs_create_group(&dev->kobj, &cpld_attr_group);
	if (ret) {
		pr_err(DRIVER_NAME ": failed to create sysfs group for cpld device\n");
		goto err;
	}

	/* all clear */
	dev_info(dev, "device probed, CPLD1 rev: %d.%d\n",
		 (int)GET_FIELD(temp, LENOVO_NE2580_CPLD1_MAJOR_VERSION),
		 (int)GET_FIELD(temp, LENOVO_NE2580_CPLD1_MINOR_VERSION));

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

/* module init and exit */

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
MODULE_DESCRIPTION("Lenovo NE2580 CPLD1 Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
