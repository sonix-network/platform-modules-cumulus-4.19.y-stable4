/*
 * Accton AS4610-54 CPLD Platform Definitions
 *
 * David Yen (dhyen@cumulusnetworks.com)
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
 *
 */

#ifndef ACT_AS4610_CPLD_H__
#define ACT_AS4610_CPLD_H__

#define CPLD_PRODUCT_ID_OFFSET                   (0x01)
#define CPLD_SFP_1_2_STATUS_OFFSET               (0x02)
#define CPLD_SFP_3_4_STATUS_OFFSET               (0x03)
#define CPLD_POE_STATUS_OFFSET                   (0x0c)
#define CPLD_POE_CONTROL_OFFSET                  (0x0d)
#define CPLD_POWER_STATUS_OFFSET                 (0x11)
#define CPLD_SYS_TEMP_OFFSET                     (0x14)
#define CPLD_PHY1_RESET_OFFSET                   (0x19)
#define CPLD_SEGMENT_LED_BLINK_OFFSET            (0x1a)
#define CPLD_FAN_SPEED_CONTROL_OFFSET            (0x2b)
#define CPLD_FAN_1_SPEED_DETECT_OFFSET           (0x2c)
#define CPLD_FAN_2_SPEED_DETECT_OFFSET           (0x2d)
#define CPLD_DEBUG_VERSION_OFFSET                (0x2e)
#define CPLD_SEGMENT_2_LED_OFFSET                (0x30)
#define CPLD_SEGMENT_1_LED_OFFSET                (0x31)
#define CPLD_SYSTEM_1_LED_OFFSET                 (0x32)
#define CPLD_SYSTEM_2_LED_OFFSET                 (0x33)
#define CPLD_CPLD_VERSION_OFFSET		 (0x0b)

#endif  // ACT_AS4610_CPLD_H__
