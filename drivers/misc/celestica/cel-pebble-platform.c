/*
 * cel_pebble_platform.c - Celestica Pebble E1050 Platform Support.
 *
 * Author: Vidya Ravipati (vidya@cumulusnetworks.com)
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
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/sff-8436.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>

#include "platform-defs.h"
#include "cel-pebble-platform.h"

#define DRIVER_NAME        "cel_pebble_platform"
#define DRIVER_VERSION     "0.1"

#define NUM_CPLD_I2C_CLIENTS (0)

/* Board Type */
#  define CPU_BOARD_TYPE_P0_VAL      (3)
#  define CPU_BOARD_TYPE_P1_VAL      (2)


static struct i2c_client *cel_pebble_cpld_i2c_client_list[NUM_CPLD_I2C_CLIENTS];
static int num_cpld_i2c_devices;

mk_eeprom(spd1,  52, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_port_eeprom(port49,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port50,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port51,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_port_eeprom(port52,  50, 512,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);

struct eeprom_platform_data board_55_eeprom = {
	.label = "board_eeprom",
};

/* 256 byte Board eeprom on P0 board */
struct at24_platform_data p0_board_55_at24 = {
	.byte_len = 256,
	.flags = AT24_FLAG_IRUGO,
	.page_size = 1,
	.eeprom_data = &board_55_eeprom,
};

/* 64k Board eeprom on P1 board */
struct at24_platform_data p1_board_55_at24 = {
	.byte_len = 8192,
	.flags = AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO,
	.page_size = 1,
	.eeprom_data = &board_55_eeprom,
};

struct cel_pebble_gpio_pin {
	int num;
	char *name;
	int is_output;
	int is_active_low;
};

static struct cel_pebble_gpio_pin cel_pebble_gpio_pins[] = {
	{
		.num = CEL_PEBBLE_GPIO_SFP_1_RS,
		.name = "sfp1_rs",
		.is_output = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_1_RX_LOS,
		.name = "sfp1_rx_los",
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_1_PRESENT_L,
		.name = "sfp1_present",
		.is_active_low = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_1_TX_DISABLE,
		.name = "sfp1_tx_enable",
		.is_active_low = true,
		.is_output = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_1_TX_FAULT,
		.name = "sfp1_tx_fault",
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_2_RS,
		.name = "sfp2_rs",
		.is_output = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_2_RX_LOS,
		.name = "sfp2_rx_los",
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_2_PRESENT_L,
		.name = "sfp2_present",
		.is_active_low = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_2_TX_DISABLE,
		.name = "sfp2_tx_enable",
		.is_active_low = true,
		.is_output = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_2_TX_FAULT,
		.name = "sfp2_tx_fault",
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_3_RS,
		.name = "sfp3_rs",
		.is_output = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_3_RX_LOS,
		.name = "sfp3_rx_los",
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_3_PRESENT_L,
		.name = "sfp3_present",
		.is_active_low = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_3_TX_DISABLE,
		.name = "sfp3_tx_enable",
		.is_active_low = true,
		.is_output = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_3_TX_FAULT,
		.name = "sfp3_tx_fault",
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_4_RS,
		.name = "sfp4_rs",
		.is_output = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_4_RX_LOS,
		.name = "sfp4_rx_los",
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_4_PRESENT_L,
		.name = "sfp4_present",
		.is_active_low = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_4_TX_DISABLE,
		.name = "sfp4_tx_enable",
		.is_active_low = true,
		.is_output = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_SFP_4_TX_FAULT,
		.name = "sfp4_tx_fault",
	},
#if 0
    {
		.num = CEL_PEBBLE_GPIO_RTC_INT_L,
		.name = "rtc_int",
		.is_active_low = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_LM75B1_OS,
		.name = "lm75b1_os",
		.is_active_low = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_LM75B2_OS,
		.name = "lm75b2_os",
		.is_active_low = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_B50282_1_INT_L,
		.name = "B50282_1_INT",
		.is_active_low = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_B50282_2_INT_L,
		.name = "B50282_2_INT",
		.is_active_low = true,
	},
	{
		.num = CEL_PEBBLE_GPIO_PCIE_INT_L,
		.name = "pcie_int",
		.is_active_low = true,
	},
#endif
};

static const char *pca953x_gpio_names[CEL_PEBBLE_GPIO_1_PIN_COUNT];

static struct pca953x_platform_data gpio_platform_data = {
	.gpio_base = CEL_PEBBLE_GPIO_1_BASE,
	.names = pca953x_gpio_names,
};

static int cel_pebble_claimed_gpios[ARRAY_SIZE(cel_pebble_gpio_pins)];

/*
 * the iSMT bus has a single PCA9547 switch that connects the devices
 */
static struct pca954x_platform_mode pca9548_mux_platform_modes[] = {
	{
		.adap_id = CEL_PEBBLE_I2C_PCA9548_BUS_0, .deselect_on_exit = 1,
	},
	{
		.adap_id = CEL_PEBBLE_I2C_PCA9548_BUS_1, .deselect_on_exit = 1,
	},
	{
		.adap_id = CEL_PEBBLE_I2C_PCA9548_BUS_2, .deselect_on_exit = 1,
	},
	{
		.adap_id = CEL_PEBBLE_I2C_PCA9548_BUS_3, .deselect_on_exit = 1,
	},
	{
		.adap_id = CEL_PEBBLE_I2C_PCA9548_BUS_4, .deselect_on_exit = 1,
	},
	{
		.adap_id = CEL_PEBBLE_I2C_PCA9548_BUS_5, .deselect_on_exit = 1,
	},
	{
		.adap_id = CEL_PEBBLE_I2C_PCA9548_BUS_6, .deselect_on_exit = 1,
	},
	{
		.adap_id = CEL_PEBBLE_I2C_PCA9548_BUS_7, .deselect_on_exit = 1,
	},
};

static struct pca954x_platform_data pca9548_mux_platform_data = {
	.modes = pca9548_mux_platform_modes,
	.num_modes = ARRAY_SIZE(pca9548_mux_platform_modes),
};

struct cel_pebble_i2c_device_info {
	int bus;
	struct i2c_board_info board_info;
};

/*
 * the list of i2c devices and their bus connections for this platform
 */
static struct cel_pebble_i2c_device_info cel_pebble_common_i2c_devices[] = {
	/* Begin I2C_I801_BUS */
	{
		.bus = CEL_PEBBLE_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("spd", 0x52),           /* DIMM */
			.platform_data = &spd1_52_at24,
		}
	},
	{
		.bus = CEL_PEBBLE_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("lm75", 0x4A),         /* LM75 Temperature Sensor */
		}
	},
	{
		.bus = CEL_PEBBLE_I2C_I801_BUS,
		.board_info = {
			I2C_BOARD_INFO("pca9548", 0x71),      /* PCA9548 8 Port I2C Switch */
			.platform_data = &pca9548_mux_platform_data,
		}
	},
	/* Begin PCA9548 BUS 1 */
	{
		.bus = CEL_PEBBLE_I2C_PCA9548_BUS_1,
		.board_info = {
		.type = "pca9505",                      /* PCA9506 GPIO Expander */
		.addr = 0x20,
		.platform_data = &gpio_platform_data,
		}
	},
	/* Begin PCA9548 BUS 2 */
	{
		.bus = CEL_PEBBLE_I2C_PCA9548_BUS_2,
		.board_info = {
			I2C_BOARD_INFO("24c04", 0x50),        /* SFP Port 49 EEPROM */
			.platform_data = &port49_50_at24,
		}
	},
	/* Begin PCA9548 BUS 3 */
	{
		.bus = CEL_PEBBLE_I2C_PCA9548_BUS_3,
		.board_info = {
			I2C_BOARD_INFO("24c04", 0x50),         /* SFP Port 50 EEPROM */
			.platform_data = &port50_50_at24,
		}
	},
	/* Begin PCA9548 BUS 4 */
	{
		.bus = CEL_PEBBLE_I2C_PCA9548_BUS_4,
		.board_info = {
			I2C_BOARD_INFO("24c04", 0x50),         /* SFP Port 51 EEPROM */
			.platform_data = &port51_50_at24,
		}
	},
	/* Begin PCA9548 BUS 5 */
	{
		.bus = CEL_PEBBLE_I2C_PCA9548_BUS_5,
		.board_info = {
			I2C_BOARD_INFO("24c04", 0x50),         /* SFP Port 52 EEPROM */
			.platform_data = &port52_50_at24,
		}
	},
	/* Begin PCA9548 BUS 6 */
	{
		.bus = CEL_PEBBLE_I2C_PCA9548_BUS_6,
		.board_info = {
			I2C_BOARD_INFO("lm75", 0x48),         /* LM75 Temperature Sensor */
		}
	},
	/* Begin PCA9548 BUS 6 */
	{
		.bus = CEL_PEBBLE_I2C_PCA9548_BUS_6,
		.board_info = {
			I2C_BOARD_INFO("lm75", 0x49),         /* LM75 Temperature Sensor */
		}
	},
};

