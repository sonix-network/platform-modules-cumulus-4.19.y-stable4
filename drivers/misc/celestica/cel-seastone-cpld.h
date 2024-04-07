/*
 * Celestica Seastone CPLD Platform Definitions
 *
 * Copyright (c) 2015 Cumulus Networks, Inc.  All rights reserved.
 * David Yen <dhyen@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef CEL_SEASTONE_H__
#define CEL_SEASTONE_H__

/*-----------------------------------------------------------------------------
 *
 * register info from R0872-M0006-01 Rev0.1 Seastone CPLD Secification.pdf
 *
 */


// CPLD 1

#define CPLD1_REG_VERSION_OFFSET               (0x100)  /* CPLD Version */
#  define CPLD_VERSION_H_MASK                  (0xF0)
#  define CPLD_VERSION_H_SHIFT                 (4)
#    define CPLD_VERSION_ID                    (0x10)
#  define CPLD_VERSION_L_MASK                  (0x0F)
#  define CPLD_VERSION_L_SHIFT                 (0)

#define CPLD1_REG_SW_SCRATCH_OFFSET            (0x101)

#define CPLD1_REG_RESET_CTRL_OFFSET            (0x102)  /* Reset Control */
#  define CPLD1_RESET_CPLD4_L                  (1 << 3)
#  define CPLD1_RESET_WATCH_DOG_MASK_L         (1 << 2)
#  define CPLD1_RESET_BUTTON_HARD_L            (1 << 1)
#  define CPLD1_POWER_CYCLE_SOFT_L             (1 << 0)

#define CPLD1_REG_RESET_SRC_OFFSET             (0x103)  /* Reset Source */
#  define CPLD1_REG_RESET_SRC_MASK             (0x0F)
#    define CPLD1_RESET_PWR_ON                 (1 << 3) /* Power On Reset */
#    define CPLD1_RESET_WATCH_DOG              (1 << 2) /* Watchdog Reset */
#    define CPLD1_RESET_CPU_HARD               (1 << 1) /* SW-set hard Reset */
#    define CPLD1_RESET_CPU_SOFT               (1 << 0) /* CPU request Reset */

#define CPLD1_REG_BOARD_TYPE_OFFSET            (0x104)  /* Board Type */
#  define CPLD1_BOARD_TYPE_REDXP_VAL           (1)
#  define CPLD1_BOARD_TYPE_SMALLXP_VAL         (2)
#  define CPLD1_BOARD_TYPE_SEA_VAL             (3)

#define CPLD1_REG_INT_STATUS_OFFSET            (0x110)
#  define CPLD1_GPIO_CPU_SUS0_L                (1 << 0)

#define CPLD1_REG_INT0_SRC_STATUS_OFFSET       (0x111)
#  define CPLD1_INT_STAT_RESET_BTN_L           (1 << 1)
#  define CPLD1_INT_STAT_GEPHY_BACK_L          (1 << 0)

#define CPLD1_REG_INT0_SRC_INT_OFFSET          (0x112)
#  define CPLD1_INT_RESET_BTN_L                (1 << 1)
#  define CPLD1_INT_GEPHY_BACK_L               (1 << 0)

#define CPLD1_REG_INT0_SRC_MASK_OFFSET         (0x113)
#  define CPLD1_INT_MASK_RESET_BTN_L           (1 << 1)
#  define CPLD1_INT_MASK_GEPHY_BACK_L          (1 << 0)

#define CPLD1_REG_PWR_STATUS_OFFSET            (0x120)
#  define CPLD1_PWR_STAT_PSU2_ALERT_L          (1 << 1)
#  define CPLD1_PWR_STAT_PSU1_ALERT_L          (1 << 0)

#define CPLD1_REG_PWR_GOOD_OFFSET              (0x121)
#  define CPLD1_PWR_GOOD_XPOR75V               (1 << 7)
#  define CPLD1_PWR_GOOD_VCC_VNN_CP            (1 << 6)
#  define CPLD1_PWR_GOOD_VDDR_CP               (1 << 5)
#  define CPLD1_PWR_GOOD_XP3R3V_STD            (1 << 4)
#  define CPLD1_PWR_GOOD_XP3R3V_EARLY          (1 << 3)
#  define CPLD1_PWR_GOOD_XP1R0V_CP             (1 << 2)
#  define CPLD1_PWR_GOOD_XP1R07V_CP            (1 << 1)
#  define CPLD1_PWR_GOOD_XP3R3V_CP             (1 << 0)

#define CPLD1_REG_7_PANEL_LED_OFFSET           (0x122)
#  define CPLD1_7_PANEL_DP                     (1 << 7)
#  define CPLD1_7_PANEL_G                      (1 << 6)
#  define CPLD1_7_PANEL_F                      (1 << 5)
#  define CPLD1_7_PANEL_E                      (1 << 4)
#  define CPLD1_7_PANEL_D                      (1 << 3)
#  define CPLD1_7_PANEL_C                      (1 << 2)
#  define CPLD1_7_PANEL_B                      (1 << 1)
#  define CPLD1_7_PANEL_A                      (1 << 0)

#define CPLD1_REG_WRITE_PROTECT_CTL_OFFSET     (0x123)
#  define CPLD1_WRITE_PROTECT_SPI1_L           (1 << 5)
#  define CPLD1_WRITE_PROTECT_SPI0_L           (1 << 4)
#  define CPLD1_WRITE_PROTECT_SPD2             (1 << 3)
#  define CPLD1_WRITE_PROTECT_SPD1             (1 << 2)
#  define CPLD1_WRITE_PROTECT_NAND_L           (1 << 1)
#  define CPLD1_WRITE_PROTECT_SYS_EEPROM       (1 << 0)

#define CPLD1_REG_MISC_STAT_CTL_OFFSET         (0x124)

#define CPLD1_REG_INFO_RAM_ADDR_HIGH           (0x130)

#define CPLD1_REG_INFO_RAM_ADDR_LOW            (0x131)

#define CPLD1_REG_INFO_RAM_READ_DATA           (0x132)

#define CPLD1_REG_INFO_RAM_WRITE_DATA          (0x133)


// CPLD 2

