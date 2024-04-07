// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AG9032v2 Master CPLD Driver
 *
 * Copyright (C) 2019, 2020 Cumulus Networks, Inc.  All Rights Reserved.
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
#include <linux/hwmon-sysfs.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "delta-ag9032v2.h"

#define DRIVER_NAME    AG9032V2_MASTERCPLD_NAME
#define DRIVER_VERSION "1.1"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* CPLD register bitfields with enum-like values */

static const char * const port_i2c_oe_values[] = {
	"enabled",  /* 0 */
	"reserved", /* 1 */
	"reserved", /* 2 */
	"disabled", /* 3 */
};

static const char * const led_fan_tray_values[] = {
	PLATFORM_LED_OFF,   /* 0 */
	PLATFORM_LED_GREEN, /* 1 */
	PLATFORM_LED_RED,   /* 2 */
	PLATFORM_LED_OFF,   /* 3 */
};

static const char * const led_fan_and_psu_values[] = {
	PLATFORM_LED_OFF,   /* 0 */
	PLATFORM_LED_GREEN, /* 1 */
	PLATFORM_LED_AMBER, /* 2 */
	PLATFORM_LED_OFF,   /* 3 */
};

static const char * const led_system_values[] = {
	PLATFORM_LED_OFF,            /* 0 */
	PLATFORM_LED_GREEN,          /* 1 */
	PLATFORM_LED_GREEN_BLINKING, /* 2 */
	PLATFORM_LED_RED,            /* 3 */
};

/* CPLD registers */

cpld_bf_ro(platform_type, DELTA_AG9032V2_HW_REVISION_REG,
	   DELTA_AG9032V2_PLATFORM_TYPE, NULL, 0);
cpld_bt_rw(reset_mb, DELTA_AG9032V2_SOFTWARE_RESET_REG,
	   DELTA_AG9032V2_MB_RST_N_P1, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_bmc_lpc, DELTA_AG9032V2_SOFTWARE_RESET_REG,
	   DELTA_AG9032V2_CPLD_LPCRST_P1, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_done, DELTA_AG9032V2_SOFTWARE_RESET_REG,
	   DELTA_AG9032V2_MB_RST_DONE_P1, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_pcie, DELTA_AG9032V2_SOFTWARE_RESET_REG,
	   DELTA_AG9032V2_PCIE_PERST_L_P1, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_cpld, DELTA_AG9032V2_SOFTWARE_RESET_REG,
	   DELTA_AG9032V2_RESET_BUTTON_N_P1, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_bmc, DELTA_AG9032V2_SOFTWARE_RESET_REG,
	   DELTA_AG9032V2_BMC_RSTN_P1, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_bcm56870, DELTA_AG9032V2_SOFTWARE_RESET_REG,
	   DELTA_AG9032V2_RESET_N_B56870_P1, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_usb, DELTA_AG9032V2_SOFTWARE_RESET_REG,
	   DELTA_AG9032V2_RST_USB_HUBN_P1, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_present, DELTA_AG9032V2_POWER_SUPPLY_STATUS_REG,
	   DELTA_AG9032V2_PS1_PRESENT_N_P1, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_all_ok, DELTA_AG9032V2_POWER_SUPPLY_STATUS_REG,
	   DELTA_AG9032V2_PS1_PWR_FAN_OK_P1, NULL, 0);
cpld_bt_ro(psu_pwr1_interrupt, DELTA_AG9032V2_POWER_SUPPLY_STATUS_REG,
	   DELTA_AG9032V2_PS1_PWR_INT_N_P1, NULL, BF_COMPLEMENT);
cpld_bt_rw(psu_pwr1_enable, DELTA_AG9032V2_POWER_SUPPLY_STATUS_REG,
	   DELTA_AG9032V2_PS1_PSON_N_P1, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_present, DELTA_AG9032V2_POWER_SUPPLY_STATUS_REG,
	   DELTA_AG9032V2_PS2_PRESENT_N_P1, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_all_ok, DELTA_AG9032V2_POWER_SUPPLY_STATUS_REG,
	   DELTA_AG9032V2_PS2_PWR_FAN_OK_P1, NULL, 0);
