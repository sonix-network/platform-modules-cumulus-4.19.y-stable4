/*
 * Celestica RedstoneXP CPLD Platform Definitions
 *
 * Alan Liebthal <alanl@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef CEL_REDSTONE_XP_H__
#define CEL_REDSTONE_XP_H__

#define CPLD_IO_BASE 0x100
#define CPLD_IO_SIZE 0x2ff

/*-----------------------------------------------------------------------------
 *
 * register info from R0854-M0035-01 Rev0.3 Redstone-XP CPLD Design
 * Specification pdf
 *
 *  CPLD1
 */
#define CPLD1_REG_VERSION_OFFSET            (0x100)
#  define CPLD1_VERSION_H_MASK              (0xF0)
#  define CPLD1_VERSION_H_SHIFT             (4)
#    define CPLD1_VERSION_ID                (0x10)
#  define CPLD1_VERSION_L_MASK              (0x0F)
#  define CPLD1_VERSION_L_SHIFT             (0)

#define CPLD1_REG_SW_SCRATCH_OFFSET         (0x101)

#define CPLD1_REG_RESET_CTRL_OFFSET         (0x102)
#  define CPLD1_CPLD4_RESET_CTL             (1 << 3)    /* CPLD4 reset */
#  define CPLD1_RESET_WATCH_DOG_MASK        (1 << 2)    /* Watchdog Reset */
#  define CPLD1_RESET_CPU_HARD_L            (1 << 1)    /* SW HARD Reset */
#  define CPLD1_RESET_CPU_SOFT_L            (1 << 0)    /* SW Soft Reset */

#define CPLD1_REG_RESET_SRC_OFFSET          (0x103)     /* Reset Source */
#  define CPLD1_REG_RESET_SRC_MASK          (0x0F)
#    define CPLD1_RESET_PWR_ON              (1 << 3)    /* Power On Reset */
#    define CPLD1_RESET_WATCH_DOG           (1 << 2)    /* Watchdog Reset */
#    define CPLD1_RESET_CPU_HARD            (1 << 1)    /* SW-set hard Reset */
#    define CPLD1_RESET_CPU_SOFT            (1 << 0)    /* CPU request Reset */

#define CPLD1_REG_INT_PORT_STATUS_OFFSET    (0x110)
#  define CPLD1_INT_PORT_GPOI_INT           (1 << 0)

#define CPLD1_REG_INT0_SRC_STATUS_OFFSET    (0x111)
#  define CPLD1_INT0_STAT_RESET_BTN         (1 << 1)
#  define CPLD1_INT0_STAT_GEPHY             (1 << 0)

#define CPLD1_REG_INT0_SRC_INT_OFFSET       (0x112)
#  define CPLD1_INT0_INT_RESET_BTN          (1 << 1)
#  define CPLD1_INT0_INT_GEPHY              (1 << 0)

#define CPLD1_REG_INT0_SRC_MASK_OFFSET      (0x113)
#  define CPLD1_INT0_MASK_RESET_BTN         (1 << 1)
#  define CPLD1_INT0_MASK_GEPHY             (1 << 0)

#define CPLD1_REG_PSU_STATUS_OFFSET         (0x120)
#  define CPLD1_PSU2_ALERT                  (1 << 1)
#  define CPLD1_PSU1_ALERT                  (1 << 0)

#define CPLD1_REG_PWR_GOOD_OFFSET           (0x121)
#  define CPLD1_PWR_GD_XP0R75V              (1 << 7)
#  define CPLD1_PWR_GD_VCC_VNN_CP           (1 << 6)
#  define CPLD1_PWR_GD_VDDR_CP              (1 << 5)
#  define CPLD1_PWR_GD_XP3R3V_STD           (1 << 4)
#  define CPLD1_PWR_GD_XP3R3V_EARLY         (1 << 3)
#  define CPLD1_PWR_GD_XP1R0V_CP            (1 << 2)
#  define CPLD1_PWR_GD_XP1R07V_CP           (1 << 1)
#  define CPLD1_PWR_GD_XP3R3V_CP            (1 << 0)

#define CPLD1_REG_BACK_PANEL_OFFSET         (0x122)
#  define CPLD1_BACK_PANEL_LED_DP           (1 << 7)
#  define CPLD1_BACK_PANEL_LED_G            (1 << 6)
#  define CPLD1_BACK_PANEL_LED_F            (1 << 5)
#  define CPLD1_BACK_PANEL_LED_E            (1 << 4)
#  define CPLD1_BACK_PANEL_LED_D            (1 << 3)
#  define CPLD1_BACK_PANEL_LED_C            (1 << 2)
#  define CPLD1_BACK_PANEL_LED_B            (1 << 1)
#  define CPLD1_BACK_PANEL_LED_A            (1 << 0)

#define CPLD1_REG_WRITE_PROTECT_CTL_OFFSET  (0x123)
#  define CPLD1_WRITE_PROTECT_SPI1_WP       (1 << 5)
#  define CPLD1_WRITE_PROTECT_SPI0_WP       (1 << 4)
#  define CPLD1_WRITE_PROTECT_SPD2_WP       (1 << 3)
#  define CPLD1_WRITE_PROTECT_SPD1_WP       (1 << 2)
#  define CPLD1_WRITE_PROTECT_NAND_WP       (1 << 1)
#  define CPLD1_WRITE_PROTECT_SYS_EEPROM    (1 << 0)

#define CPLD1_REG_MISC_STATUS_CTL_OFFSET    (0x124)
#  define CPLD1_MISC_UART0_SELECT           (1 << 4)
#  define CPLD1_MISC_USB_CB_PWR_FAULT       (1 << 3)
#  define CPLD1_MISC_BP_PRESENT             (1 << 2)
#  define CPLD1_MISC_PRESENT_MB             (1 << 1)
#  define CPLD1_MISC_SSD_PRESENT            (1 << 0)

