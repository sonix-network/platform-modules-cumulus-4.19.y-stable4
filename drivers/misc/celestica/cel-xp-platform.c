/*
 * cel_xp_platform.c - Celestica XP Platform Support
 * 
 * Copyright (c) 2015, 2020 Cumulus Networks, Inc.  All rights reserved.
 * Original author: Alan Liebthal <alanl@cumulusnetworks.com>
 * Last modified by: David Yen <dhyen@cumulusnetworks.com>
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

 /*
  * A common platform module for Celestica boxes using the Rangeley CPU module
  */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_device.h>

#include "platform-defs.h"
#include "cel-xp-platform.h"

#define DRIVER_NAME	"cel_xp_platform"
#define DRIVER_VERSION	"1.1"

static struct platform_driver cel_xp_platform_driver;

/*
 * EEPROMs, except for the ones in the QSFP+ transceivers.  Specify the label,
 * i2c address, size, and some flags, using the mk*_eeprom() macro.  The label
 * is the string that ends up in /sys/class/eeprom_dev/eepromN/label.
 */

mk_eeprom(spd1,  50, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(spd2,  51, 256,  AT24_FLAG_READONLY | AT24_FLAG_IRUGO);
mk_eeprom(psu1,  50, 256,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(psu2,  51, 256,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(board, 50, 8192, AT24_FLAG_ADDR16 | AT24_FLAG_IRUGO);
mk_eeprom(fan1,  50, 256,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan2,  50, 256,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan3,  50, 256,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan4,  50, 256,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);
mk_eeprom(fan5,  50, 256,  AT24_FLAG_IRUGO | AT24_FLAG_HOTPLUG);


/*
 * GPIOs.  The pin pin names are specified in platform_data, but everthing
 * else has to be done by calls after pin creation.  Consolidate all the
 * information in one place (struct gpio_pin).  The pin name array is filled
 * in at runtime by cel_xp_platform_init(), before creating the i2c device.
 * Then the pins are created by init_gpio_pins().
 */

struct gpio_pin {
	int num;
	char *name;
	int is_output;
	int is_active_low;
	bool value;
};

#define mk_gpio_pins(_name) \
        static struct gpio_pin _name##_pins[]
#define mk_gpio_pin(_num, _name, _output, _active_low, _value) \
        { \
                .num = (_num), \
	        .name = #_name,    \
                .is_output = (_output), \
                .is_active_low = (_active_low), \
                .value = (_value), \
        }

mk_gpio_pins(cel_rxp_gpio) = {
        mk_gpio_pin( CEL_XP_GPIO_FAN2_PRESENT,   fan1_present,     0, 1, 0),
	mk_gpio_pin( CEL_XP_GPIO_FAN1_PRESENT,   fan2_present,     0, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_FAN5_PRESENT,   fan3_present,     0, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_FAN3_PRESENT,   fan4_present,     0, 1, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR1_ALERT,     psu_pwr1_alert,   0, 1, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR1_ALL_OK,    psu_pwr1_all_ok,  0, 0, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR1_PRESENT,   psu_pwr1_present, 0, 1, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR2_ALERT,     psu_pwr2_alert,   0, 1, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR2_ALL_OK,    psu_pwr2_all_ok,  0, 0, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR2_PRESENT,   psu_pwr2_present, 0, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN2_RED,   led_fan1_red,     1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN2_GREEN, led_fan1_green,   1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN1_RED,   led_fan2_red,     1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN1_GREEN, led_fan2_green,   1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN5_RED,   led_fan3_red,     1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN5_GREEN, led_fan3_green,   1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN3_RED,   led_fan4_red,     1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN3_GREEN, led_fan4_green,   1, 1, 0),
};

mk_gpio_pins(cel_sxp_gpio) = {
        mk_gpio_pin( CEL_XP_GPIO_FAN2_PRESENT,   fan1_present,     0, 1, 0),
	mk_gpio_pin( CEL_XP_GPIO_FAN1_PRESENT,   fan2_present,     0, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_FAN4_PRESENT,   fan3_present,     0, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_FAN5_PRESENT,   fan4_present,     0, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_FAN3_PRESENT,   fan5_present,     0, 1, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR1_ALERT,     psu_pwr1_alert,   0, 1, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR1_ALL_OK,    psu_pwr1_all_ok,  0, 0, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR1_PRESENT,   psu_pwr1_present, 0, 1, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR2_ALERT,     psu_pwr2_alert,   0, 1, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR2_ALL_OK,    psu_pwr2_all_ok,  0, 0, 0),
	mk_gpio_pin( CEL_XP_GPIO_PWR2_PRESENT,   psu_pwr2_present, 0, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN2_RED,   led_fan1_red,     1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN2_GREEN, led_fan1_green,   1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN1_RED,   led_fan2_red,     1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN1_GREEN, led_fan2_green,   1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN4_RED,   led_fan3_red,     1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN4_GREEN, led_fan3_green,   1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN5_RED,   led_fan4_red,     1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN5_GREEN, led_fan4_green,   1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN3_RED,   led_fan5_red,     1, 1, 0),
        mk_gpio_pin( CEL_XP_GPIO_LED_FAN3_GREEN, led_fan5_green,   1, 1, 0),
};

static const char *pca953x_gpio_names[CEL_XP_GPIO_1_PIN_COUNT];

static struct pca953x_platform_data gpio_platform_data = {
        .gpio_base = CEL_XP_GPIO_1_BASE,
        .names = pca953x_gpio_names,
};

static int cel_rxp_claimed_gpios[ARRAY_SIZE(cel_rxp_gpio_pins)];
static int cel_sxp_claimed_gpios[ARRAY_SIZE(cel_sxp_gpio_pins)];


/*
 * MUXes.  Specify the starting bus number for the block of ports, using the
 * magic mk_pca954*() macros.
 */

static struct pca954x_platform_mode master_switch_platform_modes[] = {
	{
		.adap_id = RXP_I2C_MASTER_SWITCH_BUS, .deselect_on_exit = 0,
	},
};

static struct pca954x_platform_data master_switch_platform_data = {
	.modes = master_switch_platform_modes,
	.num_modes = 1,
};


mk_pca9548(mux1, RXP_I2C_MUX1_BUS0, 1);
mk_pca9548(mux2, RXP_I2C_MUX2_BUS0, 1);
mk_pca9548(mux3, RXP_I2C_MUX3_BUS0, 1);


/*
 * struct xp_i2c_device_info -- i2c device container struct.  This
 * struct helps organize the i2c_board_info structures and the created
 * i2c_client objects.
 *
 */

struct xp_i2c_device_info {
	int bus;
	struct i2c_board_info board_info;
};

/*
 * I2C Device Table.  Use the mk_i2cdev() macro to construct the entries.
 * Each entry is a bus number and a i2c_board_info.  The i2c_board_info
 * structure specifies the device type, address, and platform data specific
 * to the device type.
 */

static struct xp_i2c_device_info i2c_redxp_devices[] = {
        mk_i2cdev(RXP_I2C_I801_BUS,  "spd",        0x50, &spd1_50_at24),
        mk_i2cdev(RXP_I2C_I801_BUS,  "spd",        0x51, &spd2_51_at24),
        mk_i2cdev(RXP_I2C_iSMT_BUS,  "pca9541",    0x70,
                                                 &master_switch_platform_data),

        mk_i2cdev(RXP_I2C_MASTER_SWITCH_BUS,
                                     "pca9548",    0x73, &mux1_platform_data),
        mk_i2cdev(RXP_I2C_MUX1_BUS0, "dps460", 0x58, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS0, "24c02",      0x50, &psu1_50_at24),
        mk_i2cdev(RXP_I2C_MUX1_BUS1, "dps460", 0x59, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS1, "24c02",      0x51, &psu2_51_at24),
        mk_i2cdev(RXP_I2C_MUX1_BUS2, "24c64",      0x50, &board_50_at24),
        mk_i2cdev(RXP_I2C_MUX1_BUS3, "emc2305",    0x4d, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS3, "emc2305",    0x2e, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS4, "lm75",       0x48, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS5, "lm75",       0x4e, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS7, "pca9505",    0x20, &gpio_platform_data),

        mk_i2cdev(RXP_I2C_MASTER_SWITCH_BUS,
                                     "pca9548",    0x71, &mux2_platform_data),
        mk_i2cdev(RXP_I2C_MUX2_BUS4, "lm75",       0x49, NULL),
        mk_i2cdev(RXP_I2C_MUX2_BUS5, "lm75",       0x4a, NULL),
};

static struct xp_i2c_device_info i2c_smallxp_devices[] = {
        mk_i2cdev(RXP_I2C_I801_BUS,  "spd",        0x50, &spd1_50_at24),
        mk_i2cdev(RXP_I2C_I801_BUS,  "spd",        0x51, &spd2_51_at24),
        mk_i2cdev(RXP_I2C_iSMT_BUS,  "pca9541",    0x70,
                                                 &master_switch_platform_data),
        mk_i2cdev(RXP_I2C_MASTER_SWITCH_BUS,
                                     "pca9548",    0x73, &mux1_platform_data),
        mk_i2cdev(RXP_I2C_MUX1_BUS0, "dps460", 0x58, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS0, "24c02",      0x50, &psu1_50_at24),
        mk_i2cdev(RXP_I2C_MUX1_BUS1, "dps460", 0x59, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS1, "24c02",      0x51, &psu2_51_at24),
        mk_i2cdev(RXP_I2C_MUX1_BUS2, "24c64",      0x50, &board_50_at24),
        mk_i2cdev(RXP_I2C_MUX1_BUS3, "emc2305",    0x4d, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS3, "emc2305",    0x2e, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS4, "lm75",       0x48, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS5, "lm75",       0x4e, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS7, "pca9505",    0x20, &gpio_platform_data),

        mk_i2cdev(RXP_I2C_MASTER_SWITCH_BUS,
                                     "pca9548",    0x71, &mux2_platform_data),
        mk_i2cdev(RXP_I2C_MUX2_BUS3, "lm75",       0x48, NULL),
        mk_i2cdev(RXP_I2C_MUX2_BUS4, "lm75",       0x49, NULL),
};

static struct xp_i2c_device_info i2c_sea_devices[] = {
        mk_i2cdev(RXP_I2C_I801_BUS,  "spd",        0x50, &spd1_50_at24),

	/*
	 * The first DIMM is required.  The second DIMM is not populated at
	 * this time.
	 */
        mk_i2cdev(RXP_I2C_I801_BUS,  "spd",        0x51, &spd2_51_at24),

        mk_i2cdev(RXP_I2C_iSMT_BUS,  "pca9541",    0x70,
                                                 &master_switch_platform_data),
        mk_i2cdev(RXP_I2C_MASTER_SWITCH_BUS,
                                     "pca9548",    0x73, &mux1_platform_data),
        mk_i2cdev(RXP_I2C_MUX1_BUS0, "dps460",     0x5a, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS0, "24c02",      0x52, &psu1_50_at24),
        mk_i2cdev(RXP_I2C_MUX1_BUS1, "dps460",     0x5b, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS1, "24c02",      0x53, &psu2_51_at24),
        mk_i2cdev(RXP_I2C_MUX1_BUS2, "24c64",      0x50, &board_50_at24),
        mk_i2cdev(RXP_I2C_MUX1_BUS3, "emc2305",    0x2e, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS3, "emc2305",    0x4d, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS4, "lm75",       0x48, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS5, "lm75",       0x4e, NULL),
        mk_i2cdev(RXP_I2C_MUX1_BUS7, "pca9505",    0x20, &gpio_platform_data),

        mk_i2cdev(RXP_I2C_MASTER_SWITCH_BUS,
                                     "pca9548",    0x71, &mux2_platform_data),
        mk_i2cdev(RXP_I2C_MUX2_BUS3, "lm75",       0x48, NULL),
        mk_i2cdev(RXP_I2C_MUX2_BUS4, "lm75",       0x49, NULL),
        mk_i2cdev(RXP_I2C_MUX2_BUS5, "lm75",       0x4a, NULL),

        mk_i2cdev(RXP_I2C_MASTER_SWITCH_BUS,
                                     "pca9548",    0x77, &mux3_platform_data),

/*
 * Per Celestica: "The fan EEPROM is not installed by default unless customer
 * require it."  Expose them anyways, for the future.
 */

	mk_i2cdev(RXP_I2C_MUX3_BUS0, "24c02",      0x50, &fan2_50_at24),
	mk_i2cdev(RXP_I2C_MUX3_BUS1, "24c02",      0x50, &fan1_50_at24),
	mk_i2cdev(RXP_I2C_MUX3_BUS2, "24c02",      0x50, &fan5_50_at24),
	mk_i2cdev(RXP_I2C_MUX3_BUS3, "24c02",      0x50, &fan3_50_at24),
	mk_i2cdev(RXP_I2C_MUX3_BUS4, "24c02",      0x50, &fan4_50_at24),
};


static struct i2c_client *redxpClientsList[ARRAY_SIZE(i2c_redxp_devices)];
static struct i2c_client *smallxpClientsList[ARRAY_SIZE(i2c_smallxp_devices)];
static struct i2c_client *seaClientsList[ARRAY_SIZE(i2c_sea_devices)];

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
		if (clients_list[idx]) {
			i2c_unregister_device(clients_list[idx]);
		}
	}
}

static void free_xp_data(void)
{
	free_gpio_pins(cel_rxp_claimed_gpios, ARRAY_SIZE(cel_rxp_gpio_pins));
	free_gpio_pins(cel_sxp_claimed_gpios, ARRAY_SIZE(cel_sxp_gpio_pins));

	free_i2c_clients(redxpClientsList,   ARRAY_SIZE(i2c_redxp_devices));
	free_i2c_clients(smallxpClientsList, ARRAY_SIZE(i2c_smallxp_devices));
	free_i2c_clients(seaClientsList,   ARRAY_SIZE(i2c_sea_devices));
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
		return ERR_PTR(-ENODEV);
	}
	return client;
}