cpld_bt_ro(psu_pwr2_interrupt, DELTA_AG9032V2_POWER_SUPPLY_STATUS_REG,
	   DELTA_AG9032V2_PS2_PWR_INT_N_P1, NULL, BF_COMPLEMENT);
cpld_bt_rw(psu_pwr2_enable, DELTA_AG9032V2_POWER_SUPPLY_STATUS_REG,
	   DELTA_AG9032V2_PS2_PSON_N_P1, NULL, BF_COMPLEMENT);
cpld_bt_rw(mac_1v8_en, DELTA_AG9032V2_POWER_ENABLE_REG,
	   DELTA_AG9032V2_MAC_1V8_EN_P1, NULL, 0);
cpld_bt_rw(mac_avs_0v92_en, DELTA_AG9032V2_POWER_ENABLE_REG,
	   DELTA_AG9032V2_MAC_AVS_0P92V_EN_P1, NULL, 0);
cpld_bt_rw(mb_power_en, DELTA_AG9032V2_POWER_ENABLE_REG,
	   DELTA_AG9032V2_MB_PWR_ENABLE_P1, NULL, 0);
cpld_bt_rw(vcc_3v3_en, DELTA_AG9032V2_POWER_ENABLE_REG,
	   DELTA_AG9032V2_VCC_3V3_EN_P1, NULL, 0);
cpld_bt_rw(vcc_mac_1v25_en, DELTA_AG9032V2_POWER_ENABLE_REG,
	   DELTA_AG9032V2_VCC_MAC_1V25_EN_P1, NULL, 0);
cpld_bt_ro(bmc_1v2_pg, DELTA_AG9032V2_POWER_STATUS_REG,
	   DELTA_AG9032V2_BMC_1V2_PG, NULL, 0);
cpld_bt_ro(bmc_1v15_pg, DELTA_AG9032V2_POWER_STATUS_REG,
	   DELTA_AG9032V2_BMC_1V15_PG, NULL, 0);
cpld_bt_ro(mac_1v8_pg, DELTA_AG9032V2_POWER_STATUS_REG,
	   DELTA_AG9032V2_MAC_1V8_PG_P1, NULL, 0);
cpld_bt_ro(mac_avs_0v92_pg, DELTA_AG9032V2_POWER_STATUS_REG,
	   DELTA_AG9032V2_MAC_AVS_0P92V_PG_P1, NULL, 0);
cpld_bt_ro(mb_pwr_pg, DELTA_AG9032V2_POWER_STATUS_REG,
	   DELTA_AG9032V2_MB_PWR_PGD_P1, NULL, 0);
cpld_bt_ro(vcc_3v3_pg, DELTA_AG9032V2_POWER_STATUS_REG,
	   DELTA_AG9032V2_VCC_3V3_PG_P1, NULL, 0);
cpld_bt_ro(vcc_mac_1v25_pg, DELTA_AG9032V2_POWER_STATUS_REG,
	   DELTA_AG9032V2_VCC_MAC_1V25_PG_P1, NULL, 0);
cpld_bt_ro(bmc_ddr_2v5_pg, DELTA_AG9032V2_POWER_STATUS_REG,
	   DELTA_AG9032V2_BMC_DDR_2V5_PG, NULL, 0);
cpld_bt_ro(fan_alert, DELTA_AG9032V2_INTERRUPT_1_REG,
	   DELTA_AG9032V2_D_FAN_ALERTN_P1, NULL, BF_COMPLEMENT);
cpld_bt_ro(thermal_alert, DELTA_AG9032V2_INTERRUPT_1_REG,
	   DELTA_AG9032V2_THERMAL_OUTN, NULL, BF_COMPLEMENT);
cpld_bt_ro(smb_alert, DELTA_AG9032V2_INTERRUPT_1_REG,
	   DELTA_AG9032V2_SMB_ALERT_P1, NULL, BF_COMPLEMENT);