#define CPLD2_REG_VERSION_OFFSET               (0x200)

#define CPLD2_REG_SW_SCRATCH_OFFSET            (0x201)

#define CPLD2_REG_I2C_PORT_ID_OFFSET           (0x210)
#  define CPLD2_I2C_BAUD_RATE                  (1 << 6) /* 0=50KHz, 1=100KHz */
#  define CPLD2_I2C_OPCODE_ID_MASK             (0x1F)

#define CPLD2_REG_I2C_OPCODE_OFFSET            (0x211)
#  define CPLD2_I2C_DATA_LEN_MASK              (0xF0)
#  define CPLD2_I2C_DATA_LEN_SHIFT             (4)
#  define CPLD2_I2C_CMD_LEN_MASK               (0x03)
#    define CPLD2_I2C_CMD_CUR_ADDR             (0x00)

#define CPLD2_REG_DEV_ADDR_OFFSET              (0x212)
#  define CPLD2_I2C_SLAVE_ADDR_MASK            (0xFE)
#  define CPLD2_I2C_CMD_INDICATOR_MASK         (0x01)
#    define CPLD2_I2C_CMD_WRITE_OP             (0x00)
#    define CPLD2_I2C_CMD_READ_OP              (0x01)

#define CPLD2_REG_I2C_CMD_BYTE0_OFFSET         (0x213)

#define CPLD2_REG_I2C_CMD_BYTE1_OFFSET         (0x214)

#define CPLD2_REG_I2C_CMD_BYTE2_OFFSET         (0x215)

#define CPLD2_REG_I2C_STATUS_SW_RESET_OFFSET   (0x216)
#  define CPLD2_I2C_MASTER_ERR                 (1 << 7)
#  define CPLD2_I2C_BUSY_INDICATOR             (1 << 6)
#  define CPLD2_I2C_MASTER_SOFT_RESET_L        (1 << 0)

/* I2C Write data Byte 0-7 Register */
#define CPLD2_REG_I2C_WRITE_DATA_BYTE0_OFFSET  (0x220)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE1_OFFSET  (0x221)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE2_OFFSET  (0x222)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE3_OFFSET  (0x223)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE4_OFFSET  (0x224)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE5_OFFSET  (0x225)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE6_OFFSET  (0x226)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE7_OFFSET  (0x227)

/* I2C Read data Byte 0-7 Register */
#define CPLD2_REG_I2C_READ_DATA_BYTE0_OFFSET   (0x230)
#define CPLD2_REG_I2C_READ_DATA_BYTE1_OFFSET   (0x231)
#define CPLD2_REG_I2C_READ_DATA_BYTE2_OFFSET   (0x232)
#define CPLD2_REG_I2C_READ_DATA_BYTE3_OFFSET   (0x233)
#define CPLD2_REG_I2C_READ_DATA_BYTE4_OFFSET   (0x234)
#define CPLD2_REG_I2C_READ_DATA_BYTE5_OFFSET   (0x235)
#define CPLD2_REG_I2C_READ_DATA_BYTE6_OFFSET   (0x236)
#define CPLD2_REG_I2C_READ_DATA_BYTE7_OFFSET   (0x237)

#define CPLD2_REG_QPHY_1_8_STATUS_L_OFFSET     (0x240)
#  define CPLD2_QPHY_8_STATUS_L                (1 << 7)
#  define CPLD2_QPHY_7_STATUS_L                (1 << 6)
#  define CPLD2_QPHY_6_STATUS_L                (1 << 5)
#  define CPLD2_QPHY_5_STATUS_L                (1 << 4)
#  define CPLD2_QPHY_4_STATUS_L                (1 << 3)
#  define CPLD2_QPHY_3_STATUS_L                (1 << 2)
#  define CPLD2_QPHY_2_STATUS_L                (1 << 1)
#  define CPLD2_QPHY_1_STATUS_L                (1 << 0)

#define CPLD2_REG_QPHY_9_10_STATUS_L_OFFSET    (0x241)
#  define CPLD2_QPHY_10_STATUS_L               (1 << 1)
#  define CPLD2_QPHY_9_STATUS_L                (1 << 0)

#define CPLD2_REG_QPHY_1_8_INT_STATUS_OFFSET   (0x242)
#  define CPLD2_QPHY_8_INT_STATUS              (1 << 7)
#  define CPLD2_QPHY_7_INT_STATUS              (1 << 6)
#  define CPLD2_QPHY_6_INT_STATUS              (1 << 5)
#  define CPLD2_QPHY_5_INT_STATUS              (1 << 4)
#  define CPLD2_QPHY_4_INT_STATUS              (1 << 3)
#  define CPLD2_QPHY_3_INT_STATUS              (1 << 2)
#  define CPLD2_QPHY_2_INT_STATUS              (1 << 1)
#  define CPLD2_QPHY_1_INT_STATUS              (1 << 0)

#define CPLD2_REG_QPHY_9_10_INT_STATUS_OFFSET  (0x243)
#  define CPLD2_QPHY_10_INT_STATUS             (1 << 1)
#  define CPLD2_QPHY_9_INT_STATUS              (1 << 0)

#define CPLD2_REG_QPHY_1_8_INT_MASK_OFFSET     (0x244)
#  define CPLD2_QPHY_8_INT_MASK                (1 << 7)
#  define CPLD2_QPHY_7_INT_MASK                (1 << 6)
#  define CPLD2_QPHY_6_INT_MASK                (1 << 5)
#  define CPLD2_QPHY_5_INT_MASK                (1 << 4)
#  define CPLD2_QPHY_4_INT_MASK                (1 << 3)
#  define CPLD2_QPHY_3_INT_MASK                (1 << 2)
#  define CPLD2_QPHY_2_INT_MASK                (1 << 1)
#  define CPLD2_QPHY_1_INT_MASK                (1 << 0)

#define CPLD2_REG_QPHY_9_10_INT_MASK_OFFSET    (0x245)
#  define CPLD2_QPHY_10_INT_MASK               (1 << 1)
#  define CPLD2_QPHY_9_INT_MASK                (1 << 0)

