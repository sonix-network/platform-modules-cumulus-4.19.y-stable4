/*
 * DELL S4000 CPLD Platform Definitions
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

#ifndef DELL_S4000_CPLD_H__
#define DELL_S4000_CPLD_H__


/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 * register info from S4000_HW_SPEC_REV12_20130325.pdf
 */
#define DELL_S4000_CPLD_STRING_NAME_SIZE  30

/* the S4000 actually has three CPLDs: system, master, slave
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
#define  CPLD_THERMAL_VR_HOT              (1 << 2)
#define  CPLD_THERMAL_SHUTDOWN            (1 << 1)
#define  CPLD_THERMAL_ALERT               (1 << 0)

/*
 * With upgrade using dell-s4000-bios-cpld-update-b3.21.0.4-d1-0-0-81.bin
 * a new System CPLD register 0x09 has been introduced. By default,
 * the shutdown sequences support FTOS behavior. Setting bit 0 to 1,
 * enables feature to support Third Party ON shutdown sequences.
*/ 
#define CPLD_SYSTEM_FEATURE               (0x09 | SYSTEM_CPLD_IDX)
#define  CPLD_SYSTEM_FEATURE_SHUTDOWN     (1 << 0)

/*
 * Master CPLD register definitions
 */
#define CPLD_BOARD_REV_REG                (0x01 | MASTER_CPLD_IDX)

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
#define  CPLD_POWER_LED_MASK              (0x60)
#define  CPLD_POWER_LED_OFF               (0x00)
#define  CPLD_POWER_LED_AMBER             (0x20)
#define  CPLD_POWER_LED_AMBER_BLINK       (0x40)
#define  CPLD_POWER_LED_GREEN             (0x60)
#define  CPLD_LOCATED_LED_MASK            (0x18)
#define  CPLD_LOCATED_LED_OFF             (0x00)
#define  CPLD_LOCATED_LED_BLUE            (0x10)
#define  CPLD_LOCATED_LED_BLUE_BLINK      (0x08)
#define  CPLD_HEALTH_LED_MASK             (0x06)
#define  CPLD_HEALTH_LED_GREEN_BLINK      (0x00)
#define  CPLD_HEALTH_LED_GREEN            (0x02)
#define  CPLD_HEALTH_LED_AMBER            (0x04)
#define  CPLD_HEALTH_LED_AMBER_BLINK      (0x06)
#define  CPLD_MASTER_LED_MASK             (0x01)
#define  CPLD_MASTER_LED_OFF              (0x01)
#define  CPLD_MASTER_LED_GREEN            (0x00)

#define CPLD_LED_CONTROL_2_REG            (0x08 | MASTER_CPLD_IDX)
#define  CPLD_FAN_TRAY_1_PRESENT_L        (0x80)
#define  CPLD_FAN_TRAY_0_PRESENT_L        (0x40)
#define  CPLD_FAN_TRAY_2_LED_MASK         (0x30)
#define  CPLD_FAN_TRAY_2_LED_OFF          (0x00)
#define  CPLD_FAN_TRAY_2_LED_YELLOW       (0x20)
#define  CPLD_FAN_TRAY_2_LED_GREEN        (0x10)
#define  CPLD_FAN_TRAY_1_LED_MASK         (0x0C)
#define  CPLD_FAN_TRAY_1_LED_OFF          (0x00)
#define  CPLD_FAN_TRAY_1_LED_YELLOW       (0x08)
#define  CPLD_FAN_TRAY_1_LED_GREEN        (0x04)
#define  CPLD_FAN_TRAY_0_LED_MASK         (0x03)
#define  CPLD_FAN_TRAY_0_LED_OFF          (0x00)
#define  CPLD_FAN_TRAY_0_LED_YELLOW       (0x02)
#define  CPLD_FAN_TRAY_0_LED_GREEN        (0x01)

