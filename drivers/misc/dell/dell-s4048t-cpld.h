/*
 * DELL S4048T CPLD Platform Definitions
 *
 *  Copyright (C) 2016 Cumulus Networks, Inc.
 * Puneet Shenoy <puneet@cumulusnetworks.com>
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

#ifndef DELL_S4048T_CPLD_H__
#define DELL_S4048T_CPLD_H__


/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 * register info from S4048T-ON_HW_SPEC_REV09_20151209.pdf
 */
#define DELL_S4048T_CPLD_STRING_NAME_SIZE  30

/* the S4048t actually has three CPLDs: system, master, slave
 * the CPLD driver has been coded such that we have the appearance of a single
 * CPLD. We do this by embedding a two-bit index for which CPLD must be accessed
 * into each register definition. When the register is accessed, the register
 * definition is decoded into a CPLD index and register offset.
 */
#define CPLD_IDX_SHIFT                    (6)
#define SYSTEM_CPLD_ID                    (1)
#define MASTER_CPLD_ID                    (2)
#define CPLD_IDX_MASK                     (3 << CPLD_IDX_SHIFT)
#define SYSTEM_CPLD_IDX                   (SYSTEM_CPLD_ID << CPLD_IDX_SHIFT)
#define MASTER_CPLD_IDX                   (MASTER_CPLD_ID << CPLD_IDX_SHIFT)
#define NO_CPLD_I2C_CLIENTS               (2)

#define GET_CPLD_IDX(A)             (((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT)-1)
#define STRIP_CPLD_IDX(A)                 (A & ~CPLD_IDX_MASK)

/*
 * System CPLD register definitions
 */
#define CPLD_SYSTEM_HW_REV_REG            (0 | SYSTEM_CPLD_IDX)
#define  CPLD_SYSTEM_STAGE_MASK           (0xf0)
#define  CPLD_SYSTEM_REV_MASK             (0x03)

#define CPLD_SYSTEM_SW_RESET_REG          (0x01 | SYSTEM_CPLD_IDX)

#define CPLD_PHY_INT_PWR_ENABLE1_REG      (0x02 | SYSTEM_CPLD_IDX)

#define CPLD_PHY_INT_WP_PROTECT_REG       (0x03 | SYSTEM_CPLD_IDX)

#define CPLD_POWER_STATUS_ENABLE1_REG     (0x04 | SYSTEM_CPLD_IDX)
#define  CPLD_PWR_STAT_PG_DDR_VTT         (1 << 7)
#define  CPLD_PWR_STAT_PG_PVDDR           (1 << 6)
#define  CPLD_PWR_STAT_PG_PWR_CORE        (1 << 5)
#define  CPLD_PWR_STAT_PG_V1P1            (1 << 4)
#define  CPLD_PWR_STAT_PG_V1P0            (1 << 3)
#define  CPLD_PWR_STAT_PG_3V3             (1 << 2)
#define  CPLD_PWR_STAT_PG_1V8             (1 << 1)
#define  CPLD_PWR_STAT_PG_1V35            (1 << 0)

#define CPLD_POWER_STATUS_ENABLE2_REG     (0x05 | SYSTEM_CPLD_IDX)

#define CPLD_THERMAL_SENSOR_REG           (0x06 | SYSTEM_CPLD_IDX)
#define  CPLD_THERMAL_USB_FAULT_CPU       (1 << 4)
#define  CPLD_THERMAL_USB_OC_CPU          (1 << 3)
#define  CPLD_THERMAL_VR_HOT              (1 << 2)
#define  CPLD_THERMAL_SHUTDOWN            (1 << 1)
#define  CPLD_THERMAL_ALERT               (1 << 0)

#define CPLD_WATCHDOG_REG                 (0x07 | SYSTEM_CPLD_IDX)
#define  CPLD_WATCHDOG_TIMER_MASK         (7 << 4)
#define  CPLD_WATCHDOG_ENABLE             (1 << 3)
#define  CPLD_WATCHDOG_CLEAR              (1 << 0)

#define CPLD_SYSTEM_REV_REG               (0x08 | SYSTEM_CPLD_IDX)

/*
 * Master CPLD register definitions
 */
#define CPLD_MASTER_HW_REV_REG            (0x01 | MASTER_CPLD_IDX)
#define  CPLD_MASTER_STAGE_MASK           (0xf0)
#define  CPLD_MASTER_PLATFORM_MASK        (0x0f)

