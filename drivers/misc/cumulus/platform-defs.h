/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Platform Definitions
 *
 * Copyright (C) 2012-2015, 2017, 2019 Cumulus Networks, Inc.  All rights
 *     reserved.
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

#ifndef PLATFORM_DEFS_H__
#define PLATFORM_DEFS_H__

#include <linux/i2c.h>

/*
 * Define common strings and values used by the platform drivers.
 * This keeps the sysfs interface uniform.
 */

#define PLATFORM_OK             "ok"
#define PLATFORM_INSTALLED	"installed"
#define PLATFORM_NOT_INSTALLED	"not_installed"

/*
 * LED color strings
 */
#define PLATFORM_LED_ON                   "on"
#define PLATFORM_LED_OFF                  "off"
#define PLATFORM_LED_RED                  "red"
#define PLATFORM_LED_RED_BLINKING         "red_blinking"
#define PLATFORM_LED_RED_SLOW_BLINKING    "red_slow_blinking"
#define PLATFORM_LED_GREEN                "green"
#define PLATFORM_LED_GREEN_BLINKING       "green_blinking"
#define PLATFORM_LED_GREEN_SLOW_BLINKING  "green_slow_blinking"
#define PLATFORM_LED_BLUE                 "blue"
#define PLATFORM_LED_BLUE_BLINKING        "blue_blinking"
#define PLATFORM_LED_BLUE_SLOW_BLINKING   "blue_slow_blinking"
#define PLATFORM_LED_YELLOW               "yellow"
#define PLATFORM_LED_YELLOW_BLINKING      "yellow_blinking"
#define PLATFORM_LED_YELLOW_SLOW_BLINKING "yellow_slow_blinking"
#define PLATFORM_LED_AMBER                "amber"
#define PLATFORM_LED_AMBER_BLINKING       "amber_blinking"
#define PLATFORM_LED_AMBER_SLOW_BLINKING  "amber_slow_blinking"
#define PLATFORM_LED_HW_CTRL              "hw_ctrl"

/*
 * Pluggable Power Supply Strings
 */
#define xstr(s) str(s)
#define str(s) #s
#define PLATFORM_PS_NAME_BASE        psu_pwr
#define PLATFORM_PS_NAME_0           psu_pwr1
#define PLATFORM_PS_NAME_1           psu_pwr2
#define PLATFORM_PS_POWER_BAD        "power_bad"
#define PLATFORM_PS_FAN_BAD	     "fan_bad"
#define PLATFORM_PS_TEMP_BAD	     "temp_bad"

/*
 * QSFP/SFPs
 */
#define QSFP_DATA_BYTE_LEN           256
#define QSFP_PAGE_SIZE               1
#define QSFP_EEPROM_ADDR             0x50
#define SFP_DATA_BYTE_LEN            512
#define SFP_PAGE_SIZE                1
#define SFP_EEPROM_ADDR              0x50

/*
 * Fan Strings
 */
#define PLATFORM_FAN_NOT_SPINNING   "not_spinning"

#define PLATFORM_LED_COLOR_NAME_SIZE 20
struct led_color {
	char name[PLATFORM_LED_COLOR_NAME_SIZE];
	u8 value;
};

/*
 * Common I2C bus names
 */

#define ISMT_ADAPTER_NAME "SMBus iSMT adapter"
#define I801_ADAPTER_NAME "SMBus I801 adapter"

/**
 * mk_eeprom_psize - Create at24 EEPROM platform data structures with
 *              associated eeprom_class platform data structures.
 *
 * @_label:     EEPROM label -- final label in sysfs has "_eeprom" appended
 * @_addr:      i2c address
 * @_size:      size of EEPROM in bytes
 * @_page_size: chip page size
 * @_flags:     Any additional at42 flags from <linux/i2c/at24.h>
 *
 * Note: The declarations do not use __initdata as the drivers do not
 * make deep copies of the data structures.  This is in keeping with
 * other uses of platform data.
 */
#define mk_eeprom_psize(_label, _addr, _size, _page_size, _flags)	\
	struct eeprom_platform_data _label##_##_addr##_eeprom = {	\
		.label = #_label "_eeprom",				\
	};								\
	struct at24_platform_data  _label##_##_addr##_at24 = {		\
		.byte_len = _size,					\
		.flags = _flags,					\
		.page_size = _page_size,				\
		.eeprom_data = &_label##_##_addr##_eeprom,		\
	}

#define mk_eeprom(_label, _addr, _size, _flags)				\
	mk_eeprom_psize(_label, _addr, _size, 1, _flags)

#define mk_port_eeprom(_label, _addr, _size, _flags)			\
	struct eeprom_platform_data _label##_##_addr##_eeprom = {	\
		.label = #_label,					\
	};								\
	struct at24_platform_data  _label##_##_addr##_at24 = {		\
		.byte_len = _size,					\
		.flags = _flags,					\
		.page_size = 1,						\
		.eeprom_data = &_label##_##_addr##_eeprom,		\
	}

#define mk_qsfp_port_eeprom(_label, _addr, _size, _flags)		\
	struct eeprom_platform_data _label##_##_addr##_qsfp_eeprom = {	\
		.label = #_label,					\
	};								\
	struct sff_8436_platform_data _label##_##_addr##_sff8436 = {	\
		.byte_len = _size,					\
		.flags = _flags,					\
		.page_size = 1,						\
		.eeprom_data = &_label##_##_addr##_qsfp_eeprom,		\
	}

/*
 * Macros for making pca954x_platform_data definitions.
 *
 * Each mk_pca954*(name, base, desel) creates a <name>_platform_data
 * definition for the correct number of i2c-buses starting at base,
 * and with the deselect_on_exit flag set to desel.
 * See the Alpha Networks SNQ platform driver for an example.
 */
