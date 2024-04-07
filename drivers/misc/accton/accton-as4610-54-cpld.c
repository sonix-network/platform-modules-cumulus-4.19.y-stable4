/*
 * accton-as4610-cpld.c - Accton AS4610-54 Platform Support.
 *
 * Copyright (C) 2015,2019,2020 Cumulus Networks, Inc.  All rights reserved.
 * Author: David Yen (dhyen@cumulusnetworks.com)
 *
 * Based on the driver for the DNI 3448 written by Alan Liebthal
 * (alanl@cumulusnetworks.com) and the driver for the Alpha Networks
 * SNQ60X0_320F written by Ellen Wang (ellen@cumulusnetworks.com)
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
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <asm/io.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>

#include "platform-defs.h"
#include "accton-as4610-54-cpld.h"

static const char driver_name[] = "accton_as4610_cpld";
#define DRIVER_VERSION "1.0"
#define AS4610_30P_PROD_ID 1
#define AS4610_54P_PROD_ID 3
#define AS4610_54TB_PROD_ID 5
/*-------------------------------------------------------------------------
 *
 * Driver resident static variables
 *
 */

static struct i2c_client *accton_as4610_cpld_client;

/*-------------------------------------------------------------------------
 *
 * CPLD read and write functions
 *
 */

static int32_t cpld_rd(uint32_t reg)
{
	int32_t val;

	val = i2c_smbus_read_byte_data(accton_as4610_cpld_client, reg);
	if (val < 0) {
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       accton_as4610_cpld_client->addr, reg);
	}
	/*	printk("cpld: rd 0x%02X => 0x%02X\n", reg, val); */
	return val;
}

static int cpld_wr(uint32_t reg, uint8_t write_val)
{
	int res;

	res = i2c_smbus_write_byte_data(accton_as4610_cpld_client, reg,
					write_val);
	if (res) {
		pr_err("could not write to i2c device addr: 0x%02X, "
			"reg: 0x%02X, val: 0x%02X",
			accton_as4610_cpld_client->addr, reg, write_val);
	}
	return res;
}

static uint32_t raw_reg_val;

static ssize_t raw_reg_show(struct device * dev,
                            struct device_attribute * dattr,
                            char * buf)
{
	uint32_t data;

	data = cpld_rd(raw_reg_val);
	return sprintf(buf, "0x%02X => 0x%02X\n", raw_reg_val, data);
}

static ssize_t raw_reg_store(struct device * dev,
                             struct device_attribute * dattr,
                             const char * buf, size_t count)
{
	int tmp;
	int tmp2;

	if (sscanf(buf, "%x %x", &tmp, &tmp2) == 2) {
		cpld_wr(tmp, tmp2);
	} else if (sscanf(buf, "%x", &tmp) == 1) {
		raw_reg_val = tmp;
	} else {
		return -EINVAL;
	}
	return count;
}
static SYSFS_ATTR_RW(raw_reg, raw_reg_show, raw_reg_store);

#define ACCTON_AS4610_CPLD_STRING_NAME_SIZE 20


/*
 * CPLD register bitfields with enum-like values
 *
 * For example, a 2-bit led field may have values off, green, yellow,
 * and green_blinking, corresponding to 0, 1, 2, 3.
 *
 * Maximum value string length is PLATFORM_LED_COLOR_NAME_SIZE - 1,
 * but isn't enforced anywhere.  Also not enforced is the size
 * of the values array, which of course must match the field width.
 * So be cool.
 */

struct cpld_bf {
	const char *name;
	const char * const *values;
	u8 port;
	u8 shift;
	u8 width;
};

#define mk_bf(_name, _port, _shift, _width, _values) { \
	.name = #_name, \
	.port = (_port), \
	.shift = (_shift), \
	.width = (_width), \
	.values = (_values), \
}

#define MASK(n) ((1 << (n)) - 1)

static const char * const cpld_product_id_values[] = {
	"AS4610-24",
	"AS4610-24P",
	"AS4610-48",
	"AS4610-48P",
	"Unknown",
	"AS4610-48-B"
};

