/*
 * dellemc_s5248f_platform.c - Dell EMC S5248F-C3538 Platform Support.
 *
 * Copyright (c) 2018, 2019, 2020 Cumulus Networks, Inc.  All Rights Reserved.
 * Authors: David Yen (dhyen@cumulusnetworks.com)
 *	    Frank Hoeflich (frankh@cumulusnetworks.com)
 *	    Nikhil Dhar (ndhar@cumulusnetworks.com)
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

#define DRIVER_NAME		  "dellemc_s5248f_platform"
#define DRIVER_VERSION		  "1.2"

#define ISMT_ADAPTER_NAME	  "SMBus iSMT adapter"
#define I801_ADAPTER_NAME	  "SMBus I801 adapter"

#define PCI_DEVICE_ID_XILINX_FPGA 0x7021
#define FPGA_BLOCK_MAX		  32
#define FPGA_MAX_RETRIES	  5
#define FPGA_BAR		  0
#define FPGA_DESC_ENTRIES	  2
#define FPGA_FAILED_READ_REG	  0xffffffffU

/*
 * This platform has the following i2c busses:
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
 * The FPGA is the gateway for access to another 16 I2C busses, of which
 * only 4-10 inclusive are of current interest to us:
 *
 *   CL_I2C_FPGA_BUS4
 *   ...
 *   CL_I2C_FPGA_BUS10
 *
 * where a PCA9548 mux may be found on each, with the SFP/QSFP/QSFP-DD EEPROMs
 * living each on individual channels.
 */

/*
 *
 * SMBus Interface
 *
 */
enum {
	CL_I2C_I801_BUS = -2,
	CL_I2C_ISMT_BUS,
	CL_I2C_OCORES_BUS,
	CL_I2C_FPGA_BUS1 = 1,
	CL_I2C_FPGA_BUS2,
	CL_I2C_FPGA_BUS3,
	CL_I2C_FPGA_BUS4,	/* SFP28_1  - SFP28_8  */
	CL_I2C_FPGA_BUS5,	/* SFP28_9  - SFP28_16 */
	CL_I2C_FPGA_BUS6,	/* SFP28_17 - SFP28_24 */
	CL_I2C_FPGA_BUS7,	/* SFP28_25 - SFP28_32 */
	CL_I2C_FPGA_BUS8,	/* SFP28_33 - SFP28_40 */
	CL_I2C_FPGA_BUS9,	/* SFP28_41 - SFP28_48 */
	CL_I2C_FPGA_BUS10,	/* QSFP28_1 - QSFP28_4, QSFPDD_1 - QSFPDD_2 */
	CL_I2C_FPGA_BUS4_0 = 20,
	CL_I2C_FPGA_BUS4_1,
	CL_I2C_FPGA_BUS4_2,
	CL_I2C_FPGA_BUS4_3,
	CL_I2C_FPGA_BUS4_4,
	CL_I2C_FPGA_BUS4_5,
	CL_I2C_FPGA_BUS4_6,
	CL_I2C_FPGA_BUS4_7,
	CL_I2C_FPGA_BUS5_0,
	CL_I2C_FPGA_BUS5_1,
	CL_I2C_FPGA_BUS5_2,
	CL_I2C_FPGA_BUS5_3,
	CL_I2C_FPGA_BUS5_4,
	CL_I2C_FPGA_BUS5_5,
	CL_I2C_FPGA_BUS5_6,
	CL_I2C_FPGA_BUS5_7,
	CL_I2C_FPGA_BUS6_0,
	CL_I2C_FPGA_BUS6_1,
	CL_I2C_FPGA_BUS6_2,
	CL_I2C_FPGA_BUS6_3,
	CL_I2C_FPGA_BUS6_4,
	CL_I2C_FPGA_BUS6_5,
	CL_I2C_FPGA_BUS6_6,
	CL_I2C_FPGA_BUS6_7,
	CL_I2C_FPGA_BUS7_0,
	CL_I2C_FPGA_BUS7_1,
	CL_I2C_FPGA_BUS7_2,
	CL_I2C_FPGA_BUS7_3,
	CL_I2C_FPGA_BUS7_4,
	CL_I2C_FPGA_BUS7_5,
	CL_I2C_FPGA_BUS7_6,
	CL_I2C_FPGA_BUS7_7,
	CL_I2C_FPGA_BUS8_0,
	CL_I2C_FPGA_BUS8_1,
	CL_I2C_FPGA_BUS8_2,
	CL_I2C_FPGA_BUS8_3,
	CL_I2C_FPGA_BUS8_4,
	CL_I2C_FPGA_BUS8_5,
	CL_I2C_FPGA_BUS8_6,
	CL_I2C_FPGA_BUS8_7,
	CL_I2C_FPGA_BUS9_0,
	CL_I2C_FPGA_BUS9_1,
	CL_I2C_FPGA_BUS9_2,
	CL_I2C_FPGA_BUS9_3,
	CL_I2C_FPGA_BUS9_4,
	CL_I2C_FPGA_BUS9_5,
	CL_I2C_FPGA_BUS9_6,
	CL_I2C_FPGA_BUS9_7,
	CL_I2C_FPGA_BUS10_0,
	CL_I2C_FPGA_BUS10_1,
	CL_I2C_FPGA_BUS10_2,
	CL_I2C_FPGA_BUS10_3,
	CL_I2C_FPGA_BUS10_4,
	CL_I2C_FPGA_BUS10_5,
	CL_I2C_FPGA_BUS10_6,
	CL_I2C_FPGA_BUS10_7,
};

enum port_type {
	PORT_TYPE_SFP,
	PORT_TYPE_QSFP,
	PORT_TYPE_QSFPDD,
};

mk_eeprom(board, 50, 256, AT24_FLAG_IRUGO);

