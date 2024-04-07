/*
 * DELL S6000 CPLD Platform Definitions
 *
 * Alan Liebthal <alanl@cumulusnetworks.com>
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

#ifndef DELL_S6000_CPLD_H__
#define DELL_S6000_CPLD_H__


/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 * register info from S6000_HW_SPEC_REV12_20130325.pdf
 */
#define DELL_S6000_CPLD_STRING_NAME_SIZE  30

/* the s6000 actually has three CPLDs: system, master, slave
 * the CPLD driver has been coded such that we have the appearance of a single
 * CPLD. We do this by embedding a two-bit index for which CPLD must be accessed into
 * each register definition. When the register is accessed, the register definition
 * is decoded into a CPLD index and register offset.
 */
#define CPLD_IDX_SHIFT                    (6)
#define SYSTEM_CPLD_ID                    (1)
#define MASTER_CPLD_ID                    (2)
#define SLAVE_CPLD_ID                     (3)
#define CPLD_IDX_MASK                     (3 << CPLD_IDX_SHIFT)
#define SYSTEM_CPLD_IDX                   (SYSTEM_CPLD_ID << CPLD_IDX_SHIFT)
#define MASTER_CPLD_IDX                   (MASTER_CPLD_ID << CPLD_IDX_SHIFT)
#define SLAVE_CPLD_IDX                    (SLAVE_CPLD_ID << CPLD_IDX_SHIFT)
#define NUM_CPLD_I2C_CLIENTS              (3)

#define GET_CPLD_IDX(A)                   (((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT)-1)
#define STRIP_CPLD_IDX(A)                 (A & ~CPLD_IDX_MASK)

/*
 * System CPLD register definitions
 */
#define CPLD_HW_REV_REG                   (0 | SYSTEM_CPLD_IDX)
#define  CPLD_HW_REV_BOARD_STAGE_MASK     (0xf0)
#define  CPLD_HW_REV_CPLD_REV_MASK        (0x0f)

#define CPLD_SYSTEM_SW_RESET_REG          (0x01 | SYSTEM_CPLD_IDX)

#define CPLD_PHY_INT_PWR_ENABLE1_REG      (0x02 | SYSTEM_CPLD_IDX)

#define CPLD_PHY_INT_PWR_ENABLE2_REG      (0x03 | SYSTEM_CPLD_IDX)

#define CPLD_POWER_STATUS_ENABLE1_REG     (0x04 | SYSTEM_CPLD_IDX)
#define  CPLD_PWR_STAT_VCC3V3             (1 << 6)
#define  CPLD_PWR_STAT_P1V05              (1 << 5)
#define  CPLD_PWR_STAT_PVDDR              (1 << 4)
#define  CPLD_PWR_STAT_P1V5               (1 << 3)

#define CPLD_POWER_STATUS_ENABLE2_REG     (0x05 | SYSTEM_CPLD_IDX)
#define  CPLD_PWR_STAT_MB                 (1 << 3)
#define  CPLD_PWR_STAT_PVCCP              (1 << 2)
#define  CPLD_PWR_STAT_VCC1V05            (1 << 1)

#define CPLD_THERMAL_SENSOR_REG           (0x06 | SYSTEM_CPLD_IDX)
#define  CPLD_THERMAL_SHUTDOWN            (1 << 1)
#define  CPLD_THERMAL_ALERT               (1 << 0)

/*
 * Master CPLD register definitions
 */
#define CPLD_BOARD_REV_REG                (0x01 | MASTER_CPLD_IDX)

#define CPLD_MASTER_SW_RESET_REG          (0x02 | MASTER_CPLD_IDX)

#define CPLD_POWER_SUPPLY_STATUS_REG      (0x03 | MASTER_CPLD_IDX)
#define  CPLD_PSU0_MASK                   (0xE0)
#define  CPLD_PSU0_PRESENT_L              (1 << 7)
#define  CPLD_PSU0_ERROR                  (1 << 6)
#define  CPLD_PSU0_GOOD                   (1 << 5)
#define  CPLD_PSU1_MASK                   (0x0E)
#define  CPLD_PSU1_PRESENT_L              (1 << 3)
#define  CPLD_PSU1_ERROR                  (1 << 2)
#define  CPLD_PSU1_GOOD                   (1 << 1)

#define CPLD_POWER_ENABLE_REG             (0x04 | MASTER_CPLD_IDX)

#define CPLD_POWER_STATUS_REG             (0x05 | MASTER_CPLD_IDX)

#define CPLD_INT_REG                      (0x06 | MASTER_CPLD_IDX)