#define CPLD_MASTER_SW_RESET_REG          (0x02 | MASTER_CPLD_IDX)

#define CPLD_POWER_SUPPLY_STATUS_REG      (0x03 | MASTER_CPLD_IDX)
#define  CPLD_PSU1_MASK                   (0xE0)
#define  CPLD_PSU1_PRESENT_L              (1 << 7)
#define  CPLD_PSU1_ERROR                  (1 << 6)
#define  CPLD_PSU1_GOOD                   (1 << 5)
#define  CPLD_PSU2_MASK                   (0x0E)
#define  CPLD_PSU2_PRESENT_L              (1 << 3)
#define  CPLD_PSU2_ERROR                  (1 << 2)
#define  CPLD_PSU2_GOOD                   (1 << 1)

#define CPLD_POWER_ENABLE_REG             (0x04 | MASTER_CPLD_IDX)

#define CPLD_POWER_STATUS_REG             (0x05 | MASTER_CPLD_IDX)

#define CPLD_INT_REG                      (0x06 | MASTER_CPLD_IDX)

#define CPLD_LED_CONTROL_1_REG            (0x07 | MASTER_CPLD_IDX)
#define  CPLD_SYSTEM_LED_MASK             (0x60)
#define  CPLD_SYSTEM_LED_GREEN_BLINK      (0x00)
#define  CPLD_SYSTEM_LED_GREEN            (0x20)
#define  CPLD_SYSTEM_LED_YELLOW           (0x40)
#define  CPLD_SYSTEM_LED_YELLOW_BLINK     (0x60)
#define  CPLD_BEACON_LED_MASK             (0x10)
#define  CPLD_BEACON_LED_OFF              (0x00)
#define  CPLD_BEACON_LED_BLUE_BLINK       (0x10)
#define  CPLD_POWER_LED_MASK              (0x06)
#define  CPLD_POWER_LED_OFF               (0x00)
#define  CPLD_POWER_LED_YELLOW            (0x02)
#define  CPLD_POWER_LED_GREEN             (0x04)
#define  CPLD_POWER_LED_YELLOW_BLINK      (0x06)
#define  CPLD_MASTER_LED_MASK             (0x01)
#define  CPLD_MASTER_LED_OFF              (0x01)
#define  CPLD_MASTER_LED_GREEN            (0x00)

#define CPLD_LED_CONTROL_2_REG            (0x08 | MASTER_CPLD_IDX)
#define  CPLD_FAN_TRAY_4_LED_MASK         (0xC0)
#define  CPLD_FAN_TRAY_4_LED_OFF          (0x00)
#define  CPLD_FAN_TRAY_4_LED_GREEN        (0x40)
#define  CPLD_FAN_TRAY_4_LED_YELLOW       (0x80)
#define  CPLD_FAN_TRAY_3_LED_MASK         (0x30)
#define  CPLD_FAN_TRAY_3_LED_OFF          (0x00)
#define  CPLD_FAN_TRAY_3_LED_GREEN        (0x10)
#define  CPLD_FAN_TRAY_3_LED_YELLOW       (0x20)
#define  CPLD_FAN_TRAY_2_LED_MASK         (0x0C)
#define  CPLD_FAN_TRAY_2_LED_OFF          (0x00)
#define  CPLD_FAN_TRAY_2_LED_GREEN        (0x04)
#define  CPLD_FAN_TRAY_2_LED_YELLOW       (0x08)
#define  CPLD_FAN_TRAY_1_LED_MASK         (0x03)
#define  CPLD_FAN_TRAY_1_LED_OFF          (0x00)
#define  CPLD_FAN_TRAY_1_LED_GREEN        (0x01)
#define  CPLD_FAN_TRAY_1_LED_YELLOW       (0x02)

#define CPLD_FAN_STATUS_REG               (0x09 | MASTER_CPLD_IDX)
#define  CPLD_FRONT_FAN_LED_MASK          (0x30)
#define  CPLD_FRONT_FAN_LED_OFF           (0x00)
#define  CPLD_FRONT_FAN_LED_YELLOW        (0x10)
#define  CPLD_FRONT_FAN_LED_GREEN         (0x20)
#define  CPLD_FRONT_FAN_LED_YELLOW_BLINK  (0x30)
#define  CPLD_MICRO_USB_PRESENT_L         (1 << 7)
#define  CPLD_FAN_TRAY_4_PRESENT_L        (1 << 3)
#define  CPLD_FAN_TRAY_3_PRESENT_L        (1 << 2)
#define  CPLD_FAN_TRAY_2_PRESENT_L        (1 << 1)
#define  CPLD_FAN_TRAY_1_PRESENT_L        (1 << 0)