static int get_bus_by_name(char *name)
{
	struct i2c_adapter *adapter;
	int i;

	for (i = 0; i < RXP_I2C_MASTER_SWITCH_BUS; i++) {
		adapter = get_adapter(i);
		if (adapter &&
		    (strncmp(adapter->name, name, strlen(name)) == 0)) {
			return i;
		}
	}
	return -1;
}

static int init_gpio_pins(struct gpio_pin *gpio_pin_ptr, int num_pins,
			  int *claimed_list)
{
	int i;
	int ret;
	int gpio_retries = 10;
	int retry;

	for (i = 0; i < num_pins; i++, gpio_pin_ptr++) {
		unsigned long flags;

		retry = gpio_retries;
		flags = GPIOF_EXPORT_DIR_FIXED; /*  direction not changeable. */
		if (gpio_pin_ptr->is_active_low)
		flags |= GPIOF_ACTIVE_LOW;
		if (gpio_pin_ptr->is_output)
			flags |= gpio_pin_ptr->value ? GPIOF_OUT_INIT_HIGH :
					    GPIOF_OUT_INIT_LOW;
		else
		    flags |= GPIOF_DIR_IN;
		do {
			ret = gpio_request_one(gpio_pin_ptr->num, flags, NULL);
		} while (ret && retry--);
		if (ret) {
			pr_err("gpio_request_one for pin %s (%u) failed\n",
			        gpio_pin_ptr->name, gpio_pin_ptr->num);
			return ret;
		}
		claimed_list[i] = gpio_pin_ptr->num;
	}
	return 0;
}

