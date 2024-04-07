/*
 * Accton AS7816_64X Platform Definitions
 *
 * Copyright (c) 2018 Cumulus Networks, Inc.  All Rights Reserved.
 * David Yen <dhyen@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:  GPL-2.0+
 *
 */

#ifndef ACCTON_AS7816_64X_H__
#define ACCTON_AS7816_64X_H__

#define SMBUS_I801_NAME     "SMBus I801 adapter"

#define NUM_PORTS                      (32)
#define NUM_PORT_MUXES                 (4)

/* The Accton AS7816_64X actually has six CPLDs: CPU CPLD, Fan CPLD, CPLD1,
 * CPLD2, CPLD3, and CPLD4.  The CPLD driver has been coded such that we have
 * appearance of a single CPLD.  We do this by embedding a three-bit index
 * into each register definition.  When a register is accessed, the register
 * definition is decoded into a CPLD index and a register offset.
 */

#define NUM_CPLD_DEVICES                (6)
#define CPU_CPLD_ID                     (0)
#define FAN_CPLD_ID                     (1)
#define PORT_CPLD1_ID                   (2)
#define PORT_CPLD2_ID                   (3)
#define PORT_CPLD3_ID                   (4)
#define PORT_CPLD4_ID                   (5)

#define CPLD_IDX_SHIFT                  (8)
#define CPLD_IDX_MASK                   (7 << CPLD_IDX_SHIFT)
#define CPU_CPLD_IDX                    (CPU_CPLD_ID << CPLD_IDX_SHIFT)
#define FAN_CPLD_IDX                    (FAN_CPLD_ID << CPLD_IDX_SHIFT)
#define PORT_CPLD1_IDX                  (PORT_CPLD1_ID << CPLD_IDX_SHIFT)
#define PORT_CPLD2_IDX                  (PORT_CPLD2_ID << CPLD_IDX_SHIFT)
#define PORT_CPLD3_IDX                  (PORT_CPLD3_ID << CPLD_IDX_SHIFT)
#define PORT_CPLD4_IDX                  (PORT_CPLD4_ID << CPLD_IDX_SHIFT)

#define GET_CPLD_ID(A)                  ((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT)
#define STRIP_CPLD_IDX(A)               (A & ~CPLD_IDX_MASK)

/* CPU CPLD */

#define CPU_BOARD_INFO_REG                       (0x00 | CPU_CPLD_IDX)
#define CPU_CPLD_VERSION_REG                     (0x01 | CPU_CPLD_IDX)

#define CPU_WD_STATUS_1_REG                      (0x02 | CPU_CPLD_IDX)
#define CPU_WD_STATUS_2_REG                      (0x03 | CPU_CPLD_IDX)

#define CPU_SYSTEM_RESET_REG                     (0x04 | CPU_CPLD_IDX)
#define CPU_LAST_RESET_REASON_REG                (0x24 | CPU_CPLD_IDX)

/* Fan CPLD */
#define FAN_BOARD_INFO_REG                       (0x00 | FAN_CPLD_IDX)
#define FAN_CPLD_VERSION_REG                     (0x01 | FAN_CPLD_IDX)
#define FAN_CPLD_RESET_REG                       (0x08 | FAN_CPLD_IDX)
#define FAN_WD_ENABLE_REG                        (0x28 | FAN_CPLD_IDX)
#define FAN_WD_TIMER_REG                         (0x29 | FAN_CPLD_IDX)
#define FAN_WD_TRIGGER_REG                       (0x2a | FAN_CPLD_IDX)

#define FAN_MODULE_PRESENT_REG                   (0x80 | FAN_CPLD_IDX)
#define FAN_MODULE_DIRECTION_REG                 (0x81 | FAN_CPLD_IDX)

#define FAN_LED_G_REG                            (0x84 | FAN_CPLD_IDX)
#define FAN_LED_R_REG                            (0x85 | FAN_CPLD_IDX)

#define FAN_MODULE_POWER_ENABLE_REG              (0x86 | FAN_CPLD_IDX)
#define FAN_MODULE_PWM_REG                       (0x87 | FAN_CPLD_IDX)
#define FAN_CLK_DIVIDER_PWM_REG                  (0x88 | FAN_CPLD_IDX)
#define FAN_CLK_DIVIDER_TACH_REG                 (0x89 | FAN_CPLD_IDX)

