/*
 * accton_as5712_54x_platform.c - Accton AS5712-54x Platform Support.
 *
 * Copyright (C) 2016, 2019, 2020 Cumulus Networks, Inc.
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
 * https://www.gnu.org/licenses/gpl-2.0-standalone.html
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include "platform-defs.h"
#include "accton-as5712-54x-cpld.h"

#define DRIVER_NAME	"accton_as5712_54x_platform"
#define DRIVER_VERSION	"1.1"

/*
 * The platform has 2 types of i2c SMBUSes, i801 (Intel 82801
 * (ICH/PCH)) and ISMT (Intel SMBus Message Transport).  ISMT has a
 * PCA9548 mux.
 */

/* i2c bus adapter numbers for the 8 down stream PCA9548 i2c busses */
enum {
	AS5712_I2C_ISMT_BUS=0,
	AS5712_I2C_I801_BUS,
	AS5712_I2C_PCA9548_BUS_0 = 10,
	AS5712_I2C_PCA9548_BUS_1,
	AS5712_I2C_PCA9548_BUS_2,
	AS5712_I2C_PCA9548_BUS_3,
	AS5712_I2C_PCA9548_BUS_4,
	AS5712_I2C_PCA9548_BUS_5,
	AS5712_I2C_PCA9548_BUS_6,
	AS5712_I2C_PCA9548_BUS_7,
	AS5712_I2C_PORT_EEPROM_BUS_0 = 20,
};