#define CPLD1_REG_FAN_PSU_STATUS_OFFSET     (0x125)
#  define CPLD1_FAN_PSU_I2C_SLAVE           (1 << 7)
#  define CPLD1_FAN_PSU_FAN5_PRESENT_L      (1 << 6)
#  define CPLD1_FAN_PSU_FAN4_PRESENT_L      (1 << 5)
#  define CPLD1_FAN_PSU_FAN3_PRESENT_L      (1 << 4)
#  define CPLD1_FAN_PSU_FAN2_PRESENT_L      (1 << 3)
#  define CPLD1_FAN_PSU_FAN1_PRESENT_L      (1 << 2)
#  define CPLD1_FAN_PSU_PSU1_PRESENT_L      (1 << 1)
#  define CPLD1_FAN_PSU_PSU2_PRESENT_L      (1 << 0)

/*
 * CPLD2
 */
#define CPLD2_REG_VERSION_OFFSET            (0x200)
#  define CPLD2_VERSION_H_MASK              (0xF0)
#  define CPLD2_VERSION_H_SHIFT             (4)
#    define CPLD2_VERSION_ID                (0x10)
#  define CPLD2_VERSION_L_MASK              (0x0F)
#  define CPLD2_VERSION_L_SHIFT             (0)

#define CPLD2_REG_SW_SCRATCH_OFFSET         (0x201)

#define CPLD2_REG_I2C_PORT_ID_OFFSET        (0x210)
#  define CPLD2_I2C_BAUD                    (1 << 6)    /* 0:50KHz, 1:100KHz */
#  define CPLD2_I2C_ID_MASK                 (0x1F)

#define CPLD2_REG_I2C_OPCODE_OFFSET         (0x211)
#  define CPLD2_I2C_DATA_LEN_MASK           (0xF0)
#  define CPLD2_I2C_CMD_LEN_MASK            (0x03)

#define CPLD2_REG_I2C_DEV_ADDR_OFFSET       (0x212)
#  define CPLD2_I2C_SLAVE_ADDR_MASK         (0xFE)
#  define CPLD2_I2C_RW_BIT                  (1 << 0)

#define CPLD2_I2C_CMD_BYTE0_OFFSET          (0x213)
#define CPLD2_I2C_CMD_BYTE1_OFFSET          (0x214)
#define CPLD2_I2C_CMD_BYTE2_OFFSET          (0x215)

#define CPLD2_I2C_STATUS_SOFT_RST_OFFSET    (0x216)
#  define CPLD2_I2C_MASTER_ERROR            (1 << 7)
#  define CPLD2_I2C_BUSY                    (1 << 6)
#  define CPLD2_I2C_MASTER_SW_RESET_L       (1 << 0)    /* 0: reset */

#define CPLD2_I2C_WRITE_DATA_BYTE0_OFFSET   (0x220)
#define CPLD2_I2C_WRITE_DATA_BYTE1_OFFSET   (0x221)
#define CPLD2_I2C_WRITE_DATA_BYTE2_OFFSET   (0x222)
#define CPLD2_I2C_WRITE_DATA_BYTE3_OFFSET   (0x223)
#define CPLD2_I2C_WRITE_DATA_BYTE4_OFFSET   (0x224)
#define CPLD2_I2C_WRITE_DATA_BYTE5_OFFSET   (0x225)
#define CPLD2_I2C_WRITE_DATA_BYTE6_OFFSET   (0x226)
#define CPLD2_I2C_WRITE_DATA_BYTE7_OFFSET   (0x227)

#define CPLD2_I2C_READ_DATA_BYTE0_OFFSET    (0x230)
#define CPLD2_I2C_READ_DATA_BYTE1_OFFSET    (0x231)
#define CPLD2_I2C_READ_DATA_BYTE2_OFFSET    (0x232)
#define CPLD2_I2C_READ_DATA_BYTE3_OFFSET    (0x233)
#define CPLD2_I2C_READ_DATA_BYTE4_OFFSET    (0x234)
#define CPLD2_I2C_READ_DATA_BYTE5_OFFSET    (0x235)
#define CPLD2_I2C_READ_DATA_BYTE6_OFFSET    (0x236)
#define CPLD2_I2C_READ_DATA_BYTE7_OFFSET    (0x237)

#define CPLD2_SFP_1_8_RX_LOS_REGISTER       (0x246)
# define CPLD2_RX_LOS_PORT8_L               (1 << 7)
# define CPLD2_RX_LOS_PORT7_L               (1 << 6)
# define CPLD2_RX_LOS_PORT6_L               (1 << 5)
# define CPLD2_RX_LOS_PORT5_L               (1 << 4)
# define CPLD2_RX_LOS_PORT4_L               (1 << 3)
# define CPLD2_RX_LOS_PORT3_L               (1 << 2)
# define CPLD2_RX_LOS_PORT2_L               (1 << 1)
# define CPLD2_RX_LOS_PORT1_L               (1 << 0)

#define CPLD2_SFP_9_16_RX_LOS_REGISTER      (0x247)
# define CPLD2_RX_LOS_PORT16_L              (1 << 7)
# define CPLD2_RX_LOS_PORT15_L              (1 << 6)
# define CPLD2_RX_LOS_PORT14_L              (1 << 5)
# define CPLD2_RX_LOS_PORT13_L              (1 << 4)
# define CPLD2_RX_LOS_PORT12_L              (1 << 3)
# define CPLD2_RX_LOS_PORT11_L              (1 << 2)
# define CPLD2_RX_LOS_PORT10_L              (1 << 1)
# define CPLD2_RX_LOS_PORT9_L               (1 << 0)

#define CPLD2_SFP_17_18_RX_LOS_REGISTER     (0x248)
# define CPLD2_RX_LOS_PORT18_L              (1 << 1)
# define CPLD2_RX_LOS_PORT17_L              (1 << 0)

#define CPLD2_SFP_1_8_TX_DISABLE_REGISTER   (0x250)
# define CPLD2_TX_DISABLE_PORT8             (1 << 7)
# define CPLD2_TX_DISABLE_PORT7             (1 << 6)
# define CPLD2_TX_DISABLE_PORT6             (1 << 5)
# define CPLD2_TX_DISABLE_PORT5             (1 << 4)
# define CPLD2_TX_DISABLE_PORT4             (1 << 3)
# define CPLD2_TX_DISABLE_PORT3             (1 << 2)
# define CPLD2_TX_DISABLE_PORT2             (1 << 1)
# define CPLD2_TX_DISABLE_PORT1             (1 << 0)

