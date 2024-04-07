/*
 * accton-minipack-platform.c - Accton Minipack Platform Support.
 *
 * Copyright 2019 Cumulus Networks, Inc.
 * Author: Alok Kumar (alok@cumulusnetworks.com)
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
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/pci.h>

#include <linux/cumulus-platform.h>
#include "accton-minipack-platform.h"
#include "platform-defs.h"
#include "platform-bitfield.h"

#define DRIVER_NAME	"accton_minipack_platform"
#define FPGA_DRIVER     "minipack_io_fpga"
#define DRIVER_VERSION	"1.0"

#define PCI_DEVICE_ID_FACEBOOK_FPGA 0x0011
#define PCI_VENDOR_ID_FACEBOOK 0x1d9b
#define FPGA_BAR 0
#define FPGA_DESC_ENTRIES	  2

static const struct pci_device_id fpga_id[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_FACEBOOK, PCI_DEVICE_ID_FACEBOOK_FPGA) },
	{ 0, }
};

/*
 * The platform has one CP2112 USB-to-I2C bridge. The CP2112 has a
 * CPLD, a 24LC64 (64k-bit) EEPROM, a pca9548 (8-port) I2C mux
 * each of which goes to 2 other pca9548 on each PIM slot and an eeprom
 */
enum {
	MP_I2C_CP2112_BUS = -1,

	MP_I2C_MUX1_BUS0 = 10,
	MP_I2C_MUX1_BUS1,
	MP_I2C_MUX1_BUS2,
	MP_I2C_MUX1_BUS3,
	MP_I2C_MUX1_BUS4,
	MP_I2C_MUX1_BUS5,
	MP_I2C_MUX1_BUS6,
	MP_I2C_MUX1_BUS7,

	MP_I2C_MUX2_BUS0 = 20,
	MP_I2C_MUX2_BUS1,
	MP_I2C_MUX2_BUS2,
	MP_I2C_MUX2_BUS3,
	MP_I2C_MUX2_BUS4,
	MP_I2C_MUX2_BUS5,
	MP_I2C_MUX2_BUS6,
	MP_I2C_MUX2_BUS7,

	MP_I2C_MUX3_BUS0 = 30,
	MP_I2C_MUX3_BUS1,
	MP_I2C_MUX3_BUS2,
	MP_I2C_MUX3_BUS3,
	MP_I2C_MUX3_BUS4,
	MP_I2C_MUX3_BUS5,
	MP_I2C_MUX3_BUS6,
	MP_I2C_MUX3_BUS7,

	MP_I2C_MUX4_BUS0 = 40,
	MP_I2C_MUX4_BUS1,
	MP_I2C_MUX4_BUS2,
	MP_I2C_MUX4_BUS3,
	MP_I2C_MUX4_BUS4,
	MP_I2C_MUX4_BUS5,
	MP_I2C_MUX4_BUS6,
	MP_I2C_MUX4_BUS7,

	MP_I2C_MUX5_BUS0 = 50,
	MP_I2C_MUX5_BUS1,
	MP_I2C_MUX5_BUS2,
	MP_I2C_MUX5_BUS3,
	MP_I2C_MUX5_BUS4,
	MP_I2C_MUX5_BUS5,
	MP_I2C_MUX5_BUS6,
	MP_I2C_MUX5_BUS7,

	MP_I2C_MUX6_BUS0 = 60,
	MP_I2C_MUX6_BUS1,
	MP_I2C_MUX6_BUS2,
	MP_I2C_MUX6_BUS3,
	MP_I2C_MUX6_BUS4,
	MP_I2C_MUX6_BUS5,
	MP_I2C_MUX6_BUS6,
	MP_I2C_MUX6_BUS7,

	MP_I2C_MUX7_BUS0 = 70,
	MP_I2C_MUX7_BUS1,
	MP_I2C_MUX7_BUS2,
	MP_I2C_MUX7_BUS3,
	MP_I2C_MUX7_BUS4,
	MP_I2C_MUX7_BUS5,
	MP_I2C_MUX7_BUS6,
	MP_I2C_MUX7_BUS7,

	MP_I2C_MUX8_BUS0 = 80,
	MP_I2C_MUX8_BUS1,
	MP_I2C_MUX8_BUS2,
	MP_I2C_MUX8_BUS3,
	MP_I2C_MUX8_BUS4,
	MP_I2C_MUX8_BUS5,
	MP_I2C_MUX8_BUS6,
	MP_I2C_MUX8_BUS7,

	MP_I2C_MUX9_BUS0 = 90,
	MP_I2C_MUX9_BUS1,
	MP_I2C_MUX9_BUS2,
	MP_I2C_MUX9_BUS3,
	MP_I2C_MUX9_BUS4,
	MP_I2C_MUX9_BUS5,
	MP_I2C_MUX9_BUS6,
	MP_I2C_MUX9_BUS7,

	MP_I2C_MUX10_BUS0 = 100,
	MP_I2C_MUX10_BUS1,
	MP_I2C_MUX10_BUS2,
	MP_I2C_MUX10_BUS3,
	MP_I2C_MUX10_BUS4,
	MP_I2C_MUX10_BUS5,
	MP_I2C_MUX10_BUS6,
	MP_I2C_MUX10_BUS7,

	MP_I2C_MUX11_BUS0 = 110,
	MP_I2C_MUX11_BUS1,
	MP_I2C_MUX11_BUS2,
	MP_I2C_MUX11_BUS3,
	MP_I2C_MUX11_BUS4,
	MP_I2C_MUX11_BUS5,
	MP_I2C_MUX11_BUS6,
	MP_I2C_MUX11_BUS7,

	MP_I2C_MUX12_BUS0 = 120,
	MP_I2C_MUX12_BUS1,
	MP_I2C_MUX12_BUS2,
	MP_I2C_MUX12_BUS3,
	MP_I2C_MUX12_BUS4,
	MP_I2C_MUX12_BUS5,
	MP_I2C_MUX12_BUS6,
	MP_I2C_MUX12_BUS7,

	MP_I2C_MUX13_BUS0 = 130,
	MP_I2C_MUX13_BUS1,
	MP_I2C_MUX13_BUS2,
	MP_I2C_MUX13_BUS3,
	MP_I2C_MUX13_BUS4,
	MP_I2C_MUX13_BUS5,
	MP_I2C_MUX13_BUS6,
	MP_I2C_MUX13_BUS7,

	MP_I2C_MUX14_BUS0 = 140,
	MP_I2C_MUX14_BUS1,
	MP_I2C_MUX14_BUS2,
	MP_I2C_MUX14_BUS3,
	MP_I2C_MUX14_BUS4,
	MP_I2C_MUX14_BUS5,
	MP_I2C_MUX14_BUS6,
	MP_I2C_MUX14_BUS7,

	MP_I2C_MUX15_BUS0 = 150,
	MP_I2C_MUX15_BUS1,
	MP_I2C_MUX15_BUS2,
	MP_I2C_MUX15_BUS3,
	MP_I2C_MUX15_BUS4,
	MP_I2C_MUX15_BUS5,
	MP_I2C_MUX15_BUS6,
	MP_I2C_MUX15_BUS7,

	MP_I2C_MUX16_BUS0 = 160,
	MP_I2C_MUX16_BUS1,
	MP_I2C_MUX16_BUS2,
	MP_I2C_MUX16_BUS3,
	MP_I2C_MUX16_BUS4,
	MP_I2C_MUX16_BUS5,
	MP_I2C_MUX16_BUS6,
	MP_I2C_MUX16_BUS7,

