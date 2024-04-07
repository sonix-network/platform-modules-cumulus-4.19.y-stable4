/*
 *  delta-ag5648v1-platform.c - Delta AG5648V1 Platform Support.
 *
 *  Copyright (C) 2018,2019 Cumulus Networks, Inc.  All Rights Reserved
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
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/at24.h>
#include <linux/gpio.h>
#include <linux/hwmon-sysfs.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "platform-bitfield.h"
#include "delta-ag5648v1.h"

#define DRIVER_NAME	   "delta_ag5648v1_platform"
#define DRIVER_VERSION	   "1.0"

mk_pca9548(mux1, I2C_MUX1_BUS0, 1);

mk_eeprom(board, 53, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan1,	 51, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan2,	 52, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan3,	 53, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan4,	 54, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu1,	 51, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2,	 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_ISMT_BUS, "pca9548", 0x70, &mux1_platform_data),

	mk_i2cdev(I2C_MUX1_BUS0, "cpld",  0x31, NULL), /* system CPLD */
	mk_i2cdev(I2C_MUX1_BUS0, "cpld",  0x35, NULL), /* master CPLD */
	mk_i2cdev(I2C_MUX1_BUS0, "cpld",  0x39, NULL), /* slave CPLD */
	mk_i2cdev(I2C_MUX1_BUS0, "tmp75", 0x4d, NULL),
	mk_i2cdev(I2C_MUX1_BUS0, "24c02", 0x53, &board_53_at24),

	mk_i2cdev(I2C_MUX1_BUS1, "tmp75", 0x4b, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, "tmp75", 0x4c, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, "tmp75", 0x49, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, "tmp75", 0x4e, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, "tmp75", 0x4f, NULL),
	mk_i2cdev(I2C_MUX1_BUS1, "24c02", 0x51, &fan1_51_at24),
	mk_i2cdev(I2C_MUX1_BUS1, "24c02", 0x52, &fan2_52_at24),
	mk_i2cdev(I2C_MUX1_BUS1, "24c02", 0x53, &fan3_53_at24),
	mk_i2cdev(I2C_MUX1_BUS1, "24c02", 0x54, &fan4_54_at24),
	mk_i2cdev(I2C_MUX1_BUS1, "emc2305", 0x4d, NULL),

	mk_i2cdev(I2C_MUX1_BUS3, "emc2305", 0x4d, NULL),

	mk_i2cdev(I2C_MUX1_BUS4, "24c02", 0x51, &psu1_51_at24),
	mk_i2cdev(I2C_MUX1_BUS4, "24c02", 0x50, &psu2_50_at24),
	mk_i2cdev(I2C_MUX1_BUS4, "dps460", 0x59, NULL),
	mk_i2cdev(I2C_MUX1_BUS4, "dps460", 0x58, NULL),
};

/*
 * The AG5648V1 has three CPLDs: system, master, and slave.  The CPLD driver
 * has been coded such that we have the appearance of a single CPLD.  We do
 * this by embedding a two-bit index, by which CPLD must be accessed, into
 * each register definition.  When the register is accessed, the register
 * definition is decoded into a CPLD index and register offset.
 */

struct platform_data {
	int id;
};

static struct i2c_client *cpld_devices[NUM_CPLD_DEVICES];

/*
 * Simple bitfield CPLD registers
 */