#define CPLD_FAN_STATUS_REG               (0x09 | MASTER_CPLD_IDX)
#define  CPLD_FRONT_FAN_LED_MASK          (0x18)
#define  CPLD_FRONT_FAN_LED_OFF           (0x00)
#define  CPLD_FRONT_FAN_LED_YELLOW        (0x08)
#define  CPLD_FRONT_FAN_LED_GREEN         (0x10)
#define  CPLD_FRONT_FAN_LED_YELLOW_BLINK  (0x18)
#define  CPLD_MICRO_USB_PRESENT_L         (1 << 1)
#define  CPLD_FAN_TRAY_2_PRESENT_L        (1 << 0)

#define CPLD_QSFP_49_54_MUX_SELECT_REG    (0x0a | MASTER_CPLD_IDX)
#define  CPLD_QSFP_PORT_54_MUX_SELECT     (1 << 5)
#define  CPLD_QSFP_PORT_53_MUX_SELECT     (1 << 4)
#define  CPLD_QSFP_PORT_52_MUX_SELECT     (1 << 3)
#define  CPLD_QSFP_PORT_51_MUX_SELECT     (1 << 2)
#define  CPLD_QSFP_PORT_50_MUX_SELECT     (1 << 1)
#define  CPLD_QSFP_PORT_49_MUX_SELECT     (1 << 0)

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

#define CPLD_SFP_1_48_MUX_SELECT_REG      (0x11 | MASTER_CPLD_IDX)

/*
 * Slave CPLD register definitions
 */
#define CPLD_SLAVE_REV_REG                (0x00 | SLAVE_CPLD_IDX)
#define  CPLD_SLAVE_REVISION_MASK         (0x0F)

#define CPLD_SFP_1_48_MUX_REG             (0x01 | SLAVE_CPLD_IDX)
#define  CPLD_PORT_1_48_MUX_MASK          (0x3F)

#define CPLD_SFP_PORT_1_8_PRESENT_REG     (0x02 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_8_PRESENT_L        (1 << 7)
#define  CPLD_SFP_PORT_7_PRESENT_L        (1 << 6)
#define  CPLD_SFP_PORT_6_PRESENT_L        (1 << 5)
#define  CPLD_SFP_PORT_5_PRESENT_L        (1 << 4)
#define  CPLD_SFP_PORT_4_PRESENT_L        (1 << 3)
#define  CPLD_SFP_PORT_3_PRESENT_L        (1 << 2)
#define  CPLD_SFP_PORT_2_PRESENT_L        (1 << 1)
#define  CPLD_SFP_PORT_1_PRESENT_L        (1 << 0)

#define CPLD_SFP_PORT_9_16_PRESENT_REG    (0x03 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_16_PRESENT_L       (1 << 7)
#define  CPLD_SFP_PORT_15_PRESENT_L       (1 << 6)
#define  CPLD_SFP_PORT_14_PRESENT_L       (1 << 5)
#define  CPLD_SFP_PORT_13_PRESENT_L       (1 << 4)
#define  CPLD_SFP_PORT_12_PRESENT_L       (1 << 3)
#define  CPLD_SFP_PORT_11_PRESENT_L       (1 << 2)
#define  CPLD_SFP_PORT_10_PRESENT_L       (1 << 1)
#define  CPLD_SFP_PORT_9_PRESENT_L        (1 << 0)

#define CPLD_SFP_PORT_17_24_PRESENT_REG   (0x04 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_24_PRESENT_L       (1 << 7)
#define  CPLD_SFP_PORT_23_PRESENT_L       (1 << 6)
#define  CPLD_SFP_PORT_22_PRESENT_L       (1 << 5)
#define  CPLD_SFP_PORT_21_PRESENT_L       (1 << 4)
#define  CPLD_SFP_PORT_20_PRESENT_L       (1 << 3)
#define  CPLD_SFP_PORT_19_PRESENT_L       (1 << 2)
#define  CPLD_SFP_PORT_18_PRESENT_L       (1 << 1)
#define  CPLD_SFP_PORT_17_PRESENT_L       (1 << 0)

