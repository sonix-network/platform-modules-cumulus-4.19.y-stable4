/*
 * dell_s6000_platform.c - DELL S6000-S1220 Platform Support.
 *
 * Author: Curt Brune (curt@cumulusnetworks.com)
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
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/pci.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/i2c-mux-gpio.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/cumulus-platform.h>

#include "platform-defs.h"
#include "dell-s6000-cpld.h"

#define DRIVER_NAME	"dell_s6000"
#define DRIVER_VERSION	"0.1"

/*
 * The platform has 2 types of i2c SMBUSes, ISCH (Intel SCH chipset SMbus)
 * and ISMT (Intel SMBus Message Transport).  ISCH has a mux that is controlled
 * by 2 gpio pins and has 4 channels. The devices on the channels are thermal
 * sensors, system EEPROM and the CPLDs. There are 2 ISMT busses, which are
 * backed by PCIe address 0:13.0 and 0:13.1. We are interested in 0:13.1 as that
 * has the PSU EEPROMs and PMBUS access.
 */

/* GPIO signal numbers */
enum {
	GPIO_ISCH_I2CMUX_0=1,
	GPIO_ISCH_I2CMUX_1,
};

/*
 * GPIO signals used by the mux.
 */
static const unsigned dell_s6000_isch_gpiomux_gpios[] = {
	GPIO_ISCH_I2CMUX_0,
	GPIO_ISCH_I2CMUX_1
};

/*
 * Bitmask of GPIO signals used to enable a downstream i2c bus.
 * 2-bits are used to control the mux:
 *
 *   2b00 -- first downstream bus
 *   2b01 -- second downstream bus
 *   2b10 -- third downstream bus
 *   2b11 -- fourth downstream bus
 */
static const unsigned dell_s6000_isch_gpiomux_values[] = {
	0,  /* 1st downstream bus */
	1,  /* 2nd downstream bus */
	2,  /* 3rd downstream bus */
	3   /* 4th downstream bus */
};

/* i2c bus adapter numbers for the 4 down stream i2c busses */
enum {
	DELL_S6000_ISCH_I2CMUX_BUS_0 = 10,
	DELL_S6000_ISCH_I2CMUX_BUS_1,
	DELL_S6000_ISCH_I2CMUX_BUS_2,
	DELL_S6000_ISCH_I2CMUX_BUS_3,
	DELL_S6000_QSFPMUX_PORT_1 = 20,
	DELL_S6000_QSFPMUX_PORT_2,
	DELL_S6000_QSFPMUX_PORT_3,
	DELL_S6000_QSFPMUX_PORT_4,
	DELL_S6000_QSFPMUX_PORT_5,
	DELL_S6000_QSFPMUX_PORT_6,
	DELL_S6000_QSFPMUX_PORT_7,
	DELL_S6000_QSFPMUX_PORT_8,
	DELL_S6000_QSFPMUX_PORT_9,
	DELL_S6000_QSFPMUX_PORT_10,
	DELL_S6000_QSFPMUX_PORT_11,
	DELL_S6000_QSFPMUX_PORT_12,
	DELL_S6000_QSFPMUX_PORT_13,
	DELL_S6000_QSFPMUX_PORT_14,
	DELL_S6000_QSFPMUX_PORT_15,
	DELL_S6000_QSFPMUX_PORT_16,
	DELL_S6000_QSFPMUX_PORT_17,
	DELL_S6000_QSFPMUX_PORT_18,
	DELL_S6000_QSFPMUX_PORT_19,
	DELL_S6000_QSFPMUX_PORT_20,
	DELL_S6000_QSFPMUX_PORT_21,
	DELL_S6000_QSFPMUX_PORT_22,
	DELL_S6000_QSFPMUX_PORT_23,
	DELL_S6000_QSFPMUX_PORT_24,
	DELL_S6000_QSFPMUX_PORT_25,
	DELL_S6000_QSFPMUX_PORT_26,
	DELL_S6000_QSFPMUX_PORT_27,
	DELL_S6000_QSFPMUX_PORT_28,
	DELL_S6000_QSFPMUX_PORT_29,
	DELL_S6000_QSFPMUX_PORT_30,
	DELL_S6000_QSFPMUX_PORT_31,
	DELL_S6000_QSFPMUX_PORT_32,
};

static struct i2c_mux_gpio_platform_data dell_s6000_isch_i2cmux_data = {
	.parent		= -1,
	.base_nr	= DELL_S6000_ISCH_I2CMUX_BUS_0, /* starting bus number */
	.values		= dell_s6000_isch_gpiomux_values,
	.n_values	= ARRAY_SIZE(dell_s6000_isch_gpiomux_values),
	.gpios		= dell_s6000_isch_gpiomux_gpios,
	.n_gpios	= ARRAY_SIZE(dell_s6000_isch_gpiomux_gpios),
	.idle		= I2C_MUX_GPIO_NO_IDLE, /* one downstream bus is always enabled */
};

static struct platform_device *dell_s6000_isch_i2cmux;

mk_eeprom(spd  , 50, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board, 53, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu1,  51, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu2,  50, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan1,  51, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan2,  52, 256, AT24_FLAG_IRUGO);
mk_eeprom(fan3,  53, 256, AT24_FLAG_IRUGO);

/* i2c_board_info information for i2c-mux bus 0 */
static struct i2c_board_info __initdata isch_i2cmux_0_board_info[] = {
	{
		I2C_BOARD_INFO("dummy", 0x31), /* System CPLD */
	},
	{
		I2C_BOARD_INFO("dummy", 0x32), /* Master CPLD */
	},
	{
		I2C_BOARD_INFO("dummy", 0x33), /* Slave CPLD */
	},
	{
		I2C_BOARD_INFO("unknown", 0x3e), /* XDCPtm ISL90728WIE627Z */
	},
	{
		I2C_BOARD_INFO("emc1403", 0x4d), /* EMC1428 Temperature Sensor */
	},
	{
		I2C_BOARD_INFO("spd", 0x50),  /* DIMM */
		.platform_data = &spd_50_at24,
	},
	{
		I2C_BOARD_INFO("24c02", 0x53), /* 256-byte Board EEPROM */
		.platform_data = &board_53_at24,
	},
#if 0
	{
		I2C_BOARD_INFO("unknown", 0x68), /* USB Controller - PI7C9X442SL */
	},
	{
		I2C_BOARD_INFO("unknown", 0x69), /* 9VRS4339B Clock Gen */
	},
#endif
};