	MP_I2C_MUX17_BUS0 = 170,
	MP_I2C_MUX17_BUS1,
	MP_I2C_MUX17_BUS2,
	MP_I2C_MUX17_BUS3,
	MP_I2C_MUX17_BUS4,
	MP_I2C_MUX17_BUS5,
	MP_I2C_MUX17_BUS6,
	MP_I2C_MUX17_BUS7,
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

/*
 * The list of I2C devices and their bus connections for this platform.
 *
 * Each entry is a bus number and a i2c_board_info.
 * The i2c_board_info specifies the device type, address,
 * and platform data depending on the device type.
 *
 * For muxes, we specify the bus number for each port,
 * and set the deselect_on_exit but (see comment above).
 *
 * For EEPROMs, including ones in the QSFP28 transceivers,
 * we specify the label, I2C address, size, and some flags.
 * All done in the magic mk*_eeprom() macros.  The label is
 * the string that ends up in /sys/class/eeprom_dev/eepromN/label,
 * which we use to identify them at user level.
 */

mk_pca9548(mux1,  MP_I2C_MUX1_BUS0,  1);
mk_pca9548(mux2,  MP_I2C_MUX2_BUS0,  1);
mk_pca9548(mux3,  MP_I2C_MUX3_BUS0,  1);
mk_pca9548(mux4,  MP_I2C_MUX4_BUS0,  1);
mk_pca9548(mux5,  MP_I2C_MUX5_BUS0,  1);
mk_pca9548(mux6,  MP_I2C_MUX6_BUS0,  1);
mk_pca9548(mux7,  MP_I2C_MUX7_BUS0,  1);
mk_pca9548(mux8,  MP_I2C_MUX8_BUS0,  1);
mk_pca9548(mux9,  MP_I2C_MUX9_BUS0,  1);
mk_pca9548(mux10, MP_I2C_MUX10_BUS0, 1);
mk_pca9548(mux11, MP_I2C_MUX11_BUS0, 1);
mk_pca9548(mux12, MP_I2C_MUX12_BUS0, 1);
mk_pca9548(mux13, MP_I2C_MUX13_BUS0, 1);
mk_pca9548(mux14, MP_I2C_MUX14_BUS0, 1);
mk_pca9548(mux15, MP_I2C_MUX15_BUS0, 1);
mk_pca9548(mux16, MP_I2C_MUX16_BUS0, 1);
mk_pca9548(mux17, MP_I2C_MUX17_BUS0, 1);

mk_qsfp_port_eeprom(port1,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port2,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port3,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port4,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port5,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port6,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port7,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port8,  50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port9,  50, 256, SFF_8436_FLAG_IRUGO);
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
mk_qsfp_port_eeprom(port65, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port66, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port67, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port68, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port69, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port70, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port71, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port72, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port73, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port74, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port75, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port76, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port77, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port78, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port79, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port80, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port81, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port82, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port83, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port84, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port85, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port86, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port87, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port88, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port89, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port90, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port91, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port92, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port93, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port94, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port95, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port96, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port97, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port98, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port99, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port100, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port101, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port102, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port103, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port104, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port105, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port106, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port107, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port108, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port109, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port110, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port111, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port112, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port113, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port114, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port115, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port116, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port117, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port118, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port119, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port120, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port121, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port122, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port123, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port124, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port125, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port126, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port127, 50, 256, SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port128, 50, 256, SFF_8436_FLAG_IRUGO);

mk_eeprom(board, 57, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(pim1,  56, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(pim2,  56, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(pim3,  56, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(pim4,  56, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(pim5,  56, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(pim6,  56, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(pim7,  56, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(pim8,  56, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);

static struct platform_i2c_device_info i2c_devices[] = {
	/* devices on CP2112 bus */
	mk_i2cdev(MP_I2C_CP2112_BUS, "pca9548", 0x70, &mux1_platform_data),
	mk_i2cdev(MP_I2C_CP2112_BUS, "act_minipack_cpld", 0x3a, NULL),
	mk_i2cdev(MP_I2C_CP2112_BUS, "24c64",   0x57, &board_57_at24),

	/* devices on pca9548 1 */
	mk_i2cdev(MP_I2C_MUX1_BUS0, "pca9548", 0x71, &mux2_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS0, "pca9548", 0x72, &mux3_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS0, "24c64",   0x56, &pim1_56_at24),
	mk_i2cdev(MP_I2C_MUX1_BUS1, "pca9548", 0x71, &mux4_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS1, "pca9548", 0x72, &mux5_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS1, "24c64",   0x56, &pim2_56_at24),
	mk_i2cdev(MP_I2C_MUX1_BUS2, "pca9548", 0x71, &mux6_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS2, "pca9548", 0x72, &mux7_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS2, "24c64",   0x56, &pim3_56_at24),
	mk_i2cdev(MP_I2C_MUX1_BUS3, "pca9548", 0x71, &mux8_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS3, "pca9548", 0x72, &mux9_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS3, "24c64",   0x56, &pim4_56_at24),
	mk_i2cdev(MP_I2C_MUX1_BUS4, "pca9548", 0x71, &mux10_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS4, "pca9548", 0x72, &mux11_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS4, "24c64",   0x56, &pim5_56_at24),
	mk_i2cdev(MP_I2C_MUX1_BUS5, "pca9548", 0x71, &mux12_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS5, "pca9548", 0x72, &mux13_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS5, "24c64",   0x56, &pim6_56_at24),
	mk_i2cdev(MP_I2C_MUX1_BUS6, "pca9548", 0x71, &mux14_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS6, "pca9548", 0x72, &mux15_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS6, "24c64",   0x56, &pim7_56_at24),
	mk_i2cdev(MP_I2C_MUX1_BUS7, "pca9548", 0x71, &mux16_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS7, "pca9548", 0x72, &mux17_platform_data),
	mk_i2cdev(MP_I2C_MUX1_BUS7, "24c64",   0x56, &pim8_56_at24),

	/* devices on pca9548 2 */
	mk_i2cdev(MP_I2C_MUX2_BUS1, "sff8436", 0x50, &port16_50_sff8436),
	mk_i2cdev(MP_I2C_MUX2_BUS0, "sff8436", 0x50, &port15_50_sff8436),
	mk_i2cdev(MP_I2C_MUX2_BUS3, "sff8436", 0x50, &port14_50_sff8436),
	mk_i2cdev(MP_I2C_MUX2_BUS2, "sff8436", 0x50, &port13_50_sff8436),
	mk_i2cdev(MP_I2C_MUX2_BUS5, "sff8436", 0x50, &port12_50_sff8436),
	mk_i2cdev(MP_I2C_MUX2_BUS4, "sff8436", 0x50, &port11_50_sff8436),
	mk_i2cdev(MP_I2C_MUX2_BUS7, "sff8436", 0x50, &port10_50_sff8436),
	mk_i2cdev(MP_I2C_MUX2_BUS6, "sff8436", 0x50, &port9_50_sff8436),

	/* devices on pca9548 3 */
	mk_i2cdev(MP_I2C_MUX3_BUS1, "sff8436", 0x50, &port8_50_sff8436),
	mk_i2cdev(MP_I2C_MUX3_BUS0, "sff8436", 0x50, &port7_50_sff8436),
	mk_i2cdev(MP_I2C_MUX3_BUS3, "sff8436", 0x50, &port6_50_sff8436),
	mk_i2cdev(MP_I2C_MUX3_BUS2, "sff8436", 0x50, &port5_50_sff8436),
	mk_i2cdev(MP_I2C_MUX3_BUS5, "sff8436", 0x50, &port4_50_sff8436),
	mk_i2cdev(MP_I2C_MUX3_BUS4, "sff8436", 0x50, &port3_50_sff8436),
	mk_i2cdev(MP_I2C_MUX3_BUS7, "sff8436", 0x50, &port2_50_sff8436),
	mk_i2cdev(MP_I2C_MUX3_BUS6, "sff8436", 0x50, &port1_50_sff8436),

	/* devices on pca9548 4 */
	mk_i2cdev(MP_I2C_MUX4_BUS1, "sff8436", 0x50, &port32_50_sff8436),
	mk_i2cdev(MP_I2C_MUX4_BUS0, "sff8436", 0x50, &port31_50_sff8436),
	mk_i2cdev(MP_I2C_MUX4_BUS3, "sff8436", 0x50, &port30_50_sff8436),
	mk_i2cdev(MP_I2C_MUX4_BUS2, "sff8436", 0x50, &port29_50_sff8436),
	mk_i2cdev(MP_I2C_MUX4_BUS5, "sff8436", 0x50, &port28_50_sff8436),
	mk_i2cdev(MP_I2C_MUX4_BUS4, "sff8436", 0x50, &port27_50_sff8436),
	mk_i2cdev(MP_I2C_MUX4_BUS7, "sff8436", 0x50, &port26_50_sff8436),
	mk_i2cdev(MP_I2C_MUX4_BUS6, "sff8436", 0x50, &port25_50_sff8436),

	/* devices on pca9548 5 */
	mk_i2cdev(MP_I2C_MUX5_BUS1, "sff8436", 0x50, &port24_50_sff8436),
	mk_i2cdev(MP_I2C_MUX5_BUS0, "sff8436", 0x50, &port23_50_sff8436),
	mk_i2cdev(MP_I2C_MUX5_BUS3, "sff8436", 0x50, &port22_50_sff8436),
	mk_i2cdev(MP_I2C_MUX5_BUS2, "sff8436", 0x50, &port21_50_sff8436),
	mk_i2cdev(MP_I2C_MUX5_BUS5, "sff8436", 0x50, &port20_50_sff8436),
	mk_i2cdev(MP_I2C_MUX5_BUS4, "sff8436", 0x50, &port19_50_sff8436),
	mk_i2cdev(MP_I2C_MUX5_BUS7, "sff8436", 0x50, &port18_50_sff8436),
	mk_i2cdev(MP_I2C_MUX5_BUS6, "sff8436", 0x50, &port17_50_sff8436),

	/* devices on pca9548 6 */
	mk_i2cdev(MP_I2C_MUX6_BUS1, "sff8436", 0x50, &port48_50_sff8436),
	mk_i2cdev(MP_I2C_MUX6_BUS0, "sff8436", 0x50, &port47_50_sff8436),
	mk_i2cdev(MP_I2C_MUX6_BUS3, "sff8436", 0x50, &port46_50_sff8436),
	mk_i2cdev(MP_I2C_MUX6_BUS2, "sff8436", 0x50, &port45_50_sff8436),
	mk_i2cdev(MP_I2C_MUX6_BUS5, "sff8436", 0x50, &port44_50_sff8436),
	mk_i2cdev(MP_I2C_MUX6_BUS4, "sff8436", 0x50, &port43_50_sff8436),
	mk_i2cdev(MP_I2C_MUX6_BUS7, "sff8436", 0x50, &port42_50_sff8436),
	mk_i2cdev(MP_I2C_MUX6_BUS6, "sff8436", 0x50, &port41_50_sff8436),

	/* devices on pca9548 7 */
	mk_i2cdev(MP_I2C_MUX7_BUS1, "sff8436", 0x50, &port40_50_sff8436),
	mk_i2cdev(MP_I2C_MUX7_BUS0, "sff8436", 0x50, &port39_50_sff8436),
	mk_i2cdev(MP_I2C_MUX7_BUS3, "sff8436", 0x50, &port38_50_sff8436),
	mk_i2cdev(MP_I2C_MUX7_BUS2, "sff8436", 0x50, &port37_50_sff8436),
	mk_i2cdev(MP_I2C_MUX7_BUS5, "sff8436", 0x50, &port36_50_sff8436),
	mk_i2cdev(MP_I2C_MUX7_BUS4, "sff8436", 0x50, &port35_50_sff8436),
	mk_i2cdev(MP_I2C_MUX7_BUS7, "sff8436", 0x50, &port34_50_sff8436),
	mk_i2cdev(MP_I2C_MUX7_BUS6, "sff8436", 0x50, &port33_50_sff8436),

	/* devices on pca9548 8 */
	mk_i2cdev(MP_I2C_MUX8_BUS1, "sff8436", 0x50, &port64_50_sff8436),
	mk_i2cdev(MP_I2C_MUX8_BUS0, "sff8436", 0x50, &port63_50_sff8436),
	mk_i2cdev(MP_I2C_MUX8_BUS3, "sff8436", 0x50, &port62_50_sff8436),
	mk_i2cdev(MP_I2C_MUX8_BUS2, "sff8436", 0x50, &port61_50_sff8436),
	mk_i2cdev(MP_I2C_MUX8_BUS5, "sff8436", 0x50, &port60_50_sff8436),
	mk_i2cdev(MP_I2C_MUX8_BUS4, "sff8436", 0x50, &port59_50_sff8436),
	mk_i2cdev(MP_I2C_MUX8_BUS7, "sff8436", 0x50, &port58_50_sff8436),
	mk_i2cdev(MP_I2C_MUX8_BUS6, "sff8436", 0x50, &port57_50_sff8436),

	/* devices on pca9548 9 */
	mk_i2cdev(MP_I2C_MUX9_BUS1, "sff8436", 0x50, &port56_50_sff8436),
	mk_i2cdev(MP_I2C_MUX9_BUS0, "sff8436", 0x50, &port55_50_sff8436),
	mk_i2cdev(MP_I2C_MUX9_BUS3, "sff8436", 0x50, &port54_50_sff8436),
	mk_i2cdev(MP_I2C_MUX9_BUS2, "sff8436", 0x50, &port53_50_sff8436),
	mk_i2cdev(MP_I2C_MUX9_BUS5, "sff8436", 0x50, &port52_50_sff8436),
	mk_i2cdev(MP_I2C_MUX9_BUS4, "sff8436", 0x50, &port51_50_sff8436),
	mk_i2cdev(MP_I2C_MUX9_BUS7, "sff8436", 0x50, &port50_50_sff8436),
	mk_i2cdev(MP_I2C_MUX9_BUS6, "sff8436", 0x50, &port49_50_sff8436),

	/* devices on pca9548 10 */
	mk_i2cdev(MP_I2C_MUX10_BUS1, "sff8436", 0x50, &port80_50_sff8436),
	mk_i2cdev(MP_I2C_MUX10_BUS0, "sff8436", 0x50, &port79_50_sff8436),
	mk_i2cdev(MP_I2C_MUX10_BUS3, "sff8436", 0x50, &port78_50_sff8436),
	mk_i2cdev(MP_I2C_MUX10_BUS2, "sff8436", 0x50, &port77_50_sff8436),
	mk_i2cdev(MP_I2C_MUX10_BUS5, "sff8436", 0x50, &port76_50_sff8436),
	mk_i2cdev(MP_I2C_MUX10_BUS4, "sff8436", 0x50, &port75_50_sff8436),
	mk_i2cdev(MP_I2C_MUX10_BUS7, "sff8436", 0x50, &port74_50_sff8436),
	mk_i2cdev(MP_I2C_MUX10_BUS6, "sff8436", 0x50, &port73_50_sff8436),

	/* devices on pca9548 11 */
	mk_i2cdev(MP_I2C_MUX11_BUS1, "sff8436", 0x50, &port72_50_sff8436),
	mk_i2cdev(MP_I2C_MUX11_BUS0, "sff8436", 0x50, &port71_50_sff8436),
	mk_i2cdev(MP_I2C_MUX11_BUS3, "sff8436", 0x50, &port70_50_sff8436),
	mk_i2cdev(MP_I2C_MUX11_BUS2, "sff8436", 0x50, &port69_50_sff8436),
	mk_i2cdev(MP_I2C_MUX11_BUS5, "sff8436", 0x50, &port68_50_sff8436),
	mk_i2cdev(MP_I2C_MUX11_BUS4, "sff8436", 0x50, &port67_50_sff8436),
	mk_i2cdev(MP_I2C_MUX11_BUS7, "sff8436", 0x50, &port66_50_sff8436),
	mk_i2cdev(MP_I2C_MUX11_BUS6, "sff8436", 0x50, &port65_50_sff8436),

	/* devices on pca9548 12 */
	mk_i2cdev(MP_I2C_MUX12_BUS1, "sff8436", 0x50, &port96_50_sff8436),
	mk_i2cdev(MP_I2C_MUX12_BUS0, "sff8436", 0x50, &port95_50_sff8436),
	mk_i2cdev(MP_I2C_MUX12_BUS3, "sff8436", 0x50, &port94_50_sff8436),
	mk_i2cdev(MP_I2C_MUX12_BUS2, "sff8436", 0x50, &port93_50_sff8436),
	mk_i2cdev(MP_I2C_MUX12_BUS5, "sff8436", 0x50, &port92_50_sff8436),
	mk_i2cdev(MP_I2C_MUX12_BUS4, "sff8436", 0x50, &port91_50_sff8436),
	mk_i2cdev(MP_I2C_MUX12_BUS7, "sff8436", 0x50, &port90_50_sff8436),
	mk_i2cdev(MP_I2C_MUX12_BUS6, "sff8436", 0x50, &port89_50_sff8436),

	/* devices on pca9548 13 */
	mk_i2cdev(MP_I2C_MUX13_BUS1, "sff8436", 0x50, &port88_50_sff8436),
	mk_i2cdev(MP_I2C_MUX13_BUS0, "sff8436", 0x50, &port87_50_sff8436),
	mk_i2cdev(MP_I2C_MUX13_BUS3, "sff8436", 0x50, &port86_50_sff8436),
	mk_i2cdev(MP_I2C_MUX13_BUS2, "sff8436", 0x50, &port85_50_sff8436),
	mk_i2cdev(MP_I2C_MUX13_BUS5, "sff8436", 0x50, &port84_50_sff8436),
	mk_i2cdev(MP_I2C_MUX13_BUS4, "sff8436", 0x50, &port83_50_sff8436),
	mk_i2cdev(MP_I2C_MUX13_BUS7, "sff8436", 0x50, &port82_50_sff8436),
	mk_i2cdev(MP_I2C_MUX13_BUS6, "sff8436", 0x50, &port81_50_sff8436),

	/* devices on pca9548 14 */
	mk_i2cdev(MP_I2C_MUX14_BUS1, "sff8436", 0x50, &port112_50_sff8436),
	mk_i2cdev(MP_I2C_MUX14_BUS0, "sff8436", 0x50, &port111_50_sff8436),
	mk_i2cdev(MP_I2C_MUX14_BUS3, "sff8436", 0x50, &port110_50_sff8436),
	mk_i2cdev(MP_I2C_MUX14_BUS2, "sff8436", 0x50, &port109_50_sff8436),
	mk_i2cdev(MP_I2C_MUX14_BUS5, "sff8436", 0x50, &port108_50_sff8436),
	mk_i2cdev(MP_I2C_MUX14_BUS4, "sff8436", 0x50, &port107_50_sff8436),
	mk_i2cdev(MP_I2C_MUX14_BUS7, "sff8436", 0x50, &port106_50_sff8436),
	mk_i2cdev(MP_I2C_MUX14_BUS6, "sff8436", 0x50, &port105_50_sff8436),

	/* devices on pca9548 15 */
	mk_i2cdev(MP_I2C_MUX15_BUS1, "sff8436", 0x50, &port104_50_sff8436),
	mk_i2cdev(MP_I2C_MUX15_BUS0, "sff8436", 0x50, &port103_50_sff8436),
	mk_i2cdev(MP_I2C_MUX15_BUS3, "sff8436", 0x50, &port102_50_sff8436),
	mk_i2cdev(MP_I2C_MUX15_BUS2, "sff8436", 0x50, &port101_50_sff8436),
	mk_i2cdev(MP_I2C_MUX15_BUS5, "sff8436", 0x50, &port100_50_sff8436),
	mk_i2cdev(MP_I2C_MUX15_BUS4, "sff8436", 0x50, &port99_50_sff8436),
	mk_i2cdev(MP_I2C_MUX15_BUS7, "sff8436", 0x50, &port98_50_sff8436),
	mk_i2cdev(MP_I2C_MUX15_BUS6, "sff8436", 0x50, &port97_50_sff8436),

	/* devices on pca9548 16 */
	mk_i2cdev(MP_I2C_MUX16_BUS1, "sff8436", 0x50, &port128_50_sff8436),
	mk_i2cdev(MP_I2C_MUX16_BUS0, "sff8436", 0x50, &port127_50_sff8436),
	mk_i2cdev(MP_I2C_MUX16_BUS3, "sff8436", 0x50, &port126_50_sff8436),
	mk_i2cdev(MP_I2C_MUX16_BUS2, "sff8436", 0x50, &port125_50_sff8436),
	mk_i2cdev(MP_I2C_MUX16_BUS5, "sff8436", 0x50, &port124_50_sff8436),
	mk_i2cdev(MP_I2C_MUX16_BUS4, "sff8436", 0x50, &port123_50_sff8436),
	mk_i2cdev(MP_I2C_MUX16_BUS7, "sff8436", 0x50, &port122_50_sff8436),
	mk_i2cdev(MP_I2C_MUX16_BUS6, "sff8436", 0x50, &port121_50_sff8436),

	/* devices on pca9548 17 */
	mk_i2cdev(MP_I2C_MUX17_BUS1, "sff8436", 0x50, &port120_50_sff8436),
	mk_i2cdev(MP_I2C_MUX17_BUS0, "sff8436", 0x50, &port119_50_sff8436),
	mk_i2cdev(MP_I2C_MUX17_BUS3, "sff8436", 0x50, &port118_50_sff8436),
	mk_i2cdev(MP_I2C_MUX17_BUS2, "sff8436", 0x50, &port117_50_sff8436),
	mk_i2cdev(MP_I2C_MUX17_BUS5, "sff8436", 0x50, &port116_50_sff8436),
	mk_i2cdev(MP_I2C_MUX17_BUS4, "sff8436", 0x50, &port115_50_sff8436),
	mk_i2cdev(MP_I2C_MUX17_BUS7, "sff8436", 0x50, &port114_50_sff8436),
	mk_i2cdev(MP_I2C_MUX17_BUS6, "sff8436", 0x50, &port113_50_sff8436),
};

static const char * const led_status_values[] = {
	"off",
	"on",
};

static const char * const led_flash_values[] = {
	"disable",
	"enable",
};

static const char * const led_profile_values[] = {
	"white",
	"cyan",
	"blue",
	"pink",
	"red",
	"orange",
	"yellow",
	"green",
};

/* Define all the bitfields */
mk_bf_ro32(fpga, device_id,  0x0000, 24,  8, NULL, 0);
mk_bf_ro32(fpga, major_rev,  0x0000, 16,  8, NULL, 0);
mk_bf_ro32(fpga, minor_rev,  0x0000, 8,   8, NULL, 0);
mk_bf_ro32(fpga, board_id,   0x0000, 0,   8, NULL, 0);
mk_bf_rw32(fpga, scratch,    0x0004, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim_present_status, 0x0040, 16,  8, NULL, 0);

/* plumb all of the front panel port signals */

mk_bf_rw32(fpga, qsfp28_pim1_lpmode,     0x40078, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim1_modsel,     0x40040, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim1_reset,      0x40070, 0,  16, NULL, 0);
mk_bf_ro32(fpga, qsfp28_pim1_present,    0x40048, 0,  16, NULL, 0);

mk_bf_rw32(fpga, qsfp28_pim2_lpmode,     0x48078, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim2_modsel,     0x48040, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim2_reset,      0x48070, 0,  16, NULL, 0);
mk_bf_ro32(fpga, qsfp28_pim2_present,    0x48048, 0,  16, NULL, 0);

mk_bf_rw32(fpga, qsfp28_pim3_lpmode,     0x50078, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim3_modsel,     0x50040, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim3_reset,      0x50070, 0,  16, NULL, 0);
mk_bf_ro32(fpga, qsfp28_pim3_present,    0x50048, 0,  16, NULL, 0);

mk_bf_rw32(fpga, qsfp28_pim4_lpmode,     0x58078, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim4_modsel,     0x58040, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim4_reset,      0x58070, 0,  16, NULL, 0);
mk_bf_ro32(fpga, qsfp28_pim4_present,    0x58048, 0,  16, NULL, 0);

mk_bf_rw32(fpga, qsfp28_pim5_lpmode,     0x60078, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim5_modsel,     0x60040, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim5_reset,      0x60070, 0,  16, NULL, 0);
mk_bf_ro32(fpga, qsfp28_pim5_present,    0x60048, 0,  16, NULL, 0);

mk_bf_rw32(fpga, qsfp28_pim6_lpmode,     0x68078, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim6_modsel,     0x68040, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim6_reset,      0x68070, 0,  16, NULL, 0);
mk_bf_ro32(fpga, qsfp28_pim6_present,    0x68048, 0,  16, NULL, 0);

mk_bf_rw32(fpga, qsfp28_pim7_lpmode,     0x70078, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim7_modsel,     0x70040, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim7_reset,      0x70070, 0,  16, NULL, 0);
mk_bf_ro32(fpga, qsfp28_pim7_present,    0x70048, 0,  16, NULL, 0);

mk_bf_rw32(fpga, qsfp28_pim8_lpmode,     0x78078, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim8_modsel,     0x78040, 0,  16, NULL, 0);
mk_bf_rw32(fpga, qsfp28_pim8_reset,      0x78070, 0,  16, NULL, 0);
mk_bf_ro32(fpga, qsfp28_pim8_present,    0x78048, 0,  16, NULL, 0);

mk_bf_rw32(fpga, pim1_revision,	  PIM1_REVISION, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim2_revision,	  PIM2_REVISION, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim3_revision,	  PIM3_REVISION, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim4_revision,	  PIM4_REVISION, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim5_revision,	  PIM5_REVISION, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim6_revision,	  PIM6_REVISION, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim7_revision,	  PIM7_REVISION, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim8_revision,	  PIM8_REVISION, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim1_system_led, PIM1_SYSTEM_LED, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim2_system_led, PIM2_SYSTEM_LED, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim3_system_led, PIM3_SYSTEM_LED, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim4_system_led, PIM4_SYSTEM_LED, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim5_system_led, PIM5_SYSTEM_LED, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim6_system_led, PIM6_SYSTEM_LED, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim7_system_led, PIM7_SYSTEM_LED, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim8_system_led, PIM8_SYSTEM_LED, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim1_uptime,	  PIM1_UP_TIME, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim2_uptime,	  PIM2_UP_TIME, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim3_uptime,	  PIM3_UP_TIME, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim4_uptime,	  PIM4_UP_TIME, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim5_uptime,	  PIM5_UP_TIME, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim6_uptime,	  PIM6_UP_TIME, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim7_uptime,	  PIM7_UP_TIME, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim8_uptime,	  PIM8_UP_TIME, 0,  32, NULL, 0);

mk_bf_rw32(fpga, pim1_mdio_sel,	  PIM1_MDIO_SOURCE, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim2_mdio_sel,	  PIM2_MDIO_SOURCE, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim3_mdio_sel,	  PIM3_MDIO_SOURCE, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim4_mdio_sel,	  PIM4_MDIO_SOURCE, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim5_mdio_sel,	  PIM5_MDIO_SOURCE, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim6_mdio_sel,	  PIM6_MDIO_SOURCE, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim7_mdio_sel,	  PIM7_MDIO_SOURCE, 0,  32, NULL, 0);
mk_bf_rw32(fpga, pim8_mdio_sel,	  PIM8_MDIO_SOURCE, 0,  32, NULL, 0);

mk_bf_rw32(fpga, pim1_led_control, PIM1_LED_CONTROL, 4,  3, NULL, 0);
mk_bf_rw32(fpga, pim2_led_control, PIM2_LED_CONTROL, 4,  3, NULL, 0);
mk_bf_rw32(fpga, pim3_led_control, PIM3_LED_CONTROL, 4,  3, NULL, 0);
mk_bf_rw32(fpga, pim4_led_control, PIM4_LED_CONTROL, 4,  3, NULL, 0);
mk_bf_rw32(fpga, pim5_led_control, PIM5_LED_CONTROL, 4,  3, NULL, 0);
mk_bf_rw32(fpga, pim6_led_control, PIM6_LED_CONTROL, 4,  3, NULL, 0);
mk_bf_rw32(fpga, pim7_led_control, PIM7_LED_CONTROL, 4,  3, NULL, 0);
mk_bf_rw32(fpga, pim8_led_control, PIM8_LED_CONTROL, 4,  3, NULL, 0);

mk_bf_rw32(fpga, pim1_port1_led_status,      PIM1_PORT1_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port1_led_flash,       PIM1_PORT1_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port1_led_profile,     PIM1_PORT1_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port2_led_status,      PIM1_PORT2_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port2_led_flash,       PIM1_PORT2_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port2_led_profile,     PIM1_PORT2_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port3_led_status,      PIM1_PORT3_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port3_led_flash,       PIM1_PORT3_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port3_led_profile,     PIM1_PORT3_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port4_led_status,      PIM1_PORT4_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port4_led_flash,       PIM1_PORT4_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port4_led_profile,     PIM1_PORT4_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port5_led_status,      PIM1_PORT5_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port5_led_flash,       PIM1_PORT5_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port5_led_profile,     PIM1_PORT5_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port6_led_status,      PIM1_PORT6_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port6_led_flash,       PIM1_PORT6_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port6_led_profile,     PIM1_PORT6_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port7_led_status,      PIM1_PORT7_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port7_led_flash,       PIM1_PORT7_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port7_led_profile,     PIM1_PORT7_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port8_led_status,      PIM1_PORT8_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port8_led_flash,       PIM1_PORT8_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port8_led_profile,     PIM1_PORT8_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port9_led_status,      PIM1_PORT9_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port9_led_flash,       PIM1_PORT9_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port9_led_profile,     PIM1_PORT9_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port10_led_status,     PIM1_PORT10_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port10_led_flash,      PIM1_PORT10_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port10_led_profile,    PIM1_PORT10_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port11_led_status,     PIM1_PORT11_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port11_led_flash,      PIM1_PORT11_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port11_led_profile,    PIM1_PORT11_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port12_led_status,     PIM1_PORT12_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port12_led_flash,      PIM1_PORT12_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port12_led_profile,    PIM1_PORT12_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port13_led_status,     PIM1_PORT13_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port13_led_flash,      PIM1_PORT13_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port13_led_profile,    PIM1_PORT13_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port14_led_status,     PIM1_PORT14_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port14_led_flash,      PIM1_PORT14_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port14_led_profile,    PIM1_PORT14_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port15_led_status,     PIM1_PORT15_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port15_led_flash,      PIM1_PORT15_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port15_led_profile,    PIM1_PORT15_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim1_port16_led_status,     PIM1_PORT16_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim1_port16_led_flash,      PIM1_PORT16_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim1_port16_led_profile,    PIM1_PORT16_LED_CONTROL, 2,  3, led_profile_values, 0);

mk_bf_rw32(fpga, pim2_port1_led_status,      PIM2_PORT1_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port1_led_flash,       PIM2_PORT1_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port1_led_profile,     PIM2_PORT1_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port2_led_status,      PIM2_PORT2_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port2_led_flash,       PIM2_PORT2_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port2_led_profile,     PIM2_PORT2_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port3_led_status,      PIM2_PORT3_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port3_led_flash,       PIM2_PORT3_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port3_led_profile,     PIM2_PORT3_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port4_led_status,      PIM2_PORT4_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port4_led_flash,       PIM2_PORT4_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port4_led_profile,     PIM2_PORT4_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port5_led_status,      PIM2_PORT5_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port5_led_flash,       PIM2_PORT5_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port5_led_profile,     PIM2_PORT5_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port6_led_status,      PIM2_PORT6_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port6_led_flash,       PIM2_PORT6_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port6_led_profile,     PIM2_PORT6_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port7_led_status,      PIM2_PORT7_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port7_led_flash,       PIM2_PORT7_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port7_led_profile,     PIM2_PORT7_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port8_led_status,      PIM2_PORT8_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port8_led_flash,       PIM2_PORT8_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port8_led_profile,     PIM2_PORT8_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port9_led_status,      PIM2_PORT9_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port9_led_flash,       PIM2_PORT9_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port9_led_profile,     PIM2_PORT9_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port10_led_status,     PIM2_PORT10_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port10_led_flash,      PIM2_PORT10_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port10_led_profile,    PIM2_PORT10_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port11_led_status,     PIM2_PORT11_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port11_led_flash,      PIM2_PORT11_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port11_led_profile,    PIM2_PORT11_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port12_led_status,     PIM2_PORT12_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port12_led_flash,      PIM2_PORT12_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port12_led_profile,    PIM2_PORT12_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port13_led_status,     PIM2_PORT13_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port13_led_flash,      PIM2_PORT13_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port13_led_profile,    PIM2_PORT13_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port14_led_status,     PIM2_PORT14_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port14_led_flash,      PIM2_PORT14_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port14_led_profile,    PIM2_PORT14_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port15_led_status,     PIM2_PORT15_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port15_led_flash,      PIM2_PORT15_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port15_led_profile,    PIM2_PORT15_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim2_port16_led_status,     PIM2_PORT16_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim2_port16_led_flash,      PIM2_PORT16_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim2_port16_led_profile,    PIM2_PORT16_LED_CONTROL, 2,  3, led_profile_values, 0);

mk_bf_rw32(fpga, pim3_port1_led_status,      PIM3_PORT1_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port1_led_flash,       PIM3_PORT1_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port1_led_profile,     PIM3_PORT1_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port2_led_status,      PIM3_PORT2_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port2_led_flash,       PIM3_PORT2_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port2_led_profile,     PIM3_PORT2_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port3_led_status,      PIM3_PORT3_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port3_led_flash,       PIM3_PORT3_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port3_led_profile,     PIM3_PORT3_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port4_led_status,      PIM3_PORT4_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port4_led_flash,       PIM3_PORT4_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port4_led_profile,     PIM3_PORT4_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port5_led_status,      PIM3_PORT5_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port5_led_flash,       PIM3_PORT5_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port5_led_profile,     PIM3_PORT5_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port6_led_status,      PIM3_PORT6_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port6_led_flash,       PIM3_PORT6_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port6_led_profile,     PIM3_PORT6_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port7_led_status,      PIM3_PORT7_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port7_led_flash,       PIM3_PORT7_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port7_led_profile,     PIM3_PORT7_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port8_led_status,      PIM3_PORT8_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port8_led_flash,       PIM3_PORT8_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port8_led_profile,     PIM3_PORT8_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port9_led_status,      PIM3_PORT9_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port9_led_flash,       PIM3_PORT9_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port9_led_profile,     PIM3_PORT9_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port10_led_status,     PIM3_PORT10_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port10_led_flash,      PIM3_PORT10_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port10_led_profile,    PIM3_PORT10_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port11_led_status,     PIM3_PORT11_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port11_led_flash,      PIM3_PORT11_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port11_led_profile,    PIM3_PORT11_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port12_led_status,     PIM3_PORT12_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port12_led_flash,      PIM3_PORT12_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port12_led_profile,    PIM3_PORT12_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port13_led_status,     PIM3_PORT13_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port13_led_flash,      PIM3_PORT13_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port13_led_profile,    PIM3_PORT13_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port14_led_status,     PIM3_PORT14_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port14_led_flash,      PIM3_PORT14_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port14_led_profile,    PIM3_PORT14_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port15_led_status,     PIM3_PORT15_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port15_led_flash,      PIM3_PORT15_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port15_led_profile,    PIM3_PORT15_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim3_port16_led_status,     PIM3_PORT16_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim3_port16_led_flash,      PIM3_PORT16_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim3_port16_led_profile,    PIM3_PORT16_LED_CONTROL, 2,  3, led_profile_values, 0);

mk_bf_rw32(fpga, pim4_port1_led_status,      PIM4_PORT1_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port1_led_flash,       PIM4_PORT1_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port1_led_profile,     PIM4_PORT1_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port2_led_status,      PIM4_PORT2_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port2_led_flash,       PIM4_PORT2_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port2_led_profile,     PIM4_PORT2_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port3_led_status,      PIM4_PORT3_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port3_led_flash,       PIM4_PORT3_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port3_led_profile,     PIM4_PORT3_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port4_led_status,      PIM4_PORT4_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port4_led_flash,       PIM4_PORT4_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port4_led_profile,     PIM4_PORT4_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port5_led_status,      PIM4_PORT5_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port5_led_flash,       PIM4_PORT5_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port5_led_profile,     PIM4_PORT5_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port6_led_status,      PIM4_PORT6_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port6_led_flash,       PIM4_PORT6_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port6_led_profile,     PIM4_PORT6_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port7_led_status,      PIM4_PORT7_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port7_led_flash,       PIM4_PORT7_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port7_led_profile,     PIM4_PORT7_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port8_led_status,      PIM4_PORT8_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port8_led_flash,       PIM4_PORT8_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port8_led_profile,     PIM4_PORT8_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port9_led_status,      PIM4_PORT9_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port9_led_flash,       PIM4_PORT9_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port9_led_profile,     PIM4_PORT9_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port10_led_status,     PIM4_PORT10_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port10_led_flash,      PIM4_PORT10_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port10_led_profile,    PIM4_PORT10_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port11_led_status,     PIM4_PORT11_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port11_led_flash,      PIM4_PORT11_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port11_led_profile,    PIM4_PORT11_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port12_led_status,     PIM4_PORT12_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port12_led_flash,      PIM4_PORT12_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port12_led_profile,    PIM4_PORT12_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port13_led_status,     PIM4_PORT13_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port13_led_flash,      PIM4_PORT13_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port13_led_profile,    PIM4_PORT13_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port14_led_status,     PIM4_PORT14_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port14_led_flash,      PIM4_PORT14_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port14_led_profile,    PIM4_PORT14_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port15_led_status,     PIM4_PORT15_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port15_led_flash,      PIM4_PORT15_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port15_led_profile,    PIM4_PORT15_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim4_port16_led_status,     PIM4_PORT16_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim4_port16_led_flash,      PIM4_PORT16_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim4_port16_led_profile,    PIM4_PORT16_LED_CONTROL, 2,  3, led_profile_values, 0);

mk_bf_rw32(fpga, pim5_port1_led_status,      PIM5_PORT1_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port1_led_flash,       PIM5_PORT1_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port1_led_profile,     PIM5_PORT1_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port2_led_status,      PIM5_PORT2_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port2_led_flash,       PIM5_PORT2_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port2_led_profile,     PIM5_PORT2_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port3_led_status,      PIM5_PORT3_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port3_led_flash,       PIM5_PORT3_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port3_led_profile,     PIM5_PORT3_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port4_led_status,      PIM5_PORT4_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port4_led_flash,       PIM5_PORT4_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port4_led_profile,     PIM5_PORT4_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port5_led_status,      PIM5_PORT5_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port5_led_flash,       PIM5_PORT5_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port5_led_profile,     PIM5_PORT5_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port6_led_status,      PIM5_PORT6_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port6_led_flash,       PIM5_PORT6_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port6_led_profile,     PIM5_PORT6_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port7_led_status,      PIM5_PORT7_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port7_led_flash,       PIM5_PORT7_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port7_led_profile,     PIM5_PORT7_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port8_led_status,      PIM5_PORT8_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port8_led_flash,       PIM5_PORT8_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port8_led_profile,     PIM5_PORT8_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port9_led_status,      PIM5_PORT9_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port9_led_flash,       PIM5_PORT9_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port9_led_profile,     PIM5_PORT9_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port10_led_status,     PIM5_PORT10_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port10_led_flash,      PIM5_PORT10_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port10_led_profile,    PIM5_PORT10_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port11_led_status,     PIM5_PORT11_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port11_led_flash,      PIM5_PORT11_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port11_led_profile,    PIM5_PORT11_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port12_led_status,     PIM5_PORT12_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port12_led_flash,      PIM5_PORT12_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port12_led_profile,    PIM5_PORT12_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port13_led_status,     PIM5_PORT13_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port13_led_flash,      PIM5_PORT13_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port13_led_profile,    PIM5_PORT13_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port14_led_status,     PIM5_PORT14_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port14_led_flash,      PIM5_PORT14_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port14_led_profile,    PIM5_PORT14_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port15_led_status,     PIM5_PORT15_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port15_led_flash,      PIM5_PORT15_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port15_led_profile,    PIM5_PORT15_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim5_port16_led_status,     PIM5_PORT16_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim5_port16_led_flash,      PIM5_PORT16_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim5_port16_led_profile,    PIM5_PORT16_LED_CONTROL, 2,  3, led_profile_values, 0);

mk_bf_rw32(fpga, pim6_port1_led_status,      PIM6_PORT1_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port1_led_flash,       PIM6_PORT1_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port1_led_profile,     PIM6_PORT1_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port2_led_status,      PIM6_PORT2_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port2_led_flash,       PIM6_PORT2_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port2_led_profile,     PIM6_PORT2_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port3_led_status,      PIM6_PORT3_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port3_led_flash,       PIM6_PORT3_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port3_led_profile,     PIM6_PORT3_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port4_led_status,      PIM6_PORT4_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port4_led_flash,       PIM6_PORT4_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port4_led_profile,     PIM6_PORT4_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port5_led_status,      PIM6_PORT5_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port5_led_flash,       PIM6_PORT5_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port5_led_profile,     PIM6_PORT5_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port6_led_status,      PIM6_PORT6_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port6_led_flash,       PIM6_PORT6_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port6_led_profile,     PIM6_PORT6_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port7_led_status,      PIM6_PORT7_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port7_led_flash,       PIM6_PORT7_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port7_led_profile,     PIM6_PORT7_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port8_led_status,      PIM6_PORT8_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port8_led_flash,       PIM6_PORT8_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port8_led_profile,     PIM6_PORT8_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port9_led_status,      PIM6_PORT9_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port9_led_flash,       PIM6_PORT9_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port9_led_profile,     PIM6_PORT9_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port10_led_status,     PIM6_PORT10_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port10_led_flash,      PIM6_PORT10_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port10_led_profile,    PIM6_PORT10_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port11_led_status,     PIM6_PORT11_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port11_led_flash,      PIM6_PORT11_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port11_led_profile,    PIM6_PORT11_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port12_led_status,     PIM6_PORT12_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port12_led_flash,      PIM6_PORT12_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port12_led_profile,    PIM6_PORT12_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port13_led_status,     PIM6_PORT13_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port13_led_flash,      PIM6_PORT13_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port13_led_profile,    PIM6_PORT13_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port14_led_status,     PIM6_PORT14_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port14_led_flash,      PIM6_PORT14_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port14_led_profile,    PIM6_PORT14_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port15_led_status,     PIM6_PORT15_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port15_led_flash,      PIM6_PORT15_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port15_led_profile,    PIM6_PORT15_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim6_port16_led_status,     PIM6_PORT16_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim6_port16_led_flash,      PIM6_PORT16_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim6_port16_led_profile,    PIM6_PORT16_LED_CONTROL, 2,  3, led_profile_values, 0);

mk_bf_rw32(fpga, pim7_port1_led_status,      PIM7_PORT1_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port1_led_flash,       PIM7_PORT1_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port1_led_profile,     PIM7_PORT1_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port2_led_status,      PIM7_PORT2_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port2_led_flash,       PIM7_PORT2_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port2_led_profile,     PIM7_PORT2_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port3_led_status,      PIM7_PORT3_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port3_led_flash,       PIM7_PORT3_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port3_led_profile,     PIM7_PORT3_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port4_led_status,      PIM7_PORT4_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port4_led_flash,       PIM7_PORT4_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port4_led_profile,     PIM7_PORT4_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port5_led_status,      PIM7_PORT5_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port5_led_flash,       PIM7_PORT5_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port5_led_profile,     PIM7_PORT5_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port6_led_status,      PIM7_PORT6_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port6_led_flash,       PIM7_PORT6_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port6_led_profile,     PIM7_PORT6_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port7_led_status,      PIM7_PORT7_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port7_led_flash,       PIM7_PORT7_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port7_led_profile,     PIM7_PORT7_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port8_led_status,      PIM7_PORT8_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port8_led_flash,       PIM7_PORT8_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port8_led_profile,     PIM7_PORT8_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port9_led_status,      PIM7_PORT9_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port9_led_flash,       PIM7_PORT9_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port9_led_profile,     PIM7_PORT9_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port10_led_status,     PIM7_PORT10_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port10_led_flash,      PIM7_PORT10_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port10_led_profile,    PIM7_PORT10_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port11_led_status,     PIM7_PORT11_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port11_led_flash,      PIM7_PORT11_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port11_led_profile,    PIM7_PORT11_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port12_led_status,     PIM7_PORT12_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port12_led_flash,      PIM7_PORT12_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port12_led_profile,    PIM7_PORT12_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port13_led_status,     PIM7_PORT13_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port13_led_flash,      PIM7_PORT13_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port13_led_profile,    PIM7_PORT13_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port14_led_status,     PIM7_PORT14_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port14_led_flash,      PIM7_PORT14_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port14_led_profile,    PIM7_PORT14_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port15_led_status,     PIM7_PORT15_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port15_led_flash,      PIM7_PORT15_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port15_led_profile,    PIM7_PORT15_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim7_port16_led_status,     PIM7_PORT16_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim7_port16_led_flash,      PIM7_PORT16_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim7_port16_led_profile,    PIM7_PORT16_LED_CONTROL, 2,  3, led_profile_values, 0);

mk_bf_rw32(fpga, pim8_port1_led_status,      PIM8_PORT1_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port1_led_flash,       PIM8_PORT1_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port1_led_profile,     PIM8_PORT1_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port2_led_status,      PIM8_PORT2_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port2_led_flash,       PIM8_PORT2_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port2_led_profile,     PIM8_PORT2_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port3_led_status,      PIM8_PORT3_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port3_led_flash,       PIM8_PORT3_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port3_led_profile,     PIM8_PORT3_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port4_led_status,      PIM8_PORT4_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port4_led_flash,       PIM8_PORT4_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port4_led_profile,     PIM8_PORT4_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port5_led_status,      PIM8_PORT5_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port5_led_flash,       PIM8_PORT5_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port5_led_profile,     PIM8_PORT5_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port6_led_status,      PIM8_PORT6_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port6_led_flash,       PIM8_PORT6_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port6_led_profile,     PIM8_PORT6_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port7_led_status,      PIM8_PORT7_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port7_led_flash,       PIM8_PORT7_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port7_led_profile,     PIM8_PORT7_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port8_led_status,      PIM8_PORT8_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port8_led_flash,       PIM8_PORT8_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port8_led_profile,     PIM8_PORT8_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port9_led_status,      PIM8_PORT9_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port9_led_flash,       PIM8_PORT9_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port9_led_profile,     PIM8_PORT9_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port10_led_status,     PIM8_PORT10_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port10_led_flash,      PIM8_PORT10_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port10_led_profile,    PIM8_PORT10_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port11_led_status,     PIM8_PORT11_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port11_led_flash,      PIM8_PORT11_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port11_led_profile,    PIM8_PORT11_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port12_led_status,     PIM8_PORT12_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port12_led_flash,      PIM8_PORT12_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port12_led_profile,    PIM8_PORT12_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port13_led_status,     PIM8_PORT13_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port13_led_flash,      PIM8_PORT13_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port13_led_profile,    PIM8_PORT13_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port14_led_status,     PIM8_PORT14_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port14_led_flash,      PIM8_PORT14_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port14_led_profile,    PIM8_PORT14_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port15_led_status,     PIM8_PORT15_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port15_led_flash,      PIM8_PORT15_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port15_led_profile,    PIM8_PORT15_LED_CONTROL, 2,  3, led_profile_values, 0);
mk_bf_rw32(fpga, pim8_port16_led_status,     PIM8_PORT16_LED_CONTROL, 0,  1, led_status_values, 0);
mk_bf_rw32(fpga, pim8_port16_led_flash,      PIM8_PORT16_LED_CONTROL, 1,  1, led_flash_values, 0);
mk_bf_rw32(fpga, pim8_port16_led_profile,    PIM8_PORT16_LED_CONTROL, 2,  3, led_profile_values, 0);

static struct attribute *fpga_attrs[] = {
	&fpga_device_id.attr,
	&fpga_major_rev.attr,
	&fpga_minor_rev.attr,
	&fpga_board_id.attr,
	&fpga_scratch.attr,
	&fpga_pim_present_status.attr,
	&fpga_qsfp28_pim1_lpmode.attr,
	&fpga_qsfp28_pim1_modsel.attr,
	&fpga_qsfp28_pim1_reset.attr,
	&fpga_qsfp28_pim1_present.attr,
	&fpga_qsfp28_pim2_lpmode.attr,
	&fpga_qsfp28_pim2_modsel.attr,
	&fpga_qsfp28_pim2_reset.attr,
	&fpga_qsfp28_pim2_present.attr,
	&fpga_qsfp28_pim3_lpmode.attr,
	&fpga_qsfp28_pim3_modsel.attr,
	&fpga_qsfp28_pim3_reset.attr,
	&fpga_qsfp28_pim3_present.attr,
	&fpga_qsfp28_pim4_lpmode.attr,
	&fpga_qsfp28_pim4_modsel.attr,
	&fpga_qsfp28_pim4_reset.attr,
	&fpga_qsfp28_pim4_present.attr,
	&fpga_qsfp28_pim5_lpmode.attr,
	&fpga_qsfp28_pim5_modsel.attr,
	&fpga_qsfp28_pim5_reset.attr,
	&fpga_qsfp28_pim5_present.attr,
	&fpga_qsfp28_pim6_lpmode.attr,
	&fpga_qsfp28_pim6_modsel.attr,
	&fpga_qsfp28_pim6_reset.attr,
	&fpga_qsfp28_pim6_present.attr,
	&fpga_qsfp28_pim7_lpmode.attr,
	&fpga_qsfp28_pim7_modsel.attr,
	&fpga_qsfp28_pim7_reset.attr,
	&fpga_qsfp28_pim7_present.attr,
	&fpga_qsfp28_pim8_lpmode.attr,
	&fpga_qsfp28_pim8_modsel.attr,
	&fpga_qsfp28_pim8_reset.attr,
	&fpga_qsfp28_pim8_present.attr,
	&fpga_pim1_revision.attr,
	&fpga_pim2_revision.attr,
	&fpga_pim3_revision.attr,
	&fpga_pim4_revision.attr,
	&fpga_pim5_revision.attr,
	&fpga_pim6_revision.attr,
	&fpga_pim7_revision.attr,
	&fpga_pim8_revision.attr,
	&fpga_pim1_system_led.attr,
	&fpga_pim2_system_led.attr,
	&fpga_pim3_system_led.attr,
	&fpga_pim4_system_led.attr,
	&fpga_pim5_system_led.attr,
	&fpga_pim6_system_led.attr,
	&fpga_pim7_system_led.attr,
	&fpga_pim8_system_led.attr,
	&fpga_pim1_uptime.attr,
	&fpga_pim2_uptime.attr,
	&fpga_pim3_uptime.attr,
	&fpga_pim4_uptime.attr,
	&fpga_pim5_uptime.attr,
	&fpga_pim6_uptime.attr,
	&fpga_pim7_uptime.attr,
	&fpga_pim8_uptime.attr,
	&fpga_pim1_mdio_sel.attr,
	&fpga_pim2_mdio_sel.attr,
	&fpga_pim3_mdio_sel.attr,
	&fpga_pim4_mdio_sel.attr,
	&fpga_pim5_mdio_sel.attr,
	&fpga_pim6_mdio_sel.attr,
	&fpga_pim7_mdio_sel.attr,
	&fpga_pim8_mdio_sel.attr,
	&fpga_pim1_led_control.attr,
	&fpga_pim2_led_control.attr,
	&fpga_pim3_led_control.attr,
	&fpga_pim4_led_control.attr,
	&fpga_pim5_led_control.attr,
	&fpga_pim6_led_control.attr,
	&fpga_pim7_led_control.attr,
	&fpga_pim8_led_control.attr,
	&fpga_pim1_port1_led_status.attr,
	&fpga_pim1_port1_led_flash.attr,
	&fpga_pim1_port1_led_profile.attr,
	&fpga_pim1_port2_led_status.attr,
	&fpga_pim1_port2_led_flash.attr,
	&fpga_pim1_port2_led_profile.attr,
	&fpga_pim1_port3_led_status.attr,
	&fpga_pim1_port3_led_flash.attr,
	&fpga_pim1_port3_led_profile.attr,
	&fpga_pim1_port4_led_status.attr,
	&fpga_pim1_port4_led_flash.attr,
	&fpga_pim1_port4_led_profile.attr,
	&fpga_pim1_port5_led_status.attr,
	&fpga_pim1_port5_led_flash.attr,
	&fpga_pim1_port5_led_profile.attr,
	&fpga_pim1_port6_led_status.attr,
	&fpga_pim1_port6_led_flash.attr,
	&fpga_pim1_port6_led_profile.attr,
	&fpga_pim1_port7_led_status.attr,
	&fpga_pim1_port7_led_flash.attr,
	&fpga_pim1_port7_led_profile.attr,
	&fpga_pim1_port8_led_status.attr,
	&fpga_pim1_port8_led_flash.attr,
	&fpga_pim1_port8_led_profile.attr,
	&fpga_pim1_port9_led_status.attr,
	&fpga_pim1_port9_led_flash.attr,
	&fpga_pim1_port9_led_profile.attr,
	&fpga_pim1_port10_led_status.attr,
	&fpga_pim1_port10_led_flash.attr,
	&fpga_pim1_port10_led_profile.attr,
	&fpga_pim1_port11_led_status.attr,
	&fpga_pim1_port11_led_flash.attr,
	&fpga_pim1_port11_led_profile.attr,
	&fpga_pim1_port12_led_status.attr,
	&fpga_pim1_port12_led_flash.attr,
	&fpga_pim1_port12_led_profile.attr,
	&fpga_pim1_port13_led_status.attr,
	&fpga_pim1_port13_led_flash.attr,
	&fpga_pim1_port13_led_profile.attr,
	&fpga_pim1_port14_led_status.attr,
	&fpga_pim1_port14_led_flash.attr,
	&fpga_pim1_port14_led_profile.attr,
	&fpga_pim1_port15_led_status.attr,
	&fpga_pim1_port15_led_flash.attr,
	&fpga_pim1_port15_led_profile.attr,
	&fpga_pim1_port16_led_status.attr,
	&fpga_pim1_port16_led_flash.attr,
	&fpga_pim1_port16_led_profile.attr,
	&fpga_pim2_port1_led_status.attr,
	&fpga_pim2_port1_led_flash.attr,
	&fpga_pim2_port1_led_profile.attr,
	&fpga_pim2_port2_led_status.attr,
	&fpga_pim2_port2_led_flash.attr,
	&fpga_pim2_port2_led_profile.attr,
	&fpga_pim2_port3_led_status.attr,
	&fpga_pim2_port3_led_flash.attr,
	&fpga_pim2_port3_led_profile.attr,
	&fpga_pim2_port4_led_status.attr,
	&fpga_pim2_port4_led_flash.attr,
	&fpga_pim2_port4_led_profile.attr,
	&fpga_pim2_port5_led_status.attr,
	&fpga_pim2_port5_led_flash.attr,
	&fpga_pim2_port5_led_profile.attr,
	&fpga_pim2_port6_led_status.attr,
	&fpga_pim2_port6_led_flash.attr,
	&fpga_pim2_port6_led_profile.attr,
	&fpga_pim2_port7_led_status.attr,
	&fpga_pim2_port7_led_flash.attr,
	&fpga_pim2_port7_led_profile.attr,
	&fpga_pim2_port8_led_status.attr,
	&fpga_pim2_port8_led_flash.attr,
	&fpga_pim2_port8_led_profile.attr,
	&fpga_pim2_port9_led_status.attr,
	&fpga_pim2_port9_led_flash.attr,
	&fpga_pim2_port9_led_profile.attr,
	&fpga_pim2_port10_led_status.attr,
	&fpga_pim2_port10_led_flash.attr,
	&fpga_pim2_port10_led_profile.attr,
	&fpga_pim2_port11_led_status.attr,
	&fpga_pim2_port11_led_flash.attr,
	&fpga_pim2_port11_led_profile.attr,
	&fpga_pim2_port12_led_status.attr,
	&fpga_pim2_port12_led_flash.attr,
	&fpga_pim2_port12_led_profile.attr,
	&fpga_pim2_port13_led_status.attr,
	&fpga_pim2_port13_led_flash.attr,
	&fpga_pim2_port13_led_profile.attr,
	&fpga_pim2_port14_led_status.attr,
	&fpga_pim2_port14_led_flash.attr,
	&fpga_pim2_port14_led_profile.attr,
	&fpga_pim2_port15_led_status.attr,
	&fpga_pim2_port15_led_flash.attr,
	&fpga_pim2_port15_led_profile.attr,
	&fpga_pim2_port16_led_status.attr,
	&fpga_pim2_port16_led_flash.attr,
	&fpga_pim2_port16_led_profile.attr,
	&fpga_pim3_port1_led_status.attr,
	&fpga_pim3_port1_led_flash.attr,
	&fpga_pim3_port1_led_profile.attr,
	&fpga_pim3_port2_led_status.attr,
	&fpga_pim3_port2_led_flash.attr,
	&fpga_pim3_port2_led_profile.attr,
	&fpga_pim3_port3_led_status.attr,
	&fpga_pim3_port3_led_flash.attr,
	&fpga_pim3_port3_led_profile.attr,
	&fpga_pim3_port4_led_status.attr,
	&fpga_pim3_port4_led_flash.attr,
	&fpga_pim3_port4_led_profile.attr,
	&fpga_pim3_port5_led_status.attr,
	&fpga_pim3_port5_led_flash.attr,
	&fpga_pim3_port5_led_profile.attr,
	&fpga_pim3_port6_led_status.attr,
	&fpga_pim3_port6_led_flash.attr,
	&fpga_pim3_port6_led_profile.attr,
	&fpga_pim3_port7_led_status.attr,
	&fpga_pim3_port7_led_flash.attr,
	&fpga_pim3_port7_led_profile.attr,
	&fpga_pim3_port8_led_status.attr,
	&fpga_pim3_port8_led_flash.attr,
	&fpga_pim3_port8_led_profile.attr,
	&fpga_pim3_port9_led_status.attr,
	&fpga_pim3_port9_led_flash.attr,
	&fpga_pim3_port9_led_profile.attr,
	&fpga_pim3_port10_led_status.attr,
	&fpga_pim3_port10_led_flash.attr,
	&fpga_pim3_port10_led_profile.attr,
	&fpga_pim3_port11_led_status.attr,
	&fpga_pim3_port11_led_flash.attr,
	&fpga_pim3_port11_led_profile.attr,
	&fpga_pim3_port12_led_status.attr,
	&fpga_pim3_port12_led_flash.attr,
	&fpga_pim3_port12_led_profile.attr,
	&fpga_pim3_port13_led_status.attr,
	&fpga_pim3_port13_led_flash.attr,
	&fpga_pim3_port13_led_profile.attr,
	&fpga_pim3_port14_led_status.attr,
	&fpga_pim3_port14_led_flash.attr,
	&fpga_pim3_port14_led_profile.attr,
	&fpga_pim3_port15_led_status.attr,
	&fpga_pim3_port15_led_flash.attr,
	&fpga_pim3_port15_led_profile.attr,
	&fpga_pim3_port16_led_status.attr,
	&fpga_pim3_port16_led_flash.attr,
	&fpga_pim3_port16_led_profile.attr,
	&fpga_pim4_port1_led_status.attr,
	&fpga_pim4_port1_led_flash.attr,
	&fpga_pim4_port1_led_profile.attr,
	&fpga_pim4_port2_led_status.attr,
	&fpga_pim4_port2_led_flash.attr,
	&fpga_pim4_port2_led_profile.attr,
	&fpga_pim4_port3_led_status.attr,
	&fpga_pim4_port3_led_flash.attr,
	&fpga_pim4_port3_led_profile.attr,
	&fpga_pim4_port4_led_status.attr,
	&fpga_pim4_port4_led_flash.attr,
	&fpga_pim4_port4_led_profile.attr,
	&fpga_pim4_port5_led_status.attr,
	&fpga_pim4_port5_led_flash.attr,
	&fpga_pim4_port5_led_profile.attr,
	&fpga_pim4_port6_led_status.attr,
	&fpga_pim4_port6_led_flash.attr,
	&fpga_pim4_port6_led_profile.attr,
	&fpga_pim4_port7_led_status.attr,
	&fpga_pim4_port7_led_flash.attr,
	&fpga_pim4_port7_led_profile.attr,
	&fpga_pim4_port8_led_status.attr,
	&fpga_pim4_port8_led_flash.attr,
	&fpga_pim4_port8_led_profile.attr,
	&fpga_pim4_port9_led_status.attr,
	&fpga_pim4_port9_led_flash.attr,
	&fpga_pim4_port9_led_profile.attr,
	&fpga_pim4_port10_led_status.attr,
	&fpga_pim4_port10_led_flash.attr,
	&fpga_pim4_port10_led_profile.attr,
	&fpga_pim4_port11_led_status.attr,
	&fpga_pim4_port11_led_flash.attr,
	&fpga_pim4_port11_led_profile.attr,
	&fpga_pim4_port12_led_status.attr,
	&fpga_pim4_port12_led_flash.attr,
	&fpga_pim4_port12_led_profile.attr,
	&fpga_pim4_port13_led_status.attr,
	&fpga_pim4_port13_led_flash.attr,
	&fpga_pim4_port13_led_profile.attr,
	&fpga_pim4_port14_led_status.attr,
	&fpga_pim4_port14_led_flash.attr,
	&fpga_pim4_port14_led_profile.attr,
	&fpga_pim4_port15_led_status.attr,
	&fpga_pim4_port15_led_flash.attr,
	&fpga_pim4_port15_led_profile.attr,
	&fpga_pim4_port16_led_status.attr,
	&fpga_pim4_port16_led_flash.attr,
	&fpga_pim4_port16_led_profile.attr,
	&fpga_pim5_port1_led_status.attr,
	&fpga_pim5_port1_led_flash.attr,
	&fpga_pim5_port1_led_profile.attr,
	&fpga_pim5_port2_led_status.attr,
	&fpga_pim5_port2_led_flash.attr,
	&fpga_pim5_port2_led_profile.attr,
	&fpga_pim5_port3_led_status.attr,
	&fpga_pim5_port3_led_flash.attr,
	&fpga_pim5_port3_led_profile.attr,
	&fpga_pim5_port4_led_status.attr,
	&fpga_pim5_port4_led_flash.attr,
	&fpga_pim5_port4_led_profile.attr,
	&fpga_pim5_port5_led_status.attr,
	&fpga_pim5_port5_led_flash.attr,
	&fpga_pim5_port5_led_profile.attr,
	&fpga_pim5_port6_led_status.attr,
	&fpga_pim5_port6_led_flash.attr,
	&fpga_pim5_port6_led_profile.attr,
	&fpga_pim5_port7_led_status.attr,
	&fpga_pim5_port7_led_flash.attr,
	&fpga_pim5_port7_led_profile.attr,
	&fpga_pim5_port8_led_status.attr,
	&fpga_pim5_port8_led_flash.attr,
	&fpga_pim5_port8_led_profile.attr,
	&fpga_pim5_port9_led_status.attr,
	&fpga_pim5_port9_led_flash.attr,
	&fpga_pim5_port9_led_profile.attr,
	&fpga_pim5_port10_led_status.attr,
	&fpga_pim5_port10_led_flash.attr,
	&fpga_pim5_port10_led_profile.attr,
	&fpga_pim5_port11_led_status.attr,
	&fpga_pim5_port11_led_flash.attr,
	&fpga_pim5_port11_led_profile.attr,
	&fpga_pim5_port12_led_status.attr,
	&fpga_pim5_port12_led_flash.attr,
	&fpga_pim5_port12_led_profile.attr,
	&fpga_pim5_port13_led_status.attr,
	&fpga_pim5_port13_led_flash.attr,
	&fpga_pim5_port13_led_profile.attr,
	&fpga_pim5_port14_led_status.attr,
	&fpga_pim5_port14_led_flash.attr,
	&fpga_pim5_port14_led_profile.attr,
	&fpga_pim5_port15_led_status.attr,
	&fpga_pim5_port15_led_flash.attr,
	&fpga_pim5_port15_led_profile.attr,
	&fpga_pim5_port16_led_status.attr,
	&fpga_pim5_port16_led_flash.attr,
	&fpga_pim5_port16_led_profile.attr,
	&fpga_pim6_port1_led_status.attr,
	&fpga_pim6_port1_led_flash.attr,
	&fpga_pim6_port1_led_profile.attr,
	&fpga_pim6_port2_led_status.attr,
	&fpga_pim6_port2_led_flash.attr,
	&fpga_pim6_port2_led_profile.attr,
	&fpga_pim6_port3_led_status.attr,
	&fpga_pim6_port3_led_flash.attr,
	&fpga_pim6_port3_led_profile.attr,
	&fpga_pim6_port4_led_status.attr,
	&fpga_pim6_port4_led_flash.attr,
	&fpga_pim6_port4_led_profile.attr,
	&fpga_pim6_port5_led_status.attr,
	&fpga_pim6_port5_led_flash.attr,
	&fpga_pim6_port5_led_profile.attr,
	&fpga_pim6_port6_led_status.attr,
	&fpga_pim6_port6_led_flash.attr,
	&fpga_pim6_port6_led_profile.attr,
	&fpga_pim6_port7_led_status.attr,
	&fpga_pim6_port7_led_flash.attr,
	&fpga_pim6_port7_led_profile.attr,
	&fpga_pim6_port8_led_status.attr,
	&fpga_pim6_port8_led_flash.attr,
	&fpga_pim6_port8_led_profile.attr,
	&fpga_pim6_port9_led_status.attr,
	&fpga_pim6_port9_led_flash.attr,
	&fpga_pim6_port9_led_profile.attr,
	&fpga_pim6_port10_led_status.attr,
	&fpga_pim6_port10_led_flash.attr,
	&fpga_pim6_port10_led_profile.attr,
	&fpga_pim6_port11_led_status.attr,
	&fpga_pim6_port11_led_flash.attr,
	&fpga_pim6_port11_led_profile.attr,
	&fpga_pim6_port12_led_status.attr,
	&fpga_pim6_port12_led_flash.attr,
	&fpga_pim6_port12_led_profile.attr,
	&fpga_pim6_port13_led_status.attr,
	&fpga_pim6_port13_led_flash.attr,
	&fpga_pim6_port13_led_profile.attr,
	&fpga_pim6_port14_led_status.attr,
	&fpga_pim6_port14_led_flash.attr,
	&fpga_pim6_port14_led_profile.attr,
	&fpga_pim6_port15_led_status.attr,
	&fpga_pim6_port15_led_flash.attr,
	&fpga_pim6_port15_led_profile.attr,
	&fpga_pim6_port16_led_status.attr,
	&fpga_pim6_port16_led_flash.attr,
	&fpga_pim6_port16_led_profile.attr,
	&fpga_pim7_port1_led_status.attr,
	&fpga_pim7_port1_led_flash.attr,
	&fpga_pim7_port1_led_profile.attr,
	&fpga_pim7_port2_led_status.attr,
	&fpga_pim7_port2_led_flash.attr,
	&fpga_pim7_port2_led_profile.attr,
	&fpga_pim7_port3_led_status.attr,
	&fpga_pim7_port3_led_flash.attr,
	&fpga_pim7_port3_led_profile.attr,
	&fpga_pim7_port4_led_status.attr,
	&fpga_pim7_port4_led_flash.attr,
	&fpga_pim7_port4_led_profile.attr,
	&fpga_pim7_port5_led_status.attr,
	&fpga_pim7_port5_led_flash.attr,
	&fpga_pim7_port5_led_profile.attr,
	&fpga_pim7_port6_led_status.attr,
	&fpga_pim7_port6_led_flash.attr,
	&fpga_pim7_port6_led_profile.attr,
	&fpga_pim7_port7_led_status.attr,
	&fpga_pim7_port7_led_flash.attr,
	&fpga_pim7_port7_led_profile.attr,
	&fpga_pim7_port8_led_status.attr,
	&fpga_pim7_port8_led_flash.attr,
	&fpga_pim7_port8_led_profile.attr,
	&fpga_pim7_port9_led_status.attr,
	&fpga_pim7_port9_led_flash.attr,
	&fpga_pim7_port9_led_profile.attr,
	&fpga_pim7_port10_led_status.attr,
	&fpga_pim7_port10_led_flash.attr,
	&fpga_pim7_port10_led_profile.attr,
	&fpga_pim7_port11_led_status.attr,
	&fpga_pim7_port11_led_flash.attr,
	&fpga_pim7_port11_led_profile.attr,
	&fpga_pim7_port12_led_status.attr,
	&fpga_pim7_port12_led_flash.attr,
	&fpga_pim7_port12_led_profile.attr,
	&fpga_pim7_port13_led_status.attr,
	&fpga_pim7_port13_led_flash.attr,
	&fpga_pim7_port13_led_profile.attr,
	&fpga_pim7_port14_led_status.attr,
	&fpga_pim7_port14_led_flash.attr,
	&fpga_pim7_port14_led_profile.attr,
	&fpga_pim7_port15_led_status.attr,
	&fpga_pim7_port15_led_flash.attr,
	&fpga_pim7_port15_led_profile.attr,
	&fpga_pim7_port16_led_status.attr,
	&fpga_pim7_port16_led_flash.attr,
	&fpga_pim7_port16_led_profile.attr,
	&fpga_pim8_port1_led_status.attr,
	&fpga_pim8_port1_led_flash.attr,
	&fpga_pim8_port1_led_profile.attr,
	&fpga_pim8_port2_led_status.attr,
	&fpga_pim8_port2_led_flash.attr,
	&fpga_pim8_port2_led_profile.attr,
	&fpga_pim8_port3_led_status.attr,
	&fpga_pim8_port3_led_flash.attr,
	&fpga_pim8_port3_led_profile.attr,
	&fpga_pim8_port4_led_status.attr,
	&fpga_pim8_port4_led_flash.attr,
	&fpga_pim8_port4_led_profile.attr,
	&fpga_pim8_port5_led_status.attr,
	&fpga_pim8_port5_led_flash.attr,
	&fpga_pim8_port5_led_profile.attr,
	&fpga_pim8_port6_led_status.attr,
	&fpga_pim8_port6_led_flash.attr,
	&fpga_pim8_port6_led_profile.attr,
	&fpga_pim8_port7_led_status.attr,
	&fpga_pim8_port7_led_flash.attr,
	&fpga_pim8_port7_led_profile.attr,
	&fpga_pim8_port8_led_status.attr,
	&fpga_pim8_port8_led_flash.attr,
	&fpga_pim8_port8_led_profile.attr,
	&fpga_pim8_port9_led_status.attr,
	&fpga_pim8_port9_led_flash.attr,
	&fpga_pim8_port9_led_profile.attr,
	&fpga_pim8_port10_led_status.attr,
	&fpga_pim8_port10_led_flash.attr,
	&fpga_pim8_port10_led_profile.attr,
	&fpga_pim8_port11_led_status.attr,
	&fpga_pim8_port11_led_flash.attr,
	&fpga_pim8_port11_led_profile.attr,
	&fpga_pim8_port12_led_status.attr,
	&fpga_pim8_port12_led_flash.attr,
	&fpga_pim8_port12_led_profile.attr,
	&fpga_pim8_port13_led_status.attr,
	&fpga_pim8_port13_led_flash.attr,
	&fpga_pim8_port13_led_profile.attr,
	&fpga_pim8_port14_led_status.attr,
	&fpga_pim8_port14_led_flash.attr,
	&fpga_pim8_port14_led_profile.attr,
	&fpga_pim8_port15_led_status.attr,
	&fpga_pim8_port15_led_flash.attr,
	&fpga_pim8_port15_led_profile.attr,
	&fpga_pim8_port16_led_status.attr,
	&fpga_pim8_port16_led_flash.attr,
	&fpga_pim8_port16_led_profile.attr,
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

static int fpga_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct fpga_priv *priv;
	struct resource ctrl_resource;
	unsigned long start, len;
	int  err = 0;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	pci_set_drvdata(pdev, priv);
	priv->pci_dev = pdev;

	err = pcim_enable_device(pdev);
	if (err) {
		dev_err(&pdev->dev, "Failed to enable FPGA PCI device (%d)\n",
			err);
		goto exit;
	}

	/* Determine the address of the FPGA area */
	start = pci_resource_start(pdev, FPGA_BAR);
	len = pci_resource_len(pdev, FPGA_BAR);
	if (!start || !len) {
		dev_err(&pdev->dev,
			"FPGA base address uninitialized, upgrade BIOS\n");
		pci_disable_device(pdev);
		err = -ENODEV;
		goto exit;
	}

	err = fpga_dev_init(priv);
	if (err) {
		pr_err("fpga_dev_init() failed");
		err = -ENOMEM;
		pci_disable_device(pdev);
		goto exit;
	}

	/*
	 * Initialise a ctrl resource for various LED, DMA, and
	 * SFP/QSFP reset/modsel etc. registers in the physical
	 * memory range 0x0 - 0x7FFFF.
	 */
	ctrl_resource.start = start;
	ctrl_resource.end   = start + 0x7FFFF;
	ctrl_resource.flags = IORESOURCE_MEM;

	priv->pbar = devm_ioremap_resource(&pdev->dev, &ctrl_resource);
	if (!priv->pbar) {
		pr_err("devm_ioremap_resource failed for cres");
		err = -ENOMEM;
		pci_disable_device(pdev);
		goto exit;
	}

	/*
	 * Create sysfs group for ctrl resource.
	 */
	err = sysfs_create_group(&pdev->dev.kobj, &fpga_attr_group);
	if (err) {
		pr_err("sysfs_fpga_attr_group failed for FPGA driver\n");
		pci_disable_device(pdev);
		goto exit;
	}

	dev_dbg(&pdev->dev, "FPGA driver loaded\n");

exit:
	if (err) {
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

	dev_dbg(&pdev->dev, "FPGA driver unloaded\n");
}

#define fpga_suspend NULL
#define fpga_resume NULL

static struct pci_driver fpga_driver = {
	.name = FPGA_DRIVER,
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
		pr_err("pci_register_driver() failed for fpga device\n");
		return ret;
	}

	pr_info(FPGA_DRIVER " driver loaded\n");
	return 0;
}

static void fpga_exit(void)
{
	pci_unregister_driver(&fpga_driver);
	pr_debug(FPGA_DRIVER " driver unloaded\n");
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

/*
 * Utility functions for I2C
 */

static int i2c_init(void)
{
	int cp2112_bus;
	int i;
	int ret;

	cp2112_bus = cumulus_i2c_find_adapter("CP2112 SMBus Bridge");
	if (cp2112_bus < 0) {
		pr_err("could not find CP2112 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		int bus = i2c_devices[i].bus;
		struct i2c_client *client;

		if (bus == MP_I2C_CP2112_BUS)
			bus = cp2112_bus;
		client = cumulus_i2c_add_client(bus, &i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_devices[i].client = client;
	}
	pr_debug("I2C driver loaded\n");
	return 0;
err_exit:
	i2c_exit();
	return ret;
}

/*
 * CPLD driver
 */

#define CPLD_NREGS 0x40

#define cpld_read_reg cumulus_bf_i2c_read_reg
#define cpld_write_reg cumulus_bf_i2c_write_reg

/*
 * CPLD register definitions
 * Only the useful ones are exposed.
 */

static const char * const cpld_board_pcb_values[] = {
	"R0A",
	"R0B",
	"R0C",
	"R01",
};

mk_bf_ro32(cpld, board_revision,    BOARD_INFO, 4, 2, NULL, BF_DECIMAL);
mk_bf_ro32(cpld, board_pcb_ver,     BOARD_INFO, 0, 2, cpld_board_pcb_values, 0);

mk_bf_ro32(cpld, cpld_revision,     CPLD_VER, 0, 6, NULL, BF_DECIMAL);
mk_bf_ro32(cpld, cpld_released,     CPLD_VER, 6, 1, NULL, 0);

mk_bf_ro32(cpld, cpld_subversion,   CPLD_SUB_VER, 0, 8, NULL, BF_DECIMAL);
mk_bf_ro32(cpld, psu_pwr1_all_ok,   POWER_MOD_L_STATUS, 7,  1, NULL, 0);
mk_bf_ro32(cpld, psu_pwr1_alarm,    POWER_MOD_L_STATUS, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro32(cpld, psu_pwr1_present,  POWER_MOD_L_STATUS, 6,  1, NULL, BF_COMPLEMENT);
mk_bf_ro32(cpld, psu_pwr2_all_ok,   POWER_MOD_L_STATUS, 3,  1, NULL, 0);
mk_bf_ro32(cpld, psu_pwr2_alarm,    POWER_MOD_L_STATUS, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro32(cpld, psu_pwr2_present,  POWER_MOD_L_STATUS, 2,  1, NULL, BF_COMPLEMENT);

mk_bf_ro32(cpld, psu_pwr3_all_ok,   POWER_MOD_R_STATUS, 7,  1, NULL, 0);
mk_bf_ro32(cpld, psu_pwr3_alarm,    POWER_MOD_R_STATUS, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro32(cpld, psu_pwr3_present,  POWER_MOD_R_STATUS, 6,  1, NULL, BF_COMPLEMENT);
mk_bf_ro32(cpld, psu_pwr4_all_ok,   POWER_MOD_R_STATUS, 3,  1, NULL, 0);
mk_bf_ro32(cpld, psu_pwr4_alarm,    POWER_MOD_R_STATUS, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro32(cpld, psu_pwr4_present,  POWER_MOD_R_STATUS, 2,  1, NULL, BF_COMPLEMENT);

mk_bf_rw32(cpld, cp2112_reset,      SYS_RESET_2, 4, 1, NULL, 0);

mk_bf_ro32(cpld, pim_fpga_present,  PIM_FPGA_PRESENT, 0, 8, NULL, BF_COMPLEMENT);
mk_bf_ro32(cpld, scm_fcm_present,   SCM_FCM_PRESENT, 0, 3, NULL, BF_COMPLEMENT);

struct attribute *cpld_attrs[] = {
	&cpld_board_revision.attr,
	&cpld_board_pcb_ver.attr,
	&cpld_cpld_revision.attr,
	&cpld_cpld_released.attr,
	&cpld_cpld_subversion.attr,
	&cpld_psu_pwr2_all_ok.attr,
	&cpld_psu_pwr2_alarm.attr,
	&cpld_psu_pwr2_present.attr,
	&cpld_psu_pwr1_all_ok.attr,
	&cpld_psu_pwr1_alarm.attr,
	&cpld_psu_pwr1_present.attr,
	&cpld_psu_pwr4_all_ok.attr,
	&cpld_psu_pwr4_alarm.attr,
	&cpld_psu_pwr4_present.attr,
	&cpld_psu_pwr3_all_ok.attr,
	&cpld_psu_pwr3_alarm.attr,
	&cpld_psu_pwr3_present.attr,
	&cpld_cp2112_reset.attr,
	&cpld_pim_fpga_present.attr,
	&cpld_scm_fcm_present.attr,
	NULL
};

static struct attribute_group cpld_attr_group = {
	.attrs = cpld_attrs,
};

static int cpld_probe(struct i2c_client *client,
		      const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int boardrev;
	int cpldrev;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev,
			"adapter does not support I2C_FUNC_SMBUS_BYTE_DATA\n");
		ret = -EINVAL;
		goto err;
	}

	/*
	 * Probe the hardware by reading the revision numbers.
	 */
	ret = i2c_smbus_read_byte_data(client, 0);
	if (ret < 0) {
		dev_err(&client->dev,
			"read board revision register error %d\n", ret);
		goto err;
	}
	boardrev = ret;
	ret = i2c_smbus_read_byte_data(client, 1);
	if (ret < 0) {
		dev_err(&client->dev,
			"read CPLD revision register error %d\n", ret);
		goto err;
	}
	cpldrev = ret;

	/*
	 * Create sysfs nodes.
	 */
	ret = sysfs_create_group(&client->dev.kobj, &cpld_attr_group);
	if (ret) {
		dev_err(&client->dev, "sysfs_create_group failed\n");
		goto err;
	}

	/*
	 * All clear from this point on
	 */
	dev_dbg(&client->dev,
		"device created, board rev %d pcb_ver %d, CPLD rev %d %s\n",
		(boardrev & 0x70) >> 4,
		boardrev & 0x03,
		cpldrev & 0x3f,
		cpldrev & 0x40 ? "released" : "unreleased");

#ifdef DEBUG
	for (i = 0; i < CPLD_NREGS; i++) {
		ret = i2c_smbus_read_byte_data(client, i);
		dev_dbg(&client->dev,
			ret < 0 ? "CPLD[%d] read error %d\n" :
				  "CPLD[%d] %#04x\n",
			i, ret);
	}
#endif

	return 0;

err:
	return ret;
}

static int cpld_remove(struct i2c_client *client)
{
	dev_info(&client->dev, "device removed\n");
	return 0;
}

static const struct i2c_device_id cpld_id[] = {
	{ "act_minipack_cpld", 0 },	/* full name is too long, ick */
	{ }
};
MODULE_DEVICE_TABLE(i2c, cpld_id);

static struct i2c_driver cpld_driver = {
	.driver = {
		.name = "accton_minipack_cpld",
		.owner = THIS_MODULE,
	},
	.probe = cpld_probe,
	.remove = cpld_remove,
	.id_table = cpld_id,
};

static int cpld_init(void)
{
	pr_info(DRIVER_NAME " loading CPLD driver\n");
	return i2c_add_driver(&cpld_driver);
}

static void cpld_exit(void)
{
	i2c_del_driver(&cpld_driver);
	pr_info(DRIVER_NAME " CPLD driver unloaded\n");
}

/*-----------------------------------------------------------------------------
 *
 * Module init/exit
 */
static int __init accton_minipack_init(void)
{
	int ret = 0;

	ret = cpld_init();
	if (ret) {
		pr_err(DRIVER_NAME ": CPLD initialization failed\n");
		return ret;
	}

	ret = i2c_init();
	if (ret) {
		pr_err(DRIVER_NAME ": I2C subsystem initialization failed\n");
		cpld_exit();
		return ret;
	}

	ret = fpga_init();
	if (ret) {
		pr_err(DRIVER_NAME ": FPGA initialization failed\n");
		i2c_exit();
		cpld_exit();
		return ret;
	}
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully loaded\n");
	return 0;
}

static void __exit accton_minipack_exit(void)
{
	fpga_exit();
	i2c_exit();
	cpld_exit();
	pr_info(DRIVER_NAME " driver successfully unloaded\n");
}

module_init(accton_minipack_init);
module_exit(accton_minipack_exit);

MODULE_AUTHOR("Alok Kumar (alok@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton Minipack Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
