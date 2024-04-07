/*
 * DNI 7448 CPLD Platform Definitions
 *
 * Curt Brune <curt@cumulusnetworks.com>
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

#ifndef DNI_7448_H__
#define DNI_7448_H__


/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 * register info from ET-7448BF-V2_HW_SPEC_REV0_1R.pdf from DNI (July 10, 2010)
 */

#define DNI7448_CPLD_BOARD_REV_OFFSET		0x10
#define	 DNI7448_CPLD_BOARD_TYPE_MASK		0xf0
#define	 DNI7448_CPLD_BOARD_TYPE_SHIFT		4
#define	 DNI7448_CPLD_BOARD_REV_MASK		0x0f
#define	 DNI7448_CPLD_BOARD_REV_SHIFT		0

#define	DNI7448_CPLD_RESET_OFFSET		0x20
#define	DNI7448_CPLD_RESET_SYSTEM_L		(1 << 7)
#define	 DNI7448_CPLD_RESET_CPU_HARD_L		(1 << 6)
#define	 DNI7448_CPLD_RESET_CPU_SOFT_L		(1 << 5)
#define	 DNI7448_CPLD_RESET_DDR3_L		(1 << 4)
#define	 DNI7448_CPLD_RESET_FLASH_L		(1 << 3)
#define	 DNI7448_CPLD_RESET_NET_SOC_L		(1 << 2)
#define	 DNI7448_CPLD_RESET_NET_PHY_L		(1 << 1)
#define	 DNI7448_CPLD_RESET_OOB_PHY_L		(1 << 0)

#define DNI7448_CPLD_BULK_PS_OFFSET		0x30
#define	 DNI7448_CPLD_BULK_PS_0_MASK		0xf0
#define	  DNI7448_CPLD_BULK_PS_0_PRESENT_L	(1 << 7)
#define	  DNI7448_CPLD_BULK_PS_0_OK_L		(1 << 6)
#define	  DNI7448_CPLD_BULK_PS_0_TEMP_OK	(1 << 5)
#define	  DNI7448_CPLD_BULK_PS_0_FAN_OK		(1 << 4)
#define	 DNI7448_CPLD_BULK_PS_1_MASK		0x0f
#define	  DNI7448_CPLD_BULK_PS_1_PRESENT_L	(1 << 3)
#define	  DNI7448_CPLD_BULK_PS_1_OK_L		(1 << 2)
#define	  DNI7448_CPLD_BULK_PS_1_TEMP_OK		(1 << 1)
#define	  DNI7448_CPLD_BULK_PS_1_FAN_OK		(1 << 0)

#define DNI7448_CPLD_PHY_IRQ_EN_0to7_OFFSET	0x40
#define	 DNI7448_CPLD_PHY7_IRQ_EN_L		(1 << 7)
#define	 DNI7448_CPLD_PHY6_IRQ_EN_L		(1 << 6)
#define	 DNI7448_CPLD_PHY5_IRQ_EN_L		(1 << 5)
#define	 DNI7448_CPLD_PHY4_IRQ_EN_L		(1 << 4)
#define	 DNI7448_CPLD_PHY3_IRQ_EN_L		(1 << 3)
#define	 DNI7448_CPLD_PHY2_IRQ_EN_L		(1 << 2)
#define	 DNI7448_CPLD_PHY1_IRQ_EN_L		(1 << 1)
#define	 DNI7448_CPLD_PHY0_IRQ_EN_L		(1 << 0)