static struct cel_pebble_i2c_device_info  cel_pebble_p0_i2c_devices[] = {
	{
		.bus = CEL_PEBBLE_I2C_I801_BUS,
		.board_info = {
			.type = "24c04",                      /* 256 byte Board EEPROM */
			.addr = 0x55,
			.platform_data = &p0_board_55_at24,
		}
	},
};

static struct cel_pebble_i2c_device_info  cel_pebble_p1_i2c_devices[] = {
	{
		.bus = CEL_PEBBLE_I2C_I801_BUS,
		.board_info = {
			.type = "24c64",                      /* 64 Kbyte Board EEPROM */
			.addr = 0x55,
			.platform_data = &p1_board_55_at24,
		}
	},
};

/**
 * cel_pebble_i2c_init -- Initialize the I2C subsystem.
 *
 *
 */
static struct i2c_client *cel_pebble_common_clients_list[ARRAY_SIZE(cel_pebble_common_i2c_devices)];
static struct i2c_client *cel_pebble_p0_clients_list[ARRAY_SIZE(cel_pebble_p0_i2c_devices)];
static struct i2c_client *cel_pebble_p1_clients_list[ARRAY_SIZE(cel_pebble_p1_i2c_devices)];

/* GPIO pin logic to determine P0 vs P1 boards */
void gpio_sus_init(unsigned int gpio)
{
	outl_p((inl_p(0x520) | (1 << gpio)), 0x520);//100 0000 0001 0011 0000
}
void gpio_sus_set_dir(unsigned int gpio, unsigned int out_flag)
{
	if (out_flag)
		outl_p((inl_p(0x524) & (~(1 << gpio))), 0x524);//output set 0
	else
		outl_p((inl_p(0x524) | (1 << gpio)), 0x524);//input set 1
}
unsigned char gpio_sus_get_value(unsigned int gpio)
{
	unsigned char ucRet = 0;

	if ( inl_p( 0x528 ) & (1 << gpio) ) {
		ucRet = 0x01;
	}
	else {
		ucRet = 0x00;
	}

	return ( ucRet );

}
unsigned char getCpuId(void)
{
	unsigned char cpu_id = 0;

	gpio_sus_init(26);
	gpio_sus_init(27);
	gpio_sus_set_dir(26, 0);
	gpio_sus_set_dir(27, 0);

	/*read cpu id*/
	cpu_id = gpio_sus_get_value(26);
	cpu_id |= gpio_sus_get_value(27)<<1;

	pr_info("CPU ID 0x%x\n",cpu_id);
	return cpu_id;
}

