/*
 * dellemc_z9264_platform.c - Dell EMC Z9264-C3538 Platform Support.
 *
 * Copyright (c) 2018 Cumulus Networks, Inc.  All Rights Reserved.
 * Author: Nikhil Dhar (ndhar@cumulusnetworks.com)
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
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/i2c-ismt.h>
#include <linux/i2c-mux.h>
#include <linux/interrupt.h>
#include <linux/stddef.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/i2c-ocores.h>
#include <linux/platform_data/i2c-mux-gpio.h>
#include <linux/pci.h>

#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "platform-bitfield.h"
#include "dellemc-z9264f-platform.h"

#define DRIVER_NAME		  "dellemc_z9264f_platform"
#define DRIVER_VERSION		  "1.0"

#define ISMT_ADAPTER_NAME	  "SMBus iSMT adapter"
#define I801_ADAPTER_NAME	  "SMBus I801 adapter"
#define PCI_DEVICE_ID_XILINX_FPGA 0x7021
#define FPGA_BAR 0
#define FPGA_DESC_ENTRIES	  2

static struct resource ctrl_resource;
static struct resource ocores_resources[FPGA_I2C_BUS_MAX];
static struct ocores_i2c_platform_data ocores_i2c_data[FPGA_I2C_BUS_MAX];
struct ocores_i2c_device_info ocores_i2c_device_infotab[FPGA_I2C_BUS_MAX];

static const struct pci_device_id fpga_id[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_XILINX, PCI_DEVICE_ID_XILINX_FPGA) },
	{ 0, }
};

/*
 * This platform has two i2c busses:
 *   SMBus_0: SMBus iSMT adapter at dff9f000
 *   SMBus_1: SMBus I801 adapter at e000
 *
 * SMBus_0 has only one interesting device:
 *   board eeprom (0x50)
 *
 * This platform also has a large FPGA on the PCIe bus.	 It is used to
 * access the system LEDs, SFF low speed signals, SFF I2C busses, and
 * miscellaneous control and status registers.
 *
 */
enum {
	I2C_ISMT_BUS = -2,
	I2C_I801_BUS,
	FPGA_I2C_BUS1 = 1,
	FPGA_I2C_BUS2,
	FPGA_I2C_BUS3,
	FPGA_I2C_BUS4,
	FPGA_I2C_BUS5,
	FPGA_I2C_BUS6,
	FPGA_I2C_BUS7,
	FPGA_I2C_BUS8,
	FPGA_I2C_BUS9,
	FPGA_I2C_BUS10,
	FPGA_I2C_BUS11,
	FPGA_I2C_BUS4_0 = 20,
	FPGA_I2C_BUS4_1,
	FPGA_I2C_BUS4_2,
	FPGA_I2C_BUS4_3,
	FPGA_I2C_BUS4_4,
	FPGA_I2C_BUS4_5,
	FPGA_I2C_BUS4_6,
	FPGA_I2C_BUS4_7,
	FPGA_I2C_BUS5_0,
	FPGA_I2C_BUS5_1,
	FPGA_I2C_BUS5_2,
	FPGA_I2C_BUS5_3,
	FPGA_I2C_BUS5_4,
	FPGA_I2C_BUS5_5,
	FPGA_I2C_BUS5_6,
	FPGA_I2C_BUS5_7,
	FPGA_I2C_BUS6_0,
	FPGA_I2C_BUS6_1,
	FPGA_I2C_BUS6_2,
	FPGA_I2C_BUS6_3,
	FPGA_I2C_BUS6_4,
	FPGA_I2C_BUS6_5,
	FPGA_I2C_BUS6_6,
	FPGA_I2C_BUS6_7,
	FPGA_I2C_BUS7_0,
	FPGA_I2C_BUS7_1,
	FPGA_I2C_BUS7_2,
	FPGA_I2C_BUS7_3,
	FPGA_I2C_BUS7_4,
	FPGA_I2C_BUS7_5,
	FPGA_I2C_BUS7_6,
	FPGA_I2C_BUS7_7,
	FPGA_I2C_BUS8_0,
	FPGA_I2C_BUS8_1,
	FPGA_I2C_BUS8_2,
	FPGA_I2C_BUS8_3,
	FPGA_I2C_BUS8_4,
	FPGA_I2C_BUS8_5,
	FPGA_I2C_BUS8_6,
	FPGA_I2C_BUS8_7,
	FPGA_I2C_BUS9_0,
	FPGA_I2C_BUS9_1,
	FPGA_I2C_BUS9_2,
	FPGA_I2C_BUS9_3,
	FPGA_I2C_BUS9_4,
	FPGA_I2C_BUS9_5,
	FPGA_I2C_BUS9_6,
	FPGA_I2C_BUS9_7,
	FPGA_I2C_BUS10_0,
	FPGA_I2C_BUS10_1,
	FPGA_I2C_BUS10_2,
	FPGA_I2C_BUS10_3,
	FPGA_I2C_BUS10_4,
	FPGA_I2C_BUS10_5,
	FPGA_I2C_BUS10_6,
	FPGA_I2C_BUS10_7,
	FPGA_I2C_BUS11_0,
	FPGA_I2C_BUS11_1,
	FPGA_I2C_BUS11_2,
	FPGA_I2C_BUS11_3,
	FPGA_I2C_BUS11_4,
	FPGA_I2C_BUS11_5,
	FPGA_I2C_BUS11_6,
	FPGA_I2C_BUS11_7,
};

/* Accessor functions for reading and writing the FPGA registers */
static int fpga_read_reg(struct device *dev,
			 int reg,
			 int nregs,
			 u32 *val)
{
	struct fpga_priv *priv = dev_get_drvdata(dev);

	*val = readl(priv->pbar + reg);
	return 0;
}

static int fpga_write_reg(struct device *dev,
			  int reg,
			  int nregs,
			  u32 val)
{
	struct fpga_priv *priv = dev_get_drvdata(dev);

	writel(val, priv->pbar + reg);
	return 0;
}