#define DNI7448_CPLD_PHY_IRQ_EN_8to15_OFFSET	0x50
#define	 DNI7448_CPLD_QPHY3_IRQ_EN_L		(1 << 7)
#define	 DNI7448_CPLD_QPHY2_IRQ_EN_L		(1 << 6)
#define	 DNI7448_CPLD_QPHY1_IRQ_EN_L		(1 << 5)
#define	 DNI7448_CPLD_QPHY0_IRQ_EN_L		(1 << 4)
#define	 DNI7448_CPLD_PHY11_IRQ_EN_L		(1 << 3)
#define	 DNI7448_CPLD_PHY10_IRQ_EN_L		(1 << 2)
#define	 DNI7448_CPLD_PHY9_IRQ_EN_L		(1 << 1)
#define	 DNI7448_CPLD_PHY8_IRQ_EN_L		(1 << 0)

#define DNI7448_CPLD_BOARD_PWR_OFFSET		0x60
#define	 DNI7448_CPLD_FLASH_ERASE		(1 << 7)
#define	 DNI7448_CPLD_FLASH_WRITE		(1 << 6)
#define	 DNI7448_CPLD_CPU_1V_L			(1 << 4)
#define	 DNI7448_CPLD_NET_SOC_1V		(1 << 3)
#define	 DNI7448_CPLD_DDR_1V5			(1 << 2)
#define	 DNI7448_CPLD_PHY_1V_L			(1 << 1)
#define	 DNI7448_CPLD_PHY_1V_R			(1 << 0)

#define DNI7448_CPLD_BOARD_PWR_STATUS_OFFSET	0x70
#define	 DNI7448_CPLD_VCC_3V3_GOOD		(1 << 6)
#define	 DNI7448_CPLD_CPU_1V_GOOD_L		(1 << 5)
#define	 DNI7448_CPLD_NET_SOC_1V_GOOD		(1 << 4)
#define	 DNI7448_CPLD_DDR_1V5_GOOD		(1 << 3)
#define	 DNI7448_CPLD_PHY_1V5_GOOD_L		(1 << 2)
#define	 DNI7448_CPLD_PHY_1V_1to24_GOOD		(1 << 1)
#define	 DNI7448_CPLD_PHY_1V_25to52_GOOD	(1 << 0)

#define DNI7448_CPLD_SYSTEM_LED_OFFSET		0x80
#define  DNI7448_CPLD_FAN_LED			0xc0
#define   DNI7448_CPLD_FAN_LED_GREEN		0x40
#define   DNI7448_CPLD_FAN_LED_YELLOW		0x80
#define   DNI7448_CPLD_FAN_LED_OFF		0x00
#define  DNI7448_CPLD_POWER_LED			0x30
#define   DNI7448_CPLD_POWER_LED_GREEN		0x20
#define   DNI7448_CPLD_POWER_LED_YELLOW		0x10
#define   DNI7448_CPLD_POWER_LED_YELLOW_BLINK	0x30
#define   DNI7448_CPLD_POWER_LED_OFF		0x00
#define  DNI7448_CPLD_MASTER_LED		0x08
#define   DNI7448_CPLD_MASTER_LED_GREEN		0x00
#define   DNI7448_CPLD_MASTER_LED_OFF		0x08
#define  DNI7448_CPLD_STATUS_LED		0x07
#define   DNI7448_CPLD_STATUS_LED_GREEN		0x02
#define   DNI7448_CPLD_STATUS_LED_GREEN_BLINK	0x01
#define   DNI7448_CPLD_STATUS_LED_RED		0x03
#define   DNI7448_CPLD_STATUS_LED_RED_BLINK	0x04
#define   DNI7448_CPLD_STATUS_LED_OFF		0x00

#define DNI7448_CPLD_FAN_STATUS_OFFSET		0x90
#define  DNI7448_CPLD_FAN_0_MASK		0x2c
#define   DNI7448_CPLD_FAN_0_PRESENT_L		(1 << 5)
#define   DNI7448_CPLD_FAN_0_A_OK		(1 << 3)
#define   DNI7448_CPLD_FAN_0_B_OK		(1 << 2)
#define  DNI7448_CPLD_FAN_1_MASK		0x13
#define   DNI7448_CPLD_FAN_1_PRESENT_L		(1 << 4)
#define   DNI7448_CPLD_FAN_1_A_OK		(1 << 1)
#define   DNI7448_CPLD_FAN_1_B_OK		(1 << 0)