/* bitfield accessor functions */
static int cpld_read_reg(struct device *dev,
			 int reg,
			 int nregs,
			 u32 *val)
{
	int ret;
	int cpld_id = GET_CPLD_ID(reg);

	if (cpld_id < 0 || cpld_id >= NUM_CPLD_DEVICES) {
		pr_err(DRIVER_NAME "Invalid CPLD register read [0x%02X]",
		       reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	ret = i2c_smbus_read_byte_data(cpld_devices[cpld_id], reg);
	if (ret < 0) {
		pr_err(DRIVER_NAME
		       "i2c read error - addr: 0x%02X, offset: 0x%02X",
		       cpld_devices[cpld_id]->addr, reg);
		return -EINVAL;
	}
	*val = ret;
	return 0;
}

static int cpld_write_reg(struct device *dev,
			  int reg,
			  int nregs,
			  u32 val)
{
	int ret = 0;

	int cpld_id = GET_CPLD_ID(reg);

	if (cpld_id < 0 || cpld_id >= NUM_CPLD_DEVICES) {
		pr_err(DRIVER_NAME "invalid CPLD register write at [0x%02X]",
		       reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	ret = i2c_smbus_write_byte_data(cpld_devices[cpld_id], reg, val);
	if (ret) {
		pr_err(DRIVER_NAME
		       "i2c write failed: 0x%02X, reg: 0x%02X, val: 0x%02X",
		       cpld_devices[cpld_id]->addr, reg, val);
	}
	return ret;
}

/* CPLD register bitfields with enum-like values */
static const char * const led_debug_values[] = {
	PLATFORM_LED_ON,
	PLATFORM_LED_OFF,
};

static const char * const led_fan_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_YELLOW_BLINKING,
};

static const char * const led_system_values[] = {
	PLATFORM_LED_GREEN_BLINKING,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_AMBER_BLINKING,
};

static const char * const led_power_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER_BLINKING,
};

static const char * const led_fan_tray_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER,
	"reserved",
};

/* CPLD registers */
mk_bf_ro(cpld, system_board_stage,   0x40, 4, 4, NULL, 0);
mk_bf_ro(cpld, system_platform_type, 0x40, 0, 2, NULL, 0);
mk_bf_rw(cpld, ssd_present,	     0x41, 7, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, cold_rst,	     0x41, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, cpld_upgrade_rst,     0x41, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, cpu_rst,		     0x41, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, eeprom_wp,	     0x43, 4, 1, NULL, 0);
mk_bf_rw(cpld, spi_bios_wp,	     0x43, 2, 1, NULL, 0);
mk_bf_rw(cpld, wd_timer,	     0x47, 4, 3, NULL, 0);
mk_bf_rw(cpld, wd_en,		     0x47, 3, 1, NULL, 0);
mk_bf_rw(cpld, wdi_flag,	     0x47, 0, 1, NULL, 0);
mk_bf_ro(cpld, system_cpld_rev,	     0x48, 0, 7, NULL, 0);

mk_bf_ro(cpld, master_board_stage,   0x81, 4, 4, NULL, 0);
mk_bf_ro(cpld, master_platform_type, 0x81, 0, 2, NULL, 0);
mk_bf_rw(cpld, main_board_hrst,	     0x82, 5, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, mac_rst,		     0x82, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, oob_rst,		     0x82, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr1_present,     0x83, 7, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr1_all_ok,	     0x83, 6, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr1_int,	     0x83, 5, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, psu_pwr1_enable,	     0x83, 4, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr2_present,     0x83, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr2_all_ok,	     0x83, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, psu_pwr2_int,	     0x83, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, psu_pwr2_enable,	     0x83, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_modsel,        0x90, 0, 6, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_lpmod,         0x91, 0, 6, NULL, 0);
mk_bf_ro(cpld, qsfp28_present,       0x92, 0, 6, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, qsfp28_reset,         0x93, 0, 6, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, qsfp28_interrupt,     0x94, 0, 6, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, led_debug2,	     0x96, 2, 1, led_debug_values, 0);
mk_bf_rw(cpld, led_debug1,	     0x96, 1, 1, led_debug_values, 0);
mk_bf_rw(cpld, led_debug0,	     0x96, 0, 1, led_debug_values, 0);
mk_bf_ro(cpld, master_cpld_rev,	     0x97, 0, 7, NULL, 0);

mk_bf_ro(cpld, slave_cpld_rev,	     0xc1, 0, 7, NULL, 0);
mk_bf_rw(cpld, led_fan,		     0xc4, 6, 2, led_fan_values, 0);
mk_bf_rw(cpld, led_system,	     0xc4, 4, 2, led_system_values, 0);
mk_bf_rw(cpld, led_power,	     0xc4, 1, 2, led_power_values, 0);
mk_bf_rw(cpld, led_fan_tray4,	     0xc5, 6, 2, led_fan_tray_values, 0);
mk_bf_rw(cpld, led_fan_tray3,	     0xc5, 4, 2, led_fan_tray_values, 0);
mk_bf_rw(cpld, led_fan_tray2,	     0xc5, 2, 2, led_fan_tray_values, 0);
mk_bf_rw(cpld, led_fan_tray1,	     0xc5, 0, 2, led_fan_tray_values, 0);
mk_bf_ro(cpld, fan4_present,	     0xc6, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fan3_present,	     0xc6, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fan2_present,	     0xc6, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cpld, fan1_present,	     0xc6, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, fan4_eeprom_wp,	     0xc7, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, fan3_eeprom_wp,	     0xc7, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, fan2_eeprom_wp,	     0xc7, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cpld, fan1_eeprom_wp,	     0xc7, 0, 1, NULL, BF_COMPLEMENT);

/*
 * Complex QSFP registers that span more than one register
 */

struct port_status {
	char name[CPLD_STRING_NAME_SIZE];
	u8 index;
	u8 active_low;
	u8 num_bits;
	u8 num_regs;
	u8 reg[MAX_REGS];
};

static struct port_status sfp28_status[] = {
	{
		.name = "present",
		.index = _PRESENT_IDX,
		.active_low = 1,
		.num_bits = 48,
		.num_regs = 7,
		.reg = { SFP28_8_1_MODULE_ABSENT_REG,
			 SFP28_16_9_MODULE_ABSENT_REG,
			 SFP28_24_17_MODULE_ABSENT_REG,
			 SFP28_32_25_MODULE_ABSENT_REG,
			 SFP28_36_33_MODULE_ABSENT_REG,
			 SFP28_44_37_MODULE_ABSENT_REG,
			 SFP28_48_45_MODULE_ABSENT_REG, },
	},
	{
		.name = "tx_disable",
		.index = _TX_DISABLE_IDX,
		.active_low = 0,
		.num_bits = 48,
		.num_regs = 7,
		.reg = { SFP28_8_1_MODULE_TX_DISABLE_REG,
			 SFP28_16_9_MODULE_TX_DISABLE_REG,
			 SFP28_24_17_MODULE_TX_DISABLE_REG,
			 SFP28_32_25_MODULE_TX_DISABLE_REG,
			 SFP28_36_33_MODULE_TX_DISABLE_REG,
			 SFP28_44_37_MODULE_TX_DISABLE_REG,
			 SFP28_48_45_MODULE_TX_DISABLE_REG, },
	},
	{
		.name = "rx_los",
		.index = _RX_LOS_IDX,
		.active_low = 0,
		.num_bits = 48,
		.num_regs = 7,
		.reg = { SFP28_8_1_MODULE_RX_LOS_REG,
			 SFP28_16_9_MODULE_RX_LOS_REG,
			 SFP28_24_17_MODULE_RX_LOS_REG,
			 SFP28_32_25_MODULE_RX_LOS_REG,
			 SFP28_36_33_MODULE_RX_LOS_REG,
			 SFP28_44_37_MODULE_RX_LOS_REG,
			 SFP28_48_45_MODULE_RX_LOS_REG, },
	},
	{
		.name = "tx_fault",
		.index = _TX_FAULT_IDX,
		.active_low = 0,
		.num_bits = 48,
		.num_regs = 7,
		.reg = { SFP28_8_1_MODULE_TX_FAULT_REG,
			 SFP28_16_9_MODULE_TX_FAULT_REG,
			 SFP28_24_17_MODULE_TX_FAULT_REG,
			 SFP28_32_25_MODULE_TX_FAULT_REG,
			 SFP28_36_33_MODULE_TX_FAULT_REG,
			 SFP28_44_37_MODULE_TX_FAULT_REG,
			 SFP28_48_45_MODULE_TX_FAULT_REG, },
	 },
};

struct reg_offset {
	uint8_t mask;
	uint8_t shift;
};

static struct reg_offset sfp28_offset[] = {
	{ 0xff,	 0 },
	{ 0xff,	 8 },
	{ 0xff, 16 },
	{ 0xff, 24 },
	{ 0x0f, 32 },
	{ 0xff, 36 },
	{ 0xf0, 40 },
};

/* Functions for reading and writing the CPLD port config registers */
static ssize_t master_show(struct device *dev,
			   struct device_attribute *dattr,
			   char *buf)
{
	int i;
	int res;
	u32 reg_val;
	u64 val = 0;
	struct port_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);
	/* find the target register */
	target = &sfp28_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	for (i = 0; i < target->num_regs; i++) {
		res = cpld_read_reg(dev, target->reg[i], 1, &reg_val);
		val |= (reg_val & (uint64_t)sfp28_offset[i].mask) <<
			(uint64_t)sfp28_offset[i].shift;
	}
	if (target->active_low)
		val = ~val;

	return sprintf(buf, "0x%012llx\n", val & 0xffffffffffff);
}

