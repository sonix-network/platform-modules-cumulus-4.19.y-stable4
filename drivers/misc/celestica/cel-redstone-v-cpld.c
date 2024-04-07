/*
 * cel_redstone_v_cpld.c - Celestica Redstone-V CPLD sysfs driver.
 *
 * Copyright (C) 2017, 2018, 2019 Cumulus Networks, Inc.
 * Author: David Yen (dhyen@cumulusnetworks.com)
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
#include <linux/hwmon-sysfs.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <linux/io.h>

#include "cel-redstone-v.h"
#include "platform-defs.h"
#include "platform-bitfield.h"

static const char driver_name[] = "cel_redstone_v_cpld";
#define DRIVER_VERSION "1.0"

/*
 * CPLD driver
 */

static uint8_t *cpld_regs;

/* simple bit-field registers */

static int lpccpld_read_reg(struct device *dev,
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

static int lpccpld_write_reg(struct device *dev,
			     int reg,
			     int nregs,
			     u32 val)
{
	for (; nregs > 0; nregs--, reg++, val >>= 8)
		iowrite8(val, cpld_regs + reg);
	return 0;
}

static const char * const lpccpld_direction[] = {
	"F2B", "B2F"
};

mk_bf_rw(lpccpld, write_protect,  CPLD1_MISC_CONTROL_REG,
	 0, 5, NULL, 0);

mk_bf_rw(lpccpld, pwm1,	  CPLD1_FAN1_PWM_CONTROL_REG,
	 0, 8, NULL, BF_DECIMAL);
mk_bf_rw(lpccpld, pwm2,	  CPLD1_FAN2_PWM_CONTROL_REG,
	 0, 8, NULL, BF_DECIMAL);
mk_bf_rw(lpccpld, pwm3,	  CPLD1_FAN3_PWM_CONTROL_REG,
	 0, 8, NULL, BF_DECIMAL);
mk_bf_rw(lpccpld, pwm4,	  CPLD1_FAN4_PWM_CONTROL_REG,
	 0, 8, NULL, BF_DECIMAL);
mk_bf_rw(lpccpld, pwm5,	  CPLD1_FAN5_PWM_CONTROL_REG,
	 0, 8, NULL, BF_DECIMAL);

mk_bf_ro(lpccpld, fan1_present,	  CPLD1_FAN1_MISC_CONTROL_AND_STATUS_REG,
	 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, fan2_present,	  CPLD1_FAN2_MISC_CONTROL_AND_STATUS_REG,
	 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, fan3_present,	  CPLD1_FAN3_MISC_CONTROL_AND_STATUS_REG,
	 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, fan4_present,	  CPLD1_FAN4_MISC_CONTROL_AND_STATUS_REG,
	 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, fan5_present,	  CPLD1_FAN5_MISC_CONTROL_AND_STATUS_REG,
	 2, 1, NULL, BF_COMPLEMENT);

mk_bf_ro(lpccpld, fan1_direction, CPLD1_FAN1_MISC_CONTROL_AND_STATUS_REG,
	 3, 1, lpccpld_direction, 0);
mk_bf_ro(lpccpld, fan2_direction, CPLD1_FAN2_MISC_CONTROL_AND_STATUS_REG,
	 3, 1, lpccpld_direction, 0);
mk_bf_ro(lpccpld, fan3_direction, CPLD1_FAN3_MISC_CONTROL_AND_STATUS_REG,
	 3, 1, lpccpld_direction, 0);
mk_bf_ro(lpccpld, fan4_direction, CPLD1_FAN4_MISC_CONTROL_AND_STATUS_REG,
	 3, 1, lpccpld_direction, 0);
mk_bf_ro(lpccpld, fan5_direction, CPLD1_FAN5_MISC_CONTROL_AND_STATUS_REG,
	 3, 1, lpccpld_direction, 0);

mk_bf_ro(lpccpld, psu_pwr1_alarm,	  CPLD1_PSU_CONTROL_AND_STATUS_REG,
	 7, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, psu_pwr2_alarm,	  CPLD1_PSU_CONTROL_AND_STATUS_REG,
	 6, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, psu_pwr1_present,	  CPLD1_PSU_CONTROL_AND_STATUS_REG,
	 5, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, psu_pwr2_present,	  CPLD1_PSU_CONTROL_AND_STATUS_REG,
	 4, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, psu_pwr1_all_ok,	  CPLD1_PSU_CONTROL_AND_STATUS_REG,
	 3, 1, NULL, BF_DECIMAL);
mk_bf_ro(lpccpld, psu_pwr2_all_ok,	  CPLD1_PSU_CONTROL_AND_STATUS_REG,
	 2, 1, NULL, BF_DECIMAL);
mk_bf_rw(lpccpld, psu_pwr1_enable,	  CPLD1_PSU_CONTROL_AND_STATUS_REG,
	 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, psu_pwr2_enable,	  CPLD1_PSU_CONTROL_AND_STATUS_REG,
	 0, 1, NULL, BF_COMPLEMENT);

mk_bf_rw(lpccpld, qsfp_reset,	  CPLD4_QSFP28_1_4_RESET_REG,
	 0, 4, NULL, BF_COMPLEMENT);
mk_bf_rw(lpccpld, qsfp_lp_mode,	  CPLD4_QSFP28_1_4_LPMOD_REG,
	 0, 4, NULL, 0);
mk_bf_ro(lpccpld, qsfp_present,	  CPLD4_QSFP28_1_4_ABS_REG,
	 0, 4, NULL, BF_COMPLEMENT);
mk_bf_ro(lpccpld, qsfp_interrupt, CPLD4_QSFP28_1_4_INT_N_REG,
	 0, 4, NULL, BF_COMPLEMENT);

/* more complicated cpld registers */

static uint8_t cpld_reg_read(uint32_t reg)
{
	uint8_t data;

	data = ioread8(cpld_regs + reg);
	return data;
}

static void cpld_reg_write(uint32_t reg, uint8_t data)
{
	iowrite8(data, cpld_regs + reg);
}

/* cpld versions */

static ssize_t cpld_version_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	uint8_t val = cpld_reg_read(attr->index);

	return sprintf(buf, "%d.%d\n",
		       (val & CPLD1_CPLD_ID_MASK) >> CPLD1_CPLD_ID_SHIFT,
		       (val & CPLD1_CPLD_MINOR_VERSION_MASK) >>
			CPLD1_CPLD_MINOR_VERSION_SHIFT);
}

