// SPDX-License-Identifier: GPL-2.0+
/*
 * Delta AG9032v1 SW CPLD Driver
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
#include <linux/hwmon-sysfs.h>

#include "platform-defs.h"
#include "platform-bitfield.h"
#include "delta-ag9032v1.h"

#define DRIVER_NAME    AG9032V1_SWCPLD_NAME
#define DRIVER_VERSION "1.1"

/* bitfield accessor functions */

#define cpld_read_reg  cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/* CPLD register bitfields with enum-like values */

static const char * const led_psu_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER_BLINKING,
	PLATFORM_LED_OFF,
};

static const char * const led_system_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_GREEN_BLINKING,
	PLATFORM_LED_RED,
};

static const char * const led_fan_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_OFF,
};

static const char * const led_fan_tray_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_RED,
	PLATFORM_LED_OFF,
};

/* CPLD registers */

cpld_bf_ro(swcpld_board_id, DELTA_AG9032V1_SWCPLD_BOARD_ID_REG,
	   DELTA_AG9032V1_SWCPLD_BOARD_ID, NULL, 0);
cpld_bf_ro(swcpld_board_version, DELTA_AG9032V1_SWCPLD_BOARD_ID_REG,
	   DELTA_AG9032V1_SWCPLD_BOARD_VERSION, NULL, 0);
cpld_bf_ro(swcpld_version, DELTA_AG9032V1_SWCPLD_VERSION_REG,
	   DELTA_AG9032V1_SWITCH_CPLD_VERSION, NULL, 0);
