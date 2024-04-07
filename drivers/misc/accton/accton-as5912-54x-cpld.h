/*
 * Accton AS5912-54X CPLD Platform Definitions
 *
 * Copyright (C) 2019 Cumulus Networks, Inc.
 * Author: Amit Kumar Pal <amit@cumulusnetworks.com>
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

#ifndef ACCTON_AS5912_CPLD_H__
#define ACCTON_AS5912_CPLD_H__


/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 * register info from ES5654BT1-FLF-ZZ_Hardware Spec_V0.5_0627_2014.pdf
 */
#define ACCTON_AS5912_CPLD_STRING_NAME_SIZE  30

/* The AS5912-54X has three CPLDs.
 *
 * SYSTEM -- Resets, Fans, LEDs, etc.
 *
 * The CPLD driver has been coded such that we have the appearance of
 * a single CPLD. We do this by embedding a two-bit index for which
 * CPLD must be accessed into each register definition. When the
 * register is accessed, the register definition is decoded into a
 * CPLD index and register offset.
 */
#define CPLD_IDX_SHIFT                    (8)
#define CPLD_IDX_MASK                     (3 << CPLD_IDX_SHIFT)
#define CPLD_1                            (1)
#define CPLD_2                            (2)
#define CPLD_FAN                          (3)
#define CPLD_1_IDX                        (CPLD_1 << CPLD_IDX_SHIFT)
#define CPLD_2_IDX                        (CPLD_2 << CPLD_IDX_SHIFT)
#define CPLD_FAN_IDX                      (CPLD_FAN << CPLD_IDX_SHIFT)
#define NUM_CPLD_I2C_CLIENTS              (3)

#define GET_CPLD_IDX(A)                   (((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT)-1)
#define STRIP_CPLD_IDX(A)                 (A & ~CPLD_IDX_MASK)

/*
 * System CPLD register definitions
 */
#define CPLD_BOARD_ID_REG			(0x00 | CPLD_1_IDX)
#define CPLD_SYS_CPLD_VERSION_REG	(0x01 | CPLD_1_IDX)

#define CPLD_PSU_STATUS_REG		    (0x02 | CPLD_1_IDX)
#  define CPLD_PSU_MASK				(0x3f)
#  define CPLD_PSU_AC_ALERT_L		(0x10)
#  define CPLD_PSU_12V_GOOD			(0x04)
#  define CPLD_PSU_PRESENT_L		(0x01)

/* Reset control for BCM56864, BCM54616S, PCA9548PW */
#define CPLD_RESET_CONTROL_REG		(0x04)
#  define CPLD_RESET_CONTROL_MASK		(0x1f)
/* Reset PCA 9548 PW for QSFP I2C switch */
#    define CPLD_RESET_SW_PWR2			(1 << 4)
/* Reset the BCM56854 MAC */
#    define CPLD_RESET_MAC				(1 << 3)
/* Reset Mgmt PHY BCM54616S  */
#    define CPLD_RESET_MGMT_PHY			(1 << 2)
/* Reset PCA 9548PW  */
#    define CPLD_RESET_SW_PWR			(1 << 1)
/* Clear Shift Register  */
#    define CPLD_RESET_SHIFT_REG_CLR	(1 << 0)


/* Reset for XGPHY 9 to 12 */
#define CPLD_RESET_XGPHY_9_12_REG	0x05
#  define CPLD_RESET_XGPHY_9_12_MASK	(0x0f)
/* Reset PHY 12 i.e. reset:0 */
#    define CPLD_RESET_PHY_12			(1 << 3)
/* Reset PHY 11 i.e. reset:0 */
#    define CPLD_RESET_PHY_11			(1 << 2)
/* Reset PHY 10 i.e. reset:0 */
#    define CPLD_RESET_PHY_10			(1 << 1)
/* Reset PHY 9 i.e. reset:0 */
#    define CPLD_RESET_PHY_9			(1 << 0)


/* Reset for XGPHY 1 to 8 */
#define CPLD_RESET_XGPHY_1_8_REG	0x06
#  define CPLD_RESET_XGPHY_1_8_MASK	(0xff)
/* Reset PHY 8 i.e. reset:0 */
#    define CPLD_RESET_PHY_8			(1 << 7)
/* Reset PHY 7 i.e. reset:0 */
#    define CPLD_RESET_PHY_7			(1 << 6)
/* Reset PHY 6 i.e. reset:0 */
#    define CPLD_RESET_PHY_6			(1 << 5)
/* Reset PHY 5 i.e. reset:0 */
#    define CPLD_RESET_PHY_5			(1 << 4)
/* Reset PHY 4 i.e. reset:0 */
#    define CPLD_RESET_PHY_4			(1 << 3)
/* Reset PHY 3 i.e. reset:0 */
#    define CPLD_RESET_PHY_3			(1 << 2)
/* Reset PHY 2 i.e. reset:0 */
#    define CPLD_RESET_PHY_2			(1 << 1)
/* Reset PHY 1 i.e. reset:0 */
#    define CPLD_RESET_PHY_1			(1 << 0)

