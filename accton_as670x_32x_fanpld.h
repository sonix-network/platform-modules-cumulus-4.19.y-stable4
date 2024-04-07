/*
 * Accton AS670x-32x FANPLD Platform Definitions
 *
 * Copyright (C) 2014 Cumulus Networks, Inc.
 * Dustin Byford <dustin@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef ACCTON_AS670X_32X_FANPLD_H__
#define ACCTON_AS670X_32X_FANPLD_H__

#define CPLD_FAN_CONTROL		(0x20) /* read write */
#  define CPLD_FAN_CONTROL_MASK			(0x07)
#  define CPLD_FAN_CONTROL_100			(0x07)
#  define CPLD_FAN_CONTROL_75			(0x06)
#  define CPLD_FAN_CONTROL_63			(0x05)
#  define CPLD_FAN_CONTROL_50			(0x04)
#  define CPLD_FAN_CONTROL_38			(0x03)
#  define CPLD_FAN_CONTROL_0			(0x00)

#define CPLD_FAN_SENSOR_0		(0x00) /* read only */
#define CPLD_FAN_SENSOR_0A		(0x01) /* read only */
#define CPLD_FAN_SENSOR_1		(0x02) /* read only */
#define CPLD_FAN_SENSOR_1A		(0x03) /* read only */
#define CPLD_FAN_SENSOR_2		(0x04) /* read only */
#define CPLD_FAN_SENSOR_2A		(0x05) /* read only */
#define CPLD_FAN_SENSOR_3		(0x06) /* read only */
#define CPLD_FAN_SENSOR_3A		(0x07) /* read only */
#define CPLD_FAN_SENSOR_4		(0x08) /* read only */
#define CPLD_FAN_SENSOR_4A		(0x09) /* read only */

#define CPLD_FAN_DIRECTION        	(0x0a) /* read only */
#  define CPLD_FAN_DIRECTION_0			(1 << 0)
#  define CPLD_FAN_DIRECTION_1			(1 << 1)
#  define CPLD_FAN_DIRECTION_2			(1 << 2)
#  define CPLD_FAN_DIRECTION_3			(1 << 3)
#  define CPLD_FAN_DIRECTION_4			(1 << 4)
#  define CPLD_FAN_DIRECTION_FTB		0

#define CPLD_FAN_LED_NORMAL        	(0x0b) /* read only */
#  define CPLD_FAN_NORMAL_L_0			(1 << 0)
#  define CPLD_FAN_NORMAL_L_1			(1 << 1)
#  define CPLD_FAN_NORMAL_L_2			(1 << 2)
#  define CPLD_FAN_NORMAL_L_3			(1 << 3)
#  define CPLD_FAN_NORMAL_L_4			(1 << 4)

#define CPLD_FAN_LED_ABNORMAL        	(0x0c) /* read only */
#  define CPLD_FAN_ABNORMAL_L_0			(1 << 0)
#  define CPLD_FAN_ABNORMAL_L_1			(1 << 1)
#  define CPLD_FAN_ABNORMAL_L_2			(1 << 2)
#  define CPLD_FAN_ABNORMAL_L_3			(1 << 3)
#  define CPLD_FAN_ABNORMAL_L_4			(1 << 4)

#endif /* ACCTON_AS670X_32X_FANPLD_H__ */
