/*
 * Accton AS5916-54XL CPLD Platform Definitions
 *
 * Copyright (C) 2019 Cumulus Networks, Inc.
 * Author: Alok Kumar <alok@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef ACCTON_AS5916_CPLD_H__
#define ACCTON_AS5916_CPLD_H__

#define SMBUS_I801_NAME     "SMBus I801 adapter"

/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 */
#define ACCTON_AS5916_CPLD_STRING_NAME_SIZE  30

/* The AS5916-54XL has three CPLDs on switch board
 *
 * FAN-cpld: controls fan speed
 * CPLD1/CPLD2: controls system and front ports
 *
 * The CPLD driver has been coded such that we have the appearance of
 * a single CPLD. We do this by embedding a two-bit index for which
 * CPLD must be accessed into each register definition. When the
 * register is accessed, the register definition is decoded into a
 * CPLD index and register offset.
 */
#define CPLD_IDX_SHIFT                    (8)
#define CPLD_IDX_MASK                     (3 << CPLD_IDX_SHIFT)
#define CPLD_FAN                          (1)
#define CPLD_1                            (2)
#define CPLD_2                            (3)
#define FAN_CPLD_IDX                      (CPLD_FAN << CPLD_IDX_SHIFT)
#define PORT_CPLD1_IDX                    (CPLD_1 << CPLD_IDX_SHIFT)
#define PORT_CPLD2_IDX                    (CPLD_2 << CPLD_IDX_SHIFT)
#define NUM_CPLD_DEVICES                  (3)

#define GET_CPLD_ID(A)                   (((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT)-1)
#define STRIP_CPLD_IDX(A)                 (A & ~CPLD_IDX_MASK)


/* Fan CPLD */
#define FAN_BOARD_INFO_REG                       (0x00 | FAN_CPLD_IDX)
#define FAN_CPLD_VERSION_REG                     (0x01 | FAN_CPLD_IDX)
#define FAN_CPLD_RESET_REG                       (0x04 | FAN_CPLD_IDX)

#define FAN_MODULE_PRESENT_REG                   (0x0f | FAN_CPLD_IDX)
#define FAN_MODULE_DIRECTION_REG                 (0x10 | FAN_CPLD_IDX)
#define FAN_MODULE_PWM_REG                       (0x11 | FAN_CPLD_IDX)

#define FAN_FRONT_FAN1_MODULE_SPEED_REG          (0x12 | FAN_CPLD_IDX)
#define FAN_FRONT_FAN2_MODULE_SPEED_REG          (0x13 | FAN_CPLD_IDX)
#define FAN_FRONT_FAN3_MODULE_SPEED_REG          (0x14 | FAN_CPLD_IDX)
#define FAN_FRONT_FAN4_MODULE_SPEED_REG          (0x15 | FAN_CPLD_IDX)
#define FAN_FRONT_FAN5_MODULE_SPEED_REG          (0x16 | FAN_CPLD_IDX)
#define FAN_FRONT_FAN6_MODULE_SPEED_REG          (0x17 | FAN_CPLD_IDX)

#define FAN_LED_REG1                             (0x1c | FAN_CPLD_IDX)
#define FAN_LED_REG2                             (0x1d | FAN_CPLD_IDX)

#define FAN_REAR_FAN1_MODULE_SPEED_REG           (0x22 | FAN_CPLD_IDX)
#define FAN_REAR_FAN2_MODULE_SPEED_REG           (0x23 | FAN_CPLD_IDX)
#define FAN_REAR_FAN3_MODULE_SPEED_REG           (0x24 | FAN_CPLD_IDX)
#define FAN_REAR_FAN4_MODULE_SPEED_REG           (0x25 | FAN_CPLD_IDX)
#define FAN_REAR_FAN5_MODULE_SPEED_REG           (0x26 | FAN_CPLD_IDX)
#define FAN_REAR_FAN6_MODULE_SPEED_REG           (0x27 | FAN_CPLD_IDX)

