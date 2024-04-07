/*
 * Celestica Smallstone XP CPLD Platform Definitions
 *
 * Alan Liebthal <alanl@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef CEL_SMALLSTONE_XP_H__
#define CEL_SMALLSTONE_XP_H__

/*----------------------------------------------------------------------------------
 *
 * register info from R0854-M0006-01 Rev0.4 Smallstone-XP CPLD Secification.pdf
 */

#define CPLD1_REG_VERSION_OFFSET           (0x100)  /* CPLD Version */
#  define CPLD_VERSION_H_MASK              (0xF0)
#  define CPLD_VERSION_H_SHIFT             (4)
#    define CPLD_VERSION_ID                (0x10)
#  define CPLD_VERSION_L_MASK              (0x0F)
#  define CPLD_VERSION_L_SHIFT             (0)

#define CPLD1_REG_SW_SCRATCH_OFFSET        (0x101)

#define CPLD1_REG_RESET_CTRL_OFFSET        (0x102)  /* Reset Control */
#  define CPLD1_RESET_CPLD4_L              (1 << 3)
#  define CPLD1_RESET_WATCH_DOG_MASK_L     (1 << 2)
#  define CPLD1_RESET_BUTTON_HARD_L        (1 << 1)
#  define CPLD1_POWER_CYCLE_SOFT_L         (1 << 0)

#define CPLD1_REG_RESET_SRC_OFFSET         (0x103)  /* Reset Source */
#  define CPLD1_REG_RESET_SRC_MASK         (0x0F)
#    define CPLD1_RESET_PWR_ON             (1 << 3) /* Power On Reset */
#    define CPLD1_RESET_WATCH_DOG          (1 << 2) /* Watchdog Reset */
#    define CPLD1_RESET_CPU_HARD           (1 << 1) /* SW-set hard Reset */
#    define CPLD1_RESET_CPU_SOFT           (1 << 0) /* CPU request Reset */

#define CPLD1_REG_BOARD_TYPE_OFFSET        (0x104)  /* Board Type */
#  define CPLD1_BOARD_TYPE_REDXP_VAL       (1)
#  define CPLD1_BOARD_TYPE_SMALLXP_VAL     (2)

#define CPLD1_REG_INT_STATUS_OFFSET        (0x110)
#  define CPLD1_GPIO_CPU_SUS0_L            (1 << 0)

#define CPLD1_REG_INT0_SRC_STATUS_OFFSET   (0x111)
#  define CPLD1_INT_STAT_RESET_BTN_L       (1 << 1)
#  define CPLD1_INT_STAT_GEPHY_BACK_L      (1 << 0)

#define CPLD1_REG_INT0_SRC_INT_OFFSET      (0x112)
#  define CPLD1_INT_RESET_BTN_L            (1 << 1)
#  define CPLD1_INT_GEPHY_BACK_L           (1 << 0)

#define CPLD1_REG_INT0_SRC_MASK_OFFSET     (0x113)
#  define CPLD1_INT_MASK_RESET_BTN_L       (1 << 1)
#  define CPLD1_INT_MASK_GEPHY_BACK_L      (1 << 0)

#define CPLD1_REG_PWR_STATUS_OFFSET        (0x120)
#  define CPLD1_PWR_STAT_PSU2_ALERT_L      (1 << 1)
#  define CPLD1_PWR_STAT_PSU1_ALERT_L      (1 << 0)

#define CPLD1_REG_PWR_GOOD_OFFSET          (0x121)
#  define CPLD1_PWR_GOOD_XPOR75V           (1 << 7)
#  define CPLD1_PWR_GOOD_VCC_VNN_CP        (1 << 6)
#  define CPLD1_PWR_GOOD_VDDR_CP           (1 << 5)
#  define CPLD1_PWR_GOOD_XP3R3V_STD        (1 << 4)
#  define CPLD1_PWR_GOOD_XP3R3V_EARLY      (1 << 3)
#  define CPLD1_PWR_GOOD_XP1R0V_CP         (1 << 2)
#  define CPLD1_PWR_GOOD_XP1R07V_CP        (1 << 1)
#  define CPLD1_PWR_GOOD_XP3R3V_CP         (1 << 0)

#define CPLD1_REG_7_PANEL_LED_OFFSET       (0x122)
#  define CPLD1_7_PANEL_DP                 (1 << 7)
#  define CPLD1_7_PANEL_G                  (1 << 6)
#  define CPLD1_7_PANEL_F                  (1 << 5)
#  define CPLD1_7_PANEL_E                  (1 << 4)
#  define CPLD1_7_PANEL_D                  (1 << 3)
#  define CPLD1_7_PANEL_C                  (1 << 2)
#  define CPLD1_7_PANEL_B                  (1 << 1)
#  define CPLD1_7_PANEL_A                  (1 << 0)

#define CPLD1_REG_WRITE_PROTECT_CTL_OFFSET  (0x123)
#  define CPLD1_WRITE_PROTECT_SPI1_L        (1 << 5)
#  define CPLD1_WRITE_PROTECT_SPI0_L        (1 << 4)
#  define CPLD1_WRITE_PROTECT_SPD2          (1 << 3)
#  define CPLD1_WRITE_PROTECT_SPD1          (1 << 2)
#  define CPLD1_WRITE_PROTECT_NAND_L        (1 << 1)
#  define CPLD1_WRITE_PROTECT_SYS_EEPROM    (1 << 0)

#define CPLD1_REG_MISC_STAT_CTL_OFFSET      (0x124)

#define CPLD1_REG_FAN_PSU_PRESENT_OFFSET    (0x125)
#  define CPLD1_REG_FAN_PSU_I2C_SLAVE_BUSY  (1 << 7)
#  define CPLD1_REG_FAN_PSU_FAN4_ABSENT     (1 << 5)
#  define CPLD1_REG_FAN_PSU_FAN3_ABSENT     (1 << 4)
#  define CPLD1_REG_FAN_PSU_FAN2_ABSENT     (1 << 3)
#  define CPLD1_REG_FAN_PSU_FAN1_ABSENT     (1 << 2)
#  define CPLD1_REG_FAN_PSU_PSU1_ABSENT     (1 << 1)
#  define CPLD1_REG_FAN_PSU_PSU2_ABSENT     (1 << 0)

#define CPLD2_REG_VERSION_OFFSET            (0x200)