/* string arrays to map values into names */
static const char * const led_fan_values[] = {
	"off",
	"amber",
	"green",
	"amber_blinking",
};

static const char * const led_system_values[] = {
	"green_blinking",
	"green",
	"amber",
	"amber_blinking",
};

static const char * const led_beacon_values[] = {
	"off",
	"blue_blinking",
};

static const char * const led_power_values[] = {
	"off",
	"amber",
	"green",
	"amber_blinking",
};

static const char * const led_stack_values[] = {
	"green",
	"off",
};

static const char * const led_blink_values[] = {
	"green",
	"green_blinking",
};

static const char * const led_off_values[] = {
	"off",
	"on",
};

static const char * const led_dot_values[] = {
	"off",
	"on",
};

static const char * const led_digit_values[] = {
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
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
};

mk_pca9548(mux1, FPGA_I2C_BUS4_0, 1);
mk_pca9548(mux2, FPGA_I2C_BUS5_0, 1);
mk_pca9548(mux3, FPGA_I2C_BUS6_0, 1);
mk_pca9548(mux4, FPGA_I2C_BUS7_0, 1);
mk_pca9548(mux5, FPGA_I2C_BUS8_0, 1);
mk_pca9548(mux6, FPGA_I2C_BUS9_0, 1);
mk_pca9548(mux7, FPGA_I2C_BUS10_0, 1);
mk_pca9548(mux8, FPGA_I2C_BUS11_0, 1);

/* Board eeprom device */
mk_eeprom(board, 50, 256, AT24_FLAG_IRUGO);
/* QSFP eeprom devices*/

mk_qsfp_port_eeprom(port1, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port2, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port3, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port4, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port5, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port6, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port7, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port8, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port9, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port10, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port11, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port12, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port13, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port14, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port15, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port16, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port17, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port18, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port19, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port20, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port21, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port22, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port23, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port24, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port25, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port26, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port27, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port28, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port29, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port30, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port31, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port32, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port33, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port34, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port35, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port36, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port37, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port38, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port39, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port40, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port41, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port42, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port43, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port44, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port45, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port46, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port47, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port48, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port49, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port50, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port51, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port52, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port53, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port54, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port55, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port56, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port57, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port58, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port59, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port60, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port61, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port62, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port63, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port64, 50, 256, SFF_8436_FLAG_IRUGO);

