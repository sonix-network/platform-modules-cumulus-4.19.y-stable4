/*
 * Accton AS5712-54X CPLD Platform Definitions
 *
 * Curt Brune <curt@cumulusnetworks.com>
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

#ifndef ACCTON_AS5712_CPLD_H__
#define ACCTON_AS5712_CPLD_H__


/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 * register info from ES5654BT1-FLF-ZZ_Hardware Spec_V0.5_0627_2014.pdf
 */
#define ACCTON_AS5712_CPLD_STRING_NAME_SIZE  30

/* The AS5712-54X has three CPLDs: SYSTEM, PORT0 and PORT1.
 *
 * SYSTEM -- Resets, Fans, LEDs, etc.
 * PORT0  -- SFP+ I/Os for ports 1-24
 * PORT1  -- SFP+ I/Os for ports 25-48, QSFP I/Os 1-6
 *
 * The CPLD driver has been coded such that we have the appearance of
 * a single CPLD. We do this by embedding a two-bit index for which
 * CPLD must be accessed into each register definition. When the
 * register is accessed, the register definition is decoded into a
 * CPLD index and register offset.
 */
#define CPLD_IDX_SHIFT                    (6)
#define CPLD_SYSTEM_ID                    (1)
#define CPLD_PORT1_24_ID                  (2)
#define CPLD_PORT25_54_ID                 (3)
#define CPLD_IDX_MASK                     (3 << CPLD_IDX_SHIFT)
#define CPLD_SYSTEM_IDX                   (CPLD_SYSTEM_ID << CPLD_IDX_SHIFT)
#define CPLD_PORT1_24_IDX                 (CPLD_PORT1_24_ID << CPLD_IDX_SHIFT)
#define CPLD_PORT25_54_IDX                (CPLD_PORT25_54_ID << CPLD_IDX_SHIFT)
#define NUM_CPLD_I2C_CLIENTS              (3)

#define GET_CPLD_IDX(A)                   (((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT)-1)
#define STRIP_CPLD_IDX(A)                 (A & ~CPLD_IDX_MASK)

/*
 * System CPLD register definitions
 */
#define CPLD_BOARD_ID_REG		(0x00 | CPLD_SYSTEM_IDX)
#define CPLD_SYS_CPLD_VERSION_REG	(0x01 | CPLD_SYSTEM_IDX)

#define CPLD_PSU_STATUS_REG		(0x02 | CPLD_SYSTEM_IDX)
#  define CPLD_PSU_MASK				(0x7)
#  define CPLD_PSU_SHIFT			(0x4)
#  define CPLD_PSU_AC_ALERT_L			(0x4)
#  define CPLD_PSU_12V_GOOD			(0x2)
#  define CPLD_PSU_PRESENT_L			(0x1)

#define CPLD_RESET_CONTROL_REG		(0x04 | CPLD_SYSTEM_IDX)
#define CPLD_LM75_INTR_STATUS_REG	(0x07 | CPLD_SYSTEM_IDX)

#define CPLD_SYSTEM_LED_REG		(0x0a | CPLD_SYSTEM_IDX)
#  define CPLD_SYS_LED_FAN_MASK			(0x03)
#    define CPLD_SYS_LED_FAN_OFF			(3)
#    define CPLD_SYS_LED_FAN_GREEN			(2)
#    define CPLD_SYS_LED_FAN_AMBER			(1)
#    define CPLD_SYS_LED_FAN_HW_CTRL			(0)
#  define CPLD_SYS_LED_DIAG_MASK		(0x0C)
#    define CPLD_SYS_LED_DIAG_OFF			(3 << 2)
#    define CPLD_SYS_LED_DIAG_GREEN			(2 << 2)
#    define CPLD_SYS_LED_DIAG_RED			(1 << 2)
#    define CPLD_SYS_LED_DIAG_YELLOW			(0 << 2)
#  define CPLD_SYS_LED_LOC_MASK			(0x30)
#    define CPLD_SYS_LED_LOC_OFF			(1 << 4)
#    define CPLD_SYS_LED_LOC_AMBER_BLINK		(2 << 4)
#    define CPLD_SYS_LED_LOC_AMBER			(0 << 4)