#define CPLD2_REG_SW_SCRATCH_OFFSET         (0x201)

#define CPLD2_REG_1_8_ERROR_LED_OFFSET      (0x203)
#  define CPLD2_PORT_8_RED_LED1             (1 << 7)
#  define CPLD2_PORT_7_RED_LED1             (1 << 6)
#  define CPLD2_PORT_6_RED_LED1             (1 << 5)
#  define CPLD2_PORT_5_RED_LED1             (1 << 4)
#  define CPLD2_PORT_4_RED_LED1             (1 << 3)
#  define CPLD2_PORT_3_RED_LED1             (1 << 2)
#  define CPLD2_PORT_2_RED_LED1             (1 << 1)
#  define CPLD2_PORT_1_RED_LED1             (1 << 0)

#define CPLD2_REG_9_16_ERROR_LED_OFFSET     (0x204)
#  define CPLD2_PORT_16_RED_LED1            (1 << 7)
#  define CPLD2_PORT_15_RED_LED1            (1 << 6)
#  define CPLD2_PORT_14_RED_LED1            (1 << 5)
#  define CPLD2_PORT_13_RED_LED1            (1 << 4)
#  define CPLD2_PORT_12_RED_LED1            (1 << 3)
#  define CPLD2_PORT_11_RED_LED1            (1 << 2)
#  define CPLD2_PORT_10_RED_LED1            (1 << 1)
#  define CPLD2_PORT_9_RED_LED1             (1 << 0)

#define CPLD2_REG_I2C_PORT_ID_OFFSET        (0x210)
#  define CPLD2_I2C_BAUD_RATE               (1 << 6) /* 0 - 50KHz, 1 - 100KHz */
#  define CPLD2_I2C_OPCODE_ID_MASK          (0x1F)

#define CPLD2_REG_I2C_OPCODE_OFFSET         (0x211)
#  define CPLD2_I2C_DATA_LEN_MASK           (0xF0)
#  define CPLD2_I2C_DATA_LEN_SHIFT          (4)
#  define CPLD2_I2C_CMD_LEN_MASK            (0x03)
#    define CPLD2_I2C_CMD_CUR_ADDR          (0x00)

#define CPLD2_REG_DEV_ADDR_OFFSET           (0x212)
#  define CPLD2_I2C_SLAVE_ADDR_MASK         (0xFE)
#  define CPLD2_I2C_CMD_INDICATOR_MASK      (0x01)
#    define CPLD2_I2C_CMD_WRITE_OP          (0x00)
#    define CPLD2_I2C_CMD_READ_OP           (0x01)

#define CPLD2_REG_I2C_CMD_BYTE0_OFFSET      (0x213)

/* Byte 1 & 2 not used in Smallstone */
#define CPLD2_REG_I2C_CMD_BYTE1_OFFSET      (0x214)
#define CPLD2_REG_I2C_CMD_BYTE2_OFFSET      (0x215)

#define CPLD2_REG_I2C_STATUS_SW_RESET_OFFSET (0x216)
#  define CPLD2_I2C_MASTER_ERR              (1 << 7)
#  define CPLD2_I2C_BUSY_INDICATOR          (1 << 6)
#  define CPLD2_I2C_MASTER_SOFT_RESET_L     (1 << 0)


/* I2C Write data Byte 0-7 Register */
#define CPLD2_REG_I2C_WRITE_DATA_BYTE0_OFFSET  (0x220)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE1_OFFSET  (0x221)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE2_OFFSET  (0x222)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE3_OFFSET  (0x223)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE4_OFFSET  (0x224)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE5_OFFSET  (0x225)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE6_OFFSET  (0x226)
#define CPLD2_REG_I2C_WRITE_DATA_BYTE7_OFFSET  (0x227)

/* I2C Write data Byte 0-7 Register */
#define CPLD2_REG_I2C_READ_DATA_BYTE0_OFFSET  (0x230)
#define CPLD2_REG_I2C_READ_DATA_BYTE1_OFFSET  (0x231)
#define CPLD2_REG_I2C_READ_DATA_BYTE2_OFFSET  (0x232)
#define CPLD2_REG_I2C_READ_DATA_BYTE3_OFFSET  (0x233)
#define CPLD2_REG_I2C_READ_DATA_BYTE4_OFFSET  (0x234)
#define CPLD2_REG_I2C_READ_DATA_BYTE5_OFFSET  (0x235)
#define CPLD2_REG_I2C_READ_DATA_BYTE6_OFFSET  (0x236)
#define CPLD2_REG_I2C_READ_DATA_BYTE7_OFFSET  (0x237)

#define CPLD2_REG_QPHY_1_8_STATUS_OFFSET      (0x240)
#  define CPLD2_QPHY_8_STATUS                 (1 << 7)
#  define CPLD2_QPHY_7_STATUS                 (1 << 6)
#  define CPLD2_QPHY_6_STATUS                 (1 << 5)
#  define CPLD2_QPHY_5_STATUS                 (1 << 4)
#  define CPLD2_QPHY_4_STATUS                 (1 << 3)
#  define CPLD2_QPHY_3_STATUS                 (1 << 2)
#  define CPLD2_QPHY_2_STATUS                 (1 << 1)
#  define CPLD2_QPHY_1_STATUS                 (1 << 0)

#define CPLD2_REG_QPHY_9_16_STATUS_OFFSET     (0x241)
#  define CPLD2_QPHY_16_STATUS                (1 << 7)
#  define CPLD2_QPHY_15_STATUS                (1 << 6)
#  define CPLD2_QPHY_14_STATUS                (1 << 5)
#  define CPLD2_QPHY_13_STATUS                (1 << 4)
#  define CPLD2_QPHY_12_STATUS                (1 << 3)
#  define CPLD2_QPHY_11_STATUS                (1 << 2)
#  define CPLD2_QPHY_10_STATUS                (1 << 1)
#  define CPLD2_QPHY_9_STATUS                 (1 << 0)

#define CPLD2_REG_QPHY_1_8_INT_STATUS_OFFSET  (0x242)
#  define CPLD2_QPHY_8_INT_STATUS             (1 << 7)
#  define CPLD2_QPHY_7_INT_STATUS             (1 << 6)
#  define CPLD2_QPHY_6_INT_STATUS             (1 << 5)
#  define CPLD2_QPHY_5_INT_STATUS             (1 << 4)
#  define CPLD2_QPHY_4_INT_STATUS             (1 << 3)
#  define CPLD2_QPHY_3_INT_STATUS             (1 << 2)
#  define CPLD2_QPHY_2_INT_STATUS             (1 << 1)
#  define CPLD2_QPHY_1_INT_STATUS             (1 << 0)