cpld_bt_ro(smb_alert_1v, DELTA_AG9032V2_INTERRUPT_1_REG,
	   DELTA_AG9032V2_SMB_ALERT_1V_P1, NULL, BF_COMPLEMENT);
cpld_bt_ro(avs_1v_phsflt, DELTA_AG9032V2_INTERRUPT_1_REG,
	   DELTA_AG9032V2_AVS_1V_PHSFLTN, NULL, BF_COMPLEMENT);
cpld_bt_ro(pcie_int, DELTA_AG9032V2_INTERRUPT_1_REG,
	   DELTA_AG9032V2_PCIE_INTR_L, NULL, BF_COMPLEMENT);
cpld_bt_rw(5v_en, DELTA_AG9032V2_INTERRUPT_1_REG,
	   DELTA_AG9032V2_5V_EN, NULL, 0);
cpld_bt_ro(psu_fan_event, DELTA_AG9032V2_INTERRUPT_2_REG,
	   DELTA_AG9032V2_PSU_FAN_EVENTN, NULL, BF_COMPLEMENT);
cpld_bt_ro(op_module_event, DELTA_AG9032V2_INTERRUPT_2_REG,
	   DELTA_AG9032V2_OP_MODULE_EVENTN, NULL, BF_COMPLEMENT);
cpld_bt_ro(misc_int, DELTA_AG9032V2_INTERRUPT_2_REG,
	   DELTA_AG9032V2_MISC_INT, NULL, BF_COMPLEMENT);
cpld_bt_rw(vcc_mac_0v8_en, DELTA_AG9032V2_INTERRUPT_2_REG,
	   DELTA_AG9032V2_VCC_MAC_0P8V_EN, NULL, 0);
cpld_bt_ro(vcc_0v8_pg, DELTA_AG9032V2_INTERRUPT_2_REG,
	   DELTA_AG9032V2_PG_VCC0P8V, NULL, 0);
cpld_bt_ro(serirq_cpld, DELTA_AG9032V2_INTERRUPT_2_REG,
	   DELTA_AG9032V2_SERIRQ_CPLD_P1, NULL, 0);
cpld_bt_ro(irq_phy, DELTA_AG9032V2_INTERRUPT_2_REG,
	   DELTA_AG9032V2_IRQ_PHY_P1, NULL, BF_COMPLEMENT);
cpld_bt_rw(psu_fan_event_mask, DELTA_AG9032V2_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V2_PSU_FAN_EVENTN_MASK, NULL, BF_COMPLEMENT);
cpld_bt_rw(op_module_event_mask, DELTA_AG9032V2_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V2_OP_MODULE_EVENTN_MASK, NULL, BF_COMPLEMENT);
cpld_bt_rw(misc_int_mask, DELTA_AG9032V2_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V2_MISC_INT_MASK, NULL, BF_COMPLEMENT);
cpld_bt_rw(serirq_cpld_mask, DELTA_AG9032V2_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V2_SERIRQ_MASK, NULL, BF_COMPLEMENT);
cpld_bt_rw(irq_phy_mask, DELTA_AG9032V2_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V2_IRQ_PHY_P1_MASK, NULL, BF_COMPLEMENT);
cpld_bf_rw(psu_i2c_sel, DELTA_AG9032V2_PSU_AND_FAN_I2C_MUX_REG,
	   DELTA_AG9032V2_PSU_I2C_SEL, NULL, 0);
cpld_bt_rw(uart_bmc_sel, DELTA_AG9032V2_PSU_AND_FAN_I2C_MUX_REG,
	   DELTA_AG9032V2_UART_SEL, NULL, 0);
cpld_bf_rw(fan_i2c_sel, DELTA_AG9032V2_PSU_AND_FAN_I2C_MUX_REG,
	   DELTA_AG9032V2_FAN_I2C_SEL, NULL, 0);
cpld_bf_rw(port_i2c_oe, DELTA_AG9032V2_QSFP28_SFP_I2C_MUX_REG,
	   DELTA_AG9032V2_PORT_I2C_OE, port_i2c_oe_values, 0);