#define CPLD2_SFP_9_16_TX_DISABLE_REGISTER  (0x251)
# define CPLD2_TX_DISABLE_PORT16            (1 << 7)
# define CPLD2_TX_DISABLE_PORT15            (1 << 6)
# define CPLD2_TX_DISABLE_PORT14            (1 << 5)
# define CPLD2_TX_DISABLE_PORT13            (1 << 4)
# define CPLD2_TX_DISABLE_PORT12            (1 << 3)
# define CPLD2_TX_DISABLE_PORT11            (1 << 2)
# define CPLD2_TX_DISABLE_PORT10            (1 << 1)
# define CPLD2_TX_DISABLE_PORT9             (1 << 0)

#define CPLD2_SFP_17_18_TX_DISABLE_REGISTER (0x252)
# define CPLD2_TX_DISABLE_PORT18            (1 << 1)
# define CPLD2_TX_DISABLE_PORT17            (1 << 0)

#define CPLD2_SFP_1_8_TX_FAULT_REGISTER     (0x256)
# define CPLD2_TX_FAULT_PORT8_L             (1 << 7)
# define CPLD2_TX_FAULT_PORT7_L             (1 << 6)
# define CPLD2_TX_FAULT_PORT6_L             (1 << 5)
# define CPLD2_TX_FAULT_PORT5_L             (1 << 4)
# define CPLD2_TX_FAULT_PORT4_L             (1 << 3)
# define CPLD2_TX_FAULT_PORT3_L             (1 << 2)
# define CPLD2_TX_FAULT_PORT2_L             (1 << 1)
# define CPLD2_TX_FAULT_PORT1_L             (1 << 0)

#define CPLD2_SFP_9_16_TX_FAULT_REGISTER    (0x257)
# define CPLD2_TX_FAULT_PORT16_L            (1 << 7)
# define CPLD2_TX_FAULT_PORT15_L            (1 << 6)
# define CPLD2_TX_FAULT_PORT14_L            (1 << 5)
# define CPLD2_TX_FAULT_PORT13_L            (1 << 4)
# define CPLD2_TX_FAULT_PORT12_L            (1 << 3)
# define CPLD2_TX_FAULT_PORT11_L            (1 << 2)
# define CPLD2_TX_FAULT_PORT10_L            (1 << 1)
# define CPLD2_TX_FAULT_PORT9_L             (1 << 0)

#define CPLD2_SFP_17_18_TX_FAULT_REGISTER   (0x258)
# define CPLD2_TX_FAULT_PORT18_L            (1 << 1)
# define CPLD2_TX_FAULT_PORT17_L            (1 << 0)

#define CPLD2_SFP_1_8_PRESENT_REGISTER      (0x259)
# define CPLD2_PRESENT_PORT8_L              (1 << 7)
# define CPLD2_PRESENT_PORT7_L              (1 << 6)
# define CPLD2_PRESENT_PORT6_L              (1 << 5)
# define CPLD2_PRESENT_PORT5_L              (1 << 4)
# define CPLD2_PRESENT_PORT4_L              (1 << 3)
# define CPLD2_PRESENT_PORT3_L              (1 << 2)
# define CPLD2_PRESENT_PORT2_L              (1 << 1)
# define CPLD2_PRESENT_PORT1_L              (1 << 0)

#define CPLD2_SFP_9_16_PRESENT_REGISTER     (0x25a)
# define CPLD2_PRESENT_PORT16_L             (1 << 7)
# define CPLD2_PRESENT_PORT15_L             (1 << 6)
# define CPLD2_PRESENT_PORT14_L             (1 << 5)
# define CPLD2_PRESENT_PORT13_L             (1 << 4)
# define CPLD2_PRESENT_PORT12_L             (1 << 3)
# define CPLD2_PRESENT_PORT11_L             (1 << 2)
# define CPLD2_PRESENT_PORT10_L             (1 << 1)
# define CPLD2_PRESENT_PORT9_L              (1 << 0)

#define CPLD2_SFP_17_18_PRESENT_REGISTER    (0x25b)
# define CPLD2_PRESENT_PORT18_L             (1 << 1)
# define CPLD2_PRESENT_PORT17_L             (1 << 0)
/*
 * CPLD3
 */
#define CPLD3_REG_VERSION_OFFSET            (0x280)
#  define CPLD3_VERSION_H_MASK              (0xF0)
#  define CPLD3_VERSION_H_SHIFT             (4)
#define CPLD3_VERSION_ID                    (0x30)
#  define CPLD3_VERSION_L_MASK              (0x0F)
#  define CPLD3_VERSION_L_SHIFT             (0)

#define CPLD3_REG_SW_SCRATCH_OFFSET         (0x281)

#define CPLD3_REG_I2C_PORT_ID_OFFSET        (0x290)
#  define CPLD3_REG_BAUD                    (1 << 6)    /* 0:50KHz, 1:100KHz */
#  define CPLD3_I2C_FUNC_ID_MASK            (0x1F)

#define CPLD3_REG_I2C_OPCODE_OFFSET         (0x291)
#  define CPLD3_I2C_DATA_LEN_MASK           (0xF0)
#  define CPLD3_I2C_CMD_LEN_MASK            (0x03)

#define CPLD3_REG_I2C_DEV_ADDR_OFFSET       (0x292)
#  define CPLD3_I2C_SLAVE_ADDR_MASK         (0xFE)
#  define CPLD3_I2C_RW_BIT                  (1 << 0)

#define CPLD3_I2C_CMD_BYTE0_OFFSET          (0x293)
#define CPLD3_I2C_CMD_BYTE1_OFFSET          (0x294)
#define CPLD3_I2C_CMD_BYTE2_OFFSET          (0x295)

#define CPLD3_I2C_STATUS_SOFT_RST_OFFSET    (0x296)
#  define CPLD3_I2C_MASTER_ERROR            (1 << 7)
#  define CPLD3_I2C_BUSY                    (1 << 6)
#  define CPLD3_I2C_MASTER_SW_RESET_L       (1 << 0)    /* 0: reset */

#define CPLD3_I2C_WRITE_DATA_BYTE0_OFFSET   (0x2A0)
#define CPLD3_I2C_WRITE_DATA_BYTE1_OFFSET   (0x2A1)
#define CPLD3_I2C_WRITE_DATA_BYTE2_OFFSET   (0x2A2)
#define CPLD3_I2C_WRITE_DATA_BYTE3_OFFSET   (0x2A3)
#define CPLD3_I2C_WRITE_DATA_BYTE4_OFFSET   (0x2A4)
#define CPLD3_I2C_WRITE_DATA_BYTE5_OFFSET   (0x2A5)
#define CPLD3_I2C_WRITE_DATA_BYTE6_OFFSET   (0x2A6)
#define CPLD3_I2C_WRITE_DATA_BYTE7_OFFSET   (0x2A7)