#define CPLD2_REG_QPHY_9_16_INT_STATUS_OFFSET (0x243)
#  define CPLD2_QPHY_16_INT_STATUS            (1 << 7)
#  define CPLD2_QPHY_15_INT_STATUS            (1 << 6)
#  define CPLD2_QPHY_14_INT_STATUS            (1 << 5)
#  define CPLD2_QPHY_13_INT_STATUS            (1 << 4)
#  define CPLD2_QPHY_12_INT_STATUS            (1 << 3)
#  define CPLD2_QPHY_11_INT_STATUS            (1 << 2)
#  define CPLD2_QPHY_10_INT_STATUS            (1 << 1)
#  define CPLD2_QPHY_9_INT_STATUS             (1 << 0)

#define CPLD2_REG_QPHY_1_8_INT_MASK_L_OFFSET  (0x244)
#  define CPLD2_QPHY_8_INT_MASK_L             (1 << 7)
#  define CPLD2_QPHY_7_INT_MASK_L             (1 << 6)
#  define CPLD2_QPHY_6_INT_MASK_L             (1 << 5)
#  define CPLD2_QPHY_5_INT_MASK_L             (1 << 4)
#  define CPLD2_QPHY_4_INT_MASK_L             (1 << 3)
#  define CPLD2_QPHY_3_INT_MASK_L             (1 << 2)
#  define CPLD2_QPHY_2_INT_MASK_L             (1 << 1)
#  define CPLD2_QPHY_1_INT_MASK_L             (1 << 0)

#define CPLD2_REG_QPHY_9_16_INT_MASK_L_OFFSET (0x245)
#  define CPLD2_QPHY_16_INT_MASK_L            (1 << 7)
#  define CPLD2_QPHY_15_INT_MASK_L            (1 << 6)
#  define CPLD2_QPHY_14_INT_MASK_L            (1 << 5)
#  define CPLD2_QPHY_13_INT_MASK_L            (1 << 4)
#  define CPLD2_QPHY_12_INT_MASK_L            (1 << 3)
#  define CPLD2_QPHY_11_INT_MASK_L            (1 << 2)
#  define CPLD2_QPHY_10_INT_MASK_L            (1 << 1)
#  define CPLD2_QPHY_9_INT_MASK_L             (1 << 0)

#define CPLD2_REG_QSFP_1_8_RESET_L_OFFSET     (0x250)
#  define CPLD2_QSFP_8_RESET_L                (1 << 7)
#  define CPLD2_QSFP_7_RESET_L                (1 << 6)
#  define CPLD2_QSFP_6_RESET_L                (1 << 5)
#  define CPLD2_QSFP_5_RESET_L                (1 << 4)
#  define CPLD2_QSFP_4_RESET_L                (1 << 3)
#  define CPLD2_QSFP_3_RESET_L                (1 << 2)
#  define CPLD2_QSFP_2_RESET_L                (1 << 1)
#  define CPLD2_QSFP_1_RESET_L                (1 << 0)

#define CPLD2_REG_QSFP_9_16_RESET_L_OFFSET    (0x251)
#  define CPLD2_QSFP_16_RESET_L               (1 << 7)
#  define CPLD2_QSFP_15_RESET_L               (1 << 6)
#  define CPLD2_QSFP_14_RESET_L               (1 << 5)
#  define CPLD2_QSFP_13_RESET_L               (1 << 4)
#  define CPLD2_QSFP_12_RESET_L               (1 << 3)
#  define CPLD2_QSFP_11_RESET_L               (1 << 2)
#  define CPLD2_QSFP_10_RESET_L               (1 << 1)
#  define CPLD2_QSFP_9_RESET_L                (1 << 0)

#define CPLD2_REG_QSFP_1_8_LPMOD_L_OFFSET     (0x252)
#  define CPLD2_QSFP_8_LPMOD_L                (1 << 7)
#  define CPLD2_QSFP_7_LPMOD_L                (1 << 6)
#  define CPLD2_QSFP_6_LPMOD_L                (1 << 5)
#  define CPLD2_QSFP_5_LPMOD_L                (1 << 4)
#  define CPLD2_QSFP_4_LPMOD_L                (1 << 3)
#  define CPLD2_QSFP_3_LPMOD_L                (1 << 2)
#  define CPLD2_QSFP_2_LPMOD_L                (1 << 1)
#  define CPLD2_QSFP_1_LPMOD_L                (1 << 0)

#define CPLD2_REG_QSFP_9_16_LPMOD_L_OFFSET    (0x253)
#  define CPLD2_QSFP_16_LPMOD_L               (1 << 7)
#  define CPLD2_QSFP_15_LPMOD_L               (1 << 6)
#  define CPLD2_QSFP_14_LPMOD_L               (1 << 5)
#  define CPLD2_QSFP_13_LPMOD_L               (1 << 4)
#  define CPLD2_QSFP_12_LPMOD_L               (1 << 3)
#  define CPLD2_QSFP_11_LPMOD_L               (1 << 2)
#  define CPLD2_QSFP_10_LPMOD_L               (1 << 1)
#  define CPLD2_QSFP_9_LPMOD_L                (1 << 0)

#define CPLD2_REG_QSFP_1_8_ABSENT_OFFSET     (0x254)
#  define CPLD2_QSFP_8_ABSENT                (1 << 7)
#  define CPLD2_QSFP_7_ABSENT                (1 << 6)
#  define CPLD2_QSFP_6_ABSENT                (1 << 5)
#  define CPLD2_QSFP_5_ABSENT                (1 << 4)
#  define CPLD2_QSFP_4_ABSENT                (1 << 3)
#  define CPLD2_QSFP_3_ABSENT                (1 << 2)
#  define CPLD2_QSFP_2_ABSENT                (1 << 1)
#  define CPLD2_QSFP_1_ABSENT                (1 << 0)

