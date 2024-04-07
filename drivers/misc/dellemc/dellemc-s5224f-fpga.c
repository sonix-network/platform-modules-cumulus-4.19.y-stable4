// SPDX-Licnese-Identifier: GPL-2.0+
/*
 * Dell EMC S5224F-C3538 FPGA Driver
 * This driver used the Dell EMC S5296F as its model/template.  
 * The s5224f and s5296f hardware is very similar; the s5224f is 
 * pretty much a scaled-down version of the s5296f.  The key differences
 * are that the s5224f only has 24 SFP28 ports and 4 QSFP ports
 * (versus 96 SFP28 ports and 8 QSFP ports on the s5296f).  Consequentally,
 * the s5224f makes use of fewer I2C muxes.
 *
 * Copyright (c) 2019 Cumulus Networks, Inc.  All Rights Reserved.
 * Author: Andy Rao (arao@cumulusnetworks.com)
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
#include <linux/platform_data/i2c-ocores.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/cumulus-platform.h>
#include "platform-defs.h"
#include "platform-bitfield.h"
#include "dellemc-z9xxx-s52xx-fpga.h"

#define DRIVER_NAME		  "dellemc_s5224f_fpga"
#define DRIVER_VERSION		  "1.0"

#define NUM_FPGA_BUSSES	16

#define PCI_DEVICE_ID_XILINX_FPGA 0x7021
#define FPGA_BLOCK_MAX		  32
#define FPGA_MAX_RETRIES	  5
#define FPGA_BAR		  0
#define FPGA_DESC_ENTRIES	  2
#define FPGA_FAILED_READ_REG	  0xffffffffU

/*
 * The FPGA on this platform has a number of I2C cores that connect to the
 * front panel ports and CPLD.
 */

enum {
	FPGA_I2C_CH1 = 5, /* CPLD */
	FPGA_I2C_CH2,
	FPGA_I2C_CH3,
	FPGA_I2C_CH4,	/*  SFP28_1  -	SFP28_8	  */
	FPGA_I2C_CH5,	/*  SFP28_9  -	SFP28_16  */
	FPGA_I2C_CH6,	/*  SFP28_17 -	SFP28_24  */
	FPGA_I2C_CH7,	/*  QSFP28_25 -	QSFP28_28  */

	FPGA_I2C_CH4_MUX_BUS0 = 21,
	FPGA_I2C_CH4_MUX_BUS1,
	FPGA_I2C_CH4_MUX_BUS2,
	FPGA_I2C_CH4_MUX_BUS3,
	FPGA_I2C_CH4_MUX_BUS4,
	FPGA_I2C_CH4_MUX_BUS5,
	FPGA_I2C_CH4_MUX_BUS6,
	FPGA_I2C_CH4_MUX_BUS7,

	FPGA_I2C_CH5_MUX_BUS0,
	FPGA_I2C_CH5_MUX_BUS1,
	FPGA_I2C_CH5_MUX_BUS2,
	FPGA_I2C_CH5_MUX_BUS3,
	FPGA_I2C_CH5_MUX_BUS4,
	FPGA_I2C_CH5_MUX_BUS5,
	FPGA_I2C_CH5_MUX_BUS6,
	FPGA_I2C_CH5_MUX_BUS7,

	FPGA_I2C_CH6_MUX_BUS0,
	FPGA_I2C_CH6_MUX_BUS1,
	FPGA_I2C_CH6_MUX_BUS2,
	FPGA_I2C_CH6_MUX_BUS3,
	FPGA_I2C_CH6_MUX_BUS4,
	FPGA_I2C_CH6_MUX_BUS5,
	FPGA_I2C_CH6_MUX_BUS6,
	FPGA_I2C_CH6_MUX_BUS7,

	FPGA_I2C_CH7_MUX_BUS0,
	FPGA_I2C_CH7_MUX_BUS1,
	FPGA_I2C_CH7_MUX_BUS2,
	FPGA_I2C_CH7_MUX_BUS3,
	FPGA_I2C_CH7_MUX_BUS4, /* not used */
	FPGA_I2C_CH7_MUX_BUS5, /* not used */
	FPGA_I2C_CH7_MUX_BUS6, /* not used */
	FPGA_I2C_CH7_MUX_BUS7, /* not used */
};

/*
 * The list of I2C devices and their bus connections for this platform.
 *
 * First we construct the necessary data structure for each device, using the
 * method specific to the device type.	Then we put them all together in big
 * tables (see i2c device tables below).
 *
 * For muxes, we specify the starting bus number for the block of ports, using
 * the magic mk_pca954*() macro.
 *
 * For EEPROMs, we specify the label, i2c address, size, and some flags.  All
 * this is done in the magic mk*_eeprom() macro.  The label is the string that
 * ends up in /sys/class/eeprom_dev/eepromN/label, which we use to identify
 * the EEPROM at the user level.
 */