static ssize_t master_store(struct device *dev,
			    struct device_attribute *dattr,
			    const char *buf,
			    size_t count)
{
	int i;
	int res;
	uint64_t val;
	struct port_status *target = NULL;
	struct sensor_device_attribute *sensor_dev_attr = NULL;

	sensor_dev_attr = to_sensor_dev_attr(dattr);

	if (kstrtoll(buf, 0, &val) != 0)
		return -EINVAL;

	/* find the target register */
	target = &sfp28_status[sensor_dev_attr->index];
	if (!target)
		return -EINVAL;

	for (i = 0; i < target->num_regs; i++) {
		res = cpld_write_reg(dev, target->reg[i], 1,
				     target->active_low ?
				     ~(val >> sfp28_offset[i].shift) & sfp28_offset[i].mask :
				     (val >> sfp28_offset[i].shift) & sfp28_offset[i].mask);

		if (res < 0) {
			pr_err(DRIVER_NAME
			       "CPLD sfp28 register (%s) write failed",
			       target->name);
			return res;
		}
	}
	return count;
}

static SENSOR_DEVICE_ATTR_RO(sfp28_present, master_show, 0);
static SENSOR_DEVICE_ATTR_RW(sfp28_tx_disable, master_show, master_store, 1);
static SENSOR_DEVICE_ATTR_RO(sfp28_rx_los, master_show, 2);
static SENSOR_DEVICE_ATTR_RO(sfp28_tx_fault, master_show, 3);

