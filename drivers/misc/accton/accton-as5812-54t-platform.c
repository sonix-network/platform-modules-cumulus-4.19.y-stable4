/*
 * accton-as5812-54t-platform.c - Accton AS5812-54t Platform Support.
 *
 * Copyright (C) 2016,2019 Cumulus Networks, Inc.
 * Author: Vidya Sagar Ravipati <vidya@cumulusnetworks.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>

#include "platform-defs.h"
#include "accton-as5812-54t-cpld.h"

#define DRIVER_NAME	"accton_as5812_54t_platform"
#define DRIVER_VERSION	"1.0"

/*
 * The platform has 2 types of i2c SMBUSes, i801 (Intel 82801
 * (ICH/PCH)) and ISMT (Intel SMBus Message Transport).  ISMT has a
 * PCA9548 mux.
 */

/* i2c bus adapter numbers for the 8 down stream PCA9548 i2c busses */
enum {
	AS5812_I2C_ISMT_BUS=0,
	AS5812_I2C_I801_BUS,
	AS5812_I2C_PCA9548_BUS_0 = 10,
	AS5812_I2C_PCA9548_BUS_1,
	AS5812_I2C_PCA9548_BUS_2,
	AS5812_I2C_PCA9548_BUS_3,
	AS5812_I2C_PCA9548_BUS_4,
	AS5812_I2C_PCA9548_BUS_5,
	AS5812_I2C_PCA9548_BUS_6,
	AS5812_I2C_PCA9548_BUS_7,
	AS5812_I2C_PCA9548_2_BUS_1 = 30,
	AS5812_I2C_PCA9548_2_BUS_2,
	AS5812_I2C_PCA9548_2_BUS_3,
	AS5812_I2C_PCA9548_2_BUS_4,
	AS5812_I2C_PCA9548_2_BUS_5,
	AS5812_I2C_PCA9548_2_BUS_6,
	AS5812_I2C_PCA9548_2_BUS_7,
	AS5812_I2C_PCA9548_2_BUS_8,
	AS5812_I2C_PORT_EEPROM_BUS_0 = 40,
};

mk_eeprom(spd1, 52, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(spd2, 53, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(psu1, 38, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2, 3b, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu1_3y, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2_3y, 53, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(board,57, 256, AT24_FLAG_IRUGO);
mk_qsfp_port_eeprom(port49,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port50,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port51,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port52,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port53,  50, 256,  SFF_8436_FLAG_IRUGO);
mk_qsfp_port_eeprom(port54,  50, 256,  SFF_8436_FLAG_IRUGO);

/*
 * Platform data for the PCA9548 MUX
 */
static struct pca954x_platform_mode __initdata pca9548_mux1_platform_modes [] = {
	{
		.adap_id = AS5812_I2C_PCA9548_BUS_0, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_BUS_1, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_BUS_2, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_BUS_3, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_BUS_4, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_BUS_5, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_BUS_6, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_BUS_7, .deselect_on_exit = 1,
	},
};

static struct pca954x_platform_mode __initdata pca9548_mux2_platform_modes [] = {

	{
		.adap_id = AS5812_I2C_PCA9548_2_BUS_1, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_2_BUS_2, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_2_BUS_3, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_2_BUS_4, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_2_BUS_5, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_2_BUS_6, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_2_BUS_7, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5812_I2C_PCA9548_2_BUS_8, .deselect_on_exit = 1,
	},
};

static struct pca954x_platform_data __initdata pca9548_mux1_platform_data = {
	.modes = pca9548_mux1_platform_modes,
	.num_modes = ARRAY_SIZE(pca9548_mux2_platform_modes),
};

static struct pca954x_platform_data __initdata pca9548_mux2_platform_data = {
	.modes = pca9548_mux2_platform_modes,
	.num_modes = ARRAY_SIZE(pca9548_mux2_platform_modes),
};

static struct platform_i2c_device_info __initdata i2c_devices[] = {
	/* Begin I2C_ISMT Bus */
	{
		.bus = AS5812_I2C_ISMT_BUS,
		.board_info = {
			I2C_BOARD_INFO("pca9548", 0x70),  /* PCA9548 8 Port I2C Switch */
			.platform_data = &pca9548_mux1_platform_data,
		},
	},
	{
		.bus = AS5812_I2C_ISMT_BUS,
		.board_info = {
			I2C_BOARD_INFO("24c02", 0x57),	/* Board EEPROM */
			.platform_data = &board_57_at24,
		},
	},
	/* Begin PCA9548 BUS 0 is empty */
	/* Begin PCA9548 BUS 1 */
	/* Compuware PSU 1 */
	{
		.bus = AS5812_I2C_PCA9548_BUS_1,
		.board_info = {
			I2C_BOARD_INFO("24c02", 0x38),   /* PSU 1 EEPROM */
			.platform_data = &psu1_38_at24,
		},
	},
	{
		.bus = AS5812_I2C_PCA9548_BUS_1,
		.board_info = {
			I2C_BOARD_INFO("cpr4011", 0x3c),   /* PSU 1 PMBUS */
		},
	},
	/* 3Y Power PSU 1 */
	{
		.bus = AS5812_I2C_PCA9548_BUS_1,
		.board_info = {
			I2C_BOARD_INFO("24c02", 0x50),   /* PSU 1 EEPROM */
			.platform_data = &psu1_3y_50_at24,
		},
	},
	{
		.bus = AS5812_I2C_PCA9548_BUS_1,
		.board_info = {
			I2C_BOARD_INFO("cpr4011", 0x58),   /* PSU 1 PMBUS */
		},
	},
	/* Begin PCA9548 BUS 2 */
	/* Compuware PSU 2 */
	{
		.bus = AS5812_I2C_PCA9548_BUS_2,
		.board_info = {
			I2C_BOARD_INFO("24c02", 0x3b),   /* PSU 2 EEPROM */
			.platform_data = &psu2_3b_at24,
		},
	},
	{
		.bus = AS5812_I2C_PCA9548_BUS_2,
		.board_info = {
			I2C_BOARD_INFO("cpr4011", 0x3f),   /* PSU 2 PMBUS */
		},
	},
	/* 3Y Power PSU 2 */
	{
		.bus = AS5812_I2C_PCA9548_BUS_2,
		.board_info = {
			I2C_BOARD_INFO("24c02", 0x53),   /* PSU 2 EEPROM */
			.platform_data = &psu2_3y_53_at24,
		},
	},
	{
		.bus = AS5812_I2C_PCA9548_BUS_2,
		.board_info = {
			I2C_BOARD_INFO("cpr4011", 0x5b),   /* PSU 2 PMBUS */
		},
	},
	/* Begin PCA9548 BUS 3 */
	{
		.bus = AS5812_I2C_PCA9548_BUS_3,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x10), /* TODO: DC/DC Power */
		},
	},
	/* PCA9548 BUS 4 */
	{
		.bus = AS5812_I2C_PCA9548_BUS_4,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x12), /* TODO: DC/DC Power */
		},
	},
	/* Begin PCA9548 BUS 5 */
	{
		.bus = AS5812_I2C_PCA9548_BUS_5,
		.board_info = {
			I2C_BOARD_INFO("lm75", 0x48), /* LM75 Temperature Sensor */
		},
	},
	/* Begin PCA9548 BUS 6 */
	{
		.bus = AS5812_I2C_PCA9548_BUS_6,
		.board_info = {
			I2C_BOARD_INFO("lm75", 0x49), /* LM75 Temperature Sensor */
		},
	},
	/* Begin PCA9548 BUS 7 */
	{
		.bus = AS5812_I2C_PCA9548_BUS_7,
		.board_info = {
			I2C_BOARD_INFO("lm75", 0x4a), /* LM75 Temperature Sensor */
		},
	},
	/* Begin I2C_I801 Bus */
	{
		.bus = AS5812_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x2e), /* ISL9027 Potentiometer */
		},
	},
	{
		.bus = AS5812_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("spd", 0x52),  /* DIMM */
			.platform_data = &spd1_52_at24,
		},
	},
	{
		.bus = AS5812_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("spd", 0x53),  /* DIMM */
			.platform_data = &spd2_53_at24,
		},
	},
	{
		.bus = AS5812_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x60), /* System CPLD */
		},
	},
	{
		.bus = AS5812_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x69), /* Clock Gen */
		},
	},
	/* Begin I2C_ISMT Bus */
	/* PCA9548 8 Port I2C Switch */

	{
		.bus = AS5812_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("pca9548", 0x71),
			.platform_data = &pca9548_mux2_platform_data,
		},
	},
	/* Begin PCA9548 BUS 2 Channel 0 is empty */
	/* PCA9548 BUS 2 Channel 1 i.e QSFP52 */

	{
		.bus = AS5812_I2C_PCA9548_2_BUS_4,
		.board_info = {
			I2C_BOARD_INFO("sff8436", 0x50),
			.platform_data = &port52_50_sff8436,
		},
	},
	/* PCA9548 BUS 2 Channel 2 i.e QSFP53 */
	{
		.bus = AS5812_I2C_PCA9548_2_BUS_6,
		.board_info = {
			I2C_BOARD_INFO("sff8436", 0x50),
			.platform_data = &port53_50_sff8436,
		},
	},
	/* PCA9548 BUS 2 Channel 3 i.e QSFP49 */
	{
		.bus = AS5812_I2C_PCA9548_2_BUS_3,
		.board_info = {
			I2C_BOARD_INFO("sff8436", 0x50),
			.platform_data = &port49_50_sff8436,
		},
	},
	/* PCA9548 BUS 2 Channel 4 i.e QSFP54 */
	{
		.bus = AS5812_I2C_PCA9548_2_BUS_1,
		.board_info = {
			I2C_BOARD_INFO("sff8436", 0x50),
			.platform_data = &port54_50_sff8436,
		},
	},
	/* PCA9548 BUS 2 Channel 5 i.e QSFP50 */
	{
		.bus = AS5812_I2C_PCA9548_2_BUS_5,
		.board_info = {
			I2C_BOARD_INFO("sff8436", 0x50),
			.platform_data = &port50_50_sff8436,
		}
	},
	/* PCA9548 BUS 2 Channel 6 i.e QSFP51 */
	{
		.bus = AS5812_I2C_PCA9548_2_BUS_2,
		.board_info = {
			I2C_BOARD_INFO("sff8436", 0x50),
			.platform_data = &port51_50_sff8436,
		},
	},
	/* PCA9548 BUS 2 Channel 7 empty*/
};