#define FAN_FRONT_MIN_SPEED_SET_REG              (0x8a | FAN_CPLD_IDX)
#define FAN_FRONT_MAX_SPEED_SET_REG              (0x8b | FAN_CPLD_IDX)
#define FAN_REAR_MIN_SPEED_SET_REG               (0x8c | FAN_CPLD_IDX)
#define FAN_REAR_MAX_SPEED_SET_REG               (0x8d | FAN_CPLD_IDX)

#define FAN_FRONT_FAN1_MODULE_SPEED_REG          (0x90 | FAN_CPLD_IDX)
#define FAN_FRONT_FAN2_MODULE_SPEED_REG          (0x91 | FAN_CPLD_IDX)
#define FAN_FRONT_FAN3_MODULE_SPEED_REG          (0x92 | FAN_CPLD_IDX)
#define FAN_FRONT_FAN4_MODULE_SPEED_REG          (0x93 | FAN_CPLD_IDX)
#define FAN_REAR_FAN1_MODULE_SPEED_REG           (0x98 | FAN_CPLD_IDX)
#define FAN_REAR_FAN2_MODULE_SPEED_REG           (0x99 | FAN_CPLD_IDX)
#define FAN_REAR_FAN3_MODULE_SPEED_REG           (0x9A | FAN_CPLD_IDX)
#define FAN_REAR_FAN4_MODULE_SPEED_REG           (0x9B | FAN_CPLD_IDX)

/* Port CPLD1 */

#define CPLD1_BOARD_INFO_REG                     (0x00 | PORT_CPLD1_IDX)
#define CPLD1_CODE_VERSION_REG                   (0x01 | PORT_CPLD1_IDX)
#define CPLD1_BCM56970_ROV_REG                   (0x02 | PORT_CPLD1_IDX)
#define CPLD1_POWER_MODULE_STATUS_REG            (0x03 | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_RESET_1_REG                 (0x08 | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_RESET_2_REG                 (0x09 | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_RESET_3_REG                 (0x0a | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_RESET_LOCK_REG              (0x0f | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_LED_1_REG                   (0x30 | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_LED_2_REG                   (0x31 | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_LED_3_REG                   (0x32 | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_LED_4_REG                   (0x33 | PORT_CPLD1_IDX)
#define CPLD1_SYSTEM_LED_5_REG                   (0x34 | PORT_CPLD1_IDX)

#define CPLD1_MODULE_RESET_1_REG                 (0x40 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_RESET_2_REG                 (0x41 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_RESET_3_REG                 (0x42 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_RESET_4_REG                 (0x43 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_RESET_5_REG                 (0x44 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_RESET_6_REG                 (0x45 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_RESET_7_REG                 (0x46 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_RESET_8_REG                 (0x47 | PORT_CPLD1_IDX)

#define CPLD1_MODULE_INTERRUPT_1_REG             (0x50 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_INTERRUPT_2_REG             (0x51 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_INTERRUPT_3_REG             (0x52 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_INTERRUPT_4_REG             (0x53 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_INTERRUPT_5_REG             (0x54 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_INTERRUPT_6_REG             (0x55 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_INTERRUPT_7_REG             (0x56 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_INTERRUPT_8_REG             (0x57 | PORT_CPLD1_IDX)

#define CPLD1_MODULE_PRESENT_1_REG               (0x70 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_PRESENT_2_REG               (0x71 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_PRESENT_3_REG               (0x72 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_PRESENT_4_REG               (0x73 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_PRESENT_5_REG               (0x74 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_PRESENT_6_REG               (0x75 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_PRESENT_7_REG               (0x76 | PORT_CPLD1_IDX)
#define CPLD1_MODULE_PRESENT_8_REG               (0x77 | PORT_CPLD1_IDX)

/* Port CPLD2 */

#define CPLD2_CODE_VERSION_REG                   (0x01 | PORT_CPLD2_IDX)

#define CPLD2_4X25G_RGB_LED_G_REG                (0x10 | PORT_CPLD2_IDX)
#define CPLD2_4X25G_RGB_LED_R_REG                (0x11 | PORT_CPLD2_IDX)
#define CPLD2_4X25G_RGB_LED_B_REG                (0x12 | PORT_CPLD2_IDX)

