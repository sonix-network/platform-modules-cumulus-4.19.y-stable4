/*
 * DNI C7448N CPLD Platform Definitions
 *
 *  Puneet Shenoy <puneet@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef DNI_C7448N_H__
#define DNI_C7448N_H__


/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 * register info from C7448N_HW_SPEC_20130109.pdf from DNI dated Jan 10, 2013
 */

#define DNIC7448N_CPLD_BOARD_REV_OFFSET     0x10
#define DNIC7448N_CPLD_BOARD_TYPE_MASK      0xf0
#define DNIC7448N_CPLD_BOARD_TYPE_SHIFT        4
#define DNIC7448N_CPLD_BOARD_REV_MASK       0x0f
#define DNIC7448N_CPLD_BOARD_REV_SHIFT         0

#define DNIC7448N_CPLD_MODULE_CTL_OFFSET    0x11
#define  DNIC7448N_CPLD_MODULE_SLOT1_PWR_L  (1 << 0)

#define DNIC7448N_CPLD_RESET_OFFSET         0x20
#define DNIC7448N_CPLD_RESET_SYSTEM_L       (1 << 7)
#define DNIC7448N_CPLD_RESET_CPU_HARD_L     (1 << 6)
#define DNIC7448N_CPLD_RESET_CPU_SOFT_L     (1 << 5)
#define DNIC7448N_CPLD_RESET_DDR3_L         (1 << 4)
#define DNIC7448N_CPLD_RESET_FLASH_L        (1 << 3)
#define DNIC7448N_CPLD_RESET_NET_SOC_L      (1 << 2)
#define DNIC7448N_CPLD_RESET_NET_PHY_L      (1 << 1)
#define DNIC7448N_CPLD_RESET_OOB_PHY_L      (1 << 0)

#define DNIC7448N_CPLD_BULK_PS_OFFSET       0x30
#define DNIC7448N_CPLD_BULK_PS_1_MASK       0xf0
#define  DNIC7448N_CPLD_BULK_PS_1_PRESENT_L (1 << 7)
#define  DNIC7448N_CPLD_BULK_PS_1_OK_L      (1 << 6)
#define  DNIC7448N_CPLD_BULK_PS_1_ALERT_L   (1 << 5)
#define DNIC7448N_CPLD_BULK_PS_0_MASK       0x0f
#define  DNIC7448N_CPLD_BULK_PS_0_PRESENT_L (1 << 3)
#define  DNIC7448N_CPLD_BULK_PS_0_OK_L      (1 << 2)
#define  DNIC7448N_CPLD_BULK_PS_0_ALERT_L   (1 << 1)

#define DNIC7448N_CPLD_PHY_IRQ_EN_0to7_OFFSET 0x40
#define DNIC7448N_CPLD_PHY7_IRQ_EN_L          (1 << 7)
#define DNIC7448N_CPLD_PHY6_IRQ_EN_L          (1 << 6)
#define DNIC7448N_CPLD_PHY5_IRQ_EN_L          (1 << 5)
#define DNIC7448N_CPLD_PHY4_IRQ_EN_L          (1 << 4)
#define DNIC7448N_CPLD_PHY3_IRQ_EN_L          (1 << 3)
#define DNIC7448N_CPLD_PHY2_IRQ_EN_L          (1 << 2)
#define DNIC7448N_CPLD_PHY1_IRQ_EN_L          (1 << 1)
#define DNIC7448N_CPLD_PHY0_IRQ_EN_L          (1 << 0)

#define DNIC7448N_CPLD_PHY_IRQ_EN_8to15_OFFSET 0x50
#define DNIC7448N_CPLD_QPHY3_IRQ_EN_L         (1 << 7)
#define DNIC7448N_CPLD_QPHY2_IRQ_EN_L         (1 << 6)
#define DNIC7448N_CPLD_QPHY1_IRQ_EN_L         (1 << 5)
#define DNIC7448N_CPLD_QPHY0_IRQ_EN_L         (1 << 4)
#define DNIC7448N_CPLD_PHY11_IRQ_EN_L         (1 << 3)
#define DNIC7448N_CPLD_PHY10_IRQ_EN_L         (1 << 2)
#define DNIC7448N_CPLD_PHY9_IRQ_EN_L          (1 << 1)
#define DNIC7448N_CPLD_PHY8_IRQ_EN_L          (1 << 0)

