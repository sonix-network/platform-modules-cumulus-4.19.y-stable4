/*
 * Accton AS4222-28PE Platform Support
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
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/i2c-ismt.h>
#include <linux/i2c-mux.h>
#include <linux/i2c-mux-gpio.h>
#include <linux/interrupt.h>
#include <linux/stddef.h>
#include <linux/acpi.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/i2c/sff-8436.h>
#include <linux/i2c/pca954x.h>

#include <linux/cumulus-platform.h>
#include "platform_defs.h"
#include "platform_bitfield.h"

#define DRIVER_NAME	"accton_as4222_28pe_platform"
#define DRIVER_VERSION	"1.0"

#define ISMT_ADAPTER_NAME	  "SMBus iSMT adapter"
#define I801_ADAPTER_NAME	  "SMBus I801 adapter"

/*
 * This platform has both an i801 adapter (i2c-0) and an iSMT adapter (i2c-1).
 *
 * The i801 adapter is connected to the following devices:
 *
 *    so-dimm spd eeprom (0x52)
 *    so-dimm spd eeprom (0x53) - not present
 *    clock gen (0x69)
 *    potentiometer (0x2e)
 *    pca9548 (0x72)
 *	 0 ucd9090 (0x0c)
 *	 1 usb hub (0x2c)
 *	   eeprom (0x57)
 *	 2 poe mcu (0x20)
 *	   rtc (0x68)
 *	 3 sfp+0 (0x50, 0x51)
 *	 4 sfp+1 (0x50, 0x51)
 *	 5 sfp+2 (0x50, 0x51)
 *	 6 sfp+3 (0x50, 0x51)
 *	 7 lm75 (0x48)
 *	   lm75 (0x49)
 *	   lm75 (0x4a)
 *	   lm75 (0x4b)
 *
 * The iSMT adapter is connected to the following devices:
 *
 *    board eeprom (0x57)
 *    cplda (0x60)
 *
 */

enum {
	I2C_I801_BUS = -1,
	I2C_ISMT_BUS,

	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,
	I2C_MUX1_BUS4,
	I2C_MUX1_BUS5,
	I2C_MUX1_BUS6,
	I2C_MUX1_BUS7,

	I2C_MUX2_BUS0,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,
	I2C_MUX2_BUS4,
	I2C_MUX2_BUS5,
	I2C_MUX2_BUS6,
	I2C_MUX2_BUS7,

};

/*
 * The list of i2c devices and their bus connections for this platform.
 *
 * First we construct the necessary data struction for each device,
 * using the method specific to the device type.  Then we put them
 * all together in a big table (see i2c_devices below).
 *
 * For muxes, we specify the starting bus number for the block of ports,
 * using the magic mk_pca954*() macros.
 *
 * For eeproms, including ones in the qsfp+ transceivers,
 * we specify the label, i2c address, size, and some flags,
 * all done in mk*_eeprom() macros.  The label is the string
 * that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 *
 * See the comment below for gpio.
 */

mk_eeprom(board, 57, 256, AT24_FLAG_IRUGO);

mk_pca9548(mux1, I2C_MUX1_BUS0, 1);
mk_pca9548(mux2, I2C_MUX2_BUS0, 1);

mk_port_eeprom(port25, 50, 512, AT24_FLAG_IRUGO);
mk_port_eeprom(port26, 50, 512, AT24_FLAG_IRUGO);
mk_port_eeprom(port27, 50, 512, AT24_FLAG_IRUGO);
mk_port_eeprom(port28, 50, 512, AT24_FLAG_IRUGO);