cpld_bf_rw(port_i2c_sel, DELTA_AG9032V2_QSFP28_SFP_I2C_MUX_REG,
	   DELTA_AG9032V2_PORT_I2C_SEL, NULL, 0);
cpld_bf_rw(led_fan_tray1, DELTA_AG9032V2_SYSTEM_LED_3_REG,
	   DELTA_AG9032V2_FAN_1_LED, led_fan_tray_values, 0);
cpld_bf_rw(led_fan_tray2, DELTA_AG9032V2_FAN_I2C_DATA_CLK_REG,
	   DELTA_AG9032V2_FAN_2_LED, led_fan_tray_values, 0);
cpld_bf_rw(led_fan_tray3, DELTA_AG9032V2_FAN_I2C_DATA_CLK_REG,
	   DELTA_AG9032V2_FAN_3_LED, led_fan_tray_values, 0);
cpld_bf_rw(led_fan_tray4, DELTA_AG9032V2_FAN_I2C_DATA_CLK_REG,
	   DELTA_AG9032V2_FAN_4_LED, led_fan_tray_values, 0);
cpld_bf_rw(led_fan_tray5, DELTA_AG9032V2_FAN_I2C_DATA_CLK_REG,
	   DELTA_AG9032V2_FAN_5_LED, led_fan_tray_values, 0);
cpld_bf_rw(led_psu1, DELTA_AG9032V2_SYSTEM_LED_1_REG,
	   DELTA_AG9032V2_PSU1_LED, led_fan_and_psu_values, 0);
cpld_bf_rw(led_psu2, DELTA_AG9032V2_SYSTEM_LED_1_REG,
	   DELTA_AG9032V2_PSU2_LED, led_fan_and_psu_values, 0);
cpld_bf_rw(led_system, DELTA_AG9032V2_SYSTEM_LED_1_REG,
	   DELTA_AG9032V2_SYS_LED, led_system_values, 0);
cpld_bf_rw(led_fan, DELTA_AG9032V2_SYSTEM_LED_1_REG,
	   DELTA_AG9032V2_FAN_LED, led_fan_and_psu_values, 0);
cpld_bt_rw(test_led1_blue, DELTA_AG9032V2_SYSTEM_LED_2_REG,
	   DELTA_AG9032V2_TLED1_B_P1, NULL, 0);
cpld_bt_rw(test_led1_green, DELTA_AG9032V2_SYSTEM_LED_2_REG,
	   DELTA_AG9032V2_TLED1_G_P1, NULL, 0);
cpld_bt_rw(test_led1_red, DELTA_AG9032V2_SYSTEM_LED_2_REG,
	   DELTA_AG9032V2_TLED1_R_P1, NULL, 0);
cpld_bt_rw(test_led3, DELTA_AG9032V2_SYSTEM_LED_2_REG,
	   DELTA_AG9032V2_TLED3_P1, NULL, 0);
cpld_bt_ro(vrhot, DELTA_AG9032V2_SYSTEM_LED_2_REG,
	   DELTA_AG9032V2_VRHOT_P1, NULL, 0);
cpld_bt_rw(gpio_p1_p2_a, DELTA_AG9032V2_GPIO_1_REG,
	   DELTA_AG9032V2_GPIO_P1_P2_A, NULL, 0);
cpld_bt_rw(gpio_p1_p3_a, DELTA_AG9032V2_GPIO_1_REG,
	   DELTA_AG9032V2_GPIO_P1_P3_A, NULL, 0);
cpld_bt_rw(pld1_pld2_a, DELTA_AG9032V2_GPIO_1_REG,
	   DELTA_AG9032V2_PLD1_PLD2_A, NULL, 0);
cpld_bt_rw(pld1_pld2_b, DELTA_AG9032V2_GPIO_1_REG,
	   DELTA_AG9032V2_PLD1_PLD2_B, NULL, 0);
cpld_bt_rw(pld1_pld3_b, DELTA_AG9032V2_GPIO_1_REG,
	   DELTA_AG9032V2_PLD1_PLD3_B, NULL, 0);