#define DNIC7448N_CPLD_BOARD_PWR_OFFSET       0x60
#define DNIC7448N_CPLD_FLASH_ERASE            (1 << 7)
#define DNIC7448N_CPLD_FLASH_WRITE            (1 << 6)
#define DNIC7448N_CPLD_CPU_1V_L               (1 << 4)
#define DNIC7448N_CPLD_NET_SOC_1V             (1 << 3)
#define DNIC7448N_CPLD_DDR_1V5                (1 << 2)
#define DNIC7448N_CPLD_PHY_1V_L               (1 << 1)
#define DNIC7448N_CPLD_PHY_1V_R               (1 << 0)

#define DNIC7448N_CPLD_BOARD_PWR_STATUS_OFFSET 0x70
#define DNIC7448N_CPLD_VCC_3V3_GOOD            (1 << 6)
#define DNIC7448N_CPLD_CPU_1V_GOOD_L           (1 << 5)
#define DNIC7448N_CPLD_NET_SOC_1V_GOOD         (1 << 4)
#define DNIC7448N_CPLD_DDR_1V5_GOOD            (1 << 3)
#define DNIC7448N_CPLD_PHY_1V5_GOOD_L          (1 << 2)
#define DNIC7448N_CPLD_PHY_1V_1to24_GOOD       (1 << 1)
#define DNIC7448N_CPLD_PHY_1V_25to52_GOOD      (1 << 0)

#define DNIC7448N_CPLD_SYSTEM_LED_OFFSET       0x61
#define  DNIC7448N_CPLD_STATUS_LED             0xc0
#define   DNIC7448N_CPLD_STATUS_LED_BLUE_BLINK 0x00
#define   DNIC7448N_CPLD_STATUS_LED_BLUE       0x40
#define   DNIC7448N_CPLD_STATUS_LED_RED        0x80
#define   DNIC7448N_CPLD_STATUS_LED_RED_BLINK  0xc0
#define  DNIC7448N_CPLD_LOCATE_LED             0x30
#define   DNIC7448N_CPLD_LOCATE_LED_OFF        0x00
#define   DNIC7448N_CPLD_LOCATE_LED_BLUE_BLINK 0x10
#define   DNIC7448N_CPLD_LOCATE_LED_BLUE       0x20
#define  DNIC7448N_CPLD_DIAG_LED               0x08
#define   DNIC7448N_CPLD_DIAG_LED_OFF          0x00
#define   DNIC7448N_CPLD_DIAG_LED_GREEN        0x08
#define  DNIC7448N_CPLD_TEMP_LED               0x04
#define   DNIC7448N_CPLD_TEMP_LED_RED          0x04
#define   DNIC7448N_CPLD_TEMP_LED_OFF          0x00
#define  DNIC7448N_CPLD_MASTER_LED             0x03
#define   DNIC7448N_CPLD_MASTER_LED_OFF        0x00
#define   DNIC7448N_CPLD_MASTER_LED_BLUE       0x01
#define   DNIC7448N_CPLD_MASTER_LED_GREEN      0x02

#define DNIC7448N_CPLD_FAN_STATUS_OFFSET       0x90
#define  DNIC7448N_CPLD_FAN_0_MASK             0x2c
#define   DNIC7448N_CPLD_FAN_0_PRESENT_L       (1 << 5)
#define   DNIC7448N_CPLD_FAN_0_A_OK            (1 << 3)
#define   DNIC7448N_CPLD_FAN_0_B_OK            (1 << 2)
#define  DNIC7448N_CPLD_FAN_1_MASK             0x13
#define   DNIC7448N_CPLD_FAN_1_PRESENT_L       (1 << 4)
#define   DNIC7448N_CPLD_FAN_1_A_OK            (1 << 1)
#define   DNIC7448N_CPLD_FAN_1_B_OK            (1 << 0)