/**
 * Array of allocated i2c_client objects.  Need to track these in
 * order to free them later.
 *
 */
static struct i2c_client *i2c_clients[ARRAY_SIZE(i2c_devices)];

/**
 *
 * TODO:  This should be common code used by most platforms....
 *
 * Fetch i2c adapter by bus number.
 *
 * The retry / sleep logic is required as the dynamically allocated
 * adapters from the MUX devices can take "some time" to become
 * available.
 */
static struct i2c_adapter __init *get_adapter(int bus)
{
	int bail=20;
	struct i2c_adapter *adapter;

	for (; bail; bail--) {
		adapter = i2c_get_adapter(bus);
		if (adapter)
			return adapter;
		msleep(100);
	}
	return NULL;
}

/**
 *
 * TODO:  Can this be common code used by most platforms....
 *
 */
static void free_i2c_data(void)
{
	int i;

	/*
	 * Free the devices in reverse order so that child devices are
	 * freed before parent mux devices.
	 */
	for (i = ARRAY_SIZE(i2c_devices) - 1; i >= 0; i--)
		if (i2c_clients[i])
			i2c_unregister_device(i2c_clients[i]);

}

static int __init get_bus_by_name(char *name)
{
	struct i2c_adapter *adapter;
	int i;

	for (i = 0; i < AS5812_I2C_PCA9548_BUS_1; i++) {
		adapter = get_adapter(i);
		if (adapter && (strncmp(adapter->name, name, strlen(name)) == 0)) {
			i2c_put_adapter(adapter);
			return i;
		}
		i2c_put_adapter(adapter);
	}
	return -1;
}

static struct i2c_client __init *add_i2c_client(int bus, struct i2c_board_info *board_info)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = get_adapter(bus);
	if (!adapter) {
		pr_err("could not get adapter %u\n", bus);
		return ERR_PTR(-ENODEV);
	}
	client = i2c_new_device(adapter, board_info);
	if (!client) {
		pr_err("could not add device\n");
		client = ERR_PTR(-ENODEV);
	}

	i2c_put_adapter(adapter);
	return client;
}

static int iSMT_bus_num;
static int i801_bus_num;

static int hexToInt64(const char *hex_str, uint64_t *val)
{
	char prefix[] = "0x";
	if (strncasecmp(hex_str, prefix, strlen(prefix)) == 0) {
		hex_str += strlen(prefix);
	}
	return sscanf(hex_str, "%llx", val) != 1;
}

