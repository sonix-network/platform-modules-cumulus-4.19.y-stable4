/*
 * Quanta IX7 Platform Definitions
 *
 * Copyright (C) 2018, 2019 Cumulus Networks, Inc.  All Rights Reserved
 * David Yen <dhyen@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 */

#ifndef QUANTA_IX7_H__
#define QUANTA_IX7_H__

#define SMBUS_I801_NAME     "SMBus I801 adapter"
#define SMBUS_ISMT_NAME     "SMBus iSMT adapter"
#define PCA_9555_GPIO_COUNT 16
#define PCA_9554_GPIO_COUNT 8
#define IX7_PORT_COUNT	    32

#define IX7_GPIO_BASE       100
#define IX7_MB_GPIO_23_BASE    IX7_GPIO_BASE
#define IX7_CPU_GPIO_20_BASE    (IX7_MB_GPIO_23_BASE + PCA_9555_GPIO_COUNT)
#define IX7_CPU_GPIO_21_BASE    (IX7_CPU_GPIO_20_BASE + PCA_9555_GPIO_COUNT)
#define IX7_CPU_GPIO_22_BASE    (IX7_CPU_GPIO_21_BASE + PCA_9554_GPIO_COUNT)

#define QUANTA_IX7_CPLD_STRING_NAME_SIZE 30

#define IX7_PLATFORM_NAME                "IX7"
#define QUANTA_IX7_REG_MAX               8

/* IX7 CPLDs */

#define IX7_LED_QSFP28_1_16_CPLD_ID      (0x01)
#define IX7_IO_QSFP28_1_16_CPLD_ID       (0x02)
#define IX7_IO_QSFP28_17_32_CPLD_ID      (0x03)
#define IX7_LED_QSFP28_17_32_CPLD_ID     (0x04)

#define IX7_IO_QSFP28_1_4_INFO_REG       (0x01)
#define IX7_IO_QSFP28_5_8_INFO_REG       (0x02)
#define IX7_IO_QSFP28_9_12_INFO_REG      (0x03)
#define IX7_IO_QSFP28_13_16_INFO_REG     (0x04)

#define IX7_LED_DECODER_REG              (0x04)

/*
 * Structure definitions
 */
struct quanta_ix7_platform_data {
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

#endif