#define DNIC7448N_CPLD_REV_OFFSET              0xa0

#define DNIC7448N_CPLD_BOARD_IRQ_CTRL_OFFSET   0xb0
#define  DNIC7448N_CPLD_POWER_IRQ_EN_L         (1 << 6)
#define  DNIC7448N_CPLD_PHY_IRQ_EN_L           (3 << 4)
#define  DNIC7448N_CPLD_HOT_SWAP_IRQ_EN_L      (1 << 3)
#define  DNIC7448N_CPLD_THERMAL_IRQ_EN_L       (1 << 2)
#define  DNIC7448N_CPLD_FAN_IRQ_EN_L           (1 << 1)
#define  DNIC7448N_CPLD_OOB_PHY_IRQ_EN_L       (1 << 0)

#define DNIC7448N_CPLD_WATCHDOG_OFFSET         0xc0
#define  DNIC7448N_CPLD_WATCHDOG_TIME          0x70
#define  DNIC7448N_CPLD_WATCHDOG_15_SEC        0x00
#define  DNIC7448N_CPLD_WATCHDOG_20_SEC        0x10
#define  DNIC7448N_CPLD_WATCHDOG_30_SEC        0x20
#define  DNIC7448N_CPLD_WATCHDOG_40_SEC        0x30
#define  DNIC7448N_CPLD_WATCHDOG_50_SEC        0x40
#define  DNIC7448N_CPLD_WATCHDOG_60_SEC        0x50
#define  DNIC7448N_CPLD_WATCHDOG_EN            (1 << 3)
#define  DNIC7448N_CPLD_WATCHDOG_CPLD_RESET_L  (1 << 2)
#define  DNIC7448N_CPLD_WATCHDOG_RTC_RESET_L   (1 << 1)
#define  DNIC7448N_CPLD_WATCHDOG_FLAG          (1 << 0)

#define DNIC7448N_CPLD_FAN_TRAY_LED_CTRL_OFFSET 0xd0
#define  DNIC7448N_CPLD_FAN_TRAY_1_LED         0x0c
#define   DNIC7448N_CPLD_FAN_TRAY_1_LED_GREEN  0x04
#define   DNIC7448N_CPLD_FAN_TRAY_1_LED_RED    0x08
#define   DNIC7448N_CPLD_FAN_TRAY_1_LED_OFF    0x00
#define  DNIC7448N_CPLD_FAN_TRAY_0_LED         0x03
#define   DNIC7448N_CPLD_FAN_TRAY_0_LED_GREEN  0x01
#define   DNIC7448N_CPLD_FAN_TRAY_0_LED_RED    0x02
#define   DNIC7448N_CPLD_FAN_TRAY_0_LED_OFF    0x00

#define DNIC7448N_CPLD_AIRFLOW_OFFSET          0xe0

#define DNIC7448N_CPLD_BOARD_IRQ_MASK_OFFSET   0xf0
#define  DNIC7448N_CPLD_POWER_IRQ_MASK_EN_L    (1 << 6)
#define  DNIC7448N_CPLD_PHY_IRQ_MASK_EN_L      (3 << 4)
#define  DNIC7448N_CPLD_HOT_SWAP_IRQ_MASK_EN_L (1 << 3)
#define  DNIC7448N_CPLD_THERMAL_IRQ_MASK_EN_L  (1 << 2)
#define  DNIC7448N_CPLD_FAN_IRQ_MASK_EN_L      (1 << 1)
#define  DNIC7448N_CPLD_OOB_PHY_IRQ_MASK_EN_L  (1 << 0)

#define DNIC7448N_CPLD_STRING_NAME_SIZE        20

#endif /* DNI_C7448N_H__ */