static struct i2c_board_info __initdata isch_i2cmux_1_board_info[] = {
	{
		I2C_BOARD_INFO("tmp75", 0x4c), /* TMP75 Temperature Sensor */
	},
	{
		I2C_BOARD_INFO("tmp75", 0x4d), /* TMP75 Temperature Sensor */
	},
	{
		I2C_BOARD_INFO("tmp75", 0x4e), /* TMP75 Temperature Sensor */
	},
	{
		I2C_BOARD_INFO("max6620", 0x29), /* MAX6620 Fan controller */
	},
	{
		I2C_BOARD_INFO("max6620", 0x2a), /* MAX6620 Fan controller */
	},
#if 0
	{
		I2C_BOARD_INFO("ad5252", 0x2c), /* AD5252 variable resistor */
	},
#endif
	{
		I2C_BOARD_INFO("ltc4215", 0x40), /* LTC4215 PSU 2 monitor */
	},
	{
		I2C_BOARD_INFO("ltc4215", 0x42), /* LTC4215 PSU 1 monitor */
	},
	{
		I2C_BOARD_INFO("24c02", 0x51),   /* Fan 1 EEPROM */
		.platform_data = &fan1_51_at24,
	},
	{
		I2C_BOARD_INFO("24c02", 0x52),   /* Fan 2 EEPROM */
		.platform_data = &fan2_52_at24,
	},
	{
		I2C_BOARD_INFO("24c02", 0x53),   /* Fan 3 EEPROM */
		.platform_data = &fan3_53_at24,
	},
};

static struct i2c_board_info __initdata isch_i2cmux_2_board_info[] = {
};

static struct i2c_board_info __initdata isch_i2cmux_3_board_info[] = {
};

static struct i2c_board_info __initdata ismt_0_13_1_board_info[] = {
	{
		I2C_BOARD_INFO("dps460", 0x58),   /* PSU 2 PMBUS  */
	},
	{
		I2C_BOARD_INFO("dps460", 0x59),   /* PSU 1 PMBUS  */
	},
	{
		I2C_BOARD_INFO("24c02", 0x51),   /* PSU 1 EEPROM */
		.platform_data = &psu1_51_at24,
	},
	{
		I2C_BOARD_INFO("24c02", 0x50),   /* PSU 2 EEPROM */
		.platform_data = &psu2_50_at24,
	},
};

/**
 * Array of dynamically allocated i2c_client objects.  Need to track
 * these in order to free them later.
 *
 */
static struct i2c_client	*isch_i2cmux_0_clients[ARRAY_SIZE(isch_i2cmux_0_board_info)];
static struct i2c_client	*isch_i2cmux_1_clients[ARRAY_SIZE(isch_i2cmux_1_board_info)];
static struct i2c_client	*isch_i2cmux_2_clients[ARRAY_SIZE(isch_i2cmux_2_board_info)];
static struct i2c_client	*isch_i2cmux_3_clients[ARRAY_SIZE(isch_i2cmux_3_board_info)];
static struct i2c_client	*ismt_0_13_1_clients[ \
	                                 ARRAY_SIZE(ismt_0_13_1_board_info)];
/**
 * struct dell_s6000_i2c_devices -- i2c device container struct.  This
 * struct helps organize the i2c_board_info structures and the created
 * i2c_client objects.
 *
 * One i2c_client object is created for every device in the
 * i2c_board_info struct, i.e. the length of the @clients array is
 * equal to @n_devices.
 *
 */
struct dell_s6000_i2c_devices {
	int	bus;  /* i2c bus number for these devices */
	struct i2c_board_info	*devices; /* array of i2c devices to create */
	int	n_devices;  /* number of i2c devices */
	struct i2c_client	**clients; /* array of allocated i2c_client objects */
};

/**
 * i2c_devices -- array of struct dell_s6000_i2c_devices
 *
 * One struct dell_s6000_i2c_devices for each i2c bus.
 */
enum {
	I2C_DEV_ISCH_MUX0,
	I2C_DEV_ISCH_MUX1,
	I2C_DEV_ISCH_MUX2,
	I2C_DEV_ISCH_MUX3,
	I2C_DEV_ISMT_0_13_1,
};

/*
 * Note: Using __refdata here because the .devices member points to
 * __initdata that is only needed during module initialization.  The
 * other members are needed for the life of the module.  Using
 * __refdata teaches modpost build step that this reference is OK and
 * supresses a warning message.  See <linux/init.h> for more details.
 */

static struct dell_s6000_i2c_devices __refdata i2c_devices[] = {
	[I2C_DEV_ISCH_MUX0] = 	{
		.bus		= DELL_S6000_ISCH_I2CMUX_BUS_0,
		.devices	= isch_i2cmux_0_board_info,
		.n_devices	= ARRAY_SIZE(isch_i2cmux_0_board_info),
		.clients	= isch_i2cmux_0_clients,
	},
	[I2C_DEV_ISCH_MUX1] = 	{
		.bus		= DELL_S6000_ISCH_I2CMUX_BUS_1,
		.devices	= isch_i2cmux_1_board_info,
		.n_devices	= ARRAY_SIZE(isch_i2cmux_1_board_info),
		.clients	= isch_i2cmux_1_clients,
	},
	[I2C_DEV_ISCH_MUX2] = 	{
		.bus		= DELL_S6000_ISCH_I2CMUX_BUS_2,
		.devices	= isch_i2cmux_2_board_info,
		.n_devices	= ARRAY_SIZE(isch_i2cmux_2_board_info),
		.clients	= isch_i2cmux_2_clients,
	},
	[I2C_DEV_ISCH_MUX3] = 	{
		.bus		= DELL_S6000_ISCH_I2CMUX_BUS_3,
		.devices	= isch_i2cmux_3_board_info,
		.n_devices	= ARRAY_SIZE(isch_i2cmux_3_board_info),
		.clients	= isch_i2cmux_3_clients,
	},
	[I2C_DEV_ISMT_0_13_1] = {
		.bus            = 0xff,  // To be filled later
		.devices        = ismt_0_13_1_board_info,
		.n_devices      = ARRAY_SIZE(ismt_0_13_1_board_info),
		.clients        = ismt_0_13_1_clients,
	},
};

