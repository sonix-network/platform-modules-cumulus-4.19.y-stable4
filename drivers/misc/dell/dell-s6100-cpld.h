/*
 * Switch Board CPLD definitions for dell_s6100 platforms.
 *
 * Copyright (C) 2017 Cumulus Networks, Inc.
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

#ifndef S6100_CPLD_H
#define S6100_CPLD_H

#define S6100_CPLD_DRIVER_NAME	"dell-s6100-cpld"

#define S6100_CPLD_IO_BASE 0x100
#define S6100_CPLD_IO_SIZE 0xa0

/* CPLD LPC Mapped Registers */
#define S6100_CPLD_VERSION			0x00
#define S6100_CPLD_BOARD_TYPE			0x01
#define S6100_CPLD_SW_SCRATCH			0x02
#define S6100_CPLD_ID				0x03
#define S6100_CPLD_BOARD_REV			0x0F
#define S6100_CPLD_RESET_CONTROL		0x10
#define S6100_CPLD_SW_EEPROM_WP			0x20
#define S6100_CPLD_SYS_LED_CTRL			0x30
#  define SYS_LED_STACKING			  0
#  define SYS_LED_BEACON			  1
#  define SYS_LED_STATUS_MASK			  0xC0
#  define SYS_LED_STATUS_SHIFT			  0x6
#  define SYS_LED_STATUS_NORMAL			  0x0
#  define SYS_LED_STATUS_BOOTING		  0x1
#  define SYS_LED_STATUS_MAJOR_FAULT		  0x2
#  define SYS_LED_STATUS_MINOR_FAULT		  0x3
#define S6100_CPLD_7SEG_LED_CTRL		0x31
#define S6100_CPLD_PORT_LED_OPERATING_MODE	0x32
#define S6100_CPLD_PORT_LED_TEST		0x33
#define S6100_CPLD_MISC_INTR_TRIGGER_MODE	0x40
#define S6100_CPLD_MISC_INTR_STATUS_SUMMARY	0x41
#define S6100_CPLD_MISC_INTR_STATUS		0x42
#define S6100_CPLD_MISC_INTR			0x43
#define S6100_CPLD_MISC_INTR_MASK		0x44
#define S6100_CPLD_IO_MODULE_INTR_STATUS	0x50
#define S6100_CPLD_IO_MODULE_PRESENT		0x51
#define S6100_CPLD_IO_MODULE_POWER_CTRL		0x52
#define S6100_CPLD_IO_MODULE_RESET_CTRL		0x53
#define S6100_CPLD_IO_MODULE_PM_RESET_CTRL	0x54
#define S6100_CPLD_IO_MODULE_JTAG_SELECT	0x55
#define S6100_CPLD_SFPP_TXDISABLE_CTRL		0x60
#define S6100_CPLD_SFPP_RATE_SELECT_CTRL	0x61
#define S6100_CPLD_SFPP_RXLOS_STATUS		0x62
#define S6100_CPLD_SFPP_TXFAULT_STATUS		0x63
#define S6100_CPLD_SFPP_PRESENT_STATUS		0x64
#define S6100_CPLD_SFPP_TRIGGER_MODE		0x90
#define S6100_CPLD_SFPP_INTR_STATUS_SUMMARY	0x91
#define S6100_CPLD_SFPP_RXLOS_INTR		0x92
#define S6100_CPLD_SFPP_PRESENT_INTR		0x93
#define S6100_CPLD_SFPP_RXLOS_INTR_MASK		0x94
#define S6100_CPLD_SFPP_PRESENT_INTR_MASK	0x95

#endif /* S6100_CPLD_H */