static void free_gpio_pins(int *pins, int num_pins)
{
	int i;
    
	for (i = 0; i < num_pins; i++) {
		if (pins[i]) {
			gpio_free(pins[i]);
		}
	}
}

static void free_i2c_clients(struct i2c_client **clients_list, int num_clients)
{
	int i, idx;

	for (i = num_clients; i; i--) {
		idx = i - 1;
		if (clients_list[idx])
			i2c_unregister_device(clients_list[idx]);
	}
}

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

static void free_cel_pebble_i2c_data(void)
{
	free_i2c_clients(cel_pebble_p0_clients_list, ARRAY_SIZE(cel_pebble_p0_i2c_devices));
	free_i2c_clients(cel_pebble_p1_clients_list, ARRAY_SIZE(cel_pebble_p1_i2c_devices));
	free_i2c_clients(cel_pebble_common_clients_list, ARRAY_SIZE(cel_pebble_common_i2c_devices));
}

static struct i2c_client *add_i2c_client(int bus, struct i2c_board_info *board_info)
{
	struct i2c_adapter *adapter;
	struct i2c_client *client;

	adapter = get_adapter(bus);
	if (!adapter) {
		pr_err("could not get adapter %u\n", bus);
		client = ERR_PTR(-ENODEV);
		goto exit;
	}
	client = i2c_new_device(adapter, board_info);
	if (!client) {
		pr_err("could not add device\n");
		client = ERR_PTR(-ENODEV);
	}
	i2c_put_adapter(adapter);
exit:
	return client;
}

static int get_bus_by_name(char *name)
{
	struct i2c_adapter *adapter;
	int i;

	for (i = 0; i < CEL_PEBBLE_I2C_PCA9548_BUS_0; i++) {
		adapter = get_adapter(i);
		if (adapter) {
			if (strncmp(adapter->name, name, strlen(name)) == 0) {
				i2c_put_adapter(adapter);
				return i;
			}
			i2c_put_adapter(adapter);
		}
	}
	return -1;
}