/**
 * accton_as5812_54t_i2c_init -- Initialize I2C devices
 *
 */
static int __init accton_as5812_54t_i2c_init(void)
{
	struct i2c_client *client;
	int ret;
	int i;

	ret = -1;

	iSMT_bus_num = get_bus_by_name(ISMT_ADAPTER_NAME);
	if (iSMT_bus_num < 0) {
		pr_err("could not find iSMT adapter bus\n");
		ret = -ENXIO;
		goto err_exit;
	}
	i801_bus_num = get_bus_by_name(I801_ADAPTER_NAME);
	if (i801_bus_num < 0) {
		pr_err("could not find I801 adapter bus\n");
		ret = -ENODEV;
		goto err_exit;
	}

	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		/*
		 * Map logical buses S5812_I2C_ISMT_BUS and
		 * AS5812_I2C_I801_BUS to their dynamically discovered
		 * bus numbers.
		 */
		switch (i2c_devices[i].bus) {
		case AS5812_I2C_ISMT_BUS:
			i2c_devices[i].bus = iSMT_bus_num;
			break;
		case AS5812_I2C_I801_BUS:
			i2c_devices[i].bus = i801_bus_num;
			break;
		default:
			break;
			/* Fall through for PCA9548 buses */
		};
		client = add_i2c_client(i2c_devices[i].bus, &i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_clients[i] = client;
	}

	return 0;

err_exit:
	free_i2c_data();
	return ret;
}

static void __exit accton_as5812_54t_i2c_exit(void)
{
	free_i2c_data();
}

/*************************************************/
/* BEGIN CPLD Platform Driver                    */
/*                                               */
/* This driver is responsible for the sysfs intf */
/*************************************************/

/**
 * Array of the CPLD i2c devices, used by cpld_reg_read() /
 * cpld_reg_write().
 */
static struct i2c_client *cpld_i2c_clients;

/**
 * cpld_reg_read - Read an 8-bit CPLD register over i2c
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else a data byte received from the device.
 */
static s32 cpld_reg_read(uint32_t reg)
{
	int val;

	val = i2c_smbus_read_byte_data(cpld_i2c_clients, reg);
	if (val < 0) {
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       cpld_i2c_clients->addr, reg);
	}
	return val;
}

/**
 * cpld_reg_write - Writes an 8-bit CPLD register over i2c
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else zero on success.
 */
static s32 cpld_reg_write(uint32_t reg, uint8_t val)
{
	int res;
	struct i2c_client *client;

	client = cpld_i2c_clients;

	res = i2c_smbus_write_byte_data(cpld_i2c_clients, reg, val);
	if (res) {
		pr_err("I2C write error - addr: 0x%02X, offset: 0x%02X, val: 0x%02X",
		       cpld_i2c_clients->addr, reg, val);
	}
	return res;
}

static ssize_t show_name(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	return sprintf(buf, "%s\n", pdev->name);
}

static SYSFS_ATTR_RO(name, show_name);

/*
 * board ID
 */
static ssize_t board_id_show(struct device *dev,
			     struct device_attribute *dattr,
			     char *buf)
{
	s32 val;

	val = cpld_reg_read(CPLD_BOARD_ID_REG);

	if (val >= 0)
		return sprintf(buf, "%d\n", val);
	else
		return 0;

}
static SYSFS_ATTR_RO(board_id, board_id_show);

/*
 * CPLD Version
 */
static ssize_t cpld_version_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	s32 val;

	val = cpld_reg_read(CPLD_SYS_CPLD_VERSION_REG);
	if (val < 0)
		return val;

	return sprintf(buf, "%d\n", val);

}
static SYSFS_ATTR_RO(cpld_version, cpld_version_show);

static ssize_t pwm1_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	s32 val;

	val = cpld_reg_read(CPLD_FAN_PWM_REG);
	if (val < 0)
		return val;

	/* The PWM register contains a value between 0 and 0x14
	 * inclusive, representing the fan duty cycle in 5%
	 * increments.  A value of 0x14 is 100% duty cycle.
	 *
	 * For hwmon devices map the pwm value into the range 0 to
	 * 255.
	 *
	 * 255 / 20.0 = 12.75, so with integer multiply by 1275 and
	 * divide by 100.
	 */
	val = (val * 1275) / 100;
	return sprintf(buf, "%d\n", val);
}

static ssize_t pwm1_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	int ret;
	uint32_t pwm = 0;

	if (sscanf(buf, "%d", &pwm) <= 0) {
		return -EINVAL;
	}
	pwm = clamp_val(pwm, 0, 255);

	/* See comments above in pwm1_show for the mapping */
	pwm = (pwm * 100) / 1275;

	/* The register also enforces a minimum of 20% duty cycle */
	pwm = (pwm < 4) ? 4 : pwm;

	ret = cpld_reg_write(CPLD_FAN_PWM_REG, pwm);
	if (ret < 0)
		return ret;

	return count;
}
static SYSFS_ATTR_RW(pwm1, pwm1_show, pwm1_store);

/* pwm_en is for implementing the hwmon interface.
 *
 * These routines actually do nothing.
 */
static ssize_t pwm1_enable_show(struct device *dev,
				struct device_attribute *dattr,
				char *buf)
{
	return sprintf(buf, "1\n");
}

static ssize_t pwm1_enable_store(struct device *dev,
				 struct device_attribute *dattr,
				 const char *buf, size_t count)
{
	if (count < 1) {
		return -EINVAL;
	}

	if (strcmp(buf, "1\n") != 0 && strcmp(buf, "1") != 0) {
		return -EINVAL;
	}

	return count;
}
static SYSFS_ATTR_RW(pwm1_enable, pwm1_enable_show, pwm1_enable_store);

/*
 * Fan tachometers - each fan module contains two fans.  Map the sysfs
 * files to the Accton fan names as follows:
 *
 *   sysfs Name | Accton Specification
 *   ===========+=====================
 *   fan01	| FAN1
 *   fan02	| FANR1
 *   fan03	| FAN2
 *   fan04	| FANR2
 *   fan05	| FAN3
 *   fan06	| FANR3
 *   fan07	| FAN4
 *   fan08	| FANR4
 *   fan09	| FAN5
 *   fan10      | FANR5
 *
 */