#define CPLD3_I2C_READ_DATA_BYTE0_OFFSET    (0x2B0)
#define CPLD3_I2C_READ_DATA_BYTE1_OFFSET    (0x2B1)
#define CPLD3_I2C_READ_DATA_BYTE2_OFFSET    (0x2B2)
#define CPLD3_I2C_READ_DATA_BYTE3_OFFSET    (0x2B3)
#define CPLD3_I2C_READ_DATA_BYTE4_OFFSET    (0x2B4)
#define CPLD3_I2C_READ_DATA_BYTE5_OFFSET    (0x2B5)
#define CPLD3_I2C_READ_DATA_BYTE6_OFFSET    (0x2B6)
#define CPLD3_I2C_READ_DATA_BYTE7_OFFSET    (0x2B7)

#define CPLD3_SFP_19_26_RX_LOS_REGISTER     (0x2c0)
# define CPLD3_RX_LOS_PORT26                (1 << 7)
# define CPLD3_RX_LOS_PORT25                (1 << 6)
# define CPLD3_RX_LOS_PORT24                (1 << 5)
# define CPLD3_RX_LOS_PORT23                (1 << 4)
# define CPLD3_RX_LOS_PORT22                (1 << 3)
# define CPLD3_RX_LOS_PORT21                (1 << 2)
# define CPLD3_RX_LOS_PORT20                (1 << 1)
# define CPLD3_RX_LOS_PORT19                (1 << 0)

#define CPLD3_SFP_27_34_RX_LOS_REGISTER     (0x2c1)
# define CPLD3_RX_LOS_PORT34                (1 << 7)
# define CPLD3_RX_LOS_PORT33                (1 << 6)
# define CPLD3_RX_LOS_PORT32                (1 << 5)
# define CPLD3_RX_LOS_PORT31                (1 << 4)
# define CPLD3_RX_LOS_PORT30                (1 << 3)
# define CPLD3_RX_LOS_PORT29                (1 << 2)
# define CPLD3_RX_LOS_PORT28                (1 << 1)
# define CPLD3_RX_LOS_PORT27                (1 << 0)

#define CPLD3_SFP_35_36_RX_LOS_REGISTER     (0x2c2)
# define CPLD3_RX_LOS_PORT36                (1 << 1)
# define CPLD3_RX_LOS_PORT35                (1 << 0)

#define CPLD3_SFP_19_26_TX_DISABLE_REGISTER (0x2d0)
# define CPLD3_TX_DISABLE_PORT26            (1 << 7)
# define CPLD3_TX_DISABLE_PORT25            (1 << 6)
# define CPLD3_TX_DISABLE_PORT24            (1 << 5)
# define CPLD3_TX_DISABLE_PORT23            (1 << 4)
# define CPLD3_TX_DISABLE_PORT22            (1 << 3)
# define CPLD3_TX_DISABLE_PORT21            (1 << 2)
# define CPLD3_TX_DISABLE_PORT20            (1 << 1)
# define CPLD3_TX_DISABLE_PORT19            (1 << 0)

#define CPLD3_SFP_27_34_TX_DISABLE_REGISTER (0x2d1)
# define CPLD3_TX_DISABLE_PORT34            (1 << 7)
# define CPLD3_TX_DISABLE_PORT33            (1 << 6)
# define CPLD3_TX_DISABLE_PORT32            (1 << 5)
# define CPLD3_TX_DISABLE_PORT31            (1 << 4)
# define CPLD3_TX_DISABLE_PORT30            (1 << 3)
# define CPLD3_TX_DISABLE_PORT29            (1 << 2)
# define CPLD3_TX_DISABLE_PORT28            (1 << 1)
# define CPLD3_TX_DISABLE_PORT27            (1 << 0)

#define CPLD3_SFP_35_36_TX_DISABLE_REGISTER (0x2d2)
# define CPLD3_TX_DISABLE_PORT36            (1 << 1)
# define CPLD3_TX_DISABLE_PORT35            (1 << 0)

#define CPLD3_SFP_19_26_TX_FAULT_REGISTER   (0x2d6)
# define CPLD3_TX_FAULT_PORT26_L            (1 << 7)
# define CPLD3_TX_FAULT_PORT25_L            (1 << 6)
# define CPLD3_TX_FAULT_PORT24_L            (1 << 5)
# define CPLD3_TX_FAULT_PORT23_L            (1 << 4)
# define CPLD3_TX_FAULT_PORT22_L            (1 << 3)
# define CPLD3_TX_FAULT_PORT21_L            (1 << 2)
# define CPLD3_TX_FAULT_PORT20_L            (1 << 1)
# define CPLD3_TX_FAULT_PORT19_L            (1 << 0)

#define CPLD3_SFP_27_34_TX_FAULT_REGISTER   (0x2d7)
# define CPLD3_TX_FAULT_PORT34_L            (1 << 7)
# define CPLD3_TX_FAULT_PORT33_L            (1 << 6)
# define CPLD3_TX_FAULT_PORT32_L            (1 << 5)
# define CPLD3_TX_FAULT_PORT31_L            (1 << 4)
# define CPLD3_TX_FAULT_PORT30_L            (1 << 3)
# define CPLD3_TX_FAULT_PORT29_L            (1 << 2)
# define CPLD3_TX_FAULT_PORT28_L            (1 << 1)
# define CPLD3_TX_FAULT_PORT27_L            (1 << 0)

#define CPLD3_SFP_35_36_TX_FAULT_REGISTER   (0x2d8)
# define CPLD3_TX_FAULT_PORT36_L            (1 << 7)
# define CPLD3_TX_FAULT_PORT35_L            (1 << 6)