static SENSOR_DEVICE_ATTR(cpld1_version, S_IRUGO, cpld_version_show,
			NULL, CPLD1_VERSION_REG);
static SENSOR_DEVICE_ATTR(cpld2_version, S_IRUGO, cpld_version_show,
			NULL, CPLD2_VERSION_REG);
static SENSOR_DEVICE_ATTR(cpld3_version, S_IRUGO, cpld_version_show,
			NULL, CPLD3_VERSION_REG);
static SENSOR_DEVICE_ATTR(cpld4_version, S_IRUGO, cpld_version_show,
			NULL, CPLD4_VERSION_REG);

/* Fan speeds */

static ssize_t fan_input_show(struct device *dev,
			      struct device_attribute *dattr,
			      char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);

	return sprintf(buf, "%d\n",
		       cpld_reg_read(attr->index) * CPLD1_FAN_SPEED_MULT);
}

static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, fan_input_show,
	NULL, CPLD1_FAN1_FRONT_FAN_SPEED_REG);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO, fan_input_show,
	NULL, CPLD1_FAN2_FRONT_FAN_SPEED_REG);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO, fan_input_show,
	NULL, CPLD1_FAN3_FRONT_FAN_SPEED_REG);
static SENSOR_DEVICE_ATTR(fan4_input, S_IRUGO, fan_input_show,
	NULL, CPLD1_FAN4_FRONT_FAN_SPEED_REG);