static int get_xp_platform_type(void)
{
	static struct uint8_t *regs;
	int data;

	regs = ioport_map(CPLD_REG_BOARD_TYPE_OFFSET, 1);

	if (!regs) {
		pr_err(" unable to map iomem\n");
		return -1;
	}

	data = ioread8(regs);
	if (data == 0)
		data = CPLD_BOARD_TYPE_REDXP_VAL;
	iounmap(regs);

	return data;
}
     
static int cel_xp_platform_probe(struct platform_device *dev)
{
	return 0;
}

static int cel_xp_platform_remove(struct platform_device *dev)
{
	return 0;
}

static int populate_i2c_devices(struct xp_i2c_device_info *devices,
                                int num_devices,
				struct i2c_client **clients_list,
                                int iSMT_bus, int I801_bus)
{
	int i;
	int ret;
	struct i2c_client *client;

	for (i = 0; i < num_devices; i++) {
		if (devices[i].bus == RXP_I2C_iSMT_BUS) {
			devices[i].bus = iSMT_bus;
		} else if (devices[i].bus == RXP_I2C_I801_BUS) {
			devices[i].bus = I801_bus;
		}
		client = add_i2c_client(devices[i].bus,
					&devices[i].board_info);
		if (IS_ERR(client)) {
			ret = PTR_ERR(client);
			goto err_exit;
		}
		clients_list[i] = client;
	}
	return 0;

err_exit:
	return ret;
}