cpld_bt_rw(pld1_pld3_a, DELTA_AG9032V2_GPIO_1_REG,
	   DELTA_AG9032V2_PLD1_PLD3_A, NULL, 0);
cpld_bt_rw(cpld_a_cpld_gpio1, DELTA_AG9032V2_GPIO_1_REG,
	   DELTA_AG9032V2_CPLD_A_CPLD_GPIO1, NULL, 0);
cpld_bt_rw(cpu_cpld1_gpio2_p1, DELTA_AG9032V2_GPIO_1_REG,
	   DELTA_AG9032V2_CPU_CPLD1_GPIO2_P1, NULL, 0);
cpld_bt_rw(cpld_a_cpld_gpio3_p1, DELTA_AG9032V2_GPIO_1_REG,
	   DELTA_AG9032V2_CPLD_A_CPLD_GPIO3_P1, NULL, 0);
cpld_bt_rw(cpld_b_cpld_gpio2_p1, DELTA_AG9032V2_GPIO_2_REG,
	   DELTA_AG9032V2_CPLD_B_CPLD_GPIO2_P1, NULL, 0);
cpld_bt_rw(cpld_b_cpld_gpio3_p1, DELTA_AG9032V2_GPIO_2_REG,
	   DELTA_AG9032V2_CPLD_B_CPLD_GPIO3_P1, NULL, 0);
cpld_bt_rw(cpld_a_cpld_gpio2, DELTA_AG9032V2_GPIO_2_REG,
	   DELTA_AG9032V2_CPLD_A_CPLD_GPIO2, NULL, 0);
cpld_bt_rw(cpld_b_cpld_gpio1, DELTA_AG9032V2_GPIO_2_REG,
	   DELTA_AG9032V2_CPLD_B_CPLD_GPIO1, NULL, 0);
cpld_bt_rw(bmc_gpiol1, DELTA_AG9032V2_GPIO_2_REG,
	   DELTA_AG9032V2_BMC_GPIOL1, NULL, 0);
cpld_bt_rw(bmc_gpiol2, DELTA_AG9032V2_GPIO_2_REG,
	   DELTA_AG9032V2_BMC_GPIOL2, NULL, 0);
cpld_bt_rw(gpioh3, DELTA_AG9032V2_GPIO_2_REG,
	   DELTA_AG9032V2_GPIOH3, NULL, 0);
cpld_bt_rw(mb_a_cpld_rstn, DELTA_AG9032V2_GPIO_3_REG,
	   DELTA_AG9032V2_MB_A_CPLD_RSTN, NULL, 0);
cpld_bt_rw(gpio_phyrst, DELTA_AG9032V2_GPIO_3_REG,
	   DELTA_AG9032V2_GPIO_PHYRST, NULL, 0);
cpld_bt_rw(p1_mpld_1, DELTA_AG9032V2_GPIO_3_REG,
	   DELTA_AG9032V2_P1_MPLD_1, NULL, 0);
cpld_bt_rw(eeprom_wp, DELTA_AG9032V2_GPIO_3_REG,
	   DELTA_AG9032V2_EEPROM_WP, NULL, 0);
cpld_bt_ro(pcie_wake, DELTA_AG9032V2_GPIO_3_REG,
	   DELTA_AG9032V2_PCIE_WAKE_L, NULL, 0);
cpld_bf_ro(cpld_revision, DELTA_AG9032V2_CPLD_REVISION_REG,
	   DELTA_AG9032V2_CPLD_REVISION, NULL, 0);

/* special case for qsfp28 registers */

struct qsfp28_info {
	int reg;
	int active_low;
};