#define CPLD2_REG_QSFP_9_16_ABSENT_OFFSET    (0x255)
#  define CPLD2_QSFP_16_ABSENT               (1 << 7)
#  define CPLD2_QSFP_15_ABSENT               (1 << 6)
#  define CPLD2_QSFP_14_ABSENT               (1 << 5)
#  define CPLD2_QSFP_13_ABSENT               (1 << 4)
#  define CPLD2_QSFP_12_ABSENT               (1 << 3)
#  define CPLD2_QSFP_11_ABSENT               (1 << 2)
#  define CPLD2_QSFP_10_ABSENT               (1 << 1)
#  define CPLD2_QSFP_9_ABSENT                (1 << 0)

#define CPLD2_REG_QSFP_1_8_I2C_READY_L_OFFSET (0x258)
#  define CPLD2_QSFP_8_I2C_READY_L            (1 << 7)
#  define CPLD2_QSFP_7_I2C_READY_L            (1 << 6)
#  define CPLD2_QSFP_6_I2C_READY_L            (1 << 5)
#  define CPLD2_QSFP_5_I2C_READY_L            (1 << 4)
#  define CPLD2_QSFP_4_I2C_READY_L            (1 << 3)
#  define CPLD2_QSFP_3_I2C_READY_L            (1 << 2)
#  define CPLD2_QSFP_2_I2C_READY_L            (1 << 1)
#  define CPLD2_QSFP_1_I2C_READY_L            (1 << 0)

#define CPLD2_REG_QSFP_9_16_I2C_READY_L_OFFSET (0x259)
#  define CPLD2_QSFP_16_I2C_READY_L           (1 << 7)
#  define CPLD2_QSFP_15_I2C_READY_L           (1 << 6)
#  define CPLD2_QSFP_14_I2C_READY_L           (1 << 5)
#  define CPLD2_QSFP_13_I2C_READY_L           (1 << 4)
#  define CPLD2_QSFP_12_I2C_READY_L           (1 << 3)
#  define CPLD2_QSFP_11_I2C_READY_L           (1 << 2)
#  define CPLD2_QSFP_10_I2C_READY_L           (1 << 1)
#  define CPLD2_QSFP_9_I2C_READY_L            (1 << 0)

#define CPLD3_REG_VERSION_OFFSET              (0x280)

#define CPLD3_REG_SW_SCRATCH_OFFSET           (0x281)

#define CPLD3_REG_PORT_30_32_LED_MODE_OFFSET  (0x282)
#  define CPLD3_PORT_30_32_LED_MODE_SHIFT     (5)
#  define CPLD3_PORT_32_LED_MODE              (1 << 2)  /* 1:40G  0:4x10G  */
#  define CPLD3_PORT_31_LED_MODE              (1 << 1)  /* 1:40G  0:4x10G  */
#  define CPLD3_PORT_30_LED_MODE              (1 << 0)  /* 1:40G  0:4x10G  */

#define CPLD3_REG_PORT_17_24_ERROR_LED_OFFSET (0x283)
#  define CPLD3_PORT_24_RED_LED1_L            (1 << 7)
#  define CPLD3_PORT_23_RED_LED1_L            (1 << 6)
#  define CPLD3_PORT_22_RED_LED1_L            (1 << 5)
#  define CPLD3_PORT_21_RED_LED1_L            (1 << 4)
#  define CPLD3_PORT_20_RED_LED1_L            (1 << 3)
#  define CPLD3_PORT_19_RED_LED1_L            (1 << 2)
#  define CPLD3_PORT_18_RED_LED1_L            (1 << 1)
#  define CPLD3_PORT_17_RED_LED1_L            (1 << 0)

#define CPLD3_REG_PORT_25_32_ERROR_LED_OFFSET (0x284)
#  define CPLD3_PORT_32_RED_LED1_L            (1 << 7)
#  define CPLD3_PORT_31_RED_LED1_L            (1 << 6)
#  define CPLD3_PORT_30_RED_LED1_L            (1 << 5)
#  define CPLD3_PORT_29_RED_LED1_L            (1 << 4)
#  define CPLD3_PORT_28_RED_LED1_L            (1 << 3)
#  define CPLD3_PORT_27_RED_LED1_L            (1 << 2)
#  define CPLD3_PORT_26_RED_LED1_L            (1 << 1)
#  define CPLD3_PORT_25_RED_LED1_L            (1 << 0)

#define CPLD3_REG_PORT_30_31_ERROR_LED_OFFSET (0x285)
#  define CPLD3_PORT_31_RED_LED4_L            (1 << 7)
#  define CPLD3_PORT_31_RED_LED3_L            (1 << 6)
#  define CPLD3_PORT_31_RED_LED2_L            (1 << 5)
#  define CPLD3_PORT_30_RED_LED4_L            (1 << 3)
#  define CPLD3_PORT_30_RED_LED3_L            (1 << 2)
#  define CPLD3_PORT_30_RED_LED2_L            (1 << 1)

#define CPLD3_REG_PORT_32_ERROR_LED_OFFSET    (0x286)
#  define CPLD3_PORT_32_RED_LED4_L            (1 << 3)
#  define CPLD3_PORT_32_RED_LED3_L            (1 << 2)
#  define CPLD3_PORT_32_RED_LED2_L            (1 << 1)


#define CPLD3_REG_I2C_PORT_ID_OFFSET          (0x290)
#  define CPLD3_I2C_BAUD_RATE                 (1 << 6) /* 0 - 50KHz, 1 - 100KHz */
#  define CPLD3_I2C_OPCODE_ID_MASK            (0x1F)

#define CPLD3_REG_I2C_OPCODE_OFFSET           (0x291)
#  define CPLD3_I2C_DATA_LEN_MASK             (0xF0)
#  define CPLD3_I2C_DATA_LEN_SHIFT            (4)
#  define CPLD3_I2C_CMD_LEN_MASK              (0x03)
#    define CPLD3_I2C_CMD_CUR_ADDR            (0x00)

#define CPLD3_REG_DEV_ADDR_OFFSET             (0x292)
#  define CPLD3_I2C_SLAVE_ADDR_MASK           (0xFE)
#  define CPLD3_I2C_CMD_INDICATOR_MASK        (0x01)
#    define CPLD3_I2C_CMD_WRITE_OP            (0x00)
#    define CPLD3_I2C_CMD_READ_OP             (0x01)

#define CPLD3_REG_I2C_CMD_BYTE0_OFFSET        (0x293)