static SENSOR_DEVICE_ATTR(fan5_input, S_IRUGO, fan_input_show,
	NULL, CPLD1_FAN5_FRONT_FAN_SPEED_REG);

/* LEDs */

struct led {
	char name[CPLD_STRING_NAME_SIZE];
	uint32_t reg;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_system",
		.reg  = CPLD1_FRONT_PANEL_LED_CONTROL_REG,
		.mask = (CPLD1_SYSTEM_LED_CONTROL_MASK |
			 CPLD1_SYSTEM_STATUS_LED_CONTROL_MASK),
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, (CPLD1_SYSTEM_LED_GREEN |
					       CPLD1_SYSTEM_STATUS_LED_ON) },
			{ PLATFORM_LED_YELLOW_BLINKING,
			  (CPLD1_SYSTEM_LED_YELLOW |
			   CPLD1_SYSTEM_STATUS_LED_1HZ_BLINK) },
			{ PLATFORM_LED_GREEN_BLINKING,
			  (CPLD1_SYSTEM_LED_GREEN |
			   CPLD1_SYSTEM_STATUS_LED_1HZ_BLINK) },
			{ PLATFORM_LED_OFF, (CPLD1_SYSTEM_LED_OFF |
					     CPLD1_SYSTEM_STATUS_LED_OFF) },
		},
	},
	{
		.name = "led_bmc",
		.reg  = CPLD1_FRONT_PANEL_LED_CONTROL_REG,
		.mask = CPLD1_BMC_STATUS_LED_CONTROL_MASK,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD1_BMC_STATUS_LED_GREEN },
			{ PLATFORM_LED_OFF, CPLD1_BMC_STATUS_LED_OFF },
		},
	},
};

static int n_leds = ARRAY_SIZE(cpld_leds);

static ssize_t led_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	uint8_t tmpx;
	uint8_t tmp;
	int i;
	struct led *target = NULL;

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (!target)
		return sprintf(buf, "undefined target\n");

	/* read the register */
	tmpx = cpld_reg_read(target->reg);

	/* find the color */
	tmp = tmpx & target->mask;
	for (i = 0; i < target->n_colors; i++) {
		if (tmp == target->colors[i].value)
			break;
	}
	if (i == target->n_colors)
		return sprintf(buf, "undefined color: 0x%x; mask: 0x%x\n",
			       tmpx, target->mask);
	else
		return sprintf(buf, "%s\n", target->colors[i].name);
}

static ssize_t led_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	uint8_t tmp;
	int i;
	struct led *target = NULL;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (!target)
		return -EINVAL;

	/* find the color */
	if (sscanf(buf, "%19s", raw) <= 0)
		return -EINVAL;
	for (i = 0; i < target->n_colors; i++) {
		if (strcmp(raw, target->colors[i].name) == 0)
			break;
	}
	if (i == target->n_colors)
		return -EINVAL;

	tmp = cpld_reg_read(target->reg);
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;
	cpld_reg_write(target->reg, tmp);

	return count;
}
static SYSFS_ATTR_RW(led_system, led_show, led_store);
static SYSFS_ATTR_RW(led_bmc,	 led_show, led_store);

/* SFP ports */

struct sfp_reg_offset_info {
	uint8_t	 mask;
	uint8_t	 shift;
};

struct sfp_reg_info {
	char node_name[REG_NAME_LEN];
	uint32_t regs[NUM_SFP_REGS];
	int	 active_low;
};