mk_eeprom(spd1, 52, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(spd2, 53, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(psu1, 38, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2, 3b, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu1_3y, 50, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2_3y, 53, 256, AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(board,57, 256, AT24_FLAG_IRUGO);

/*
 * Platform data for the PCA9548 MUX
 */
static struct pca954x_platform_mode __initdata pca9548_mux_platform_modes [] = {
	{
		.adap_id = AS5712_I2C_PCA9548_BUS_0, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5712_I2C_PCA9548_BUS_1, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5712_I2C_PCA9548_BUS_2, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5712_I2C_PCA9548_BUS_3, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5712_I2C_PCA9548_BUS_4, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5712_I2C_PCA9548_BUS_5, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5712_I2C_PCA9548_BUS_6, .deselect_on_exit = 1,
	},
	{
		.adap_id = AS5712_I2C_PCA9548_BUS_7, .deselect_on_exit = 1,
	},
};

static struct pca954x_platform_data __initdata pca9548_mux_platform_data = {
	.modes = pca9548_mux_platform_modes,
	.num_modes = ARRAY_SIZE(pca9548_mux_platform_modes),
};

static struct platform_i2c_device_info __initdata i2c_devices[] = {
	/* Begin I2C_ISMT Bus */
	{
		.bus = AS5712_I2C_ISMT_BUS,
		.board_info = {
			I2C_BOARD_INFO("pca9548", 0x70),  /* PCA9548 8 Port I2C Switch */
			.platform_data = &pca9548_mux_platform_data,
		},
	},
	{
		.bus = AS5712_I2C_ISMT_BUS,
		.board_info = {
			I2C_BOARD_INFO("24c02", 0x57),	/* Board EEPROM */
			.platform_data = &board_57_at24,
		},
	},
	/* Begin PCA9548 BUS 0 */
	{
		.bus = AS5712_I2C_PCA9548_BUS_0,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x2c), /* USB Hub Controller */
		},
	},
	/* Begin PCA9548 BUS 1 */
	/* Compuware PSU 1 */
	{
		.bus = AS5712_I2C_PCA9548_BUS_1,
		.board_info = {
			I2C_BOARD_INFO("24c02", 0x38),   /* PSU 1 EEPROM */
			.platform_data = &psu1_38_at24,
		},
	},
	{
		.bus = AS5712_I2C_PCA9548_BUS_1,
		.board_info = {
			I2C_BOARD_INFO("cpr4011", 0x3c),   /* PSU 1 PMBUS */
		},
	},
	/* 3Y Power PSU 1 */
	{
		.bus = AS5712_I2C_PCA9548_BUS_1,
		.board_info = {
			I2C_BOARD_INFO("24c02", 0x50),   /* PSU 1 EEPROM */
			.platform_data = &psu1_3y_50_at24,
		},
	},
	{
		.bus = AS5712_I2C_PCA9548_BUS_1,
		.board_info = {
			I2C_BOARD_INFO("cpr4011", 0x58),   /* PSU 1 PMBUS */
		},
	},
	/* Begin PCA9548 BUS 2 */
	/* Compuware PSU 2 */
	{
		.bus = AS5712_I2C_PCA9548_BUS_2,
		.board_info = {
			I2C_BOARD_INFO("24c02", 0x3b),   /* PSU 2 EEPROM */
			.platform_data = &psu2_3b_at24,
		},
	},
	{
		.bus = AS5712_I2C_PCA9548_BUS_2,
		.board_info = {
			I2C_BOARD_INFO("cpr4011", 0x3f),   /* PSU 2 PMBUS */
		},
	},
	/* 3Y Power PSU 2 */
	{
		.bus = AS5712_I2C_PCA9548_BUS_2,
		.board_info = {
			I2C_BOARD_INFO("24c02", 0x53),   /* PSU 2 EEPROM */
			.platform_data = &psu2_3y_53_at24,
		},
	},
	{
		.bus = AS5712_I2C_PCA9548_BUS_2,
		.board_info = {
			I2C_BOARD_INFO("cpr4011", 0x5b),   /* PSU 2 PMBUS */
		},
	},
	/* Begin PCA9548 BUS 3 */
	{
		.bus = AS5712_I2C_PCA9548_BUS_3,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x08), /* TODO: DC/DC Power */
		},
	},
	/* PCA9548 BUS 4 is empty */
	/* Begin PCA9548 BUS 5 */
	{
		.bus = AS5712_I2C_PCA9548_BUS_5,
		.board_info = {
			I2C_BOARD_INFO("lm75", 0x48), /* LM75 Temperature Sensor */
		},
	},
	/* Begin PCA9548 BUS 6 */
	{
		.bus = AS5712_I2C_PCA9548_BUS_6,
		.board_info = {
			I2C_BOARD_INFO("lm75", 0x49), /* LM75 Temperature Sensor */
		},
	},
	/* Begin PCA9548 BUS 7 */
	{
		.bus = AS5712_I2C_PCA9548_BUS_7,
		.board_info = {
			I2C_BOARD_INFO("lm75", 0x4a), /* LM75 Temperature Sensor */
		},
	},
	/* Begin I2C_I801 Bus */
	{
		.bus = AS5712_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x2e), /* ISL9027 Potentiometer */
		},
	},
	{
		.bus = AS5712_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("spd", 0x52),  /* DIMM */
			.platform_data = &spd1_52_at24,
		},
	},
	{
		.bus = AS5712_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("spd", 0x53),  /* DIMM */
			.platform_data = &spd2_53_at24,
		},
	},
	{
		.bus = AS5712_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x60), /* System CPLD */
		},
	},
	{
		.bus = AS5712_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x61), /* Port CPLD: 1 - 24 */
		},
	},
	{
		.bus = AS5712_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x62), /* Port CPLD: 25 - 54 */
		},
	},
	{
		.bus = AS5712_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("dummy", 0x69), /* Clock Gen */
		},
	},
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

/**
 *
 * TODO:  This should be common code used by most platforms....
 *
 */
static int __init get_bus_by_name(char *name)
{
	struct i2c_adapter *adapter;
	int i;

	for (i = 0; i < AS5712_I2C_PCA9548_BUS_0; i++) {
		adapter = get_adapter(i);
		if (adapter && (strncmp(adapter->name, name, strlen(name)) == 0)) {
			i2c_put_adapter(adapter);
			return i;
		}
		i2c_put_adapter(adapter);
	}
	return -1;
}

/**
 *
 * TODO:  This should be common code used by most platforms....
 *
 */
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

static struct i2c_adapter *i801_adapter;

/**
 * accton_as5712_54x_i2c_init -- Initialize I2C devices
 *
 */