/* Byte 1 & 2 not used in Smallstone */
#define CPLD3_REG_I2C_CMD_BYTE1_OFFSET        (0x294)
#define CPLD3_REG_I2C_CMD_BYTE2_OFFSET        (0x295)

#define CPLD3_REG_I2C_STATUS_SW_RESET_OFFSET  (0x296)
#  define CPLD3_I2C_MASTER_ERR                (1 << 7)
#  define CPLD3_I2C_BUSY_INDICATOR            (1 << 6)
#  define CPLD3_I2C_MASTER_SOFT_RESET_L       (1 << 0)


/* I2C Write data Byte 0-7 Register */
#define CPLD3_REG_I2C_WRITE_DATA_BYTE0_OFFSET  (0x2a0)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE1_OFFSET  (0x2a1)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE2_OFFSET  (0x2a2)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE3_OFFSET  (0x2a3)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE4_OFFSET  (0x2a4)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE5_OFFSET  (0x2a5)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE6_OFFSET  (0x2a6)
#define CPLD3_REG_I2C_WRITE_DATA_BYTE7_OFFSET  (0x2a7)

/* I2C Write data Byte 0-7 Register */
#define CPLD3_REG_I2C_READ_DATA_BYTE0_OFFSET   (0x2b0)
#define CPLD3_REG_I2C_READ_DATA_BYTE1_OFFSET   (0x2b1)
#define CPLD3_REG_I2C_READ_DATA_BYTE2_OFFSET   (0x2b2)
#define CPLD3_REG_I2C_READ_DATA_BYTE3_OFFSET   (0x2b3)
#define CPLD3_REG_I2C_READ_DATA_BYTE4_OFFSET   (0x2b4)
#define CPLD3_REG_I2C_READ_DATA_BYTE5_OFFSET   (0x2b5)
#define CPLD3_REG_I2C_READ_DATA_BYTE6_OFFSET   (0x2b6)
#define CPLD3_REG_I2C_READ_DATA_BYTE7_OFFSET   (0x2b7)

#define CPLD3_REG_QPHY_17_24_STATUS_OFFSET     (0x2c0)
#  define CPLD3_QPHY_24_STATUS                 (1 << 7)
#  define CPLD3_QPHY_23_STATUS                 (1 << 6)
#  define CPLD3_QPHY_22_STATUS                 (1 << 5)
#  define CPLD3_QPHY_21_STATUS                 (1 << 4)
#  define CPLD3_QPHY_20_STATUS                 (1 << 3)
#  define CPLD3_QPHY_19_STATUS                 (1 << 2)
#  define CPLD3_QPHY_18_STATUS                 (1 << 1)
#  define CPLD3_QPHY_17_STATUS                 (1 << 0)

#define CPLD3_REG_QPHY_25_32_STATUS_OFFSET     (0x2c1)
#  define CPLD3_QPHY_32_STATUS                 (1 << 7)
#  define CPLD3_QPHY_31_STATUS                 (1 << 6)
#  define CPLD3_QPHY_30_STATUS                 (1 << 5)
#  define CPLD3_QPHY_29_STATUS                 (1 << 4)
#  define CPLD3_QPHY_28_STATUS                 (1 << 3)
#  define CPLD3_QPHY_27_STATUS                 (1 << 2)
#  define CPLD3_QPHY_26_STATUS                 (1 << 1)
#  define CPLD3_QPHY_25_STATUS                 (1 << 0)

#define CPLD3_REG_QPHY_17_24_INT_STATUS_OFFSET (0x2c2)
#  define CPLD3_QPHY_24_INT_STATUS             (1 << 7)
#  define CPLD3_QPHY_23_INT_STATUS             (1 << 6)
#  define CPLD3_QPHY_22_INT_STATUS             (1 << 5)
#  define CPLD3_QPHY_21_INT_STATUS             (1 << 4)
#  define CPLD3_QPHY_20_INT_STATUS             (1 << 3)
#  define CPLD3_QPHY_19_INT_STATUS             (1 << 2)
#  define CPLD3_QPHY_18_INT_STATUS             (1 << 1)
#  define CPLD3_QPHY_17_INT_STATUS             (1 << 0)

#define CPLD3_REG_QPHY_25_32_INT_STATUS_OFFSET (0x2c3)
#  define CPLD3_QPHY_32_INT_STATUS             (1 << 7)
#  define CPLD3_QPHY_31_INT_STATUS             (1 << 6)
#  define CPLD3_QPHY_30_INT_STATUS             (1 << 5)
#  define CPLD3_QPHY_29_INT_STATUS             (1 << 4)
#  define CPLD3_QPHY_28_INT_STATUS             (1 << 3)
#  define CPLD3_QPHY_27_INT_STATUS             (1 << 2)
#  define CPLD3_QPHY_26_INT_STATUS             (1 << 1)
#  define CPLD3_QPHY_25_INT_STATUS             (1 << 0)

#define CPLD3_REG_QPHY_17_24_INT_MASK_L_OFFSET (0x2c4)
#  define CPLD3_QPHY_24_INT_MASK_L             (1 << 7)
#  define CPLD3_QPHY_23_INT_MASK_L             (1 << 6)
#  define CPLD3_QPHY_22_INT_MASK_L             (1 << 5)
#  define CPLD3_QPHY_21_INT_MASK_L             (1 << 4)
#  define CPLD3_QPHY_20_INT_MASK_L             (1 << 3)
#  define CPLD3_QPHY_19_INT_MASK_L             (1 << 2)
#  define CPLD3_QPHY_18_INT_MASK_L             (1 << 1)
#  define CPLD3_QPHY_17_INT_MASK_L             (1 << 0)

#define CPLD3_REG_QPHY_25_32_INT_MASK_L_OFFSET (0x2c5)
#  define CPLD3_QPHY_32_INT_MASK_L             (1 << 7)
#  define CPLD3_QPHY_31_INT_MASK_L             (1 << 6)
#  define CPLD3_QPHY_30_INT_MASK_L             (1 << 5)
#  define CPLD3_QPHY_29_INT_MASK_L             (1 << 4)
#  define CPLD3_QPHY_28_INT_MASK_L             (1 << 3)
#  define CPLD3_QPHY_27_INT_MASK_L             (1 << 2)
#  define CPLD3_QPHY_26_INT_MASK_L             (1 << 1)
#  define CPLD3_QPHY_25_INT_MASK_L             (1 << 0)