static struct qsfp28_info qsfp28_reg[] = {
	{
		.reg = DELTA_AG9032V2_QSFP28_MODULE_SELECT_1_REG,
		.active_low = 1,
	},
	{
		.reg = DELTA_AG9032V2_QSFP28_LP_MODE_ENABLE_1_REG,
		.active_low = 0,
	},
	{
		.reg = DELTA_AG9032V2_QSFP28_PRESENCE_1_REG,
		.active_low = 1,
	},
	{
		.reg = DELTA_AG9032V2_QSFP28_RESET_1_REG,
		.active_low = 1,
	},
	{
		.reg = DELTA_AG9032V2_QSFP28_INTERRUPT_1_REG,
		.active_low = 1,
	},
};

static ssize_t qsfp_show(struct device *dev, struct device_attribute *dattr,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = to_i2c_client(dev);
	int idx = attr->index;
	int reg = qsfp28_reg[idx].reg;
	int i;
	s32 tmp;
	u64 val = 0;

	for (i = 0; i < NUM_QSFP_REGS; i++) {
		tmp = i2c_smbus_read_byte_data(client, (reg + i));
		if (tmp < 0) {
			pr_err(DRIVER_NAME ": SW CPLD read error - reg: 0x%02X\n",
			       reg);
			return -EINVAL;
		}
		if (qsfp28_reg[idx].active_low)
			tmp = ~tmp;
		/* the registers have the highest numbered port in
		 * the first bit, we have to reverse the order.
		 */
		tmp = (tmp & 0xf0) >> 4 | (tmp & 0x0f) << 4;
		tmp = (tmp & 0xcc) >> 2 | (tmp & 0x33) << 2;
		tmp = (tmp & 0xaa) >> 1 | (tmp & 0x55) << 1;
		val |= ((u64)tmp << (8 * i));
	}
	return sprintf(buf, "0x%08llx\n", val & 0xffffffff);
}

static ssize_t qsfp_store(struct device *dev, struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	struct i2c_client *client = to_i2c_client(dev);
	int idx = attr->index;
	int reg = qsfp28_reg[idx].reg;
	int ret;
	u64 val = 0;
	int i;
	u8 tmp;

	ret = kstrtou64(buf, 0, &val);
	if (ret < 0)
		return ret;

	for (i = 0; i < NUM_QSFP_REGS; i++) {
		tmp = val >> (i * 8) & 0xff;
		if (qsfp28_reg[idx].active_low)
			tmp = ~tmp;
		/* the registers have the highest numbered port in
		 * the first bit, we have to reverse the order.
		 */
		tmp = (tmp & 0xf0) >> 4 | (tmp & 0x0f) << 4;
		tmp = (tmp & 0xcc) >> 2 | (tmp & 0x33) << 2;
		tmp = (tmp & 0xaa) >> 1 | (tmp & 0x55) << 1;
		ret = i2c_smbus_write_byte_data(client, (reg + i), tmp);
		if (ret) {
			pr_err(DRIVER_NAME ": CPLD2 write error - reg: 0x%02X\n",
			       reg);
			return -EINVAL;
		}
	}
	return count;
}