#define mk_pca9548(_name, _basebusnum, _deselect_on_exit) \
	static struct pca954x_platform_mode _name##_platform_modes[] = { \
	    mk_pca954x_platform_mode((_basebusnum) + 0, _deselect_on_exit), \
	    mk_pca954x_platform_mode((_basebusnum) + 1, _deselect_on_exit), \
	    mk_pca954x_platform_mode((_basebusnum) + 2, _deselect_on_exit), \
	    mk_pca954x_platform_mode((_basebusnum) + 3, _deselect_on_exit), \
	    mk_pca954x_platform_mode((_basebusnum) + 4, _deselect_on_exit), \
	    mk_pca954x_platform_mode((_basebusnum) + 5, _deselect_on_exit), \
	    mk_pca954x_platform_mode((_basebusnum) + 6, _deselect_on_exit), \
	    mk_pca954x_platform_mode((_basebusnum) + 7, _deselect_on_exit), \
	}; \
	mk_pca954x_platform_data(_name)

#define mk_pca9547(_name, _basebusnum, _deselect_on_exit) \
	mk_pca9548(_name, _basebusnum, _deselect_on_exit)

#define mk_pca9545(_name, _basebusnum, _deselect_on_exit) \
	static struct pca954x_platform_mode _name##_platform_modes[] = { \
	    mk_pca954x_platform_mode((_basebusnum) + 0, _deselect_on_exit), \
	    mk_pca954x_platform_mode((_basebusnum) + 1, _deselect_on_exit), \
	    mk_pca954x_platform_mode((_basebusnum) + 2, _deselect_on_exit), \
	    mk_pca954x_platform_mode((_basebusnum) + 3, _deselect_on_exit), \
	}; \
	mk_pca954x_platform_data(_name)

#define mk_pca9543(_name, _basebusnum, _deselect_on_exit) \
	static struct pca954x_platform_mode _name##_platform_modes[] = { \
	    mk_pca954x_platform_mode((_basebusnum) + 0, _deselect_on_exit), \
	    mk_pca954x_platform_mode((_basebusnum) + 1, _deselect_on_exit), \
	}; \
	mk_pca954x_platform_data(_name)

#define mk_pca9541(_name, _busnum) \
	static struct pca954x_platform_mode _name##_platform_modes[] = { \
	    mk_pca954x_platform_mode((_busnum), 0), \
	}; \
	mk_pca954x_platform_data(_name)

/* helpers for mk_pca954*() */
#define mk_pca954x_platform_mode(_busnum, _deselect_on_exit) \
	{ .adap_id = (_busnum), .deselect_on_exit = (_deselect_on_exit) }
#define mk_pca954x_platform_data(_name) \
	static struct pca954x_platform_data _name##_platform_data = { \
		.modes = _name##_platform_modes, \
		.num_modes = ARRAY_SIZE(_name##_platform_modes), \
	}

/**
 * struct platform_i2c_device_info -- i2c device container struct.  This
 * struct helps organize the i2c_board_info structures and the created
 * i2c_client objects.
 *
 */
struct platform_i2c_device_info {
	int bus;
	struct i2c_board_info board_info;
	struct i2c_client *client;
};

#define mk_i2cdev(_bus, _type, _addr, _data) \
	{ \
		.bus = (_bus), \
		.board_info = { \
			.type = (_type), \
			.addr = (_addr), \
			.platform_data = (_data), \
		} \
	}

/**
 * SYSFS_ATTR_RW - Convienence macro to create a read/write file in sysfs.
 *
 * @_name: sysfs file name
 * @_show: sysfs read call back
 * @_store: sysfs write call back
 *
 * The sysfs file will be created with read permission for all users
 * and write permission for root (file mode 0644).
 */
#define SYSFS_ATTR_RW(_name, _show, _store) \
	DEVICE_ATTR(_name, 0644, _show, _store)

/**
 * SYSFS_ATTR_RO - Convienence macro to create a read only file in sysfs.
 *
 * @_name: sysfs file name
 * @_show: sysfs read call back
 *
 * The sysfs file will be created with read permission for all users
 * (file mode 0444).
 */
#define SYSFS_ATTR_RO(_name, _show) \
	DEVICE_ATTR(_name, 0444, _show, NULL)

/**
 * SYSFS_ATTR_WO - Convienence macro to create a write only file in sysfs.
 *
 * @_name: sysfs file name
 * @_store: sysfs write call back
 *
 * The sysfs file will be created with write permission for root user
 * (file mode 0200).
 */
#define SYSFS_ATTR_WO(_name, _store) \
	DEVICE_ATTR(_name, 0200, NULL, _store)

/**
 * SENSOR_DEVICE_ATTR_RW - Convienence macro to create a read/write file in
 *    sysfs.
 *
 * @_name:      sysfs file name
 * @_show:      sysfs read call back
 * @_store:     sysfs write call back
 * @_index:     sysfs integer index
 * The sysfs file will be created with read permission for all users
 * and write permission for root (file mode 0644).
 */
#define SENSOR_DEVICE_ATTR_RW(_name, _show, _store, _index) \
	SENSOR_DEVICE_ATTR(_name, 0644, _show, _store, _index)

/**
 * SENSOR_DEVICE_ATTR_RO - Convienence macro to create a read only file in
 *    sysfs.
 *
 * @_name:      sysfs file name
 * @_show:      sysfs read call back
 * @_index:     sysfs integer index
 * The sysfs file will be created with read permission for all users
 * (file mode 0444).
 */
#define SENSOR_DEVICE_ATTR_RO(_name, _show, _index) \
	SENSOR_DEVICE_ATTR(_name, 0444, _show, NULL, _index)

#endif /* PLATFORM_DEFS_H__ */
