/*
 * Quanta IX2 Rangeley Pebble Platform Definitions
 *
 * Copyright (C) 2016 Cumulus Networks, Inc.
 * Vidya Sagar Ravipati <vidya@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 */

#ifndef QUANTA_IX2_RANGELEY_PLATFORM_H__
#define QUANTA_IX2_RANGELEY_PLATFORM_H__

#define SMB_I801_NAME	"SMBus I801 adapter"
#define PCA_9555_GPIO_COUNT 16
#define PCA_9698_GPIO_COUNT 40
#define IX2_PORT_COUNT	56

#define IX2_GPIO_BASE      100
#define IX2_GPIO_20_BASE   IX2_GPIO_BASE
#define IX2_GPIO_21_BASE   (IX2_GPIO_20_BASE + PCA_9555_GPIO_COUNT)
#define IX2_GPIO_22_BASE   (IX2_GPIO_21_BASE + PCA_9555_GPIO_COUNT)
#define IX2_GPIO_26_BASE   (IX2_GPIO_22_BASE + PCA_9555_GPIO_COUNT)
#define IX2_GPIO_23_BASE   (IX2_GPIO_26_BASE + PCA_9555_GPIO_COUNT)
#define IX2_GPIO_25_BASE   (IX2_GPIO_23_BASE + PCA_9555_GPIO_COUNT)
#define IX2_GPIO_26_2_BASE (IX2_GPIO_25_BASE + PCA_9555_GPIO_COUNT)
#define IX2_GPIO_21_2_BASE (IX2_GPIO_26_2_BASE + PCA_9555_GPIO_COUNT)

/*
 * Structure definitions
 */
struct quanta_ix2_rangeley_platform_data {
	int idx;
};

/*
 * GPIO definitions
 *
 * The GPIO infrastructure has an unfortunate interface for
 * initializing pins.  The pin names are specified in platform_data,
 * but everthing else has to be done by calls after pin creation.
 * So we consolidate all the information in one place (struct gpio_pin),
 * and try to make the best of it.  The pin name array is filled in
 * at runtime by init_gpio_platform_data(), before creating the I2C
 * device.  Then the pins are created by init_gpio_pins().
 */

struct gpio_pin {
	int num;
	const char *name;
	unsigned long flags;
};

#define mk_gpio_pins(_name) \
	static struct gpio_pin _name##_pins[]
#define mk_gpio_pin(_num, _name, _flags) \
	{ \
		.num = (_num), \
		.name = #_name, \
		.flags = (GPIOF_EXPORT_DIR_FIXED | (_flags)) \
	}
#define mk_gpio_platform_data(_name, _base, _numpins) \
	static char const *_name##_pinnames[_numpins]; \
	static struct pca953x_platform_data _name##_platform_data = { \
		.gpio_base = (_base), \
		.names = _name##_pinnames, \
	}

enum {
	IX2_I2C_I801_BUS = -1,

	IX2_I2C_MUX1_BUS0 = 10,
	IX2_I2C_MUX1_BUS1,
	IX2_I2C_MUX1_BUS2,
	IX2_I2C_MUX1_BUS3,
	IX2_I2C_MUX1_BUS4,
	IX2_I2C_MUX1_BUS5,
	IX2_I2C_MUX1_BUS6,
	IX2_I2C_MUX1_BUS7,

	IX2_I2C_MUX2_BUS0 = 20,
	IX2_I2C_MUX2_BUS1,
	IX2_I2C_MUX2_BUS2,
	IX2_I2C_MUX2_BUS3,
	IX2_I2C_MUX2_BUS4,
	IX2_I2C_MUX2_BUS5,
	IX2_I2C_MUX2_BUS6,
	IX2_I2C_MUX2_BUS7,

	IX2_I2C_MUX3_BUS0 = 30,
	IX2_I2C_MUX3_BUS1,
	IX2_I2C_MUX3_BUS2,
	IX2_I2C_MUX3_BUS3,
	IX2_I2C_MUX3_BUS4,
	IX2_I2C_MUX3_BUS5,
	IX2_I2C_MUX3_BUS6,
	IX2_I2C_MUX3_BUS7,

	IX2_I2C_MUX4_BUS0 = 40,
	IX2_I2C_MUX4_BUS1,
	IX2_I2C_MUX4_BUS2,
	IX2_I2C_MUX4_BUS3,
	IX2_I2C_MUX4_BUS4,
	IX2_I2C_MUX4_BUS5,
	IX2_I2C_MUX4_BUS6,
	IX2_I2C_MUX4_BUS7,

	IX2_I2C_MUX5_BUS0 = 50,
	IX2_I2C_MUX5_BUS1,
	IX2_I2C_MUX5_BUS2,
	IX2_I2C_MUX5_BUS3,
	IX2_I2C_MUX5_BUS4,
	IX2_I2C_MUX5_BUS5,
	IX2_I2C_MUX5_BUS6,
	IX2_I2C_MUX5_BUS7,

	IX2_I2C_MUX6_BUS0 = 60,
	IX2_I2C_MUX6_BUS1,
	IX2_I2C_MUX6_BUS2,
	IX2_I2C_MUX6_BUS3,
	IX2_I2C_MUX6_BUS4,
	IX2_I2C_MUX6_BUS5,
	IX2_I2C_MUX6_BUS6,
	IX2_I2C_MUX6_BUS7,

	IX2_I2C_MUX7_BUS0 = 70,
	IX2_I2C_MUX7_BUS1,
	IX2_I2C_MUX7_BUS2,
	IX2_I2C_MUX7_BUS3,
	IX2_I2C_MUX7_BUS4,
	IX2_I2C_MUX7_BUS5,
	IX2_I2C_MUX7_BUS6,
	IX2_I2C_MUX7_BUS7,

	IX2_I2C_MUX8_BUS0 = 80,
	IX2_I2C_MUX8_BUS1,
	IX2_I2C_MUX8_BUS2,
	IX2_I2C_MUX8_BUS3,
	IX2_I2C_MUX8_BUS4,
	IX2_I2C_MUX8_BUS5,
	IX2_I2C_MUX8_BUS6,
	IX2_I2C_MUX8_BUS7,

	IX2_I2C_MUX9_BUS0 = 90,
	IX2_I2C_MUX9_BUS1,
	IX2_I2C_MUX9_BUS2,
	IX2_I2C_MUX9_BUS3,
	IX2_I2C_MUX9_BUS4,
	IX2_I2C_MUX9_BUS5,
	IX2_I2C_MUX9_BUS6,
	IX2_I2C_MUX9_BUS7,

	IX2_I2C_MUX10_BUS0 = 100,
	IX2_I2C_MUX10_BUS1,
	IX2_I2C_MUX10_BUS2,
	IX2_I2C_MUX10_BUS3,
	IX2_I2C_MUX10_BUS4,
	IX2_I2C_MUX10_BUS5,
	IX2_I2C_MUX10_BUS6,
	IX2_I2C_MUX10_BUS7,

	IX2_I2C_MUX11_BUS = 110,
};

#endif