static const char * const cpld_fan_speed_values[] = {
	"0%",
	"12.5%",
	"25.0%",
	"37.5%",
	"50.0%",
	"62.5%",
	"75.0%",
	"87.5%",
	"100%",
};

static const char * const cpld_led_display_values[] = {
        "0",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
};

static const char * const cpld_led_panel_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_YELLOW,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_OFF,
};

static struct cpld_bf cpld_bfs[] = {
	mk_bf(external_cpu_support, CPLD_PRODUCT_ID_OFFSET,         6, 1, NULL),
	mk_bf(poe_support,          CPLD_PRODUCT_ID_OFFSET,         5, 1, NULL),
	mk_bf(product_id,           CPLD_PRODUCT_ID_OFFSET,         0, 4,
	      cpld_product_id_values),
	mk_bf(sfp1_present,         CPLD_SFP_1_2_STATUS_OFFSET,     6, 1, NULL),
	mk_bf(sfp1_tx_fault,        CPLD_SFP_1_2_STATUS_OFFSET,     5, 1, NULL),
	mk_bf(sfp1_rx_los,          CPLD_SFP_1_2_STATUS_OFFSET,     4, 1, NULL),
	mk_bf(sfp2_present,         CPLD_SFP_1_2_STATUS_OFFSET,     2, 1, NULL),
	mk_bf(sfp2_tx_fault,        CPLD_SFP_1_2_STATUS_OFFSET,     1, 1, NULL),
	mk_bf(sfp2_rx_los,          CPLD_SFP_1_2_STATUS_OFFSET,     0, 1, NULL),
	mk_bf(sfp3_present,         CPLD_SFP_3_4_STATUS_OFFSET,     6, 1, NULL),
	mk_bf(sfp3_tx_fault,        CPLD_SFP_3_4_STATUS_OFFSET,     5, 1, NULL),
	mk_bf(sfp3_rx_los,          CPLD_SFP_3_4_STATUS_OFFSET,     4, 1, NULL),
	mk_bf(sfp4_present,         CPLD_SFP_3_4_STATUS_OFFSET,     2, 1, NULL),
	mk_bf(sfp4_tx_fault,        CPLD_SFP_3_4_STATUS_OFFSET,     1, 1, NULL),
	mk_bf(sfp4_rx_los,          CPLD_SFP_3_4_STATUS_OFFSET,     0, 1, NULL),
	mk_bf(54v_power_ok,         CPLD_POE_STATUS_OFFSET,         2, 1, NULL),
	mk_bf(24_port_poe_present,  CPLD_POE_STATUS_OFFSET,         1, 1, NULL),
	mk_bf(48_port_poe_present,  CPLD_POE_STATUS_OFFSET,         0, 1, NULL),
	mk_bf(reset_poe,            CPLD_POE_CONTROL_OFFSET,        1, 1, NULL),
	mk_bf(enable_poe,           CPLD_POE_CONTROL_OFFSET,        0, 1, NULL),
	mk_bf(fan1_ok,              CPLD_POWER_STATUS_OFFSET,       5, 1, NULL),
	mk_bf(fan2_ok,              CPLD_POWER_STATUS_OFFSET,       4, 1, NULL),
	mk_bf(psu_pwr2_all_ok,      CPLD_POWER_STATUS_OFFSET,       3, 1, NULL),
	mk_bf(psu_pwr2_present,     CPLD_POWER_STATUS_OFFSET,       2, 1, NULL),
	mk_bf(psu_pwr1_all_ok,      CPLD_POWER_STATUS_OFFSET,       1, 1, NULL),
	mk_bf(psu_pwr1_present,     CPLD_POWER_STATUS_OFFSET,       0, 1, NULL),
	mk_bf(over_temp,            CPLD_SYS_TEMP_OFFSET,           5, 1, NULL),
	mk_bf(mgmt_phy_reset,       CPLD_PHY1_RESET_OFFSET,         7, 1, NULL),
	mk_bf(10g_phy_reset,        CPLD_PHY1_RESET_OFFSET,         6, 1, NULL),
	mk_bf(1g_phy6_reset,        CPLD_PHY1_RESET_OFFSET,         5, 1, NULL),
	mk_bf(1g_phy5_reset,        CPLD_PHY1_RESET_OFFSET,         4, 1, NULL),
	mk_bf(1g_phy4_reset,        CPLD_PHY1_RESET_OFFSET,         3, 1, NULL),
	mk_bf(1g_phy3_reset,        CPLD_PHY1_RESET_OFFSET,         2, 1, NULL),
	mk_bf(1g_phy2_reset,        CPLD_PHY1_RESET_OFFSET,         1, 1, NULL),
	mk_bf(1g_phy1_reset,        CPLD_PHY1_RESET_OFFSET,         0, 1, NULL),
	mk_bf(7_segment_led_blink,  CPLD_SEGMENT_LED_BLINK_OFFSET,  6, 1, NULL),
	/* Note: 
	 * The 4610-54P only uses fan2, so omitting all fan1 elements and
	 * renaming all fan2 elements to fan1. 4610-54T-B uses both fan1 and fan2.
	 * The 4610-54T doesn't use any of these fans.
	 */
	mk_bf(fan1_target,          CPLD_FAN_SPEED_CONTROL_OFFSET,  0, 4, NULL),
	mk_bf(fan1_input,           CPLD_FAN_2_SPEED_DETECT_OFFSET, 0, 8, NULL),
	mk_bf(fan2_target,          CPLD_FAN_SPEED_CONTROL_OFFSET,  0, 4, NULL),
	mk_bf(fan2_input,           CPLD_FAN_1_SPEED_DETECT_OFFSET, 0, 8, NULL),
	mk_bf(debug_version,        CPLD_DEBUG_VERSION_OFFSET,      0, 8, NULL),
	mk_bf(led_display2_dot,     CPLD_SEGMENT_2_LED_OFFSET,      4, 1, NULL),
	mk_bf(led_display2_number,  CPLD_SEGMENT_2_LED_OFFSET,      0, 4,
	      cpld_led_display_values),
	mk_bf(led_display1_dot,     CPLD_SEGMENT_1_LED_OFFSET,      4, 1, NULL),
	mk_bf(led_display1_number,  CPLD_SEGMENT_1_LED_OFFSET,      0, 4,
	      cpld_led_display_values),
	mk_bf(led_system,           CPLD_SYSTEM_1_LED_OFFSET,       6, 2,
	      cpld_led_panel_values),
	mk_bf(led_primary,          CPLD_SYSTEM_1_LED_OFFSET,       4, 2,
	      cpld_led_panel_values),
	mk_bf(led_psu1,             CPLD_SYSTEM_1_LED_OFFSET,       2, 2,
	      cpld_led_panel_values),
	mk_bf(led_psu2,             CPLD_SYSTEM_1_LED_OFFSET,       0, 2,
	      cpld_led_panel_values),
	mk_bf(led_stack1,           CPLD_SYSTEM_2_LED_OFFSET,       6, 2,
	      cpld_led_panel_values),
	mk_bf(led_stack2,           CPLD_SYSTEM_2_LED_OFFSET,       4, 2,
	      cpld_led_panel_values),
	mk_bf(led_fan,              CPLD_SYSTEM_2_LED_OFFSET,       2, 2,
	      cpld_led_panel_values),
	mk_bf(led_poe,              CPLD_SYSTEM_2_LED_OFFSET,       0, 2,
	      cpld_led_panel_values),
	mk_bf(cpld_version,        CPLD_CPLD_VERSION_OFFSET,        0, 8, NULL),
};

