/*
 * accton_as7412_32x_platform.c - Accton as7412-32x Platform Support.
 *
 * Copyright 2016 Cumulus Networks, Inc.
 * Author: Alan Liebthal (alanl@cumulusnetworks.com)
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
#include <linux/i2c/sff-8436.h>
#include <linux/i2c/pca954x.h>
#include <linux/i2c/pmbus.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include "platform_defs.h"
#include "accton_as7412_32x_cpld.h"

#define DRIVER_NAME	"accton_as7412_32x_platform"
#define DRIVER_VERSION	"0.1"

/*---------------------------------------------------------------------
 *
 * Platform driver
 *
 *-------------------------------------------------------------------*/
static struct i2c_client *act_as7412_cpld_clients[NUM_CPLD_I2C_CLIENTS];

/*
 * The platform has 2 types of i2c SMBUSes, i801 (Intel 82801
 * (ICH/PCH)) and ISMT (Intel SMBus Message Transport).  Each has a
 * PCA9548 mux.
 */

/* i2c bus adapter numbers for the i2c busses */
enum {
	CL_I2C_I801_BUS = 0,
	CL_I2C_ISMT_BUS,
	CL_I2C_MUX_76_BUS0 = 10,
	CL_I2C_MUX_71_BUS0 = 20,
};