#define CPLD3_REG_QSFP_17_24_RESET_L_OFFSET     (0x2d0)
#  define CPLD3_QSFP_24_RESET_L                 (1 << 7)
#  define CPLD3_QSFP_23_RESET_L                 (1 << 6)
#  define CPLD3_QSFP_22_RESET_L                 (1 << 5)
#  define CPLD3_QSFP_21_RESET_L                 (1 << 4)
#  define CPLD3_QSFP_20_RESET_L                 (1 << 3)
#  define CPLD3_QSFP_19_RESET_L                 (1 << 2)
#  define CPLD3_QSFP_18_RESET_L                 (1 << 1)
#  define CPLD3_QSFP_17_RESET_L                 (1 << 0)

#define CPLD3_REG_QSFP_25_32_RESET_L_OFFSET     (0x2d1)
#  define CPLD3_QSFP_32_RESET_L                 (1 << 7)
#  define CPLD3_QSFP_31_RESET_L                 (1 << 6)
#  define CPLD3_QSFP_30_RESET_L                 (1 << 5)
#  define CPLD3_QSFP_29_RESET_L                 (1 << 4)
#  define CPLD3_QSFP_28_RESET_L                 (1 << 3)
#  define CPLD3_QSFP_27_RESET_L                 (1 << 2)
#  define CPLD3_QSFP_26_RESET_L                 (1 << 1)
#  define CPLD3_QSFP_25_RESET_L                 (1 << 0)

#define CPLD3_REG_QSFP_17_24_LPMOD_L_OFFSET    (0x2d2)
#  define CPLD3_QSFP_24_LPMOD_L                (1 << 7)
#  define CPLD3_QSFP_23_LPMOD_L                (1 << 6)
#  define CPLD3_QSFP_22_LPMOD_L                (1 << 5)
#  define CPLD3_QSFP_21_LPMOD_L                (1 << 4)
#  define CPLD3_QSFP_20_LPMOD_L                (1 << 3)
#  define CPLD3_QSFP_19_LPMOD_L                (1 << 2)
#  define CPLD3_QSFP_18_LPMOD_L                (1 << 1)
#  define CPLD3_QSFP_17_LPMOD_L                (1 << 0)

#define CPLD3_REG_QSFP_25_32_LPMOD_L_OFFSET    (0x2d3)
#  define CPLD3_QSFP_32_LPMOD_L                (1 << 7)
#  define CPLD3_QSFP_31_LPMOD_L                (1 << 6)
#  define CPLD3_QSFP_30_LPMOD_L                (1 << 5)
#  define CPLD3_QSFP_29_LPMOD_L                (1 << 4)
#  define CPLD3_QSFP_28_LPMOD_L                (1 << 3)
#  define CPLD3_QSFP_27_LPMOD_L                (1 << 2)
#  define CPLD3_QSFP_26_LPMOD_L                (1 << 1)
#  define CPLD3_QSFP_25_LPMOD_L                (1 << 0)

#define CPLD3_REG_QSFP_17_24_ABSENT_OFFSET      (0x2d4)
#  define CPLD3_QSFP_24_ABSENT                  (1 << 7)
#  define CPLD3_QSFP_23_ABSENT                  (1 << 6)
#  define CPLD3_QSFP_22_ABSENT                  (1 << 5)
#  define CPLD3_QSFP_21_ABSENT                  (1 << 4)
#  define CPLD3_QSFP_20_ABSENT                  (1 << 3)
#  define CPLD3_QSFP_19_ABSENT                  (1 << 2)
#  define CPLD3_QSFP_18_ABSENT                  (1 << 1)
#  define CPLD3_QSFP_17_ABSENT                  (1 << 0)

#define CPLD3_REG_QSFP_25_32_ABSENT_OFFSET      (0x2d5)
#  define CPLD3_QSFP_32_ABSENT                  (1 << 7)
#  define CPLD3_QSFP_31_ABSENT                  (1 << 6)
#  define CPLD3_QSFP_30_ABSENT                  (1 << 5)
#  define CPLD3_QSFP_29_ABSENT                  (1 << 4)
#  define CPLD3_QSFP_28_ABSENT                  (1 << 3)
#  define CPLD3_QSFP_27_ABSENT                  (1 << 2)
#  define CPLD3_QSFP_26_ABSENT                  (1 << 1)
#  define CPLD3_QSFP_25_ABSENT                  (1 << 0)

#define CPLD3_REG_QSFP_17_24_INT_STATUS_OFFSET (0x2d6)
#  define CPLD3_QSFP_24_INT_STATUS_L           (1 << 7)
#  define CPLD3_QSFP_23_INT_STATUS_L           (1 << 6)
#  define CPLD3_QSFP_22_INT_STATUS_L           (1 << 5)
#  define CPLD3_QSFP_21_INT_STATUS_L           (1 << 4)
#  define CPLD3_QSFP_20_INT_STATUS_L           (1 << 3)
#  define CPLD3_QSFP_19_INT_STATUS_L           (1 << 2)
#  define CPLD3_QSFP_18_INT_STATUS_L           (1 << 1)
#  define CPLD3_QSFP_17_INT_STATUS_L           (1 << 0)

#define CPLD3_REG_QSFP_25_32_INT_STATUS_OFFSET (0x2d7)
#  define CPLD3_QSFP_32_INT_STATUS_L           (1 << 7)
#  define CPLD3_QSFP_31_INT_STATUS_L           (1 << 6)
#  define CPLD3_QSFP_30_INT_STATUS_L           (1 << 5)
#  define CPLD3_QSFP_29_INT_STATUS_L           (1 << 4)
#  define CPLD3_QSFP_28_INT_STATUS_L           (1 << 3)
#  define CPLD3_QSFP_27_INT_STATUS_L           (1 << 2)
#  define CPLD3_QSFP_26_INT_STATUS_L           (1 << 1)
#  define CPLD3_QSFP_25_INT_STATUS_L           (1 << 0)

#define CPLD3_REG_QSFP_17_24_I2C_READY_L_OFFSET (0x2d8)
#  define CPLD3_QSFP_24_I2C_READY_L            (1 << 7)
#  define CPLD3_QSFP_23_I2C_READY_L            (1 << 6)
#  define CPLD3_QSFP_22_I2C_READY_L            (1 << 5)
#  define CPLD3_QSFP_21_I2C_READY_L            (1 << 4)
#  define CPLD3_QSFP_20_I2C_READY_L            (1 << 3)
#  define CPLD3_QSFP_19_I2C_READY_L            (1 << 2)
#  define CPLD3_QSFP_18_I2C_READY_L            (1 << 1)
#  define CPLD3_QSFP_17_I2C_READY_L            (1 << 0)