#define CPLD_LM75_INTR_STATUS_REG	(0x07)
/* XGPHY interrupt status for PHY1 to PHY8 */
#define CPLD_XGPHY_1_8_INTR_STATUS_REG	(0x04 | CPLD_1_IDX)
/* XGPHY interrupt status for PHY9 to PHY12 */
#define CPLD_XGPHY_9_12_INTR_STATUS_REG	(0x09)

/* System LED control */
#define CPLD_SYSTEM_LED_REG         (0x65 | CPLD_1_IDX)
#  define CPLD_SYS_LED_FAN_MASK			(0x03)
#    define CPLD_SYS_LED_FAN_OFF			(3)
#    define CPLD_SYS_LED_FAN_AMBER			(2)
#    define CPLD_SYS_LED_FAN_GREEN			(1)
#    define CPLD_SYS_LED_FAN_HW_CTRL			(0)
#  define CPLD_SYS_LED_DIAG_MASK		(0x0C)
#    define CPLD_SYS_LED_DIAG_OFF			(3 << 2)
#    define CPLD_SYS_LED_DIAG_RED			(2 << 2)
#    define CPLD_SYS_LED_DIAG_GREEN			(1 << 2)
#    define CPLD_SYS_LED_DIAG_YELLOW			(0 << 2)
#  define CPLD_SYS_LED_LOC_MASK			(0x10)
#    define CPLD_SYS_LED_LOC_OFF			(1 << 4)
#    define CPLD_SYS_LED_LOC_AMBER_BLINK		(0 << 4)

/* PSU LED status */
#define CPLD_PSU_LED_REG		(0x66 | CPLD_1_IDX)
#  define CPLD_SYS_LED_PSU2_MASK		(0x0c)
#    define CPLD_SYS_LED_PSU2_OFF			(3 << 2)
#    define CPLD_SYS_LED_PSU2_AMBER			(2 << 2)
#    define CPLD_SYS_LED_PSU2_GREEN			(1 << 2)
#    define CPLD_SYS_LED_PSU2_HW_CTRL			(0)
#  define CPLD_SYS_LED_PSU1_MASK		(0x30)
#    define CPLD_SYS_LED_PSU1_OFF			(3 << 4)
#    define CPLD_SYS_LED_PSU1_AMBER			(2 << 4)
#    define CPLD_SYS_LED_PSU1_GREEN			(1 << 4)
#    define CPLD_SYS_LED_PSU1_HW_CTRL			(0)

/* Fan Fault status */
#define CPLD_FAN_FAULT_REG		(0x0f | CPLD_FAN_IDX)
/* Fan PWM Cycle control */
#define CPLD_FAN_PWM_REG		(0x0d | CPLD_FAN_IDX)
/* Fan1 speed value */
#define CPLD_FAN1_SPEED_REG		(0x12 | CPLD_FAN_IDX)
/* Fan2 speed value */
#define CPLD_FAN2_SPEED_REG		(0x11 | CPLD_FAN_IDX)
/* Fan3 speed value */
#define CPLD_FAN3_SPEED_REG		(0x12 | CPLD_FAN_IDX)
/* Fan4 speed value */
#define CPLD_FAN4_SPEED_REG		(0x13 | CPLD_FAN_IDX)
/* Fan5 speed value */
#define CPLD_FAN5_SPEED_REG		(0x14 | CPLD_FAN_IDX)

#define CPLD_FAN1_4_LED_REG		(0x1c | CPLD_FAN_IDX)
#   define CPLD_FAN_1_LED          (0xc0)
#   define CPLD_FAN_1_BITS(VAL)    ((VAL & CPLD_FAN_1_LED) >> 6)
#   define CPLD_FAN_2_LED          (0x30)
#   define CPLD_FAN_2_BITS(VAL)    ((VAL & CPLD_FAN_2_LED) >> 4)
#   define CPLD_FAN_3_LED          (0x0c)
#   define CPLD_FAN_3_BITS(VAL)    ((VAL & CPLD_FAN_3_LED) >> 2)
#   define CPLD_FAN_4_LED          (0x03)
#   define CPLD_FAN_4_BITS(VAL)    (VAL & CPLD_FAN_4_LED)

#define CPLD_FAN5_6_LED_REG		(0x1d | CPLD_FAN_IDX)
#   define CPLD_FAN_5_LED          (0x0c)
#   define CPLD_FAN_5_BITS(VAL)    ((VAL & CPLD_FAN_5_LED) >> 2)
#   define CPLD_FAN_6_LED          (0x03)
#   define CPLD_FAN_6_BITS(VAL)    (VAL & CPLD_FAN_6_LED)

