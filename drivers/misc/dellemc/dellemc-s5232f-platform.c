/*
 * dellemc_s5232f_platform.c - Dell EMC S5232F-C3538 Platform Support.
 *
 * Copyright (c) 2018, 2020 Cumulus Networks, Inc.  All Rights Reserved.
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

#define DRIVER_NAME		  "dellemc_s5232f_platform"
#define DRIVER_VERSION		  "1.1"

#define ISMT_ADAPTER_NAME	  "SMBus iSMT adapter"
#define I801_ADAPTER_NAME	  "SMBus I801 adapter"

#define PCI_DEVICE_ID_XILINX_FPGA 0x7021
#define FPGA_BLOCK_MAX		  32
#define FPGA_MAX_RETRIES	  5
#define FPGA_BAR		  0
#define FPGA_DESC_ENTRIES	  2
#define FPGA_FAILED_READ_REG	  0xffffffffU

/*
 * This platform has two i2c busses:
 *   SMBus_0: SMBus iSMT adapter at dff9f000
 *   SMBus_1: SMBus I801 adapter at e000
 *
 * SMBus_0 has only one interesting device:
 *   board eeprom (0x50)
 *
 * SMBus_1 has the two JC42 devices:
 *   dram jc42 (0x18) [not used]
 *   dram jc42 (0x19) [not used]
 *
 * This platform also has a large FPGA on the PCIe bus.	 It is used to
 * access the system LEDs, SFF low speed signals, SFF I2C busses, and
 * miscellaneous control and status registers.
 *
 */

enum {
	I2C_ISMT_BUS = -1,
	I2C_I801_BUS,

	I2C_FPGA_BUS1 = 1,
	I2C_FPGA_BUS2,
	I2C_FPGA_BUS3,
	I2C_FPGA_BUS4,	/* QSFP28_1  - QSFP28_8  */
	I2C_FPGA_BUS5,	/* QSFP28_9  - QSFP28_16 */
	I2C_FPGA_BUS6,	/* QSFP28_17 - QSFP28_24 */
	I2C_FPGA_BUS7,	/* QSFP28_25 - QSFP28_32 */

	I2C_FPGA_BUS4_0 = 20,
	I2C_FPGA_BUS4_1,
	I2C_FPGA_BUS4_2,
	I2C_FPGA_BUS4_3,
	I2C_FPGA_BUS4_4,
	I2C_FPGA_BUS4_5,
	I2C_FPGA_BUS4_6,
	I2C_FPGA_BUS4_7,

	I2C_FPGA_BUS5_0,
	I2C_FPGA_BUS5_1,
	I2C_FPGA_BUS5_2,
	I2C_FPGA_BUS5_3,
	I2C_FPGA_BUS5_4,
	I2C_FPGA_BUS5_5,
	I2C_FPGA_BUS5_6,
	I2C_FPGA_BUS5_7,

	I2C_FPGA_BUS6_0,
	I2C_FPGA_BUS6_1,
	I2C_FPGA_BUS6_2,
	I2C_FPGA_BUS6_3,
	I2C_FPGA_BUS6_4,
	I2C_FPGA_BUS6_5,
	I2C_FPGA_BUS6_6,
	I2C_FPGA_BUS6_7,

	I2C_FPGA_BUS7_0,
	I2C_FPGA_BUS7_1,
	I2C_FPGA_BUS7_2,
	I2C_FPGA_BUS7_3,
	I2C_FPGA_BUS7_4,
	I2C_FPGA_BUS7_5,
	I2C_FPGA_BUS7_6,
	I2C_FPGA_BUS7_7,
};