static ssize_t fan_tach_show(struct device *dev,
			     struct device_attribute *dattr,
			     char *buf)
{
	int fan = 0;
	uint8_t reg;
	s32 val;
	int rpm;

	if ((sscanf(dattr->attr.name, "fan%d_", &fan) < 1) || (fan < 1) || (fan > 10))
		return -EINVAL;

	if (fan & 0x1) {
		reg = CPLD_FAN1_SPEED_REG + ((fan - 1) / 2);
	} else {
		reg = CPLD_FANR1_SPEED_REG + ((fan - 1) / 2);
	}

	val = cpld_reg_read(reg);
	if (val < 0)
		return val;

	/* From Accton hardware spec the RPM is 150 times the register value */
	rpm = (uint8_t)val * 150;

	return sprintf(buf, "%d\n", rpm);
}
static SYSFS_ATTR_RO(fan01_input, fan_tach_show);
static SYSFS_ATTR_RO(fan02_input, fan_tach_show);
static SYSFS_ATTR_RO(fan03_input, fan_tach_show);
static SYSFS_ATTR_RO(fan04_input, fan_tach_show);
static SYSFS_ATTR_RO(fan05_input, fan_tach_show);
static SYSFS_ATTR_RO(fan06_input, fan_tach_show);
static SYSFS_ATTR_RO(fan07_input, fan_tach_show);
static SYSFS_ATTR_RO(fan08_input, fan_tach_show);
static SYSFS_ATTR_RO(fan09_input, fan_tach_show);
static SYSFS_ATTR_RO(fan10_input, fan_tach_show);

/*
 * Fan faults - see the discussion above for fan_tach_show() for the
 * mapping of sysfs file names to the Accton fan names.
 *
 */
static ssize_t fan_status_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	int fan = 0;
	uint8_t reg;
	s32 val;
	uint8_t shift;

	if ((sscanf(dattr->attr.name, "fan%d_", &fan) < 1) || (fan < 1) || (fan > 10))
		return -EINVAL;

	if (fan & 0x1) {
		reg = CPLD_FAN_FAULT_REG;
	} else {
		reg = CPLD_FANR_FAULT_REG;
	}

	val = cpld_reg_read(reg);
	if (val < 0)
		return val;

	shift = (fan - 1) / 2;
	return sprintf(buf, "%d\n",
		       ((uint8_t)val & (0x1 << shift) ? 0 : 1));
}
static SYSFS_ATTR_RO(fan01_ok,	fan_status_show);
static SYSFS_ATTR_RO(fan02_ok,	fan_status_show);
static SYSFS_ATTR_RO(fan03_ok,	fan_status_show);
static SYSFS_ATTR_RO(fan04_ok,	fan_status_show);
static SYSFS_ATTR_RO(fan05_ok,	fan_status_show);
static SYSFS_ATTR_RO(fan06_ok,	fan_status_show);
static SYSFS_ATTR_RO(fan07_ok,	fan_status_show);
static SYSFS_ATTR_RO(fan08_ok,	fan_status_show);
static SYSFS_ATTR_RO(fan09_ok,	fan_status_show);
static SYSFS_ATTR_RO(fan10_ok, fan_status_show);

/*
 * Fan tray LEDs
 *
 */
static char *fan_led_color[] = {
	PLATFORM_LED_OFF,
	PLATFORM_LED_GREEN,
	PLATFORM_LED_RED,
	PLATFORM_LED_YELLOW,
};

static ssize_t led_fan_tray_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	int tray = 0;
	uint8_t reg, val8;
	s32 val;
	uint8_t shift = 0, mask = 0x3;

	if ((sscanf(dattr->attr.name, "led_fan_tray_%d", &tray) < 1) || (tray < 1) || (tray > 5))
		return -EINVAL;

	if (tray < 5) {
		reg = CPLD_FAN1_4_LED_REG;
		shift = (tray - 1) * 2;
	} else {
		reg = CPLD_FAN5_LED_REG;
		shift = 0;
	}

	val = cpld_reg_read(reg);
	if (val < 0)
		return val;

	val8 = (uint8_t)val;
	val8 = (val8 >> shift) & mask;
	return sprintf(buf, "%s\n", fan_led_color[val8]);
}

static SYSFS_ATTR_RO(led_fan_tray_1, led_fan_tray_show);
static SYSFS_ATTR_RO(led_fan_tray_2, led_fan_tray_show);
static SYSFS_ATTR_RO(led_fan_tray_3, led_fan_tray_show);
static SYSFS_ATTR_RO(led_fan_tray_4, led_fan_tray_show);
static SYSFS_ATTR_RO(led_fan_tray_5, led_fan_tray_show);

static ssize_t air_flow_fan_tray_show(struct device *dev,
				      struct device_attribute *dattr,
				      char *buf)
{
	int tray = 0;
	s32 val;
	uint8_t direction;

	if ((sscanf(dattr->attr.name, "air_flow_fan_tray_%d", &tray) < 1) || (tray < 1) || (tray > 5))
		return -EINVAL;

	val = cpld_reg_read(CPLD_FAN_DIRECTION_REG);
	if (val < 0)
		return val;

	direction = (uint8_t)val & (0x1 << (tray - 1));
	return sprintf(buf, "%s\n", direction ? "back-to-front" : "front-to-back");
}

static SYSFS_ATTR_RO(air_flow_fan_tray_1, air_flow_fan_tray_show);
static SYSFS_ATTR_RO(air_flow_fan_tray_2, air_flow_fan_tray_show);
static SYSFS_ATTR_RO(air_flow_fan_tray_3, air_flow_fan_tray_show);
static SYSFS_ATTR_RO(air_flow_fan_tray_4, air_flow_fan_tray_show);
static SYSFS_ATTR_RO(air_flow_fan_tray_5, air_flow_fan_tray_show);

/*------------------------------------------------------------------------------
 *
 * PSU status definitions
 *
 * All the definition names use positive logic and return 1 for OK and
 * 0 for not OK.
 */
#define ACCTON_AS5812_54T_CPLD_STRING_NAME_SIZE 30
struct cpld_status {
	char name[ACCTON_AS5812_54T_CPLD_STRING_NAME_SIZE];
	uint8_t good_mask;
	uint8_t bad_mask;
	char msg_good[ACCTON_AS5812_54T_CPLD_STRING_NAME_SIZE];
	char msg_bad[ACCTON_AS5812_54T_CPLD_STRING_NAME_SIZE];
};