static int __init dell_s6000_cpld_init(struct dell_s6000_i2c_devices *device_list);
static void __exit dell_s6000_cpld_exit(void);

static int __init dell_s6000_qsfp_mux_init(void);
static void __exit dell_s6000_qsfp_mux_exit(void);

static int cpld_qsfp_mux_select_chan(struct i2c_mux_core *muxc, u32 chan);
static void qsfp_mux_free_data(void);

#define NUM_QSFP_MUXES             (2)
#define NUM_QSFP_CLIENTS_PER_MUX   (16)

struct qsfp_mux_info_item {
	struct platform_device *mux_dev;
	struct i2c_mux_core *muxc;
	struct i2c_client *qsfp_clients[NUM_QSFP_CLIENTS_PER_MUX];
	struct i2c_board_info *qsfp_board_infos[NUM_QSFP_CLIENTS_PER_MUX];
};

struct qsfp_mux_info_struct {
	struct qsfp_mux_info_item mux_item[NUM_QSFP_MUXES];
	int num_qsfp_muxes;
};

static struct qsfp_mux_info_struct *qsfp_mux_info;

/**
 * Invalid QSFP mux number
 *
 */
#define QSFP_MUX_INVALID_CHANNEL (0xDEADBEEF)

static struct i2c_adapter *get_adapter(int bus)
{
	int bail=20;
	struct i2c_adapter *adapter;

	for (; bail; bail--) {
		adapter = i2c_get_adapter(bus);
		if (adapter) {
			return adapter;
		}
		msleep(100);
	}
	return NULL;
}

/**
 * dell_s6000_i2c_init -- Initialize the I2C subsystem.
 *
 * This platform has a 4 chanenel i2c mux attached to the Centerton's
 * SMB1.0 interface.  The MUX is controlled by 2 GPIO signals from the
 * Centerton's integrated GPIO controller.
 *
 */

/* Name of the driver for the i2c bus the GPIO mux is a child of */
#define SMB1_NAME "SMBus SCH"
/* Name of the driver for iSMT devices */
#define SMB2_NAME "SMBus iSMT"

static int __init dell_s6000_i2c_init(void)
{
	int ret = 0;
	int i, j;
	int found_isch = 0, found_ismt_0_13_1 = 0;
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	struct pci_dev *pdev;

	/* Find the SMB1.0 i2c device.  This device will be the parent
	 * of the GPIO mux.
	 *
	 * Also find the SMBUS iSMT device where the PSU PMBUS device
	 * and eeprom reside.
	 * Loop through the first 10 i2c adapters looking for one
	 * whose name begins with "SMBus SCH".
	 *
	 */
	for (i = 0; i < 10; i++) {
		adapter = get_adapter(i);
		if (adapter) {
			if (!strncmp(adapter->name, SMB2_NAME, strlen(SMB2_NAME))) {
				pdev = to_pci_dev(adapter->dev.parent);
				/*
				 * devfn has 5 bits device id and 3 bit function id.
				 * The PSU PMBUS and EEPROM reside on PCI BDF 0:13.1.
				 */
				if (pdev && pdev->bus &&
				    (pdev->bus->number == 0) &&
				    (pdev->devfn == 0x99)) {
					i2c_devices[I2C_DEV_ISMT_0_13_1].bus = i;
					found_ismt_0_13_1 = 1;
				}
			}
			else if (!strncmp(adapter->name, SMB1_NAME, strlen(SMB1_NAME))) {
				dell_s6000_isch_i2cmux_data.parent = i;
				found_isch = 1;
			}
			i2c_put_adapter(adapter);
		}
		if (found_isch && found_ismt_0_13_1)
			break;
	}

	if (!found_isch) {
		pr_err("Unable to find SMB1.0 i2c adapter: %s\n", SMB1_NAME);
		return -ENXIO;
	}
	if (!found_ismt_0_13_1) {
		pr_err("Unable to find i2c adapter for ISMT bus at 0:13.1\n");
		return -ENXIO;
	}
	/* Create gpio-i2cmux device */
	dell_s6000_isch_i2cmux = platform_device_alloc("i2c-mux-gpio", 0);
	if (!dell_s6000_isch_i2cmux) {
		pr_err("platform_device_alloc() failed");
		return -ENOMEM;
	}
	ret = platform_device_add_data(dell_s6000_isch_i2cmux,
				       &dell_s6000_isch_i2cmux_data,
				       sizeof(dell_s6000_isch_i2cmux_data));
	if (ret) {
		pr_err("platform_device_add_data() failed");
		goto err_mux_put;
	}
	ret = platform_device_add(dell_s6000_isch_i2cmux);
	if (ret) {
		pr_err("platform_device_add() failed");
		goto err_mux_put;
	}

	qsfp_mux_info = kzalloc(sizeof(struct qsfp_mux_info_struct), GFP_KERNEL);
	if (!qsfp_mux_info) {
		return -ENOMEM;
	}

	/* Register board i2c devices. */
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++)
		for (j = 0; j < i2c_devices[i].n_devices; j++) {
			client = cumulus_i2c_add_client(i2c_devices[i].bus,
							&i2c_devices[i].devices[j]);
			if (IS_ERR(client)) {
				pr_err("i2c_new_device failed for qsfp mux.\n");
				ret = PTR_ERR(client);
				goto err_clients;
			}
			i2c_devices[i].clients[j] = client;
		}

	return 0;