mk_eeprom(spd1,  52, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(spd2,  53, 256, AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(board, 57, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu1,  53, 256, AT24_FLAG_IRUGO);
mk_eeprom(psu2,  50, 256, AT24_FLAG_IRUGO);

struct cl_platform_device_info {
	int bus;
	struct i2c_board_info board_info;
};

struct port_info {
	struct i2c_board_info *b;
	struct i2c_client *c;
};

#define cl_i2c_pca9548(addr, busno)					\
	enum {								\
		CL_I2C_MUX_##addr##_0 = busno,				\
		CL_I2C_MUX_##addr##_1,					\
		CL_I2C_MUX_##addr##_2,					\
		CL_I2C_MUX_##addr##_3,					\
		CL_I2C_MUX_##addr##_4,					\
		CL_I2C_MUX_##addr##_5,					\
		CL_I2C_MUX_##addr##_6,					\
		CL_I2C_MUX_##addr##_7,					\
	};								\
	static struct pca954x_platform_mode cl_i2c_mode_pca9548_##addr[] = { \
		{ .adap_id = CL_I2C_MUX_##addr##_0, .deselect_on_exit = 1,}, \
		{ .adap_id = CL_I2C_MUX_##addr##_1, .deselect_on_exit = 1,}, \
		{ .adap_id = CL_I2C_MUX_##addr##_2, .deselect_on_exit = 1,}, \
		{ .adap_id = CL_I2C_MUX_##addr##_3, .deselect_on_exit = 1,}, \
		{ .adap_id = CL_I2C_MUX_##addr##_4, .deselect_on_exit = 1,}, \
		{ .adap_id = CL_I2C_MUX_##addr##_5, .deselect_on_exit = 1,}, \
		{ .adap_id = CL_I2C_MUX_##addr##_6, .deselect_on_exit = 1,}, \
		{ .adap_id = CL_I2C_MUX_##addr##_7, .deselect_on_exit = 1,}, \
	};								\
	static struct pca954x_platform_data cl_i2c_data_pca9548_##addr = { \
		.modes = cl_i2c_mode_pca9548_##addr,			\
		.num_modes = ARRAY_SIZE(cl_i2c_mode_pca9548_##addr),	\
	}								\

cl_i2c_pca9548(76, CL_I2C_MUX_76_BUS0);
cl_i2c_pca9548(71, CL_I2C_MUX_71_BUS0);

static struct pmbus_platform_data psu1_data = {
	.flags = PMBUS_SKIP_STATUS_CHECK,
};

static struct pmbus_platform_data psu2_data = {
	.flags = PMBUS_SKIP_STATUS_CHECK,
};

static struct cl_platform_device_info i2c_devices[] = {
	/* I2C_i801 Bus */
	{
		.bus = CL_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("spd", 0x52),
			.platform_data = &spd1_52_at24,
		},
	},
	{
		.bus = CL_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("spd", 0x53),
			.platform_data = &spd2_53_at24,
		},
	},
	{
		.bus = CL_I2C_I801_BUS,
		{
			I2C_BOARD_INFO("pca9548", 0x76),
			.platform_data = &cl_i2c_data_pca9548_76,
		},
	},
	{          /* SMB1.MUX0 */
		.bus = CL_I2C_MUX_76_0,
		{	/* fan board CPLD */
			I2C_BOARD_INFO("dummy", 0x66),
		},
	},
	{          /* SMB1.MUX0 */
		.bus = CL_I2C_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x48),
		},
	},
	{
		.bus = CL_I2C_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x49),
		},
	},
	{
		.bus = CL_I2C_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x4a),
		},
	},
	{
		.bus = CL_I2C_MUX_76_1,
		{
			I2C_BOARD_INFO("lm75", 0x4b),
		},
	},
	{
		.bus = CL_I2C_MUX_76_2,
		{       /* main CPLD */
			I2C_BOARD_INFO("dummy", 0x60),
		},
	},
	/* I2C_iSMT Bus */
	{
		.bus = CL_I2C_ISMT_BUS,
		{
			I2C_BOARD_INFO("pca9548", 0x71),
			.platform_data = &cl_i2c_data_pca9548_71,
		},
	},
	{           /* SMB2.MUX0 */
		.bus = CL_I2C_MUX_71_0,
		{
			I2C_BOARD_INFO("24c02", 0x50),      /* PSU1 EEPROM */
			.platform_data = &psu2_50_at24,
		},
	},
	{
		.bus = CL_I2C_MUX_71_0,
		{
			I2C_BOARD_INFO("pmbus", 0x58),
			.platform_data = &psu2_data,
		},
	},
	{
		.bus = CL_I2C_MUX_71_1,
		{
			I2C_BOARD_INFO("24c02", 0x53),      /* PSU2 EEPROM */
			.platform_data = &psu1_53_at24,
		},
	},
	{
		.bus = CL_I2C_MUX_71_1,
		{
			I2C_BOARD_INFO("pmbus", 0x5b),
			.platform_data = &psu1_data,
		},
	},
	{
		.bus = CL_I2C_MUX_71_5,
		{
			I2C_BOARD_INFO("24c02", 0x57),      /* board EEPROM */
			.platform_data = &board_57_at24,
		},
	},

};

/**
 * Array of allocated i2c_client objects.  Need to track these in
 * order to free them later.
 *
 */
struct i2c_client_info {
	struct i2c_client *i2c_client;
	struct cl_platform_device_info *platform_info;
};

static struct i2c_client_info i2c_clients[ARRAY_SIZE(i2c_devices)];
static int num_i2c_clients;
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
static struct i2c_adapter *get_adapter(int bus)
{
	int bail = 10;
	struct i2c_adapter *adapter;

	for (; bail; bail--) {
		adapter = i2c_get_adapter(bus);
		if (adapter)
			return adapter;
		msleep(10.5);
	}
	return NULL;
}

static struct i2c_client *add_i2c_client(int bus,
					 struct i2c_board_info *board_info)
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

static void free_i2c_data(void)
{
	int i;
	/*
	 * Free the devices in reverse order so that child devices are
	 * freed before parent mux devices.
	 */
	for (i = num_i2c_clients - 1; i >= 0; i--)
		i2c_unregister_device(i2c_clients[i].i2c_client);
}

static int __init get_bus_by_name(char *name)
{
	struct i2c_adapter *adapter;
	int i;

	for (i = 0; i <= CL_I2C_ISMT_BUS; i++) {
		adapter = get_adapter(i);
		if (adapter &&
		    (strncmp(adapter->name, name, strlen(name)) == 0)) {
			i2c_put_adapter(adapter);
			return i;
		}
		i2c_put_adapter(adapter);
	}
	return -1;
}

static int ismt_bus_num;
static int i801_bus_num;

/**
 * act_as7412_i2c_init -- Initialize I2C devices
 *
 */
static int __init act_as7412_i2c_init(void)
{
	struct i2c_client *client;
	int i;
	int ret = -1;
	int cpld_count = 0;

	ismt_bus_num = get_bus_by_name(ISMT_ADAPTER_NAME);
	if (ismt_bus_num < 0) {
		pr_err("could not find ismt adapter bus\n");
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
		 * Map logical buses CL_I2C_ISMT_BUS and
		 * CL_I2C_I801_BUS to their dynamically discovered
		 * bus numbers.
		 */
		switch (i2c_devices[i].bus) {
		case CL_I2C_ISMT_BUS:
			i2c_devices[i].bus = ismt_bus_num;
			break;
		case CL_I2C_I801_BUS:
			i2c_devices[i].bus = i801_bus_num;
			break;
		default:
			break;
			/* Fall through for PCA9548 buses */
		};
		client = add_i2c_client(i2c_devices[i].bus,
					&i2c_devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		i2c_clients[num_i2c_clients].platform_info = &i2c_devices[i];
		i2c_clients[num_i2c_clients++].i2c_client = client;
		if (strcmp(i2c_devices[i].board_info.type, "dummy") == 0)
			act_as7412_cpld_clients[cpld_count++] = client;
	}
	return 0;

err_exit:
	free_i2c_data();
	return ret;
}

static void __exit act_as7412_i2c_exit(void)
{
	free_i2c_data();
}

/*---------------------------------------------------------------------
 *
 * CPLD driver
 *
 *-------------------------------------------------------------------*/

static struct platform_device *act_as7412_cpld_device;

#define CPLD_NAME  "act_as7412_32x_cpld"

static uint8_t cpld_reg_read(uint32_t reg)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	uint8_t val;
	struct i2c_client *client;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("attempt to read invalid CPLD register [0x%02X]", reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	client = act_as7412_cpld_clients[cpld_idx];
	val = i2c_smbus_read_byte_data(client, reg);
	if (val < 0)
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
		       client->addr, reg);
	return val;
}

static s32 cpld_reg_write(uint32_t reg, uint8_t write_val)
{
	int cpld_idx = GET_CPLD_IDX(reg);
	int res;
	struct i2c_client *client;

	if (cpld_idx < 0 || cpld_idx >= NUM_CPLD_I2C_CLIENTS) {
		pr_err("write to invalid CPLD register [0x%02X]", reg);
		return -EINVAL;
	}

	reg = STRIP_CPLD_IDX(reg);
	client = act_as7412_cpld_clients[cpld_idx];
	res = i2c_smbus_write_byte_data(client, reg, write_val);
	if (res)
		pr_err("CPLD wr - addr: 0x%02X, reg: 0x%02X, val: 0x%02X",
		       client->addr, reg, write_val);
	return res;
}

/*
 * cpld_version
 */
static ssize_t cpld_version_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	return sprintf(buf, "0x%02X\n", ACT7412_CPLD_VERSION_REG);
}
static SYSFS_ATTR_RO(cpld_version, cpld_version_show);