mk_pca9548(fpga_ch4_mux,  FPGA_I2C_CH4_MUX_BUS0,  1);
mk_pca9548(fpga_ch5_mux,  FPGA_I2C_CH5_MUX_BUS0,  1);
mk_pca9548(fpga_ch6_mux,  FPGA_I2C_CH6_MUX_BUS0,  1);
mk_pca9548(fpga_ch7_mux,  FPGA_I2C_CH7_MUX_BUS0,  1);

mk_port_eeprom(port1,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port2,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port3,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port4,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port5,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port6,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port7,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port8,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port9,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port10,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port11,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port12,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port13,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port14,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port15,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port16,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port17,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port18,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port19,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port20,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port21,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port22,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port23,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port24,	50, 512, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

mk_qsfp_port_eeprom(port25,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port26,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port27,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port28,  50, 256, SFF_8436_FLAG_IRUGO);

/*
 * i2c device tables
 *
 * We use the magic mk_i2cdev() macro to construct the entries.	 Each entry is
 * a bus number and an i2c_board_info.	The i2c_board_info structure specifies
 * the device type, address, and platform data specific to the device type.
 *
 * The i2c_devices[] table contains all the devices that we expose on the
 * Denverton i2c busses.
 *
 * The fpga_i2c_mux_devices[] table contains just the PCA9548 muxes on the fpga
 * i2c busses.
 *
 * The fpga_i2c_devices[] table has all the remaining devices exposed on the
 * fpga i2c busses.  A separate structure, fpga_device_infotab[], is
 * built and used to communicate i2c device information to the fpga i2c driver
 * in "slices" so that it can manipulate the fpga registers as needed in order
 * to access those devices per the OpenCores i2c specification.
 *
 */

static struct platform_i2c_device_info fpga_i2c_busses[] = {
	mk_i2cdev(FPGA_I2C_CH1,	 "", 0x01, NULL),
	mk_i2cdev(FPGA_I2C_CH2,	 "", 0x01, NULL),
	mk_i2cdev(FPGA_I2C_CH3,	 "", 0x01, NULL),
	mk_i2cdev(FPGA_I2C_CH4,	 "", 0x01, NULL),
	mk_i2cdev(FPGA_I2C_CH5,	 "", 0x01, NULL),
	mk_i2cdev(FPGA_I2C_CH6,	 "", 0x01, NULL),
	mk_i2cdev(FPGA_I2C_CH7,	 "", 0x01, NULL),
};

static struct platform_i2c_device_info fpga_i2c_devices[] = {
	mk_i2cdev(FPGA_I2C_CH4, "pca9548", 0x74, &fpga_ch4_mux_platform_data),
	mk_i2cdev(FPGA_I2C_CH4_MUX_BUS0, "24c04", 0x50, &port1_50_at24),
	mk_i2cdev(FPGA_I2C_CH4_MUX_BUS1, "24c04", 0x50, &port2_50_at24),
	mk_i2cdev(FPGA_I2C_CH4_MUX_BUS2, "24c04", 0x50, &port3_50_at24),
	mk_i2cdev(FPGA_I2C_CH4_MUX_BUS3, "24c04", 0x50, &port4_50_at24),
	mk_i2cdev(FPGA_I2C_CH4_MUX_BUS4, "24c04", 0x50, &port5_50_at24),
	mk_i2cdev(FPGA_I2C_CH4_MUX_BUS5, "24c04", 0x50, &port6_50_at24),
	mk_i2cdev(FPGA_I2C_CH4_MUX_BUS6, "24c04", 0x50, &port7_50_at24),
	mk_i2cdev(FPGA_I2C_CH4_MUX_BUS7, "24c04", 0x50, &port8_50_at24),

	mk_i2cdev(FPGA_I2C_CH5, "pca9548", 0x74, &fpga_ch5_mux_platform_data),
	mk_i2cdev(FPGA_I2C_CH5_MUX_BUS0, "24c04", 0x50, &port9_50_at24),
	mk_i2cdev(FPGA_I2C_CH5_MUX_BUS1, "24c04", 0x50, &port10_50_at24),
	mk_i2cdev(FPGA_I2C_CH5_MUX_BUS2, "24c04", 0x50, &port11_50_at24),
	mk_i2cdev(FPGA_I2C_CH5_MUX_BUS3, "24c04", 0x50, &port12_50_at24),
	mk_i2cdev(FPGA_I2C_CH5_MUX_BUS4, "24c04", 0x50, &port13_50_at24),
	mk_i2cdev(FPGA_I2C_CH5_MUX_BUS5, "24c04", 0x50, &port14_50_at24),
	mk_i2cdev(FPGA_I2C_CH5_MUX_BUS6, "24c04", 0x50, &port15_50_at24),
	mk_i2cdev(FPGA_I2C_CH5_MUX_BUS7, "24c04", 0x50, &port16_50_at24),

	mk_i2cdev(FPGA_I2C_CH6, "pca9548", 0x74, &fpga_ch6_mux_platform_data),
	mk_i2cdev(FPGA_I2C_CH6_MUX_BUS0, "24c04", 0x50, &port17_50_at24),
	mk_i2cdev(FPGA_I2C_CH6_MUX_BUS1, "24c04", 0x50, &port18_50_at24),
	mk_i2cdev(FPGA_I2C_CH6_MUX_BUS2, "24c04", 0x50, &port19_50_at24),
	mk_i2cdev(FPGA_I2C_CH6_MUX_BUS3, "24c04", 0x50, &port20_50_at24),
	mk_i2cdev(FPGA_I2C_CH6_MUX_BUS4, "24c04", 0x50, &port21_50_at24),
	mk_i2cdev(FPGA_I2C_CH6_MUX_BUS5, "24c04", 0x50, &port22_50_at24),
	mk_i2cdev(FPGA_I2C_CH6_MUX_BUS6, "24c04", 0x50, &port23_50_at24),
	mk_i2cdev(FPGA_I2C_CH6_MUX_BUS7, "24c04", 0x50, &port24_50_at24),

	mk_i2cdev(FPGA_I2C_CH7, "pca9548", 0x74, &fpga_ch7_mux_platform_data),
	mk_i2cdev(FPGA_I2C_CH7_MUX_BUS0, "sff8436", 0x50, &port25_50_sff8436),
	mk_i2cdev(FPGA_I2C_CH7_MUX_BUS1, "sff8436", 0x50, &port26_50_sff8436),
	mk_i2cdev(FPGA_I2C_CH7_MUX_BUS2, "sff8436", 0x50, &port27_50_sff8436),
	mk_i2cdev(FPGA_I2C_CH7_MUX_BUS3, "sff8436", 0x50, &port28_50_sff8436),
};

/*
 * Probe the FPGA device on the PCIe bus.  In the process, expose the relevant
 * FPGA Misc Control registers in the first partition.	Map the address ranges
 * of the I2C slices and pass the ranges to driver for each device.
 *
 */

#define FPGA_I2C_DRIVER_NAME "ocores-i2c"

/* fpga_id - PCI device ID supported by this driver */
static const struct pci_device_id fpga_id[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_XILINX, PCI_DEVICE_ID_XILINX_FPGA) },
	{ 0, }
};