/*
 * CPLD driver interface
 */

/* sysfs registration */
static struct attribute *cpld_attrs[] = {
	&cpld_system_board_stage.attr,
	&cpld_system_platform_type.attr,
	&cpld_ssd_present.attr,
	&cpld_cold_rst.attr,
	&cpld_cpld_upgrade_rst.attr,
	&cpld_cpu_rst.attr,
	&cpld_eeprom_wp.attr,
	&cpld_spi_bios_wp.attr,
	&cpld_wd_timer.attr,
	&cpld_wd_en.attr,
	&cpld_wdi_flag.attr,
	&cpld_system_cpld_rev.attr,

	&cpld_master_board_stage.attr,
	&cpld_master_platform_type.attr,
	&cpld_main_board_hrst.attr,
	&cpld_mac_rst.attr,
	&cpld_oob_rst.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_psu_pwr1_int.attr,
	&cpld_psu_pwr1_enable.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr2_all_ok.attr,
	&cpld_psu_pwr2_int.attr,
	&cpld_psu_pwr2_enable.attr,
	&cpld_qsfp28_modsel.attr,
	&cpld_qsfp28_lpmod.attr,
	&cpld_qsfp28_present.attr,
	&cpld_qsfp28_reset.attr,
	&cpld_qsfp28_interrupt.attr,
	&cpld_led_debug2.attr,
	&cpld_led_debug1.attr,
	&cpld_led_debug0.attr,
	&cpld_master_cpld_rev.attr,
	&sensor_dev_attr_sfp28_present.dev_attr.attr,
	&sensor_dev_attr_sfp28_tx_disable.dev_attr.attr,
	&sensor_dev_attr_sfp28_rx_los.dev_attr.attr,
	&sensor_dev_attr_sfp28_tx_fault.dev_attr.attr,