#define CPLD2_REG_QSFP_1_8_RESET_L_OFFSET      (0x250)
#  define CPLD2_QSFP_8_RESET_L                 (1 << 7)
#  define CPLD2_QSFP_7_RESET_L                 (1 << 6)
#  define CPLD2_QSFP_6_RESET_L                 (1 << 5)
#  define CPLD2_QSFP_5_RESET_L                 (1 << 4)
#  define CPLD2_QSFP_4_RESET_L                 (1 << 3)
#  define CPLD2_QSFP_3_RESET_L                 (1 << 2)
#  define CPLD2_QSFP_2_RESET_L                 (1 << 1)
#  define CPLD2_QSFP_1_RESET_L                 (1 << 0)

#define CPLD2_REG_QSFP_9_10_RESET_L_OFFSET     (0x251)
#  define CPLD2_QSFP_10_RESET_L                (1 << 1)
#  define CPLD2_QSFP_9_RESET_L                 (1 << 0)

#define CPLD2_REG_QSFP_1_8_LPMOD_OFFSET        (0x252)
#  define CPLD2_QSFP_8_LPMOD                   (1 << 7)
#  define CPLD2_QSFP_7_LPMOD                   (1 << 6)
#  define CPLD2_QSFP_6_LPMOD                   (1 << 5)
#  define CPLD2_QSFP_5_LPMOD                   (1 << 4)
#  define CPLD2_QSFP_4_LPMOD                   (1 << 3)
#  define CPLD2_QSFP_3_LPMOD                   (1 << 2)
#  define CPLD2_QSFP_2_LPMOD                   (1 << 1)
#  define CPLD2_QSFP_1_LPMOD                   (1 << 0)

#define CPLD2_REG_QSFP_9_10_LPMOD_OFFSET       (0x253)
#  define CPLD2_QSFP_10_LPMOD                  (1 << 1)
#  define CPLD2_QSFP_9_LPMOD                   (1 << 0)

#define CPLD2_REG_QSFP_1_8_ABSENT_OFFSET       (0x254)
#  define CPLD2_QSFP_8_ABSENT                  (1 << 7)
#  define CPLD2_QSFP_7_ABSENT                  (1 << 6)
#  define CPLD2_QSFP_6_ABSENT                  (1 << 5)
#  define CPLD2_QSFP_5_ABSENT                  (1 << 4)
#  define CPLD2_QSFP_4_ABSENT                  (1 << 3)
#  define CPLD2_QSFP_3_ABSENT                  (1 << 2)
#  define CPLD2_QSFP_2_ABSENT                  (1 << 1)
#  define CPLD2_QSFP_1_ABSENT                  (1 << 0)

#define CPLD2_REG_QSFP_9_10_ABSENT_OFFSET      (0x255)
#  define CPLD2_QSFP_10_ABSENT                 (1 << 1)
#  define CPLD2_QSFP_9_ABSENT                  (1 << 0)

#define CPLD2_REG_QSFP_1_8_INT_STATUS_L_OFFSET (0x256)
#  define CPLD2_QSFP_8_INT_STATUS_L            (1 << 7)
#  define CPLD2_QSFP_7_INT_STATUS_L            (1 << 6)
#  define CPLD2_QSFP_6_INT_STATUS_L            (1 << 5)
#  define CPLD2_QSFP_5_INT_STATUS_L            (1 << 4)
#  define CPLD2_QSFP_4_INT_STATUS_L            (1 << 3)
#  define CPLD2_QSFP_3_INT_STATUS_L            (1 << 2)
#  define CPLD2_QSFP_2_INT_STATUS_L            (1 << 1)
#  define CPLD2_QSFP_1_INT_STATUS_L            (1 << 0)

#define CPLD2_REG_QSFP_9_10_INT_STATUS_L_OFFSET (0x257)
#  define CPLD2_QSFP_10_INT_STATUS_L           (1 << 1)
#  define CPLD2_QSFP_9_INT_STATUS_L            (1 << 0)

#define CPLD2_REG_QSFP_1_8_I2C_READY_OFFSET    (0x258)
#  define CPLD2_QSFP_8_I2C_READY               (1 << 7)
#  define CPLD2_QSFP_7_I2C_READY               (1 << 6)
#  define CPLD2_QSFP_6_I2C_READY               (1 << 5)
#  define CPLD2_QSFP_5_I2C_READY               (1 << 4)
#  define CPLD2_QSFP_4_I2C_READY               (1 << 3)
#  define CPLD2_QSFP_3_I2C_READY               (1 << 2)
#  define CPLD2_QSFP_2_I2C_READY               (1 << 1)
#  define CPLD2_QSFP_1_I2C_READY               (1 << 0)

#define CPLD2_REG_QSFP_9_10_I2C_READY_OFFSET   (0x259)
#  define CPLD2_QSFP_10_I2C_READY              (1 << 1)
#  define CPLD2_QSFP_9_I2C_READY               (1 << 0)

#define CPLD2_REG_SFPP_1_2_CTL_STAT_OFFSET     (0x25A)
#  define CPLD2_SFPP_2_RS_CTL                  (1 << 7)
#  define CPLD2_SFPP_2_ABS_STAT                (1 << 6)
#  define CPLD2_SFPP_2_TXFAULT_STAT            (1 << 5)
#  define CPLD2_SFPP_2_TXDISABLE_CTL           (1 << 4)
#  define CPLD2_SFPP_1_RS_CTL                  (1 << 7)
#  define CPLD2_SFPP_1_ABS_STAT                (1 << 6)
#  define CPLD2_SFPP_1_TXFAULT_STAT            (1 << 5)
#  define CPLD2_SFPP_1_TXDISABLE_CTL           (1 << 4)


// CPLD 3

#define CPLD3_REG_VERSION_OFFSET               (0x280)

#define CPLD3_REG_SW_SCRATCH_OFFSET            (0x281)

#define CPLD3_REG_I2C_PORT_ID_OFFSET           (0x290)
#  define CPLD3_I2C_BAUD_RATE                  (1 << 6) /* 0=50KHz, 1=100KHz */
#  define CPLD3_I2C_OPCODE_ID_MASK             (0x1F)