/*
 * i2c device tables
 *
 * We use the magic mk_i2cdev() macro to construct the entries.  Each entry is
 * a bus number and an i2c_board_info.	The i2c_board_info structure specifies
 * the device type, address, and platform data specific to the device type.
 *
 * The i2c_devices[] table contains all the devices that we expose on the
 * Denverton i2c busses.
 *
 */

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_ISMT_BUS,	 "24c02",   0x57, &board_57_at24),
	mk_i2cdev(I2C_ISMT_BUS,	 "pca9548", 0x77, &mux1_platform_data),
	mk_i2cdev(I2C_MUX1_BUS1, "cplda",   0x60, NULL),
	mk_i2cdev(I2C_MUX1_BUS0, "pca9548", 0x72, &mux2_platform_data),

	mk_i2cdev(I2C_MUX2_BUS2, "poeuc",   0x20, NULL),
	mk_i2cdev(I2C_MUX2_BUS3, "24c02",   0x50, &port25_50_at24),
	mk_i2cdev(I2C_MUX2_BUS4, "24c02",   0x50, &port26_50_at24),
	mk_i2cdev(I2C_MUX2_BUS5, "24c02",   0x50, &port27_50_at24),
	mk_i2cdev(I2C_MUX2_BUS6, "24c02",   0x50, &port28_50_at24),

	mk_i2cdev(I2C_MUX2_BUS7, "lm75",    0x48, NULL),
	mk_i2cdev(I2C_MUX2_BUS7, "lm75",    0x49, NULL),
	mk_i2cdev(I2C_MUX2_BUS7, "lm75",    0x4a, NULL),
	mk_i2cdev(I2C_MUX2_BUS7, "lm75",    0x4b, NULL),
};

static struct i2c_client *poeuc_device;

/* I2C Initialization */

static void i2c_exit(void)
{
	int i;
	struct i2c_client *c;

	/* Unregister the i2c clients */
	for (i = ARRAY_SIZE(i2c_devices); --i >= 0;) {
		c = i2c_devices[i].client;
		if (c)
			i2c_devices[i].client = NULL;
			i2c_unregister_device(c);
	}

	pr_info("I2C driver unloaded\n");
}

static int i2c_init(void)
{
	int ismt_bus;
	int i801_bus;
	int ret;
	int i;
	int bus;
	struct i2c_client *client;
	int count;

	ismt_bus = cumulus_i2c_find_adapter(ISMT_ADAPTER_NAME);
	if (ismt_bus < 0) {
		pr_err("Could not find iSMT adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	i801_bus = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (i801_bus < 0) {
		pr_err("Could not find the i801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	count = 0;
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		bus = i2c_devices[i].bus;
		if (bus == I2C_ISMT_BUS)
			bus = ismt_bus;
		else if (bus == I2C_I801_BUS)
			bus = i801_bus;
		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			pr_err("Add client failed for bus %d: %d\n",
			       bus, ret);
			goto err_exit;
		}
		i2c_devices[i].client = client;
		if (strcmp(i2c_devices[i].board_info.type, "poeuc") == 0)
			poeuc_device = client;
	}
	pr_info("I2C driver loaded\n");
	return 0;

err_exit:
	i2c_exit();
	return ret;
}

/* bitfield accessor functions */

#define cplda_read_reg cumulus_bf_i2c_read_reg
#define cplda_write_reg cumulus_bf_i2c_write_reg

/* CPLDA register bitfields with enum-like values */

static const char * const led_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_AMBER_BLINKING,
};

static const char * const freq_values[] = {
	"16Hz",
	"4Hz",
	"1Hz",
	"0.25Hz",
};

/* CPLDA registers */

mk_bf_ro(cplda, product_id,	     0x01, 4, 2, NULL, 0);
mk_bf_ro(cplda, pcb_version,	     0x01, 0, 3, NULL, 0);

mk_bf_ro(cplda, sfp2_tx_fault,	     0x02, 5, 1, NULL, 0);
mk_bf_ro(cplda, sfp2_rx_los,	     0x02, 4, 1, NULL, 0);
mk_bf_ro(cplda, sfp1_tx_fault,	     0x02, 1, 1, NULL, 0);
mk_bf_ro(cplda, sfp1_rx_los,	     0x02, 0, 1, NULL, 0);