	&cpld_slave_cpld_rev.attr,
	&cpld_led_fan.attr,
	&cpld_led_system.attr,
	&cpld_led_power.attr,
	&cpld_led_fan_tray4.attr,
	&cpld_led_fan_tray3.attr,
	&cpld_led_fan_tray2.attr,
	&cpld_led_fan_tray1.attr,
	&cpld_fan4_present.attr,
	&cpld_fan3_present.attr,
	&cpld_fan2_present.attr,
	&cpld_fan1_present.attr,
	&cpld_fan4_eeprom_wp.attr,
	&cpld_fan3_eeprom_wp.attr,
	&cpld_fan2_eeprom_wp.attr,
	&cpld_fan1_eeprom_wp.attr,
	NULL,
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

/*
 * The AG5648V1 has eight 8:1 mux/demux devices for steering the I2C bus to the
 * correct SFF module.  The mux selects are controlled by the "SFP28/QSFP28
 * I2C MUX Select Register" in the Master CPLD.
 */

#define INVALID_MUX_CHANNEL (0x00)

struct port_mux_info_item {
	int num_items;
	struct platform_device *mux_dev;
	struct i2c_mux_core *muxc;
	struct i2c_client **mux_clients;
	struct i2c_board_info **mux_board_infos;
};

struct port_mux_info_struct {
	struct port_mux_info_item mux_item[NUM_SFF_PORTS];
	int num_port_muxes;
};

static struct port_mux_info_struct *port_mux_info;

/*
 * Invalid QSFP mux number
 */

static bool is_qsfp_port(int port)
{
	if ((port >= FIRST_QSFP28_PORT) &
	    (port < FIRST_QSFP28_PORT + NUM_QSFP28_PORTS))
		return true;
	else
		return false;
}

static struct i2c_board_info *alloc_port_mux_board_info(int port)
{
	char *label = NULL;
	struct eeprom_platform_data *eeprom_data = NULL;
	struct i2c_board_info *board_info = NULL;

	label = kzalloc(PORT_LABEL_SIZE, GFP_KERNEL);
	if (!label)
		goto err_kzalloc;

	eeprom_data = kzalloc(sizeof(*eeprom_data), GFP_KERNEL);
	if (!eeprom_data)
		goto err_kzalloc;

	board_info = kzalloc(sizeof(*board_info), GFP_KERNEL);
	if (!board_info)
		goto err_kzalloc;

	snprintf(label, PORT_LABEL_SIZE, "port%u", port);
	eeprom_data->label = label;

	if (is_qsfp_port(port)) {
		struct sff_8436_platform_data *sff8436_data;

		sff8436_data = kzalloc(sizeof(*sff8436_data),
				       GFP_KERNEL);
		if (!sff8436_data)
			goto err_kzalloc;

		sff8436_data->byte_len = 256;
		sff8436_data->flags = SFF_8436_FLAG_IRUGO;
		sff8436_data->page_size = 1;
		sff8436_data->eeprom_data = eeprom_data;
		board_info->platform_data = sff8436_data;
		strcpy(board_info->type, "sff8436");
	} else {
		struct at24_platform_data *at24_data;

		at24_data = kzalloc(sizeof(*at24_data),
				    GFP_KERNEL);
		if (!at24_data)
			goto err_kzalloc;

		at24_data->byte_len = 512;
		at24_data->flags = AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG;
		at24_data->page_size = 1;
		at24_data->eeprom_data = eeprom_data;
		board_info->platform_data = at24_data;
		strcpy(board_info->type, "24c04");
	}