#define CPLD_PSU_LED_REG		(0x0b | CPLD_SYSTEM_IDX)
#  define CPLD_SYS_LED_PSU1_MASK		(0x03)
#    define CPLD_SYS_LED_PSU1_OFF			(3)
#    define CPLD_SYS_LED_PSU1_GREEN			(2)
#    define CPLD_SYS_LED_PSU1_AMBER			(1)
#    define CPLD_SYS_LED_PSU1_HW_CTRL			(0)
#  define CPLD_SYS_LED_PSU2_MASK		(0x0C)
#    define CPLD_SYS_LED_PSU2_OFF			(3 << 2)
#    define CPLD_SYS_LED_PSU2_GREEN			(2 << 2)
#    define CPLD_SYS_LED_PSU2_AMBER			(1 << 2)
#    define CPLD_SYS_LED_PSU2_HW_CTRL			(0 << 2)

#define CPLD_FAN_FAULT_REG		(0x0c | CPLD_SYSTEM_IDX)
#define CPLD_FAN_PWM_REG		(0x0d | CPLD_SYSTEM_IDX)
#define CPLD_FAN1_SPEED_REG		(0x10 | CPLD_SYSTEM_IDX)
#define CPLD_FAN2_SPEED_REG		(0x11 | CPLD_SYSTEM_IDX)
#define CPLD_FAN3_SPEED_REG		(0x12 | CPLD_SYSTEM_IDX)
#define CPLD_FAN4_SPEED_REG		(0x13 | CPLD_SYSTEM_IDX)
#define CPLD_FAN5_SPEED_REG		(0x14 | CPLD_SYSTEM_IDX)
#define CPLD_FAN1_4_LED_REG		(0x16 | CPLD_SYSTEM_IDX)
#define CPLD_FAN5_LED_REG		(0x17 | CPLD_SYSTEM_IDX)
#define CPLD_FANR1_SPEED_REG		(0x18 | CPLD_SYSTEM_IDX)
#define CPLD_FANR2_SPEED_REG		(0x19 | CPLD_SYSTEM_IDX)
#define CPLD_FANR3_SPEED_REG		(0x1a | CPLD_SYSTEM_IDX)
#define CPLD_FANR4_SPEED_REG		(0x1b | CPLD_SYSTEM_IDX)
#define CPLD_FANR5_SPEED_REG		(0x1c | CPLD_SYSTEM_IDX)
#define CPLD_FAN_DIRECTION_REG		(0x1e | CPLD_SYSTEM_IDX)
#define CPLD_FANR_FAULT_REG		(0x1f | CPLD_SYSTEM_IDX)

/*
 * Port0 CPLD register definitions -- ports 1 to 24
 */
#define CPLD_PORT1_24_CPLD_VERSION_REG	(0x01 | CPLD_PORT1_24_IDX)
#define CPLD_PORT1_24_I2C_SELECT_REG	(0x02 | CPLD_PORT1_24_IDX)

#define CPLD_SFP1_8_PRESENT_INTR_REG	(0x03 | CPLD_PORT1_24_IDX)
#define CPLD_SFP9_16_PRESENT_INTR_REG	(0x04 | CPLD_PORT1_24_IDX)
#define CPLD_SFP17_24_PRESENT_INTR_REG	(0x05 | CPLD_PORT1_24_IDX)

#define CPLD_SFP1_8_PRESENT_STAT_REG	(0x06 | CPLD_PORT1_24_IDX)
#define CPLD_SFP9_16_PRESENT_STAT_REG	(0x07 | CPLD_PORT1_24_IDX)
#define CPLD_SFP17_24_PRESENT_STAT_REG	(0x08 | CPLD_PORT1_24_IDX)

#define CPLD_SFP1_8_TX_FAULT_REG	(0x09 | CPLD_PORT1_24_IDX)
#define CPLD_SFP9_16_TX_FAULT_REG	(0x0a | CPLD_PORT1_24_IDX)
#define CPLD_SFP17_24_TX_FAULT_REG	(0x0b | CPLD_PORT1_24_IDX)

