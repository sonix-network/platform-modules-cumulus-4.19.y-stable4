/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Lenovo NE2572 Platform Definitions
 *
 * Copyright (c) 2018, 2019 Cumulus Networks, Inc.  All Rights Reserved.
 * David Yen <dhyen@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:  GPL-2.0+
 *
 */

#ifndef LENOVO_NE2572_H__
#define LENOVO_NE2572_H__

#define NUM_PORTS                      (32)
#define NUM_PORT_MUXES                 (4)

/* The Lenovo NE2572 actually has six CPLDs: Master, Power, System, and
 * three Port CPLDs.  The CPLD driver has been coded such that we have the
 * appearance of a single CPLD.  We do this be embedding a three-bit index
 * into each register definition.  When the register is accessed, the
 * register definition is decoded into a CPLD index and a register offset.
 */

#define NUM_CPLD_DEVICES                (6)
#define MASTER_CPLD_ID                  (0)
#define POWER_CPLD_ID                   (1)
#define SYSTEM_CPLD_ID                  (2)
#define PORT_CPLD0_ID                   (3)
#define PORT_CPLD1_ID                   (4)
#define PORT_CPLD2_ID                   (5)

#define CPLD_IDX_SHIFT                  (8)
#define CPLD_IDX_MASK                   (7 << CPLD_IDX_SHIFT)
#define MASTER_CPLD_IDX                 (MASTER_CPLD_ID << CPLD_IDX_SHIFT)
#define POWER_CPLD_IDX                  (POWER_CPLD_ID << CPLD_IDX_SHIFT)
#define SYSTEM_CPLD_IDX                 (SYSTEM_CPLD_ID << CPLD_IDX_SHIFT)
#define PORT_CPLD0_IDX                  (PORT_CPLD0_ID << CPLD_IDX_SHIFT)
#define PORT_CPLD1_IDX                  (PORT_CPLD1_ID << CPLD_IDX_SHIFT)
#define PORT_CPLD2_IDX                  (PORT_CPLD2_ID << CPLD_IDX_SHIFT)

#define GET_CPLD_ID(A)                  ((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT)
#define STRIP_CPLD_IDX(A)               (A & ~CPLD_IDX_MASK)

/* Master CPLD */

#define MASTER_CPLD_REVISION_REG                   (0x00 | MASTER_CPLD_IDX)

#define MASTER_TPM_CTRL_REG                        (0x0D | MASTER_CPLD_IDX)
#define   MASTER_CPLD_TPM_PP_MASK                  (0x04)
#define   MASTER_TPM_PRESENT_MASK                  (0x02)
#define   MASTER_TPM_RESET_MASK                    (0x01)

/* Power CPLD */

#define POWER_CPLD_REVISION_REG                    (0x00 | POWER_CPLD_IDX)

#define POWER_BOARD_TYPE_REG                       (0x01 | POWER_CPLD_IDX)
#define   POWER_BOARD_TYPE_MASK                    (0xF8)
#define   POWER_BOARD_TYPE_SHIFT                   (3)
#define   POWER_BOARD_ID_MASK                      (0x07)

#define POWER_PSU2_STATUS_REG                      (0x03 | POWER_CPLD_IDX)
#define   POWER_PSU2_PRESENT_MASK                  (0x04)
#define   POWER_PSU2_PRESENT_SHIFT                 (2)
#define   POWER_PSU2_POWER_OK_MASK                 (0x02)
#define   POWER_PSU2_POWER_OK_SHIFT                (1)
#define   POWER_PSU2_POWER_ON_MASK                 (0x01)
#define   POWER_PSU2_POWER_ON_SHIFT                (0)

#define POWER_PSU1_STATUS_REG                      (0x04 | POWER_CPLD_IDX)
#define   POWER_PSU1_PRESENT_MASK                  (0x04)
#define   POWER_PSU1_PRESENT_SHIFT                 (2)
#define   POWER_PSU1_POWER_OK_MASK                 (0x02)
#define   POWER_PSU1_POWER_OK_SHIFT                (1)
#define   POWER_PSU1_POWER_ON_MASK                 (0x01)
#define   POWER_PSU1_POWER_ON_SHIFT                (0)

#define POWER_RESET_CONTROL_REG                    (0x0A | POWER_CPLD_IDX)
#define   POWER_CPU_RESET_MASK                     (0x02)
#define   POWER_CPU_RESET_SHIFT                    (1)
#define   POWER_PCA9548_0_RESET_MASK               (0x01)