/* Define all the bitfields */
mk_bf_ro(fpga, major_rev,	 0x0000, 8,  8, NULL, 0);
mk_bf_ro(fpga, minor_rev,	 0x0000, 0,  8, NULL, 0);
mk_bf_rw(fpga, scratch,		 0x0004, 0, 32, NULL, 0);
mk_bf_ro(fpga, com_e_type,	 0x0008, 8,  3, NULL, 0);
mk_bf_ro(fpga, board_rev,	 0x0008, 4,  4, NULL, 0);
mk_bf_ro(fpga, board_type,	 0x0008, 0,  4, NULL, 0);
mk_bf_ro(fpga, timestamp1,	 0x000c, 0, 32, NULL, 0);
mk_bf_ro(fpga, timestamp2,	 0x0010, 0, 32, NULL, 0);
mk_bf_ro(fpga, led_fan,		 0x0024, 6,  2, led_fan_values, 0);
mk_bf_rw(fpga, led_system,	 0x0024, 4,  2, led_system_values, 0);
mk_bf_rw(fpga, led_beacon,	 0x0024, 3,  1, led_beacon_values, 0);
mk_bf_rw(fpga, led_power,	 0x0024, 1,  2, led_power_values, 0);
mk_bf_ro(fpga, led_stack,	 0x0024, 0,  1, led_stack_values, 0);
mk_bf_rw(fpga, led_blink,	 0x0024, 6,  1, led_blink_values, 0);
mk_bf_rw(fpga, led_off,		 0x0028, 5,  1, led_off_values, 0);
mk_bf_rw(fpga, led_dot,		 0x0028, 4,  1, led_dot_values, 0);
mk_bf_rw(fpga, led_digit,	 0x0028, 0,  4, led_digit_values, 0);
mk_bf_ro(fpga, psu_pwr2_all_ok,	 0x0038, 3,  1, NULL, 0);
mk_bf_ro(fpga, psu_pwr2_present, 0x0038, 2,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, psu_pwr1_all_ok,	 0x0038, 1,  1, NULL, 0);
mk_bf_ro(fpga, psu_pwr1_present, 0x0038, 0,  1, NULL, BF_COMPLEMENT);
/* plumb all of the front panel port signals */
mk_bf_rw(fpga, port1_lpmode,	 0x4000, 6,  1, NULL, 0);
mk_bf_rw(fpga, port1_modsel,	 0x4000, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port1_reset,	 0x4000, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port1_present,	 0x4004, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port2_lpmode,	 0x4010, 6,  1, NULL, 0);
mk_bf_rw(fpga, port2_modsel,	 0x4010, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port2_reset,	 0x4010, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port2_present,	 0x4014, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port3_lpmode,	 0x4020, 6,  1, NULL, 0);
mk_bf_rw(fpga, port3_modsel,	 0x4020, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port3_reset,	 0x4020, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port3_present,	 0x4024, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port4_lpmode,	 0x4030, 6,  1, NULL, 0);
mk_bf_rw(fpga, port4_modsel,	 0x4030, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port4_reset,	 0x4030, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port4_present,	 0x4034, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port5_lpmode,	 0x4040, 6,  1, NULL, 0);
mk_bf_rw(fpga, port5_modsel,	 0x4040, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port5_reset,	 0x4040, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port5_present,	 0x4044, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port6_lpmode,	 0x4050, 6,  1, NULL, 0);
mk_bf_rw(fpga, port6_modsel,	 0x4050, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port6_reset,	 0x4050, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port6_present,	 0x4054, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port7_lpmode,	 0x4060, 6,  1, NULL, 0);
mk_bf_rw(fpga, port7_modsel,	 0x4060, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port7_reset,	 0x4060, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port7_present,	 0x4064, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port8_lpmode,	 0x4070, 6,  1, NULL, 0);
mk_bf_rw(fpga, port8_modsel,	 0x4070, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port8_reset,	 0x4070, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port8_present,	 0x4074, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port9_lpmode,	 0x4080, 6,  1, NULL, 0);
mk_bf_rw(fpga, port9_modsel,	 0x4080, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port9_reset,	 0x4080, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port9_present,	 0x4084, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port10_lpmode,    0x4090, 6,  1, NULL, 0);
mk_bf_rw(fpga, port10_modsel,	 0x4090, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port10_reset,	 0x4090, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port10_present,	 0x4094, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port11_lpmode,    0x40a0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port11_modsel,	 0x40a0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port11_reset,	 0x40a0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port11_present,	 0x40a4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port12_lpmode,    0x40b0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port12_modsel,	 0x40b0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port12_reset,	 0x40b0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port12_present,	 0x40b4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port13_lpmode,    0x40c0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port13_modsel,	 0x40c0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port13_reset,	 0x40c0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port13_present,	 0x40c4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port14_lpmode,    0x40d0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port14_modsel,	 0x40d0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port14_reset,	 0x40d0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port14_present,	 0x40d4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port15_lpmode,    0x40e0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port15_modsel,	 0x40e0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port15_reset,	 0x40e0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port15_present,	 0x40e4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port16_lpmode,    0x40f0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port16_modsel,	 0x40f0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port16_reset,	 0x40f0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port16_present,	 0x40f4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port17_lpmode,    0x4100, 6,  1, NULL, 0);
mk_bf_rw(fpga, port17_modsel,	 0x4100, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port17_reset,	 0x4100, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port17_present,	 0x4104, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port18_lpmode,    0x4110, 6,  1, NULL, 0);
mk_bf_rw(fpga, port18_modsel,	 0x4110, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port18_reset,	 0x4110, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port18_present,	 0x4114, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port19_lpmode,    0x4120, 6,  1, NULL, 0);
mk_bf_rw(fpga, port19_modsel,	 0x4120, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port19_reset,	 0x4120, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port19_present,	 0x4124, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port20_lpmode,    0x4130, 6,  1, NULL, 0);
mk_bf_rw(fpga, port20_modsel,	 0x4130, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port20_reset,	 0x4130, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port20_present,	 0x4134, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port21_lpmode,    0x4140, 6,  1, NULL, 0);
mk_bf_rw(fpga, port21_modsel,	 0x4140, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port21_reset,	 0x4140, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port21_present,	 0x4144, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port22_lpmode,    0x4150, 6,  1, NULL, 0);
mk_bf_rw(fpga, port22_modsel,	 0x4150, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port22_reset,	 0x4150, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port22_present,	 0x4154, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port23_lpmode,    0x4160, 6,  1, NULL, 0);
mk_bf_rw(fpga, port23_modsel,	 0x4160, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port23_reset,	 0x4160, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port23_present,	 0x4164, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port24_lpmode,    0x4170, 6,  1, NULL, 0);
mk_bf_rw(fpga, port24_modsel,	 0x4170, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port24_reset,	 0x4170, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port24_present,	 0x4174, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port25_lpmode,    0x4180, 6,  1, NULL, 0);
mk_bf_rw(fpga, port25_modsel,	 0x4180, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port25_reset,	 0x4180, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port25_present,	 0x4184, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port26_lpmode,    0x4190, 6,  1, NULL, 0);
mk_bf_rw(fpga, port26_modsel,	 0x4190, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port26_reset,	 0x4190, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port26_present,	 0x4194, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port27_lpmode,    0x41a0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port27_modsel,	 0x41a0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port27_reset,	 0x41a0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port27_present,	 0x41a4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port28_lpmode,    0x41b0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port28_modsel,	 0x41b0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port28_reset,	 0x41b0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port28_present,	 0x41b4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port29_lpmode,    0x41c0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port29_modsel,	 0x41c0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port29_reset,	 0x41c0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port29_present,	 0x41c4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port30_lpmode,    0x41d0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port30_modsel,	 0x41d0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port30_reset,	 0x41d0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port30_present,	 0x41d4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port31_lpmode,    0x41e0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port31_modsel,	 0x41e0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port31_reset,	 0x41e0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port31_present,	 0x41e4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port32_lpmode,    0x41f0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port32_modsel,	 0x41f0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port32_reset,	 0x41f0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port32_present,	 0x41f4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port33_lpmode,    0x4200, 6,  1, NULL, 0);
mk_bf_rw(fpga, port33_modsel,	 0x4200, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port33_reset,	 0x4200, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port33_present,	 0x4204, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port34_lpmode,	 0x4210, 6,  1, NULL, 0);
mk_bf_rw(fpga, port34_modsel,	 0x4210, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port34_reset,	 0x4210, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port34_present,	 0x4214, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port35_lpmode,	 0x4220, 6,  1, NULL, 0);
mk_bf_rw(fpga, port35_modsel,	 0x4220, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port35_reset,	 0x4220, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port35_present,	 0x4224, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port36_lpmode,	 0x4230, 6,  1, NULL, 0);
mk_bf_rw(fpga, port36_modsel,	 0x4230, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port36_reset,	 0x4230, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port36_present,	 0x4234, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port37_lpmode,	 0x4240, 6,  1, NULL, 0);
mk_bf_rw(fpga, port37_modsel,	 0x4240, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port37_reset,	 0x4240, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port37_present,	 0x4244, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port38_lpmode,	 0x4250, 6,  1, NULL, 0);
mk_bf_rw(fpga, port38_modsel,	 0x4250, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port38_reset,	 0x4250, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port38_present,	 0x4254, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port39_lpmode,	 0x4260, 6,  1, NULL, 0);
mk_bf_rw(fpga, port39_modsel,	 0x4260, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port39_reset,	 0x4260, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port39_present,	 0x4264, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port40_lpmode,	 0x4270, 6,  1, NULL, 0);
mk_bf_rw(fpga, port40_modsel,	 0x4270, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port40_reset,	 0x4270, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port40_present,	 0x4274, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port41_lpmode,	 0x4280, 6,  1, NULL, 0);
mk_bf_rw(fpga, port41_modsel,	 0x4280, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port41_reset,	 0x4280, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port41_present,	 0x4284, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port42_lpmode,    0x4290, 6,  1, NULL, 0);
mk_bf_rw(fpga, port42_modsel,	 0x4290, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port42_reset,	 0x4290, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port42_present,	 0x4294, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port43_lpmode,    0x42a0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port43_modsel,	 0x42a0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port43_reset,	 0x42a0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port43_present,	 0x42a4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port44_lpmode,    0x42b0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port44_modsel,	 0x42b0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port44_reset,	 0x42b0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port44_present,	 0x42b4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port45_lpmode,    0x42c0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port45_modsel,	 0x42c0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port45_reset,	 0x42c0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port45_present,	 0x42c4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port46_lpmode,    0x42d0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port46_modsel,	 0x42d0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port46_reset,	 0x42d0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port46_present,	 0x42d4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port47_lpmode,    0x42e0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port47_modsel,	 0x42e0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port47_reset,	 0x42e0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port47_present,	 0x42e4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port48_lpmode,    0x42f0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port48_modsel,	 0x42f0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port48_reset,	 0x42f0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port48_present,	 0x42f4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port49_lpmode,    0x4300, 6,  1, NULL, 0);
mk_bf_rw(fpga, port49_modsel,	 0x4300, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port49_reset,	 0x4300, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port49_present,	 0x4304, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port50_lpmode,    0x4310, 6,  1, NULL, 0);
mk_bf_rw(fpga, port50_modsel,	 0x4310, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port50_reset,	 0x4310, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port50_present,	 0x4314, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port51_lpmode,    0x4320, 6,  1, NULL, 0);
mk_bf_rw(fpga, port51_modsel,	 0x4320, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port51_reset,	 0x4320, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port51_present,	 0x4324, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port52_lpmode,    0x4330, 6,  1, NULL, 0);
mk_bf_rw(fpga, port52_modsel,	 0x4330, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port52_reset,	 0x4330, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port52_present,	 0x4334, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port53_lpmode,    0x4340, 6,  1, NULL, 0);
mk_bf_rw(fpga, port53_modsel,	 0x4340, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port53_reset,	 0x4340, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port53_present,	 0x4344, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port54_lpmode,    0x4350, 6,  1, NULL, 0);
mk_bf_rw(fpga, port54_modsel,	 0x4350, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port54_reset,	 0x4350, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port54_present,	 0x4354, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port55_lpmode,    0x4360, 6,  1, NULL, 0);
mk_bf_rw(fpga, port55_modsel,	 0x4360, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port55_reset,	 0x4360, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port55_present,	 0x4364, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port56_lpmode,    0x4370, 6,  1, NULL, 0);
mk_bf_rw(fpga, port56_modsel,	 0x4370, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port56_reset,	 0x4370, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port56_present,	 0x4374, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port57_lpmode,    0x4380, 6,  1, NULL, 0);
mk_bf_rw(fpga, port57_modsel,	 0x4380, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port57_reset,	 0x4380, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port57_present,	 0x4384, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port58_lpmode,    0x4390, 6,  1, NULL, 0);
mk_bf_rw(fpga, port58_modsel,	 0x4390, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port58_reset,	 0x4390, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port58_present,	 0x4394, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port59_lpmode,    0x43a0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port59_modsel,	 0x43a0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port59_reset,	 0x43a0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port59_present,	 0x43a4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port60_lpmode,    0x43b0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port60_modsel,	 0x43b0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port60_reset,	 0x43b0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port60_present,	 0x43b4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port61_lpmode,    0x43c0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port61_modsel,	 0x43c0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port61_reset,	 0x43c0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port61_present,	 0x43c4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port62_lpmode,    0x43d0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port62_modsel,	 0x43d0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port62_reset,	 0x43d0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port62_present,	 0x43d4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port63_lpmode,    0x43e0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port63_modsel,	 0x43e0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port63_reset,	 0x43e0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port63_present,	 0x43e4, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port64_lpmode,    0x43f0, 6,  1, NULL, 0);
mk_bf_rw(fpga, port64_modsel,	 0x43f0, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port64_reset,	 0x43f0, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port64_present,	 0x43f4, 4,  1, NULL, BF_COMPLEMENT);

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_ISMT_BUS, "24c02", 0x50, &board_50_at24),
};