#define CPLD2_4X10G_RGB_LED_G_REG                (0x13 | PORT_CPLD2_IDX)
#define CPLD2_4X10G_RGB_LED_R_REG                (0x14 | PORT_CPLD2_IDX)
#define CPLD2_4X10G_RGB_LED_B_REG                (0x15 | PORT_CPLD2_IDX)

#define CPLD2_1X25G_RGB_LED_G_REG                (0x16 | PORT_CPLD2_IDX)
#define CPLD2_1X25G_RGB_LED_R_REG                (0x17 | PORT_CPLD2_IDX)
#define CPLD2_1X25G_RGB_LED_B_REG                (0x18 | PORT_CPLD2_IDX)

#define CPLD2_1X10G_RGB_LED_G_REG                (0x19 | PORT_CPLD2_IDX)
#define CPLD2_1X10G_RGB_LED_R_REG                (0x1a | PORT_CPLD2_IDX)
#define CPLD2_1X10G_RGB_LED_B_REG                (0x1b | PORT_CPLD2_IDX)

/* Port CPLD3 */

#define CPLD3_CODE_VERSION_REG                   (0x01 | PORT_CPLD3_IDX)

#define CPLD3_4X25G_RGB_LED_G_REG                (0x10 | PORT_CPLD3_IDX)
#define CPLD3_4X25G_RGB_LED_R_REG                (0x11 | PORT_CPLD3_IDX)
#define CPLD3_4X25G_RGB_LED_B_REG                (0x12 | PORT_CPLD3_IDX)

#define CPLD3_4X10G_RGB_LED_G_REG                (0x13 | PORT_CPLD3_IDX)
#define CPLD3_4X10G_RGB_LED_R_REG                (0x14 | PORT_CPLD3_IDX)
#define CPLD3_4X10G_RGB_LED_B_REG                (0x15 | PORT_CPLD3_IDX)

#define CPLD3_1X25G_RGB_LED_G_REG                (0x16 | PORT_CPLD3_IDX)
#define CPLD3_1X25G_RGB_LED_R_REG                (0x17 | PORT_CPLD3_IDX)
#define CPLD3_1X25G_RGB_LED_B_REG                (0x18 | PORT_CPLD3_IDX)

#define CPLD3_1X10G_RGB_LED_G_REG                (0x19 | PORT_CPLD3_IDX)
#define CPLD3_1X10G_RGB_LED_R_REG                (0x1a | PORT_CPLD3_IDX)
#define CPLD3_1X10G_RGB_LED_B_REG                (0x1b | PORT_CPLD3_IDX)

/* Port CPLD4 */

#define CPLD4_CODE_VERSION_REG                   (0x01 | PORT_CPLD4_IDX)

#define CPLD4_4X25G_RGB_LED_G_REG                (0x10 | PORT_CPLD4_IDX)
#define CPLD4_4X25G_RGB_LED_R_REG                (0x11 | PORT_CPLD4_IDX)
#define CPLD4_4X25G_RGB_LED_B_REG                (0x12 | PORT_CPLD4_IDX)

#define CPLD4_4X10G_RGB_LED_G_REG                (0x13 | PORT_CPLD4_IDX)
#define CPLD4_4X10G_RGB_LED_R_REG                (0x14 | PORT_CPLD4_IDX)
#define CPLD4_4X10G_RGB_LED_B_REG                (0x15 | PORT_CPLD4_IDX)

#define CPLD4_1X25G_RGB_LED_G_REG                (0x16 | PORT_CPLD4_IDX)
#define CPLD4_1X25G_RGB_LED_R_REG                (0x17 | PORT_CPLD4_IDX)
#define CPLD4_1X25G_RGB_LED_B_REG                (0x18 | PORT_CPLD4_IDX)

#define CPLD4_1X10G_RGB_LED_G_REG                (0x19 | PORT_CPLD4_IDX)
#define CPLD4_1X10G_RGB_LED_R_REG                (0x1a | PORT_CPLD4_IDX)
#define CPLD4_1X10G_RGB_LED_B_REG                (0x1b | PORT_CPLD4_IDX)

#endif