/*
 * The list of I2C devices and their bus connections for this platform.
 *
 * First we construct the necessary data struction for each device, using the
 * method specific to the device type.  Then we put them all together in big
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

mk_eeprom(board, 50, 256, AT24_FLAG_IRUGO);

mk_pca9548(fpga_bus4, I2C_FPGA_BUS4_0, 1);
mk_pca9548(fpga_bus5, I2C_FPGA_BUS5_0, 1);
mk_pca9548(fpga_bus6, I2C_FPGA_BUS6_0, 1);
mk_pca9548(fpga_bus7, I2C_FPGA_BUS7_0, 1);

/*
 * i2c device tables
 *
 * We use the magic mk_i2cdev() macro to construct the entries.  Each entry is
 * a bus number and an i2c_board_info.  The i2c_board_info structure specifies
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

static struct platform_i2c_device_info i2c_devices[] = {
	mk_i2cdev(I2C_ISMT_BUS, "24c02", 0x50, &board_50_at24),
};

static struct platform_i2c_device_info fpga_i2c_mux_devices[] = {
	mk_i2cdev(I2C_FPGA_BUS4, "pca9548", 0x74, &fpga_bus4_platform_data),
	mk_i2cdev(I2C_FPGA_BUS5, "pca9548", 0x74, &fpga_bus5_platform_data),
	mk_i2cdev(I2C_FPGA_BUS6, "pca9548", 0x74, &fpga_bus6_platform_data),
	mk_i2cdev(I2C_FPGA_BUS7, "pca9548", 0x74, &fpga_bus7_platform_data),
};

static struct platform_i2c_device_info fpga_i2c_devices[] = {
	mk_i2cdev(I2C_FPGA_BUS4_0,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS4_1,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS4_2,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS4_3,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS4_4,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS4_5,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS4_6,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS4_7,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS5_0,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS5_1,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS5_2,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS5_3,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS5_4,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS5_5,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS5_6,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS5_7,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS6_0,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS6_1,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS6_2,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS6_3,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS6_4,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS6_5,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS6_6,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS6_7,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS7_0,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS7_1,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS7_2,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS7_3,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS7_4,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS7_5,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS7_6,  "", 0x0, NULL),
	mk_i2cdev(I2C_FPGA_BUS7_7,  "", 0x0, NULL),
};

/*
 * Denverton I2C Initialization
 */

static void i2c_exit(void)
{
	int i;
	struct i2c_client *c;

	/* unregister the fpga i2c clients */
	for (i = ARRAY_SIZE(fpga_i2c_devices); --i >= 0;) {
		c = fpga_i2c_devices[i].client;
		if (c)
			i2c_unregister_device(c);
	}

	/* unregister the Denverton i2c clients */
	for (i = ARRAY_SIZE(i2c_devices); --i >= 0;) {
		c = i2c_devices[i].client;
		if (c)
			i2c_unregister_device(c);
	}

	pr_info("I2C driver unloaded\n");
}

static int i2c_init(void)
{
	struct i2c_client *client;
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
		int bus;

		bus = i2c_devices[i].bus;
		if (bus == I2C_ISMT_BUS)
			bus = ismt_bus;
		else if (bus == I2C_I801_BUS)
			bus = i801_bus;
		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			pr_err("add client failed for bus %d: %d\n",
			       bus, ret);
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

/*
 * FPGA Interface
*/

#define FPGA_I2C_DRIVER_NAME "ocores-i2c"

#define ITABSIZE ARRAY_SIZE(fpga_i2c_mux_devices)
struct ocores_i2c_device_info fpga_device_infotab[ITABSIZE];
static struct ocores_i2c_platform_data fpga_i2c_data[ITABSIZE];

/* fpga_id - PCI device ID supported by this driver */
static const struct pci_device_id fpga_id[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_XILINX, PCI_DEVICE_ID_XILINX_FPGA) },
	{ 0, }
};

static struct pci_driver fpga_driver;

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
	u8 __iomem *pbar;                       /* PCIe base address register */
	struct pci_dev *pci_dev;
	struct fpga_desc *hw;			/* descriptor virt base addr */
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