static struct sfp_reg_info sfp_reg_info[] = {
	{
		.node_name = "sfp_rx_los",
		.active_low = 1,
		.regs = {
			CPLD2_SFP_1_8_RXLOS_REG,
			CPLD2_SFP_9_16_RXLOS_REG,
			CPLD2_SFP_17_18_RXLOS_REG,
			CPLD3_SFP_19_26_RXLOS_REG,
			CPLD3_SFP_27_34_RXLOS_REG,
			CPLD3_SFP_35_36_RXLOS_REG,
			CPLD4_SFP_37_44_RXLOS_REG,
			CPLD4_SFP_45_48_RXLOS_REG
		},
	},
	{
		.node_name = "sfp_tx_disable",
		.regs = {
			CPLD2_SFP_1_8_TXDISABLE_REG,
			CPLD2_SFP_9_16_TXDISABLE_REG,
			CPLD2_SFP_17_18_TXDISABLE_REG,
			CPLD3_SFP_19_26_TXDISABLE_REG,
			CPLD3_SFP_27_34_TXDISABLE_REG,
			CPLD3_SFP_35_36_TXDISABLE_REG,
			CPLD4_SFP_37_44_TXDISABLE_REG,
			CPLD4_SFP_45_48_TXDISABLE_REG
		},
	},
	{
		.node_name = "sfp_rs",
		.regs = {
			CPLD2_SFP_1_8_RS_CONTROL_REG,
			CPLD2_SFP_9_16_RS_CONTROL_REG,
			CPLD2_SFP_17_18_RS_CONTROL_REG,
			CPLD3_SFP_19_26_RS_CONTROL_REG,
			CPLD3_SFP_27_34_RS_CONTROL_REG,
			CPLD3_SFP_35_36_RS_CONTROL_REG,
			CPLD4_SFP_37_44_RS_CONTROL_REG,
			CPLD4_SFP_45_48_RS_CONTROL_REG
		},
	},
	{
		.node_name = "sfp_tx_fault",
		.regs = {
			CPLD2_SFP_1_8_TXFAULT_REG,
			CPLD2_SFP_9_16_TXFAULT_REG,
			CPLD2_SFP_17_18_TXFAULT_REG,
			CPLD3_SFP_19_26_TXFAULT_REG,
			CPLD3_SFP_27_34_TXFAULT_REG,
			CPLD3_SFP_35_36_TXFAULT_REG,
			CPLD4_SFP_37_44_TXFAULT_REG,
			CPLD4_SFP_45_48_TXFAULT_REG
		},
	},
	{
		.node_name = "sfp_present",
		.active_low = 1,
		.regs = {
			CPLD2_SFP_1_8_ABS_REG,
			CPLD2_SFP_9_16_ABS_REG,
			CPLD2_SFP_17_18_ABS_REG,
			CPLD3_SFP_19_26_ABS_REG,
			CPLD3_SFP_27_34_ABS_REG,
			CPLD3_SFP_35_36_ABS_REG,
			CPLD4_SFP_37_44_ABS_REG,
			CPLD4_SFP_45_48_ABS_REG
		},
	}
};

#define NUM_SFP_REG_NAMES (sizeof(sfp_reg_info) /	\
			   sizeof(struct sfp_reg_info))

static struct sfp_reg_offset_info reg_offset_info[] = {
	{
		.mask = 0xff,
		.shift = 0,
	},
	{
		.mask = 0xff,
		.shift = 8,
	},
	{
		.mask = 0x03,
		.shift = 16,
	},
	{
		.mask = 0xff,
		.shift = 18,
	},
	{
		.mask = 0xff,
		.shift = 26,
	},
	{
		.mask = 0x03,
		.shift = 34,
	},
	{
		.mask = 0xff,
		.shift = 36,
	},
	{
		.mask = 0x0f,
		.shift = 44,
	},
};

static ssize_t sfp_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	struct sfp_reg_info *reg_info = NULL;
	uint64_t val = 0, rval;
	int i;

	for (i = 0; i < NUM_SFP_REG_NAMES; i++) {
		if (strcmp(sfp_reg_info[i].node_name,
			   dattr->attr.name) == 0) {
			reg_info = &sfp_reg_info[i];
			break;
		}
	}

	for (i = 0; i < NUM_SFP_REGS; i++) {
		rval = cpld_reg_read(reg_info->regs[i]);
		val |= (rval & (uint64_t)reg_offset_info[i].mask) <<
			(uint64_t)reg_offset_info[i].shift;
	}
	if (reg_info->active_low)
		val = ~val;

	return sprintf(buf, "0x%012llx\n", val & 0xffffffffffff);
}