#define CPLD_QSFP_49_54_MOD_SELECT_REG    (0x0a | MASTER_CPLD_IDX)
#define  CPLD_QSFP_PORT_54_MOD_SELECT     (1 << 5)
#define  CPLD_QSFP_PORT_53_MOD_SELECT     (1 << 4)
#define  CPLD_QSFP_PORT_52_MOD_SELECT     (1 << 3)
#define  CPLD_QSFP_PORT_51_MOD_SELECT     (1 << 2)
#define  CPLD_QSFP_PORT_50_MOD_SELECT     (1 << 1)
#define  CPLD_QSFP_PORT_49_MOD_SELECT     (1 << 0)

#define CPLD_QSFP_49_54_LP_MODE_REG       (0x0b | MASTER_CPLD_IDX)
#define  CPLD_QSFP_PORT_54_LP_MODE        (1 << 5)
#define  CPLD_QSFP_PORT_53_LP_MODE        (1 << 4)
#define  CPLD_QSFP_PORT_52_LP_MODE        (1 << 3)
#define  CPLD_QSFP_PORT_51_LP_MODE        (1 << 2)
#define  CPLD_QSFP_PORT_50_LP_MODE        (1 << 1)
#define  CPLD_QSFP_PORT_49_LP_MODE        (1 << 0)

#define CPLD_QSFP_49_54_PRESENT_REG       (0x0c | MASTER_CPLD_IDX)
#define  CPLD_QSFP_PORT_54_PRESENT_L      (1 << 5)
#define  CPLD_QSFP_PORT_53_PRESENT_L      (1 << 4)
#define  CPLD_QSFP_PORT_52_PRESENT_L      (1 << 3)
#define  CPLD_QSFP_PORT_51_PRESENT_L      (1 << 2)
#define  CPLD_QSFP_PORT_50_PRESENT_L      (1 << 1)
#define  CPLD_QSFP_PORT_49_PRESENT_L      (1 << 0)

#define CPLD_QSFP_49_54_RESET_REG         (0x0d | MASTER_CPLD_IDX)
#define  CPLD_QSFP_PORT_54_RESET_L        (1 << 5)
#define  CPLD_QSFP_PORT_53_RESET_L        (1 << 4)
#define  CPLD_QSFP_PORT_52_RESET_L        (1 << 3)
#define  CPLD_QSFP_PORT_51_RESET_L        (1 << 2)
#define  CPLD_QSFP_PORT_50_RESET_L        (1 << 1)
#define  CPLD_QSFP_PORT_49_RESET_L        (1 << 0)

#define CPLD_QSFP_49_54_INTERRUPT_REG     (0x0e | MASTER_CPLD_IDX)
#define  CPLD_QSFP_PORT_54_INTERRUPT_L    (1 << 5)
#define  CPLD_QSFP_PORT_53_INTERRUPT_L    (1 << 4)
#define  CPLD_QSFP_PORT_52_INTERRUPT_L    (1 << 3)
#define  CPLD_QSFP_PORT_51_INTERRUPT_L    (1 << 2)
#define  CPLD_QSFP_PORT_50_INTERRUPT_L    (1 << 1)
#define  CPLD_QSFP_PORT_49_INTERRUPT_L    (1 << 0)

#define CPLD_QSFP_49_54_MUX_SELECT_REG    (0x11 | MASTER_CPLD_IDX)

#define CPLD_MASTER_REV_REG               (0x12 | MASTER_CPLD_IDX)

#define CPLD_MASTER_PHY_RST1_REG          (0x13 | MASTER_CPLD_IDX)

#define CPLD_MASTER_PHY_RST2_REG          (0x14 | MASTER_CPLD_IDX)

#define CPLD_MASTER_WRITE_PROTECT_REG     (0x15 | MASTER_CPLD_IDX)

#endif  /* DELL_S4048T_CPLD_H__ */