	board_info->addr = 0x50;

	return board_info;

 err_kzalloc:
	kfree(board_info);
	kfree(eeprom_data);
	kfree(label);
	return NULL;
}

static struct i2c_adapter *mux1_bus0_adap;

static int qsfp_mux_i2c_reg_write(uint8_t reg, u8 val)
{
	int rv;
	struct i2c_client *client;
	struct i2c_adapter *adap;

	adap = mux1_bus0_adap;
	client = cpld_devices[GET_CPLD_ID(reg)];
	reg = STRIP_CPLD_IDX(reg);

	if (adap->algo->master_xfer) {
		struct i2c_msg msg = { 0 };
		char buf[2];

		msg.addr = client->addr;
		msg.flags = 0;
		msg.len = 2;
		buf[0] = reg;
		buf[1] = val;
		msg.buf = buf;
		rv = adap->algo->master_xfer(adap, &msg, 1);
	} else {
		union i2c_smbus_data data;

		data.byte = val;
		rv = adap->algo->smbus_xfer(adap, client->addr,
					    client->flags,
					    I2C_SMBUS_WRITE,
					    reg, I2C_SMBUS_BYTE_DATA, &data);
	}
	return rv;
}

static int cpld_sff_mux_select_chan(struct i2c_mux_core *muxc,
				    u32 chan)
{
	static u32 prev_chan = INVALID_MUX_CHANNEL;
	uint8_t val;
	int ret;

	if (likely(chan == prev_chan))
		return 0;

	prev_chan = chan;

	val = chan - I2C_FIRST_SFF_PORT + 1;
	ret = qsfp_mux_i2c_reg_write(SFP28_QSFP28_I2C_MUX_SELECT_REG, val);
	if (ret < 0)
		return ret;

	return 0;
}

/*-----------------------------------------------------------------------------
 *
 * Initialization routines
 *
 */

/* I2C initialization */
static struct i2c_client *cpld_devices[NUM_CPLD_DEVICES];
static int num_cpld_devices;

static void i2c_exit(void)
{
	int i;

	for (i = ARRAY_SIZE(i2c_devices); --i >= 0;) {
		struct i2c_client *c = i2c_devices[i].client;

		if (c) {
			i2c_devices[i].client = NULL;
			i2c_unregister_device(c);
		}
	}
}

static int __init i2c_init(void)
{
	int ismt_bus = -1;
	int i;
	int ret;

	ismt_bus = cumulus_i2c_find_adapter(SMB_ISMT_NAME);
	if (ismt_bus < 0) {
		pr_err(DRIVER_NAME "could not find iSMT adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	num_cpld_devices = 0;
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == I2C_ISMT_BUS)
			bus = ismt_bus;

		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_devices[i].client = client;
		if (strcmp(i2c_devices[i].board_info.type, "cpld") == 0) {
			cpld_devices[num_cpld_devices] = client;
			num_cpld_devices++;
		}
	}
	return 0;

err_exit:
	i2c_exit();
	return ret;
}

/* CPLD Initialization */
static int cpld_probe(struct platform_device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->dev.kobj, &cpld_attr_group);
	if (ret)
		pr_err(DRIVER_NAME
		       "sysfs_cpld_driver_group failed for cpld driver");

	return ret;
}

static int cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver cpld_driver = {
	.driver = {
		.name = "delta_ag5648v1_cpld",
		.owner = THIS_MODULE,
	},
	.probe = cpld_probe,
	.remove = cpld_remove,
};

static struct platform_device *cpld_device;