struct platform_i2c_device_info ocores_i2c_devices[] = {
	/* FPGA bus 4 eeproms */
	mk_i2cdev(FPGA_I2C_BUS4,  "pca9548",  0x74,  &mux1_platform_data),
	mk_i2cdev(FPGA_I2C_BUS4_0, "sff8436", 0x50, &port1_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS4_1, "sff8436", 0x50, &port2_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS4_2, "sff8436", 0x50, &port3_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS4_3, "sff8436", 0x50, &port4_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS4_4, "sff8436", 0x50, &port5_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS4_5, "sff8436", 0x50, &port6_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS4_6, "sff8436", 0x50, &port7_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS4_7, "sff8436", 0x50, &port8_50_sff8436),
	/* FPGA bus 5 eeproms */
	mk_i2cdev(FPGA_I2C_BUS5,  "pca9548",  0x74,  &mux2_platform_data),
	mk_i2cdev(FPGA_I2C_BUS5_0, "sff8436", 0x50, &port9_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS5_1, "sff8436", 0x50, &port10_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS5_2, "sff8436", 0x50, &port11_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS5_3, "sff8436", 0x50, &port12_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS5_4, "sff8436", 0x50, &port13_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS5_5, "sff8436", 0x50, &port14_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS5_6, "sff8436", 0x50, &port15_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS5_7, "sff8436", 0x50, &port16_50_sff8436),
	/* FPGA bus 6 eeproms */
	mk_i2cdev(FPGA_I2C_BUS6,  "pca9548",  0x74,  &mux3_platform_data),
	mk_i2cdev(FPGA_I2C_BUS6_0, "sff8436", 0x50, &port17_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS6_1, "sff8436", 0x50, &port18_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS6_2, "sff8436", 0x50, &port19_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS6_3, "sff8436", 0x50, &port20_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS6_4, "sff8436", 0x50, &port21_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS6_5, "sff8436", 0x50, &port22_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS6_6, "sff8436", 0x50, &port23_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS6_7, "sff8436", 0x50, &port24_50_sff8436),
	/* FPGA bus 7 eeproms */
	mk_i2cdev(FPGA_I2C_BUS7,  "pca9548",  0x74,  &mux4_platform_data),
	mk_i2cdev(FPGA_I2C_BUS7_0, "sff8436", 0x50, &port25_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS7_1, "sff8436", 0x50, &port26_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS7_2, "sff8436", 0x50, &port27_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS7_3, "sff8436", 0x50, &port28_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS7_4, "sff8436", 0x50, &port29_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS7_5, "sff8436", 0x50, &port30_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS7_6, "sff8436", 0x50, &port31_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS7_7, "sff8436", 0x50, &port32_50_sff8436),
	/* FPGA bus 8 eeproms */
	mk_i2cdev(FPGA_I2C_BUS8,  "pca9548",  0x74,  &mux5_platform_data),
	mk_i2cdev(FPGA_I2C_BUS8_0, "sff8436", 0x50, &port33_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS8_1, "sff8436", 0x50, &port34_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS8_2, "sff8436", 0x50, &port35_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS8_3, "sff8436", 0x50, &port36_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS8_4, "sff8436", 0x50, &port37_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS8_5, "sff8436", 0x50, &port38_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS8_6, "sff8436", 0x50, &port39_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS8_7, "sff8436", 0x50, &port40_50_sff8436),
	/* FPGA bus 9 eeproms */
	mk_i2cdev(FPGA_I2C_BUS9,  "pca9548",  0x74,  &mux6_platform_data),
	mk_i2cdev(FPGA_I2C_BUS9_0, "sff8436", 0x50, &port41_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS9_1, "sff8436", 0x50, &port42_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS9_2, "sff8436", 0x50, &port43_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS9_3, "sff8436", 0x50, &port44_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS9_4, "sff8436", 0x50, &port45_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS9_5, "sff8436", 0x50, &port46_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS9_6, "sff8436", 0x50, &port47_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS9_7, "sff8436", 0x50, &port48_50_sff8436),
	/* FPGA bus 10 eeproms */
	mk_i2cdev(FPGA_I2C_BUS10, "pca9548",  0x74,  &mux7_platform_data),
	mk_i2cdev(FPGA_I2C_BUS10_0, "sff8436", 0x50, &port49_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS10_1, "sff8436", 0x50, &port50_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS10_2, "sff8436", 0x50, &port51_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS10_3, "sff8436", 0x50, &port52_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS10_4, "sff8436", 0x50, &port53_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS10_5, "sff8436", 0x50, &port54_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS10_6, "sff8436", 0x50, &port55_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS10_7, "sff8436", 0x50, &port56_50_sff8436),
	/* FPGA bus 11 eeproms */
	mk_i2cdev(FPGA_I2C_BUS11, "pca9548",  0x74,  &mux8_platform_data),
	mk_i2cdev(FPGA_I2C_BUS11_0, "sff8436", 0x50, &port57_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS11_1, "sff8436", 0x50, &port58_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS11_2, "sff8436", 0x50, &port59_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS11_3, "sff8436", 0x50, &port60_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS11_4, "sff8436", 0x50, &port61_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS11_5, "sff8436", 0x50, &port62_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS11_6, "sff8436", 0x50, &port63_50_sff8436),
	mk_i2cdev(FPGA_I2C_BUS11_7, "sff8436", 0x50, &port64_50_sff8436),
};