#define DNI7448_CPLD_REV_OFFSET			0xa0

#define DNI7448_CPLD_BOARD_IRQ_CTRL_OFFSET	0xb0
#define  DNI7448_CPLD_POWER_IRQ_EN_L		(1 << 6)
#define  DNI7448_CPLD_PHY_IRQ_EN_L		(3 << 4)
#define  DNI7448_CPLD_HOT_SWAP_IRQ_EN_L		(1 << 3)
#define  DNI7448_CPLD_THERMAL_IRQ_EN_L		(1 << 2)
#define  DNI7448_CPLD_FAN_IRQ_EN_L		(1 << 1)
#define  DNI7448_CPLD_OOB_PHY_IRQ_EN_L		(1 << 0)

#define	DNI7448_CPLD_WATCHDOG_OFFSET		0xc0
#define  DNI7448_CPLD_WATCHDOG_TIME_MASK	0x70
#define   DNI7448_CPLD_WATCHDOG_15_SEC		0x00
#define   DNI7448_CPLD_WATCHDOG_20_SEC		0x10
#define   DNI7448_CPLD_WATCHDOG_30_SEC		0x20
#define   DNI7448_CPLD_WATCHDOG_40_SEC		0x30
#define   DNI7448_CPLD_WATCHDOG_50_SEC		0x40
#define   DNI7448_CPLD_WATCHDOG_60_SEC		0x50
#define  DNI7449_CPLD_WATCHDOG_MAX		60
#define  DNI7449_CPLD_WATCHDOG_MIN		15
#define  DNI7448_CPLD_WATCHDOG_EN		(1 << 3)
#define  DNI7448_CPLD_WATCHDOG_CPLD_RESET_L	(1 << 2)
#define  DNI7448_CPLD_WATCHDOG_RTC_RESET_L	(1 << 1)
#define  DNI7448_CPLD_WATCHDOG_KICK_L		(1 << 0)

#define DNI7448_CPLD_FAN_TRAY_LED_CTRL_OFFSET	0xd0
#define  DNI7448_CPLD_PORT_LED_EN		(1 << 7)
#define  DNI7448_CPLD_FAN_TRAY_0_LED		0x0c
#define   DNI7448_CPLD_FAN_TRAY_0_LED_GREEN	0x04
#define   DNI7448_CPLD_FAN_TRAY_0_LED_RED	0x08
#define   DNI7448_CPLD_FAN_TRAY_0_LED_OFF	0x00
#define  DNI7448_CPLD_FAN_TRAY_1_LED		0x03
#define   DNI7448_CPLD_FAN_TRAY_1_LED_GREEN	0x01
#define   DNI7448_CPLD_FAN_TRAY_1_LED_RED	0x02
#define   DNI7448_CPLD_FAN_TRAY_1_LED_OFF	0x00

#define DNI7448_CPLD_AIRFLOW_OFFSET		0xe0

#define DNI7448_CPLD_BOARD_IRQ_MASK_OFFSET	0xf0
#define  DNI7448_CPLD_POWER_IRQ_MASK_EN_L	(1 << 6)
#define  DNI7448_CPLD_PHY_IRQ_MASK_EN_L		(3 << 4)
#define  DNI7448_CPLD_HOT_SWAP_IRQ_MASK_EN_L	(1 << 3)
#define  DNI7448_CPLD_THERMAL_IRQ_MASK_EN_L	(1 << 2)
#define  DNI7448_CPLD_FAN_IRQ_MASK_EN_L		(1 << 1)
#define  DNI7448_CPLD_OOB_PHY_IRQ_MASK_EN_L	(1 << 0)

#define DNI7448_CPLD_STRING_NAME_SIZE 20

#endif /* DNI_7448_H__ */