static int __init cpld_init(void)
{
	int ret;

	if (num_cpld_devices != NUM_CPLD_DEVICES) {
		pr_err(DRIVER_NAME
		       "incorrect number of CPLD devices (%d); expected: %d\n",
		       num_cpld_devices, NUM_CPLD_DEVICES);
		return -ENODEV;
	}

	ret = platform_driver_register(&cpld_driver);
	if (ret) {
		pr_err(DRIVER_NAME
		       "platform_driver_register() failed for cpld device\n");
		return ret;
	}

	cpld_device = platform_device_alloc("delta_ag5648v1_cpld", 0);
	if (!cpld_device) {
		pr_err(DRIVER_NAME
		       "platform_device_alloc() failed for cpld device\n");
		platform_driver_unregister(&cpld_driver);
		return -ENOMEM;
	}

	ret = platform_device_add(cpld_device);
	if (ret) {
		pr_err(DRIVER_NAME
		       "platform_device_add() failed for cpld device.\n");
		platform_device_put(cpld_device);
		return ret;
	}

	pr_info(DRIVER_NAME "CPLD driver loaded\n");
	return 0;
}

static void cpld_exit(void)
{
	platform_driver_unregister(&cpld_driver);
	platform_device_unregister(cpld_device);
	pr_err(DRIVER_NAME "CPLD driver unloaded\n");
}

/* SFF port mux initialization */
static void free_port_mux_board_info(struct i2c_board_info *board_info)
{
	struct sff_8436_platform_data *sff8436_data =
		board_info->platform_data;
	struct eeprom_platform_data *eeprom_data = sff8436_data->eeprom_data;

	kfree(eeprom_data->label);
	kfree(eeprom_data);
	kfree(sff8436_data);
	kfree(board_info);
}

static void port_mux_info_free_data(void)
{
	int i, j;
	struct i2c_mux_core *muxc;

	if (port_mux_info) {
		for (i = 0; i < port_mux_info->num_port_muxes; i++) {
			for (j = 0; j < port_mux_info->mux_item[i].num_items;
			     j++) {
				if (port_mux_info->mux_item[i].mux_clients[j])
					i2c_unregister_device(port_mux_info->
					    mux_item[i].mux_clients[j]);
				if (port_mux_info->
				    mux_item[i].mux_board_infos[j])
					free_port_mux_board_info(port_mux_info->
					    mux_item[i].mux_board_infos[j]);
			}
			muxc = port_mux_info->mux_item[i].muxc;
			if (muxc)
				i2c_mux_del_adapters(muxc);
			kfree(port_mux_info->mux_item[i].mux_clients);
			kfree(port_mux_info->mux_item[i].mux_board_infos);
			platform_device_unregister(port_mux_info->
						   mux_item[i].mux_dev);
		}
		kfree(port_mux_info);
		port_mux_info = NULL;
	}
}