#define CPLD_SFP_PORT_25_32_PRESENT_REG   (0x05 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_32_PRESENT_L       (1 << 7)
#define  CPLD_SFP_PORT_31_PRESENT_L       (1 << 6)
#define  CPLD_SFP_PORT_30_PRESENT_L       (1 << 5)
#define  CPLD_SFP_PORT_29_PRESENT_L       (1 << 4)
#define  CPLD_SFP_PORT_28_PRESENT_L       (1 << 3)
#define  CPLD_SFP_PORT_27_PRESENT_L       (1 << 2)
#define  CPLD_SFP_PORT_26_PRESENT_L       (1 << 1)
#define  CPLD_SFP_PORT_25_PRESENT_L       (1 << 0)

#define CPLD_SFP_PORT_33_40_PRESENT_REG   (0x06 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_40_PRESENT_L       (1 << 7)
#define  CPLD_SFP_PORT_39_PRESENT_L       (1 << 6)
#define  CPLD_SFP_PORT_38_PRESENT_L       (1 << 5)
#define  CPLD_SFP_PORT_37_PRESENT_L       (1 << 4)
#define  CPLD_SFP_PORT_36_PRESENT_L       (1 << 3)
#define  CPLD_SFP_PORT_35_PRESENT_L       (1 << 2)
#define  CPLD_SFP_PORT_34_PRESENT_L       (1 << 1)
#define  CPLD_SFP_PORT_33_PRESENT_L       (1 << 0)

#define CPLD_SFP_PORT_41_48_PRESENT_REG   (0x07 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_48_PRESENT_L       (1 << 7)
#define  CPLD_SFP_PORT_47_PRESENT_L       (1 << 6)
#define  CPLD_SFP_PORT_46_PRESENT_L       (1 << 5)
#define  CPLD_SFP_PORT_45_PRESENT_L       (1 << 4)
#define  CPLD_SFP_PORT_44_PRESENT_L       (1 << 3)
#define  CPLD_SFP_PORT_43_PRESENT_L       (1 << 2)
#define  CPLD_SFP_PORT_42_PRESENT_L       (1 << 1)
#define  CPLD_SFP_PORT_41_PRESENT_L       (1 << 0)

#define CPLD_SFP_PORT_1_8_TX_DISABLE_REG   (0x08 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_8_TX_DISABLE        (1 << 7)
#define  CPLD_SFP_PORT_7_TX_DISABLE        (1 << 6)
#define  CPLD_SFP_PORT_6_TX_DISABLE        (1 << 5)
#define  CPLD_SFP_PORT_5_TX_DISABLE        (1 << 4)
#define  CPLD_SFP_PORT_4_TX_DISABLE        (1 << 3)
#define  CPLD_SFP_PORT_3_TX_DISABLE        (1 << 2)
#define  CPLD_SFP_PORT_2_TX_DISABLE        (1 << 1)
#define  CPLD_SFP_PORT_1_TX_DISABLE        (1 << 0)

#define CPLD_SFP_PORT_9_16_TX_DISABLE_REG  (0x09 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_16_TX_DISABLE       (1 << 7)
#define  CPLD_SFP_PORT_15_TX_DISABLE       (1 << 6)
#define  CPLD_SFP_PORT_14_TX_DISABLE       (1 << 5)
#define  CPLD_SFP_PORT_13_TX_DISABLE       (1 << 4)
#define  CPLD_SFP_PORT_12_TX_DISABLE       (1 << 3)
#define  CPLD_SFP_PORT_11_TX_DISABLE       (1 << 2)
#define  CPLD_SFP_PORT_10_TX_DISABLE       (1 << 1)
#define  CPLD_SFP_PORT_9_TX_DISABLE        (1 << 0)