err_clients:
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++)
		for (j = 0; j < i2c_devices[i].n_devices; j++)
			if (i2c_devices[i].clients[j])
				i2c_unregister_device(i2c_devices[i].clients[j]);

	platform_device_del(dell_s6000_isch_i2cmux);

err_mux_put:
	platform_device_put(dell_s6000_isch_i2cmux);
	return ret;
}

static void __exit dell_s6000_i2c_exit(void)
{
	int i, j;

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++)
		for (j = 0; j < i2c_devices[i].n_devices; j++)
			if (i2c_devices[i].clients[j])
				i2c_unregister_device(i2c_devices[i].clients[j]);

	platform_device_unregister(dell_s6000_isch_i2cmux);
}

/*---------------------------------------------------------------------
 *
 * QSFP mux
 *
 *  We have two i2c EEPROM devices, each one 'windows' in the EEPROM contents
 *  of 16 different ports (one at a time). To handle this, we insert a 16
 *  channel mux before each qsfp device so that in sysfs it looks like 16
 *  different EEPROMs.
 */
#define QSFP_LABEL_SIZE  8
static struct i2c_board_info *alloc_qsfp_board_info(int port) {
     char *label;
     struct eeprom_platform_data *eeprom_data;
     struct sff_8436_platform_data *sff8436_data;
     struct i2c_board_info *board_info;

     label = kzalloc(QSFP_LABEL_SIZE, GFP_KERNEL);
     if (!label) {
	  goto err_exit;
     }
     eeprom_data = kzalloc(sizeof(struct eeprom_platform_data), GFP_KERNEL);
     if (!eeprom_data) {
	  goto err_exit_eeprom;
     }
     sff8436_data = kzalloc(sizeof(struct sff_8436_platform_data), GFP_KERNEL);
     if (!sff8436_data) {
	  goto err_exit_sff8436;
     }
     board_info = kzalloc(sizeof(struct i2c_board_info), GFP_KERNEL);
     if (!board_info) {
	  goto err_exit_board;
     }

     snprintf(label, QSFP_LABEL_SIZE, "port%u", port);
     eeprom_data->label = label;

     sff8436_data->byte_len = 256;
     sff8436_data->flags = SFF_8436_FLAG_IRUGO;
     sff8436_data->page_size = 1;
     sff8436_data->eeprom_data = eeprom_data;

     strcpy(board_info->type, "sff8436");
     board_info->addr = 0x50;
     board_info->platform_data = sff8436_data;

     return board_info;

err_exit_board:
     kfree(sff8436_data);
err_exit_sff8436:
     kfree(eeprom_data);
err_exit_eeprom:
     kfree(label);
err_exit:
     return NULL;
};

static void free_qsfp_board_info(struct i2c_board_info *board_info)
{
     struct sff_8436_platform_data *sff8436_data = board_info->platform_data;
     struct eeprom_platform_data *eeprom_data = sff8436_data->eeprom_data;

     kfree(eeprom_data->label);
     kfree(eeprom_data);
     kfree(sff8436_data);
     kfree(board_info);
}

static int create_qsfp_mux(int bus, int first_port_num, int first_mux_num, int device_num)
{
     struct i2c_adapter *adapter;
     struct platform_device *mux_dev;
     struct qsfp_mux_info_item *mux_item;
     int ret = 0;
     int i;

     if (device_num >= NUM_QSFP_MUXES) {
	  pr_err("Attempt to create invalid QSFP mux %u.\n", device_num);
	  return -EINVAL;
     }
     adapter = get_adapter(bus);
     if (!adapter) {
	  pr_err("Could not find i2c adapter for QSFP mux bus %d.\n", bus);
	  return -ENODEV;
     }
     mux_dev = platform_device_alloc("cpld-qsfp-mux", device_num);
     if (!mux_dev) {
	  pr_err("platform_device_alloc() failed for qsfp mux %u.\n", device_num);
	  ret = -ENOMEM;
	  goto err_exit;
     }
     mux_item = &qsfp_mux_info->mux_item[qsfp_mux_info->num_qsfp_muxes++];
     mux_item->mux_dev = mux_dev;

     ret = platform_device_add(mux_dev);
     if (ret) {
	  pr_err("platform_device_add() failed for qsfp mux %u.\n", device_num);
	  ret = -ENOMEM;
	  goto err_exit;
     }

     mux_item->muxc = i2c_mux_alloc(adapter, &mux_dev->dev, NUM_QSFP_CLIENTS_PER_MUX,
				    0, 0, cpld_qsfp_mux_select_chan, NULL);
     if (!mux_item->muxc) {
	  pr_err("i2c_mux_alloc() failed for qsfp mux %u.\n", device_num);
	  ret = -ENOMEM;
	  goto err_exit;
     }

     for (i = 0; i < NUM_QSFP_CLIENTS_PER_MUX; i++) {
	  struct i2c_board_info *eeprom_info;
	  struct i2c_client *client;
	  int mux_num;

	  mux_num = first_mux_num + i;

	  ret = i2c_mux_add_adapter(mux_item->muxc, mux_num, mux_num, 0);
	  if (ret) {
	       pr_err("i2c_mux_add_adapter() failed for channel %u.\n", mux_num);
	       goto err_exit;
	  }

	  eeprom_info = alloc_qsfp_board_info(first_port_num + i);
	  if (!eeprom_info) {
	       ret = -ENOMEM;
	       goto err_exit;
	  }
	  mux_item->qsfp_board_infos[i] = eeprom_info;
	  client = i2c_new_device(mux_item->muxc->adapter[i], eeprom_info);
	  if (!client) {
	       pr_err("i2c_new_device failed for QSFP EEPROM device.\n");
	       ret = -ENOMEM;
	       goto err_exit;
	  }
	  mux_item->qsfp_clients[i] = client;
     }

err_exit:
     i2c_put_adapter(adapter);
     return ret;
}

