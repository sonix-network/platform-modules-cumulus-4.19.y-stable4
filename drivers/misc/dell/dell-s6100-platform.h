/*
 * Platform device definitions for dell_s6100 platforms.
 *
 * Copyright (C) 2015,2016,2017 Cumulus Networks, Inc.
 * Author: Puneet Shenoy <puneet@cumulusnetworks.com>
 * Author: Curt Brune <curt@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * https://www.gnu.org/licenses/gpl-2.0-standalone.html
 */

#ifndef DELL_S6100_PLATFORM_H
#define DELL_S6100_PLATFORM_H

/**
 * NUM_IO_MODULES
 *
 * Number of IO module slots in the chassis
 */
#define NUM_IO_MODULES (4)

/**
 * struct s6100_gpio_data - gpio configuration
 *
 * @name:    GPIO signal name
 * @reg_map: Which register map to use, default 0
 * @reg:     Register inside SMF/CPLD that controls GPIO
 * @bit:     Bit offset within @reg for this GPIO signal
 * @flags:   GPIO configuration, like in/out and active-low
 * @value:   Initiial GPIO value for output GPIOs
 *
 * Defines a GPIO signal, driven by a s6100 CPLD register.
 */
struct s6100_gpio_data {
	const char *name;
	uint8_t     reg_map;
	uint16_t    reg;
	uint8_t     bit;
	uint32_t    flags;
	int         value;
};

/**
 * ATTR_DATA_RO()
 *
 * Helper macro for making struct sensor_device_attribute entries.
 */
#define ATTR_DATA_RO(_name, _show, _index) \
	SENSOR_ATTR(_name, S_IRUGO, _show, NULL, _index)

/**
 * ATTR_DATA_RO_2()
 *
 * Helper macro for making read-only struct sensor_device_attribute_2 entries.
 */
#define ATTR_DATA_RO_2(_name, _show, _index, _nr) \
	SENSOR_ATTR_2(_name, S_IRUGO, _show, NULL, _nr, _index)

/**
 * SENSOR_ATTR_DATA_RO()
 *
 * Helper macro for making read-only struct sensor_device_attribute_2
 * variables.
 */
#define SENSOR_ATTR_DATA_RO(_name, _show, _index) \
	SENSOR_DEVICE_ATTR(_name, S_IRUGO, _show, NULL, _index)

/**
 * SENSOR_ATTR_DATA_RO_2()
 *
 * Helper macro for making read-only struct sensor_device_attribute_2
 * variables.
 */
#define SENSOR_ATTR_DATA_RO_2(_name, _show, _index, _nr) \
	SENSOR_DEVICE_ATTR_2(_name, S_IRUGO, _show, NULL, _nr, _index)

/**
 * SENSOR_ATTR_DATA_RW_2()
 *
 * Helper macro for making read-write struct sensor_device_attribute_2
 * variables.
 */
#define SENSOR_ATTR_DATA_RW_2(_name, _show, _store, _index, _nr) \
	SENSOR_DEVICE_ATTR_2(_name, S_IRUGO | S_IWUSR,		 \
			     _show, _store, _nr, _index)

#endif /* DELL_S6100_PLATFORM_H */