static struct cpld_status cpld_psu_status[] = {
	{
		.name = "all_ok",
		.good_mask = CPLD_PSU_AC_ALERT_L |CPLD_PSU_12V_GOOD,
		.bad_mask =  CPLD_PSU_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "present",
		.good_mask = 0,
		.bad_mask = CPLD_PSU_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "ac_ok",
		.good_mask = CPLD_PSU_AC_ALERT_L,
		.bad_mask =  CPLD_PSU_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "dc_ok",
		.good_mask = CPLD_PSU_12V_GOOD,
		.bad_mask = CPLD_PSU_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
};
static int n_psu_states = ARRAY_SIZE(cpld_psu_status);

/*
 * PSU power supply status
 */
static ssize_t psu_power_show(struct device *dev,
			      struct device_attribute *dattr,
			      char *buf)
{
	int psu = 0, i;
	s32 val;
	uint8_t val8, bad = 0;
	uint8_t name_len = strlen(xstr(PLATFORM_PS_NAME_0));
	struct cpld_status* target = NULL;

	/* find the target PSU */
	if ((sscanf(dattr->attr.name, "psu_pwr%d_", &psu) < 1) || (psu < 1) || (psu > 2))
		return -EINVAL;

	for (i = 0; i < n_psu_states; i++) {
		if (strcmp(dattr->attr.name + name_len + 1, cpld_psu_status[i].name) == 0) {
			target = &cpld_psu_status[i];
			break;
		}
	}
	if (target == NULL)
		return -EINVAL;

	val = cpld_reg_read(CPLD_PSU_STATUS_REG);
	if (val < 0)
		return val;

	val8 = (uint8_t)val;
	val8 = (val8 >> ((psu - 1) * CPLD_PSU_SHIFT)) & CPLD_PSU_MASK;

	/*
	** All of the "good" bits must be set.
	** None of the "bad" bits can be set.
	*/
	if ((val8 & target->good_mask) == target->good_mask) {
		if (val8 & target->bad_mask) {
			bad++;
		}
	} else {
		bad++;
	}

	return sprintf(buf, "%s\n", bad ? target->msg_bad : target->msg_good);

}

static SYSFS_ATTR_RO(psu_pwr1_all_ok,	   psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_present,	 psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_ac_ok,	   psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_dc_ok,	   psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_all_ok,	   psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_present,	 psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_ac_ok,	   psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_dc_ok,	   psu_power_show);

/*------------------------------------------------------------------------------
 *
 * System LED definitions
 *
 */

struct led {
	char name[ACCTON_AS5812_54T_CPLD_STRING_NAME_SIZE];
	uint8_t reg;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_psu1",
		.reg  = CPLD_PSU_LED_REG,
		.mask = CPLD_SYS_LED_PSU1_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYS_LED_PSU1_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_PSU1_AMBER},
			{ PLATFORM_LED_OFF, CPLD_SYS_LED_PSU1_OFF},
			{ PLATFORM_LED_HW_CTRL, CPLD_SYS_LED_PSU1_HW_CTRL},
		},
	},
	{
		.name = "led_psu2",
		.reg  = CPLD_PSU_LED_REG,
		.mask = CPLD_SYS_LED_PSU2_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYS_LED_PSU2_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_PSU2_AMBER},
			{ PLATFORM_LED_OFF, CPLD_SYS_LED_PSU2_OFF},
			{ PLATFORM_LED_HW_CTRL, CPLD_SYS_LED_PSU2_HW_CTRL},
		},
	},
	{
		.name = "led_diag",
		.reg  = CPLD_SYSTEM_LED_REG,
		.mask = CPLD_SYS_LED_DIAG_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYS_LED_DIAG_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_DIAG_YELLOW},
			{ PLATFORM_LED_OFF, CPLD_SYS_LED_DIAG_OFF},
			{ PLATFORM_LED_RED, CPLD_SYS_LED_DIAG_RED},
		},
	},
	{
		.name = "led_fan",
		.reg  = CPLD_SYSTEM_LED_REG,
		.mask = CPLD_SYS_LED_FAN_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYS_LED_FAN_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_FAN_AMBER},
			{ PLATFORM_LED_OFF, CPLD_SYS_LED_FAN_OFF},
			{ PLATFORM_LED_HW_CTRL, CPLD_SYS_LED_FAN_HW_CTRL},
		},
	},
	{
		.name = "led_loc",
		.reg  = CPLD_SYSTEM_LED_REG,
		.mask = CPLD_SYS_LED_LOC_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_OFF, CPLD_SYS_LED_LOC_OFF},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_LOC_AMBER},
			{ PLATFORM_LED_YELLOW_BLINKING, CPLD_SYS_LED_LOC_AMBER_BLINK},
		},
	},
};
static int n_leds = ARRAY_SIZE(cpld_leds);

/*
 * Front Panel Status LEDs
 */
static ssize_t led_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	s32 val;
	uint8_t val8;
	int i;
	struct led *target = NULL;

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL)
		return -EINVAL;

	/* read the register */
	val = cpld_reg_read(target->reg);
	if (val < 0)
		return val;

	val8 = (uint8_t)val;

	/* find the color */
	val8 &= target->mask;
	for (i = 0; i < target->n_colors; i++)
		if (val8 == target->colors[i].value)
			break;

	if (i == target->n_colors)
		return sprintf(buf, "undefined color\n");
	else
		return sprintf(buf, "%s\n", target->colors[i].name);

}

static ssize_t led_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	s32 val;
	uint8_t val8;
	int i, ret;
	struct led *target = NULL;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL)
		return -EINVAL;

	/* find the color */
	if (sscanf(buf, "%19s", raw) <= 0)
		return -EINVAL;

	for (i = 0; i < target->n_colors; i++) {
		if (strcmp(raw, target->colors[i].name) == 0) {
			break;
		}
	}
	if (i == target->n_colors)
		return -EINVAL;

	/* set the new value */
	val = cpld_reg_read(target->reg);
	if (val < 0)
		return val;

	val8 = (uint8_t)val;
	val8 &= ~target->mask;
	val8 |= target->colors[i].value;
	ret = cpld_reg_write(target->reg, val8);
	if (ret < 0)
		return ret;

	return count;
}
static SYSFS_ATTR_RW(led_psu1, led_show, led_store);
static SYSFS_ATTR_RW(led_psu2, led_show, led_store);
static SYSFS_ATTR_RW(led_diag, led_show, led_store);
static SYSFS_ATTR_RW(led_fan,  led_show, led_store);
static SYSFS_ATTR_RW(led_loc,  led_show, led_store);

/*------------------------------------------------------------------------------
 *
 * Reset definitions
 *
 */