static int __init accton_as5712_54x_i2c_init(void)
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
		 * Map logical buses S5712_I2C_ISMT_BUS and
		 * AS5712_I2C_I801_BUS to their dynamically discovered
		 * bus numbers.
		 */
		switch (i2c_devices[i].bus) {
		case AS5712_I2C_ISMT_BUS:
			i2c_devices[i].bus = iSMT_bus_num;
			break;
		case AS5712_I2C_I801_BUS:
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

static void accton_as5712_54x_i2c_exit(void)
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
static struct i2c_client *cpld_i2c_clients[NUM_CPLD_I2C_CLIENTS];

/**
 * cpld_reg_read - Read an 8-bit CPLD register over i2c
 * @reg: CPLD Register offset to read
 *
 * Returns a negative errno else a data byte received from the device.
 */
static s32 cpld_reg_read(uint32_t reg)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	int val;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("attempt to read invalid CPLD register [0x%02X]", reg);
		return -EINVAL;
	}
	val = i2c_smbus_read_byte_data(cpld_i2c_clients[cpld_idx],
				       STRIP_CPLD_IDX(reg));
	if (val < 0) {
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       cpld_i2c_clients[cpld_idx]->addr, STRIP_CPLD_IDX(reg));
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
	int cpld_idx = GET_CPLD_IDX(reg);
	int res;
	struct i2c_client *client;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("attempt to write to invalid CPLD register [0x%02X]", reg);
		return -EINVAL;
	}
	client = cpld_i2c_clients[cpld_idx];
	reg = STRIP_CPLD_IDX(reg);

	res = i2c_smbus_write_byte_data(cpld_i2c_clients[cpld_idx],
					STRIP_CPLD_IDX(reg), val);
	if (res) {
		pr_err("I2C write error - addr: 0x%02X, offset: 0x%02X, val: 0x%02X",
		       cpld_i2c_clients[cpld_idx]->addr, reg, val);
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
	uint8_t reg;

	switch (dattr->attr.name[4]) {
	case '1':
		reg = CPLD_SYS_CPLD_VERSION_REG; break;
	case '2':
		reg = CPLD_PORT1_24_CPLD_VERSION_REG; break;
	case '3':
		reg = CPLD_PORT25_54_CPLD_VERSION_REG; break;
	default:
		return -EINVAL;
	};

	val = cpld_reg_read(reg);
	if (val < 0)
		return val;

	return sprintf(buf, "%d\n", val);

}
static SYSFS_ATTR_RO(cpld1_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld2_version, cpld_version_show);
static SYSFS_ATTR_RO(cpld3_version, cpld_version_show);

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
#define ACCTON_AS5712_54X_CPLD_STRING_NAME_SIZE 30
struct cpld_status {
	char name[ACCTON_AS5712_54X_CPLD_STRING_NAME_SIZE];
	uint8_t good_mask;
	uint8_t bad_mask;
	char msg_good[ACCTON_AS5712_54X_CPLD_STRING_NAME_SIZE];
	char msg_bad[ACCTON_AS5712_54X_CPLD_STRING_NAME_SIZE];
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
	char name[ACCTON_AS5712_54X_CPLD_STRING_NAME_SIZE];
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
static SYSFS_ATTR_RW(led_fan,	led_show, led_store);
static SYSFS_ATTR_RW(led_loc,	led_show, led_store);

/*------------------------------------------------------------------------------
 *
 * SFP status definitions
 *
 * All the definition use positive logic.
 */
#define ACCTON_AS5712_54X_CPLD_STRING_NAME_SIZE 30
struct sfp_status {
	char name[ACCTON_AS5712_54X_CPLD_STRING_NAME_SIZE];
	uint8_t reg;
	uint8_t active_low;
};

static struct sfp_status cpld_sfp_status[] = {
	{
		.name = "present",
		.reg = CPLD_SFP1_8_PRESENT_STAT_REG,
		.active_low = 1,
	},
	{
		.name = "tx_fault",
		.reg = CPLD_SFP1_8_TX_FAULT_REG,
	},
	{
		.name = "tx_enable",
		.reg = CPLD_SFP1_8_TX_DISABLE_REG,
		.active_low = 1,
	},
	{
		.name = "rx_los",
		.reg = CPLD_SFP1_8_RX_LOS_REG,
	},
};
static int n_sfp_status = ARRAY_SIZE(cpld_sfp_status);

static ssize_t sfp_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	int i;
	s32 val;
	uint8_t reg;
	uint8_t name_len = 4; /* strlen("sfp_"); */
	struct sfp_status *target = NULL;
	uint64_t status_mask = 0;