#define CPLD3_SFP_19_26_PRESENT_REGISTER    (0x2d9)
# define CPLD3_PRESENT_PORT26_L             (1 << 7)
# define CPLD3_PRESENT_PORT25_L             (1 << 6)
# define CPLD3_PRESENT_PORT24_L             (1 << 5)
# define CPLD3_PRESENT_PORT23_L             (1 << 4)
# define CPLD3_PRESENT_PORT22_L             (1 << 3)
# define CPLD3_PRESENT_PORT21_L             (1 << 2)
# define CPLD3_PRESENT_PORT20_L             (1 << 1)
# define CPLD3_PRESENT_PORT19_L             (1 << 0)

#define CPLD3_SFP_27_34_PRESENT_REGISTER    (0x2da)
# define CPLD3_PRESENT_PORT34_L             (1 << 7)
# define CPLD3_PRESENT_PORT33_L             (1 << 6)
# define CPLD3_PRESENT_PORT32_L             (1 << 5)
# define CPLD3_PRESENT_PORT31_L             (1 << 4)
# define CPLD3_PRESENT_PORT30_L             (1 << 3)
# define CPLD3_PRESENT_PORT29_L             (1 << 2)
# define CPLD3_PRESENT_PORT28_L             (1 << 1)
# define CPLD3_PRESENT_PORT27_L             (1 << 0)

#define CPLD3_SFP_35_36_PRESENT_REGISTER    (0x2db)
# define CPLD3_PRESENT_PORT36_L             (1 << 7)
# define CPLD3_PRESENT_PORT35_L             (1 << 6)

/*
 * CPLD4
 */
#define CPLD4_REG_VERSION_OFFSET            (0x300)
#  define CPLD4_VERSION_H_MASK              (0xF0)
#  define CPLD4_VERSION_H_SHIFT             (4)
#    define CPLD4_VERSION_ID                (0x10)
#  define CPLD4_VERSION_L_MASK              (0x0F)
#  define CPLD4_VERSION_L_SHIFT             (0)

#define CPLD4_REG_SW_SCRATCH_OFFSET         (0x301)

#define CPLD4_REG_RESET_CTL_OFFSET          (0x302)
#  define CPLD4_RESET_CTL_BCM56850_L        (1 << 3)    /* 0: reset */
#  define CPLD4_RESET_CTL_CPLD5_L           (1 << 2)    /* 0: reset */
#  define CPLD4_RESET_CTL_CPLD3_L           (1 << 1)    /* 0: reset */
#  define CPLD4_RESET_CTL_CPLD2_L           (1 << 0)    /* 0: reset */

#define CPLD4_REG_SYS_LED_CTRL_OFFSET       (0x303)
#  define CPLD4_PSU2_LED_L                  (1 << 3)    /* 0:On, 1:Off */
#  define CPLD4_PSU1_LED_L                  (1 << 2)    /* 0:On, 1:Off */
#  define CPLD4_SYS_LED_GREEN               (0x00)
#  define CPLD4_SYS_LED_GREEN_SLOW_BLINK    (0x01)
#  define CPLD4_SYS_LED_GREEN_FAST_BLINK    (0x02)
#  define CPLD4_SYS_LED_OFF                 (0x03)
#  define CPLD4_SYS_LED_MASK                (0x03)

#define CPLD4_REG_INT_PORT_STATUS_OFFSET    (0x305)
#  define CPLD4_INT_1_L                     (1 << 0)    /* 0:int, 1:no int */

#define CPLD4_REG_INT0_SRC_STATUS_OFFSET    (0x306)
#  define CPLD4_CPLD5_RX_LOS_STAT_L         (1 << 2)    /* 0:int, 1:no int */
#  define CPLD4_CPLD3_RX_LOS_STAT_L         (1 << 1)    /* 0:int, 1:no int */
#  define CPLD4_CPLD2_RX_LOS_STAT_L         (1 << 0)    /* 0:int, 1:no int */

#define CPLD4_REG_INT1_SRC_STATUS_OFFSET    (0x307)
#  define CPLD4_QPHY5_STAT_L                (1 << 5)    /* 0:int, 1:no int */
#  define CPLD4_QPHY4_STAT0_L               (1 << 4)    /* 0:int, 1:no int */
#  define CPLD4_QPHY4_STAT1_L               (1 << 3)    /* 0:int, 1:no int */
#  define CPLD4_QPHY3_STAT_L                (1 << 2)    /* 0:int, 1:no int */
#  define CPLD4_QPHY2_STAT_L                (1 << 1)    /* 0:int, 1:no int */
#  define CPLD4_QPHY1_STAT_L                (1 << 0)    /* 0:int, 1:no int */

#define CPLD4_REG_INT2_SRC_STATUS_OFFSET    (0x308)
#  define CPLD4_NCP4200_ALERT_STAT_L        (1 << 2)    /* 0:int, 1:no int */
#  define CPLD4_NCP4200_FAULT_STAT_L        (1 << 1)    /* 0:int, 1:no int */
#  define CPLD4_NCP4200_ROV_ALERT_STAT_L    (1 << 0)    /* 0:int, 1:no int */

#define CPLD4_REG_INT0_SRC_INT_OFFSET       (0x309)
#  define CPLD4_CPLD5_RX_LOS_INT_L          (1 << 2)    /* 0:int, 1:no int */
#  define CPLD4_CPLD3_RX_LOS_INT_L          (1 << 1)    /* 0:int, 1:no int */
#  define CPLD4_CPLD2_RX_LOS_INT_L          (1 << 0)    /* 0:int, 1:no int */

#define CPLD4_REG_INT1_SRC_INT_OFFSET       (0x30A)
#  define CPLD4_QPHY5_INT_L                 (1 << 5)    /* 0:int, 1:no int */
#  define CPLD4_QPHY4_INT0_L                (1 << 4)    /* 0:int, 1:no int */
#  define CPLD4_QPHY4_INT1_L                (1 << 3)    /* 0:int, 1:no int */
#  define CPLD4_QPHY3_INT_L                 (1 << 2)    /* 0:int, 1:no int */
#  define CPLD4_QPHY2_INT_L                 (1 << 1)    /* 0:int, 1:no int */
#  define CPLD4_QPHY1_INT_L                 (1 << 0)    /* 0:int, 1:no int */

#define CPLD4_REG_INT2_SRC_INT_OFFSET       (0x30B)
#  define CPLD4_NCP4200_ALERT_INT_L         (1 << 2)    /* 0:int, 1:no int */
#  define CPLD4_NCP4200_FAULT_INT_L         (1 << 1)    /* 0:int, 1:no int */
#  define CPLD4_NCP4200_ROV_ALERT_INT_L     (1 << 0)    /* 0:int, 1:no int */