#define CPLD_SFP1_8_TX_DISABLE_REG	(0x0c | CPLD_PORT1_24_IDX)
#define CPLD_SFP9_16_TX_DISABLE_REG	(0x0d | CPLD_PORT1_24_IDX)
#define CPLD_SFP17_24_TX_DISABLE_REG	(0x0e | CPLD_PORT1_24_IDX)

#define CPLD_SFP1_8_RX_LOS_REG		(0x0f | CPLD_PORT1_24_IDX)
#define CPLD_SFP9_16_RX_LOS_REG		(0x10 | CPLD_PORT1_24_IDX)
#define CPLD_SFP17_24_RX_LOS_REG	(0x11 | CPLD_PORT1_24_IDX)

/*
 * Port1 CPLD register definitions -- ports 25 to 48, QSFP 1-6
 */
#define CPLD_PORT25_54_CPLD_VERSION_REG	(0x01 | CPLD_PORT25_54_IDX)
#define CPLD_PORT25_54_I2C_SELECT_REG	(0x02 | CPLD_PORT25_54_IDX)

#define CPLD_SFP25_32_PRESENT_INTR_REG	(0x03 | CPLD_PORT25_54_IDX)
#define CPLD_SFP33_40_PRESENT_INTR_REG	(0x04 | CPLD_PORT25_54_IDX)
#define CPLD_SFP41_48_PRESENT_INTR_REG	(0x05 | CPLD_PORT25_54_IDX)

#define CPLD_SFP25_32_PRESENT_STAT_REG	(0x06 | CPLD_PORT25_54_IDX)
#define CPLD_SFP33_40_PRESENT_STAT_REG	(0x07 | CPLD_PORT25_54_IDX)
#define CPLD_SFP41_48_PRESENT_STAT_REG	(0x08 | CPLD_PORT25_54_IDX)

#define CPLD_SFP25_32_TX_FAULT_REG	(0x09 | CPLD_PORT25_54_IDX)
#define CPLD_SFP33_40_TX_FAULT_REG	(0x0a | CPLD_PORT25_54_IDX)
#define CPLD_SFP41_48_TX_FAULT_REG	(0x0b | CPLD_PORT25_54_IDX)

#define CPLD_SFP25_32_TX_DISABLE_REG	(0x0c | CPLD_PORT25_54_IDX)
#define CPLD_SFP33_40_TX_DISABLE_REG	(0x0d | CPLD_PORT25_54_IDX)
#define CPLD_SFP41_48_TX_DISABLE_REG	(0x0e | CPLD_PORT25_54_IDX)

#define CPLD_SFP25_32_RX_LOS_REG	(0x0f | CPLD_PORT25_54_IDX)
#define CPLD_SFP33_40_RX_LOS_REG	(0x10 | CPLD_PORT25_54_IDX)
#define CPLD_SFP41_48_RX_LOS_REG	(0x11 | CPLD_PORT25_54_IDX)

#define CPLD_QSFP_PRESENT_INTR_REG	(0x12 | CPLD_PORT25_54_IDX)
#define CPLD_QSFP_FAULT_STAT_REG	(0x13 | CPLD_PORT25_54_IDX)
#define CPLD_QSFP_PRESENT_STAT_REG	(0x14 | CPLD_PORT25_54_IDX)
#define CPLD_QSFP_MOD_RESET_REG		(0x15 | CPLD_PORT25_54_IDX)
#define CPLD_QSFP_LPMODE_REG		(0x16 | CPLD_PORT25_54_IDX)

/*
 * Constants for EEPROM port mapping
 */
#define EEPROM_MUX_PORT_RANGE1_SFP	(24)
#define EEPROM_MUX_PORT_RANGE2_SFP	(48)
#define EEPROM_MUX_PORT_RANGE_QSFP	(54)
#define EEPROM_MUX_TOTAL_PORTS		(54)

#endif  // ACCTON_AS5712_CPLD_H__