/*
 * Platform data for the PCA9548 MUXes living on base FPGA busses 4-10.
 * Since bus 10 doesn't get fully populated, we initialise it "by hand."
 */
mk_pca9548(fpga_bus4,  CL_I2C_FPGA_BUS4_0,  1);
mk_pca9548(fpga_bus5,  CL_I2C_FPGA_BUS5_0,  1);
mk_pca9548(fpga_bus6,  CL_I2C_FPGA_BUS6_0,  1);
mk_pca9548(fpga_bus7,  CL_I2C_FPGA_BUS7_0,  1);
mk_pca9548(fpga_bus8,  CL_I2C_FPGA_BUS8_0,  1);
mk_pca9548(fpga_bus9,  CL_I2C_FPGA_BUS9_0,  1);
static struct pca954x_platform_mode fpga_bus10_platform_modes[] = {
       {
       .adap_id = CL_I2C_FPGA_BUS10_0, .deselect_on_exit = 1,
       },
       {
       .adap_id = CL_I2C_FPGA_BUS10_1, .deselect_on_exit = 1,
       },
       {
       .adap_id = CL_I2C_FPGA_BUS10_2, .deselect_on_exit = 1,
       },
       {
       .adap_id = CL_I2C_FPGA_BUS10_3, .deselect_on_exit = 1,
       },
       {
       .adap_id = CL_I2C_FPGA_BUS10_4, .deselect_on_exit = 1,
       },
       {
       .adap_id = CL_I2C_FPGA_BUS10_5, .deselect_on_exit = 1,
       },
};

static struct pca954x_platform_data fpga_bus10_platform_data = {
       .modes = fpga_bus10_platform_modes,
       .num_modes = ARRAY_SIZE(fpga_bus10_platform_modes),
};

/*
 * Here's the scoop on i2c_devices[].
 *
 * The i2c_devices[] array contains all that we expose on I2C on the
 * "Denverton" side, i.e. everything on the "near" side of the FPGA.  The
 * i2c_clients[] array tracks these entries 1-1, and they are freed upon
 * exit based on the size/entries in i2c_devices[].  Not much there.
 *
 * The ocores_i2c_mux_devices[] array contains only the minimum information
 * needed to pass to the i2c-ocores driver in order to set up all I2C
 * adapters on the far side of the FPGA.  This means just the PCA9548 muxes,
 * one of which is found on each FPGA I2C bus.  A "slice" is defined for
 * each bus with memory resources (in the form of register blocks) needed to
 * communicate on that bus via OpenCores.  The "slice" is registered via a
 * platform driver connection to the i2c-ocores driver.
 *
 * The ocores_i2c_devices[] array does the same thing only for all remaining
 * devices exposed on the "far" side of the FPGA.  A separate structure,
 * ocores_i2c_device_infotab[], is built and used to communicate I2C device
 * information to the i2c-ocores kernel module in "slices" so that it can
 * manipulate FPGA registers as needed in order to access those devices per
 * the OpenCores I2C standard.
 *
 * The purpose of a separate ocores_i2c_devices[]
 * array is to provide good separation between "near" and "far" I2C devices
 * and to allow ocores_i2c_device_infotab[] and some related tables to be
 * built cleanly using ocores_i2c_devices[] only.
 */
struct ocores_i2c_platform_dev_info {
	int bus;
	struct i2c_board_info board_info;
	int has_port;  /* 1 for SFP, 2 for QSFP, 3 for QSFP-DD */
	int port_base;
	int port_bus;
	int num_ports;
};

static struct ocores_i2c_platform_dev_info i2c_devices[] = {
	/*
	 * This section of devices is on the "near side" of the FPGA to
	 * the Denverton CPU.  Vanilla board EEPROMs, DIMMs etc. are here.
	 */
	mk_i2cdev(CL_I2C_ISMT_BUS, "24c02", 0x50, &board_50_at24),
};

static struct ocores_i2c_platform_dev_info ocores_i2c_mux_devices[] = {
	mk_i2cdev(CL_I2C_FPGA_BUS4, "pca9548", 0x74, &fpga_bus4_platform_data),
	mk_i2cdev(CL_I2C_FPGA_BUS5, "pca9548", 0x74, &fpga_bus5_platform_data),
	mk_i2cdev(CL_I2C_FPGA_BUS6, "pca9548", 0x74, &fpga_bus6_platform_data),
	mk_i2cdev(CL_I2C_FPGA_BUS7, "pca9548", 0x74, &fpga_bus7_platform_data),
	mk_i2cdev(CL_I2C_FPGA_BUS8, "pca9548", 0x74, &fpga_bus8_platform_data),
	mk_i2cdev(CL_I2C_FPGA_BUS9, "pca9548", 0x74, &fpga_bus9_platform_data),
	mk_i2cdev(CL_I2C_FPGA_BUS10, "pca9548", 0x74,
		  &fpga_bus10_platform_data),
};