static struct attribute *fpga_attrs[] = {
	&fpga_major_rev.attr,
	&fpga_minor_rev.attr,
	&fpga_scratch.attr,
	&fpga_com_e_type.attr,
	&fpga_board_rev.attr,
	&fpga_board_type.attr,
	&fpga_timestamp1.attr,
	&fpga_timestamp2.attr,
	&fpga_led_fan.attr,
	&fpga_led_system.attr,
	&fpga_led_beacon.attr,
	&fpga_led_power.attr,
	&fpga_led_stack.attr,
	&fpga_led_blink.attr,
	&fpga_led_off.attr,
	&fpga_led_dot.attr,
	&fpga_led_digit.attr,
	&fpga_psu_pwr2_all_ok.attr,
	&fpga_psu_pwr2_present.attr,
	&fpga_psu_pwr1_all_ok.attr,
	&fpga_psu_pwr1_present.attr,
	&fpga_port1_lpmode.attr,
	&fpga_port1_modsel.attr,
	&fpga_port1_reset.attr,
	&fpga_port1_present.attr,
	&fpga_port2_lpmode.attr,
	&fpga_port2_modsel.attr,
	&fpga_port2_reset.attr,
	&fpga_port2_present.attr,
	&fpga_port3_lpmode.attr,
	&fpga_port3_modsel.attr,
	&fpga_port3_reset.attr,
	&fpga_port3_present.attr,
	&fpga_port4_lpmode.attr,
	&fpga_port4_modsel.attr,
	&fpga_port4_reset.attr,
	&fpga_port4_present.attr,
	&fpga_port5_lpmode.attr,
	&fpga_port5_modsel.attr,
	&fpga_port5_reset.attr,
	&fpga_port5_present.attr,
	&fpga_port6_lpmode.attr,
	&fpga_port6_modsel.attr,
	&fpga_port6_reset.attr,
	&fpga_port6_present.attr,
	&fpga_port7_lpmode.attr,
	&fpga_port7_modsel.attr,
	&fpga_port7_reset.attr,
	&fpga_port7_present.attr,
	&fpga_port8_lpmode.attr,
	&fpga_port8_modsel.attr,
	&fpga_port8_reset.attr,
	&fpga_port8_present.attr,
	&fpga_port9_lpmode.attr,
	&fpga_port9_modsel.attr,
	&fpga_port9_reset.attr,
	&fpga_port9_present.attr,
	&fpga_port10_lpmode.attr,
	&fpga_port10_modsel.attr,
	&fpga_port10_reset.attr,
	&fpga_port10_present.attr,
	&fpga_port11_lpmode.attr,
	&fpga_port11_modsel.attr,
	&fpga_port11_reset.attr,
	&fpga_port11_present.attr,
	&fpga_port12_lpmode.attr,
	&fpga_port12_modsel.attr,
	&fpga_port12_reset.attr,
	&fpga_port12_present.attr,
	&fpga_port13_lpmode.attr,
	&fpga_port13_modsel.attr,
	&fpga_port13_reset.attr,
	&fpga_port13_present.attr,
	&fpga_port14_lpmode.attr,
	&fpga_port14_modsel.attr,
	&fpga_port14_reset.attr,
	&fpga_port14_present.attr,
	&fpga_port15_lpmode.attr,
	&fpga_port15_modsel.attr,
	&fpga_port15_reset.attr,
	&fpga_port15_present.attr,
	&fpga_port16_lpmode.attr,
	&fpga_port16_modsel.attr,
	&fpga_port16_reset.attr,
	&fpga_port16_present.attr,
	&fpga_port17_lpmode.attr,
	&fpga_port17_modsel.attr,
	&fpga_port17_reset.attr,
	&fpga_port17_present.attr,
	&fpga_port18_lpmode.attr,
	&fpga_port18_modsel.attr,
	&fpga_port18_reset.attr,
	&fpga_port18_present.attr,
	&fpga_port19_lpmode.attr,
	&fpga_port19_modsel.attr,
	&fpga_port19_reset.attr,
	&fpga_port19_present.attr,
	&fpga_port20_lpmode.attr,
	&fpga_port20_modsel.attr,
	&fpga_port20_reset.attr,
	&fpga_port20_present.attr,
	&fpga_port21_lpmode.attr,
	&fpga_port21_modsel.attr,
	&fpga_port21_reset.attr,
	&fpga_port21_present.attr,
	&fpga_port22_lpmode.attr,
	&fpga_port22_modsel.attr,
	&fpga_port22_reset.attr,
	&fpga_port22_present.attr,
	&fpga_port23_lpmode.attr,
	&fpga_port23_modsel.attr,
	&fpga_port23_reset.attr,
	&fpga_port23_present.attr,
	&fpga_port24_lpmode.attr,
	&fpga_port24_modsel.attr,
	&fpga_port24_reset.attr,
	&fpga_port24_present.attr,
	&fpga_port25_lpmode.attr,
	&fpga_port25_modsel.attr,
	&fpga_port25_reset.attr,
	&fpga_port25_present.attr,
	&fpga_port26_lpmode.attr,
	&fpga_port26_modsel.attr,
	&fpga_port26_reset.attr,
	&fpga_port26_present.attr,
	&fpga_port27_lpmode.attr,
	&fpga_port27_modsel.attr,
	&fpga_port27_reset.attr,
	&fpga_port27_present.attr,
	&fpga_port28_lpmode.attr,
	&fpga_port28_modsel.attr,
	&fpga_port28_reset.attr,
	&fpga_port28_present.attr,
	&fpga_port29_lpmode.attr,
	&fpga_port29_modsel.attr,
	&fpga_port29_reset.attr,
	&fpga_port29_present.attr,
	&fpga_port30_lpmode.attr,
	&fpga_port30_modsel.attr,
	&fpga_port30_reset.attr,
	&fpga_port30_present.attr,
	&fpga_port31_lpmode.attr,
	&fpga_port31_modsel.attr,
	&fpga_port31_reset.attr,
	&fpga_port31_present.attr,
	&fpga_port32_lpmode.attr,
	&fpga_port32_modsel.attr,
	&fpga_port32_reset.attr,
	&fpga_port32_present.attr,
	&fpga_port33_lpmode.attr,
	&fpga_port33_modsel.attr,
	&fpga_port33_reset.attr,
	&fpga_port33_present.attr,
	&fpga_port34_lpmode.attr,
	&fpga_port34_modsel.attr,
	&fpga_port34_reset.attr,
	&fpga_port34_present.attr,
	&fpga_port35_lpmode.attr,
	&fpga_port35_modsel.attr,
	&fpga_port35_reset.attr,
	&fpga_port35_present.attr,
	&fpga_port36_lpmode.attr,
	&fpga_port36_modsel.attr,
	&fpga_port36_reset.attr,
	&fpga_port36_present.attr,
	&fpga_port37_lpmode.attr,
	&fpga_port37_modsel.attr,
	&fpga_port37_reset.attr,
	&fpga_port37_present.attr,
	&fpga_port38_lpmode.attr,
	&fpga_port38_modsel.attr,
	&fpga_port38_reset.attr,
	&fpga_port38_present.attr,
	&fpga_port39_lpmode.attr,
	&fpga_port39_modsel.attr,
	&fpga_port39_reset.attr,
	&fpga_port39_present.attr,
	&fpga_port40_lpmode.attr,
	&fpga_port40_modsel.attr,
	&fpga_port40_reset.attr,
	&fpga_port40_present.attr,
	&fpga_port41_lpmode.attr,
	&fpga_port41_modsel.attr,
	&fpga_port41_reset.attr,
	&fpga_port41_present.attr,
	&fpga_port42_lpmode.attr,
	&fpga_port42_modsel.attr,
	&fpga_port42_reset.attr,
	&fpga_port42_present.attr,
	&fpga_port43_lpmode.attr,
	&fpga_port43_modsel.attr,
	&fpga_port43_reset.attr,
	&fpga_port43_present.attr,
	&fpga_port44_lpmode.attr,
	&fpga_port44_modsel.attr,
	&fpga_port44_reset.attr,
	&fpga_port44_present.attr,
	&fpga_port45_lpmode.attr,
	&fpga_port45_modsel.attr,
	&fpga_port45_reset.attr,
	&fpga_port45_present.attr,
	&fpga_port46_lpmode.attr,
	&fpga_port46_modsel.attr,
	&fpga_port46_reset.attr,
	&fpga_port46_present.attr,
	&fpga_port47_lpmode.attr,
	&fpga_port47_modsel.attr,
	&fpga_port47_reset.attr,
	&fpga_port47_present.attr,
	&fpga_port48_lpmode.attr,
	&fpga_port48_modsel.attr,
	&fpga_port48_reset.attr,
	&fpga_port48_present.attr,
	&fpga_port49_lpmode.attr,
	&fpga_port49_modsel.attr,
	&fpga_port49_reset.attr,
	&fpga_port49_present.attr,
	&fpga_port50_lpmode.attr,
	&fpga_port50_modsel.attr,
	&fpga_port50_reset.attr,
	&fpga_port50_present.attr,
	&fpga_port51_lpmode.attr,
	&fpga_port51_modsel.attr,
	&fpga_port51_reset.attr,
	&fpga_port51_present.attr,
	&fpga_port52_lpmode.attr,
	&fpga_port52_modsel.attr,
	&fpga_port52_reset.attr,
	&fpga_port52_present.attr,
	&fpga_port53_lpmode.attr,
	&fpga_port53_modsel.attr,
	&fpga_port53_reset.attr,
	&fpga_port53_present.attr,
	&fpga_port54_lpmode.attr,
	&fpga_port54_modsel.attr,
	&fpga_port54_reset.attr,
	&fpga_port54_present.attr,
	&fpga_port55_lpmode.attr,
	&fpga_port55_modsel.attr,
	&fpga_port55_reset.attr,
	&fpga_port55_present.attr,
	&fpga_port56_lpmode.attr,
	&fpga_port56_modsel.attr,
	&fpga_port56_reset.attr,
	&fpga_port56_present.attr,
	&fpga_port57_lpmode.attr,
	&fpga_port57_modsel.attr,
	&fpga_port57_reset.attr,
	&fpga_port57_present.attr,
	&fpga_port58_lpmode.attr,
	&fpga_port58_modsel.attr,
	&fpga_port58_reset.attr,
	&fpga_port58_present.attr,
	&fpga_port59_lpmode.attr,
	&fpga_port59_modsel.attr,
	&fpga_port59_reset.attr,
	&fpga_port59_present.attr,
	&fpga_port60_lpmode.attr,
	&fpga_port60_modsel.attr,
	&fpga_port60_reset.attr,
	&fpga_port60_present.attr,
	&fpga_port61_lpmode.attr,
	&fpga_port61_modsel.attr,
	&fpga_port61_reset.attr,
	&fpga_port61_present.attr,
	&fpga_port62_lpmode.attr,
	&fpga_port62_modsel.attr,
	&fpga_port62_reset.attr,
	&fpga_port62_present.attr,
	&fpga_port63_lpmode.attr,
	&fpga_port63_modsel.attr,
	&fpga_port63_reset.attr,
	&fpga_port63_present.attr,
	&fpga_port64_lpmode.attr,
	&fpga_port64_modsel.attr,
	&fpga_port64_reset.attr,
	&fpga_port64_present.attr,
	NULL,
};