static int create_port_mux(int bus, int num_mux_clients, int first_port_num,
			   int first_mux_num,
			   const char *mux_name,
			   int (select_function(struct i2c_mux_core *, u32)))
{
	struct i2c_adapter *adapter;
	struct port_mux_info_item *mux_item;
	int ret;
	int i;

	adapter = i2c_get_adapter(bus);
	if (!adapter) {
		pr_err(DRIVER_NAME
		       "Could not find i2c adapter for %s bus %d.\n", mux_name,
		       bus);
		return -ENODEV;
	}

	mux_item = &port_mux_info->mux_item[port_mux_info->num_port_muxes];

	mux_item->mux_clients = kzalloc(num_mux_clients *
					sizeof(struct i2c_client *),
					GFP_KERNEL);
	if (!mux_item->mux_clients)
		goto err_exit;

	mux_item->mux_board_infos = kzalloc(num_mux_clients *
					    sizeof(struct qsfp_board_info *),
					    GFP_KERNEL);
	if (!mux_item->mux_board_infos)
		goto err_exit_clients;

	mux_item->num_items = num_mux_clients;

	mux_item->mux_dev = platform_device_alloc(mux_name, 0);
	if (!mux_item->mux_dev) {
		pr_err(DRIVER_NAME
		       "platform_device_alloc() failed for %s.\n", mux_name);
		goto err_exit_infos;
	}

	ret = platform_device_add(mux_item->mux_dev);
	if (ret) {
		pr_err(DRIVER_NAME
		       "platform_device_add() failed for %s.\n", mux_name);
		goto err_exit_device;
	}
	mux_item->muxc = i2c_mux_alloc(adapter, &mux_item->mux_dev->dev,
				       num_mux_clients, 0, 0,
				       select_function, NULL);
	if (!mux_item->muxc) {
		pr_err(DRIVER_NAME
		       "i2c_mux_alloc() failed for mux %s.\n", mux_name);
		ret = -ENOMEM;
		goto err_exit_device;
	}

	port_mux_info->num_port_muxes++;

	for (i = 0; i < num_mux_clients; i++) {
		struct i2c_board_info *eeprom_info;
		struct i2c_client *client;
		int mux_num;

		mux_num = first_mux_num + i;
		ret = i2c_mux_add_adapter(mux_item->muxc, mux_num, mux_num, 0);
		if (ret) {
			pr_err(DRIVER_NAME
			       ":i2c_mux_add_adapter failed for channel %u.\n",
			       mux_num);
			goto err_exit;
		}

		eeprom_info = alloc_port_mux_board_info(first_port_num + i);
		if (!eeprom_info) {
			ret = -ENOMEM;
			goto err_exit;
		}

		mux_item->mux_board_infos[i] = eeprom_info;
		client = i2c_new_device(mux_item->muxc->adapter[i],
					eeprom_info);
		if (!client) {
			pr_err(DRIVER_NAME
			       ":i2c_new_device failed for %s device.\n",
			       mux_name);
			goto err_exit;
		}
		mux_item->mux_clients[i] = client;
	}
	i2c_put_adapter(adapter);
	return 0;

 err_exit_device:
	platform_device_put(mux_item->mux_dev);

 err_exit_infos:
	kfree(mux_item->mux_board_infos);

 err_exit_clients:
	kfree(mux_item->mux_clients);

 err_exit:
	port_mux_info_free_data();
	return -ENOMEM;
}

static int __init port_mux_init(void)
{
	int ret;

	port_mux_info = kzalloc(sizeof(*port_mux_info),	GFP_KERNEL);
	mux1_bus0_adap = i2c_get_adapter(I2C_MUX1_BUS0);
	if (!port_mux_info)
		return -ENOMEM;
	ret = create_port_mux(I2C_MUX1_BUS2, NUM_SFF_PORTS,
			      FIRST_SFP28_PORT, I2C_FIRST_SFF_PORT,
			      "cpld-sff-mux",
			      cpld_sff_mux_select_chan);
	i2c_put_adapter(mux1_bus0_adap);
	return ret;
}

static void __exit port_mux_exit(void)
{
	port_mux_info_free_data();
}

/*---------------------------------------------------------------------
 *
 * Module init/exit
 */

static int __init delta_ag5648v1_init(void)
{
	int ret = 0;

	ret = i2c_init();
	if (ret) {
		pr_err(DRIVER_NAME "I2C subsystem initialization failed\n");
		return ret;
	}

	ret = cpld_init();
	if (ret) {
		pr_err(DRIVER_NAME "CPLD initialization failed\n");
		i2c_exit();
		return ret;
	}

	ret = port_mux_init();
	if (ret) {
		pr_err(DRIVER_NAME "SFP28/QSFP28 mux initialization failed\n");
		cpld_exit();
		i2c_exit();
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit delta_ag5648v1_exit(void)
{
	port_mux_exit();
	cpld_exit();
	i2c_exit();
	pr_err(DRIVER_NAME " driver unloaded\n");
}

module_init(delta_ag5648v1_init);
module_exit(delta_ag5648v1_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Delta AG5648V1 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