#define CPLD3_REG_I2C_OPCODE_OFFSET            (0x291)
#  define CPLD3_I2C_DATA_LEN_MASK              (0xF0)
#  define CPLD3_I2C_DATA_LEN_SHIFT             (4)
#  define CPLD3_I2C_CMD_LEN_MASK               (0x03)
#    define CPLD3_I2C_CMD_CUR_ADDR             (0x00)

#define CPLD3_REG_DEV_ADDR_OFFSET              (0x292)
#  define CPLD3_I2C_SLAVE_ADDR_MASK            (0xFE)
#  define CPLD3_I2C_CMD_INDICATOR_MASK         (0x01)
#    define CPLD3_I2C_CMD_WRITE_OP             (0x00)
#    define CPLD3_I2C_CMD_READ_OP              (0x01)

#define CPLD3_REG_I2C_CMD_BYTE0_OFFSET         (0x293)

#define CPLD3_REG_I2C_CMD_BYTE1_OFFSET         (0x294)

#define CPLD3_REG_I2C_CMD_BYTE2_OFFSET         (0x295)

#define CPLD3_REG_I2C_STATUS_SW_RESET_OFFSET   (0x296)
#  define CPLD3_I2C_MASTER_ERR                 (1 << 7)
#  define CPLD3_I2C_BUSY_INDICATOR             (1 << 6)
#  define CPLD3_I2C_MASTER_SOFT_RESET_L        (1 << 0)

/* I2C Write data Byte 0-7 Register */
#define CPLD3_REG_I2C_WRITE_DATA_BYTE0_OFFSET  (0x2a0)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE1_OFFSET  (0x2a1)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE2_OFFSET  (0x2a2)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE3_OFFSET  (0x2a3)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE4_OFFSET  (0x2a4)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE5_OFFSET  (0x2a5)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE6_OFFSET  (0x2a6)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE7_OFFSET  (0x2a7)

/* I2C Read data Byte 0-7 Register */
#define CPLD3_REG_I2C_READ_DATA_BYTE0_OFFSET   (0x2b0)
#define CPLD3_REG_I2C_READ_DATA_BYTE1_OFFSET   (0x2b1)
#define CPLD3_REG_I2C_READ_DATA_BYTE2_OFFSET   (0x2b2)
#define CPLD3_REG_I2C_READ_DATA_BYTE3_OFFSET   (0x2b3)
#define CPLD3_REG_I2C_READ_DATA_BYTE4_OFFSET   (0x2b4)
#define CPLD3_REG_I2C_READ_DATA_BYTE5_OFFSET   (0x2b5)
#define CPLD3_REG_I2C_READ_DATA_BYTE6_OFFSET   (0x2b6)
#define CPLD3_REG_I2C_READ_DATA_BYTE7_OFFSET   (0x2b7)

#define CPLD3_REG_QPHY_11_18_STATUS_L_OFFSET   (0x2c0)
#  define CPLD3_QPHY_18_STATUS_L               (1 << 7)
#  define CPLD3_QPHY_17_STATUS_L               (1 << 6)
#  define CPLD3_QPHY_16_STATUS_L               (1 << 5)
#  define CPLD3_QPHY_15_STATUS_L               (1 << 4)
#  define CPLD3_QPHY_14_STATUS_L               (1 << 3)
#  define CPLD3_QPHY_13_STATUS_L               (1 << 2)
#  define CPLD3_QPHY_12_STATUS_L               (1 << 1)
#  define CPLD3_QPHY_11_STATUS_L               (1 << 0)

#define CPLD3_REG_QPHY_19_21_STATUS_L_OFFSET   (0x2c1)
#  define CPLD3_QPHY_21_STATUS_L               (1 << 2)
#  define CPLD3_QPHY_20_STATUS_L               (1 << 1)
#  define CPLD3_QPHY_19_STATUS_L               (1 << 0)

#define CPLD3_REG_QPHY_11_18_INT_STATUS_OFFSET (0x2c2)
#  define CPLD3_QPHY_18_INT_STATUS             (1 << 7)
#  define CPLD3_QPHY_17_INT_STATUS             (1 << 6)
#  define CPLD3_QPHY_16_INT_STATUS             (1 << 5)
#  define CPLD3_QPHY_15_INT_STATUS             (1 << 4)
#  define CPLD3_QPHY_14_INT_STATUS             (1 << 3)
#  define CPLD3_QPHY_13_INT_STATUS             (1 << 2)
#  define CPLD3_QPHY_12_INT_STATUS             (1 << 1)
#  define CPLD3_QPHY_11_INT_STATUS             (1 << 0)

#define CPLD3_REG_QPHY_19_21_INT_STATUS_OFFSET (0x2c3)
#  define CPLD3_QPHY_21_INT_STATUS             (1 << 2)
#  define CPLD3_QPHY_20_INT_STATUS             (1 << 1)
#  define CPLD3_QPHY_19_INT_STATUS             (1 << 0)

#define CPLD3_REG_QPHY_11_18_INT_MASK_OFFSET   (0x2c4)
#  define CPLD3_QPHY_18_INT_MASK               (1 << 7)
#  define CPLD3_QPHY_17_INT_MASK               (1 << 6)
#  define CPLD3_QPHY_16_INT_MASK               (1 << 5)
#  define CPLD3_QPHY_15_INT_MASK               (1 << 4)
#  define CPLD3_QPHY_14_INT_MASK               (1 << 3)
#  define CPLD3_QPHY_13_INT_MASK               (1 << 2)
#  define CPLD3_QPHY_12_INT_MASK               (1 << 1)
#  define CPLD3_QPHY_11_INT_MASK               (1 << 0)

#define CPLD3_REG_QPHY_19_21_INT_MASK_OFFSET   (0x2c5)
#  define CPLD3_QPHY_21_INT_MASK               (1 << 2)
#  define CPLD3_QPHY_20_INT_MASK               (1 << 1)
#  define CPLD3_QPHY_19_INT_MASK               (1 << 0)