static int __init dell_s6000_qsfp_mux_init(void)
{
     int ret;

     ret = create_qsfp_mux(DELL_S6000_ISCH_I2CMUX_BUS_2, 1, DELL_S6000_QSFPMUX_PORT_1, 0);
     if (ret) {
	  qsfp_mux_free_data();
	  goto exit;
     }

     ret = create_qsfp_mux(DELL_S6000_ISCH_I2CMUX_BUS_3, 17, DELL_S6000_QSFPMUX_PORT_17, 1);
     if (ret) {
	  qsfp_mux_free_data();
	  goto exit;
     }

exit:
     return ret;
}

static void __exit dell_s6000_qsfp_mux_exit(void)
{
     qsfp_mux_free_data();
}

static int __init dell_s6000_init(void)
{
	int ret = 0;

	ret = dell_s6000_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		return ret;
	}

	ret = dell_s6000_cpld_init(&i2c_devices[0]);
	if (ret) {
		pr_err("Registering CPLD driver failed.\n");
		return ret;
	}

	ret = dell_s6000_qsfp_mux_init();
	if (ret) {
		pr_err("Initializing QSFP mux failed.\n");
		return ret;
	}

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;
}

static void __exit dell_s6000_exit(void)
{
	dell_s6000_qsfp_mux_exit();
	dell_s6000_cpld_exit();
	dell_s6000_i2c_exit();
	pr_err("Driver unloaded\n");
}

static void qsfp_mux_free_data(void)
{
	int i, j;

	if (qsfp_mux_info) {
		for (i = 0; i < qsfp_mux_info->num_qsfp_muxes; i++) {
			for (j = 0; j < NUM_QSFP_CLIENTS_PER_MUX; j++) {
				if (qsfp_mux_info->mux_item[i].qsfp_clients[j]) {
					i2c_unregister_device(qsfp_mux_info->mux_item[i].qsfp_clients[j]);
				}
				if (qsfp_mux_info->mux_item[i].qsfp_board_infos[j]) {
				     free_qsfp_board_info(qsfp_mux_info->mux_item[i].qsfp_board_infos[j]);
				}
			}
			if (qsfp_mux_info->mux_item[i].muxc)
				i2c_mux_del_adapters(qsfp_mux_info->mux_item[i].muxc);
			platform_device_unregister(qsfp_mux_info->mux_item[i].mux_dev);
		}
		kfree(qsfp_mux_info);
		qsfp_mux_info = NULL;
	}
}

/*---------------------------------------------------------------------
 *
 * CPLD
 */
static struct i2c_client *cpld_i2c_clients[NUM_CPLD_I2C_CLIENTS];

static uint8_t cpld_reg_read(uint32_t reg)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	int val;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("attempt to read invalid CPLD register [0x%02X]", reg);
		return -EINVAL;
	}
	val = i2c_smbus_read_byte_data(cpld_i2c_clients[cpld_idx], STRIP_CPLD_IDX(reg));
	if (val < 0) {
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X", cpld_i2c_clients[cpld_idx]->addr, STRIP_CPLD_IDX(reg));
	}
	return val;
}

static int cpld_reg_write(uint32_t reg, uint8_t write_val)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	int res;
	struct i2c_client *client;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("attempt to write to invalid CPLD register [0x%02X]", reg);
		return -EINVAL;
	}
	client = cpld_i2c_clients[cpld_idx];
	reg = STRIP_CPLD_IDX(reg);

	res = i2c_smbus_write_byte_data(cpld_i2c_clients[cpld_idx], STRIP_CPLD_IDX(reg), write_val);
	if (res) {
		pr_err("could not write to i2c device reg: 0x%02X, val: 0x%02X", reg, write_val);
	}
	return res;
}

static ssize_t bulk_power_show(struct device * dev,
			       struct device_attribute * dattr,
			       char * buf)
{
	uint8_t read_val;
	uint8_t mask;
	uint8_t present_l;
	uint8_t pwr_ok;
	uint8_t error;

	read_val = cpld_reg_read(CPLD_POWER_SUPPLY_STATUS_REG);
	if (read_val < 0) {
		return -ENXIO;
	}
	if (strcmp(dattr->attr.name, xstr(PLATFORM_PS_NAME_0)) == 0) {
		mask      = CPLD_PSU0_MASK;
		present_l = CPLD_PSU0_PRESENT_L;
		pwr_ok    = CPLD_PSU0_GOOD;
		error     = CPLD_PSU0_ERROR;
	} else {
		mask      = CPLD_PSU1_MASK;
		present_l = CPLD_PSU1_PRESENT_L;
		pwr_ok    = CPLD_PSU1_GOOD;
		error     = CPLD_PSU1_ERROR;
	}
	read_val &= mask;

	if (~read_val & present_l) {
		sprintf(buf, PLATFORM_INSTALLED);
		if ( !(read_val & pwr_ok) || ( read_val & error ) ) {
			strcat(buf, ", "PLATFORM_PS_POWER_BAD);
		} else {
			strcat(buf, ", "PLATFORM_OK);
		}	
	} else {
		// Not present
		sprintf(buf + strlen(buf), PLATFORM_NOT_INSTALLED);
	}
	strcat(buf, "\n");

	return strlen(buf);
}
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_0, bulk_power_show);
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_1, bulk_power_show);

static ssize_t fan_show(struct device * dev,
			struct device_attribute * dattr,
			char *buf)
{
	uint8_t tmp;
	uint8_t present_l;
	int reg = CPLD_FAN1_STATUS_REG;

	if (strcmp(dattr->attr.name, "fan_0") == 0) {
		present_l = CPLD_FAN_TRAY_0_PRESENT_L;
	}
	else if (strcmp(dattr->attr.name, "fan_1") == 0) {
		present_l = CPLD_FAN_TRAY_1_PRESENT_L;
	}
	else {
		reg = CPLD_FAN2_STATUS_REG;
		present_l = CPLD_FAN_TRAY_2_PRESENT_L;
	}

	tmp = cpld_reg_read(reg);
	if (tmp < 0) {
		return 0;
	}
	if (~tmp & present_l) {
		return sprintf(buf, PLATFORM_OK "\n");
	}
	return sprintf(buf, PLATFORM_NOT_INSTALLED "\n");
}