/*
 * psu_pwr1
 */
static ssize_t bulk_power_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	uint8_t read_val;
	uint8_t mask;
	uint8_t present_l;
	uint8_t pwr_ok;
	uint8_t error_l;

	read_val = cpld_reg_read(ACT7412_CPLD_PSU_STATUS_REG);
	if (strcmp(dattr->attr.name, xstr(PLATFORM_PS_NAME_0)) == 0) {
		mask      = ACT7412_PSU1_MASK;
		present_l = ACT7412_PSU1_PRESENT_L;
		pwr_ok    = ACT7412_PSU1_POWER_GOOD;
		error_l   = ACT7412_PSU1_ALERT_L;
	} else {
		mask      = ACT7412_PSU2_MASK;
		present_l = ACT7412_PSU2_PRESENT_L;
		pwr_ok    = ACT7412_PSU2_POWER_GOOD;
		error_l   = ACT7412_PSU2_ALERT_L;
	}
	read_val &= mask;

	if (~read_val & present_l) {
		sprintf(buf, PLATFORM_INSTALLED);
		if (!(read_val & pwr_ok) || !(read_val & error_l))
			strcat(buf, ", " PLATFORM_PS_POWER_BAD);
		else
			strcat(buf, ", " PLATFORM_OK);
	} else {
		sprintf(buf, PLATFORM_NOT_INSTALLED);
	}
	strcat(buf, "\n");

	return strlen(buf);
}
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_0, bulk_power_show);
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_1, bulk_power_show);

/*
 * fanX ok
 */
static uint8_t fan_present_bits[] = {
	ACT7412_FAN_TRAY1_PRESENT_L, ACT7412_FAN_TRAY2_PRESENT_L,
	ACT7412_FAN_TRAY3_PRESENT_L, ACT7412_FAN_TRAY4_PRESENT_L,
	ACT7412_FAN_TRAY5_PRESENT_L, ACT7412_FAN_TRAY6_PRESENT_L,
};

static ssize_t fan_ok_show(struct device *dev,
			   struct device_attribute *dattr,
			   char *buf)
{
	char num_str[3] = {0};
	int fan_num;
	uint8_t read_val;
	int ret;

	memcpy(num_str, &dattr->attr.name[3], 2);
	if (num_str[1] == '_')
		num_str[1] = '\0';
	ret = kstrtoint(num_str, 0, &fan_num);
	if (ret < 0)
		return ret;

	read_val = cpld_reg_read(ACT7412_CPLD_FAN_PRESENT_REG);
	return sprintf(buf, "%c\n",
		       (read_val & fan_present_bits[(--fan_num) / 2]
		       ? '0' : '1'));
}
static SYSFS_ATTR_RO(fan1_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan2_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan3_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan4_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan5_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan6_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan7_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan8_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan9_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan10_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan11_ok, fan_ok_show);
static SYSFS_ATTR_RO(fan12_ok, fan_ok_show);