	/* find the target register */
	for (i = 0; i < n_sfp_status; i++) {
		if (strcmp(dattr->attr.name + name_len, cpld_sfp_status[i].name) == 0) {
			target = &cpld_sfp_status[i];
			break;
		}
	}
	if (target == NULL)
		return -EINVAL;

	/* Assemble the 6 8-bit status register values into a single 48-bit return value */
	for (i = 0; i < 6; i++) {
		reg = target->reg + (i % 3) +
			((CPLD_SFP25_32_PRESENT_STAT_REG - CPLD_SFP1_8_PRESENT_STAT_REG) * (i / 3));
		val = cpld_reg_read(reg);
		if (val < 0)
			return val;
		if (target->active_low)
			val = ~val;
		status_mask |= ((uint64_t)(val & 0xFF)) << (i * 8);
	}

	return sprintf(buf, "0x%llx\n", status_mask);

}

static int hexToInt64(const char *hex_str, uint64_t *val)
{
	char prefix[] = "0x";
	if (strncasecmp(hex_str, prefix, strlen(prefix)) == 0) {
		hex_str += strlen(prefix);
	}
	return sscanf(hex_str, "%llx", val) != 1;
}

static ssize_t sfp_tx_enable_store(struct device *dev,
				   struct device_attribute *dattr,
				   const char *buf, size_t count)
{
	int ret, i;
	uint64_t enable_mask;
	uint8_t val, reg;

	if (hexToInt64(buf, &enable_mask))
		return -EINVAL;

	/* 48-bit tx_enable register is active low */
	enable_mask = ~enable_mask & ((1LL << 48) - 1);

	/* Disassemble the 48-bit mask into 6 8-bit register values */
	for (i = 0; i < 6; i++) {
		reg = CPLD_SFP1_8_TX_DISABLE_REG + (i % 3) +
			((CPLD_SFP25_32_PRESENT_STAT_REG - CPLD_SFP1_8_PRESENT_STAT_REG) * (i / 3));
		val = (enable_mask >> (i * 8)) & 0xFF;
		ret = cpld_reg_write(reg, val);
		if (ret < 0)
			return ret;
	}

	return count;
}

static SYSFS_ATTR_RO(sfp_present,   sfp_show);
static SYSFS_ATTR_RO(sfp_tx_fault,  sfp_show);
static SYSFS_ATTR_RO(sfp_rx_los,    sfp_show);
static SYSFS_ATTR_RW(sfp_tx_enable, sfp_show, sfp_tx_enable_store);

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
};
static int n_qsfp_status = ARRAY_SIZE(cpld_qsfp_status);

/* the qsfp ports as defined in hardware
 * don't match what is on the front panel.
 * qsfp bit positions 5 - 0 correspond to the
 * front panel silkscreen like this:
 *   54, 51, 53, 50, 52, 49
 * we will use the bit shifts below to shuffle things around
 * so that bits 5 - 0 match the silkscreen as:
 *   54, 53, 52, 51, 50, 49
 * which is much easier to use.
 */
static uint8_t qsfp_read_shift_bit[] = {
	0x01, 0x08, 0x02, 0x10, 0x04, 0x20
};

static uint8_t qsfp_write_shift_bit[] = {
	0x01, 0x04, 0x10, 0x02, 0x08, 0x20
};
#define NUM_QSFP_SHIFT_BITS (sizeof(qsfp_write_shift_bit)/sizeof(uint8_t))
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
			show_val |= qsfp_read_shift_bit[i];

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
			write_val |= qsfp_write_shift_bit[i];

	if (target->active_low)
		write_val = ~write_val;
	ret = cpld_reg_write(target->reg, write_val & QSFP_BIT_MASK);
	if (ret < 0)
		return ret;

	return count;
}