#define CPLD4_REG_INT0_SRC_MASK_OFFSET      (0x30C)
#  define CPLD4_CPLD5_RX_LOS_MASK           (1 << 2)    /* 0:no mask, 1:mask */
#  define CPLD4_CPLD3_RX_LOS_MASK           (1 << 1)    /* 0:no mask, 1:mask */
#  define CPLD4_CPLD2_RX_LOS_MASK           (1 << 0)    /* 0:no mask, 1:mask */

#define CPLD4_REG_INT1_SRC_MASK_OFFSET      (0x30D)
#  define CPLD4_QPHY5_MASK                  (1 << 5)    /* 0:no mask, 1:mask */
#  define CPLD4_QPHY4_MASK0                 (1 << 4)    /* 0:no mask, 1:mask */
#  define CPLD4_QPHY4_MASK1                 (1 << 3)    /* 0:no mask, 1:mask */
#  define CPLD4_QPHY3_MASK                  (1 << 2)    /* 0:no mask, 1:mask */
#  define CPLD4_QPHY2_MASK                  (1 << 1)    /* 0:no mask, 1:mask */
#  define CPLD4_QPHY1_MASK                  (1 << 0)    /* 0:no mask, 1:mask */

#define CPLD4_REG_INT2_SRC_MASK_OFFSET      (0x30E)
#  define CPLD4_NCP4200_ALERT_MASK          (1 << 2)    /* 0:no mask, 1:mask */
#  define CPLD4_NCP4200_FAULT_MASK          (1 << 1)    /* 0:no mask, 1:mask */
#  define CPLD4_NCP4200_ROV_ALERT_MASK      (1 << 0)    /* 0:no mask, 1:mask */

#define CPLD4_REG_I2C_PORT_ID_OFFSET        (0x310)
#  define CPLD4_REG_BAUD                    (1 << 6)    /* 0:50KHz, 1:100KHz */
#  define CPLD4_I2C_FUNC_ID_MASK            (0x1F)

#define CPLD4_REG_I2C_OPCODE_OFFSET         (0x311)
#  define CPLD4_I2C_DATA_LEN_MASK           (0xF0)
#  define CPLD4_I2C_CMD_LEN_MASK            (0x03)

#define CPLD4_REG_I2C_DEV_ADDR_OFFSET       (0x312)
#  define CPLD4_I2C_SLAVE_ADDR_MASK         (0xFE)
#  define CPLD4_I2C_RW_BIT                  (1 << 0)

#define CPLD4_I2C_CMD_BYTE0_OFFSET          (0x313)
#define CPLD4_I2C_CMD_BYTE1_OFFSET          (0x314)
#define CPLD4_I2C_CMD_BYTE2_OFFSET          (0x315)

#define CPLD4_I2C_STATUS_SOFT_RST_OFFSET    (0x316)
#  define CPLD4_I2C_MASTER_ERROR            (1 << 7)
#  define CPLD4_I2C_BUSY                    (1 << 6)
#  define CPLD4_I2C_MASTER_SW_RESET_L       (1 << 0)    /* 0: reset */

#define CPLD4_I2C_WRITE_DATA_BYTE0_OFFSET   (0x320)
#define CPLD4_I2C_WRITE_DATA_BYTE1_OFFSET   (0x321)
#define CPLD4_I2C_WRITE_DATA_BYTE2_OFFSET   (0x322)
#define CPLD4_I2C_WRITE_DATA_BYTE3_OFFSET   (0x323)
#define CPLD4_I2C_WRITE_DATA_BYTE4_OFFSET   (0x324)
#define CPLD4_I2C_WRITE_DATA_BYTE5_OFFSET   (0x325)
#define CPLD4_I2C_WRITE_DATA_BYTE6_OFFSET   (0x326)
#define CPLD4_I2C_WRITE_DATA_BYTE7_OFFSET   (0x327)

#define CPLD4_I2C_READ_DATA_BYTE0_OFFSET    (0x330)
#define CPLD4_I2C_READ_DATA_BYTE1_OFFSET    (0x331)
#define CPLD4_I2C_READ_DATA_BYTE2_OFFSET    (0x332)
#define CPLD4_I2C_READ_DATA_BYTE3_OFFSET    (0x333)
#define CPLD4_I2C_READ_DATA_BYTE4_OFFSET    (0x334)
#define CPLD4_I2C_READ_DATA_BYTE5_OFFSET    (0x335)
#define CPLD4_I2C_READ_DATA_BYTE6_OFFSET    (0x336)
#define CPLD4_I2C_READ_DATA_BYTE7_OFFSET    (0x337)

#define CPLD4_QSFP_RESET_CONTROL_REGISTER   (0x360)
# define CPLD4_RESET_CONTROL_MASK           (0x3f)
# define CPLD4_RESET_CONTROL_PORT6_L        (1 << 5)
# define CPLD4_RESET_CONTROL_PORT5_L        (1 << 4)
# define CPLD4_RESET_CONTROL_PORT4_L        (1 << 3)
# define CPLD4_RESET_CONTROL_PORT3_L        (1 << 2)
# define CPLD4_RESET_CONTROL_PORT2_L        (1 << 1)
# define CPLD4_RESET_CONTROL_PORT1_L        (1)

#define CPLD4_QSFP_LP_MODE_REGISTER         (0x361)
# define CPLD4_LP_MODE_MASK                 (0x3f)
# define CPLD4_LP_MODE_PORT6                (1 << 5)
# define CPLD4_LP_MODE_PORT5                (1 << 4)
# define CPLD4_LP_MODE_PORT4                (1 << 3)
# define CPLD4_LP_MODE_PORT3                (1 << 2)
# define CPLD4_LP_MODE_PORT2                (1 << 1)
# define CPLD4_LP_MODE_PORT1                (1)

#define CPLD4_QSFP_PRESENT_REGISTER         (0x362)
# define CPLD4_PRESENT_MASK                 (0x3f)
# define CPLD4_PRESENT_PORT6_L              (1 << 5)
# define CPLD4_PRESENT_PORT5_L              (1 << 4)
# define CPLD4_PRESENT_PORT4_L              (1 << 3)
# define CPLD4_PRESENT_PORT3_L              (1 << 2)
# define CPLD4_PRESENT_PORT2_L              (1 << 1)
# define CPLD4_PRESENT_PORT1_L              (1)