static struct attribute_group fpga_attr_group = {
	.attrs = fpga_attrs,
};

static int fpga_dev_init(struct fpga_priv *priv)
{
	/* allocate memory for the descriptor */
	priv->hw = dmam_alloc_coherent(&priv->pci_dev->dev,
				       (FPGA_DESC_ENTRIES
					* sizeof(struct fpga_desc)),
				       &priv->io_rng_dma,
				       GFP_KERNEL);
	if (!priv->hw)
		return -ENOMEM;

	memset(priv->hw, 0, (FPGA_DESC_ENTRIES * sizeof(struct fpga_desc)));
	return 0;
}

static void fpga_dev_release(struct fpga_priv *priv)
{
	devm_kfree(&priv->pci_dev->dev, priv->hw);
}

/* I2C Initialization */
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
	pr_info("I2C driver unloaded\n");
}

static int i2c_init(void)
{
	int ismt_bus;
	int i801_bus;
	int ret;
	int i;

	ismt_bus = cumulus_i2c_find_adapter(ISMT_ADAPTER_NAME);
	if (ismt_bus < 0) {
		pr_err("could not find the iSMT adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}
	i801_bus = cumulus_i2c_find_adapter(I801_ADAPTER_NAME);
	if (i801_bus < 0) {
		pr_err("could not find the i801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == I2C_ISMT_BUS)
			bus = ismt_bus;
		else if (bus == I2C_I801_BUS)
			bus = i801_bus;
		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_devices[i].client = client;
	}
	pr_info("I2C driver loaded\n");
	return 0;
err_exit:
	i2c_exit();
	return ret;
}

static int fpga_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct fpga_priv *priv;
	struct ocores_i2c_device_info *info;
	struct ocores_i2c_platform_data *oipd;
	struct resource *ores;
	struct i2c_board_info *binfo;
	struct platform_device *platdev;
	unsigned long start, len;
	int i, err = 0, fail = 0;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	pci_set_drvdata(pdev, priv);
	priv->pci_dev = pdev;

	err = pcim_enable_device(pdev);
	if (err) {
		dev_err(&pdev->dev, "Failed to enable FPGA PCI device (%d)\n",
			err);
		devm_kfree(&pdev->dev, priv);
		goto exit;
	}

	/* Determine the address of the FPGA area */
	start = pci_resource_start(pdev, FPGA_BAR);
	len = pci_resource_len(pdev, FPGA_BAR);
	if (!start || !len) {
		dev_err(&pdev->dev,
			"FPGA base address uninitialized, upgrade BIOS\n");
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		err = -ENODEV;
		goto exit;
	}

	err = fpga_dev_init(priv);
	if (err) {
		pr_err("fpga_dev_init() failed");
		err = -ENOMEM;
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		goto exit;
	}

	/*
	 * Initialise a ctrl resource for various LED, DMA, and
	 * SFP/QSFP reset/modsel etc. registers in the physical
	 * memory range 0x0 - 0x5FFF.
	 */
	ctrl_resource.start = start;
	ctrl_resource.end   = start + 0x5FFF;
	ctrl_resource.flags = IORESOURCE_MEM;

	priv->pbar = devm_ioremap_resource(&pdev->dev, &ctrl_resource);
	if (!priv->pbar) {
		pr_err("devm_ioremap_resource failed for cres");
		err = -ENOMEM;
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		goto exit;
	}

	/*
	 * Create sysfs group for ctrl resource.
	 */
	err = sysfs_create_group(&pdev->dev.kobj, &fpga_attr_group);
	if (err) {
		pr_err("sysfs_fpga_attr_group failed for FPGA driver\n");
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		goto exit;
	}

	/*
	 * FPGA I2C_CH1  gets start 0x6000, end 0x600F
	 * FPGA I2C_CH2  gets start 0x6010, end 0x601F
	 * FPGA I2C_CH3  gets start 0x6020, end 0x602F
	 * FPGA I2C_CH4  gets start 0x6030, end 0x603F
	 * ...
	 * FPGA I2C_CH16 gets start 0x60F0, end 0x60FF
	 * 
	 * The loop initializes ocores_resources array
	 * with resources for I2C_CH4 to I2C_CH11
	 */
	for (i = 0; i < ARRAY_SIZE(ocores_resources); i++) {
		ocores_resources[i].start = start + 0x6000 + ((i + 3) * 0x10);
		ocores_resources[i].end   = ocores_resources[i].start + 0xF;
		ocores_resources[i].flags = IORESOURCE_MEM;
	}

	info = ocores_i2c_device_infotab;
	ores = ocores_resources;
	for (i = 0; i < FPGA_I2C_BUS_MAX; i++, info++, ores++) {
		platdev = platform_device_alloc("ocores-i2c", i);
		if (!platdev) {
			pr_err("platform_device_alloc(%d) failed", i);
			err = -ENOMEM;
			fail = 1;
			goto exit;
		}
		ores = &ocores_resources[i];
		err = platform_device_add_resources(platdev, ores, 1);
		if (err) {
			pr_err("platform_device_add_resources(%d) failed", i);
			fail = 1;
			goto exit;
		}

		info->bus = ocores_i2c_devices[i + 8 * i].bus;
		binfo = &ocores_i2c_devices[i + 8 * i].board_info;
		info->info = binfo;
		oipd = &ocores_i2c_data[i];
		oipd->clock_khz		= 100000;
		oipd->devices		= info;
		oipd->num_devices	= 1;
		oipd->interrupt_mode	= OCI2C_POLL;

		err = platform_device_add_data(platdev, oipd, sizeof(*oipd));
		if (err) {
			pr_err("platform_device_add_data(%d) failed", i);
			fail = 1;
			goto exit;
		}

		err = platform_device_add(platdev);
		if (err) {
			pr_err("platform_device_add() failed for ocores %d", i);
			fail = 1;
			goto exit;
		}
	}

	pr_info("fpga driver loaded\n");

exit:
	if (fail) {
		sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
	}
	return err;
}

static void fpga_remove(struct pci_dev *pdev)
{
	struct fpga_priv *priv;

	priv = dev_get_drvdata(&pdev->dev);
	sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
	devm_iounmap(&pdev->dev, priv->pbar);
	fpga_dev_release(priv);
	pci_disable_device(pdev);
	devm_kfree(&pdev->dev, priv);

	pr_info("FPGA driver unloaded\n");
}

#ifdef CONFIG_PM
static int fpga_suspend(struct pci_dev *pdev, pm_message_t mesg)
{
	pci_save_state(pdev);
	pci_set_power_state(pdev, pci_choose_state(pdev, mesg));
	return 0;
}

static int fpga_resume(struct pci_dev *pdev)
{
	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);
	return pci_enable_device(pdev);
}

#else

#define fpga_suspend NULL
#define fpga_resume NULL

#endif

static struct pci_driver fpga_driver = {
	.name = "fpga_driver",
	.id_table = fpga_id,
	.probe = fpga_probe,
	.remove = fpga_remove,
	.suspend = fpga_suspend,
	.resume = fpga_resume,
};

static int fpga_init(void)
{
	int ret, i;
	struct i2c_client *client;

	ret = pci_register_driver(&fpga_driver);
	if (ret) {
		pr_err("pci_register_driver() failed for fpga device\n");
		return ret;
	}

	for (i = 0; i < ARRAY_SIZE(ocores_i2c_devices); i++) {
		if (ocores_i2c_devices[i].bus < 16)
			continue;
		client = cumulus_i2c_add_client
			(ocores_i2c_devices[i].bus,
			 &ocores_i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			pr_err("fpga_init: cum_i2c_add_cli(%d) failed: %d",
			       ocores_i2c_devices[i].bus,
			       ret);
			return ret;
		}
	}

	pr_info("FPGA driver loaded\n");
	return 0;
}

static void fpga_exit(void)
{
	pci_unregister_driver(&fpga_driver);
	pr_info("FPGA driver unloaded\n");
}

/*-----------------------------------------------------------------------------
 *
 * Module init/exit
 */
static int __init dellemc_z9264f_init(void)
{
	int ret = 0;

	ret = i2c_init();
	if (ret) {
		pr_err("I2C subsystem initialization failed\n");
		return ret;
	}
	ret = fpga_init();
	if (ret) {
		pr_err("FPGA initialization failed\n");
		i2c_exit();
		return ret;
	}
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit dellemc_z9264f_exit(void)
{
	fpga_exit();
	i2c_exit();
	pr_info(DRIVER_NAME " driver successfully unloaded\n");
}

module_init(dellemc_z9264f_init);
module_exit(dellemc_z9264f_exit);

MODULE_AUTHOR("Nikhil Dhar (ndhar@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC Z9264F Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