static struct pci_driver fpga_driver;

#define FPGA_PORTS 28

/* FPGA Hardware Descriptor */
struct fpga_desc {
	u8 tgtaddr_rw;	/* target address & r/w bit */
	u8 wr_len_cmd;	/* write length in bytes or a command */
	u8 rd_len;	/* read length */
	u8 control;	/* control bits */
	u8 status;	/* status bits */
	u8 retry;	/* collision retry and retry count */
	u8 rxbytes;	/* received bytes */
	u8 txbytes;	/* transmitted bytes */
	u32 dptr_low;	/* lower 32 bit of the data pointer */
	u32 dptr_high;	/* upper 32 bit of the data pointer */
} __packed;

struct fpga_priv {
	u8 __iomem                   *pbar;
	struct pci_dev               *pci_dev;
	struct fpga_desc             *hw;
};

/* Accessor functions for reading and writing the FPGA registers */
static int fpga_read_reg(struct device *dev,
			 int reg,
			 int nregs,
			 u32 *val)
{
	struct fpga_priv *priv = dev_get_drvdata(dev);

	if (!priv)
		return -EIO;
	*val = readl(priv->pbar + reg);
	return 0;
}

static int fpga_write_reg(struct device *dev,
			  int reg,
			  int nregs,
			  u32 val)
{
	struct fpga_priv *priv = dev_get_drvdata(dev);

	if (!priv)
		return -EIO;
	writel(val, priv->pbar + reg);
	return 0;
}

/*
 * OpenCores PCI mapping data and register accessors
 */

/* string arrays to map values into names */
static const char * const led_fan_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER_BLINKING,
};

static const char * const led_system_values[] = {
	PLATFORM_LED_GREEN_BLINKING,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_AMBER_BLINKING,
};

static const char * const led_beacon_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_BLUE_BLINKING,
};

static const char * const led_power_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_AMBER,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_AMBER_BLINKING,
};

static const char * const led_stack_values[] = {
	PLATFORM_LED_GREEN,
	PLATFORM_LED_OFF,
};

static const char * const led_blink_values[] = {
	PLATFORM_LED_GREEN,
	PLATFORM_LED_GREEN_BLINKING,
};

static const char * const led_off_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_ON,
};