mk_bf_ro(cplda, sfp4_tx_fault,	     0x03, 5, 1, NULL, 0);
mk_bf_ro(cplda, sfp4_rx_los,	     0x03, 4, 1, NULL, 0);
mk_bf_ro(cplda, sfp3_tx_fault,	     0x03, 1, 1, NULL, 0);
mk_bf_ro(cplda, sfp3_rx_los,	     0x03, 0, 1, NULL, 0);

mk_bf_rw(cplda, reset_led,	     0x04, 5, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_ucd9090,	     0x04, 4, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_mac_spi,	     0x04, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_poe,	     0x04, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_gl850g,	     0x04, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_pca9548,	     0x04, 0, 1, NULL, BF_COMPLEMENT);
                                                                     
mk_bf_rw(cplda, reset_phy1,	     0x05, 6, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_bcm5241,	     0x05, 5, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_i210,	     0x05, 4, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_mac_pcie,	     0x05, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_mac,	     0x05, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_cpu,	     0x05, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_cplda,	     0x06, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, reset_button_enable, 0x06, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_rw(cplda, sfp4_tx_disable,     0x09, 7, 1, NULL, 0);
mk_bf_rw(cplda, sfp3_tx_disable,     0x09, 6, 1, NULL, 0);
mk_bf_rw(cplda, sfp2_tx_disable,     0x09, 5, 1, NULL, 0);
mk_bf_rw(cplda, sfp1_tx_disable,     0x09, 4, 1, NULL, 0);
mk_bf_ro(cplda, sfp4_present,	     0x09, 3, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cplda, sfp3_present,	     0x09, 2, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cplda, sfp2_present,	     0x09, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cplda, sfp1_present,	     0x09, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_rw(cplda, psu_pwr_all_ok,	     0x0b, 5, 1, NULL, 0);
mk_bf_rw(cplda, poe_enable,	     0x0b, 4, 1, NULL, 0);
mk_bf_rw(cplda, sfp4_enable,	     0x0b, 3, 1, NULL, 0);
mk_bf_rw(cplda, sfp3_enable,	     0x0b, 2, 1, NULL, 0);
mk_bf_rw(cplda, sfp2_enable,	     0x0b, 1, 1, NULL, 0);
mk_bf_rw(cplda, sfp1_enable,	     0x0b, 0, 1, NULL, 0);

mk_bf_ro(cplda, sfp4_pwr_good,	     0x0c, 7, 1, NULL, 0);
mk_bf_ro(cplda, sfp3_pwr_good,	     0x0c, 6, 1, NULL, 0);
mk_bf_ro(cplda, sfp2_pwr_good,	     0x0c, 5, 1, NULL, 0);
mk_bf_ro(cplda, sfp1_pwr_good,	     0x0c, 4, 1, NULL, 0);

mk_bf_ro(cplda, 12v_pwr_good,	     0x0c, 1, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cplda, 48v_pwr_good,	     0x0c, 0, 1, NULL, BF_COMPLEMENT);

mk_bf_ro(cplda, vcc1p0_phy_pwr_good, 0x0d, 5, 1, NULL, 0);
mk_bf_ro(cplda, vcc1p0_mac_pwr_good, 0x0d, 4, 1, NULL, 0);
mk_bf_ro(cplda, tps51200_pwr_good,   0x0d, 3, 1, NULL, 0);
mk_bf_ro(cplda, vcc1p5_pwr_good,     0x0d, 2, 1, NULL, 0);
mk_bf_ro(cplda, vcc3p3_pwr_good,     0x0d, 1, 1, NULL, 0);
mk_bf_ro(cplda, vcc5p0_pwr_good,     0x0d, 0, 1, NULL, 0);

mk_bf_ro(cplda, mac_avs0,	     0x11, 1, 1, NULL, 0);
mk_bf_ro(cplda, poe_present,	     0x11, 0, 1, NULL, 0);

mk_bf_ro(cplda, usb_pwr_good,	     0x12, 3, 1, NULL, 0);
mk_bf_ro(cplda, cpu_thermal_trip,    0x12, 2, 1, NULL, 0);
mk_bf_ro(cplda, cplda_int_cpu,	     0x12, 1, 1, NULL, 0);
mk_bf_ro(cplda, usb1_vbus,	     0x12, 0, 1, NULL, 0);

mk_bf_rw(cplda, enable_leds,	     0x1d, 0, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, led_psu,	     0x1a, 4, 3, led_values, 0);
mk_bf_rw(cplda, led_system,	     0x1a, 0, 3, led_values, 0);
mk_bf_rw(cplda, led_poe,	     0x1b, 4, 3, led_values, 0);
mk_bf_rw(cplda, led_fan,	     0x1b, 0, 3, led_values, 0);

mk_bf_rw(cplda, power_reset,	     0x40, 0, 1, NULL, 0);
mk_bf_rw(cplda, cplda_wp,	     0x50, 7, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, fan_pwm,	     0x60, 0, 4, NULL, 0); /* 1 to 8 */
mk_bf_rw(cplda, fan1_ok,	     0x64, 7, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, fan2_ok,	     0x64, 6, 1, NULL, BF_COMPLEMENT);
mk_bf_rw(cplda, fan3_ok,	     0x64, 5, 1, NULL, BF_COMPLEMENT);
mk_bf_ro(cplda, cplda_version,	     0x7c, 0, 8, NULL, 0);
mk_bf_ro(cplda, cplda_debug_version, 0x7d, 0, 8, NULL, 0);

/* special case for fan speeds to get rpms */

#define FAN1_SPEED_REG 0x61
#define FAN2_SPEED_REG 0x62
#define FAN3_SPEED_REG 0x63

static int fan_speed_reg[] = {
	FAN1_SPEED_REG,
	FAN2_SPEED_REG,
	FAN3_SPEED_REG,
};

#define NUM_FAN_SPEED_REGS ARRAY_SIZE(fan_speed_reg)

static ssize_t fan_show(struct device *dev, struct device_attribute *dattr,
			char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int idx = attr->index;
	int reg;
	int ret;
	int rpm;

	if (idx < 0 || idx >= NUM_FAN_SPEED_REGS)
		return sprintf(buf, "Invalid fan speed register index: %d\n",
			       idx);

	reg = fan_speed_reg[idx];
	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		pr_err("CPLDA read error - addr: 0x%02X, offset: 0x%02X\n",
		       client->addr, reg);
		return -EINVAL;
	}

	rpm = ret * 50;
	return sprintf(buf, "%d\n", rpm);
}

static SENSOR_DEVICE_ATTR_RO(fan1_input, fan_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan2_input, fan_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan3_input, fan_show, 2);

/* POE packet size */
#define PAYLD_SZ 12
static ssize_t mcu_show(struct device *dev, struct device_attribute *dattr,
			char *buf)
{
	int ret = 0, i, count = 0;
	uint8_t read_buf[PAYLD_SZ];

	ret = i2c_smbus_read_i2c_block_data(poeuc_device, 0x00, PAYLD_SZ, read_buf);
	if (ret < 0) {
	        pr_err("POE controller read error%d\n", ret);
		return ret;
	}
	for(i = 0; i < PAYLD_SZ; i++) {
		/* 6 = 2 bytes (0x) + 2 bytes hex value + 
		 *     1 byte space + 1 null terminator
		 */
		count = snprintf(buf, 6, "0x%02x ", read_buf[i]);
		buf = buf + count;
	}
	return count * PAYLD_SZ;
}

static ssize_t mcu_store(struct device *dev, struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	int ret = 0, i = 0;
	char parse_buf[count + 1];
	char *p = parse_buf, *val = NULL;
	u8 write_buf[count];

	memset(parse_buf, 0, count + 1);
	strncpy(parse_buf, buf, count + 1);
	while((val = strsep(&p, " ")) != NULL) {
		int tmp_val;
		ret = kstrtouint(val, 0, &tmp_val);
		if (ret < 0) {
			pr_err("String to uint conversion failed");
			return ret;
		}
		write_buf[i] = tmp_val;
		i++;
	}
	ret = i2c_smbus_write_i2c_block_data(poeuc_device, write_buf[0], PAYLD_SZ - 1, &write_buf[1]);
	if (ret < 0) {
	        pr_err("POE controller write error %d\n", ret);
		return -EINVAL;
	}
	return count;
}

SYSFS_ATTR_RW(mcu_reg, mcu_show, mcu_store);

/* sysfs registration */

static struct attribute *cplda_attrs[] = {
	&cplda_product_id.attr,
	&cplda_pcb_version.attr,
	&cplda_sfp2_tx_fault.attr,
	&cplda_sfp2_rx_los.attr,
	&cplda_sfp1_tx_fault.attr,
	&cplda_sfp1_rx_los.attr,
	&cplda_sfp4_tx_fault.attr,
	&cplda_sfp4_rx_los.attr,
	&cplda_sfp3_tx_fault.attr,
	&cplda_sfp3_rx_los.attr,
	&cplda_reset_led.attr,
	&cplda_reset_ucd9090.attr,
	&cplda_reset_mac_spi.attr,
	&cplda_reset_poe.attr,
	&cplda_reset_gl850g.attr,
	&cplda_reset_pca9548.attr,
	&cplda_reset_phy1.attr,
	&cplda_reset_bcm5241.attr,
	&cplda_reset_i210.attr,
	&cplda_reset_mac_pcie.attr,
	&cplda_reset_mac.attr,
	&cplda_reset_cpu.attr,
	&cplda_reset_cplda.attr,
	&cplda_reset_button_enable.attr,
	&cplda_sfp4_tx_disable.attr,
	&cplda_sfp3_tx_disable.attr,
	&cplda_sfp2_tx_disable.attr,
	&cplda_sfp1_tx_disable.attr,
	&cplda_sfp4_present.attr,
	&cplda_sfp3_present.attr,
	&cplda_sfp2_present.attr,
	&cplda_sfp1_present.attr,
	&cplda_psu_pwr_all_ok.attr,
	&cplda_poe_enable.attr,
	&cplda_sfp4_enable.attr,
	&cplda_sfp3_enable.attr,
	&cplda_sfp2_enable.attr,
	&cplda_sfp1_enable.attr,
	&cplda_sfp4_pwr_good.attr,
	&cplda_sfp3_pwr_good.attr,
	&cplda_sfp2_pwr_good.attr,
	&cplda_sfp1_pwr_good.attr,
	&cplda_12v_pwr_good.attr,
	&cplda_48v_pwr_good.attr,
	&cplda_vcc1p0_phy_pwr_good.attr,
	&cplda_vcc1p0_mac_pwr_good.attr,
	&cplda_tps51200_pwr_good.attr,
	&cplda_vcc1p5_pwr_good.attr,
	&cplda_vcc3p3_pwr_good.attr,
	&cplda_vcc5p0_pwr_good.attr,
	&cplda_mac_avs0.attr,
	&cplda_poe_present.attr,
	&cplda_usb_pwr_good.attr,
	&cplda_cpu_thermal_trip.attr,
	&cplda_cplda_int_cpu.attr,
	&cplda_usb1_vbus.attr,
	&cplda_enable_leds.attr,
	&cplda_led_psu.attr,
	&cplda_led_system.attr,
	&cplda_led_poe.attr,
	&cplda_led_fan.attr,
	&cplda_power_reset.attr,
	&cplda_cplda_wp.attr,
	&cplda_fan_pwm.attr,
	&cplda_fan1_ok.attr,
	&cplda_fan2_ok.attr,
	&cplda_fan3_ok.attr,
	&cplda_cplda_version.attr,
	&cplda_cplda_debug_version.attr,

	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,

	NULL,
};

static struct attribute_group cplda_attr_group = {
	.attrs = cplda_attrs,
};

static struct attribute *poeuc_attrs[] = {
	&dev_attr_mcu_reg.attr,
	NULL,
};

/* POE controller attributes */
static struct attribute_group poeuc_attr_group = {
	.attrs = poeuc_attrs,
};

/* CPLDA initialization */

static int cplda_probe(struct i2c_client *client,
		      const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	u8 version;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev,
			"adapter does not support I2C_FUNC_SMBUS_BYTE_DATA\n");
		ret = -EINVAL;
		goto err;
	}

	/*
	 * Probe the hardware by reading the revision number.
	 */
	ret = i2c_smbus_read_byte_data(client, 0x7c);
	if (ret < 0) {
		dev_err(&client->dev,
			"read CPLDA version register error %d\n", ret);
		goto err;
	}
	version = ret;

	/*
	 * Create sysfs nodes.
	 */
	ret = sysfs_create_group(&client->dev.kobj, &cplda_attr_group);
	if (ret) {
		dev_err(&client->dev, "sysfs_create_group failed\n");
		goto err;
	}

	/*
	 * All clear from this point on
	 */
	dev_info(&client->dev,
		 "device created, CPLDA version %d\n",
		 version);

	return 0;