#define POWER_FAN6_STATUS_REG                      (0x10 | POWER_CPLD_IDX)
#define   POWER_FAN6_1_SPEED_ERROR_MASK            (0x08)
#define   POWER_FAN6_1_SPEED_ERROR_SHIFT           (3)
#define   POWER_FAN6_0_SPEED_ERROR_MASK            (0x04)
#define   POWER_FAN6_0_SPEED_ERROR_SHIFT           (2)
#define   POWER_FAN6_PRESENT_MASK                  (0x02)
#define   POWER_FAN6_PRESENT_SHIFT                 (1)
#define   POWER_FAN6_DIRECTION_MASK                (0x01)

#define POWER_FAN5_STATUS_REG                      (0x11 | POWER_CPLD_IDX)
#define   POWER_FAN5_1_SPEED_ERROR_MASK            (0x08)
#define   POWER_FAN5_1_SPEED_ERROR_SHIFT           (3)
#define   POWER_FAN5_0_SPEED_ERROR_MASK            (0x04)
#define   POWER_FAN5_0_SPEED_ERROR_SHIFT           (2)
#define   POWER_FAN5_PRESENT_MASK                  (0x02)
#define   POWER_FAN5_PRESENT_SHIFT                 (1)
#define   POWER_FAN5_DIRECTION_MASK                (0x01)

#define POWER_FAN4_STATUS_REG                      (0x12 | POWER_CPLD_IDX)
#define   POWER_FAN4_1_SPEED_ERROR_MASK            (0x08)
#define   POWER_FAN4_1_SPEED_ERROR_SHIFT           (3)
#define   POWER_FAN4_0_SPEED_ERROR_MASK            (0x04)
#define   POWER_FAN4_0_SPEED_ERROR_SHIFT           (2)
#define   POWER_FAN4_PRESENT_MASK                  (0x02)
#define   POWER_FAN4_PRESENT_SHIFT                 (1)
#define   POWER_FAN4_DIRECTION_MASK                (0x01)

#define POWER_FAN3_STATUS_REG                      (0x13 | POWER_CPLD_IDX)
#define   POWER_FAN3_1_SPEED_ERROR_MASK            (0x08)
#define   POWER_FAN3_1_SPEED_ERROR_SHIFT           (3)
#define   POWER_FAN3_0_SPEED_ERROR_MASK            (0x04)
#define   POWER_FAN3_0_SPEED_ERROR_SHIFT           (2)
#define   POWER_FAN3_PRESENT_MASK                  (0x02)
#define   POWER_FAN3_PRESENT_SHIFT                 (1)
#define   POWER_FAN3_DIRECTION_MASK                (0x01)

#define POWER_FAN2_STATUS_REG                      (0x14 | POWER_CPLD_IDX)
#define   POWER_FAN2_1_SPEED_ERROR_MASK            (0x08)
#define   POWER_FAN2_1_SPEED_ERROR_SHIFT           (3)
#define   POWER_FAN2_0_SPEED_ERROR_MASK            (0x04)
#define   POWER_FAN2_0_SPEED_ERROR_SHIFT           (2)
#define   POWER_FAN2_PRESENT_MASK                  (0x02)
#define   POWER_FAN2_PRESENT_SHIFT                 (1)
#define   POWER_FAN2_DIRECTION_MASK                (0x01)

#define POWER_FAN1_STATUS_REG                      (0x15 | POWER_CPLD_IDX)
#define   POWER_FAN1_1_SPEED_ERROR_MASK            (0x08)
#define   POWER_FAN1_1_SPEED_ERROR_SHIFT           (3)
#define   POWER_FAN1_0_SPEED_ERROR_MASK            (0x04)
#define   POWER_FAN1_0_SPEED_ERROR_SHIFT           (2)
#define   POWER_FAN1_PRESENT_MASK                  (0x02)
#define   POWER_FAN1_PRESENT_SHIFT                 (1)
#define   POWER_FAN1_DIRECTION_MASK                (0x01)

#define POWER_FAN6_0_SPEED_REG                     (0x16 | POWER_CPLD_IDX)
#define POWER_FAN6_1_SPEED_REG                     (0x17 | POWER_CPLD_IDX)

#define POWER_FAN5_0_SPEED_REG                     (0x18 | POWER_CPLD_IDX)
#define POWER_FAN5_1_SPEED_REG                     (0x19 | POWER_CPLD_IDX)

#define POWER_FAN4_0_SPEED_REG                     (0x1A | POWER_CPLD_IDX)
#define POWER_FAN4_1_SPEED_REG                     (0x1B | POWER_CPLD_IDX)