#define CPLD_SFP_PORT_17_24_TX_DISABLE_REG (0x0a | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_24_TX_DISABLE       (1 << 7)
#define  CPLD_SFP_PORT_23_TX_DISABLE       (1 << 6)
#define  CPLD_SFP_PORT_22_TX_DISABLE       (1 << 5)
#define  CPLD_SFP_PORT_21_TX_DISABLE       (1 << 4)
#define  CPLD_SFP_PORT_20_TX_DISABLE       (1 << 3)
#define  CPLD_SFP_PORT_19_TX_DISABLE       (1 << 2)
#define  CPLD_SFP_PORT_18_TX_DISABLE       (1 << 1)
#define  CPLD_SFP_PORT_17_TX_DISABLE       (1 << 0)

#define CPLD_SFP_PORT_25_32_TX_DISABLE_REG (0x0b | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_32_TX_DISABLE       (1 << 7)
#define  CPLD_SFP_PORT_31_TX_DISABLE       (1 << 6)
#define  CPLD_SFP_PORT_30_TX_DISABLE       (1 << 5)
#define  CPLD_SFP_PORT_29_TX_DISABLE       (1 << 4)
#define  CPLD_SFP_PORT_28_TX_DISABLE       (1 << 3)
#define  CPLD_SFP_PORT_27_TX_DISABLE       (1 << 2)
#define  CPLD_SFP_PORT_26_TX_DISABLE       (1 << 1)
#define  CPLD_SFP_PORT_25_TX_DISABLE       (1 << 0)

#define CPLD_SFP_PORT_33_40_TX_DISABLE_REG (0x0c | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_40_TX_DISABLE       (1 << 7)
#define  CPLD_SFP_PORT_39_TX_DISABLE       (1 << 6)
#define  CPLD_SFP_PORT_38_TX_DISABLE       (1 << 5)
#define  CPLD_SFP_PORT_37_TX_DISABLE       (1 << 4)
#define  CPLD_SFP_PORT_36_TX_DISABLE       (1 << 3)
#define  CPLD_SFP_PORT_35_TX_DISABLE       (1 << 2)
#define  CPLD_SFP_PORT_34_TX_DISABLE       (1 << 1)
#define  CPLD_SFP_PORT_33_TX_DISABLE       (1 << 0)

#define CPLD_SFP_PORT_41_48_TX_DISABLE_REG (0x0d | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_48_TX_DISABLE       (1 << 7)
#define  CPLD_SFP_PORT_47_TX_DISABLE       (1 << 6)
#define  CPLD_SFP_PORT_46_TX_DISABLE       (1 << 5)
#define  CPLD_SFP_PORT_45_TX_DISABLE       (1 << 4)
#define  CPLD_SFP_PORT_44_TX_DISABLE       (1 << 3)
#define  CPLD_SFP_PORT_43_TX_DISABLE       (1 << 2)
#define  CPLD_SFP_PORT_42_TX_DISABLE       (1 << 1)
#define  CPLD_SFP_PORT_41_TX_DISABLE       (1 << 0)

#define CPLD_SFP_PORT_1_8_RX_LOS_REG       (0x0e | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_8_RX_LOS            (1 << 7)
#define  CPLD_SFP_PORT_7_RX_LOS            (1 << 6)
#define  CPLD_SFP_PORT_6_RX_LOS            (1 << 5)
#define  CPLD_SFP_PORT_5_RX_LOS            (1 << 4)
#define  CPLD_SFP_PORT_4_RX_LOS            (1 << 3)
#define  CPLD_SFP_PORT_3_RX_LOS            (1 << 2)
#define  CPLD_SFP_PORT_2_RX_LOS            (1 << 1)
#define  CPLD_SFP_PORT_1_RX_LOS            (1 << 0)