static SYSFS_ATTR_RO(fan_0, fan_show);
static SYSFS_ATTR_RO(fan_1, fan_show);
static SYSFS_ATTR_RO(fan_2, fan_show);


/*------------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[DELL_S6000_CPLD_STRING_NAME_SIZE];
	unsigned int offset;
	uint8_t mask;
	int n_colors;
	char *last_color;
	int show_last_color;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
     {
	  .name = "led_system",
	  .offset = CPLD_LED_CONTROL_REG,
	  .mask = CPLD_SYSTEM_LED_MASK,
	  .n_colors = 4,
	  .colors = {
	       { PLATFORM_LED_GREEN, CPLD_SYSTEM_LED_GREEN },
	       { PLATFORM_LED_YELLOW, CPLD_SYSTEM_LED_YELLOW },
	       { PLATFORM_LED_YELLOW_BLINKING, CPLD_SYSTEM_LED_YELLOW_BLINK },
	       { PLATFORM_LED_GREEN_BLINKING, CPLD_SYSTEM_LED_GREEN_BLINK },
	  },
     },
     {
	  .name = "led_fan",
	  .offset = CPLD_FAN2_STATUS_REG,
	  .mask = CPLD_FRONT_FAN_LED_MASK,
	  .last_color = PLATFORM_LED_OFF,
	  .show_last_color = 1,
	  .n_colors = 4,
	  .colors = {
	       { PLATFORM_LED_GREEN, CPLD_FRONT_FAN_LED_GREEN },
	       { PLATFORM_LED_YELLOW, CPLD_FRONT_FAN_LED_YELLOW },
	       { PLATFORM_LED_YELLOW_BLINKING, CPLD_FRONT_FAN_LED_YELLOW_BLINK },
	       { PLATFORM_LED_OFF, CPLD_FRONT_FAN_LED_OFF },
	  },
     },
     {
	  /*
	   * The fan LED status bits in the CPLD do not correctly report
	   * the state of the fan LED when we read it. So we save the last
	   * value written and return that value rather than read it from
	   * the hardware. (see bug CM-2934).
	   */
	  .name = "led_power",
	  .offset = CPLD_LED_CONTROL_REG,
	  .mask = CPLD_POWER_LED_MASK,
	  .n_colors = 4,
	  .colors = {
	       { PLATFORM_LED_GREEN, CPLD_POWER_LED_GREEN },
	       { PLATFORM_LED_YELLOW, CPLD_POWER_LED_YELLOW },
	       { PLATFORM_LED_YELLOW_BLINKING, CPLD_POWER_LED_YELLOW_BLINK },
	       { PLATFORM_LED_OFF, CPLD_POWER_LED_OFF },
	  },
     },
     {
	  .name = "led_master",
	  .offset = CPLD_LED_CONTROL_REG,
	  .mask = CPLD_MASTER_LED_MASK,
	  .n_colors = 2,
	  .colors = {
	       { PLATFORM_LED_GREEN, CPLD_MASTER_LED_GREEN },
	       { PLATFORM_LED_OFF, CPLD_MASTER_LED_OFF },
	  },
     },
     {
	  .name = "led_fan_tray_0",
	  .offset = CPLD_FAN1_STATUS_REG,
	  .mask = CPLD_FAN_TRAY_0_LED_MASK,
	  .n_colors = 3,
	  .colors = {
	       { PLATFORM_LED_GREEN, CPLD_FAN_TRAY_0_LED_GREEN },
	       { PLATFORM_LED_YELLOW, CPLD_FAN_TRAY_0_LED_YELLOW },
	       { PLATFORM_LED_OFF, CPLD_FAN_TRAY_0_LED_OFF },
	  },
     },
     {
	  .name = "led_fan_tray_1",
	  .offset = CPLD_FAN1_STATUS_REG,
	  .mask = CPLD_FAN_TRAY_1_LED_MASK,
	  .n_colors = 3,
	  .colors = {
	       { PLATFORM_LED_GREEN, CPLD_FAN_TRAY_1_LED_GREEN },
	       { PLATFORM_LED_YELLOW, CPLD_FAN_TRAY_1_LED_YELLOW },
	       { PLATFORM_LED_OFF, CPLD_FAN_TRAY_1_LED_OFF },
	  },
     },
     {
	  .name = "led_fan_tray_2",
	  .offset = CPLD_FAN1_STATUS_REG,
	  .mask = CPLD_FAN_TRAY_2_LED_MASK,
	  .n_colors = 3,
	  .colors = {
	       { PLATFORM_LED_GREEN, CPLD_FAN_TRAY_2_LED_GREEN },
	       { PLATFORM_LED_YELLOW, CPLD_FAN_TRAY_2_LED_YELLOW },
	       { PLATFORM_LED_OFF, CPLD_FAN_TRAY_2_LED_OFF },
	  },
     },
};

static int n_leds = ARRAY_SIZE(cpld_leds);

/*
 * LEDs
 */
static ssize_t led_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	uint8_t tmp;
	int i;
	struct led * target = NULL;

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined\n");
	}

	if (target->show_last_color) {
	     return sprintf(buf, "%s\n", target->last_color);
	}

	/* read the register */
	tmp = cpld_reg_read(target->offset);
	if (tmp < 0) {
		return 0;
	}

	/* find the color */
	tmp &= target->mask;
	for (i = 0; i < target->n_colors; i++) {
		if (tmp == target->colors[i].value) {
			break;
		}
	}
	if (i == target->n_colors) {
		return sprintf(buf, "undefined\n");
	} else {
		return sprintf(buf, "%s\n", target->colors[i].name);
	}
}