#define CPLD3_REG_QSFP_25_32_I2C_READY_L_OFFSET (0x2d9)
#  define CPLD3_QSFP_32_I2C_READY_L            (1 << 7)
#  define CPLD3_QSFP_31_I2C_READY_L            (1 << 6)
#  define CPLD3_QSFP_30_I2C_READY_L            (1 << 5)
#  define CPLD3_QSFP_29_I2C_READY_L            (1 << 4)
#  define CPLD3_QSFP_28_I2C_READY_L            (1 << 3)
#  define CPLD3_QSFP_27_I2C_READY_L            (1 << 2)
#  define CPLD3_QSFP_26_I2C_READY_L            (1 << 1)
#  define CPLD3_QSFP_25_I2C_READY_L            (1 << 0)

#define CPLD4_REG_VERSION_OFFSET               (0x300)

#define CPLD4_REG_SW_SCRATCH_OFFSET            (0x301)

#define CPLD4_REG_RESET_CONTROL_OFFSET         (0x302)
#  define CPLD4_RESET_BCM54616_L               (1 << 5)
#  define CPLD4_RESET_BCM56850_L               (1 << 4)
#  define CPLD4_RESET_USBHUB_L                 (1 << 3)
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

#define CPLD5_REG_VERSION_OFFSET               (0x380)

#define CPLD5_REG_SW_SCRATCH_OFFSET            (0x381)

#define CPLD5_REG_PORT_1_8_LED_MODE_OFFSET     (0x390)
#  define CPLD5_PORT8_LED_MODE                 (1 << 7)
#  define CPLD5_PORT7_LED_MODE                 (1 << 6)
#  define CPLD5_PORT6_LED_MODE                 (1 << 5)
#  define CPLD5_PORT5_LED_MODE                 (1 << 4)
#  define CPLD5_PORT4_LED_MODE                 (1 << 3)
#  define CPLD5_PORT3_LED_MODE                 (1 << 2)
#  define CPLD5_PORT2_LED_MODE                 (1 << 1)
#  define CPLD5_PORT1_LED_MODE                 (1 << 0)

#define CPLD5_REG_PORT_9_16_LED_MODE_OFFSET    (0x391)
#  define CPLD5_PORT16_LED_MODE                (1 << 7)
#  define CPLD5_PORT15_LED_MODE                (1 << 6)
#  define CPLD5_PORT14_LED_MODE                (1 << 5)
#  define CPLD5_PORT13_LED_MODE                (1 << 4)
#  define CPLD5_PORT12_LED_MODE                (1 << 3)
#  define CPLD5_PORT11_LED_MODE                (1 << 2)
#  define CPLD5_PORT10_LED_MODE                (1 << 1)
#  define CPLD5_PORT9_LED_MODE                 (1 << 0)

#define CPLD5_REG_PORT_17_24_LED_MODE_OFFSET   (0x392)
#  define CPLD5_PORT24_LED_MODE                (1 << 7)
#  define CPLD5_PORT23_LED_MODE                (1 << 6)
#  define CPLD5_PORT22_LED_MODE                (1 << 5)
#  define CPLD5_PORT21_LED_MODE                (1 << 4)
#  define CPLD5_PORT20_LED_MODE                (1 << 3)
#  define CPLD5_PORT19_LED_MODE                (1 << 2)
#  define CPLD5_PORT18_LED_MODE                (1 << 1)
#  define CPLD5_PORT17_LED_MODE                (1 << 0)

#define CPLD5_REG_PORT_25_29_LED_MODE_OFFSET   (0x393)
#  define CPLD5_PORT_25_29_LED_MODE_MASK       (0x1f)
#  define CPLD5_PORT29_LED_MODE                (1 << 4)
#  define CPLD5_PORT28_LED_MODE                (1 << 3)
#  define CPLD5_PORT27_LED_MODE                (1 << 2)
#  define CPLD5_PORT26_LED_MODE                (1 << 1)
#  define CPLD5_PORT25_LED_MODE                (1 << 0)

#define CPLD5_REG_PORT_1_2_ERROR_LED_OFFSET    (0x394)
#  define CPLD5_RED_LED_234_MASK               (0xee)
#define CPLD5_PORT_2_RED_LED4_L                (1 << 7)
#define CPLD5_PORT_2_RED_LED3_L                (1 << 6)
#define CPLD5_PORT_2_RED_LED2_L                (1 << 5)
#define CPLD5_PORT_1_RED_LED4_L                (1 << 3)
#define CPLD5_PORT_1_RED_LED3_L                (1 << 2)
#define CPLD5_PORT_1_RED_LED2_L                (1 << 1)

#define CPLD5_REG_PORT_3_4_ERROR_LED_OFFSET    (0x395)
#define CPLD5_PORT_4_RED_LED4_L                (1 << 7)
#define CPLD5_PORT_4_RED_LED3_L                (1 << 6)
#define CPLD5_PORT_4_RED_LED2_L                (1 << 5)
#define CPLD5_PORT_3_RED_LED4_L                (1 << 3)
#define CPLD5_PORT_3_RED_LED3_L                (1 << 2)
#define CPLD5_PORT_3_RED_LED2_L                (1 << 1)

#define CPLD5_REG_PORT_5_6_ERROR_LED_OFFSET    (0x396)
#define CPLD5_PORT_6_RED_LED4_L                (1 << 7)
#define CPLD5_PORT_6_RED_LED3_L                (1 << 6)
#define CPLD5_PORT_6_RED_LED2_L                (1 << 5)
#define CPLD5_PORT_5_RED_LED4_L                (1 << 3)
#define CPLD5_PORT_5_RED_LED3_L                (1 << 2)
#define CPLD5_PORT_5_RED_LED2_L                (1 << 1)

#define CPLD5_REG_PORT_7_8_ERROR_LED_OFFSET    (0x397)
#define CPLD5_PORT_8_RED_LED4_L                (1 << 7)
#define CPLD5_PORT_8_RED_LED3_L                (1 << 6)
#define CPLD5_PORT_8_RED_LED2_L                (1 << 5)
#define CPLD5_PORT_7_RED_LED4_L                (1 << 3)
#define CPLD5_PORT_7_RED_LED3_L                (1 << 2)
#define CPLD5_PORT_7_RED_LED2_L                (1 << 1)