#define CPLD_SFP_PORT_9_16_RX_LOS_REG      (0x0f | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_16_RX_LOS           (1 << 7)
#define  CPLD_SFP_PORT_15_RX_LOS           (1 << 6)
#define  CPLD_SFP_PORT_14_RX_LOS           (1 << 5)
#define  CPLD_SFP_PORT_13_RX_LOS           (1 << 4)
#define  CPLD_SFP_PORT_12_RX_LOS           (1 << 3)
#define  CPLD_SFP_PORT_11_RX_LOS           (1 << 2)
#define  CPLD_SFP_PORT_10_RX_LOS           (1 << 1)
#define  CPLD_SFP_PORT_9_RX_LOS            (1 << 0)

#define CPLD_SFP_PORT_17_24_RX_LOS_REG     (0x10 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_24_RX_LOS           (1 << 7)
#define  CPLD_SFP_PORT_23_RX_LOS           (1 << 6)
#define  CPLD_SFP_PORT_22_RX_LOS           (1 << 5)
#define  CPLD_SFP_PORT_21_RX_LOS           (1 << 4)
#define  CPLD_SFP_PORT_20_RX_LOS           (1 << 3)
#define  CPLD_SFP_PORT_19_RX_LOS           (1 << 2)
#define  CPLD_SFP_PORT_18_RX_LOS           (1 << 1)
#define  CPLD_SFP_PORT_17_RX_LOS           (1 << 0)

#define CPLD_SFP_PORT_25_32_RX_LOS_REG     (0x11 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_32_RX_LOS           (1 << 7)
#define  CPLD_SFP_PORT_31_RX_LOS           (1 << 6)
#define  CPLD_SFP_PORT_30_RX_LOS           (1 << 5)
#define  CPLD_SFP_PORT_29_RX_LOS           (1 << 4)
#define  CPLD_SFP_PORT_28_RX_LOS           (1 << 3)
#define  CPLD_SFP_PORT_27_RX_LOS           (1 << 2)
#define  CPLD_SFP_PORT_26_RX_LOS           (1 << 1)
#define  CPLD_SFP_PORT_25_RX_LOS           (1 << 0)

#define CPLD_SFP_PORT_33_40_RX_LOS_REG     (0x12 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_40_RX_LOS           (1 << 7)
#define  CPLD_SFP_PORT_39_RX_LOS           (1 << 6)
#define  CPLD_SFP_PORT_38_RX_LOS           (1 << 5)
#define  CPLD_SFP_PORT_37_RX_LOS           (1 << 4)
#define  CPLD_SFP_PORT_36_RX_LOS           (1 << 3)
#define  CPLD_SFP_PORT_35_RX_LOS           (1 << 2)
#define  CPLD_SFP_PORT_34_RX_LOS           (1 << 1)
#define  CPLD_SFP_PORT_33_RX_LOS           (1 << 0)

#define CPLD_SFP_PORT_41_48_RX_LOS_REG     (0x13 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_48_RX_LOS           (1 << 7)
#define  CPLD_SFP_PORT_47_RX_LOS           (1 << 6)
#define  CPLD_SFP_PORT_46_RX_LOS           (1 << 5)
#define  CPLD_SFP_PORT_45_RX_LOS           (1 << 4)
#define  CPLD_SFP_PORT_44_RX_LOS           (1 << 3)
#define  CPLD_SFP_PORT_43_RX_LOS           (1 << 2)
#define  CPLD_SFP_PORT_42_RX_LOS           (1 << 1)
#define  CPLD_SFP_PORT_41_RX_LOS           (1 << 0)

#define CPLD_SFP_PORT_1_8_TX_FAULT_REG     (0x14 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_8_TX_FAULT          (1 << 7)
#define  CPLD_SFP_PORT_7_TX_FAULT          (1 << 6)
#define  CPLD_SFP_PORT_6_TX_FAULT          (1 << 5)
#define  CPLD_SFP_PORT_5_TX_FAULT          (1 << 4)
#define  CPLD_SFP_PORT_4_TX_FAULT          (1 << 3)
#define  CPLD_SFP_PORT_3_TX_FAULT          (1 << 2)
#define  CPLD_SFP_PORT_2_TX_FAULT          (1 << 1)
#define  CPLD_SFP_PORT_1_TX_FAULT          (1 << 0)