err:
	return ret;
}

static int cplda_remove(struct i2c_client *client)
{
	sysfs_remove_group(&client->dev.kobj, &cplda_attr_group);
	dev_info(&client->dev, "device removed\n");
	return 0;
}

static const struct i2c_device_id cplda_id[] = {
	{ "cplda", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, cplda_id);

static struct i2c_driver cplda_driver = {
	.driver = {
		.name = "cplda",
		.owner = THIS_MODULE,
	},
	.probe = cplda_probe,
	.remove = cplda_remove,
	.id_table = cplda_id,
};

static int controller_probe(struct platform_device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->dev.kobj, &poeuc_attr_group);
	if (ret)
		pr_err("Failed to create sysfs group for POE controller driver\n");

	return ret;
}

static int controller_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver poe_uc_driver = {
	.driver = {
		.name = "stm_poe_controller",
		.owner = THIS_MODULE,
	},
	.probe = controller_probe,
	.remove = controller_remove,
};

static struct platform_device *controller_device;

static int cplda_init(void)
{
	pr_info(DRIVER_NAME ": loading CPLDA driver\n");
	return i2c_add_driver(&cplda_driver);
}

static void cplda_exit(void)
{
	i2c_del_driver(&cplda_driver);
	pr_info(DRIVER_NAME ": CPLDA driver unloaded\n");
}