/*
 * fan_watchdog_enable
 */
static ssize_t fan_wdog_show(struct device *dev,
			     struct device_attribute *dattr,
			     char *buf)
{
	return sprintf(buf, "%c\n",
		       ((cpld_reg_read(ACT7412_CPLD_FAN_WATCHDOG_REG) &
			 ACT7412_FAN_WATCHDOG_ENABLE) ? '1' : '0'));
}

static ssize_t fan_wdog_store(struct device *dev,
			      struct device_attribute *dattr,
			      const char *buf, size_t count)
{
	int val;
	int ret;
	uint8_t read_val;

	ret = kstrtoint(buf, 0, &val);
	if (ret < 0)
		return ret;

	read_val = cpld_reg_read(ACT7412_CPLD_FAN_WATCHDOG_REG);
	if (val)
		read_val |= ACT7412_FAN_WATCHDOG_ENABLE;
	else
		read_val &= ~ACT7412_FAN_WATCHDOG_ENABLE;
	cpld_reg_write(ACT7412_CPLD_FAN_WATCHDOG_REG, read_val);
	return count;
}
static SYSFS_ATTR_RW(fan_watchdog_enable, fan_wdog_show, fan_wdog_store);

/*
 * fanX speed
 */
static u8 fan_speed_regs[] = {
	ACT7412_CPLD_FAN1_SPEED_REG,  ACT7412_CPLD_FAN2_SPEED_REG,
	ACT7412_CPLD_FAN3_SPEED_REG,  ACT7412_CPLD_FAN4_SPEED_REG,
	ACT7412_CPLD_FAN5_SPEED_REG,  ACT7412_CPLD_FAN6_SPEED_REG,
	ACT7412_CPLD_FAN7_SPEED_REG,  ACT7412_CPLD_FAN8_SPEED_REG,
	ACT7412_CPLD_FAN9_SPEED_REG,  ACT7412_CPLD_FAN10_SPEED_REG,
	ACT7412_CPLD_FAN11_SPEED_REG, ACT7412_CPLD_FAN12_SPEED_REG,
};

static ssize_t fan_input_show(struct device *dev,
			      struct device_attribute *dattr,
			      char *buf)
{
	char num_str[3] = {0};
	int fan_num;
	int ret;

	memcpy(num_str, &dattr->attr.name[3], 2);
	if (num_str[1] == '_')
		num_str[1] = '\0';
	ret = kstrtoint(num_str, 0, &fan_num);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%u\n", ACT7412_FAN_SPEED_MULTIPLIER *
		       cpld_reg_read(fan_speed_regs[fan_num - 1]));
}
static SYSFS_ATTR_RO(fan1_input, fan_input_show);
static SYSFS_ATTR_RO(fan2_input, fan_input_show);
static SYSFS_ATTR_RO(fan3_input, fan_input_show);
static SYSFS_ATTR_RO(fan4_input, fan_input_show);
static SYSFS_ATTR_RO(fan5_input, fan_input_show);
static SYSFS_ATTR_RO(fan6_input, fan_input_show);
static SYSFS_ATTR_RO(fan7_input, fan_input_show);
static SYSFS_ATTR_RO(fan8_input, fan_input_show);
static SYSFS_ATTR_RO(fan9_input, fan_input_show);
static SYSFS_ATTR_RO(fan10_input, fan_input_show);
static SYSFS_ATTR_RO(fan11_input, fan_input_show);
static SYSFS_ATTR_RO(fan12_input, fan_input_show);

/*
 * fan pwm
 */
static ssize_t pwm_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	return sprintf(buf, "%u\n", cpld_reg_read(ACT7412_CPLD_PWM_REG) *
		       ACT7412_FAN_PWM_MULTIPLIER);
}

static ssize_t pwm_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	int val;
	int ret;

	/* there is only one fan but we make it look like
	 * there's 6 (one for each tray)
	 */
	ret = kstrtoint(buf, 0, &val);
	if (ret < 0)
		return ret;
	val /= ACT7412_FAN_PWM_MULTIPLIER;
	if (val < 0 || val > 255)
		return -EINVAL;
	cpld_reg_write(ACT7412_CPLD_PWM_REG, (u8)val);
	return count;
}
SYSFS_ATTR_RW(pwm1, pwm_show, pwm_store);
SYSFS_ATTR_RW(pwm2, pwm_show, pwm_store);
SYSFS_ATTR_RW(pwm3, pwm_show, pwm_store);
SYSFS_ATTR_RW(pwm4, pwm_show, pwm_store);
SYSFS_ATTR_RW(pwm5, pwm_show, pwm_store);
SYSFS_ATTR_RW(pwm6, pwm_show, pwm_store);