static int cpld_find_bf(struct device_attribute *dattr)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cpld_bfs); i++)
		if (strcmp(cpld_bfs[i].name, dattr->attr.name) == 0)
			return i;
	return -EINVAL;
}

int hexToInt(const char *hexStr, uint32_t *valPtr)
{
  char prefix[] = "0x";
  if (strncasecmp(hexStr, prefix, strlen(prefix)) == 0) {
    hexStr += strlen(prefix);
    return sscanf(hexStr, "%x", valPtr) != 1;
  }
  return sscanf(hexStr, "%u", valPtr) != 1;
}

static int cpld_parse_bf_value(const char *buf,
			       struct cpld_bf *bf)
{
	char str[PLATFORM_LED_COLOR_NAME_SIZE];
	int nvalues = 1 << bf->width;
	int i;

#if PLATFORM_LED_COLOR_NAME_SIZE != 20
#error PLATFORM_LED_COLOR_NAME_SIZE is supposed to be 20
#endif
	if (sscanf(buf, "%19s", str) != 1) {
		return -EINVAL;
	}

	if (bf->values == NULL) {
		if (hexToInt(buf, &i)) {
			return -EINVAL;
		} else {
			return i;
		}
	}

	for (i = 0; i < nvalues; i++) {
		if (strcmp(str, bf->values[i]) == 0) {
			return i;
		}
	}