static ssize_t led_store(struct device * dev,
                         struct device_attribute * dattr,
                         const char * buf, size_t count)
{
	uint8_t tmp;
	int i;
	struct led * target = NULL;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL) {
		return -EINVAL;
	}

	/* find the color */
	if (sscanf(buf, "%19s", raw) <= 0) {
		return -EINVAL;
	}
	for (i = 0; i < target->n_colors; i++) {
		if (strcmp(raw, target->colors[i].name) == 0) {
			break;
		}
	}
	if (i == target->n_colors) {
		return -EINVAL;
	}
	target->last_color = target->colors[i].name;

	/* set the new value */
	tmp = cpld_reg_read(target->offset);
	if (tmp < 0) {
		return 0;
	}
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;

	cpld_reg_write(target->offset, tmp);

	return count;
}

static SYSFS_ATTR_RW(led_system, led_show, led_store);
static SYSFS_ATTR_RW(led_fan, led_show, led_store);
static SYSFS_ATTR_RW(led_power, led_show, led_store);
static SYSFS_ATTR_RW(led_master, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_0, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_1, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_2, led_show, led_store);


/*------------------------------------------------------------------------------
 *
 * QSFP definitions
 *
 */
static const uint8_t qsfp_mode_offsets[] = { CPLD_QSFP_1_8_MOD_MODE_CTL_REG,
                                             CPLD_QSFP_9_16_MOD_MODE_CTL_REG,
                                             CPLD_QSFP_17_24_MOD_MODE_CTL_REG,
                                             CPLD_QSFP_25_32_MOD_MODE_CTL_REG };

static const uint8_t qsfp_reset_offsets[] = { CPLD_QSFP_1_8_RESET_CTL_REG,
                                              CPLD_QSFP_9_16_RESET_CTL_REG,
                                              CPLD_QSFP_17_24_RESET_CTL_REG,
                                              CPLD_QSFP_25_32_RESET_CTL_REG };

static const uint8_t qsfp_lpmod_offsets[] = { CPLD_QSFP_1_8_LP_MODE_CTL_REG,
                                              CPLD_QSFP_9_16_LP_MODE_CTL_REG,
                                              CPLD_QSFP_17_24_LP_MODE_CTL_REG,
                                              CPLD_QSFP_25_32_LP_MODE_CTL_REG };

static const uint8_t qsfp_present_offsets[] = { CPLD_QSFP_1_8_PRESENCE_CTL_REG,
                                                CPLD_QSFP_9_16_PRESENCE_CTL_REG,
                                                CPLD_QSFP_17_24_PRESENCE_CTL_REG,
                                                CPLD_QSFP_25_32_PRESENCE_CTL_REG };

static const uint8_t qsfp_interrupt_offsets[] = { CPLD_QSFP_1_8_INT_CTL_REG,
                                                   CPLD_QSFP_9_16_INT_CTL_REG,
                                                   CPLD_QSFP_17_24_INT_CTL_REG,
                                                   CPLD_QSFP_25_32_INT_CTL_REG };

static ssize_t qsfp_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	const char *signal;
	const uint8_t *offsets;
	uint32_t data;
	int offset_idx;

	/* qsfp_(reset|lpmod|present) */
	signal = &dattr->attr.name[5];
	if (strcmp(signal, "reset") == 0) {
		offsets = qsfp_reset_offsets;
	} else if (strcmp(signal, "lpmod") == 0) {
		offsets = qsfp_lpmod_offsets;
	} else if (strcmp(signal, "present") == 0) {
		offsets = qsfp_present_offsets;
	} else if (strcmp(signal, "i2c_enable") == 0) {
		offsets = qsfp_mode_offsets;
	} else if (strcmp(signal, "interrupt") == 0) {
	     offsets = qsfp_interrupt_offsets;
	} else {
		return -EINVAL;
	}

	data = 0;
	for (offset_idx = 0; offset_idx < 4; offset_idx++) {
		data |= cpld_reg_read(offsets[offset_idx]) << (offset_idx * 8);
	}

	return sprintf(buf, "0x%08X\n", data);
}

static ssize_t qsfp_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	const char *signal;
	const uint8_t *offsets;
	unsigned long int data;
	int offset_idx;

	if (kstrtoul(buf, 0, &data) < 0)
		return -EINVAL;

	/* qsfp_(reset|lpmod|present) */
	signal = &dattr->attr.name[5];
	if (strcmp(signal, "reset") == 0) {
		offsets = qsfp_reset_offsets;
	} else if (strcmp(signal, "lpmod") == 0) {
		offsets = qsfp_lpmod_offsets;
	} else if (strcmp(signal, "present") == 0) {
		offsets = qsfp_present_offsets;
	} else if (strcmp(signal, "i2c_enable") == 0) {
		offsets = qsfp_mode_offsets;
	} else {
		return -EINVAL;
	}

	for (offset_idx = 0; offset_idx < 4; offset_idx++) {
		cpld_reg_write(offsets[offset_idx], (data >> (offset_idx * 8)) & 0xff);
	}

	return count;
}

static SYSFS_ATTR_RW(qsfp_reset, qsfp_show, qsfp_store);
static SYSFS_ATTR_RW(qsfp_lpmod, qsfp_show, qsfp_store);
static SYSFS_ATTR_RW(qsfp_present, qsfp_show, qsfp_store);
static SYSFS_ATTR_RW(qsfp_i2c_enable, qsfp_show, qsfp_store);
static SYSFS_ATTR_RO(qsfp_interrupt, qsfp_show);