static struct attribute *act_as7412_cpld_attrs[] = {
	&dev_attr_cpld_version.attr,
	&dev_attr_psu_pwr1.attr,
	&dev_attr_psu_pwr2.attr,
	&dev_attr_pwm1.attr,
	&dev_attr_pwm2.attr,
	&dev_attr_pwm3.attr,
	&dev_attr_pwm4.attr,
	&dev_attr_pwm5.attr,
	&dev_attr_pwm6.attr,
	&dev_attr_fan1_ok.attr,
	&dev_attr_fan2_ok.attr,
	&dev_attr_fan3_ok.attr,
	&dev_attr_fan4_ok.attr,
	&dev_attr_fan5_ok.attr,
	&dev_attr_fan6_ok.attr,
	&dev_attr_fan7_ok.attr,
	&dev_attr_fan8_ok.attr,
	&dev_attr_fan9_ok.attr,
	&dev_attr_fan10_ok.attr,
	&dev_attr_fan11_ok.attr,
	&dev_attr_fan12_ok.attr,
	&dev_attr_fan1_input.attr,
	&dev_attr_fan2_input.attr,
	&dev_attr_fan3_input.attr,
	&dev_attr_fan4_input.attr,
	&dev_attr_fan5_input.attr,
	&dev_attr_fan6_input.attr,
	&dev_attr_fan7_input.attr,
	&dev_attr_fan8_input.attr,
	&dev_attr_fan9_input.attr,
	&dev_attr_fan10_input.attr,
	&dev_attr_fan11_input.attr,
	&dev_attr_fan12_input.attr,
	&dev_attr_fan_watchdog_enable.attr,
	NULL,
};

static struct attribute_group act_as7412_cpld_attr_group = {
	.attrs = act_as7412_cpld_attrs,
};

static int act_as7412_cpld_probe(struct platform_device *dev)
{
	int ret;

	ret = sysfs_create_group(&dev->dev.kobj, &act_as7412_cpld_attr_group);
	if (ret)
		pr_err("sysfs_cpld_driver_group failed for cpld driver");

	return ret;
}

static int act_as7412_cpld_remove(struct platform_device *dev)
{
	return 0;
}

static struct platform_driver act_as7412_cpld_driver = {
	.driver = {
		.name = CPLD_NAME,
		.owner = THIS_MODULE,
	},
	.probe = act_as7412_cpld_probe,
	.remove = act_as7412_cpld_remove,
};

static int __init act_as7412_cpld_init(void)
{
	int ret;

	ret = platform_driver_register(&act_as7412_cpld_driver);
	if (ret) {
		pr_err("platform_driver_register() failed for CPLD device");
		goto err_drvr;
	}

	act_as7412_cpld_device = platform_device_alloc(CPLD_NAME, 0);
	if (!act_as7412_cpld_device) {
		pr_err("platform_device_alloc() failed for CPLD device");
		ret = -ENOMEM;
		goto err_dev_alloc;
	}

	ret = platform_device_add(act_as7412_cpld_device);
	if (ret) {
		pr_err("platform_device_add() failed for CPLD device.\n");
		goto err_dev_add;
	}
	return 0;

err_dev_add:
	platform_device_put(act_as7412_cpld_device);

err_dev_alloc:
	platform_driver_unregister(&act_as7412_cpld_driver);

err_drvr:
	return ret;
}

static void __exit act_as7412_cpld_exit(void)
{
}

static int __init
act_as7412_platform_init(void)
{
	int ret = 0;

	ret = act_as7412_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		return ret;
	}

	ret = act_as7412_cpld_init();
	if (ret) {
		pr_err("Initializing CPLD subsystem failed\n");
		return ret;
	}

	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " loaded\n");
	return 0;
}

static void __exit
act_as7412_platform_exit(void)
{
	act_as7412_cpld_exit();
	act_as7412_i2c_exit();
	pr_info(DRIVER_NAME ": version " DRIVER_VERSION " unloaded\n");
}

module_init(act_as7412_platform_init);
module_exit(act_as7412_platform_exit);

MODULE_AUTHOR("Alan Liebthal (alanl@cumulusnetworks.com)");
MODULE_DESCRIPTION("Accton AS7412-32X Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