#define CPLD3_REG_QSFP_11_18_RESET_L_OFFSET    (0x2d0)
#  define CPLD3_QSFP_18_RESET_L                (1 << 7)
#  define CPLD3_QSFP_17_RESET_L                (1 << 6)
#  define CPLD3_QSFP_16_RESET_L                (1 << 5)
#  define CPLD3_QSFP_15_RESET_L                (1 << 4)
#  define CPLD3_QSFP_14_RESET_L                (1 << 3)
#  define CPLD3_QSFP_13_RESET_L                (1 << 2)
#  define CPLD3_QSFP_12_RESET_L                (1 << 1)
#  define CPLD3_QSFP_11_RESET_L                (1 << 0)

#define CPLD3_REG_QSFP_19_21_RESET_L_OFFSET    (0x2d1)
#  define CPLD3_QSFP_21_RESET_L                (1 << 2)
#  define CPLD3_QSFP_20_RESET_L                (1 << 1)
#  define CPLD3_QSFP_19_RESET_L                (1 << 0)

#define CPLD3_REG_QSFP_11_18_LPMOD_OFFSET      (0x2d2)
#  define CPLD3_QSFP_18_LPMOD                  (1 << 7)
#  define CPLD3_QSFP_17_LPMOD                  (1 << 6)
#  define CPLD3_QSFP_16_LPMOD                  (1 << 5)
#  define CPLD3_QSFP_15_LPMOD                  (1 << 4)
#  define CPLD3_QSFP_14_LPMOD                  (1 << 3)
#  define CPLD3_QSFP_13_LPMOD                  (1 << 2)
#  define CPLD3_QSFP_12_LPMOD                  (1 << 1)
#  define CPLD3_QSFP_11_LPMOD                  (1 << 0)

#define CPLD3_REG_QSFP_19_21_LPMOD_OFFSET      (0x2d3)
#  define CPLD3_QSFP_21_LPMOD                  (1 << 2)
#  define CPLD3_QSFP_20_LPMOD                  (1 << 1)
#  define CPLD3_QSFP_19_LPMOD                  (1 << 0)

#define CPLD3_REG_QSFP_11_18_ABSENT_OFFSET     (0x2d4)
#  define CPLD3_QSFP_18_ABSENT                 (1 << 7)
#  define CPLD3_QSFP_17_ABSENT                 (1 << 6)
#  define CPLD3_QSFP_16_ABSENT                 (1 << 5)
#  define CPLD3_QSFP_15_ABSENT                 (1 << 4)
#  define CPLD3_QSFP_14_ABSENT                 (1 << 3)
#  define CPLD3_QSFP_13_ABSENT                 (1 << 2)
#  define CPLD3_QSFP_12_ABSENT                 (1 << 1)
#  define CPLD3_QSFP_11_ABSENT                 (1 << 0)

#define CPLD3_REG_QSFP_19_21_ABSENT_OFFSET     (0x2d5)
#  define CPLD3_QSFP_21_ABSENT                 (1 << 2)
#  define CPLD3_QSFP_20_ABSENT                 (1 << 1)
#  define CPLD3_QSFP_19_ABSENT                 (1 << 0)

#define CPLD3_REG_QSFP_11_18_INT_STATUS_OFFSET (0x2d6)
#  define CPLD3_QSFP_18_INT_STATUS_L           (1 << 7)
#  define CPLD3_QSFP_17_INT_STATUS_L           (1 << 6)
#  define CPLD3_QSFP_16_INT_STATUS_L           (1 << 5)
#  define CPLD3_QSFP_15_INT_STATUS_L           (1 << 4)
#  define CPLD3_QSFP_14_INT_STATUS_L           (1 << 3)
#  define CPLD3_QSFP_13_INT_STATUS_L           (1 << 2)
#  define CPLD3_QSFP_12_INT_STATUS_L           (1 << 1)
#  define CPLD3_QSFP_11_INT_STATUS_L           (1 << 0)

#define CPLD3_REG_QSFP_19_21_INT_STATUS_OFFSET (0x2d7)
#  define CPLD3_QSFP_21_INT_STATUS_L           (1 << 2)
#  define CPLD3_QSFP_20_INT_STATUS_L           (1 << 1)
#  define CPLD3_QSFP_19_INT_STATUS_L           (1 << 0)

#define CPLD3_REG_QSFP_11_18_I2C_READY_OFFSET  (0x2d8)
#  define CPLD3_QSFP_18_I2C_READY              (1 << 7)
#  define CPLD3_QSFP_17_I2C_READY              (1 << 6)
#  define CPLD3_QSFP_16_I2C_READY              (1 << 5)
#  define CPLD3_QSFP_15_I2C_READY              (1 << 4)
#  define CPLD3_QSFP_14_I2C_READY              (1 << 3)
#  define CPLD3_QSFP_13_I2C_READY              (1 << 2)
#  define CPLD3_QSFP_12_I2C_READY              (1 << 1)
#  define CPLD3_QSFP_11_I2C_READY              (1 << 0)

#define CPLD3_REG_QSFP_19_21_I2C_READY_OFFSET  (0x2d9)
#  define CPLD3_QSFP_21_I2C_READY              (1 << 2)
#  define CPLD3_QSFP_20_I2C_READY              (1 << 1)
#  define CPLD3_QSFP_19_I2C_READY              (1 << 0)


// CPLD 4

#define CPLD4_REG_VERSION_OFFSET               (0x300)

#define CPLD4_REG_SW_SCRATCH_OFFSET            (0x301)

#define CPLD4_REG_RESET_CONTROL_OFFSET         (0x302)
#  define CPLD4_RESET_XC7A35T_L                (1 << 7)
#  define CPLD4_RESET_USBHUB_L                 (1 << 6)
#  define CPLD4_RESET_BCM54616_L               (1 << 5)
#  define CPLD4_RESET_BCM82381_L               (1 << 4)
#  define CPLD4_RESET_BCM56960_L               (1 << 3)
#  define CPLD4_RESET_CPLD5_L                  (1 << 2)
#  define CPLD4_RESET_CPLD3_L                  (1 << 1)
#  define CPLD4_RESET_CPLD2_L                  (1 << 0)