static int controller_init(void)
{
	int ret;

	ret = platform_driver_register(&poe_uc_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for POE controller device\n");
		return ret;
	}

	controller_device = platform_device_alloc("stm_poe_controller", 0);
	if (!controller_device) {
		pr_err("platform_device_alloc() failed for poe MCU device\n");
		platform_driver_unregister(&poe_uc_driver);
		return -ENOMEM;
	}

	ret = platform_device_add(controller_device);
	if (ret) {
		pr_err("platform_device_add() failed for poe MCU device\n");
		return ret;
	}

	pr_info("POE MCU driver loaded\n");
	return 0;
}

static void controller_exit(void)
{
	platform_driver_unregister(&poe_uc_driver);
	platform_device_unregister(controller_device);
	pr_err("POE MCU driver unloaded\n");
}
/* Module init and exit */

static int __init act_4222_init(void)
{
	int ret;

	ret = i2c_init();
	if (ret) {
		pr_err("I2C initialization failed\n");
		i2c_exit();
		return ret;
	}

	ret = cplda_init();
	if (ret) {
		pr_err("CPLDA initialization failed\n");
		cplda_exit();
		i2c_exit();
		return ret;
	}
	ret = controller_init();
	if (ret) {
		pr_err("Controller initialization failed\n");
		controller_exit();
		cplda_exit();
		i2c_exit();
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit act_4222_exit(void)
{
	controller_exit();
	cplda_exit();
	i2c_exit();
	pr_info(DRIVER_NAME " driver unloaded\n");
}

module_init(act_4222_init);
module_exit(act_4222_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton AS4222-28PE Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