static struct ocores_i2c_platform_dev_info ocores_i2c_devices[] = {
	mk_i2cdev(CL_I2C_FPGA_BUS4_0,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS4_1,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS4_2,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS4_3,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS4_4,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS4_5,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS4_6,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS4_7,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS5_0,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS5_1,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS5_2,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS5_3,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS5_4,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS5_5,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS5_6,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS5_7,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS6_0,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS6_1,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS6_2,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS6_3,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS6_4,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS6_5,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS6_6,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS6_7,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS7_0,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS7_1,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS7_2,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS7_3,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS7_4,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS7_5,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS7_6,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS7_7,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS8_0,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS8_1,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS8_2,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS8_3,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS8_4,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS8_5,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS8_6,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS8_7,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS9_0,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS9_1,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS9_2,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS9_3,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS9_4,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS9_5,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS9_6,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS9_7,  "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS10_0, "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS10_1, "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS10_2, "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS10_3, "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS10_4, "", 0x0, NULL),
	mk_i2cdev(CL_I2C_FPGA_BUS10_5, "", 0x0, NULL),
};

/*
 * Arrays of allocated i2c_client objects.  Need to track these in
 * order to free them later.
 *
 * Note that "far side" i2c_clients are created over there and freed over
 * there by the i2c-ocores driver.  They don't appear here at all (except
 * that we're experimenting with doing the muxes here now ...).
 */
static struct i2c_client *i2c_clients[ARRAY_SIZE(i2c_devices)];
static struct i2c_client *ocores_i2c_clients[ARRAY_SIZE(ocores_i2c_devices)];

#define OCORES_DRV_NAME "ocores-i2c"

#define ITABSIZE	ARRAY_SIZE(ocores_i2c_mux_devices)
struct ocores_i2c_device_info ocores_i2c_device_infotab[ITABSIZE];
static struct ocores_i2c_platform_data ocores_i2c_data[ITABSIZE];

/* I2C teardown */
static void i2c_exit(void)
{
	int i;
	struct i2c_client *c;

	/*
	 * Unregister far-side (ocores_i2c_devices) I2C clients.
	 */
	for (i = ARRAY_SIZE(ocores_i2c_clients); --i >= 0;) {
		c = ocores_i2c_clients[i];
		if (c)
			i2c_unregister_device(c);
	}

	/*
	 * Unregister near-side I2C clients.
	 */
	for (i = ARRAY_SIZE(i2c_clients); --i >= 0;) {
		c = i2c_clients[i];
		if (c)
			i2c_unregister_device(c);
	}
}

static bool
is_qsfp_port(int port)
{
	/*
	 * For now, include QSFP-DD ports here even tho we can't be
	 * sure which driver they use (maybe sff8436, maybe not).
	 * Right now that's all we use this test for.
	 */
	if (port >= 49)
		return true;
	else
		return false;
}

static int nextport(int port)
{
	int nextp;
	/*
	 * Dell S5248F QSFP port numbering is a bit odd at best.
	 *
	 * SFP ports are normal, numbered 1-48.
	 * QSFP ports are:  53, 54, 55, and 56.
	 * QSFP-DD ports have been provisionally added (there may
	 * be changes in the future to fully support 200G ports)
	 */
	if (port < 1 || port == 50 || port == 51 || port >= 56)
		return 0;
	
	if (port == 49)
		nextp = 52;
	else
		nextp = port + 1;

	return nextp;
}

/**
 * alloc_port_i2c_board_info -- Allocate an i2c_board_info struct
 * @port - front panel port number, one based
 *
 * For each port in the system allocate an i2c_board_info struct to
 * describe the SFP+ (at24)/QSFP (sff8436)/QSFP-DD (sff8436?) EEPROM.
 *
 * Returns the allocated i2c_board_info struct or else returns NULL on failure.
 */
#define EEPROM_LABEL_SIZE  8
static struct __init i2c_board_info * alloc_port_i2c_board_info(int port)
{
	char *label = NULL;
	struct eeprom_platform_data *eeprom_data = NULL;
	struct i2c_board_info *board_info = NULL;

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

	if (is_qsfp_port(port)) {
		struct sff_8436_platform_data *sff8436_data;

		sff8436_data = kzalloc(sizeof(*sff8436_data), GFP_KERNEL);
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

		at24_data = kzalloc(sizeof(*at24_data), GFP_KERNEL);
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
};

/* I2C Initialization */
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
		if (bus == CL_I2C_ISMT_BUS)
			bus = ismt_bus;
		else if (bus == CL_I2C_I801_BUS)
			bus = i801_bus;
		client = cumulus_i2c_add_client(bus,
						&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			pr_err("i2c_init: cum_i2c_add_cli(%d,%s) failed: %d",
			       bus, (char *)&i2c_devices[i].board_info.type,
			       ret);
			goto err_exit;
		}
		i2c_clients[i] = client;
	}

	pr_info("I2C driver loaded\n");
	return 0;

err_exit:
	i2c_exit();
	return ret;
}

