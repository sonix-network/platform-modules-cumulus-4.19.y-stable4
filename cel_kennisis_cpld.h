/*
 * Celestica Kennisis CPLD Platform Definitions
 *
 * Curt Brune <curt@cumulusnetworks.com>
 * Vidya Ravipati <vidya@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef CEL_KENNISIS_H__
#define CEL_KENNISIS_H__

/*------------------------------------------------------------------------------
 *
 * register info from R0646-M0010-01 Rev1.0 Kennisis Functional Spec_121911.pdf
 */

#define CPLD_REG_VERSION_OFFSET		(0x00)
#  define CPLD_VERSION_H_MASK		(0xF0)
#  define CPLD_VERSION_H_SHIFT		(4)
#  define CPLD_VERSION_L_MASK		(0x0F)
#  define CPLD_VERSION_L_SHIFT		(0)

#define CPLD_REG_SW_SCRATCH_OFFSET	(0x02)

#define CPLD_REG_RESET_CTRL_1_OFFSET	(0x04)	/* all resets active low */
#  define CPLD_RESET_BCM56634_L			(1 << 7) /* Triumph PHY */
#  define CPLD_RESET_BCM54616S_L		(1 << 6) /* Front panel mgmt PHY */
#  define CPLD_RESET_NOR_FLASH_L		(1 << 5)
#  define CPLD_RESET_USB_PHY_L			(1 << 3)
#  define CPLD_RESET_BCM8747_L			(1 << 1) /* 10GE PHY */

#define CPLD_REG_RESET_CTRL_2_OFFSET	(0x06)	/* all resets active low */
#  define CPLD_RESET_BCM54680_1_L		(1 << 7) /* 8x1G PHY BCM54280  */
#  define CPLD_RESET_BCM54680_2_L		(1 << 6) /* 8x1G PHY BCM54280  */
#  define CPLD_RESET_BCM54680_3_L		(1 << 5) /* 8x1G PHY BCM54280  */
#  define CPLD_RESET_BCM54680_4_L		(1 << 4) /* 8x1G PHY BCM54280  */
#  define CPLD_RESET_BCM54680_5_L		(1 << 3) /* 8x1G PHY BCM54280  */
#  define CPLD_RESET_BCM54680_6_L		(1 << 2) /* 8x1G PHY BCM54280  */
#  define CPLD_RESET_DDR3_L				(1 << 1)

#define CPLD_REG_RESET_CTRL_0_OFFSET	(0x08)	/* all resets active low */
#  define CPLD_RESET_CPU_SYS_L			(1 << 2) /* Control System Reset */
#  define CPLD_RESET_WATCH_DOG_MASK		(1 << 1) /* Watchdog Reset */
#  define CPLD_RESET_CPU_SRST_L			(1 << 0) /* P2020 SW Reset */

#define CPLD_REG_SYS_LED_CTRL_OFFSET		(0x0A)
#  define CPLD_SYS_UBOOT_OK_MASK			(0x78)
#    define CPLD_SYS_UBOOT_OK				(0x50)
#  define CPLD_SYS_LED_MASK					(0x07)
#    define CPLD_SYS_LED_GREEN_SLOW_BLINK	(0x00)
#    define CPLD_SYS_LED_GREEN_FAST_BLINK	(0x01)
#    define CPLD_SYS_LED_GREEN				(0x02)
#    define CPLD_SYS_LED_RED_SLOW_BLINK		(0x03)
#    define CPLD_SYS_LED_RED_FAST_BLINK		(0x04)
#    define CPLD_SYS_LED_RED				(0x05)

#define CPLD_REG_RTC_STATE_OFFSET		(0x0E)
#  define CPLD_3V_PWR_GOOD              (1 << 7)
#  define CPLD_EMC2305_ALARM_STATUS		(1 << 6)
#  define CPLD_RTC_INTA_STATUS			(1 << 5)
#  define CPLD_RTC_INTB_STATUS			(1 << 4)
#  define CPLD_EMC2305_ALARM_CHANGE		(1 << 3)
#  define CPLD_RTC_INTA_CHANGE			(1 << 2)
#  define CPLD_RTC_INTB_CHANGE			(1 << 1)

#define CPLD_REG_TEMP_STATUS_OFFSET	(0x10) /* TEMP Sensor Status */
#  define CPLD_TEMP_STATUS_8_OK		(1 << 7) /* 0 - Over temp, 1- below temp */
#  define CPLD_TEMP_STATUS_7_OK		(1 << 6)
#  define CPLD_TEMP_STATUS_6_OK		(1 << 5)
#  define CPLD_TEMP_STATUS_5_OK		(1 << 4)
#  define CPLD_TEMP_STATUS_4_OK		(1 << 3)
#  define CPLD_TEMP_STATUS_3_OK		(1 << 2)
#  define CPLD_TEMP_STATUS_2_OK		(1 << 1)
#  define CPLD_TEMP_STATUS_1_OK		(1 << 0)