/* Define all the bitfields */
mk_bf_ro(fpga, major_rev,	 0x0000, 8,  8, NULL, 0);
mk_bf_ro(fpga, minor_rev,	 0x0000, 0,  8, NULL, 0);
mk_bf_rw(fpga, scratch,		 0x0004, 0, 32, NULL, 0);
mk_bf_ro(fpga, com_e_type,	 0x0008, 8,  3, NULL, 0);
mk_bf_ro(fpga, board_rev,	 0x0008, 4,  4, NULL, 0);
mk_bf_ro(fpga, board_type,	 0x0008, 0,  4, NULL, 0);
mk_bf_ro(fpga, timestamp1,	 0x000c, 0, 32, NULL, 0);
mk_bf_ro(fpga, timestamp2,	 0x0010, 0, 32, NULL, 0);
mk_bf_rw(fpga, cpu_ctrl,	 0x0024, 8,  1, NULL, 0);
mk_bf_rw(fpga, led_fan,		 0x0024, 6,  2, led_fan_values, 0);
mk_bf_rw(fpga, led_system,	 0x0024, 4,  2, led_system_values, 0);
mk_bf_rw(fpga, led_beacon,	 0x0024, 3,  1, led_beacon_values, 0);
mk_bf_rw(fpga, led_power,	 0x0024, 1,  2, led_power_values, 0);
mk_bf_rw(fpga, led_stack,	 0x0024, 0,  1, led_stack_values, 0);
mk_bf_rw(fpga, led_blink,	 0x0028, 6,  1, led_blink_values, 0);
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

static struct attribute *fpga_attrs[] = {
	&fpga_major_rev.attr,
	&fpga_minor_rev.attr,
	&fpga_scratch.attr,
	&fpga_com_e_type.attr,
	&fpga_board_rev.attr,
	&fpga_board_type.attr,
	&fpga_timestamp1.attr,
	&fpga_timestamp2.attr,
	&fpga_cpu_ctrl.attr,
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
 * alloc_port_i2c_board_info -- Allocate an i2c_board_info struct
 * @port - front panel port number, one based
 *
 * For each port in the system allocate an i2c_board_info struct to
 * describe the QSFP28 (sff8436) EEPROM.
 *
 * Returns the allocated i2c_board_info struct or else returns NULL on failure.
 */
#define EEPROM_LABEL_SIZE  8
static struct __init i2c_board_info * alloc_port_i2c_board_info(int port)
{
	char *label = NULL;
	struct eeprom_platform_data *eeprom_data = NULL;
	struct i2c_board_info *board_info = NULL;
	struct sff_8436_platform_data *sff8436_data;

	label = kzalloc(EEPROM_LABEL_SIZE, GFP_KERNEL);
	if (!label)
		goto err_kzalloc;

	eeprom_data = kzalloc(sizeof(*eeprom_data), GFP_KERNEL);
	if (!eeprom_data)
		goto err_kzalloc;

	board_info = kzalloc(sizeof(*board_info), GFP_KERNEL);
	if (!board_info)
		goto err_kzalloc;

	snprintf(label, EEPROM_LABEL_SIZE, "port%u", port);
	eeprom_data->label = label;

	sff8436_data = kzalloc(sizeof(*sff8436_data), GFP_KERNEL);
	if (!sff8436_data)
		goto err_kzalloc;

	sff8436_data->byte_len = 256;
	sff8436_data->flags = SFF_8436_FLAG_IRUGO;
	sff8436_data->page_size = 1;
	sff8436_data->eeprom_data = eeprom_data;
	board_info->platform_data = sff8436_data;
	strcpy(board_info->type, "sff8436");

	board_info->addr = 0x50;

	return board_info;

err_kzalloc:
	kfree(board_info);
	kfree(eeprom_data);
	kfree(label);