#define CPLD5_REG_PORT_9_10_ERROR_LED_OFFSET   (0x398)
#define CPLD5_PORT_10_RED_LED4_L               (1 << 7)
#define CPLD5_PORT_10_RED_LED3_L               (1 << 6)
#define CPLD5_PORT_10_RED_LED2_L               (1 << 5)
#define CPLD5_PORT_9_RED_LED4_L                (1 << 3)
#define CPLD5_PORT_9_RED_LED3_L                (1 << 2)
#define CPLD5_PORT_9_RED_LED2_L                (1 << 1)

#define CPLD5_REG_PORT_11_12_ERROR_LED_OFFSET  (0x399)
#define CPLD5_PORT_12_RED_LED4_L               (1 << 7)
#define CPLD5_PORT_12_RED_LED3_L               (1 << 6)
#define CPLD5_PORT_12_RED_LED2_L               (1 << 5)
#define CPLD5_PORT_11_RED_LED4_L               (1 << 3)
#define CPLD5_PORT_11_RED_LED3_L               (1 << 2)
#define CPLD5_PORT_11_RED_LED2_L               (1 << 1)

#define CPLD5_REG_PORT_13_14_ERROR_LED_OFFSET  (0x39a)
#define CPLD5_PORT_14_RED_LED4_L               (1 << 7)
#define CPLD5_PORT_14_RED_LED3_L               (1 << 6)
#define CPLD5_PORT_14_RED_LED2_L               (1 << 5)
#define CPLD5_PORT_13_RED_LED4_L               (1 << 3)
#define CPLD5_PORT_13_RED_LED3_L               (1 << 2)
#define CPLD5_PORT_13_RED_LED2_L               (1 << 1)

#define CPLD5_REG_PORT_15_16_ERROR_LED_OFFSET  (0x39b)
#define CPLD5_PORT_16_RED_LED4_L               (1 << 7)
#define CPLD5_PORT_16_RED_LED3_L               (1 << 6)
#define CPLD5_PORT_16_RED_LED2_L               (1 << 5)
#define CPLD5_PORT_15_RED_LED4_L               (1 << 3)
#define CPLD5_PORT_15_RED_LED3_L               (1 << 2)
#define CPLD5_PORT_15_RED_LED2_L               (1 << 1)

#define CPLD5_REG_PORT_17_18_ERROR_LED_OFFSET  (0x39c)
#define CPLD5_PORT_18_RED_LED4_L               (1 << 7)
#define CPLD5_PORT_18_RED_LED3_L               (1 << 6)
#define CPLD5_PORT_18_RED_LED2_L               (1 << 5)
#define CPLD5_PORT_17_RED_LED4_L               (1 << 3)
#define CPLD5_PORT_17_RED_LED3_L               (1 << 2)
#define CPLD5_PORT_17_RED_LED2_L               (1 << 1)

#define CPLD5_REG_PORT_19_20_ERROR_LED_OFFSET  (0x39d)
#define CPLD5_PORT_20_RED_LED4_L               (1 << 7)
#define CPLD5_PORT_20_RED_LED3_L               (1 << 6)
#define CPLD5_PORT_20_RED_LED2_L               (1 << 5)
#define CPLD5_PORT_19_RED_LED4_L               (1 << 3)
#define CPLD5_PORT_19_RED_LED3_L               (1 << 2)
#define CPLD5_PORT_19_RED_LED2_L               (1 << 1)

#define CPLD5_REG_PORT_21_22_ERROR_LED_OFFSET  (0x39e)
#define CPLD5_PORT_22_RED_LED4_L               (1 << 7)
#define CPLD5_PORT_22_RED_LED3_L               (1 << 6)
#define CPLD5_PORT_22_RED_LED2_L               (1 << 5)
#define CPLD5_PORT_21_RED_LED4_L               (1 << 3)
#define CPLD5_PORT_21_RED_LED3_L               (1 << 2)
#define CPLD5_PORT_21_RED_LED2_L               (1 << 1)

#define CPLD5_REG_PORT_23_24_ERROR_LED_OFFSET  (0x39f)
#define CPLD5_PORT_24_RED_LED4_L               (1 << 7)
#define CPLD5_PORT_24_RED_LED3_L               (1 << 6)
#define CPLD5_PORT_24_RED_LED2_L               (1 << 5)
#define CPLD5_PORT_23_RED_LED4_L               (1 << 3)
#define CPLD5_PORT_23_RED_LED3_L               (1 << 2)
#define CPLD5_PORT_23_RED_LED2_L               (1 << 1)

#define CPLD5_REG_PORT_25_26_ERROR_LED_OFFSET  (0x3a0)
#define CPLD5_PORT_26_RED_LED4_L               (1 << 7)
#define CPLD5_PORT_26_RED_LED3_L               (1 << 6)
#define CPLD5_PORT_26_RED_LED2_L               (1 << 5)
#define CPLD5_PORT_25_RED_LED4_L               (1 << 3)
#define CPLD5_PORT_25_RED_LED3_L               (1 << 2)
#define CPLD5_PORT_25_RED_LED2_L               (1 << 1)

#define CPLD5_REG_PORT_27_28_ERROR_LED_OFFSET  (0x3a1)
#define CPLD5_PORT_28_RED_LED4_L               (1 << 7)
#define CPLD5_PORT_28_RED_LED3_L               (1 << 6)
#define CPLD5_PORT_28_RED_LED2_L               (1 << 5)
#define CPLD5_PORT_27_RED_LED4_L               (1 << 3)
#define CPLD5_PORT_27_RED_LED3_L               (1 << 2)
#define CPLD5_PORT_27_RED_LED2_L               (1 << 1)

#define CPLD5_REG_PORT_29_ERROR_LED_OFFSET     (0x3a2)
#  define CPLD5_PORT_29_RED_LED_MASK           (0x0e)
#define CPLD5_PORT_29_RED_LED4_L               (1 << 3)
#define CPLD5_PORT_29_RED_LED3_L               (1 << 2)
#define CPLD5_PORT_29_RED_LED2_L               (1 << 1)


#endif /* CEL_SMALLSTONE_XP_H__ */