static SYSFS_ATTR_RO(qsfp_present, qsfp_show);
static SYSFS_ATTR_RO(qsfp_fault,   qsfp_show);
static SYSFS_ATTR_RW(qsfp_lp_mode, qsfp_show, qsfp_store);
static SYSFS_ATTR_RW(qsfp_reset,   qsfp_show, qsfp_store);

static struct attribute *accton_as5712_54x_cpld_attrs[] = {
	&dev_attr_board_id.attr,
	&dev_attr_cpld1_version.attr,
	&dev_attr_cpld2_version.attr,
	&dev_attr_cpld3_version.attr,
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
	&dev_attr_sfp_present.attr,
	&dev_attr_sfp_tx_fault.attr,
	&dev_attr_sfp_rx_los.attr,
	&dev_attr_sfp_tx_enable.attr,
	&dev_attr_qsfp_present.attr,
	&dev_attr_qsfp_fault.attr,
	&dev_attr_qsfp_lp_mode.attr,
	&dev_attr_qsfp_reset.attr,
	&dev_attr_name.attr,
	NULL,
};

static struct attribute_group accton_as5712_54x_cpld_attr_group = {
	.attrs = accton_as5712_54x_cpld_attrs,
};

static struct attribute *accton_as5712_54x_cpld_sensor_attrs[] = {
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

ATTRIBUTE_GROUPS(accton_as5712_54x_cpld_sensor);

struct accton_as5712_54x_cpld_data {
	struct device *hwmon_dev;
};
static struct accton_as5712_54x_cpld_data cpld_data;

static int accton_as5712_54x_cpld_probe(struct platform_device *dev)
{
	s32 ret = 0;

	ret = sysfs_create_group(&dev->dev.kobj, &accton_as5712_54x_cpld_attr_group);
	if (ret) {
		pr_err("sysfs_create_group failed for cpld driver");
		return ret;
	}
	cpld_data.hwmon_dev = hwmon_device_register_with_groups(&dev->dev,
					dev->name,
					NULL,
					accton_as5712_54x_cpld_sensor_groups);
	if (IS_ERR(cpld_data.hwmon_dev)) {
		ret = PTR_ERR(cpld_data.hwmon_dev);
		dev_err(&dev->dev, "hwmon registration failed");
		goto err_hwmon_device;
	}

	return 0;

err_hwmon_device:
	sysfs_remove_group(&dev->dev.kobj, &accton_as5712_54x_cpld_attr_group);
	return ret;

}

static int accton_as5712_54x_cpld_remove(struct platform_device *dev)
{

	if (cpld_data.hwmon_dev)
		hwmon_device_unregister(cpld_data.hwmon_dev);

	sysfs_remove_group(&dev->dev.kobj, &accton_as5712_54x_cpld_attr_group);

	return 0;
}

static struct platform_driver accton_as5712_54x_cpld_driver = {
	.driver = {
		.name = "accton_as5712_54x_cpld",
		.owner = THIS_MODULE,
	},
	.probe = accton_as5712_54x_cpld_probe,
	.remove = accton_as5712_54x_cpld_remove,
};

static struct platform_device *accton_as5712_54x_cpld_device;

/**
 * accton_as5712_54x_i2c_init -- CPLD I2C devices
 *
 * Create a device that provides generic access to the CPLD registers.
 */
static int __init accton_as5712_54x_cpld_init(void)
{
	int i;
	int ret;

	/* Find the 3 CPLD I2C devices -- their I2C addresses are unique */
	for (i = 0; i < ARRAY_SIZE(i2c_devices); i++) {
		if (i2c_clients[i]) {
			switch (i2c_clients[i]->addr) {
			case 0x60:
				cpld_i2c_clients[CPLD_SYSTEM_ID - 1] = i2c_clients[i];
				break;
			case 0x61:
				cpld_i2c_clients[CPLD_PORT1_24_ID - 1] = i2c_clients[i];
				break;
			case 0x62:
				cpld_i2c_clients[CPLD_PORT25_54_ID - 1] = i2c_clients[i];
				break;
			default:
				continue;
			}
		}
	}

	/* Verify we found them all */
	for (i = 0; i < ARRAY_SIZE(cpld_i2c_clients); i++) {
		if (cpld_i2c_clients[i] == NULL) {
			pr_err("Unable to find all CPLD I2C devices.  "
			       "Missing cpld_i2c_clients[%d]\n", i);
			return -ENODEV;
		}
	}

	ret = platform_driver_register(&accton_as5712_54x_cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for cpld device");
		goto err_drvr;
	}

	accton_as5712_54x_cpld_device = platform_device_alloc("accton_as5712_54x_cpld", 0);
	if (!accton_as5712_54x_cpld_device) {
		pr_err("platform_device_alloc() failed for cpld device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}

	ret = platform_device_add(accton_as5712_54x_cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for cpld device.\n");
		goto err_dev_add;
	}
	return 0;

err_dev_add:
	platform_device_put(accton_as5712_54x_cpld_device);

err_dev_alloc:
	platform_driver_unregister(&accton_as5712_54x_cpld_driver);

err_drvr:
	return ret;
}

/*************************************************/
/* BEGIN Port EEPROM MUX Driver                  */
/*************************************************/

/**
 * port_eeprom_mux_i2c_reg_write - Writes to CPLDs from within an I2C mux callback
 * @reg: CPLD Register offset to write
 * @val: 8-bit value to write to CPLD register
 *
 * Returns a negative errno else zero on success.
 */
static int port_eeprom_mux_i2c_reg_write(uint8_t reg, u8 val)
{
	int rv;
	struct i2c_client *client;
	struct i2c_adapter *adap;
	extern struct i2c_adapter *i801_adapter;

	/*
	 * Write to a register in our synthetic port EEPROM mux.
	 */

	/*
	 * Normally, we would call i2c_get_adapter() here to increase the
	 * i801 adapter's refcount and then put it back when we return.
	 * When using the i2c_mux_add_adapter() framework, that causes
	 * deadlocks on i2c_core_lock here.  So we wing it a little by
	 * saving a copy of the adapter and referring to it here, on the
	 * premise that we never expect to go to a zero refcount on i801
	 * during an EEPROM reg write.
	 */
	adap = i801_adapter;
	if (!adap) {
		pr_err("Unable to get i801 bus adapter.\n");
		return -ENODEV;
	}

	client = cpld_i2c_clients[GET_CPLD_IDX(reg)];
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

static int port_eeprom_mux_deselect_chan(struct i2c_mux_core *muxc, u32 chan)
{
	/* Since we can only access one SFP EEPROM at a time,
	   we can do a clear at both CPLD registers, just to be safe. */
	port_eeprom_mux_i2c_reg_write(CPLD_PORT1_24_I2C_SELECT_REG, 0xff);
	port_eeprom_mux_i2c_reg_write(CPLD_PORT25_54_I2C_SELECT_REG, 0xff);

	return 0;
}

static int port_eeprom_mux_select_chan(struct i2c_mux_core *muxc, u32 chan)
{
	int val = 0xff;
	u32 mux_reg = 0;

	if (chan < EEPROM_MUX_PORT_RANGE1_SFP) {
		mux_reg = CPLD_PORT1_24_I2C_SELECT_REG;
		val = chan;
	} else if (chan < EEPROM_MUX_PORT_RANGE_QSFP) {
		mux_reg = CPLD_PORT25_54_I2C_SELECT_REG;
		val = chan - EEPROM_MUX_PORT_RANGE1_SFP;
		/*
		 * Reorder the QSFP ports to match the silkscreen.
		 * The first and last QSFP are okay, but the other four
		 * need to be shuffled.
		 */
		switch (val) {
			case 25:       // port 50 => port 51
				val = 26;
				break;
			case 26:       // port 51 => port 53
				val = 28;
				break;
			case 27:       // port 52 => port 50
				val = 25;
				break;
			case 28:       // port 53 => port 52
				val = 27;
				break;
		}
	} else {
		pr_err("invalid mux channel number %u\n", chan);
		return -EINVAL;
	}

	port_eeprom_mux_i2c_reg_write(mux_reg, val);

	return 0;
}

/* Platform device for the port EEPROM mux device */
static struct platform_device *port_eeprom_mux_dev;

/**
 * struct port_eeprom_mux_data - Platform specific data for port EEPROM mux
 *
 */
struct port_eeprom_mux_data {
	struct i2c_client *client;
	struct i2c_board_info *eeprom_info;
};
static struct i2c_mux_core *muxc;
static struct port_eeprom_mux_data mux_data[EEPROM_MUX_TOTAL_PORTS];

static bool is_qsfp_port(int port) {
	if (port >= 49)
		return true;
	else
		return false;
}

/**
 * alloc_port_i2c_board_info -- Allocate an i2c_board_info struct
 * @port - front panel port number, one based
 *
 * For each port in the system allocate an i2c_board_info struct to
 * describe the SFP+/QSFP at24 EEPROM.
 *
 * Returns the allocated i2c_board_info struct or else returns NULL on failure.
 */
#define EEPROM_LABEL_SIZE  8
static struct __init i2c_board_info *alloc_port_i2c_board_info(int port) {
	char *label = NULL;
	struct eeprom_platform_data *eeprom_data = NULL;
	struct i2c_board_info *board_info = NULL;

	label = kzalloc(EEPROM_LABEL_SIZE, GFP_KERNEL);
	if (!label)
		goto err_kzalloc;

	eeprom_data = kzalloc(sizeof(struct eeprom_platform_data), GFP_KERNEL);
	if (!eeprom_data)
		goto err_kzalloc;

	board_info = kzalloc(sizeof(struct i2c_board_info), GFP_KERNEL);
	if (!board_info)
		goto err_kzalloc;

	snprintf(label, EEPROM_LABEL_SIZE, "port%u", port);
	eeprom_data->label = label;

	if (is_qsfp_port(port)) {
		struct sff_8436_platform_data *sff8436_data;
		sff8436_data = kzalloc(sizeof(struct sff_8436_platform_data), GFP_KERNEL);
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
		at24_data = kzalloc(sizeof(struct at24_platform_data), GFP_KERNEL);
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

/**
 * free_port_eeprom_mux_data -- Free all the resources related to the EEPROM mux
 *
 */
static void free_port_eeprom_mux_data(void)
{
	int i;
	extern struct i2c_mux_core *muxc;

	for (i = 0; i < EEPROM_MUX_TOTAL_PORTS; i++) {
		if (mux_data[i].client)
			i2c_unregister_device(mux_data[i].client);
		if (mux_data[i].eeprom_info) {
			struct i2c_board_info *board_info = mux_data[i].eeprom_info;
			struct eeprom_platform_data *eeprom_data;

			if (is_qsfp_port(i + 1)) {
				struct sff_8436_platform_data *sff8436_data;
				sff8436_data = board_info->platform_data;
				eeprom_data = sff8436_data->eeprom_data;
				kfree(sff8436_data);
			} else {
				struct at24_platform_data *at24_data;
				at24_data = board_info->platform_data;
				eeprom_data = at24_data->eeprom_data;
				kfree(at24_data);
			}
			kfree(eeprom_data->label);
			kfree(eeprom_data);
			kfree(board_info);
		}
	}
	if (muxc)
		i2c_mux_del_adapters(muxc);
}

/**
 * accton_as5712_54x_port_eeprom_mux_init - Create a MUX for SFP+/QSFP EEPROMs
 *
 * All of the port SFP+ and QSFP EEPROMs are muxed onto the I2C_I801
 * Bus at address 0x50.  The active port is selected by the
 * CPLD_PORT1_24_I2C_SELECT_REG and CPLD_PORT25_54_I2C_SELECT_REG
 * registers.
 *
 * This routine creates a I2C mux devices to service these EEPROMs.
 *
 * Returns a negative errno else zero on success.
 */
static int __init accton_as5712_54x_port_eeprom_mux_init(void)
{
	int ret = 0;
	int i;
	int nr;
	struct i2c_adapter *parent;
	extern struct i2c_adapter *i801_adapter;

	parent = i2c_get_adapter(i801_bus_num);
	if (!parent) {
		pr_err("Unable to find i801_bus i2c adapter.\n");
		return -ENXIO;
	}
	i801_adapter = parent;

	port_eeprom_mux_dev = platform_device_alloc("cpld-port-eeprom-mux", 0);
	if (!port_eeprom_mux_dev) {
		pr_err("platform_device_alloc() failed for port eeprom mux.\n");
		ret = -ENOMEM;
		goto mux_err_adapter;
	}

	ret = platform_device_add(port_eeprom_mux_dev);
	if (ret) {
		pr_err("platform_device_add() failed for port eeprom mux.\n");
		goto mux_err_device_add;
	}

	muxc = i2c_mux_alloc(parent, &port_eeprom_mux_dev->dev,
			     EEPROM_MUX_TOTAL_PORTS, 0, 0,
			     port_eeprom_mux_select_chan,
			     port_eeprom_mux_deselect_chan);
	if (!muxc) {
		pr_err("i2c_mux_alloc() failed for %u muxes.\n",
		       EEPROM_MUX_TOTAL_PORTS);
		ret = -ENOMEM;
		goto mux_err_device_add;
	}

	nr = AS5712_I2C_PORT_EEPROM_BUS_0;
	for (i = 0; i < EEPROM_MUX_TOTAL_PORTS; i++, nr++) {
		struct i2c_board_info *board_info;
		struct i2c_client *client;

		ret = i2c_mux_add_adapter(muxc, nr, i, 0);
		if (ret) {
			pr_err("i2c_mux_add_adapter() failed for channel %u.\n",i);
			goto mux_err_i2c_add_mux;
		}

		board_info = alloc_port_i2c_board_info(i + 1);
		if (!board_info) {
			pr_err("Failed to allocate i2c board info for channel %u.\n", i);
			ret = -ENOMEM;
			goto mux_err_i2c_add_mux;
		}
		mux_data[i].eeprom_info = board_info;

		client = i2c_new_device(muxc->adapter[i], board_info);
		if (!client) {
			pr_err("i2c_new_device failed for channel %u.\n", i);
			ret = -ENOMEM;
			goto mux_err_i2c_add_mux;
		}
		mux_data[i].client = client;
	}

	i2c_put_adapter(parent);
	return 0;

mux_err_i2c_add_mux:
	free_port_eeprom_mux_data();

mux_err_device_add:
	platform_device_put(port_eeprom_mux_dev);

mux_err_adapter:
	i2c_put_adapter(parent);

	return ret;
}

static void __exit accton_as5712_54x_port_eeprom_mux_exit(void)
{
     free_port_eeprom_mux_data();
     platform_device_unregister(port_eeprom_mux_dev);
}

static void accton_as5712_54x_cpld_exit(void)
{
	platform_driver_unregister(&accton_as5712_54x_cpld_driver);
	platform_device_unregister(accton_as5712_54x_cpld_device);
}

static int __init accton_as5712_54x_platform_init(void)
{
	int ret = 0;

	ret = accton_as5712_54x_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		return ret;
	}

	ret = accton_as5712_54x_cpld_init();
	if (ret) {
		pr_err("Registering CPLD driver failed.\n");
		goto err_cpld_init;
	}

	ret = accton_as5712_54x_port_eeprom_mux_init();
	if (ret) {
		pr_err("Initializing SFP+/QSFP EEPROM mux failed.\n");
		goto err_eeprom_mux_init;
	}

	/* Clear the mux, in case it points to a SFP EEPROM by default */
	port_eeprom_mux_i2c_reg_write(CPLD_PORT1_24_I2C_SELECT_REG, 0xff);
	port_eeprom_mux_i2c_reg_write(CPLD_PORT25_54_I2C_SELECT_REG, 0xff);

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;

err_cpld_init:
	accton_as5712_54x_i2c_exit();

err_eeprom_mux_init:
	accton_as5712_54x_cpld_exit();

	return ret;
}

static void __exit accton_as5712_54x_platform_exit(void)
{
	accton_as5712_54x_cpld_exit();
	accton_as5712_54x_port_eeprom_mux_exit();
	accton_as5712_54x_i2c_exit();
	pr_info(DRIVER_NAME": version "DRIVER_VERSION" unloaded\n");
}

module_init(accton_as5712_54x_platform_init);
module_exit(accton_as5712_54x_platform_exit);

MODULE_AUTHOR("Curt Brune (curt@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton AS5712-54X Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