#define CPLD4_QSFP_INTERRUPT_REGISTER       (0x363)
# define CPLD4_INTERRUPT_MASK               (0x3f)
# define CPLD4_INTERRUPT_PORT6              (1 << 5)
# define CPLD4_INTERRUPT_PORT5              (1 << 4)
# define CPLD4_INTERRUPT_PORT4              (1 << 3)
# define CPLD4_INTERRUPT_PORT3              (1 << 2)
# define CPLD4_INTERRUPT_PORT2              (1 << 1)
# define CPLD4_INTERRUPT_PORT1              (1)

/*
 * CPLD5
 */
#define CPLD5_REG_VERSION_OFFSET            (0x380)
#  define CPLD5_VERSION_H_MASK              (0xF0)
#  define CPLD5_VERSION_H_SHIFT             (4)
#    define CPLD5_VERSION_ID                (0x50)
#  define CPLD5_VERSION_L_MASK              (0x0F)
#  define CPLD5_VERSION_L_SHIFT             (0)

#define CPLD5_REG_SW_SCRATCH_OFFSET         (0x381)

#define CPLD5_REG_QSFP_PORT_MODE_OFFSET     (0x382)
#  define CPLD5_QSFP_PORT6_MODE             (1 << 5)    /* 0:4x10G, 1:40G */
#  define CPLD5_QSFP_PORT5_MODE             (1 << 4)    /* 0:4x10G, 1:40G */
#  define CPLD5_QSFP_PORT4_MODE             (1 << 3)    /* 0:4x10G, 1:40G */
#  define CPLD5_QSFP_PORT3_MODE             (1 << 2)    /* 0:4x10G, 1:40G */
#  define CPLD5_QSFP_PORT2_MODE             (1 << 1)    /* 0:4x10G, 1:40G */
#  define CPLD5_QSFP_PORT1_MODE             (1 << 0)    /* 0:4x10G, 1:40G */

#define CPLD5_REG_PORT_1_2_ERROR_LED_OFFSET (0x383)
#  define CPLD5_PORT2_4_RED_LED_L           (1 << 7)    /* 0:Error, 1:OK */
#  define CPLD5_PORT2_3_RED_LED_L           (1 << 6)    /* 0:Error, 1:OK */
#  define CPLD5_PORT2_2_RED_LED_L           (1 << 5)    /* 0:Error, 1:OK */
#  define CPLD5_PORT2_1_RED_LED_L           (1 << 4)    /* 0:Error, 1:OK */
#  define CPLD5_PORT1_4_RED_LED_L           (1 << 3)    /* 0:Error, 1:OK */
#  define CPLD5_PORT1_3_RED_LED_L           (1 << 2)    /* 0:Error, 1:OK */
#  define CPLD5_PORT1_2_RED_LED_L           (1 << 1)    /* 0:Error, 1:OK */
#  define CPLD5_PORT1_1_RED_LED_L           (1 << 0)    /* 0:Error, 1:OK */

#define CPLD5_REG_PORT_3_4_ERROR_LED_OFFSET (0x384)
#  define CPLD5_PORT4_4_RED_LED_L           (1 << 7)    /* 0:Error, 1:OK */
#  define CPLD5_PORT4_3_RED_LED_L           (1 << 6)    /* 0:Error, 1:OK */
#  define CPLD5_PORT4_2_RED_LED_L           (1 << 5)    /* 0:Error, 1:OK */
#  define CPLD5_PORT4_1_RED_LED_L           (1 << 4)    /* 0:Error, 1:OK */
#  define CPLD5_PORT3_4_RED_LED_L           (1 << 3)    /* 0:Error, 1:OK */
#  define CPLD5_PORT3_3_RED_LED_L           (1 << 2)    /* 0:Error, 1:OK */
#  define CPLD5_PORT3_2_RED_LED_L           (1 << 1)    /* 0:Error, 1:OK */
#  define CPLD5_PORT3_1_RED_LED_L           (1 << 0)    /* 0:Error, 1:OK */

#define CPLD5_REG_PORT_5_6_ERROR_LED_OFFSET (0x385)
#  define CPLD5_PORT6_4_RED_LED_L           (1 << 7)    /* 0:Error, 1:OK */
#  define CPLD5_PORT6_3_RED_LED_L           (1 << 6)    /* 0:Error, 1:OK */
#  define CPLD5_PORT6_2_RED_LED_L           (1 << 5)    /* 0:Error, 1:OK */
#  define CPLD5_PORT6_1_RED_LED_L           (1 << 4)    /* 0:Error, 1:OK */
#  define CPLD5_PORT5_4_RED_LED_L           (1 << 3)    /* 0:Error, 1:OK */
#  define CPLD5_PORT5_3_RED_LED_L           (1 << 2)    /* 0:Error, 1:OK */
#  define CPLD5_PORT5_2_RED_LED_L           (1 << 1)    /* 0:Error, 1:OK */
#  define CPLD5_PORT5_1_RED_LED_L           (1 << 0)    /* 0:Error, 1:OK */

#define CPLD5_REG_I2C_PORT_ID_OFFSET        (0x390)
#  define CPLD5_REG_BAUD                    (1 << 6)    /* 0:50KHz, 1:100KHz */
#  define CPLD5_I2C_FUNC_ID_MASK            (0x1F)

#define CPLD5_REG_I2C_OPCODE_OFFSET         (0x391)
#  define CPLD5_I2C_DATA_LEN_MASK           (0xF0)
#  define CPLD5_I2C_CMD_LEN_MASK            (0x03)

#define CPLD5_REG_I2C_DEV_ADDR_OFFSET       (0x392)
#  define CPLD5_I2C_SLAVE_ADDR_MASK         (0xFE)
#  define CPLD5_I2C_RW_BIT                  (1 << 0)

#define CPLD5_I2C_CMD_BYTE0_OFFSET          (0x393)
#define CPLD5_I2C_CMD_BYTE1_OFFSET          (0x394)
#define CPLD5_I2C_CMD_BYTE2_OFFSET          (0x395)