#define CPLD_REG_TEMP_INT_OFFSET	(0x12) /* TEMP Sensor Interrupt */
#  define CPLD_TEMP_INT_8_L			(1 << 7) /* 1 - Over temp, 0 - below temp */
#  define CPLD_TEMP_INT_7_L			(1 << 6)
#  define CPLD_TEMP_INT_6_L			(1 << 5)
#  define CPLD_TEMP_INT_5_L			(1 << 4)
#  define CPLD_TEMP_INT_4_L			(1 << 3)
#  define CPLD_TEMP_INT_3_L			(1 << 2)
#  define CPLD_TEMP_INT_2_L			(1 << 1)
#  define CPLD_TEMP_INT_1_L			(1 << 0)

#define CPLD_REG_SFP_DRVR_CTRL_OFFSET	(0x14) /* SFP Driver Control */
#  define CPLD_SFP_TXONOFF_1		(1 << 3) /* BCM8747 PMD Transmit DRVR */
#  define CPLD_SFP_TXONOFF_2		(1 << 2)
#  define CPLD_SFP_TXONOFF_3		(1 << 1)
#  define CPLD_SFP_TXONOFF_4		(1 << 0)

#define CPLD_REG_SFP_STATUS_1_OFFSET	(0x18) /* SFP Status Register 1 */
#  define CPLD_SFP_PCMULK_1		(1 << 7) /* BCM8747 PMD CMU lock detect */
#  define CPLD_SFP_PCMULK_2		(1 << 6)
#  define CPLD_SFP_PCMULK_3		(1 << 5)
#  define CPLD_SFP_PCMULK_4		(1 << 4)
#  define CPLD_SFP_PCDRLK_1		(1 << 3) /* BCM8747 PMD CDR lock detect */
#  define CPLD_SFP_PCDRLK_2		(1 << 2)
#  define CPLD_SFP_PCDRLK_3		(1 << 1)
#  define CPLD_SFP_PCDRLK_4		(1 << 0)

#define CPLD_REG_SFP_STATUS_2_OFFSET	(0x1A) /* SFP Status Register 2 */
#  define CPLD_SFP_PLOSB_1_L	(1 << 7) /* BCM8747 PMD LOS */
#  define CPLD_SFP_PLOSB_2_L	(1 << 6)
#  define CPLD_SFP_PLOSB_3_L	(1 << 5)
#  define CPLD_SFP_PLOSB_4_L	(1 << 4)
#  define CPLD_SFP_LASI_1_L		(1 << 3) /* BCM8747 PMD link alarm status INT */
#  define CPLD_SFP_LASI_2_L		(1 << 2)
#  define CPLD_SFP_LASI_3_L		(1 << 1)
#  define CPLD_SFP_LASI_4_L		(1 << 0)

#define CPLD_REG_SFP_INT_1_OFFSET	(0x1C) /* SFP Interrupt Register 1 */
#  define CPLD_SFP_INT_PCMULK_1		(1 << 7) /* BCM8747 PMD CMU lock detect */
#  define CPLD_SFP_INT_PCMULK_2		(1 << 6)
#  define CPLD_SFP_INT_PCMULK_3		(1 << 5)
#  define CPLD_SFP_INT_PCMULK_4		(1 << 4)
#  define CPLD_SFP_INT_PCDRLK_1		(1 << 3) /* BCM8747 PMD CDR lock detect */
#  define CPLD_SFP_INT_PCDRLK_2		(1 << 2)
#  define CPLD_SFP_INT_PCDRLK_3		(1 << 1)
#  define CPLD_SFP_INT_PCDRLK_4		(1 << 0)

#define CPLD_REG_SFP_INT_2_OFFSET	(0x1E) /* SFP Interrupt Register 2 */
#  define CPLD_SFP_PLOSB_1_L	(1 << 7) /* BCM8747 PMD LOS */
#  define CPLD_SFP_PLOSB_2_L	(1 << 6)
#  define CPLD_SFP_PLOSB_3_L	(1 << 5)
#  define CPLD_SFP_PLOSB_4_L	(1 << 4)
#  define CPLD_SFP_LASI_1_L		(1 << 3) /* BCM8747 PMD link alarm status INT */
#  define CPLD_SFP_LASI_2_L		(1 << 2)
#  define CPLD_SFP_LASI_3_L		(1 << 1)
#  define CPLD_SFP_LASI_4_L		(1 << 0)

#define CPLD_REG_PHY_STATUS_OFFSET	(0x20) /* PHY Status */
#  define CPLD_PHY_STATUS_6_L		(1 << 6)
#  define CPLD_PHY_STATUS_5_L		(1 << 5)
#  define CPLD_PHY_STATUS_4_L		(1 << 4)
#  define CPLD_PHY_STATUS_3_L		(1 << 3)
#  define CPLD_PHY_STATUS_2_L		(1 << 2)
#  define CPLD_PHY_STATUS_1_L		(1 << 1)
#  define CPLD_BCM5461_STATUS_L		(1 << 0)