/*
 *
 * FPGA Interface
 *
 */

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
	u8 __iomem *pbar;		/* PCIe BAR virtual addr */
	struct pci_dev *pci_dev;
	struct fpga_desc *hw;		/* FPGA descriptor virtual base addr */
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
 * OpenCores PCI mapping data and register accessors
 */

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
mk_bf_rw(fpga, port1_tx_enable,	 0x4000, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port1_tx_fault,	 0x4004, 2,  1, NULL, 0);
mk_bf_ro(fpga, port1_rx_los,	 0x4004, 1,  1, NULL, 0);
mk_bf_ro(fpga, port1_present,	 0x4004, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port2_tx_enable,	 0x4010, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port2_tx_fault,	 0x4014, 2,  1, NULL, 0);
mk_bf_ro(fpga, port2_rx_los,	 0x4014, 1,  1, NULL, 0);
mk_bf_ro(fpga, port2_present,	 0x4014, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port3_tx_enable,	 0x4020, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port3_tx_fault,	 0x4024, 2,  1, NULL, 0);
mk_bf_ro(fpga, port3_rx_los,	 0x4024, 1,  1, NULL, 0);
mk_bf_ro(fpga, port3_present,	 0x4024, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port4_tx_enable,	 0x4030, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port4_tx_fault,	 0x4034, 2,  1, NULL, 0);
mk_bf_ro(fpga, port4_rx_los,	 0x4034, 1,  1, NULL, 0);
mk_bf_ro(fpga, port4_present,	 0x4034, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port5_tx_enable,	 0x4040, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port5_tx_fault,	 0x4044, 2,  1, NULL, 0);
mk_bf_ro(fpga, port5_rx_los,	 0x4044, 1,  1, NULL, 0);
mk_bf_ro(fpga, port5_present,	 0x4044, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port6_tx_enable,	 0x4050, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port6_tx_fault,	 0x4054, 2,  1, NULL, 0);
mk_bf_ro(fpga, port6_rx_los,	 0x4054, 1,  1, NULL, 0);
mk_bf_ro(fpga, port6_present,	 0x4054, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port7_tx_enable,	 0x4060, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port7_tx_fault,	 0x4064, 2,  1, NULL, 0);
mk_bf_ro(fpga, port7_rx_los,	 0x4064, 1,  1, NULL, 0);
mk_bf_ro(fpga, port7_present,	 0x4064, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port8_tx_enable,	 0x4070, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port8_tx_fault,	 0x4074, 2,  1, NULL, 0);
mk_bf_ro(fpga, port8_rx_los,	 0x4074, 1,  1, NULL, 0);
mk_bf_ro(fpga, port8_present,	 0x4074, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port9_tx_enable,	 0x4080, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port9_tx_fault,	 0x4084, 2,  1, NULL, 0);
mk_bf_ro(fpga, port9_rx_los,	 0x4084, 1,  1, NULL, 0);
mk_bf_ro(fpga, port9_present,	 0x4084, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port10_tx_enable, 0x4090, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port10_tx_fault,	 0x4094, 2,  1, NULL, 0);
mk_bf_ro(fpga, port10_rx_los,	 0x4094, 1,  1, NULL, 0);
mk_bf_ro(fpga, port10_present,	 0x4094, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port11_tx_enable, 0x40a0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port11_tx_fault,	 0x40a4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port11_rx_los,	 0x40a4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port11_present,	 0x40a4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port12_tx_enable, 0x40b0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port12_tx_fault,	 0x40b4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port12_rx_los,	 0x40b4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port12_present,	 0x40b4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port13_tx_enable, 0x40c0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port13_tx_fault,	 0x40c4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port13_rx_los,	 0x40c4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port13_present,	 0x40c4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port14_tx_enable, 0x40d0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port14_tx_fault,	 0x40d4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port14_rx_los,	 0x40d4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port14_present,	 0x40d4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port15_tx_enable, 0x40e0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port15_tx_fault,	 0x40e4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port15_rx_los,	 0x40e4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port15_present,	 0x40e4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port16_tx_enable, 0x40f0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port16_tx_fault,	 0x40f4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port16_rx_los,	 0x40f4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port16_present,	 0x40f4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port17_tx_enable, 0x4100, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port17_tx_fault,	 0x4104, 2,  1, NULL, 0);
mk_bf_ro(fpga, port17_rx_los,	 0x4104, 1,  1, NULL, 0);
mk_bf_ro(fpga, port17_present,	 0x4104, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port18_tx_enable, 0x4110, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port18_tx_fault,	 0x4114, 2,  1, NULL, 0);
mk_bf_ro(fpga, port18_rx_los,	 0x4114, 1,  1, NULL, 0);
mk_bf_ro(fpga, port18_present,	 0x4114, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port19_tx_enable, 0x4120, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port19_tx_fault,	 0x4124, 2,  1, NULL, 0);
mk_bf_ro(fpga, port19_rx_los,	 0x4124, 1,  1, NULL, 0);
mk_bf_ro(fpga, port19_present,	 0x4124, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port20_tx_enable, 0x4130, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port20_tx_fault,	 0x4134, 2,  1, NULL, 0);
mk_bf_ro(fpga, port20_rx_los,	 0x4134, 1,  1, NULL, 0);
mk_bf_ro(fpga, port20_present,	 0x4134, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port21_tx_enable, 0x4140, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port21_tx_fault,	 0x4144, 2,  1, NULL, 0);
mk_bf_ro(fpga, port21_rx_los,	 0x4144, 1,  1, NULL, 0);
mk_bf_ro(fpga, port21_present,	 0x4144, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port22_tx_enable, 0x4150, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port22_tx_fault,	 0x4154, 2,  1, NULL, 0);
mk_bf_ro(fpga, port22_rx_los,	 0x4154, 1,  1, NULL, 0);
mk_bf_ro(fpga, port22_present,	 0x4154, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port23_tx_enable, 0x4160, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port23_tx_fault,	 0x4164, 2,  1, NULL, 0);
mk_bf_ro(fpga, port23_rx_los,	 0x4164, 1,  1, NULL, 0);
mk_bf_ro(fpga, port23_present,	 0x4164, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port24_tx_enable, 0x4170, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port24_tx_fault,	 0x4174, 2,  1, NULL, 0);
mk_bf_ro(fpga, port24_rx_los,	 0x4174, 1,  1, NULL, 0);
mk_bf_ro(fpga, port24_present,	 0x4174, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port25_tx_enable, 0x4180, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port25_tx_fault,	 0x4184, 2,  1, NULL, 0);
mk_bf_ro(fpga, port25_rx_los,	 0x4184, 1,  1, NULL, 0);
mk_bf_ro(fpga, port25_present,	 0x4184, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port26_tx_enable, 0x4190, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port26_tx_fault,	 0x4194, 2,  1, NULL, 0);
mk_bf_ro(fpga, port26_rx_los,	 0x4194, 1,  1, NULL, 0);
mk_bf_ro(fpga, port26_present,	 0x4194, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port27_tx_enable, 0x41a0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port27_tx_fault,	 0x41a4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port27_rx_los,	 0x41a4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port27_present,	 0x41a4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port28_tx_enable, 0x41b0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port28_tx_fault,	 0x41b4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port28_rx_los,	 0x41b4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port28_present,	 0x41b4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port29_tx_enable, 0x41c0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port29_tx_fault,	 0x41c4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port29_rx_los,	 0x41c4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port29_present,	 0x41c4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port30_tx_enable, 0x41d0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port30_tx_fault,	 0x41d4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port30_rx_los,	 0x41d4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port30_present,	 0x41d4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port31_tx_enable, 0x41e0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port31_tx_fault,	 0x41e4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port31_rx_los,	 0x41e4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port31_present,	 0x41e4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port32_tx_enable, 0x41f0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port32_tx_fault,	 0x41f4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port32_rx_los,	 0x41f4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port32_present,	 0x41f4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port33_tx_enable, 0x4200, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port33_tx_fault,	 0x4204, 2,  1, NULL, 0);
mk_bf_ro(fpga, port33_rx_los,	 0x4204, 1,  1, NULL, 0);
mk_bf_ro(fpga, port33_present,	 0x4204, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port34_tx_enable, 0x4210, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port34_tx_fault,	 0x4214, 2,  1, NULL, 0);
mk_bf_ro(fpga, port34_rx_los,	 0x4214, 1,  1, NULL, 0);
mk_bf_ro(fpga, port34_present,	 0x4214, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port35_tx_enable, 0x4220, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port35_tx_fault,	 0x4224, 2,  1, NULL, 0);
mk_bf_ro(fpga, port35_rx_los,	 0x4224, 1,  1, NULL, 0);
mk_bf_ro(fpga, port35_present,	 0x4224, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port36_tx_enable, 0x4230, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port36_tx_fault,	 0x4234, 2,  1, NULL, 0);
mk_bf_ro(fpga, port36_rx_los,	 0x4234, 1,  1, NULL, 0);
mk_bf_ro(fpga, port36_present,	 0x4234, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port37_tx_enable, 0x4240, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port37_tx_fault,	 0x4244, 2,  1, NULL, 0);
mk_bf_ro(fpga, port37_rx_los,	 0x4244, 1,  1, NULL, 0);
mk_bf_ro(fpga, port37_present,	 0x4244, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port38_tx_enable, 0x4250, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port38_tx_fault,	 0x4254, 2,  1, NULL, 0);
mk_bf_ro(fpga, port38_rx_los,	 0x4254, 1,  1, NULL, 0);
mk_bf_ro(fpga, port38_present,	 0x4254, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port39_tx_enable, 0x4260, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port39_tx_fault,	 0x4264, 2,  1, NULL, 0);
mk_bf_ro(fpga, port39_rx_los,	 0x4264, 1,  1, NULL, 0);
mk_bf_ro(fpga, port39_present,	 0x4264, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port40_tx_enable, 0x4270, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port40_tx_fault,	 0x4274, 2,  1, NULL, 0);
mk_bf_ro(fpga, port40_rx_los,	 0x4274, 1,  1, NULL, 0);
mk_bf_ro(fpga, port40_present,	 0x4274, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port41_tx_enable, 0x4280, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port41_tx_fault,	 0x4284, 2,  1, NULL, 0);
mk_bf_ro(fpga, port41_rx_los,	 0x4284, 1,  1, NULL, 0);
mk_bf_ro(fpga, port41_present,	 0x4284, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port42_tx_enable, 0x4290, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port42_tx_fault,	 0x4294, 2,  1, NULL, 0);
mk_bf_ro(fpga, port42_rx_los,	 0x4294, 1,  1, NULL, 0);
mk_bf_ro(fpga, port42_present,	 0x4294, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port43_tx_enable, 0x42a0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port43_tx_fault,	 0x42a4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port43_rx_los,	 0x42a4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port43_present,	 0x42a4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port44_tx_enable, 0x42b0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port44_tx_fault,	 0x42b4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port44_rx_los,	 0x42b4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port44_present,	 0x42b4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port45_tx_enable, 0x42c0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port45_tx_fault,	 0x42c4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port45_rx_los,	 0x42c4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port45_present,	 0x42c4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port46_tx_enable, 0x42d0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port46_tx_fault,	 0x42d4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port46_rx_los,	 0x42d4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port46_present,	 0x42d4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port47_tx_enable, 0x42e0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port47_tx_fault,	 0x42e4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port47_rx_los,	 0x42e4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port47_present,	 0x42e4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port48_tx_enable, 0x42f0, 0,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port48_tx_fault,	 0x42f4, 2,  1, NULL, 0);
mk_bf_ro(fpga, port48_rx_los,	 0x42f4, 1,  1, NULL, 0);
mk_bf_ro(fpga, port48_present,	 0x42f4, 0,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port49_lpmode,	 0x4300, 6,  1, NULL, 0);
mk_bf_rw(fpga, port49_modsel,	 0x4300, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port49_reset,	 0x4300, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port49_present,	 0x4304, 4,  1, NULL, BF_COMPLEMENT);