	return -EINVAL;
}

static ssize_t cpld_bf_show(struct device *dev,
			    struct device_attribute *dattr,
			    char *buf)
{
	struct cpld_bf * bf = NULL;
	int ret;

	if ((ret = cpld_find_bf(dattr)) < 0) {
		return ret;
	}

	bf = cpld_bfs + ret;

	if ((ret = cpld_rd(bf->port)) < 0) {
		return ret;
	}
	ret >>= bf->shift;
	ret &= MASK(bf->width);

	if (bf->values == NULL) {
		return sprintf(buf, "%d\n", ret);
	}

	return sprintf(buf, "%s\n", bf->values[ret]);
}

static ssize_t cpld_bf_store(struct device *dev,
			     struct device_attribute *dattr,
			     const char *buf, size_t size)
{
	struct cpld_bf * bf = NULL;
	u8 val;
	int ret;

	if ((ret = cpld_find_bf(dattr)) < 0) {
		return ret;
	}
	bf = cpld_bfs + ret;

	if ((ret = cpld_parse_bf_value(buf, bf)) < 0) {
		return ret;
	}

	val = ret << bf->shift;

	if ((ret = cpld_rd(bf->port)) < 0) {
		return ret;
	}

	val |= ret & ~(MASK(bf->width) << bf->shift);
	if ((ret = cpld_wr(bf->port, val)) < 0) {
		return ret;
	}

	return size;
}

static ssize_t cpld_rpm_show(struct device *dev,
			     struct device_attribute *dattr,
			     char *buf)
{
	struct cpld_bf * bf = NULL;
	int ret;
	int temp;

	if ((ret = cpld_find_bf(dattr)) < 0) {
		return ret;
	}

	bf = cpld_bfs + ret;

	if ((ret = cpld_rd(bf->port)) < 0) {
		return ret;
	}
	ret >>= bf->shift;
	ret &= MASK(bf->width);

	temp = (int)ret*379*30/100;

	return sprintf(buf, "%d\n", temp);
}


// Fan speed/PWM

static uint8_t accton_as4610_54p_pwm = 128;  // default value

static ssize_t pwm_show(struct device * dev,
			 struct device_attribute * dattr,
			 char * buf)
{
        return sprintf(buf, "%d\n", accton_as4610_54p_pwm);
}

static ssize_t pwm_store(struct device * dev,
			  struct device_attribute * dattr,
			  const char *buf, size_t count)
{
        uint32_t pwm = 0;

        if (sscanf(buf, "%d", &pwm) <= 0) {
                return -EINVAL;
        }
	pwm = clamp_val(pwm, 0, 128);
        accton_as4610_54p_pwm = pwm;
        pwm >>= 4;
        cpld_wr(CPLD_FAN_SPEED_CONTROL_OFFSET, pwm);

        return count;
}