#define CPLD_SFP_PORT_9_16_TX_FAULT_REG    (0x15 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_16_TX_FAULT         (1 << 7)
#define  CPLD_SFP_PORT_15_TX_FAULT         (1 << 6)
#define  CPLD_SFP_PORT_14_TX_FAULT         (1 << 5)
#define  CPLD_SFP_PORT_13_TX_FAULT         (1 << 4)
#define  CPLD_SFP_PORT_12_TX_FAULT         (1 << 3)
#define  CPLD_SFP_PORT_11_TX_FAULT         (1 << 2)
#define  CPLD_SFP_PORT_10_TX_FAULT         (1 << 1)
#define  CPLD_SFP_PORT_9_TX_FAULT          (1 << 0)

#define CPLD_SFP_PORT_17_24_TX_FAULT_REG   (0x16 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_24_TX_FAULT         (1 << 7)
#define  CPLD_SFP_PORT_23_TX_FAULT         (1 << 6)
#define  CPLD_SFP_PORT_22_TX_FAULT         (1 << 5)
#define  CPLD_SFP_PORT_21_TX_FAULT         (1 << 4)
#define  CPLD_SFP_PORT_20_TX_FAULT         (1 << 3)
#define  CPLD_SFP_PORT_19_TX_FAULT         (1 << 2)
#define  CPLD_SFP_PORT_18_TX_FAULT         (1 << 1)
#define  CPLD_SFP_PORT_17_TX_FAULT         (1 << 0)

#define CPLD_SFP_PORT_25_32_TX_FAULT_REG   (0x17 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_32_TX_FAULT         (1 << 7)
#define  CPLD_SFP_PORT_31_TX_FAULT         (1 << 6)
#define  CPLD_SFP_PORT_30_TX_FAULT         (1 << 5)
#define  CPLD_SFP_PORT_29_TX_FAULT         (1 << 4)
#define  CPLD_SFP_PORT_28_TX_FAULT         (1 << 3)
#define  CPLD_SFP_PORT_27_TX_FAULT         (1 << 2)
#define  CPLD_SFP_PORT_26_TX_FAULT         (1 << 1)
#define  CPLD_SFP_PORT_25_TX_FAULT         (1 << 0)

#define CPLD_SFP_PORT_33_40_TX_FAULT_REG   (0x18 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_40_TX_FAULT         (1 << 7)
#define  CPLD_SFP_PORT_39_TX_FAULT         (1 << 6)
#define  CPLD_SFP_PORT_38_TX_FAULT         (1 << 5)
#define  CPLD_SFP_PORT_37_TX_FAULT         (1 << 4)
#define  CPLD_SFP_PORT_36_TX_FAULT         (1 << 3)
#define  CPLD_SFP_PORT_35_TX_FAULT         (1 << 2)
#define  CPLD_SFP_PORT_34_TX_FAULT         (1 << 1)
#define  CPLD_SFP_PORT_33_TX_FAULT         (1 << 0)

#define CPLD_SFP_PORT_41_48_TX_FAULT_REG   (0x19 | SLAVE_CPLD_IDX)
#define  CPLD_SFP_PORT_48_TX_FAULT         (1 << 7)
#define  CPLD_SFP_PORT_47_TX_FAULT         (1 << 6)
#define  CPLD_SFP_PORT_46_TX_FAULT         (1 << 5)
#define  CPLD_SFP_PORT_45_TX_FAULT         (1 << 4)
#define  CPLD_SFP_PORT_44_TX_FAULT         (1 << 3)
#define  CPLD_SFP_PORT_43_TX_FAULT         (1 << 2)
#define  CPLD_SFP_PORT_42_TX_FAULT         (1 << 1)
#define  CPLD_SFP_PORT_41_TX_FAULT         (1 << 0)


#endif  // DELL_S4000_CPLD_H__