/* note: port51 would have been more logical, but silkscreen says "52" */
mk_bf_rw(fpga, port52_lpmode,	 0x4320, 6,  1, NULL, 0);
mk_bf_rw(fpga, port52_modsel,	 0x4320, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port52_reset,	 0x4320, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port52_present,	 0x4324, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port53_lpmode,	 0x4340, 6,  1, NULL, 0);
mk_bf_rw(fpga, port53_modsel,	 0x4340, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port53_reset,	 0x4340, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port53_present,	 0x4344, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port54_lpmode,	 0x4350, 6,  1, NULL, 0);
mk_bf_rw(fpga, port54_modsel,	 0x4350, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port54_reset,	 0x4350, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port54_present,	 0x4354, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port55_lpmode,	 0x4360, 6,  1, NULL, 0);
mk_bf_rw(fpga, port55_modsel,	 0x4360, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port55_reset,	 0x4360, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port55_present,	 0x4364, 4,  1, NULL, BF_COMPLEMENT);

mk_bf_rw(fpga, port56_lpmode,	 0x4370, 6,  1, NULL, 0);
mk_bf_rw(fpga, port56_modsel,	 0x4370, 5,  1, NULL, BF_COMPLEMENT);
mk_bf_rw(fpga, port56_reset,	 0x4370, 4,  1, NULL, BF_COMPLEMENT);
mk_bf_ro(fpga, port56_present,	 0x4374, 4,  1, NULL, BF_COMPLEMENT);

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

	&fpga_port1_tx_enable.attr,
	&fpga_port1_tx_fault.attr,
	&fpga_port1_rx_los.attr,
	&fpga_port1_present.attr,
	&fpga_port2_tx_enable.attr,
	&fpga_port2_tx_fault.attr,
	&fpga_port2_rx_los.attr,
	&fpga_port2_present.attr,
	&fpga_port3_tx_enable.attr,
	&fpga_port3_tx_fault.attr,
	&fpga_port3_rx_los.attr,
	&fpga_port3_present.attr,
	&fpga_port4_tx_enable.attr,
	&fpga_port4_tx_fault.attr,
	&fpga_port4_rx_los.attr,
	&fpga_port4_present.attr,
	&fpga_port5_tx_enable.attr,
	&fpga_port5_tx_fault.attr,
	&fpga_port5_rx_los.attr,
	&fpga_port5_present.attr,
	&fpga_port6_tx_enable.attr,
	&fpga_port6_tx_fault.attr,
	&fpga_port6_rx_los.attr,
	&fpga_port6_present.attr,
	&fpga_port7_tx_enable.attr,
	&fpga_port7_tx_fault.attr,
	&fpga_port7_rx_los.attr,
	&fpga_port7_present.attr,
	&fpga_port8_tx_enable.attr,
	&fpga_port8_tx_fault.attr,
	&fpga_port8_rx_los.attr,
	&fpga_port8_present.attr,
	&fpga_port9_tx_enable.attr,
	&fpga_port9_tx_fault.attr,
	&fpga_port9_rx_los.attr,
	&fpga_port9_present.attr,
	&fpga_port10_tx_enable.attr,
	&fpga_port10_tx_fault.attr,
	&fpga_port10_rx_los.attr,
	&fpga_port10_present.attr,
	&fpga_port11_tx_enable.attr,
	&fpga_port11_tx_fault.attr,
	&fpga_port11_rx_los.attr,
	&fpga_port11_present.attr,
	&fpga_port12_tx_enable.attr,
	&fpga_port12_tx_fault.attr,
	&fpga_port12_rx_los.attr,
	&fpga_port12_present.attr,
	&fpga_port13_tx_enable.attr,
	&fpga_port13_tx_fault.attr,
	&fpga_port13_rx_los.attr,
	&fpga_port13_present.attr,
	&fpga_port14_tx_enable.attr,
	&fpga_port14_tx_fault.attr,
	&fpga_port14_rx_los.attr,
	&fpga_port14_present.attr,
	&fpga_port15_tx_enable.attr,
	&fpga_port15_tx_fault.attr,
	&fpga_port15_rx_los.attr,
	&fpga_port15_present.attr,
	&fpga_port16_tx_enable.attr,
	&fpga_port16_tx_fault.attr,
	&fpga_port16_rx_los.attr,
	&fpga_port16_present.attr,
	&fpga_port17_tx_enable.attr,
	&fpga_port17_tx_fault.attr,
	&fpga_port17_rx_los.attr,
	&fpga_port17_present.attr,
	&fpga_port18_tx_enable.attr,
	&fpga_port18_tx_fault.attr,
	&fpga_port18_rx_los.attr,
	&fpga_port18_present.attr,
	&fpga_port19_tx_enable.attr,
	&fpga_port19_tx_fault.attr,
	&fpga_port19_rx_los.attr,
	&fpga_port19_present.attr,
	&fpga_port20_tx_enable.attr,
	&fpga_port20_tx_fault.attr,
	&fpga_port20_rx_los.attr,
	&fpga_port20_present.attr,
	&fpga_port21_tx_enable.attr,
	&fpga_port21_tx_fault.attr,
	&fpga_port21_rx_los.attr,
	&fpga_port21_present.attr,
	&fpga_port22_tx_enable.attr,
	&fpga_port22_tx_fault.attr,
	&fpga_port22_rx_los.attr,
	&fpga_port22_present.attr,
	&fpga_port23_tx_enable.attr,
	&fpga_port23_tx_fault.attr,
	&fpga_port23_rx_los.attr,
	&fpga_port23_present.attr,
	&fpga_port24_tx_enable.attr,
	&fpga_port24_tx_fault.attr,
	&fpga_port24_rx_los.attr,
	&fpga_port24_present.attr,
	&fpga_port25_tx_enable.attr,
	&fpga_port25_tx_fault.attr,
	&fpga_port25_rx_los.attr,
	&fpga_port25_present.attr,
	&fpga_port26_tx_enable.attr,
	&fpga_port26_tx_fault.attr,
	&fpga_port26_rx_los.attr,
	&fpga_port26_present.attr,
	&fpga_port27_tx_enable.attr,
	&fpga_port27_tx_fault.attr,
	&fpga_port27_rx_los.attr,
	&fpga_port27_present.attr,
	&fpga_port28_tx_enable.attr,
	&fpga_port28_tx_fault.attr,
	&fpga_port28_rx_los.attr,
	&fpga_port28_present.attr,
	&fpga_port29_tx_enable.attr,
	&fpga_port29_tx_fault.attr,
	&fpga_port29_rx_los.attr,
	&fpga_port29_present.attr,
	&fpga_port30_tx_enable.attr,
	&fpga_port30_tx_fault.attr,
	&fpga_port30_rx_los.attr,
	&fpga_port30_present.attr,
	&fpga_port31_tx_enable.attr,
	&fpga_port31_tx_fault.attr,
	&fpga_port31_rx_los.attr,
	&fpga_port31_present.attr,
	&fpga_port32_tx_enable.attr,
	&fpga_port32_tx_fault.attr,
	&fpga_port32_rx_los.attr,
	&fpga_port32_present.attr,
	&fpga_port33_tx_enable.attr,
	&fpga_port33_tx_fault.attr,
	&fpga_port33_rx_los.attr,
	&fpga_port33_present.attr,
	&fpga_port34_tx_enable.attr,
	&fpga_port34_tx_fault.attr,
	&fpga_port34_rx_los.attr,
	&fpga_port34_present.attr,
	&fpga_port35_tx_enable.attr,
	&fpga_port35_tx_fault.attr,
	&fpga_port35_rx_los.attr,
	&fpga_port35_present.attr,
	&fpga_port36_tx_enable.attr,
	&fpga_port36_tx_fault.attr,
	&fpga_port36_rx_los.attr,
	&fpga_port36_present.attr,
	&fpga_port37_tx_enable.attr,
	&fpga_port37_tx_fault.attr,
	&fpga_port37_rx_los.attr,
	&fpga_port37_present.attr,
	&fpga_port38_tx_enable.attr,
	&fpga_port38_tx_fault.attr,
	&fpga_port38_rx_los.attr,
	&fpga_port38_present.attr,
	&fpga_port39_tx_enable.attr,
	&fpga_port39_tx_fault.attr,
	&fpga_port39_rx_los.attr,
	&fpga_port39_present.attr,
	&fpga_port40_tx_enable.attr,
	&fpga_port40_tx_fault.attr,
	&fpga_port40_rx_los.attr,
	&fpga_port40_present.attr,
	&fpga_port41_tx_enable.attr,
	&fpga_port41_tx_fault.attr,
	&fpga_port41_rx_los.attr,
	&fpga_port41_present.attr,
	&fpga_port42_tx_enable.attr,
	&fpga_port42_tx_fault.attr,
	&fpga_port42_rx_los.attr,
	&fpga_port42_present.attr,
	&fpga_port43_tx_enable.attr,
	&fpga_port43_tx_fault.attr,
	&fpga_port43_rx_los.attr,
	&fpga_port43_present.attr,
	&fpga_port44_tx_enable.attr,
	&fpga_port44_tx_fault.attr,
	&fpga_port44_rx_los.attr,
	&fpga_port44_present.attr,
	&fpga_port45_tx_enable.attr,
	&fpga_port45_tx_fault.attr,
	&fpga_port45_rx_los.attr,
	&fpga_port45_present.attr,
	&fpga_port46_tx_enable.attr,
	&fpga_port46_tx_fault.attr,
	&fpga_port46_rx_los.attr,
	&fpga_port46_present.attr,
	&fpga_port47_tx_enable.attr,
	&fpga_port47_tx_fault.attr,
	&fpga_port47_rx_los.attr,
	&fpga_port47_present.attr,
	&fpga_port48_tx_enable.attr,
	&fpga_port48_tx_fault.attr,
	&fpga_port48_rx_los.attr,
	&fpga_port48_present.attr,
	&fpga_port49_lpmode.attr,
	&fpga_port49_modsel.attr,
	&fpga_port49_reset.attr,
	&fpga_port49_present.attr,
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
 * The Dell S5248F has register blocks allocated to manage 16 FPGA I2C busses
 * I2CH1-I2C16 via the OpenCores specification:
 * Rev 0.9, I2C-Master Core Specification, July 3 2003
 *
 * The offsets for these registers for the Dell FPGA are given in:
 * Rev 11, Z9XXX/S52XX Programmable Logic Design Doc, June 11 2018
 * Section 4.3.
 *
 * Sizing ocores_resources to NUM_FPGA_BUSSES+1 allows for direct indexing to
 * 1-based FPGA bus resources.  Entry 0 is empty and is not used.
 */