struct reset_info {
	char name[ACCTON_AS5812_54T_CPLD_STRING_NAME_SIZE];
	uint8_t shift;
	uint8_t reg;
};
static struct reset_info r_info[] = {
	{
		.name = "reset_mux_pca9548_qsfp",
		.shift = CPLD_RESET_SW_PWR2,
		.reg = CPLD_RESET_CONTROL_REG,
	},
	{
		.name = "reset_56864_mac",
		.shift = CPLD_RESET_MAC,
		.reg = CPLD_RESET_CONTROL_REG,
	},
	{
		.name = "reset_phy_mgmt",
		.shift = CPLD_RESET_MGMT_PHY,
		.reg = CPLD_RESET_CONTROL_REG,
	},
	{
		.name = "reset_mux_pca9548",
		.shift = CPLD_RESET_SW_PWR,
		.reg = CPLD_RESET_CONTROL_REG,
	},
	{
		.name = "reset_shift_reg_clear",
		.shift = CPLD_RESET_SHIFT_REG_CLR,
		.reg = CPLD_RESET_CONTROL_REG,
	},
	{
		.name = "reset_phy_12",
		.shift = CPLD_RESET_PHY_12,
		.reg = CPLD_RESET_XGPHY_9_12_REG,
	},
	{
		.name = "reset_phy_11",
		.shift = CPLD_RESET_PHY_11,
		.reg = CPLD_RESET_XGPHY_9_12_REG,
	},
	{
		.name = "reset_phy_10",
		.shift = CPLD_RESET_PHY_10,
		.reg = CPLD_RESET_XGPHY_9_12_REG,
	},
	{
		.name = "reset_phy_9",
		.shift = CPLD_RESET_PHY_9,
		.reg = CPLD_RESET_XGPHY_9_12_REG,
	},
	{
		.name = "reset_phy_8",
		.shift = CPLD_RESET_PHY_8,
		.reg = CPLD_RESET_XGPHY_1_8_REG,
	},
	{
		.name = "reset_phy_7",
		.shift = CPLD_RESET_PHY_7,
		.reg = CPLD_RESET_XGPHY_1_8_REG,
	},
	{
		.name = "reset_phy_6",
		.shift = CPLD_RESET_PHY_6,
		.reg = CPLD_RESET_XGPHY_1_8_REG,
	},
	{
		.name = "reset_phy_5",
		.shift = CPLD_RESET_PHY_5,
		.reg = CPLD_RESET_XGPHY_1_8_REG,
	},
	{
		.name = "reset_phy_4",
		.shift = CPLD_RESET_PHY_4,
		.reg = CPLD_RESET_XGPHY_1_8_REG,
	},
	{
		.name = "reset_phy_3",
		.shift = CPLD_RESET_PHY_3,
		.reg = CPLD_RESET_XGPHY_1_8_REG,
	},
	{
		.name = "reset_phy_2",
		.shift = CPLD_RESET_PHY_2,
		.reg = CPLD_RESET_XGPHY_1_8_REG,
	},
	{
		.name = "reset_phy_1",
		.shift = CPLD_RESET_PHY_1,
		.reg = CPLD_RESET_XGPHY_1_8_REG,
	},
};
static int n_resets = ARRAY_SIZE(r_info);

static ssize_t reset_show(struct device * dev,
			    struct device_attribute * dattr,
			    char * buf)
{
	uint8_t tmp, reg;
	int i;
	struct reset_info *target = NULL;

	for (i = 0; i < n_resets; i++) {
		if (strcmp(dattr->attr.name, r_info[i].name) == 0) {
			target = &r_info[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined target\n");
	}

	reg = target->reg;
	tmp = cpld_reg_read(reg);
	tmp &= target->shift;

	/* reset is active low */
	if (tmp & target->shift)
		return sprintf(buf, "0\n");

	return sprintf(buf, "1\n");
}

static ssize_t reset_store(struct device * dev,
			         struct device_attribute * dattr,
			         const char * buf, size_t count)
{
	int val;
	int i;
	uint8_t tmp;
	uint8_t reg;
	struct reset_info *target = NULL;

	/* find the target dev */
	for (i = 0; i < n_resets; i++) {
		if (strcmp(dattr->attr.name, r_info[i].name) == 0) {
			target = &r_info[i];
			break;
		}
	}
	if (target == NULL) {
		return -EINVAL;
	}

	if (sscanf(buf, "%d", &val) <= 0) {
		return -EINVAL;
	}

	reg = target->reg;
	tmp = cpld_reg_read(reg);
	tmp &= ~target->shift;

	/* reset is active low */
	if (val == 0)
		tmp |= target->shift;

	cpld_reg_write(reg, tmp);
	return count;
}

static SYSFS_ATTR_RW(reset_mux_pca9548_qsfp, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_56864_mac, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_mgmt, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_mux_pca9548, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_12, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_11, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_10, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_9, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_8, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_7, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_6, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_5, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_4, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_3, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_2, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy_1, reset_show, reset_store);

/*
 * Helper API to reset and show all XGPHYs
 */
static ssize_t reset_xgphy_show(struct device * dev,
			         struct device_attribute * dattr,
			         char * buf)
{
	uint8_t tmp8, tmp12;
	uint8_t reg8, reg12;
	int status;

	reg8 = CPLD_RESET_XGPHY_9_12_REG;
	tmp12 = cpld_reg_read(reg8);
	/* reset is active low */
	tmp12 = tmp12 ^ CPLD_RESET_XGPHY_9_12_MASK;

	reg12 = CPLD_RESET_XGPHY_1_8_REG;
	tmp8 = cpld_reg_read(reg12);
	/* reset is active low */
	tmp8 = tmp8 ^ CPLD_RESET_XGPHY_1_8_MASK;

	status = (tmp12 << 8) | tmp8;

	return sprintf(buf, "0x%x\n", status);
}

static ssize_t reset_xgphy_store(struct device * dev,
			         struct device_attribute * dattr,
			         const char * buf, size_t count)
{
	int val;
	int reg8, reg12;
	uint8_t tmp8 = 0, tmp12 = 0;
	uint8_t val8 = 0, val12 = 0;

	if (sscanf(buf, "0x%x", &val) <= 0) {
		return -EINVAL;
	}

	val8 = val & CPLD_RESET_XGPHY_1_8_MASK;
	val12 = (val >> 8 ) & CPLD_RESET_XGPHY_9_12_MASK;

	/* Current values for reset */
	reg12 = CPLD_RESET_XGPHY_9_12_REG;
	tmp12 = cpld_reg_read(reg12);
	/* reset is active low */
	tmp12 = tmp12 ^ CPLD_RESET_XGPHY_9_12_MASK;

	reg8 = CPLD_RESET_XGPHY_1_8_REG;
	tmp8 = cpld_reg_read(reg8);
	/* reset is active low */
	tmp8 = tmp8 ^ CPLD_RESET_XGPHY_1_8_MASK;

	/* reset is active low */
	if (val8 != tmp8) {
		val8 = val8 ^ CPLD_RESET_XGPHY_1_8_MASK;
		cpld_reg_write(reg8, val8);
	}

	if (val12 != tmp12) {
		val12 = val12 ^ CPLD_RESET_XGPHY_9_12_MASK;
		cpld_reg_write(reg12, val12);
	}

return count;
}

static SYSFS_ATTR_RW(reset_xgphy_1_12, reset_xgphy_show, reset_xgphy_store);

/*------------------------------------------------------------------------------
 *
 * SFP status definitions
 *
 * All the definition use positive logic.
 */
struct sfp_status {
	char name[ACCTON_AS5812_54T_CPLD_STRING_NAME_SIZE];
	uint8_t reg;
	uint8_t active_low;
};

static struct sfp_status cpld_qsfp_status[] = {
	{
		.name = "present",
		.reg = CPLD_QSFP_PRESENT_STAT_REG,
		.active_low = 1,
	},
	{
		.name = "fault",
		.reg = CPLD_QSFP_FAULT_STAT_REG,
		.active_low = 1,
	},
	{
		.name = "lp_mode",
		.reg = CPLD_QSFP_LPMODE_REG,
	},
	{
		.name = "reset",
		.reg = CPLD_QSFP_MOD_RESET_REG,
		.active_low = 1,
	},
	{
		.name = "present_interrupt",
		.reg = CPLD_QSFP_PRESENT_INTR_REG,
		.active_low = 1,
	},
};
static int n_qsfp_status = ARRAY_SIZE(cpld_qsfp_status);

static uint8_t qsfp_shift_bit[] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20
};

#define NUM_QSFP_SHIFT_BITS (sizeof(qsfp_shift_bit)/sizeof(uint8_t))
#define QSFP_BIT_MASK       (0x3f)

static ssize_t qsfp_show(struct device *dev,
			 struct device_attribute *dattr,
			 char *buf)
{
	int i;
	s32 read_val;
	uint8_t show_val;
	uint8_t name_len = 5; /* strlen("qsfp_"); */
	struct sfp_status *target = NULL;

	/* find the target register */
	for (i = 0; i < n_qsfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len, cpld_qsfp_status[i].name) == 0) {
			target = &cpld_qsfp_status[i];
			break;
		}
	}
	if (target == NULL)
		return -EINVAL;

	read_val = cpld_reg_read(target->reg);
	if (read_val < 0)
		return read_val;

	show_val = 0;
	for (i = 0; i < NUM_QSFP_SHIFT_BITS; i++)
		if (read_val & (1 << i))
			show_val |= qsfp_shift_bit[i];

	if (target->active_low)
		show_val = ~show_val;

	return sprintf(buf, "0x%02x\n", show_val & QSFP_BIT_MASK);

}