static SENSOR_DEVICE_ATTR_RW(qsfp28_modsel,    qsfp_show, qsfp_store, 0);
static SENSOR_DEVICE_ATTR_RW(qsfp28_lpmode,    qsfp_show, qsfp_store, 1);
static SENSOR_DEVICE_ATTR_RO(qsfp28_present,   qsfp_show, 2);
static SENSOR_DEVICE_ATTR_RW(qsfp28_reset,     qsfp_show, qsfp_store, 3);
static SENSOR_DEVICE_ATTR_RO(qsfp28_interrupt, qsfp_show, 4);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_platform_type.attr,
	&cpld_reset_mb.attr,
	&cpld_reset_bmc_lpc.attr,
	&cpld_reset_done.attr,
	&cpld_reset_pcie.attr,
	&cpld_reset_cpld.attr,
	&cpld_reset_bmc.attr,
	&cpld_reset_bcm56870.attr,
	&cpld_reset_usb.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_psu_pwr1_interrupt.attr,
	&cpld_psu_pwr1_enable.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr2_all_ok.attr,
	&cpld_psu_pwr2_interrupt.attr,
	&cpld_psu_pwr2_enable.attr,
	&cpld_mac_1v8_en.attr,
	&cpld_mac_avs_0v92_en.attr,
	&cpld_mb_power_en.attr,
	&cpld_vcc_3v3_en.attr,
	&cpld_vcc_mac_1v25_en.attr,
	&cpld_bmc_1v2_pg.attr,
	&cpld_bmc_1v15_pg.attr,
	&cpld_mac_1v8_pg.attr,
	&cpld_mac_avs_0v92_pg.attr,
	&cpld_mb_pwr_pg.attr,
	&cpld_vcc_3v3_pg.attr,
	&cpld_vcc_mac_1v25_pg.attr,
	&cpld_bmc_ddr_2v5_pg.attr,
	&cpld_fan_alert.attr,
	&cpld_thermal_alert.attr,
	&cpld_smb_alert.attr,
	&cpld_smb_alert_1v.attr,
	&cpld_avs_1v_phsflt.attr,
	&cpld_pcie_int.attr,
	&cpld_5v_en.attr,
	&cpld_psu_fan_event.attr,
	&cpld_op_module_event.attr,
	&cpld_misc_int.attr,
	&cpld_vcc_mac_0v8_en.attr,
	&cpld_vcc_0v8_pg.attr,
	&cpld_serirq_cpld.attr,
	&cpld_irq_phy.attr,
	&cpld_psu_fan_event_mask.attr,
	&cpld_op_module_event_mask.attr,
	&cpld_misc_int_mask.attr,
	&cpld_serirq_cpld_mask.attr,
	&cpld_irq_phy_mask.attr,
	&cpld_psu_i2c_sel.attr,
	&cpld_uart_bmc_sel.attr,
	&cpld_fan_i2c_sel.attr,
	&cpld_port_i2c_oe.attr,
	&cpld_port_i2c_sel.attr,
	&cpld_led_fan_tray1.attr,
	&cpld_led_fan_tray2.attr,
	&cpld_led_fan_tray3.attr,
	&cpld_led_fan_tray4.attr,
	&cpld_led_fan_tray5.attr,
	&cpld_led_psu1.attr,
	&cpld_led_psu2.attr,
	&cpld_led_system.attr,
	&cpld_led_fan.attr,
	&cpld_test_led1_blue.attr,
	&cpld_test_led1_green.attr,
	&cpld_test_led1_red.attr,
	&cpld_test_led3.attr,
	&cpld_vrhot.attr,
	&cpld_gpio_p1_p2_a.attr,
	&cpld_gpio_p1_p3_a.attr,
	&cpld_pld1_pld2_a.attr,
	&cpld_pld1_pld2_b.attr,
	&cpld_pld1_pld3_b.attr,
	&cpld_pld1_pld3_a.attr,
	&cpld_cpld_a_cpld_gpio1.attr,
	&cpld_cpu_cpld1_gpio2_p1.attr,
	&cpld_cpld_a_cpld_gpio3_p1.attr,
	&cpld_cpld_b_cpld_gpio2_p1.attr,
	&cpld_cpld_b_cpld_gpio3_p1.attr,
	&cpld_cpld_a_cpld_gpio2.attr,
	&cpld_cpld_b_cpld_gpio1.attr,
	&cpld_bmc_gpiol1.attr,
	&cpld_bmc_gpiol2.attr,
	&cpld_gpioh3.attr,
	&cpld_mb_a_cpld_rstn.attr,
	&cpld_gpio_phyrst.attr,
	&cpld_p1_mpld_1.attr,
	&cpld_eeprom_wp.attr,
	&cpld_pcie_wake.attr,
	&cpld_cpld_revision.attr,

	&sensor_dev_attr_qsfp28_modsel.dev_attr.attr,
	&sensor_dev_attr_qsfp28_lpmode.dev_attr.attr,
	&sensor_dev_attr_qsfp28_present.dev_attr.attr,
	&sensor_dev_attr_qsfp28_reset.dev_attr.attr,
	&sensor_dev_attr_qsfp28_interrupt.dev_attr.attr,
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
MODULE_DESCRIPTION("Delta AG9032v2 Master CPLD Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