#define POWER_FAN3_0_SPEED_REG                     (0x1C | POWER_CPLD_IDX)
#define POWER_FAN3_1_SPEED_REG                     (0x1D | POWER_CPLD_IDX)

#define POWER_FAN2_0_SPEED_REG                     (0x1E | POWER_CPLD_IDX)
#define POWER_FAN2_1_SPEED_REG                     (0x1F | POWER_CPLD_IDX)

#define POWER_FAN1_0_SPEED_REG                     (0x20 | POWER_CPLD_IDX)
#define POWER_FAN1_1_SPEED_REG                     (0x21 | POWER_CPLD_IDX)

#define POWER_FAN_MIN_SPEED_REG                    (0x22 | POWER_CPLD_IDX)

#define POWER_FAN_PWM_REG                          (0x23 | POWER_CPLD_IDX)
#define   POWER_FAN_PWM_OFF                        (0x00)
#define   POWER_FAN_PWM_30                         (0x4B)
#define   POWER_FAN_PWM_50                         (0x80)
#define   POWER_FAN_PWM_100                        (0xFF)

/* System CPLD */

#define SYSTEM_CPLD_REVISION_REG                   (0x00 | SYSTEM_CPLD_IDX)

#define SYSTEM_RESET_0_REG                         (0x01 | SYSTEM_CPLD_IDX)
#define   SYSTEM_PCA9548_3_9_RESET_MASK            (0x40)
#define   SYSTEM_PCA9548_3_9_RESET_SHIFT           (6)
#define   SYSTEM_BCM56967_RESET_MASK               (0x20)
#define   SYSTEM_BCM56967_RESET_SHIFT              (5)
#define   SYSTEM_BCM53115M_RESET_MASK              (0x10)
#define   SYSTEM_BCM53115M_RESET_SHIFT             (4)
#define   SYSTEM_88E1112_RESET_MASK                (0x08)
#define   SYSTEM_88E1112_RESET_SHIFT               (3)
#define   SYSTEM_PCA9548_2_RESET_MASK              (0x04)
#define   SYSTEM_PCA9548_2_RESET_SHIFT             (2)
#define   SYSTEM_PCA9548_1_RESET_MASK              (0x02)
#define   SYSTEM_PCA9548_1_RESET_SHIFT             (1)
#define   SYSTEM_PCA9545_0_RESET_MASK              (0x01)

#define SYSTEM_RESET_1_REG                         (0x02 | SYSTEM_CPLD_IDX)
#define   SYSTEM_BCM82831_RESET_MASK               (0x08)
#define   SYSTEM_BCM82831_RESET_SHIFT              (3)
#define   SYSTEM_PORT_CPLD2_RESET_MASK             (0x04)
#define   SYSTEM_PORT_CPLD2_RESET_SHIFT            (2)
#define   SYSTEM_PORT_CPLD1_RESET_MASK             (0x02)
#define   SYSTEM_PORT_CPLD1_RESET_SHIFT            (1)
#define   SYSTEM_PORT_CPLD0_RESET_MASK             (0x18)

#define SYSTEM_SYSTEM_LED_REG                      (0x08 | SYSTEM_CPLD_IDX)
#define   SYSTEM_STACKING_LED_COLOR_MASK           (0x40)
#define   SYSTEM_STACKING_LED_AMBER                (0x00)
#define   SYSTEM_STACKING_LED_GREEN                (0x40)
#define   SYSTEM_STACKING_LED_COLOR_SHIFT          (6)
#define   SYSTEM_STACKING_LED_MASK                 (0x30)
#define   SYSTEM_STACKING_LED_OFF                  (0x00)
#define   SYSTEM_STACKING_LED_ON                   (0x10)
#define   SYSTEM_STACKING_LED_SLOW_BLINKING        (0x20)
#define   SYSTEM_STACKING_LED_FAST_BLINKING        (0x30)
#define   SYSTEM_STACKING_LED_SHIFT                (4)
#define   SYSTEM_PSU_LED_COLOR_MASK                (0x04)
#define   SYSTEM_PSU_LED_AMBER                     (0x00)
#define   SYSTEM_PSU_LED_GREEN                     (0x04)
#define   SYSTEM_PSU_LED_COLOR_SHIFT               (2)
#define   SYSTEM_PSU_LED_MASK                      (0x03)
#define   SYSTEM_PSU_LED_OFF                       (0x00)
#define   SYSTEM_PSU_LED_ON                        (0x01)
#define   SYSTEM_PSU_LED_SLOW_BLINKING             (0x02)
#define   SYSTEM_PSU_LED_FAST_BLINKING             (0x03)