static ssize_t qsfp_store(struct device *dev,
			  struct device_attribute *dattr,
			  const char *buf, size_t count)
{
	int ret, i;
	uint8_t name_len = 5; /* strlen("qsfp_"); */
	uint64_t val64;
	uint8_t write_val;
	struct sfp_status *target = NULL;

	if (hexToInt64(buf, &val64))
		return -EINVAL;

	/* Only the lower 6 bits are valid */
	if (val64 > QSFP_BIT_MASK)
		return -EINVAL;

	/* find the target register */
	for (i = 0; i < n_qsfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len, cpld_qsfp_status[i].name) == 0) {
			target = &cpld_qsfp_status[i];
			break;
		}
	}
	if (target == NULL)
		return -EINVAL;

	write_val = 0;
	for (i = 0; i < NUM_QSFP_SHIFT_BITS; i++)
		if (val64 & (1 << i))
			write_val |= qsfp_shift_bit[i];

	if (target->active_low)
		write_val = ~write_val;
	ret = cpld_reg_write(target->reg, write_val & QSFP_BIT_MASK);
	if (ret < 0)
		return ret;

	return count;
}

static SYSFS_ATTR_RO(qsfp_present_interrupt, qsfp_show);
static SYSFS_ATTR_RO(qsfp_present, qsfp_show);
static SYSFS_ATTR_RO(qsfp_fault,   qsfp_show);
static SYSFS_ATTR_RW(qsfp_lp_mode, qsfp_show, qsfp_store);
static SYSFS_ATTR_RW(qsfp_reset,   qsfp_show, qsfp_store);

static struct attribute *accton_as5812_54t_cpld_attrs[] = {
	&dev_attr_board_id.attr,
	&dev_attr_cpld_version.attr,
	&dev_attr_reset_mux_pca9548_qsfp.attr,
	&dev_attr_reset_56864_mac.attr,
	&dev_attr_reset_phy_mgmt.attr,
	&dev_attr_reset_mux_pca9548.attr,
	&dev_attr_reset_phy_12.attr,
	&dev_attr_reset_phy_11.attr,
	&dev_attr_reset_phy_10.attr,
	&dev_attr_reset_phy_9.attr,
	&dev_attr_reset_phy_8.attr,
	&dev_attr_reset_phy_7.attr,
	&dev_attr_reset_phy_6.attr,
	&dev_attr_reset_phy_5.attr,
	&dev_attr_reset_phy_4.attr,
	&dev_attr_reset_phy_3.attr,
	&dev_attr_reset_phy_2.attr,
	&dev_attr_reset_phy_1.attr,
	&dev_attr_reset_xgphy_1_12.attr,
	&dev_attr_pwm1_enable.attr,
	&dev_attr_fan01_ok.attr,
	&dev_attr_fan02_ok.attr,
	&dev_attr_fan03_ok.attr,
	&dev_attr_fan04_ok.attr,
	&dev_attr_fan05_ok.attr,
	&dev_attr_fan06_ok.attr,
	&dev_attr_fan07_ok.attr,
	&dev_attr_fan08_ok.attr,
	&dev_attr_fan09_ok.attr,
	&dev_attr_fan10_ok.attr,
	&dev_attr_led_fan_tray_1.attr,
	&dev_attr_led_fan_tray_2.attr,
	&dev_attr_led_fan_tray_3.attr,
	&dev_attr_led_fan_tray_4.attr,
	&dev_attr_led_fan_tray_5.attr,
	&dev_attr_air_flow_fan_tray_1.attr,
	&dev_attr_air_flow_fan_tray_2.attr,
	&dev_attr_air_flow_fan_tray_3.attr,
	&dev_attr_air_flow_fan_tray_4.attr,
	&dev_attr_air_flow_fan_tray_5.attr,
	&dev_attr_psu_pwr1_all_ok.attr,
	&dev_attr_psu_pwr1_present.attr,
	&dev_attr_psu_pwr1_ac_ok.attr,
	&dev_attr_psu_pwr1_dc_ok.attr,
	&dev_attr_psu_pwr2_all_ok.attr,
	&dev_attr_psu_pwr2_present.attr,
	&dev_attr_psu_pwr2_ac_ok.attr,
	&dev_attr_psu_pwr2_dc_ok.attr,
	&dev_attr_led_psu1.attr,
	&dev_attr_led_psu2.attr,
	&dev_attr_led_diag.attr,
	&dev_attr_led_fan.attr,
	&dev_attr_led_loc.attr,
	&dev_attr_qsfp_present_interrupt.attr,
	&dev_attr_qsfp_present.attr,
	&dev_attr_qsfp_fault.attr,
	&dev_attr_qsfp_lp_mode.attr,
	&dev_attr_qsfp_reset.attr,
	&dev_attr_name.attr,
	NULL,
};