static struct attribute *dell_s6000_cpld_attrs[] = {
	&dev_attr_psu_pwr1.attr,
	&dev_attr_psu_pwr2.attr,
	&dev_attr_fan_0.attr,
	&dev_attr_fan_1.attr,
	&dev_attr_fan_2.attr,
	&dev_attr_led_system.attr,
	&dev_attr_led_fan.attr,
	&dev_attr_led_power.attr,
	&dev_attr_led_master.attr,
	&dev_attr_led_fan_tray_0.attr,
	&dev_attr_led_fan_tray_1.attr,
	&dev_attr_led_fan_tray_2.attr,
	&dev_attr_qsfp_reset.attr,
	&dev_attr_qsfp_lpmod.attr,
	&dev_attr_qsfp_present.attr,
	&dev_attr_qsfp_i2c_enable.attr,
	&dev_attr_qsfp_interrupt.attr,
	NULL,
};

static struct attribute_group dell_s6000_cpld_attr_group = {
	.attrs = dell_s6000_cpld_attrs,
};

static int dell_s6000_cpld_probe(struct platform_device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->dev.kobj, &dell_s6000_cpld_attr_group);
	if (ret) {
		pr_err("sysfs_create_group failed for cpld driver");
	}
	return ret;
}

static int dell_s6000_cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver dell_s6000_cpld_driver = {
	.driver = {
		.name = "dell_s6000_cpld",
		.owner = THIS_MODULE,
	},
	.probe = dell_s6000_cpld_probe,
	.remove = dell_s6000_cpld_remove,
};

static struct platform_device *dell_s6000_cpld_device;

static int __init dell_s6000_cpld_init(struct dell_s6000_i2c_devices *device_list)
{
	int i, j=0;
	struct i2c_client *client;
	int ret;

	for (i = 0; i < device_list->n_devices; i++)
	{
		/* we make the assumption that the first three 'dummy' devices
		* in the provided device list are our cpld clients
		*/
		client = device_list->clients[i];
		if (strcmp(client->name, "dummy") == 0 && j < NUM_CPLD_I2C_CLIENTS)
		{
			cpld_i2c_clients[j++] = client;
		}
	}

	ret = platform_driver_register(&dell_s6000_cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for cpld device");
		goto err_drvr;
	}

	dell_s6000_cpld_device = platform_device_alloc("dell_s6000_cpld", 0);
	if (!dell_s6000_cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}

	ret = platform_device_add(dell_s6000_cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_dev_add;
	}
	return 0;

err_dev_add:
	platform_device_put(dell_s6000_cpld_device);

err_dev_alloc:
	platform_driver_unregister(&dell_s6000_cpld_driver);

err_drvr:
	return ret;
}

/*---------------------------------------------------------------------
 *
 * CPLD QSFP mux
 */

static int qsfp_mux_i2c_reg_write(uint8_t command, uint8_t val)
{
	int rv;
	struct i2c_client *client;
	struct i2c_adapter *adap;

	adap = get_adapter(i2c_devices[0].bus);
	if (!adap) {
		pr_err("Unable to get DELL_S6000_ISCH_I2CMUX_BUS_0 adapater.\n");
		return -ENODEV;
	}

	client = cpld_i2c_clients[GET_CPLD_IDX(command)];
	command = STRIP_CPLD_IDX(command);

	if (adap->algo->master_xfer) {
		struct i2c_msg msg = { 0 };
		char buf[2];

		msg.addr = client->addr;
		msg.flags = 0;
		msg.len = 2;
		buf[0] = command;
		buf[1] = val;
		msg.buf = buf;
		rv = adap->algo->master_xfer(adap, &msg, 1);
	} else {
		union i2c_smbus_data data;

		data.byte = val;
		rv = adap->algo->smbus_xfer(adap, client->addr,
		                            client->flags,
		                            I2C_SMBUS_WRITE,
		                            command, I2C_SMBUS_BYTE_DATA, &data);
	}
	i2c_put_adapter(adap);
	return rv;
}

static int qsfp_bit_shift_lookup[] = { 1, 0, 3, 2, 5, 4, 7, 6 };
static int cpld_qsfp_mux_select_chan(struct i2c_mux_core *muxc, u32 chan)
{
	int val;
	u32 mux_reg = 0;
	static u32 prev_reg = CPLD_QSFP_1_8_MOD_MODE_CTL_REG;
	static u32 prev_chan = QSFP_MUX_INVALID_CHANNEL;

	if (likely(chan == prev_chan))
		return 0;

	prev_chan = chan;

	val = chan - DELL_S6000_QSFPMUX_PORT_1;
	if (val < 8) {
		mux_reg = CPLD_QSFP_1_8_MOD_MODE_CTL_REG;
	} else if (val < 16) {
		mux_reg = CPLD_QSFP_9_16_MOD_MODE_CTL_REG;
		val -= 8;
	} else if (val < 24) {
		mux_reg = CPLD_QSFP_17_24_MOD_MODE_CTL_REG;
		val -= 16;
	} else if (val < 32) {
		mux_reg = CPLD_QSFP_25_32_MOD_MODE_CTL_REG;
		val -= 24;
	} else {
		pr_err("invalid mux channel number %u\n", chan);
		return -EINVAL;
	}

	/* de-select previous MUX channel if necessary*/
	if (mux_reg != prev_reg)
		qsfp_mux_i2c_reg_write(prev_reg, 0xff);

	/* switch port numbers got swapped, so
	 *    port1 => bus + 1, port2 => bus + 0, port3 => bus + 3 ...
	 */
	qsfp_mux_i2c_reg_write(mux_reg, ~(1 << qsfp_bit_shift_lookup[val]));
	prev_reg = mux_reg;
	/* SFF8436 spec mandates setup time of 2ms 
	 * Setup time on the select lines before 
	 * start of a host initiated serial bus sequence*/
	msleep(2);
	return 0;
}

static void __exit dell_s6000_cpld_exit()
{
	platform_driver_unregister(&dell_s6000_cpld_driver);
	platform_device_unregister(dell_s6000_cpld_device);
}

module_init(dell_s6000_init);
module_exit(dell_s6000_exit);

MODULE_AUTHOR("Curt Brune (curt@cumulusnetworks.com)");
MODULE_DESCRIPTION("DELL S6000 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