#define SYSTEM_FAN_0_LED_REG                       (0x09 | SYSTEM_CPLD_IDX)
#define   SYSTEM_FAN_LED_COLOR_MASK                (0x04)
#define   SYSTEM_FAN_LED_AMBER                     (0x00)
#define   SYSTEM_FAN_LED_GREEN                     (0x04)
#define   SYSTEM_FAN_LED_COLOR_SHIFT               (2)
#define   SYSTEM_FAN_LED_MASK                      (0x03)
#define   SYSTEM_FAN_LED_OFF                       (0x00)
#define   SYSTEM_FAN_LED_ON                        (0x01)
#define   SYSTEM_FAN_LED_SLOW_BLINKING             (0x02)
#define   SYSTEM_FAN_LED_FAST_BLINKING             (0x03)

#define SYSTEM_FAN_REAR_0_LED_REG                  (0x0E | SYSTEM_CPLD_IDX)
#define   SYSTEM_FAN2_REAR_LED_MASK                (0x30)
#define   SYSTEM_FAN2_REAR_LED_OFF                 (0x00)
#define   SYSTEM_FAN2_REAR_LED_ON                  (0x10)
#define   SYSTEM_FAN2_REAR_LED_SLOW_BLINKING       (0x20)
#define   SYSTEM_FAN2_REAR_LED_FAST_BLANKING       (0x30)
#define   SYSTEM_FAN2_REAR_LED_SHIFT               (4)
#define   SYSTEM_FAN1_REAR_LED_MASK                (0x03)
#define   SYSTEM_FAN1_REAR_LED_OFF                 (0x00)
#define   SYSTEM_FAN1_REAR_LED_ON                  (0x01)
#define   SYSTEM_FAN1_REAR_LED_SLOW_BLINKING       (0x02)
#define   SYSTEM_FAN1_REAR_LED_FAST_BLANKING       (0x03)

#define SYSTEM_FAN_REAR_1_LED_REG                  (0x0F | SYSTEM_CPLD_IDX)
#define   SYSTEM_FAN4_REAR_LED_MASK                (0x30)
#define   SYSTEM_FAN4_REAR_LED_OFF                 (0x00)
#define   SYSTEM_FAN4_REAR_LED_ON                  (0x10)
#define   SYSTEM_FAN4_REAR_LED_SLOW_BLINKING       (0x20)
#define   SYSTEM_FAN4_REAR_LED_FAST_BLANKING       (0x30)
#define   SYSTEM_FAN4_REAR_LED_SHIFT               (4)
#define   SYSTEM_FAN3_REAR_LED_MASK                (0x03)
#define   SYSTEM_FAN3_REAR_LED_OFF                 (0x00)
#define   SYSTEM_FAN3_REAR_LED_ON                  (0x01)
#define   SYSTEM_FAN3_REAR_LED_SLOW_BLINKING       (0x02)
#define   SYSTEM_FAN3_REAR_LED_FAST_BLANKING       (0x03)

#define SYSTEM_FAN_REAR_2_LED_REG                  (0x10 | SYSTEM_CPLD_IDX)
#define   SYSTEM_FAN6_REAR_LED_MASK                (0x30)
#define   SYSTEM_FAN6_REAR_LED_OFF                 (0x00)
#define   SYSTEM_FAN6_REAR_LED_ON                  (0x10)
#define   SYSTEM_FAN6_REAR_LED_SLOW_BLINKING       (0x20)
#define   SYSTEM_FAN6_REAR_LED_FAST_BLANKING       (0x30)
#define   SYSTEM_FAN6_REAR_LED_SHIFT               (4)
#define   SYSTEM_FAN5_REAR_LED_MASK                (0x03)
#define   SYSTEM_FAN5_REAR_LED_OFF                 (0x00)
#define   SYSTEM_FAN5_REAR_LED_ON                  (0x01)
#define   SYSTEM_FAN5_REAR_LED_SLOW_BLINKING       (0x02)
#define   SYSTEM_FAN5_REAR_LED_FAST_BLANKING       (0x03)

#define SYSTEM_SERVICE_BLUE_LED_REG                (0x11 | SYSTEM_CPLD_IDX)
#define   SYSTEM_SERVICE_BLUE_LED_MASK             (0x03)
#define   SYSTEM_SERVICE_BLUE_LED_OFF              (0x00)
#define   SYSTEM_SERVICE_BLUE_LED_ON               (0x01)
#define   SYSTEM_SERVICE_BLUE_LED_SLOW_BLINKING    (0x02)
#define   SYSTEM_SERVICE_BLUE_LED_FAST_BLANKING    (0x03)