static const char * const led_dot_values[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_ON,
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

#define fpga_bf_ro(_name, _reg, _field, _values, _flags) \
	mk_bf_ro(fpga, _name, _reg, _field##_LSB, FIELD_WIDTH(_field), \
		 _values, _flags)

#define fpga_bf_rw(_name, _reg, _field, _values, _flags) \
	mk_bf_rw(fpga, _name, _reg, _field##_LSB, FIELD_WIDTH(_field), \
		 _values, _flags)

#define fpga_bt_ro(_name, _reg, _field, _values, _flags) \
	mk_bf_ro(fpga, _name, _reg, _field##_BIT, 1, _values, _flags)

#define fpga_bt_rw(_name, _reg, _field, _values, _flags) \
	mk_bf_rw(fpga, _name, _reg, _field##_BIT, 1, _values, _flags)

#define fpga_rg_ro(_name, _reg, _values, _flags) \
	mk_bf_ro(fpga, _name, _reg, 0, 32, _values, _flags)

#define fpga_rg_rw(_name, _reg, _values, _flags) \
	mk_bf_rw(fpga, _name, _reg, 0, 32, _values, _flags)

#define fpga_sfp_port(_num) \
	fpga_bt_rw(port##_num##_tx_enable, \
	   DELL_Z9S52_PORT_XCVR_PORT_CTRL_REG + (16 * ((_num) - 1)), \
	   DELL_Z9S52_PORT_XCVR_PORT_CTRL_TX_DIS, NULL, BF_COMPLEMENT); \
	fpga_bt_ro(port##_num##_tx_fault, \
	   DELL_Z9S52_PORT_XCVR_PORT_STS_REG + (16 * ((_num) - 1)), \
	   DELL_Z9S52_PORT_XCVR_PORT_STS_TXFAULT, NULL, 0); \
	fpga_bt_ro(port##_num##_rx_los, \
	   DELL_Z9S52_PORT_XCVR_PORT_STS_REG + (16 * ((_num) - 1)), \
	   DELL_Z9S52_PORT_XCVR_PORT_STS_RXLOS, NULL, 0); \
	fpga_bt_ro(port##_num##_present, \
	   DELL_Z9S52_PORT_XCVR_PORT_STS_REG + (16 * ((_num) - 1)), \
	   DELL_Z9S52_PORT_XCVR_PORT_STS_MODABS, NULL, BF_COMPLEMENT)

#define fpga_qsfp_port(_num) \
	fpga_bt_rw(port##_num##_lpmode, \
	   DELL_Z9S52_PORT_XCVR_PORT_CTRL_REG + (16 * ((_num) - 1)), \
	   DELL_Z9S52_PORT_XCVR_PORT_CTRL_LPMOD, NULL, 0); \
	fpga_bt_rw(port##_num##_modsel, \
	   DELL_Z9S52_PORT_XCVR_PORT_CTRL_REG + (16 * ((_num) - 1)), \
	   DELL_Z9S52_PORT_XCVR_PORT_CTRL_MODSEL, NULL, BF_COMPLEMENT); \
	fpga_bt_rw(port##_num##_reset, \
	   DELL_Z9S52_PORT_XCVR_PORT_CTRL_REG + (16 * ((_num) - 1)), \
	   DELL_Z9S52_PORT_XCVR_PORT_CTRL_RST, NULL, BF_COMPLEMENT); \
	fpga_bt_ro(port##_num##_present, \
	   DELL_Z9S52_PORT_XCVR_PORT_STS_REG + (16 * ((_num) - 1)), \
	   DELL_Z9S52_PORT_XCVR_PORT_STS_PRSNT, NULL, BF_COMPLEMENT)

#define fpga_sfp_port_attrs(_num) \
	&fpga_port##_num##_tx_enable.attr, \
	&fpga_port##_num##_tx_fault.attr, \
	&fpga_port##_num##_rx_los.attr, \
	&fpga_port##_num##_present.attr

#define fpga_qsfp_port_attrs(_num) \
	&fpga_port##_num##_lpmode.attr, \
	&fpga_port##_num##_modsel.attr, \
	&fpga_port##_num##_reset.attr, \
	&fpga_port##_num##_present.attr

/* Define all the bitfields */
fpga_bf_ro(major_revision, DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_REG,
	   DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_MJR_REV, NULL, 0);
fpga_bf_ro(minor_revision, DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_REG,
	   DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_MNR_REV, NULL, 0);
fpga_rg_rw(scratch, DELL_Z9S52_CTRL_FPGA_GPR_REG, NULL, 0);
fpga_bf_ro(com_e_type, DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_REG,
	   DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_COME_TYPE, NULL, 0);
fpga_bf_ro(board_revision, DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_REG,
	   DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_BRD_REV, NULL, 0);
fpga_bf_ro(board_type, DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_REG,
	   DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_BRD_TYPE, NULL, 0);
fpga_rg_ro(timestamp1, DELL_Z9S52_CTRL_BUILD_TIMESTAMP1_REG, NULL, 0);
fpga_rg_ro(timestamp2, DELL_Z9S52_CTRL_BUILD_TIMESTAMP2_REG, NULL, 0);
fpga_bf_rw(led_fan, DELL_Z9S52_CTRL_SYSTEM_LED_REG,
	   DELL_Z9S52_CTRL_SYSTEM_LED_FAN_LED, led_fan_values, 0);
fpga_bf_rw(led_system, DELL_Z9S52_CTRL_SYSTEM_LED_REG,
	   DELL_Z9S52_CTRL_SYSTEM_LED_SYSTEM, led_system_values, 0);
fpga_bt_rw(led_beacon, DELL_Z9S52_CTRL_SYSTEM_LED_REG,
	   DELL_Z9S52_CTRL_SYSTEM_LED_BEACON, led_beacon_values, 0);
fpga_bf_rw(led_power, DELL_Z9S52_CTRL_SYSTEM_LED_REG,
	   DELL_Z9S52_CTRL_SYSTEM_LED_POWER, led_power_values, 0);
fpga_bt_rw(led_stack, DELL_Z9S52_CTRL_SYSTEM_LED_REG,
	   DELL_Z9S52_CTRL_SYSTEM_LED_STACK_LED, led_stack_values, 0);
fpga_bt_rw(led_blink, DELL_Z9S52_CTRL_SVN_SEG_STK_LED_REG,
	   DELL_Z9S52_CTRL_SVN_SEG_STK_LED_LED_BLNK, led_blink_values, 0);
fpga_bt_rw(led_off, DELL_Z9S52_CTRL_SVN_SEG_STK_LED_REG,
	   DELL_Z9S52_CTRL_SVN_SEG_STK_LED_LED_OFF, led_off_values, 0);
fpga_bt_rw(led_dot, DELL_Z9S52_CTRL_SVN_SEG_STK_LED_REG,
	   DELL_Z9S52_CTRL_SVN_SEG_STK_LED_DOT,  led_dot_values, 0);
fpga_bf_rw(led_digit, DELL_Z9S52_CTRL_SVN_SEG_STK_LED_REG,
	   DELL_Z9S52_CTRL_SVN_SEG_STK_LED_DGT, led_digit_values, 0);
fpga_bt_ro(psu_pwr2_all_ok, DELL_Z9S52_CTRL_PWR_STS_REG,
	   DELL_Z9S52_CTRL_PWR_STS_PSU2_PG, NULL, 0);
fpga_bt_ro(psu_pwr2_present, DELL_Z9S52_CTRL_PWR_STS_REG,
	   DELL_Z9S52_CTRL_PWR_STS_PSU2_PRSNT, NULL, BF_COMPLEMENT);
fpga_bt_ro(psu_pwr1_all_ok, DELL_Z9S52_CTRL_PWR_STS_REG,
	   DELL_Z9S52_CTRL_PWR_STS_PSU1_PG, NULL, 0);
fpga_bt_ro(psu_pwr1_present, DELL_Z9S52_CTRL_PWR_STS_REG,
	   DELL_Z9S52_CTRL_PWR_STS_PSU1_PRSNT, NULL, BF_COMPLEMENT);
fpga_bf_ro(fpga_vendor_id, DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_REG,
	   DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_VNDR, NULL, 0);
fpga_bt_rw(cause_cold_reset, DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG,
	   DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_COLD_RESET, NULL, 0);
fpga_bt_rw(cause_warm_reset, DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG,
	   DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_WARM_RESET, NULL, 0);
fpga_bt_rw(cause_btn_cold_reboot, DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG,
	   DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_RST_BTN_COLD_REBOOT, NULL, 0);
fpga_bt_rw(cause_btn_shutdown, DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG,
	   DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_RST_BTN_SHUT_DOWN, NULL, 0);
fpga_bt_rw(cause_hotswap_shutdown, DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG,
	   DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_HOTSWP_SHUT_DOWN, NULL, 0);
fpga_bt_rw(cause_bmc_shutdown, DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG,
	   DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_BMC_SHUTDOWN, NULL, 0);
fpga_bt_rw(cause_wd_fail, DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG,
	   DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_WD_FAIL, NULL, 0);
fpga_bt_rw(cause_cpu_thermtrip, DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG,
	   DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_CPU_THRMTRIP, NULL, 0);
fpga_bt_rw(cause_psu_shutdown, DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG,
	   DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_PSU_SHUT_DOWN, NULL, 0);
fpga_bt_rw(cause_shutdown, DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG,
	   DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_SHUT_DOWN, NULL, 0);
fpga_bt_rw(cause_power_err, DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG,
	   DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_PWR_ERR, NULL, 0);
fpga_bt_rw(cpu_ctrl, DELL_Z9S52_CTRL_SYSTEM_LED_REG,
	   DELL_Z9S52_CTRL_SYSTEM_LED_CPU_CTRL, NULL, 0);
fpga_bt_rw(pwr_down, DELL_Z9S52_CTRL_BMC_PWR_CTRL_REG,
	   DELL_Z9S52_CTRL_BMC_PWR_CTRL_PWR_DWN, NULL, 0);
fpga_bt_rw(eeprom_sel, DELL_Z9S52_CTRL_BMC_EEPRM_CTRL_REG,
DELL_Z9S52_CTRL_BMC_EEPRM_CTRL_EEPRM_SEL, NULL, 0);

/* plumb all of the front panel port signals */
fpga_sfp_port(1);
fpga_sfp_port(2);
fpga_sfp_port(3);
fpga_sfp_port(4);
fpga_sfp_port(5);
fpga_sfp_port(6);
fpga_sfp_port(7);
fpga_sfp_port(8);
fpga_sfp_port(9);
fpga_sfp_port(10);
fpga_sfp_port(11);
fpga_sfp_port(12);
fpga_sfp_port(13);
fpga_sfp_port(14);
fpga_sfp_port(15);
fpga_sfp_port(16);
fpga_sfp_port(17);
fpga_sfp_port(18);
fpga_sfp_port(19);
fpga_sfp_port(20);
fpga_sfp_port(21);
fpga_sfp_port(22);
fpga_sfp_port(23);
fpga_sfp_port(24);

fpga_qsfp_port(25);
fpga_qsfp_port(26);
fpga_qsfp_port(27);
fpga_qsfp_port(28);

static struct attribute *fpga_attrs[] = {
	&fpga_major_revision.attr,
	&fpga_minor_revision.attr,
	&fpga_scratch.attr,
	&fpga_com_e_type.attr,
	&fpga_board_revision.attr,
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
	&fpga_fpga_vendor_id.attr,
	&fpga_cause_cold_reset.attr,
	&fpga_cause_warm_reset.attr,
	&fpga_cause_btn_cold_reboot.attr,
	&fpga_cause_btn_shutdown.attr,
	&fpga_cause_hotswap_shutdown.attr,
	&fpga_cause_bmc_shutdown.attr,
	&fpga_cause_wd_fail.attr,
	&fpga_cause_cpu_thermtrip.attr,
	&fpga_cause_psu_shutdown.attr,
	&fpga_cause_shutdown.attr,
	&fpga_cause_power_err.attr,
	&fpga_cpu_ctrl.attr,
	&fpga_pwr_down.attr,
	&fpga_eeprom_sel.attr,

	fpga_sfp_port_attrs(1),
	fpga_sfp_port_attrs(2),
	fpga_sfp_port_attrs(3),
	fpga_sfp_port_attrs(4),
	fpga_sfp_port_attrs(5),
	fpga_sfp_port_attrs(6),
	fpga_sfp_port_attrs(7),
	fpga_sfp_port_attrs(8),
	fpga_sfp_port_attrs(9),
	fpga_sfp_port_attrs(10),
	fpga_sfp_port_attrs(11),
	fpga_sfp_port_attrs(12),
	fpga_sfp_port_attrs(13),
	fpga_sfp_port_attrs(14),
	fpga_sfp_port_attrs(15),
	fpga_sfp_port_attrs(16),
	fpga_sfp_port_attrs(17),
	fpga_sfp_port_attrs(18),
	fpga_sfp_port_attrs(19),
	fpga_sfp_port_attrs(20),
	fpga_sfp_port_attrs(21),
	fpga_sfp_port_attrs(22),
	fpga_sfp_port_attrs(23),
	fpga_sfp_port_attrs(24),

	fpga_qsfp_port_attrs(25),
	fpga_qsfp_port_attrs(26),
	fpga_qsfp_port_attrs(27),
	fpga_qsfp_port_attrs(28),

	NULL,
};

static struct attribute_group  fpga_attr_group = {
	.attrs = fpga_attrs,
};

static int fpga_dev_init(struct fpga_priv *priv)
{
	size_t size;

	size = FPGA_DESC_ENTRIES * sizeof(*priv->hw);

	/* allocate memory for the FPGA descriptor */
	priv->hw = devm_kzalloc(&priv->pci_dev->dev, size, GFP_KERNEL);
	if (!priv->hw)
		return -ENOMEM;

	memset(priv->hw, 0, size);
	return 0;
}

static void fpga_dev_release(struct fpga_priv *priv)
{
	devm_kfree(&priv->pci_dev->dev, priv->hw);
}

/*
 * The Dell S5224F has register blocks allocated to manage 16 FPGA I2C busses
 * I2C1-I2C16 using:
 *
 *     I2C-Master Core Specification, Rev 0.9 Dated July 3 2003.
 *
 * The offsets of these register blocks are described in:
 *
 *     Z9XXX/S52XX Programmable Logic Design Doc, Rev 11 Dated June 11 2018,
 *     Section 4.3.
 */

struct ocores_i2c_device_info fpga_device_infotab[NUM_FPGA_BUSSES];
static struct ocores_i2c_platform_data fpga_i2c_data[NUM_FPGA_BUSSES];

static struct resource ctrl_resource;

static struct platform_device *platdev[NUM_FPGA_BUSSES];

static int fpga_probe(struct pci_dev *pdev, const struct pci_device_id *devid)
{
	struct fpga_priv *priv;
	struct resource *cres;
	struct ocores_i2c_platform_data *fipd;
	unsigned long start, len;
	int i, j, ch, index;
	int err;
	struct i2c_client *client;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dev_err(&pdev->dev, "Failed to allocate memory for FPGA PCI device\n");
		err = -ENOMEM;
		goto error_return;
	}
	pci_set_drvdata(pdev, priv);
	priv->pci_dev = pdev;

	err = pcim_enable_device(pdev);
	if (err) {
		dev_err(&pdev->dev, "Failed to enable FPGA PCI device (%d)\n",
			err);
		goto err_enable_device;
	}

	/* Determine the address of the FPGA area */
	start = pci_resource_start(pdev, FPGA_BAR);
	len = pci_resource_len(pdev, FPGA_BAR);
	if (!start || !len) {
		dev_err(&pdev->dev,
			"FPGA base address uninitialized, upgrade BIOS\n");
		err = -ENODEV;
		goto err_base_addr;
	}

	err = fpga_dev_init(priv);
	if (err) {
		dev_err(&pdev->dev, "FPGA init failed");
		err = -ENOMEM;
		goto err_fpga_init;
	}

	/* Initialize a resource for the registers defined in the FPGA */
	cres = &ctrl_resource;
	cres->start = start;
	cres->end   = cres->start + 0x5FFF;
	cres->flags = IORESOURCE_MEM;

	priv->pbar = devm_ioremap_resource(&pdev->dev, cres);
	if (!priv->pbar) {
		dev_err(&pdev->dev,"devm_ioremap_resource failed for cres\n");
		err = -ENOMEM;
		goto err_ioremap;
	}

	/* Create sysfs group for the FPGA */
	err = sysfs_create_group(&pdev->dev.kobj, &fpga_attr_group);
	if (err) {
		pr_err(DRIVER_NAME \
		       ": sysfs_fpga_attr_group failed for FPGA driver\n");
		devm_iounmap(&pdev->dev, priv->pbar);
		goto err_sysfs_create;
	}

	/*
	 * The device slice loop.  Each of the devices in
	 * fpga_device_infotab[] carves out its own name, resources,
	 * and private data and is initialized here.  At the end, each
	 * infotab device registers with the i2c-ocores driver to share
	 * its idea of its resources.  We therefore get one i2c-ocores
	 * driver instance for each device in fpga_device_infotab[].
	 *
	 * Tacit assumption in multiple places in this slice loop is that
	 * we are registering the minimum we can get away with, which is
	 * each one of the root FPGA I2C busses/adapters 1-16 inclusive.
	 * Everything else can be added from dellemc_s5224f_init() afterward.
	 *
	 * fpga_device_infotab[], ocores_devtab[], and fpga_i2c_data[] had
	 * better all have the same array size as fpga_i2c_mux_devices[]!
	 */
	for (i = FPGA_I2C_CH1; i <= FPGA_I2C_CH7; i++) {
		ch = i - FPGA_I2C_CH1 + 1; /* I2C channel number */
		index = i - FPGA_I2C_CH1; /* array index */

		/* Define the fpga_i2c_resource for each FPGA I2C bus.
		 *
		 * FPGA I2C_CH1	 gets start 0x6000, end 0x600F
		 * FPGA I2C_CH2	 gets start 0x6010, end 0x601F
		 * ...
		 * FPGA I2C_CH16 gets start 0x60F0, end 0x60FF
		 */

		cres = &ctrl_resource;
		cres->start = start + 0x6000 + ((i - FPGA_I2C_CH1) * 0x10);
		cres->end   = cres->start + 0x00F;
		cres->flags = IORESOURCE_MEM;

		platdev[index] = platform_device_alloc(FPGA_I2C_DRIVER_NAME, index);
		if (!platdev[index]) {
			pr_err(DRIVER_NAME \
			       ": device allocation failed for FPGA I2C ch%d\n",
			       ch);
			err = -ENOMEM;
			goto err_device_alloc;
		}

		err = platform_device_add_resources(platdev[index], cres, 1);
		if (err) {
			pr_err(DRIVER_NAME \
			       ": failed to add resources for FPGA I2C ch%d\n",
			       ch);
			goto err_add_resources;
		}
		fpga_device_infotab[index].bus = fpga_i2c_busses[index].bus;
		fpga_device_infotab[index].info =
			&fpga_i2c_busses[index].board_info;

		fipd = &fpga_i2c_data[index];
		fipd->clock_khz		= 100000;
		fipd->devices		= &fpga_device_infotab[index];
		fipd->num_devices	= 1;
		fipd->interrupt_mode	= OCI2C_POLL;

		err = platform_device_add_data(platdev[index], fipd, sizeof(*fipd));
		if (err) {
			pr_err(DRIVER_NAME \
			       ": add data failed for FPGA I2C ch%d\n", ch);
			goto err_add_data;
		}

		err = platform_device_add(platdev[index]);
		if (err) {
			pr_err(DRIVER_NAME \
			       ": failed to add device for FPGA I2C ch%d\n", ch);
			goto err_device_add;
		}
	}

	/*
	 * Allocate all the I2C devices on the FPGA I2C busses.	 The I2C
	 * adapters should have been created already in the probe function.
	 */
	for (j = 0; j < ARRAY_SIZE(fpga_i2c_devices); j++) {
		int bus;
		struct i2c_board_info board_info;

		bus = fpga_i2c_devices[j].bus;
		board_info = fpga_i2c_devices[j].board_info;
		client = cumulus_i2c_add_client(bus, &board_info);
		if (IS_ERR(client)) {
			err = PTR_ERR(client);
			pr_err(DRIVER_NAME \
			       ": add FPGA I2C client failed for bus %d: %d\n",
			       bus, err);
			goto err_devices;
		}
		fpga_i2c_devices[j].client = client;
	}

	pr_info(DRIVER_NAME ": FPGA driver loaded\n");
	return 0;

err_devices:
	while (--j >= 0) {
		if (fpga_i2c_devices[j].client) {
			i2c_unregister_device(fpga_i2c_devices[j].client);
			fpga_i2c_devices[j].client = NULL;
		}
	}

err_device_add:
err_add_data:
err_add_resources:
err_device_alloc:
	while (--i >= FPGA_I2C_CH1) {
		if (platdev[index]) {
			index = i - FPGA_I2C_CH1; /* array index */
			platform_device_unregister(platdev[index]);
			platdev[index] = NULL;
		}
	}

	sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
err_sysfs_create:
	devm_iounmap(&pdev->dev, priv->pbar);
err_ioremap:
	fpga_dev_release(priv);
err_fpga_init:
err_base_addr:
	pci_disable_device(pdev);
err_enable_device:
	devm_kfree(&pdev->dev, priv);
error_return:
	return err;
}

static void fpga_remove(struct pci_dev *pdev)
{
	int i, index;
	struct i2c_client *c;
	struct fpga_priv *priv;

	/* unregister the FPGA i2c clients */
	for (i = ARRAY_SIZE(fpga_i2c_devices); --i >= 0;) {
		c = fpga_i2c_devices[i].client;
		if (c) {
			i2c_unregister_device(c);
			fpga_i2c_devices[i].client = NULL;
		}
	}

	for (i = FPGA_I2C_CH1; i <= FPGA_I2C_CH7; i++) {
		index = i - FPGA_I2C_CH1; /* array index */
		platform_device_unregister(platdev[index]);
		platdev[index] = NULL;
	}
	priv = dev_get_drvdata(&pdev->dev);
	sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
	devm_iounmap(&pdev->dev, priv->pbar);
	/*	devm_kfree(&pdev->dev, priv->hw);*/
	fpga_dev_release(priv);
	pci_disable_device(pdev);
	devm_kfree(&pdev->dev, priv);

	pr_info(DRIVER_NAME ": driver unloaded\n");
}

#define fpga_suspend NULL
#define fpga_resume NULL

static struct pci_driver fpga_driver = {
	.name = DRIVER_NAME,
	.id_table = fpga_id,
	.probe = fpga_probe,
	.remove = fpga_remove,
	.suspend = fpga_suspend,
	.resume = fpga_resume,
};

static int fpga_init(void)
{
	int ret;

	ret = pci_register_driver(&fpga_driver);
	if (ret) {
		pr_err(DRIVER_NAME ": failed to register FPGA device\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": driver registered\n");
	return 0;
}

static void fpga_exit(void)
{
	pci_unregister_driver(&fpga_driver);
	pr_info(DRIVER_NAME ": driver unloaded\n");
}

/* Module init and exit */

static int __init dellemc_s5224f_init(void)
{
	int ret;

	ret = fpga_init();
	if (ret) {
		pr_err(DRIVER_NAME ": FPGA initialization failed\n");
		fpga_exit();
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit dellemc_s5224f_exit(void)
{
	fpga_exit();
	pr_info(DRIVER_NAME ": driver successfully unloaded\n");
}

module_init(dellemc_s5224f_init);
module_exit(dellemc_s5224f_exit);

MODULE_AUTHOR("Andy Rao (arao@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC S5224F Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