#define CPLD4_REG_SYS_LED_CTRL_OFFSET          (0x303)
#  define CPLD4_PSU2_LED_L                     (1 << 3)    /* 0:On, 1:Off */
#  define CPLD4_PSU1_LED_L                     (1 << 2)    /* 0:On, 1:Off */
#  define CPLD4_SYS_LED_GREEN                  (0x00)
#  define CPLD4_SYS_LED_GREEN_SLOW_BLINK       (0x01)
#  define CPLD4_SYS_LED_GREEN_FAST_BLINK       (0x02)
#  define CPLD4_SYS_LED_MASK                   (0x03)
#  define CPLD4_SYS_LED_OFF                    (0x03)

#define CPLD4_REG_MISC_STAT_CTL_OFFSET         (0x304)

#define CPLD4_REG_PWR_GOOD_OFFSET              (0x305)
#  define CPLD4_XP1R0V_PWRGD_STAT              (1 << 4)
#  define CPLD4_XP1R0V_ROV_PWRGD_STAT          (1 << 3)
#  define CPLD4_XP1R25V_PG_STAT                (1 << 2)
#  define CPLD4_XP1R8V_PG_STAT                 (1 << 1)
#  define CPLD4_XP3R3V_PWRGD_STAT              (1 << 0)

#define CPLD4_REG_FPGA_DOWNLOAD_STAT_CTRL_OFFSET (0x306)
#  define CPLD4_PWR_GOOD_XC7A35T_DOUT          (1 << 5)
#  define CPLD4_PWR_GOOD_XC7A35T_INT_L         (1 << 4)
#  define CPLD4_PWR_GOOD_XC7A35T_DONE          (1 << 3)
#  define CPLD4_PWR_GOOD_XC7A35T_DIN_STAT      (1 << 2)
#  define CPLD4_PWR_GOOD_XC7A35T_PROG_STAT_L   (1 << 1)
#  define CPLD4_PWR_GOOD_XC7A35T_CCLK_STAT     (1 << 0)

#define CPLD4_REG_INT_PORT_STATUS_L_OFFSET     (0x310)
#  define CPLD4_CPLD4_CPLD1_INT1_STAT_L        (1 << 0)

#define CPLD4_REG_INT0_SOURCE_STAT_L_OFFSET    (0x311)
#  define CPLD4_CPLD5_QSFP3_INT_FROM_CPLD5_L   (1 << 5)
#  define CPLD4_CPLD3_QSFP2_INT_FROM_CPLD3_L   (1 << 4)
#  define CPLD4_CPLD5_QSFP1_INT_FROM_CPLD2_L   (1 << 3)
#  define CPLD4_CPLD5_QSFP3_ABS_FROM_CPLD5_L   (1 << 2)
#  define CPLD4_CPLD3_QSFP2_ABS_FROM_CPLD3_L   (1 << 1)
#  define CPLD4_CPLD2_QSFP1_ABS_FROM_CPLD2_L   (1 << 0)

#define CPLD4_REG_INT1_SOURCE_STAT_L_OFFSET    (0x312)
#  define CPLD4_BCM54616A_INT_STAT_L           (1 << 7)
#  define CPLD4_DS110_1_INT_STAT_L             (1 << 6)
#  define CPLD4_LM75_INT_STAT_L                (1 << 5)
#  define CPLD4_NCP3200_ALERT_STAT_L           (1 << 4)
#  define CPLD4_NCP4200_FAULT_STAT_L           (1 << 3)
#  define CPLD4_NCP4208_ROV_ALERT_STAT_L       (1 << 2)
#  define CPLD4_PWR_MGMT_PMBUS_ALERT_STAT_L    (1 << 1)
#  define CPLD4_PWR_MGMT_PMBUS_CTRL_STAT_L     (1 << 0)

#define CPLD4_REG_INT0_SOURCE_INT_OFFSET       (0x313)
#  define CPLD4_CPLD5_QSFP3_INT                (1 << 5)
#  define CPLD4_CPLD3_QSFP2_INT                (1 << 4)
#  define CPLD4_CPLD5_QSFP1_INT                (1 << 3)
#  define CPLD4_CPLD5_QSFP3_ABS                (1 << 2)
#  define CPLD4_CPLD3_QSFP2_ABS                (1 << 1)
#  define CPLD4_CPLD2_QSFP1_ABS                (1 << 0)

#define CPLD4_REG_INT1_SOURCE_INT_OFFSET       (0x314)
#  define CPLD4_BCM54616A_INT                  (1 << 7)
#  define CPLD4_DS110_1_INT                    (1 << 6)
#  define CPLD4_LM75_INT                       (1 << 5)
#  define CPLD4_NCP3200_ALERT                  (1 << 4)
#  define CPLD4_NCP4200_FAULT                  (1 << 3)
#  define CPLD4_NCP4208_ROV_ALERT              (1 << 2)
#  define CPLD4_PWR_MGMT_PMBUS_ALERT           (1 << 1)
#  define CPLD4_PWR_MGMT_PMBUS_CTRL            (1 << 0)

#define CPLD4_REG_INT0_SOURCE_MASK_OFFSET      (0x315)
#  define CPLD4_CPLD5_QSFP3_INT_MASK           (1 << 5)
#  define CPLD4_CPLD3_QSFP2_INT_MASK           (1 << 4)
#  define CPLD4_CPLD5_QSFP1_INT_MASK           (1 << 3)
#  define CPLD4_CPLD5_QSFP3_ABS_MASK           (1 << 2)
#  define CPLD4_CPLD3_QSFP2_ABS_MASK           (1 << 1)
#  define CPLD4_CPLD2_QSFP1_ABS_MASK           (1 << 0)

#define CPLD4_REG_INT1_SOURCE_MASK_OFFSET      (0x316)
#  define CPLD4_BCM54616A_INT_MASK             (1 << 7)
#  define CPLD4_DS110_1_INT_MASK               (1 << 6)
#  define CPLD4_LM75_INT_MASK                  (1 << 5)
#  define CPLD4_NCP3200_ALERT_MASK             (1 << 4)
#  define CPLD4_NCP4200_FAULT_MASK             (1 << 3)
#  define CPLD4_NCP4208_ROV_ALERT_MASK         (1 << 2)
#  define CPLD4_PWR_MGMT_PMBUS_ALERT_MASK      (1 << 1)
#  define CPLD4_PWR_MGMT_PMBUS_CTRL_MASK       (1 << 0)