#define CPLD_REG_PHY_INT_OFFSET	(0x22) /* PHY Interrupt */
#  define CPLD_PHY_INT_6		(1 << 6)
#  define CPLD_PHY_INT_5		(1 << 5)
#  define CPLD_PHY_INT_4		(1 << 4)
#  define CPLD_PHY_INT_3		(1 << 3)
#  define CPLD_PHY_INT_2		(1 << 2)
#  define CPLD_PHY_INT_1		(1 << 1)
#  define CPLD_BCM5461_INT		(1 << 0)

#define CPLD_REG_SFP_RS_CTRL_OFFSET	(0x28) /* SFP RS Control */
#  define CPLD_SFP_PLUS_RS4_MASK	(0xC0)
#    define CPLD_SFP_PLUS_RS4_1		(1 << 7) /* 0 - 1G, 1 - 10G */
#    define CPLD_SFP_PLUS_RS4_0		(1 << 6)
#  define CPLD_SFP_PLUS_RS3_MASK	(0x30)
#    define CPLD_SFP_PLUS_RS3_1		(1 << 5)
#    define CPLD_SFP_PLUS_RS3_0		(1 << 4)
#  define CPLD_SFP_PLUS_RS2_MASK	(0x0C)
#    define CPLD_SFP_PLUS_RS2_1		(1 << 3)
#    define CPLD_SFP_PLUS_RS2_0		(1 << 2)
#  define CPLD_SFP_PLUS_RS1_MASK	(0x03)
#    define CPLD_SFP_PLUS_RS1_1		(1 << 1)
#    define CPLD_SFP_PLUS_RS1_0		(1 << 0)

#define CPLD_REG_FAN_STATUS_OFFSET	(0x2A) /* Fan Status */
#  define CPLD_FAN_PRESENT_MASK		(0x07)
#    define CPLD_FAN_5_PRESENT_L	(1 << 4)
#    define CPLD_FAN_4_PRESENT_L	(1 << 3)
#    define CPLD_FAN_3_PRESENT_L	(1 << 2)
#    define CPLD_FAN_2_PRESENT_L	(1 << 1)
#    define CPLD_FAN_1_PRESENT_L	(1 << 0)

#define CPLD_REG_PSU_STATUS_1_OFFSET	(0x2C)
#  define CPLD_PSU_ALERT_1_MASK		(0x0A)
#    define CPLD_PSU_ALERT_1_STATUS	(1 << 3)
#    define CPLD_PSU_ALERT_1_INT	(1 << 1)
#  define CPLD_PSU_ALERT_2_MASK		(0x05)
#    define CPLD_PSU_ALERT_2_STATUS	(1 << 2)
#    define CPLD_PSU_ALERT_2_INT	(1 << 0)

#define CPLD_REG_PSU_STATUS_2_OFFSET   (0x2E)
#  define CPLD_PSU_1_MASK			(0x22)
#    define CPLD_PSU_1_PRESENT_L	(1 << 5)
#    define CPLD_PSU_1_PSON			(1 << 3)
#    define CPLD_PSU_1_OK			(1 << 1)
#  define CPLD_PSU_2_MASK			(0x11)
#    define CPLD_PSU_2_PRESENT_L	(1 << 4)
#    define CPLD_PSU_2_PSON			(1 << 2)
#    define CPLD_PSU_2_OK			(1 << 0)


#define CPLD_REG_FLASH_WRITE_OFFSET	(0x30)
#  define CPLD_FLASH_NAND_WRITE_OK	(1 << 1)
#  define CPLD_FLASH_NOR_WRITE_OK	(1 << 0)

#define CPLD_REG_UBOOT_CFG_STATUS_OFFSET	(0x32)
#  define CPLD_UBOOT_CFG_NOR_FLASH		(0x03)
#  define CPLD_UBOOT_CFG_NAND_FLASH		(0x0C)

#define CPLD_REG_RESET_STATUS_OFFSET	(0x34)
#  define CPLD_RESET_FSM				(1 << 4)
#  define CPLD_RESET_WDO				(1 << 3)
#  define CPLD_RESET_CPU_SW				(1 << 2)
#  define CPLD_RESET_CPU_HW				(1 << 1)
#  define CPLD_RESET_POR				(1 << 0)

#define CPLD_REG_NCP4200_STATUS_OFFSET	(0x36)
#  define CPLD_NCP4200_ALERT_L			(1 << 1)
#  define CPLD_NCP4200_FAULT_L			(1 << 0)

#define CPLD_REG_NCP4200_CTRL_OFFSET	(0x38)
#  define CPLD_NCP4200_PWR_SAVE_CTRL	(1 << 0)

#define CPLD_BOARD_VERSION_OFFSET		(0x3E)
#  define CPLD_BOARD_VERSION_MASK		(0xff)
#  define CPLD_BOARD_VERSION_SHIFT		0

/*
 *  * End register defines.
 *   */

/* Internal routine */
#define TYPE_TX_CTRL            (0)
#define TYPE_CMU_LOCK_DETECT    (1)
#define TYPE_CDR_LOCK_DETECT    (2)
#define TYPE_RX_LOS             (3)
#define TYPE_LNK_ALARM_INT      (4)
#define TYPE_TX_SPEED           (5)


#endif /* CEL_KENNISIS_H__ */