#define SYSTEM_RESET_BUTTON_REG                    (0x12 | SYSTEM_CPLD_IDX)
#define   SYSTEM_RESET_BUTTON_MASK                 (0x03)
#define   SYSTEM_RESET_BUTTON_NOT_PRESSED          (0x00)
#define   SYSTEM_RESET_BUTTON_RESERVED             (0x01)
#define   SYSTEM_RESET_BUTTON_PRESSED_AND_RELEASED (0x02)
#define   SYSTEM_RESET_BUTTON_PRESSED_AND_HELD     (0x03)

/* Port CPLDs */

#define PORT_SFP28_1_24_SELECT_RANGE               (24)
#define PORT_SFP28_CPLD0_REVISION_REG              (0x00 | PORT_CPLD0_IDX)
#define PORT_SFP28_1_8_TX_FAULT_REG                (0x10 | PORT_CPLD0_IDX)
#define PORT_SFP28_9_16_TX_FAULT_REG               (0x11 | PORT_CPLD0_IDX)
#define PORT_SFP28_17_24_TX_FAULT_REG              (0x12 | PORT_CPLD0_IDX)
#define PORT_SFP28_1_8_RX_LOS_REG                  (0x13 | PORT_CPLD0_IDX)
#define PORT_SFP28_9_16_RX_LOS_REG                 (0x14 | PORT_CPLD0_IDX)
#define PORT_SFP28_17_24_RX_LOS_REG                (0x15 | PORT_CPLD0_IDX)
#define PORT_SFP28_1_8_PRESENT_REG                 (0x16 | PORT_CPLD0_IDX)
#define PORT_SFP28_9_16_PRESENT_REG                (0x17 | PORT_CPLD0_IDX)
#define PORT_SFP28_17_24_PRESENT_REG               (0x18 | PORT_CPLD0_IDX)
#define PORT_SFP28_1_8_TX_DISABLE_REG              (0x19 | PORT_CPLD0_IDX)
#define PORT_SFP28_9_16_TX_DISABLE_REG             (0x1A | PORT_CPLD0_IDX)
#define PORT_SFP28_17_24_TX_DISABLE_REG            (0x1B | PORT_CPLD0_IDX)

#define PORT_SFP28_25_48_SELECT_RANGE              (48)
#define PORT_SFP28_CPLD1_REVISION_REG              (0x00 | PORT_CPLD1_IDX)
#define PORT_SFP28_25_32_TX_FAULT_REG              (0x10 | PORT_CPLD1_IDX)
#define PORT_SFP28_33_40_TX_FAULT_REG              (0x11 | PORT_CPLD1_IDX)
#define PORT_SFP28_41_48_TX_FAULT_REG              (0x12 | PORT_CPLD1_IDX)
#define PORT_SFP28_25_32_RX_LOS_REG                (0x13 | PORT_CPLD1_IDX)
#define PORT_SFP28_33_40_RX_LOS_REG                (0x14 | PORT_CPLD1_IDX)
#define PORT_SFP28_41_48_RX_LOS_REG                (0x15 | PORT_CPLD1_IDX)
#define PORT_SFP28_25_32_PRESENT_REG               (0x16 | PORT_CPLD1_IDX)
#define PORT_SFP28_33_40_PRESENT_REG               (0x17 | PORT_CPLD1_IDX)
#define PORT_SFP28_41_48_PRESENT_REG               (0x18 | PORT_CPLD1_IDX)
#define PORT_SFP28_25_32_TX_DISABLE_REG            (0x19 | PORT_CPLD1_IDX)
#define PORT_SFP28_33_40_TX_DISABLE_REG            (0x1A | PORT_CPLD1_IDX)
#define PORT_SFP28_41_48_TX_DISABLE_REG            (0x1B | PORT_CPLD1_IDX)

#define PORT_QSFP28_49_54_SELECT_RANGE             (54)
#define PORT_QSFP28_CPLD_REVISION_REG              (0x00 | PORT_CPLD2_IDX)
#define PORT_QSFP28_49_54_INTERRUPT_REG            (0x01 | PORT_CPLD2_IDX)
#define PORT_QSFP28_49_54_PRESENT_REG              (0x05 | PORT_CPLD2_IDX)
#define PORT_QSFP28_49_54_RESET_REG                (0x07 | PORT_CPLD2_IDX)
#define PORT_QSFP28_49_54_LPMODE_REG               (0x09 | PORT_CPLD2_IDX)
#define PORT_QSFP28_49_54_MODSEL_REG               (0x0B | PORT_CPLD2_IDX)

#endif