// CPLD 5

#define CPLD5_REG_VERSION_OFFSET               (0x380)

#define CPLD5_REG_SW_SCRATCH_OFFSET            (0x381)

#define CPLD5_REG_I2C_PORT_ID_OFFSET           (0x390)
#  define CPLD5_I2C_BAUD_RATE                  (1 << 6) /* 0=50KHz, 1=100KHz */
#  define CPLD5_I2C_OPCODE_ID_MASK             (0x1F)

#define CPLD5_REG_I2C_OPCODE_OFFSET            (0x391)
#  define CPLD5_I2C_DATA_LEN_MASK              (0xF0)
#  define CPLD5_I2C_DATA_LEN_SHIFT             (4)
#  define CPLD5_I2C_CMD_LEN_MASK               (0x03)
#    define CPLD5_I2C_CMD_CUR_ADDR             (0x00)

#define CPLD5_REG_DEV_ADDR_OFFSET              (0x392)
#  define CPLD5_I2C_SLAVE_ADDR_MASK            (0xFE)
#  define CPLD5_I2C_CMD_INDICATOR_MASK         (0x01)
#    define CPLD5_I2C_CMD_WRITE_OP             (0x00)
#    define CPLD5_I2C_CMD_READ_OP              (0x01)

#define CPLD5_REG_I2C_CMD_BYTE0_OFFSET         (0x393)

#define CPLD5_REG_I2C_CMD_BYTE1_OFFSET         (0x394)

#define CPLD5_REG_I2C_CMD_BYTE2_OFFSET         (0x395)

#define CPLD5_REG_I2C_STATUS_SW_RESET_OFFSET   (0x396)
#  define CPLD5_I2C_MASTER_ERR                 (1 << 7)
#  define CPLD5_I2C_BUSY_INDICATOR             (1 << 6)
#  define CPLD5_I2C_MASTER_SOFT_RESET_L        (1 << 0)

/* I2C Write data Byte 0-7 Register */
#define CPLD5_REG_I2C_WRITE_DATA_BYTE0_OFFSET  (0x3a0)
#define CPLD5_REG_I2C_WRITE_DATA_BYTE1_OFFSET  (0x3a1)
#define CPLD5_REG_I2C_WRITE_DATA_BYTE2_OFFSET  (0x3a2)
#define CPLD5_REG_I2C_WRITE_DATA_BYTE3_OFFSET  (0x3a3)
#define CPLD5_REG_I2C_WRITE_DATA_BYTE4_OFFSET  (0x3a4)
#define CPLD5_REG_I2C_WRITE_DATA_BYTE5_OFFSET  (0x3a5)
#define CPLD5_REG_I2C_WRITE_DATA_BYTE6_OFFSET  (0x3a6)
#define CPLD5_REG_I2C_WRITE_DATA_BYTE7_OFFSET  (0x3a7)

/* I2C Read data Byte 0-7 Register */
#define CPLD5_REG_I2C_READ_DATA_BYTE0_OFFSET   (0x3b0)
#define CPLD5_REG_I2C_READ_DATA_BYTE1_OFFSET   (0x3b1)
#define CPLD5_REG_I2C_READ_DATA_BYTE2_OFFSET   (0x3b2)
#define CPLD5_REG_I2C_READ_DATA_BYTE3_OFFSET   (0x3b3)
#define CPLD5_REG_I2C_READ_DATA_BYTE4_OFFSET   (0x3b4)
#define CPLD5_REG_I2C_READ_DATA_BYTE5_OFFSET   (0x3b5)
#define CPLD5_REG_I2C_READ_DATA_BYTE6_OFFSET   (0x3b6)
#define CPLD5_REG_I2C_READ_DATA_BYTE7_OFFSET   (0x3b7)

#define CPLD5_REG_QPHY_22_29_STATUS_L_OFFSET   (0x3c0)
#  define CPLD5_QPHY_29_STATUS_L               (1 << 7)
#  define CPLD5_QPHY_28_STATUS_L               (1 << 6)
#  define CPLD5_QPHY_27_STATUS_L               (1 << 5)
#  define CPLD5_QPHY_26_STATUS_L               (1 << 4)
#  define CPLD5_QPHY_25_STATUS_L               (1 << 3)
#  define CPLD5_QPHY_24_STATUS_L               (1 << 2)
#  define CPLD5_QPHY_23_STATUS_L               (1 << 1)
#  define CPLD5_QPHY_22_STATUS_L               (1 << 0)

#define CPLD5_REG_QPHY_30_32_STATUS_L_OFFSET   (0x3c1)
#  define CPLD5_QPHY_32_STATUS_L               (1 << 2)
#  define CPLD5_QPHY_31_STATUS_L               (1 << 1)
#  define CPLD5_QPHY_30_STATUS_L               (1 << 0)

#define CPLD5_REG_QPHY_22_29_INT_STATUS_OFFSET (0x3c2)
#  define CPLD5_QPHY_29_INT_STATUS             (1 << 7)
#  define CPLD5_QPHY_28_INT_STATUS             (1 << 6)
#  define CPLD5_QPHY_27_INT_STATUS             (1 << 5)
#  define CPLD5_QPHY_26_INT_STATUS             (1 << 4)
#  define CPLD5_QPHY_25_INT_STATUS             (1 << 3)
#  define CPLD5_QPHY_24_INT_STATUS             (1 << 2)
#  define CPLD5_QPHY_23_INT_STATUS             (1 << 1)
#  define CPLD5_QPHY_22_INT_STATUS             (1 << 0)

#define CPLD5_REG_QPHY_30_32_INT_STATUS_OFFSET (0x3c3)
#  define CPLD5_QPHY_32_INT_STATUS             (1 << 2)
#  define CPLD5_QPHY_31_INT_STATUS             (1 << 1)
#  define CPLD5_QPHY_30_INT_STATUS             (1 << 0)