static int init_gpio_pins(struct cel_pebble_gpio_pin *gpio_pin_ptr, int num_pins, int *claimed_list)
{
	int i;
	int ret;

	for (i = 0; i < num_pins; i++, gpio_pin_ptr++) {
		unsigned long flags = GPIOF_EXPORT;

		flags |= gpio_pin_ptr->is_output ? GPIOF_DIR_OUT : GPIOF_DIR_IN;
		flags |= gpio_pin_ptr->is_active_low ? GPIOF_ACTIVE_LOW : 0;

		ret = gpio_request_one(gpio_pin_ptr->num, flags, NULL);
		if (ret) {
			pr_err("request for gpio pin %s (%u) failed\n",
			       gpio_pin_ptr->name, gpio_pin_ptr->num);
			return ret;
		}
		claimed_list[i] = gpio_pin_ptr->num;
	}
	return 0;
}

static int populate_i2c_devices(struct cel_pebble_i2c_device_info *devices,
                                int num_devices, struct i2c_client **clients_list,
                                int I801_bus)
{
	int i;
	int ret;
	struct i2c_client *client;

	num_cpld_i2c_devices = 0;
	for (i = 0; i < num_devices; i++) {
		if (devices[i].bus == CEL_PEBBLE_I2C_I801_BUS) {
			devices[i].bus = I801_bus;
		}
		client = add_i2c_client(devices[i].bus, &devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		clients_list[i] = client;
		if (strcmp(devices[i].board_info.type, "dummy") == 0 && num_cpld_i2c_devices < NUM_CPLD_I2C_CLIENTS) {
			cel_pebble_cpld_i2c_client_list[num_cpld_i2c_devices] = client;
			num_cpld_i2c_devices++;
		}
	}
	return 0;

err_exit:
	return ret;
}

static int __init cel_pebble_i2c_init(void)
{
	int I801_bus;
	int ret, i;
	unsigned char platform = CPU_BOARD_TYPE_P1_VAL;
	ret = -1;

	I801_bus = get_bus_by_name(I801_ADAPTER_NAME);
	if (I801_bus < 0) {
		 pr_err("could not find I801 adapter bus\n");
		 ret = -ENODEV;
		 goto err_exit;
	}

	platform = getCpuId();

	/* 
	 * populate the platform_data structure for the gpio with
	 * our pin names
	 */
	for (i = 0; i < ARRAY_SIZE(cel_pebble_gpio_pins); i++) {
		pca953x_gpio_names[cel_pebble_gpio_pins[i].num -
						CEL_PEBBLE_GPIO_1_BASE] =
							cel_pebble_gpio_pins[i].name;
	}

	/*
	 * populate the i2c devices
	 */
	ret = populate_i2c_devices(cel_pebble_common_i2c_devices, ARRAY_SIZE(cel_pebble_common_i2c_devices),
	                           &cel_pebble_common_clients_list[0], I801_bus);
	if (ret)
		goto err_exit;

    if (CPU_BOARD_TYPE_P0_VAL == platform) {
		ret = populate_i2c_devices(cel_pebble_p0_i2c_devices, ARRAY_SIZE(cel_pebble_p0_i2c_devices),
	                           &cel_pebble_p0_clients_list[0], I801_bus);
	} else if (CPU_BOARD_TYPE_P1_VAL == platform) {
		ret = populate_i2c_devices(cel_pebble_p1_i2c_devices, ARRAY_SIZE(cel_pebble_p1_i2c_devices),
	                           &cel_pebble_p1_clients_list[0], I801_bus);
	}
	if (ret)
		goto err_exit;

	ret = init_gpio_pins(cel_pebble_gpio_pins,
					ARRAY_SIZE(cel_pebble_gpio_pins),
					cel_pebble_claimed_gpios);
	if (ret)
		goto err_exit;

	return 0;

err_exit:
	free_cel_pebble_i2c_data();
	return ret;
}

static void __exit cel_pebble_i2c_exit(void)
{
	free_gpio_pins(cel_pebble_claimed_gpios, ARRAY_SIZE(cel_pebble_gpio_pins));
	free_cel_pebble_i2c_data();
}


/*---------------------------------------------------------------------
 *
 * Module init/exit
 */
static int __init cel_pebble_init(void)
{
	int ret = 0;

	ret = cel_pebble_i2c_init();
	if (ret) {
		pr_err("Initializing I2C subsystem failed\n");
		return ret;
	}

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;
}

static void __exit cel_pebble_exit(void)
{
	cel_pebble_i2c_exit();
	pr_err(DRIVER_NAME" driver unloaded\n");
}

module_init(cel_pebble_init);
module_exit(cel_pebble_exit);

MODULE_AUTHOR("Vidya Ravipati (vidya@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica Pebble E1050 Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