static int __init cel_xp_platform_init(void)
{
	int iSMT_bus;
	int I801_bus;
	int ret;
	int i;
	int platform;
	struct gpio_pin *platform_gpio_pins;
	int num_platform_pins;
	int *platform_claimed_gpios;

	ret = -1;

	iSMT_bus = get_bus_by_name(ISMT_ADAPTER_NAME);
	if (iSMT_bus < 0) {
		pr_err("could not find iSMT adapter bus\n");
		 ret = -ENODEV;
		 goto err_exit;
	}
	I801_bus = get_bus_by_name(I801_ADAPTER_NAME);
	if (I801_bus < 0) {
		 pr_err("could not find I801 adapter bus\n");
		 ret = -ENODEV;
		 goto err_exit;
	}

	platform = get_xp_platform_type();
	if ((platform == CPLD_BOARD_TYPE_SMALLXP_VAL) ||
	    (platform == CPLD_BOARD_TYPE_SEA_VAL)) {
		platform_gpio_pins = cel_sxp_gpio_pins;
		num_platform_pins = ARRAY_SIZE(cel_sxp_gpio_pins);
		platform_claimed_gpios = cel_sxp_claimed_gpios;
	} else if (platform == CPLD_BOARD_TYPE_REDXP_VAL) {
		platform_gpio_pins = cel_rxp_gpio_pins;
		num_platform_pins = ARRAY_SIZE(cel_rxp_gpio_pins);
		platform_claimed_gpios = cel_rxp_claimed_gpios;
	} else {
		pr_err("unknown platform ID: %u\n", platform);
		goto err_exit;
	}

	/* populate the platform_data structure for the gpio with
	 * our pin names
	 */
	for (i = 0; i < num_platform_pins; i++) {
		pca953x_gpio_names[platform_gpio_pins[i].num -
                           CEL_XP_GPIO_1_BASE] =
                           platform_gpio_pins[i].name;
	}

	/* populate the platform specific i2c devices
	*/
	if (CPLD_BOARD_TYPE_SEA_VAL == platform) {
		ret = populate_i2c_devices(i2c_sea_devices,
					   ARRAY_SIZE(i2c_sea_devices),
		                           seaClientsList,
					   iSMT_bus,
					   I801_bus);
	} else if (CPLD_BOARD_TYPE_SMALLXP_VAL == platform) {
		ret = populate_i2c_devices(i2c_smallxp_devices,
					   ARRAY_SIZE(i2c_smallxp_devices),
		                           smallxpClientsList,
					   iSMT_bus,
					   I801_bus);
	} else if (CPLD_BOARD_TYPE_REDXP_VAL == platform) {
		ret = populate_i2c_devices(i2c_redxp_devices,
					   ARRAY_SIZE(i2c_redxp_devices),
		                           redxpClientsList,
					   iSMT_bus,
					   I801_bus);
	}
	if (ret)
		goto err_exit;

	ret = init_gpio_pins(platform_gpio_pins, num_platform_pins,
                         platform_claimed_gpios);
	if (ret)
		goto err_exit;

	pr_info(DRIVER_NAME": version "DRIVER_VERSION" successfully loaded\n");
	return 0;

err_exit:
	free_xp_data();
	return ret;
}

static void __exit cel_xp_platform_exit(void)
{
	free_xp_data();
	platform_driver_unregister(&cel_xp_platform_driver);
	pr_info(DRIVER_NAME": version "DRIVER_VERSION" unloaded\n");
}

static struct platform_driver cel_xp_platform_driver = {
	.driver = {
		.name = "cel_xp_platform",
		.owner = THIS_MODULE,
	},
	.probe = cel_xp_platform_probe,
	.remove = cel_xp_platform_remove,
};


module_init(cel_xp_platform_init);
module_exit(cel_xp_platform_exit);

MODULE_AUTHOR("Alan Liebthal (alanl@cumulusnetworks.com)");
MODULE_DESCRIPTION("Celestica XP Platform Support");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