#define CPLD5_I2C_STATUS_SOFT_RST_OFFSET    (0x396)
#  define CPLD5_I2C_MASTER_ERROR            (1 << 7)
#  define CPLD5_I2C_BUSY                    (1 << 6)
#  define CPLD5_I2C_MASTER_SW_RESET_L       (1 << 0)    /* 0: reset */

#define CPLD5_I2C_WRITE_DATA_BYTE0_OFFSET   (0x3A0)
#define CPLD5_I2C_WRITE_DATA_BYTE1_OFFSET   (0x3A1)
#define CPLD5_I2C_WRITE_DATA_BYTE2_OFFSET   (0x3A2)
#define CPLD5_I2C_WRITE_DATA_BYTE3_OFFSET   (0x3A3)
#define CPLD5_I2C_WRITE_DATA_BYTE4_OFFSET   (0x3A4)
#define CPLD5_I2C_WRITE_DATA_BYTE5_OFFSET   (0x3A5)
#define CPLD5_I2C_WRITE_DATA_BYTE6_OFFSET   (0x3A6)
#define CPLD5_I2C_WRITE_DATA_BYTE7_OFFSET   (0x3A7)

#define CPLD5_I2C_READ_DATA_BYTE0_OFFSET    (0x3B0)
#define CPLD5_I2C_READ_DATA_BYTE1_OFFSET    (0x3B1)
#define CPLD5_I2C_READ_DATA_BYTE2_OFFSET    (0x3B2)
#define CPLD5_I2C_READ_DATA_BYTE3_OFFSET    (0x3B3)
#define CPLD5_I2C_READ_DATA_BYTE4_OFFSET    (0x3B4)
#define CPLD5_I2C_READ_DATA_BYTE5_OFFSET    (0x3B5)
#define CPLD5_I2C_READ_DATA_BYTE6_OFFSET    (0x3B6)
#define CPLD5_I2C_READ_DATA_BYTE7_OFFSET    (0x3B7)

#define CPLD5_SFP_37_44_RX_LOS_REGISTER     (0x3c0)
# define CPLD5_RX_LOS_PORT44                (1 << 7)
# define CPLD5_RX_LOS_PORT43                (1 << 6)
# define CPLD5_RX_LOS_PORT42                (1 << 5)
# define CPLD5_RX_LOS_PORT41                (1 << 4)
# define CPLD5_RX_LOS_PORT40                (1 << 3)
# define CPLD5_RX_LOS_PORT39                (1 << 2)
# define CPLD5_RX_LOS_PORT38                (1 << 1)
# define CPLD5_RX_LOS_PORT37                (1 << 0)

#define CPLD5_SFP_45_48_RX_LOS_REGISTER     (0x3c1)
# define CPLD5_RX_LOS_PORT48                (1 << 3)
# define CPLD5_RX_LOS_PORT47                (1 << 2)
# define CPLD5_RX_LOS_PORT46                (1 << 1)
# define CPLD5_RX_LOS_PORT45                (1 << 0)

#define CPLD5_SFP_37_44_TX_DISABLE_REGISTER (0x3d0)
# define CPLD5_TX_DISABLE_PORT44            (1 << 7)
# define CPLD5_TX_DISABLE_PORT43            (1 << 6)
# define CPLD5_TX_DISABLE_PORT42            (1 << 5)
# define CPLD5_TX_DISABLE_PORT41            (1 << 4)
# define CPLD5_TX_DISABLE_PORT40            (1 << 3)
# define CPLD5_TX_DISABLE_PORT39            (1 << 2)
# define CPLD5_TX_DISABLE_PORT38            (1 << 1)
# define CPLD5_TX_DISABLE_PORT37            (1 << 0)

#define CPLD5_SFP_45_48_TX_DISABLE_REGISTER (0x3d1)
# define CPLD5_TX_DISABLE_PORT48            (1 << 3)
# define CPLD5_TX_DISABLE_PORT47            (1 << 2)
# define CPLD5_TX_DISABLE_PORT46            (1 << 1)
# define CPLD5_TX_DISABLE_PORT45            (1 << 0)

#define CPLD5_SFP_37_44_TX_FAULT_REGISTER   (0x3d4)
# define CPLD5_TX_FAULT_PORT44_L            (1 << 7)
# define CPLD5_TX_FAULT_PORT43_L            (1 << 6)
# define CPLD5_TX_FAULT_PORT42_L            (1 << 5)
# define CPLD5_TX_FAULT_PORT41_L            (1 << 4)
# define CPLD5_TX_FAULT_PORT40_L            (1 << 3)
# define CPLD5_TX_FAULT_PORT39_L            (1 << 2)
# define CPLD5_TX_FAULT_PORT38_L            (1 << 1)
# define CPLD5_TX_FAULT_PORT37_L            (1 << 0)

#define CPLD5_SFP_45_48_TX_FAULT_REGISTER   (0x3d5)
# define CPLD5_TX_FAULT_PORT48_L            (1 << 3)
# define CPLD5_TX_FAULT_PORT47_L            (1 << 2)
# define CPLD5_TX_FAULT_PORT46_L            (1 << 1)
# define CPLD5_TX_FAULT_PORT45_L            (1 << 0)

#define CPLD5_SFP_37_44_PRESENT_REGISTER    (0x3d6)
# define CPLD5_PRESENT_PORT44_L             (1 << 7)
# define CPLD5_PRESENT_PORT43_L             (1 << 6)
# define CPLD5_PRESENT_PORT42_L             (1 << 5)
# define CPLD5_PRESENT_PORT41_L             (1 << 4)
# define CPLD5_PRESENT_PORT40_L             (1 << 3)
# define CPLD5_PRESENT_PORT39_L             (1 << 2)
# define CPLD5_PRESENT_PORT38_L             (1 << 1)
# define CPLD5_PRESENT_PORT37_L             (1 << 0)

#define CPLD5_SFP_45_48_PRESENT_REGISTER    (0x3d7)
# define CPLD5_PRESENT_PORT48_L             (1 << 3)
# define CPLD5_PRESENT_PORT47_L             (1 << 2)
# define CPLD5_PRESENT_PORT46_L             (1 << 1)
# define CPLD5_PRESENT_PORT45_L             (1 << 0)

#endif /* CEL_REDSTONE_XP_H__ */