cpld_bt_rw(reset_sw, DELTA_AG9032V1_SWCPLD_SOFTWARE_RESET_1_REG,
	   DELTA_AG9032V1_SW_RST, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_bcm56960, DELTA_AG9032V1_SWCPLD_SOFTWARE_RESET_1_REG,
	   DELTA_AG9032V1_RESET_N_B56960, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_bcm54616s, DELTA_AG9032V1_SWCPLD_SOFTWARE_RESET_1_REG,
	   DELTA_AG9032V1_RESET_N_B54616S, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_mb_a_cpld, DELTA_AG9032V1_SWCPLD_SOFTWARE_RESET_1_REG,
	   DELTA_AG9032V1_MB_A_CPLD_RESET_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(reset_mb_b_cpld, DELTA_AG9032V1_SWCPLD_SOFTWARE_RESET_1_REG,
	   DELTA_AG9032V1_MB_B_CPLD_RESET_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(ps1_pson, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_PS1_PSON_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(ps2_pson, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_PS2_PSON_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(hot_swap1_en, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_HOTSWAP1_EN_N, NULL, 0);
cpld_bt_rw(hot_swap2_en, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_HOTSWAP2_EN_N, NULL, 0);
cpld_bt_rw(vcc_5v_en, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_VCC_5V_EN, NULL, 0);
cpld_bt_rw(mac_1v8_en, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_MAC_1V8_EN, NULL, 0);
cpld_bt_rw(mac_1v25_en, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_MAC_1V25_EN, NULL, 0);
cpld_bt_rw(mac_avs_1v_en, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_1_REG,
	   DELTA_AG9032V1_MAC_AVS_1V_EN, NULL, 0);
cpld_bt_rw(mac_1v_en, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_2_REG,
	   DELTA_AG9032V1_MAC_1V_EN, NULL, 0);
cpld_bt_ro(psu_pwr1_all_ok, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_3_REG,
	   DELTA_AG9032V1_PS1_PWR_FAN_OK, NULL, 0);
cpld_bt_ro(psu_pwr2_all_ok, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_3_REG,
	   DELTA_AG9032V1_PS2_PWR_FAN_OK, NULL, 0);
cpld_bt_ro(hot_swap1_pg, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_3_REG,
	   DELTA_AG9032V1_HOT_SWAP_PG1, NULL, 0);
cpld_bt_ro(hot_swap2_pg, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_3_REG,
	   DELTA_AG9032V1_HOT_SWAP_PG2, NULL, 0);
cpld_bt_ro(vcc_5v_pg, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_3_REG,
	   DELTA_AG9032V1_VCC_5V_PG, NULL, 0);
cpld_bt_ro(mac_1v8_pg, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_3_REG,
	   DELTA_AG9032V1_MAC_1V8_PG, NULL, 0);
cpld_bt_ro(vcc_mac_1v25_pg, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_3_REG,
	   DELTA_AG9032V1_VCC_MAC_1V25_PG, NULL, 0);
cpld_bt_ro(mac_avs_1v_pg, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_3_REG,
	   DELTA_AG9032V1_MAC_AVS_1V_PG, NULL, 0);
cpld_bt_ro(mac_1v_pg, DELTA_AG9032V1_SWCPLD_POWER_STATUS_CONTROL_4_REG,
	   DELTA_AG9032V1_MAC_1V_PG, NULL, 0);
cpld_bt_ro(hot_swap1_int, DELTA_AG9032V1_SWCPLD_INTERRUPT_0_REG,
	   DELTA_AG9032V1_HOT_SWAP_INT1, NULL, BF_COMPLEMENT);
cpld_bt_ro(hot_swap2_int, DELTA_AG9032V1_SWCPLD_INTERRUPT_0_REG,
	   DELTA_AG9032V1_HOT_SWAP_INT2, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr1_int, DELTA_AG9032V1_SWCPLD_INTERRUPT_0_REG,
	   DELTA_AG9032V1_PS1_PWR_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(psu_pwr2_int, DELTA_AG9032V1_SWCPLD_INTERRUPT_0_REG,
	   DELTA_AG9032V1_PS2_PWR_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(fan_alert, DELTA_AG9032V1_SWCPLD_INTERRUPT_0_REG,
	   DELTA_AG9032V1_FAN_ALERT_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(hot_swap1_int_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_0_REG,
	   DELTA_AG9032V1_HOT_SWAP_INT1_MASK, NULL, 0);
cpld_bt_rw(hot_swap2_int_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_0_REG,
	   DELTA_AG9032V1_HOT_SWAP_INT2_MASK, NULL, 0);
cpld_bt_rw(psu_pwr1_int_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_0_REG,
	   DELTA_AG9032V1_PS1_PWR_INT_N_MASK, NULL, 0);
cpld_bt_rw(psu_pwr2_int_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_0_REG,
	   DELTA_AG9032V1_PS2_PWR_INT_N_MASK, NULL, 0);
cpld_bt_rw(fan_alert_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_0_REG,
	   DELTA_AG9032V1_FAN_ALERT_N_MASK, NULL, 0);
cpld_bt_ro(qsfp_1_8_int, DELTA_AG9032V1_SWCPLD_INTERRUPT_1_REG,
	   DELTA_AG9032V1_QSFP_1_8_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(qsfp_9_16_int, DELTA_AG9032V1_SWCPLD_INTERRUPT_1_REG,
	   DELTA_AG9032V1_QSFP_9_16_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(qsfp_17_24_int, DELTA_AG9032V1_SWCPLD_INTERRUPT_1_REG,
	   DELTA_AG9032V1_QSFP_17_24_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(qsfp_25_32_int, DELTA_AG9032V1_SWCPLD_INTERRUPT_1_REG,
	   DELTA_AG9032V1_QSFP_25_32_INT_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(qsfp_1_8_abs, DELTA_AG9032V1_SWCPLD_INTERRUPT_1_REG,
	   DELTA_AG9032V1_QSFP_1_8_ABS_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(qsfp_9_17_abs, DELTA_AG9032V1_SWCPLD_INTERRUPT_1_REG,
	   DELTA_AG9032V1_QSFP_9_16_ABS_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(qsfp_17_24_abs, DELTA_AG9032V1_SWCPLD_INTERRUPT_1_REG,
	   DELTA_AG9032V1_QSFP_17_24_ABS_N, NULL, BF_COMPLEMENT);
cpld_bt_ro(qsfp_25_32_abs, DELTA_AG9032V1_SWCPLD_INTERRUPT_1_REG,
	   DELTA_AG9032V1_QSFP_25_32_ABS_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(qsfp_1_8_int_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V1_QSFP_1_8_INT_N_MASK, NULL, 0);
cpld_bt_rw(qsfp_9_16_int_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V1_QSFP_9_16_INT_N_MASK, NULL, 0);
cpld_bt_rw(qsfp_17_24_int_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V1_QSFP_17_24_INT_N_MASK, NULL, 0);
cpld_bt_rw(qsfp_25_32_int_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V1_QSFP_25_32_INT_N_MASK, NULL, 0);
cpld_bt_rw(qsfp_1_8_abs_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V1_QSFP_1_8_ABS_N_MASK, NULL, 0);
cpld_bt_rw(qsfp_9_16_abs_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V1_QSFP_9_16_ABS_N_MASK, NULL, 0);
cpld_bt_rw(qsfp_17_24_abs_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V1_QSFP_17_24_ABS_N_MASK, NULL, 0);
cpld_bt_rw(qsfp_25_32_abs_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_1_REG,
	   DELTA_AG9032V1_QSFP_25_32_ABS_N_MASK, NULL, 0);
cpld_bt_ro(bcm54616s_irq, DELTA_AG9032V1_SWCPLD_INTERRUPT_3_REG,
	   DELTA_AG9032V1_BCM54616S_IRQ_N, NULL, BF_COMPLEMENT);
cpld_bt_rw(bcm54616s_irq_mask, DELTA_AG9032V1_SWCPLD_INTERRUPT_MASK_3_REG,
	   DELTA_AG9032V1_BCM54616S_IRQ_N_MASK, NULL, 0);
cpld_bf_rw(led_system, DELTA_AG9032V1_SWCPLD_SYSTEM_LED_REG,
	   DELTA_AG9032V1_SYS_LED, led_system_values, 0);
cpld_bf_rw(led_fan, DELTA_AG9032V1_SWCPLD_SYSTEM_LED_REG,
	   DELTA_AG9032V1_FAN_LED, led_fan_values, 0);
cpld_bf_rw(led_psu1, DELTA_AG9032V1_SWCPLD_SYSTEM_LED_REG,
	   DELTA_AG9032V1_PSU1_LED, led_psu_values, 0);
cpld_bf_rw(led_psu2, DELTA_AG9032V1_SWCPLD_SYSTEM_LED_REG,
	   DELTA_AG9032V1_PSU2_LED, led_psu_values, 0);
cpld_bf_rw(led_fan_tray1, DELTA_AG9032V1_SWCPLD_FAN_TRAY_LED_1_REG,
	   DELTA_AG9032V1_FAN1_LED, led_fan_tray_values, 0);
cpld_bf_rw(led_fan_tray2, DELTA_AG9032V1_SWCPLD_FAN_TRAY_LED_1_REG,
	   DELTA_AG9032V1_FAN2_LED, led_fan_tray_values, 0);
cpld_bf_rw(led_fan_tray3, DELTA_AG9032V1_SWCPLD_FAN_TRAY_LED_1_REG,
	   DELTA_AG9032V1_FAN3_LED, led_fan_tray_values, 0);
cpld_bf_rw(led_fan_tray4, DELTA_AG9032V1_SWCPLD_FAN_TRAY_LED_1_REG,
	   DELTA_AG9032V1_FAN4_LED, led_fan_tray_values, 0);
cpld_bf_rw(led_fan_tray5, DELTA_AG9032V1_SWCPLD_FAN_TRAY_LED_2_REG,
	   DELTA_AG9032V1_FAN5_LED, led_fan_tray_values, 0);

/* special case for qsfp28 registers */

struct qsfp28_info {
	int reg;
	int active_low;
};

static struct qsfp28_info qsfp28_reg[] = {
	{
		.reg = DELTA_AG9032V1_SWCPLD_QSFP28_MODSEL_1_REG,
		.active_low = 1,
	},
	{
		.reg = DELTA_AG9032V1_SWCPLD_QSFP28_LPMODE_EN_1_REG,
		.active_low = 0,
	},
	{
		.reg = DELTA_AG9032V1_SWCPLD_QSFP28_MOD_ABS_1_REG,
		.active_low = 1,
	},
	{
		.reg = DELTA_AG9032V1_SWCPLD_QSFP28_RST_N_1_REG,
		.active_low = 1,
	},
	{
		.reg = DELTA_AG9032V1_SWCPLD_QSFP28_MOD_INT_N_1_REG,
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
			dev_err(dev, "SW CPLD read error - reg: 0x%02X\n",
				reg);
			return -EINVAL;
		}
		if (qsfp28_reg[idx].active_low)
			tmp = ~tmp;
		/*
		 * The registers have the highest numbered port in the
		 * least significant bit.  We have to reverse the order.
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
		/*
		 * The registers have the highest numbered port in the
		 * least significant bit.  We have to reverse the order.
		 */
		tmp = (tmp & 0xf0) >> 4 | (tmp & 0x0f) << 4;
		tmp = (tmp & 0xcc) >> 2 | (tmp & 0x33) << 2;
		tmp = (tmp & 0xaa) >> 1 | (tmp & 0x55) << 1;
		ret = i2c_smbus_write_byte_data(client, (reg + i), tmp);
		if (ret) {
			dev_err(dev, "CPLD2 write error - reg: 0x%02X\n",
				reg);
			return -EINVAL;
		}
	}
	return count;
}

static SENSOR_DEVICE_ATTR_RW(qsfp_modsel,    qsfp_show, qsfp_store, 0);
static SENSOR_DEVICE_ATTR_RW(qsfp_lpmode,    qsfp_show, qsfp_store, 1);
static SENSOR_DEVICE_ATTR_RO(qsfp_present,   qsfp_show, 2);
static SENSOR_DEVICE_ATTR_RW(qsfp_reset,     qsfp_show, qsfp_store, 3);
static SENSOR_DEVICE_ATTR_RO(qsfp_interrupt, qsfp_show, 4);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&cpld_swcpld_board_id.attr,
	&cpld_swcpld_board_version.attr,
	&cpld_swcpld_version.attr,
	&cpld_reset_sw.attr,
	&cpld_reset_bcm56960.attr,
	&cpld_reset_bcm54616s.attr,
	&cpld_reset_mb_a_cpld.attr,
	&cpld_reset_mb_b_cpld.attr,
	&cpld_ps1_pson.attr,
	&cpld_ps2_pson.attr,
	&cpld_hot_swap1_en.attr,
	&cpld_hot_swap2_en.attr,
	&cpld_vcc_5v_en.attr,
	&cpld_mac_1v8_en.attr,
	&cpld_mac_1v25_en.attr,
	&cpld_mac_avs_1v_en.attr,
	&cpld_mac_1v_en.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_psu_pwr2_all_ok.attr,
	&cpld_hot_swap1_pg.attr,
	&cpld_hot_swap2_pg.attr,
	&cpld_vcc_5v_pg.attr,
	&cpld_mac_1v8_pg.attr,
	&cpld_vcc_mac_1v25_pg.attr,
	&cpld_mac_avs_1v_pg.attr,
	&cpld_mac_1v_pg.attr,
	&cpld_hot_swap1_int.attr,
	&cpld_hot_swap2_int.attr,
	&cpld_psu_pwr1_int.attr,
	&cpld_psu_pwr2_int.attr,
	&cpld_fan_alert.attr,
	&cpld_hot_swap1_int_mask.attr,
	&cpld_hot_swap2_int_mask.attr,
	&cpld_psu_pwr1_int_mask.attr,
	&cpld_psu_pwr2_int_mask.attr,
	&cpld_fan_alert_mask.attr,
	&cpld_qsfp_1_8_int.attr,
	&cpld_qsfp_9_16_int.attr,
	&cpld_qsfp_17_24_int.attr,
	&cpld_qsfp_25_32_int.attr,
	&cpld_qsfp_1_8_abs.attr,
	&cpld_qsfp_9_17_abs.attr,
	&cpld_qsfp_17_24_abs.attr,
	&cpld_qsfp_25_32_abs.attr,
	&cpld_qsfp_1_8_int_mask.attr,
	&cpld_qsfp_9_16_int_mask.attr,
	&cpld_qsfp_17_24_int_mask.attr,
	&cpld_qsfp_25_32_int_mask.attr,
	&cpld_qsfp_1_8_abs_mask.attr,
	&cpld_qsfp_9_16_abs_mask.attr,
	&cpld_qsfp_17_24_abs_mask.attr,
	&cpld_qsfp_25_32_abs_mask.attr,
	&cpld_bcm54616s_irq.attr,
	&cpld_bcm54616s_irq_mask.attr,
	&cpld_led_system.attr,
	&cpld_led_fan.attr,
	&cpld_led_psu1.attr,
	&cpld_led_psu2.attr,
	&cpld_led_fan_tray1.attr,
	&cpld_led_fan_tray2.attr,
	&cpld_led_fan_tray3.attr,
	&cpld_led_fan_tray4.attr,
	&cpld_led_fan_tray5.attr,

	&sensor_dev_attr_qsfp_modsel.dev_attr.attr,
	&sensor_dev_attr_qsfp_lpmode.dev_attr.attr,
	&sensor_dev_attr_qsfp_present.dev_attr.attr,
	&sensor_dev_attr_qsfp_reset.dev_attr.attr,
	&sensor_dev_attr_qsfp_interrupt.dev_attr.attr,

	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

/* cpld probe */

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
					DELTA_AG9032V1_SWCPLD_VERSION_REG);
	if (temp < 0) {
		dev_err(dev, "read SWCPLD version register error: %d\n",
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
	dev_info(dev, "device probed, SWCPLD rev: %lu\n",
		 GET_FIELD(temp, DELTA_AG9032V1_SWITCH_CPLD_VERSION));

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
MODULE_DESCRIPTION("Delta AG9032v1 SW CPLD Driver");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