#define CPLD_LED_CONTROL_REG              (0x07 | MASTER_CPLD_IDX)
#define  CPLD_SYSTEM_LED_MASK             (0x60)
#define  CPLD_SYSTEM_LED_GREEN_BLINK      (0x00)
#define  CPLD_SYSTEM_LED_GREEN            (0x20)
#define  CPLD_SYSTEM_LED_YELLOW           (0x40)
#define  CPLD_SYSTEM_LED_YELLOW_BLINK     (0x60)
#define  CPLD_POWER_LED_MASK              (0x06)
#define  CPLD_POWER_LED_OFF               (0x00)
#define  CPLD_POWER_LED_YELLOW            (0x02)
#define  CPLD_POWER_LED_GREEN             (0x04)
#define  CPLD_POWER_LED_YELLOW_BLINK      (0x06)
#define  CPLD_MASTER_LED_MASK             (0x01)
#define  CPLD_MASTER_LED_OFF              (0x01)
#define  CPLD_MASTER_LED_GREEN            (0x00)

#define CPLD_FAN1_STATUS_REG              (0x08 | MASTER_CPLD_IDX)
#define  CPLD_FAN_TRAY_1_PRESENT_L        (1 << 7)
#define  CPLD_FAN_TRAY_0_PRESENT_L        (1 << 6)
#define  CPLD_FAN_TRAY_2_LED_MASK         (0x30)
#define  CPLD_FAN_TRAY_2_LED_OFF          (0x00)
#define  CPLD_FAN_TRAY_2_LED_GREEN        (0x10)
#define  CPLD_FAN_TRAY_2_LED_YELLOW       (0x20)
#define  CPLD_FAN_TRAY_1_LED_MASK         (0x0C)
#define  CPLD_FAN_TRAY_1_LED_OFF          (0x00)
#define  CPLD_FAN_TRAY_1_LED_GREEN        (0x04)
#define  CPLD_FAN_TRAY_1_LED_YELLOW       (0x08)
#define  CPLD_FAN_TRAY_0_LED_MASK         (0x03)
#define  CPLD_FAN_TRAY_0_LED_OFF          (0x00)
#define  CPLD_FAN_TRAY_0_LED_GREEN        (0x01)
#define  CPLD_FAN_TRAY_0_LED_YELLOW       (0x02)

#define CPLD_FAN2_STATUS_REG              (0x09 | MASTER_CPLD_IDX)
#define  CPLD_FRONT_FAN_LED_MASK          (0x18)
#define  CPLD_FRONT_FAN_LED_OFF           (0x00)
#define  CPLD_FRONT_FAN_LED_GREEN         (0x10)
#define  CPLD_FRONT_FAN_LED_YELLOW        (0x08)
#define  CPLD_FRONT_FAN_LED_YELLOW_BLINK  (0x18)
#define  CPLD_FAN_TRAY_2_PRESENT_L        (0x01)

#define CPLD_QSFP_17_24_MOD_MODE_CTL_REG  (0x0a | MASTER_CPLD_IDX)
#define CPLD_QSFP_25_32_MOD_MODE_CTL_REG  (0x0b | MASTER_CPLD_IDX)
#define CPLD_QSFP_17_24_LP_MODE_CTL_REG   (0x0c | MASTER_CPLD_IDX)
#define CPLD_QSFP_25_32_LP_MODE_CTL_REG   (0x0d | MASTER_CPLD_IDX)
#define CPLD_QSFP_17_24_PRESENCE_CTL_REG  (0x0e | MASTER_CPLD_IDX)
#define CPLD_QSFP_25_32_PRESENCE_CTL_REG  (0x0f | MASTER_CPLD_IDX)
#define CPLD_QSFP_17_24_RESET_CTL_REG     (0x10 | MASTER_CPLD_IDX)
#define CPLD_QSFP_25_32_RESET_CTL_REG     (0x11 | MASTER_CPLD_IDX)
#define CPLD_QSFP_17_24_INT_CTL_REG       (0x12 | MASTER_CPLD_IDX)
#define CPLD_QSFP_25_32_INT_CTL_REG       (0x13 | MASTER_CPLD_IDX)
#define CPLD_7_DIGIT_STACK_REG            (0x14 | MASTER_CPLD_IDX)
#define CPLD_QSFP_STATUS_CHANGE_REG       (0x15 | MASTER_CPLD_IDX)
#define CPLD_DEVICE_SHUTDOWN_REG          (0x16 | MASTER_CPLD_IDX)

/*
 * Slave CPLD register definitions
 */
#define CPLD_QSFP_1_8_MOD_MODE_CTL_REG    (0x00 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_9_16_MOD_MODE_CTL_REG   (0x01 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_1_8_LP_MODE_CTL_REG     (0x02 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_9_16_LP_MODE_CTL_REG    (0x03 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_1_8_PRESENCE_CTL_REG    (0x04 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_9_16_PRESENCE_CTL_REG   (0x05 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_1_8_RESET_CTL_REG       (0x06 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_9_16_RESET_CTL_REG      (0x07 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_1_8_INT_CTL_REG         (0x08 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_9_16_INT_CTL_REG        (0x09 | SLAVE_CPLD_IDX)
#define CPLD_QSFP_1_16_STATUS_CHANGE_CTL_REG (0x0a | SLAVE_CPLD_IDX)

#endif  // DELL_S6000_CPLD_H__