#define NUM_FPGA_BUSSES	16
static struct resource ocores_resources[NUM_FPGA_BUSSES + 1];
static struct resource ctrl_resource;

static int
fpga_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct fpga_priv *priv;
	struct ocores_i2c_device_info *info;
	struct resource *ores, *cres;
	struct ocores_i2c_platform_dev_info *muxdev;
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

	/* Determine the physical address of the FPGA BAR area */
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
	 * Initialise a ctrl resource for various LED and
	 * SFP/QSFP reset/modsel etc. registers in the physical
	 * memory range 0x0000 - 0x5FFF.
	 */
	cres = &ctrl_resource;
	cres->start = start;
	cres->end   = cres->start + 0x5FFF;
	cres->flags = IORESOURCE_MEM;

	priv->pbar = devm_ioremap_resource(&pdev->dev, cres);
	if (!priv->pbar) {
		pr_err("devm_ioremap_resource failed for cres");
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
	 * Initialise ocores_resources[] for all FPGA busses.
	 * The device slice loop will index into this array to
	 * get platform device resources.
	 *
	 * FPGA I2C_CH1  gets start 0x6000, end 0x600F
	 * FPGA I2C_CH1  gets start 0x6010, end 0x601F
	 * ...
	 * FPGA I2C_CH16 gets start 0x60F0, end 0x60FF
	 */
	ores = ocores_resources;
	ores++;		/* skip over 0th entry - it's never used */
	for (i = 1; i < ARRAY_SIZE(ocores_resources); i++, ores++) {
		ores->start = start + 0x6000 + ((i - 1) * 0x10);
		ores->end   = ores->start + 0xF;
		ores->flags = IORESOURCE_MEM;
	}

	/*
	 * The device slice loop.  Each of the devices in
	 * ocores_i2c_device_infotab[] carves out its own name, resources,
	 * and private data and is initialised here.  At the end, each
	 * infotab device registers with the i2c-ocores driver to share
	 * its idea of its resources.  We therefore get one i2c-ocores
	 * driver instance for each device in ocores_i2c_device_infotab[].
	 *
	 * Tacit assumption in multiple places in this slice loop is that
	 * we are registering the minimum we can get away with, which is
	 * each one of the root FPGA I2C busses/adapters 4-10 inclusive.
	 * Everything else can be added from dellemc_s5248f_init() afterward.
	 *
	 * ocores_i2c_device_infotab[], ocores_resources[], ocores_devtab[],
	 * and ocores_i2c_data[] had better all have the same array size as
	 * ocores_i2c_mux_devices[]!
	 */
	muxdev = ocores_i2c_mux_devices;
	info   = ocores_i2c_device_infotab;
	for (i = 0;
	     i < ARRAY_SIZE(ocores_i2c_device_infotab);
	     i++, muxdev++, info++) {
		platdev = platform_device_alloc(OCORES_DRV_NAME, i);
		if (!platdev) {
			pr_err("platform_device_alloc(%d) failed", i);
			err = -ENOMEM;
			sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
			devm_iounmap(&pdev->dev, priv->pbar);
			fpga_dev_release(priv);
			pci_disable_device(pdev);
			devm_kfree(&pdev->dev, priv);
			goto fail;
		}

		ores = &ocores_resources[muxdev->bus];
		err = platform_device_add_resources(platdev, ores, 1);
		if (err) {
			pr_err("platform_device_add_resources(%d) failed", i);
			sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
			devm_iounmap(&pdev->dev, priv->pbar);
			fpga_dev_release(priv);
			pci_disable_device(pdev);
			devm_kfree(&pdev->dev, priv);
			goto fail;
		}

		info->bus = muxdev->bus;
		info->info = &muxdev->board_info;

		oipd = &ocores_i2c_data[i];
		oipd->clock_khz		= 100000;
		oipd->devices		= info;
		oipd->num_devices	= 1;
		oipd->interrupt_mode	= OCI2C_POLL;

		err = platform_device_add_data(platdev, oipd, sizeof(*oipd));
		if (err) {
			pr_err("platform_device_add_data(%d) failed", i);
			sysfs_remove_group(&pdev->dev.kobj, &fpga_attr_group);
			devm_iounmap(&pdev->dev, priv->pbar);
			fpga_dev_release(priv);
			pci_disable_device(pdev);
			devm_kfree(&pdev->dev, priv);
			goto fail;
		}

		err = platform_device_add(platdev);
		if (err) {
			pr_err("platform_device_add() failed for ocores %d", i);
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
	.name = "dellemc_s5248f_fpga",
	.id_table = fpga_id,
	.probe = fpga_probe,
	.remove = fpga_remove,
	.suspend = fpga_suspend,
	.resume = fpga_resume,
};

static int fpga_init(void)
{
	struct ocores_i2c_platform_dev_info *oipdi;
	struct i2c_client *client;
	int i;
	int ret;
	int port;

	ret = pci_register_driver(&fpga_driver);
	if (ret) {
		pr_err("pci_register_driver() failed for fpga device\n");
		return ret;
	}

	/*
	 * Alloc SFP/QSFP/QSFP-DD device EEPROM blocks.  i2c-ocores has
	 * by now read the board info and created the I2C adapters; we
	 * can now create the clients.
	 *
	 * XXX: This loop presumes that ports are declared in asccending
	 *	order in ocores_i2c_devices[].
	 * XXX: This loop presumes that only muxes/switches and ports
	 *	are declared for bus CL_I2C_FPGA_BUS4 and higher.  If
	 *	other devices are added on these busses later, the ports
	 *	will probably have to be separated out into their own
	 *	table for ease of processing as is done on other platforms.
	 */
	port = 1;
	oipdi = ocores_i2c_devices;
	for (i = 0; i < ARRAY_SIZE(ocores_i2c_devices); i++, oipdi++) {
		struct i2c_board_info *b_info;

					/* skip to SFP/QSFP ports */
		if (oipdi->bus < CL_I2C_FPGA_BUS4_0)
			continue;

		b_info = alloc_port_i2c_board_info(port);
		if (!b_info) {
			ret = -ENOMEM;
			pr_err("fpga_init: port allocation failed for bus %d",
			       oipdi->bus);
			goto err_exit;
		}
		oipdi->board_info = *b_info;

		client = cumulus_i2c_add_client(oipdi->bus, &oipdi->board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			pr_err("fpga_init: cum_i2c_add_cli(%d,%s) failed: %d",
			       oipdi->bus, (char *)&oipdi->board_info.type,
			       ret);
			goto err_exit;
		}
		ocores_i2c_clients[i] = client;

		port = nextport(port);
	}

	pr_info("FPGA driver registered\n");
	return 0;

err_exit:
	pr_err("Error, FPGA driver NOT registered\n");
	return -1;
}

static void fpga_exit(void)
{
	pci_unregister_driver(&fpga_driver);
	pr_debug("FPGA driver unregistered\n");
}

/*-----------------------------------------------------------------------------
 *
 * Module init/exit
 */
static int __init dellemc_s5248f_init(void)
{
	int ret;

	ret = i2c_init();
	if (ret) {
		pr_err("dellemc_s5248f_init: I2C subsystem initialization failed\n");
		i2c_exit();
		return ret;
	}

	ret = fpga_init();
	if (ret) {
		pr_err("dellemc_s5248f_init: FPGA initialization failed\n");
		fpga_exit();
		i2c_exit();
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION
		" successfully initialized\n");

	return 0;
}

static void __exit dellemc_s5248f_exit(void)
{
	i2c_exit();
	fpga_exit();
}

module_init(dellemc_s5248f_init);
module_exit(dellemc_s5248f_exit);

MODULE_AUTHOR("Frank Hoeflich (frankh@cumulusnetworks.com)");
MODULE_DESCRIPTION("Dell EMC S5248F Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