	return NULL;
};

/*
 * The Dell S5232F has register blocks allocated to manage 16 FPGA I2C busses
 * I2C1-I2C16 using:
 *
 *     I2C-Master Core Specification, Rev 0.9 Dated July 3 2003.
 *
 * The offsets of these register blocks are described in:
 *
 *     Z9XXX/S52XX Programmable Logic Design Doc, Rev 11 Dated June 11 2018,
 *     Section 4.3.
 *
 * Sizing fpga_resources to NUM_FPGA_BUSSES+1 allows for direct indexing to
 * the 1-based FPGA bus resources.  Entry 0 is empty and is not used.
 */

#define NUM_FPGA_BUSSES	16

static struct resource fpga_resources[NUM_FPGA_BUSSES + 1];
static struct resource ctrl_resource;

static int fpga_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct fpga_priv *priv;
	struct ocores_i2c_device_info *info;
	struct resource *ores, *cres;
	struct platform_i2c_device_info *muxdev;
	struct ocores_i2c_platform_data *oipd;
	struct platform_device *platdev;
	unsigned long start, len;
	int i;
	int err;

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
		return err;
	}

	/* Determine the address of the FPGA area */
	start = pci_resource_start(pdev, FPGA_BAR);
	len = pci_resource_len(pdev, FPGA_BAR);
	if (!start || !len) {
		dev_err(&pdev->dev,
			"FPGA base address uninitialized, upgrade BIOS\n");
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		return -ENODEV;
	}

	err = fpga_dev_init(priv);
	if (err) {
		dev_err(&pdev->dev, "init failed");
		err = -ENOMEM;
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		goto fail;
	}

	/*
	 * Initialize a control resource for various LED and
	 * QSFP28 reset/modsel etc. registers in the physical
	 * memory range 0x0000 - 0x5FFF.
	 */
	cres = &ctrl_resource;
	cres->start = start;
	cres->end   = cres->start + 0x5FFF;
	cres->flags = IORESOURCE_MEM;

	priv->pbar = devm_ioremap_resource(&pdev->dev, cres);
	if (!priv->pbar) {
		pr_err("devm_ioremap_resource failed for cres\n");
		err = -ENOMEM;
		devm_kfree(&pdev->dev, priv->hw);
		fpga_dev_release(priv);
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		goto fail;
	}

	/*
	 * Create sysfs group for ctrl resource.
	 */
	err = sysfs_create_group(&pdev->dev.kobj, &fpga_attr_group);
	if (err) {
		pr_err("sysfs_fpga_attr_group failed for FPGA driver\n");
		devm_iounmap(&pdev->dev, priv->pbar);
		fpga_dev_release(priv);
		pci_disable_device(pdev);
		devm_kfree(&pdev->dev, priv);
		goto fail;
	}

	/*
	 * Initialize fpga_resources[] for all FPGA busses.
	 * The device slice loop will index into this array to
	 * get platform device resources.
	 *
	 * FPGA I2C_CH1  gets start 0x6000, end 0x600F
	 * FPGA I2C_CH1  gets start 0x6010, end 0x601F
	 * ...
	 * FPGA I2C_CH16 gets start 0x60F0, end 0x60FF
	 */
	ores = fpga_resources;
	ores++;		/* skip over 0th entry - it's never used */
	for (i = 1; i < ARRAY_SIZE(fpga_resources); i++, ores++) {
		ores->start = start + 0x6000 + ((i - 1) * 0x10);
		ores->end   = ores->start + 0xF;
		ores->flags = IORESOURCE_MEM;
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
	 * each one of the root FPGA I2C busses/adapters 4-10 inclusive.
	 * Everything else can be added from dellemc_s5232f_init() afterward.
	 *
	 * fpga_device_infotab[], fpga_resources[], ocores_devtab[],
	 * and fpga_i2c_data[] had better all have the same array size as
	 * fpga_i2c_mux_devices[]!
	 */
	muxdev = fpga_i2c_mux_devices;
	info   = fpga_device_infotab;
	for (i = 0; i < ITABSIZE; i++, muxdev++, info++) {
		platdev = platform_device_alloc(FPGA_I2C_DRIVER_NAME, i);
		if (!platdev) {
			pr_err("device allocation failed for ocores %d\n", i);
			err = -ENOMEM;
			sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
			devm_iounmap(&pdev->dev, priv->pbar);
			fpga_dev_release(priv);
			pci_disable_device(pdev);
			devm_kfree(&pdev->dev, priv);
			goto fail;
		}

		ores = &fpga_resources[muxdev->bus];
		err = platform_device_add_resources(platdev, ores, 1);
		if (err) {
			pr_err("failed to add resources for ocores %d\n", i);
			sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
			devm_iounmap(&pdev->dev, priv->pbar);
			fpga_dev_release(priv);
			pci_disable_device(pdev);
			devm_kfree(&pdev->dev, priv);
			goto fail;
		}

		info->bus = muxdev->bus;
		info->info = &muxdev->board_info;

		oipd = &fpga_i2c_data[i];
		oipd->clock_khz		= 100000;
		oipd->devices		= info;
		oipd->num_devices	= 1;
		oipd->interrupt_mode	= OCI2C_POLL;

		err = platform_device_add_data(platdev, oipd, sizeof(*oipd));
		if (err) {
			pr_err("add data failed for ocores %d\n", i);
			sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
			devm_iounmap(&pdev->dev, priv->pbar);
			fpga_dev_release(priv);
			pci_disable_device(pdev);
			devm_kfree(&pdev->dev, priv);
			goto fail;
		}

		err = platform_device_add(platdev);
		if (err) {
			pr_err("failed to add device for ocores %d\n", i);
			sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
			devm_iounmap(&pdev->dev, priv->pbar);
			fpga_dev_release(priv);
			pci_disable_device(pdev);
			devm_kfree(&pdev->dev, priv);
			goto fail;
		}
	}

	pr_debug("fpga driver loaded\n");

fail:
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

#define fpga_suspend NULL
#define fpga_resume NULL

static struct pci_driver fpga_driver = {
	.name = "dellemc_s5232f_fpga",
	.id_table = fpga_id,
	.probe = fpga_probe,
	.remove = fpga_remove,
	.suspend = fpga_suspend,
	.resume = fpga_resume,
};

static int fpga_init(void)
{
	struct platform_i2c_device_info *oipdi;
	struct i2c_client *client;
	int i;
	int ret;
	int port;

	ret = pci_register_driver(&fpga_driver);
	if (ret) {
		pr_err("failed to register fpga device\n");
		return ret;
	}

	/*
	 * Allocate QSFP28 device EEPROM blocks.  i2c-ocores has
	 * by now read the board info and created the I2C adapters; we
	 * can now create the clients.
	 *
	 * XXX: This loop presumes that ports are declared in asccending
	 *	order in fpga_i2c_devices[].
	 * XXX: This loop presumes that only muxes/switches and ports
	 *	are declared for bus I2C_FPGA_BUS4 and higher.  If
	 *	other devices are added on these busses later, the ports
	 *	will probably have to be separated out into their own
	 *	table for ease of processing as is done on other platforms.
	 */
	port = 1;
	oipdi = fpga_i2c_devices;
	for (i = 0; i < ARRAY_SIZE(fpga_i2c_devices); i++, oipdi++) {
		struct i2c_board_info *b_info;

		/* skip to QSFP28 ports */
		if (oipdi->bus < I2C_FPGA_BUS4_0)
			continue;

		b_info = alloc_port_i2c_board_info(port);
		if (!b_info) {
			ret = -ENOMEM;
			pr_err("port allocation failed for bus %d\n",
			       oipdi->bus);
			goto err_exit;
		}
		oipdi->board_info = *b_info;

		client = cumulus_i2c_add_client(oipdi->bus, &oipdi->board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			pr_err("add client failed for bus %d: %d\n",
			       oipdi->bus, ret);
			goto err_exit;
		}
		fpga_i2c_devices[i].client = client;

		port = port + 1;
	}

	pr_info("FPGA driver registered\n");
	return 0;

err_exit:
	pr_err("FPGA driver failed to register\n");
	return -1;
}

static void fpga_exit(void)
{
	pci_unregister_driver(&fpga_driver);
	pr_info("FPGA driver unregistered\n");
}

/*
 * Module init/exit
 */

static int __init dellemc_s5232f_init(void)
{
	int ret = 0;

	ret = i2c_init();
	if (ret) {
		pr_err("I2C subsystem initialization failed\n");
		i2c_exit();
		return ret;
	}

	ret = fpga_init();
	if (ret) {
		pr_err("FPGA initialization failed\n");
		fpga_exit();
		i2c_exit();
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit dellemc_s5232f_exit(void)
{
	fpga_exit();
	i2c_exit();
	pr_info(DRIVER_NAME " driver successfully unloaded\n");
}

module_init(dellemc_s5232f_init);
module_exit(dellemc_s5232f_exit);

MODULE_AUTHOR("David Yen (dhyen@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC S5232F Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