static struct attribute_group accton_as5812_54t_cpld_attr_group = {
	.attrs = accton_as5812_54t_cpld_attrs,
};

static struct attribute *accton_as5812_54t_cpld_sensor_attrs[] = {
	&dev_attr_fan01_input.attr,
	&dev_attr_fan02_input.attr,
	&dev_attr_fan03_input.attr,
	&dev_attr_fan04_input.attr,
	&dev_attr_fan05_input.attr,
	&dev_attr_fan06_input.attr,
	&dev_attr_fan07_input.attr,
	&dev_attr_fan08_input.attr,
	&dev_attr_fan09_input.attr,
	&dev_attr_fan10_input.attr,
	&dev_attr_pwm1.attr,
	NULL,
};

ATTRIBUTE_GROUPS(accton_as5812_54t_cpld_sensor);

struct accton_as5812_54t_cpld_data {
	struct device *hwmon_dev;
};
static struct accton_as5812_54t_cpld_data cpld_data;

static int accton_as5812_54t_cpld_probe(struct platform_device *dev)
{
	s32 ret = 0;

	ret = sysfs_create_group(&dev->dev.kobj, &accton_as5812_54t_cpld_attr_group);
	if (ret) {
		pr_err("sysfs_create_group failed for cpld driver");
		return ret;
	}
	cpld_data.hwmon_dev = hwmon_device_register_with_groups(&dev->dev,
						dev->name,
						NULL,
						accton_as5812_54t_cpld_sensor_groups);
	if (IS_ERR(cpld_data.hwmon_dev)) {
		ret = PTR_ERR(cpld_data.hwmon_dev);
		dev_err(&dev->dev, "hwmon registration failed");
		goto err_hwmon_device;
	}

	return 0;

err_hwmon_device:
	sysfs_remove_group(&dev->dev.kobj, &accton_as5812_54t_cpld_attr_group);
	return ret;

}

static int accton_as5812_54t_cpld_remove(struct platform_device *dev)
{

	if (cpld_data.hwmon_dev)
		hwmon_device_unregister(cpld_data.hwmon_dev);

	sysfs_remove_group(&dev->dev.kobj, &accton_as5812_54t_cpld_attr_group);

	return 0;
}

static struct platform_driver accton_as5812_54t_cpld_driver = {
	.driver = {
		.name = "accton_as5812_54t_cpld",
		.owner = THIS_MODULE,
	},
	.probe = accton_as5812_54t_cpld_probe,
	.remove = accton_as5812_54t_cpld_remove,
};

static struct platform_device *accton_as5812_54t_cpld_device;

/**
 * accton_as5812_54t_i2c_init -- CPLD I2C devices
 *
 * Create a device that provides generic access to the CPLD registers.
 */
static int __init accton_as5812_54t_cpld_init(void)
{
	int i;
	int ret;

	/* Find the CPLD I2C devices -- their I2C addresses are unique */
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		if (i2c_clients[i]) {
			switch (i2c_clients[i]->addr) {
			case 0x60:
				cpld_i2c_clients = i2c_clients[i];
				break;
			default:
				continue;
			}
		}
	}

	/* Verify we found them all */
	if (cpld_i2c_clients == NULL) {
		pr_err("Unable to find all CPLD I2C devices.  "
		       "Missing cpld_i2c_clients 0x60\n");
		return -ENODEV;
	}

	ret = platform_driver_register(&accton_as5812_54t_cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for cpld device");
		goto err_drvr;
	}

	accton_as5812_54t_cpld_device = platform_device_alloc("accton_as5812_54t_cpld", 0);
	if (!accton_as5812_54t_cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}

	ret = platform_device_add(accton_as5812_54t_cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_dev_add;
	}
	return 0;

err_dev_add:
	platform_device_put(accton_as5812_54t_cpld_device);

err_dev_alloc:
	platform_driver_unregister(&accton_as5812_54t_cpld_driver);

err_drvr:
	return ret;
}

static void __exit accton_as5812_54t_cpld_exit(void)
{
	platform_driver_unregister(&accton_as5812_54t_cpld_driver);
	platform_device_unregister(accton_as5812_54t_cpld_device);
}

static int __init accton_as5812_54t_platform_init(void)
{
	int ret = 0;

	ret = accton_as5812_54t_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		return ret;
	}

	ret = accton_as5812_54t_cpld_init();
	if (ret) {
		pr_err("Registering CPLD driver failed.\n");
		goto err_cpld_init;
	}

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;

err_cpld_init:
	free_i2c_data();

	return ret;
}

static void __exit accton_as5812_54t_platform_exit(void)
{
	accton_as5812_54t_cpld_exit();
	accton_as5812_54t_i2c_exit();
	pr_info(DRIVER_NAME": version "DRIVER_VERSION" unloaded\n");
}

module_init(accton_as5812_54t_platform_init);
module_exit(accton_as5812_54t_platform_exit);

MODULE_AUTHOR("Vidya Sagar Ravipati (vidya@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton AS5812-54T Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