static ssize_t sfp_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	struct sfp_reg_offset_info *reg_info;
	uint64_t  data;
	uint32_t *regs = NULL;
	int i;

	if (kstrtoull(buf, 0, &data) != 0)
		return -EINVAL;

	for (i = 0; i < NUM_SFP_REG_NAMES; i++) {
		if (strcmp(sfp_reg_info[i].node_name,
			   dattr->attr.name) == 0) {
			regs = sfp_reg_info[i].regs;
			break;
		}
	}
	for (i = 0; i < NUM_SFP_REGS; i++) {
		reg_info = &reg_offset_info[i];
		cpld_reg_write(regs[i],
			       (data >> reg_info->shift) & reg_info->mask);
	}

	return count;
}
static SYSFS_ATTR_RO(sfp_rx_los,     sfp_show);
static SYSFS_ATTR_RW(sfp_tx_disable, sfp_show, sfp_store);
static SYSFS_ATTR_RW(sfp_rs,         sfp_show, sfp_store);
static SYSFS_ATTR_RO(sfp_tx_fault,   sfp_show);
static SYSFS_ATTR_RO(sfp_present,    sfp_show);

/* sysfs registration */

static struct attribute *cpld_attrs[] = {
	&lpccpld_write_protect.attr,
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&lpccpld_pwm1.attr,
	&lpccpld_pwm2.attr,
	&lpccpld_pwm3.attr,
	&lpccpld_pwm4.attr,
	&lpccpld_pwm5.attr,
	&lpccpld_fan1_present.attr,
	&lpccpld_fan2_present.attr,
	&lpccpld_fan3_present.attr,
	&lpccpld_fan4_present.attr,
	&lpccpld_fan5_present.attr,
	&lpccpld_fan1_direction.attr,
	&lpccpld_fan2_direction.attr,
	&lpccpld_fan3_direction.attr,
	&lpccpld_fan4_direction.attr,
	&lpccpld_fan5_direction.attr,
	&lpccpld_psu_pwr1_alarm.attr,
	&lpccpld_psu_pwr2_alarm.attr,
	&lpccpld_psu_pwr1_present.attr,
	&lpccpld_psu_pwr2_present.attr,
	&lpccpld_psu_pwr1_all_ok.attr,
	&lpccpld_psu_pwr2_all_ok.attr,
	&lpccpld_psu_pwr1_enable.attr,
	&lpccpld_psu_pwr2_enable.attr,
	&lpccpld_qsfp_reset.attr,
	&lpccpld_qsfp_lp_mode.attr,
	&lpccpld_qsfp_present.attr,
	&lpccpld_qsfp_interrupt.attr,
	&sensor_dev_attr_cpld1_version.dev_attr.attr,
	&sensor_dev_attr_cpld2_version.dev_attr.attr,
	&sensor_dev_attr_cpld3_version.dev_attr.attr,
	&sensor_dev_attr_cpld4_version.dev_attr.attr,
	&dev_attr_led_system.attr,
	&dev_attr_led_bmc.attr,
	&dev_attr_sfp_rx_los.attr,
	&dev_attr_sfp_tx_disable.attr,
	&dev_attr_sfp_rs.attr,
	&dev_attr_sfp_tx_fault.attr,
	&dev_attr_sfp_present.attr,
	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

/*
 * CPLD driver interface
 */

static struct platform_device *cpld_device;

static int cpld_probe(struct platform_device *dev)
{
	int ret = 0;

	cpld_regs = ioport_map(CPLD_IO_BASE, CPLD_IO_SIZE);

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
		.name = "cel_redstone_v_cpld",
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

	cpld_device = platform_device_alloc("cel_redstone_v_cpld", 0);
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
	return 0;

err_dealloc:
	platform_device_unregister(cpld_device);

err_unregister:
	platform_driver_unregister(&cpld_driver);

err_exit:
	pr_err("%s platform_driver_register failed (%i)\n",
	       driver_name, rv);
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
MODULE_DESCRIPTION("Celestica Redstone-V platform support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