static ssize_t pwm_enable_show(struct device * dev,
			       struct device_attribute * dattr,
			       char * buf)
{
        return sprintf(buf, "1\n");
}

static ssize_t pwm_enable_store(struct device * dev,
				struct device_attribute * dattr,
				const char *buf, size_t count)
{
        // Do nothing. Needed for pwmd
        return count;
}

static SYSFS_ATTR_RO(external_cpu_support, cpld_bf_show);
static SYSFS_ATTR_RO(poe_support,          cpld_bf_show);
static SYSFS_ATTR_RO(product_id,           cpld_bf_show);
static SYSFS_ATTR_RO(sfp1_present,         cpld_bf_show);
static SYSFS_ATTR_RO(sfp1_tx_fault,        cpld_bf_show);
static SYSFS_ATTR_RO(sfp1_rx_los,          cpld_bf_show);
static SYSFS_ATTR_RO(sfp2_present,         cpld_bf_show);
static SYSFS_ATTR_RO(sfp2_tx_fault,        cpld_bf_show);
static SYSFS_ATTR_RO(sfp2_rx_los,          cpld_bf_show);
static SYSFS_ATTR_RO(sfp3_present,         cpld_bf_show);
static SYSFS_ATTR_RO(sfp3_tx_fault,        cpld_bf_show);
static SYSFS_ATTR_RO(sfp3_rx_los,          cpld_bf_show);
static SYSFS_ATTR_RO(sfp4_present,         cpld_bf_show);
static SYSFS_ATTR_RO(sfp4_tx_fault,        cpld_bf_show);
static SYSFS_ATTR_RO(sfp4_rx_los,          cpld_bf_show);
static SYSFS_ATTR_RO(54v_power_ok,         cpld_bf_show);
static SYSFS_ATTR_RO(24_port_poe_present,  cpld_bf_show);
static SYSFS_ATTR_RO(48_port_poe_present,  cpld_bf_show);
static SYSFS_ATTR_RW(reset_poe,            cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(enable_poe,           cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RO(fan1_ok,              cpld_bf_show);
static SYSFS_ATTR_RO(fan2_ok,              cpld_bf_show);
static SYSFS_ATTR_RO(psu_pwr2_all_ok,      cpld_bf_show);
static SYSFS_ATTR_RO(psu_pwr2_present,     cpld_bf_show);
static SYSFS_ATTR_RO(psu_pwr1_all_ok,      cpld_bf_show);
static SYSFS_ATTR_RO(psu_pwr1_present,     cpld_bf_show);
static SYSFS_ATTR_RO(over_temp,            cpld_bf_show);
static SYSFS_ATTR_RW(mgmt_phy_reset,       cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(10g_phy_reset,        cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(1g_phy6_reset,        cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(1g_phy5_reset,        cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(1g_phy4_reset,        cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(1g_phy3_reset,        cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(1g_phy2_reset,        cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(1g_phy1_reset,        cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(7_segment_led_blink,  cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(fan1_target,          cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RO(fan1_input,           cpld_rpm_show);
static SYSFS_ATTR_RW(fan2_target,          cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RO(fan2_input,           cpld_rpm_show);
static SYSFS_ATTR_RO(debug_version,        cpld_bf_show);
static SYSFS_ATTR_RW(led_display2_dot,     cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(led_display2_number,  cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(led_display1_dot,     cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(led_display1_number,  cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(led_system,           cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(led_primary,          cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(led_psu1,             cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(led_psu2,             cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(led_stack1,           cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(led_stack2,           cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(led_fan,              cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(led_poe,              cpld_bf_show, cpld_bf_store);
static SYSFS_ATTR_RW(pwm1,                 pwm_show, pwm_store);
static SYSFS_ATTR_RW(pwm1_enable,          pwm_enable_show,
					   pwm_enable_store);
static SYSFS_ATTR_RO(cpld_version,         cpld_bf_show);

/*-------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */

/* CPLD attributes common to all 4610s */
static struct attribute *accton_as4610_cmn_cpld_attrs[] = {
	&dev_attr_external_cpu_support.attr,
	&dev_attr_poe_support.attr,
	&dev_attr_product_id.attr,
	&dev_attr_sfp1_present.attr,
	&dev_attr_sfp1_tx_fault.attr,
	&dev_attr_sfp1_rx_los.attr,
	&dev_attr_sfp2_present.attr,
	&dev_attr_sfp2_tx_fault.attr,
	&dev_attr_sfp2_rx_los.attr,
	&dev_attr_sfp3_present.attr,
	&dev_attr_sfp3_tx_fault.attr,
	&dev_attr_sfp3_rx_los.attr,
	&dev_attr_sfp4_present.attr,
	&dev_attr_sfp4_tx_fault.attr,
	&dev_attr_sfp4_rx_los.attr,
	&dev_attr_54v_power_ok.attr,
	&dev_attr_24_port_poe_present.attr,
	&dev_attr_48_port_poe_present.attr,
	&dev_attr_reset_poe.attr,
	&dev_attr_enable_poe.attr,
	&dev_attr_psu_pwr2_all_ok.attr,
	&dev_attr_psu_pwr2_present.attr,
	&dev_attr_psu_pwr1_all_ok.attr,
	&dev_attr_psu_pwr1_present.attr,
	&dev_attr_over_temp.attr,
	&dev_attr_mgmt_phy_reset.attr,
	&dev_attr_10g_phy_reset.attr,
	&dev_attr_1g_phy6_reset.attr,
	&dev_attr_1g_phy5_reset.attr,
	&dev_attr_1g_phy4_reset.attr,
	&dev_attr_1g_phy3_reset.attr,
	&dev_attr_1g_phy2_reset.attr,
	&dev_attr_1g_phy1_reset.attr,
	&dev_attr_7_segment_led_blink.attr,
	&dev_attr_debug_version.attr,
	&dev_attr_led_display2_dot.attr,
	&dev_attr_led_display2_number.attr,
	&dev_attr_led_display1_dot.attr,
	&dev_attr_led_display1_number.attr,
	&dev_attr_led_system.attr,
	&dev_attr_led_primary.attr,
	&dev_attr_led_psu1.attr,
	&dev_attr_led_psu2.attr,
	&dev_attr_led_stack1.attr,
	&dev_attr_led_stack2.attr,
	&dev_attr_led_fan.attr,
	&dev_attr_led_poe.attr,
	&dev_attr_pwm1.attr,
	&dev_attr_pwm1_enable.attr,
	&dev_attr_raw_reg.attr,
	&dev_attr_cpld_version.attr,
	NULL,
};

static struct attribute *accton_as4610_54P_cpld_attrs[] = {
	&dev_attr_fan1_ok.attr,
	&dev_attr_fan1_input.attr,
	&dev_attr_fan1_target.attr,
	NULL
};

static struct attribute *accton_as4610_54TB_cpld_attrs[] = {
	&dev_attr_fan1_ok.attr,
	&dev_attr_fan1_input.attr,
	&dev_attr_fan2_ok.attr,
	&dev_attr_fan2_input.attr,
	&dev_attr_fan1_target.attr,
	&dev_attr_fan2_target.attr,
	NULL
};

/*-------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static struct attribute_group *grp;

static struct attribute_group accton_as4610_cmn_cpld_attr_group = {
	.attrs = accton_as4610_cmn_cpld_attrs,
};

static struct attribute_group accton_as4610_54P_cpld_attr_group = {
	.attrs = accton_as4610_54P_cpld_attrs,
};

static struct attribute_group accton_as4610_54TB_cpld_attr_group = {
	.attrs = accton_as4610_54TB_cpld_attrs,
};

struct accton_as4610_cpld_data {
	struct device *hwmon_dev;
};

static struct accton_as4610_cpld_data cpld_data;

static int accton_as4610_cpld_setup(void)
{
	/* Put some interesting, one-time initializations here. */

	return 0;
}

static int accton_as4610_cpld_probe(struct i2c_client *client,
				    const struct i2c_device_id *id)
{
	int retval = 0;
	struct kobject *kobj = &client->dev.kobj;
	int product_id = 0;

	if (dev_get_drvdata(&client->dev)) {
		dev_info(&client->dev, "already probed\n");
		return 0;
	}
	accton_as4610_cpld_client = client;

	/* create common sysfs attributes */
	retval = sysfs_create_group(kobj, &accton_as4610_cmn_cpld_attr_group);
	if (retval)
		return retval;

	/* create fan sysfs groups based on product IDs */
	product_id = cpld_rd(CPLD_PRODUCT_ID_OFFSET);
	if (product_id < 0)
		return product_id;

	product_id = product_id & 0x0f;

	if (product_id == AS4610_54P_PROD_ID ||
	    product_id == AS4610_30P_PROD_ID)
		grp = &accton_as4610_54P_cpld_attr_group;
	if (product_id == AS4610_54TB_PROD_ID)
		grp = &accton_as4610_54TB_cpld_attr_group;

	if (grp) {
		retval = sysfs_create_group(kobj, grp);
		if (retval)
			goto err_cleanup_cmn_attrs;
	}

	if (accton_as4610_cpld_setup())
		return -EIO;

	dev_info(&client->dev, "probed & iomapped @ 0x%p, 0x%02X\n",
		 accton_as4610_cpld_setup, accton_as4610_cpld_client->addr);

	cpld_data.hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(cpld_data.hwmon_dev)) {
		retval = PTR_ERR(cpld_data.hwmon_dev);
		dev_err(&client->dev, "hwmon registration failed");
		goto err_hwmon_device;
	}

	return 0;

err_hwmon_device:
	if (grp)
		sysfs_remove_group(kobj, grp);
err_cleanup_cmn_attrs:
	sysfs_remove_group(kobj, &accton_as4610_cmn_cpld_attr_group);
	return retval;
}

static int accton_as4610_cpld_remove(struct i2c_client *client)
{
	struct kobject *kobj = &client->dev.kobj;
	
	if (grp)
		sysfs_remove_group(kobj, grp);
	sysfs_remove_group(kobj, &accton_as4610_cmn_cpld_attr_group);

	dev_info(&client->dev, "removed\n");
	return 0;
}

static struct of_device_id accton_as4610_cpld_ids[] = {
	{
		.compatible = "as4610-54-cpld",
	},
	{ /* end of list */ },
};

static struct i2c_device_id accton_as4610_cpld_i2c_ids[] = {
	{
		.name = "as4610-54-cpld",
	},
	{ /* end of list */ },
};
MODULE_DEVICE_TABLE(i2c, accton_as4610_cpld_i2c_ids);

static struct i2c_driver accton_as4610_cpld_driver = {
	.probe = accton_as4610_cpld_probe,
	.remove = accton_as4610_cpld_remove,
	.id_table = accton_as4610_cpld_i2c_ids,
	.driver = {
		.name  = driver_name,
		.owner = THIS_MODULE,
		.of_match_table = accton_as4610_cpld_ids,
	},
};


/*-------------------------------------------------------------------------
 *
 * module interface
 *
 */

static int __init accton_as4610_cpld_init(void)
{
	int rv;

	printk("accton_as4610_cpld_init()\n");
	rv = i2c_add_driver(&accton_as4610_cpld_driver);
	if (rv) {
		printk(KERN_ERR
		       "%s i2c_add_driver failed (%i)\n",
		       driver_name, rv);
	}
	return rv;
}

static void __exit accton_as4610_cpld_exit(void)
{
	printk("accton_as4610_cpld_exit()\n");
	return i2c_del_driver(&accton_as4610_cpld_driver);
}

MODULE_AUTHOR("David Yen <dhyen@cumulusnetworks.com>");
MODULE_DESCRIPTION("CPLD driver for Accton AS4610-54");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(accton_as4610_cpld_init);
module_exit(accton_as4610_cpld_exit);