#define FAN_MODULE_POWER_ENABLE_REG              (0x30 | FAN_CPLD_IDX)
#define FAN_WD_TIMER_REG                         (0x31 | FAN_CPLD_IDX)
#define FAN_WD_TRIGGER_REG                       (0x32 | FAN_CPLD_IDX)
#define FAN_WD_ENABLE_REG                        (0x33 | FAN_CPLD_IDX)


/* Port CPLD1 */

#define CPLD1_BOARD_INFO_REG                     (0x00 | PORT_CPLD1_IDX)
#define CPLD1_CODE_VERSION_REG                   (0x01 | PORT_CPLD1_IDX)
#define CPLD1_POWER_MODULE_STATUS_REG            (0x02 | PORT_CPLD1_IDX)

#define CPLD1_MODULE_PRESENT_1_REG               (0x10 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_PRESENT_2_REG               (0x11 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_PRESENT_3_REG               (0x12 | PORT_CPLD1_IDX)

#define CPLD1_MODULE_TX_FAULT_1_REG              (0x14 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_TX_FAULT_2_REG              (0x16 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_TX_FAULT_3_REG              (0x18 | PORT_CPLD1_IDX)

#define CPLD1_MODULE_TX_DIS_1_REG                (0x20 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_TX_DIS_2_REG                (0x22 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_TX_DIS_3_REG                (0x24 | PORT_CPLD1_IDX)

#define CPLD1_MODULE_RX_LOSS_1_REG               (0x30 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_RX_LOSS_2_REG               (0x32 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_RX_LOSS_3_REG               (0x34 | PORT_CPLD1_IDX)

#define CPLD1_SYSTEM_RESET_5_REG                 (0x50 | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_RESET_6_REG                 (0x51 | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_RESET_7_REG                 (0x52 | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_RESET_8_REG                 (0x53 | PORT_CPLD1_IDX)

#define CPLD1_SYSTEM_LED_1_REG                   (0x65 | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_LED_2_REG                   (0x66 | PORT_CPLD1_IDX)


/* Port CPLD2 */

#define CPLD2_CODE_VERSION_REG                   (0x01 | PORT_CPLD2_IDX)
#define CPLD2_SYSTEM_RESET_REG                   (0x03 | PORT_CPLD2_IDX)

#define CPLD2_MODULE_PRESENT_4_REG               (0x10 | PORT_CPLD2_IDX)
#define CPLD2_MODULE_PRESENT_5_REG               (0x11 | PORT_CPLD2_IDX)
#define CPLD2_MODULE_PRESENT_6_REG               (0x12 | PORT_CPLD2_IDX)

#define CPLD2_MODULE_TX_FAULT_4_REG              (0x14 | PORT_CPLD2_IDX)
#define CPLD2_MODULE_TX_FAULT_5_REG              (0x16 | PORT_CPLD2_IDX)
#define CPLD2_MODULE_TX_FAULT_6_REG              (0x18 | PORT_CPLD2_IDX)

#define CPLD2_MODULE_TX_DIS_4_REG                (0x20 | PORT_CPLD2_IDX)
#define CPLD2_MODULE_TX_DIS_5_REG                (0x22 | PORT_CPLD2_IDX)
#define CPLD2_MODULE_TX_DIS_6_REG                (0x24 | PORT_CPLD2_IDX)

#define CPLD2_MODULE_RX_LOSS_4_REG               (0x30 | PORT_CPLD2_IDX)
#define CPLD2_MODULE_RX_LOSS_5_REG               (0x32 | PORT_CPLD2_IDX)
#define CPLD2_MODULE_RX_LOSS_6_REG               (0x34 | PORT_CPLD2_IDX)

#define CPLD2_QSFP_PRESENT_REG                   (0x52 | PORT_CPLD2_IDX)
#define CPLD2_QSFP_MOD_RST_REG                   (0x53 | PORT_CPLD2_IDX)
#define CPLD2_QSFP_LPMODE_REG                    (0x54 | PORT_CPLD2_IDX)


#endif  // ACCTON_AS5916_CPLD_H__