#define CPLD_FANR1_SPEED_REG	(0x22 | CPLD_FAN_IDX)
#define CPLD_FANR2_SPEED_REG	(0x19 | CPLD_FAN_IDX)
#define CPLD_FANR3_SPEED_REG	(0x1a | CPLD_FAN_IDX)
#define CPLD_FANR4_SPEED_REG	(0x1b | CPLD_FAN_IDX)
#define CPLD_FANR5_SPEED_REG	(0x1c | CPLD_FAN_IDX)
/* Fan Air flow direction detector */
#define CPLD_FAN_DIRECTION_REG	(0x10 | CPLD_FAN_IDX)
/* FANR fult status */
#define CPLD_FANR_FAULT_REG		(0x1f | CPLD_FAN_IDX)

#define CPLD_QSFP_PRESENT_INTR_REG	(0x50 | CPLD_2_IDX)
#define CPLD_QSFP_FAULT_STAT_REG	(0x51 | CPLD_2_IDX)
#define CPLD_QSFP_PRESENT_STAT_REG	(0x52 | CPLD_2_IDX)
#define CPLD_QSFP_MOD_RESET_REG		(0x53 | CPLD_2_IDX)
#define CPLD_QSFP_LPMODE_REG		(0x54 | CPLD_2_IDX)

/*
 * Port0 CPLD register definitions -- ports 1 to 24
 */
#define CPLD_PORT1_8_CPLD_VERSION_REG	(0x01 | CPLD_1_IDX)

#define CPLD_SFP1_8_PRESENT_INTR_REG	(0x04 | CPLD_1_IDX)
#define CPLD_SFP9_16_PRESENT_INTR_REG	(0x06 | CPLD_1_IDX)
#define CPLD_SFP17_24_PRESENT_INTR_REG	(0x08 | CPLD_1_IDX)

#define CPLD_SFP1_8_PRESENT_STAT_REG	(0x10 | CPLD_1_IDX)
#define CPLD_SFP9_16_PRESENT_STAT_REG	(0x11 | CPLD_1_IDX)
#define CPLD_SFP17_24_PRESENT_STAT_REG	(0x12 | CPLD_1_IDX)

#define CPLD_SFP1_8_TX_FAULT_REG	    (0x14 | CPLD_1_IDX)
#define CPLD_SFP9_16_TX_FAULT_REG	    (0x16 | CPLD_1_IDX)
#define CPLD_SFP17_24_TX_FAULT_REG	    (0x18 | CPLD_1_IDX)

#define CPLD_SFP1_8_TX_DISABLE_REG	    (0x20 | CPLD_1_IDX)
#define CPLD_SFP9_16_TX_DISABLE_REG	    (0x22 | CPLD_1_IDX)
#define CPLD_SFP17_24_TX_DISABLE_REG	(0x24 | CPLD_1_IDX)

#define CPLD_SFP1_8_RX_LOS_REG		    (0x30 | CPLD_1_IDX)
#define CPLD_SFP9_16_RX_LOS_REG		    (0x32 | CPLD_1_IDX)
#define CPLD_SFP17_24_RX_LOS_REG	    (0x34 | CPLD_1_IDX)

/*
 * Port1 CPLD register definitions -- ports 25 to 48
 */
#define CPLD_PORT25_54_CPLD_VERSION_REG	(0x01 | CPLD_2_IDX)

#define CPLD_SFP25_32_PRESENT_INTR_REG	(0x04 | CPLD_2_IDX)
#define CPLD_SFP33_40_PRESENT_INTR_REG	(0x06 | CPLD_2_IDX)
#define CPLD_SFP41_48_PRESENT_INTR_REG	(0x08 | CPLD_2_IDX)

#define CPLD_SFP25_32_PRESENT_STAT_REG	(0x10 | CPLD_2_IDX)
#define CPLD_SFP33_40_PRESENT_STAT_REG	(0x11 | CPLD_2_IDX)
#define CPLD_SFP41_48_PRESENT_STAT_REG	(0x12 | CPLD_2_IDX)

#define CPLD_SFP25_32_TX_FAULT_REG	(0x14 | CPLD_2_IDX)
#define CPLD_SFP33_40_TX_FAULT_REG	(0x16 | CPLD_2_IDX)
#define CPLD_SFP41_48_TX_FAULT_REG	(0x18 | CPLD_2_IDX)

#define CPLD_SFP25_32_TX_DISABLE_REG	(0x20 | CPLD_2_IDX)
#define CPLD_SFP33_40_TX_DISABLE_REG	(0x22 | CPLD_2_IDX)
#define CPLD_SFP41_48_TX_DISABLE_REG	(0x24 | CPLD_2_IDX)

#define CPLD_SFP25_32_RX_LOS_REG	(0x30 | CPLD_2_IDX)
#define CPLD_SFP33_40_RX_LOS_REG	(0x32 | CPLD_2_IDX)
#define CPLD_SFP41_48_RX_LOS_REG	(0x34 | CPLD_2_IDX)


#endif  // ACCTON_AS5912_CPLD_H__