#define CPLD5_REG_QPHY_22_29_INT_MASK_OFFSET   (0x3c4)
#  define CPLD5_QPHY_29_INT_MASK               (1 << 7)
#  define CPLD5_QPHY_28_INT_MASK               (1 << 6)
#  define CPLD5_QPHY_27_INT_MASK               (1 << 5)
#  define CPLD5_QPHY_26_INT_MASK               (1 << 4)
#  define CPLD5_QPHY_25_INT_MASK               (1 << 3)
#  define CPLD5_QPHY_24_INT_MASK               (1 << 2)
#  define CPLD5_QPHY_23_INT_MASK               (1 << 1)
#  define CPLD5_QPHY_22_INT_MASK               (1 << 0)

#define CPLD5_REG_QPHY_30_32_INT_MASK_OFFSET   (0x3c5)
#  define CPLD5_QPHY_32_INT_MASK               (1 << 2)
#  define CPLD5_QPHY_31_INT_MASK               (1 << 1)
#  define CPLD5_QPHY_30_INT_MASK               (1 << 0)

#define CPLD5_REG_QSFP_22_29_RESET_L_OFFSET    (0x3d0)
#  define CPLD5_QSFP_29_RESET_L                (1 << 7)
#  define CPLD5_QSFP_28_RESET_L                (1 << 6)
#  define CPLD5_QSFP_27_RESET_L                (1 << 5)
#  define CPLD5_QSFP_26_RESET_L                (1 << 4)
#  define CPLD5_QSFP_25_RESET_L                (1 << 3)
#  define CPLD5_QSFP_24_RESET_L                (1 << 2)
#  define CPLD5_QSFP_23_RESET_L                (1 << 1)
#  define CPLD5_QSFP_22_RESET_L                (1 << 0)

#define CPLD5_REG_QSFP_30_32_RESET_L_OFFSET    (0x3d1)
#  define CPLD5_QSFP_32_RESET_L                (1 << 2)
#  define CPLD5_QSFP_31_RESET_L                (1 << 1)
#  define CPLD5_QSFP_30_RESET_L                (1 << 0)

#define CPLD5_REG_QSFP_22_29_LPMOD_OFFSET      (0x3d2)
#  define CPLD5_QSFP_29_LPMOD                  (1 << 7)
#  define CPLD5_QSFP_28_LPMOD                  (1 << 6)
#  define CPLD5_QSFP_27_LPMOD                  (1 << 5)
#  define CPLD5_QSFP_26_LPMOD                  (1 << 4)
#  define CPLD5_QSFP_25_LPMOD                  (1 << 3)
#  define CPLD5_QSFP_24_LPMOD                  (1 << 2)
#  define CPLD5_QSFP_23_LPMOD                  (1 << 1)
#  define CPLD5_QSFP_22_LPMOD                  (1 << 0)

#define CPLD5_REG_QSFP_30_32_LPMOD_OFFSET      (0x3d3)
#  define CPLD5_QSFP_32_LPMOD                  (1 << 2)
#  define CPLD5_QSFP_31_LPMOD                  (1 << 1)
#  define CPLD5_QSFP_30_LPMOD                  (1 << 0)

#define CPLD5_REG_QSFP_22_29_ABSENT_OFFSET     (0x3d4)
#  define CPLD5_QSFP_29_ABSENT                 (1 << 7)
#  define CPLD5_QSFP_28_ABSENT                 (1 << 6)
#  define CPLD5_QSFP_27_ABSENT                 (1 << 5)
#  define CPLD5_QSFP_26_ABSENT                 (1 << 4)
#  define CPLD5_QSFP_25_ABSENT                 (1 << 3)
#  define CPLD5_QSFP_24_ABSENT                 (1 << 2)
#  define CPLD5_QSFP_23_ABSENT                 (1 << 1)
#  define CPLD5_QSFP_22_ABSENT                 (1 << 0)

#define CPLD5_REG_QSFP_30_32_ABSENT_OFFSET     (0x3d5)
#  define CPLD5_QSFP_32_ABSENT                 (1 << 2)
#  define CPLD5_QSFP_31_ABSENT                 (1 << 1)
#  define CPLD5_QSFP_30_ABSENT                 (1 << 0)

#define CPLD5_REG_QSFP_22_29_INT_STATUS_OFFSET (0x3d6)
#  define CPLD5_QSFP_29_INT_STATUS_L           (1 << 7)
#  define CPLD5_QSFP_28_INT_STATUS_L           (1 << 6)
#  define CPLD5_QSFP_27_INT_STATUS_L           (1 << 5)
#  define CPLD5_QSFP_26_INT_STATUS_L           (1 << 4)
#  define CPLD5_QSFP_25_INT_STATUS_L           (1 << 3)
#  define CPLD5_QSFP_24_INT_STATUS_L           (1 << 2)
#  define CPLD5_QSFP_23_INT_STATUS_L           (1 << 1)
#  define CPLD5_QSFP_22_INT_STATUS_L           (1 << 0)

#define CPLD5_REG_QSFP_30_32_INT_STATUS_OFFSET (0x3d7)
#  define CPLD5_QSFP_32_INT_STATUS_L           (1 << 2)
#  define CPLD5_QSFP_31_INT_STATUS_L           (1 << 1)
#  define CPLD5_QSFP_30_INT_STATUS_L           (1 << 0)

#define CPLD5_REG_QSFP_22_29_I2C_READY_OFFSET  (0x3d8)
#  define CPLD5_QSFP_29_I2C_READY              (1 << 7)
#  define CPLD5_QSFP_28_I2C_READY              (1 << 6)
#  define CPLD5_QSFP_27_I2C_READY              (1 << 5)
#  define CPLD5_QSFP_26_I2C_READY              (1 << 4)
#  define CPLD5_QSFP_25_I2C_READY              (1 << 3)
#  define CPLD5_QSFP_24_I2C_READY              (1 << 2)
#  define CPLD5_QSFP_23_I2C_READY              (1 << 1)
#  define CPLD5_QSFP_22_I2C_READY              (1 << 0)

#define CPLD5_REG_QSFP_30_32_I2C_READY_OFFSET  (0x3d9)
#  define CPLD5_QSFP_32_I2C_READY              (1 << 2)
#  define CPLD5_QSFP_31_I2C_READY              (1 << 1)
#  define CPLD5_QSFP_30_I2C_READY              (1 << 0)

#endif /* CEL_SEASTONE_H__ */

