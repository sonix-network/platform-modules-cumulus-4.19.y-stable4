/*
 * Accton AS5610_52X CPLD Platform Definitions
 *
 * Puneet Shenoy <puneet@cumulusnetworks.com>
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

#ifndef ACCTON_AS5610_52X_H__
#define ACCTON_AS5610_52X_H__

/*
 * Begin register defines.
 */

#define CPLD_REG_MODEL_TYPE	    (0x00)
#  define CPLD_PCB_VER_MASK           (0x38)
#  define CPLD_PCB_VER_SHIFT          (0x3)
#  define CPLD_PCB_MODEL_TYPE_MASK    (0x7)
#  define CPLD_PCB_MODEL_TYPE_SHIFT   (0x0)

#define CPLD_REG_PS2_STATUS	    (0x01)
#define CPLD_REG_PS1_STATUS	    (0x02)
#  define CPLD_PS_ABSENT              (1 << 0)
#  define CPLD_PS_DC_OK               (1 << 1)
#  define CPLD_PS_FAN_OK              (1 << 2)
#  define CPLD_PS_NOT_OT              (1 << 3)
#  define CPLD_PS_FAN_DIR             (1 << 4)

#define CPLD_REG_SYSTEM_STATUS	    (0x03)
#  define CPLD_SYS_PWR_GOOD           (1 << 0)
#  define CPLD_SYS_FAN_ABSENT         (1 << 2)
#  define CPLD_SYS_FAN_BAD            (1 << 3)
#  define CPLD_SYS_FAN_AIR_FLOW       (1 << 4)

#define CPLD_REG_SFP_INTR_STATUS    (0x05)
#  define CPLD_SFP_PRESENT            (1 << 0)
#  define CPLD_SFP_RX_LOS             (1 << 1)
#  define CPLD_QSFP_P0_INTR           (1 << 2)
#  define CPLD_QSFP_P1_INTR           (1 << 3)
#  define CPLD_QSFP_P2_INTR           (1 << 4)
#  define CPLD_QSFP_P3_INTR           (1 << 5)


#define CPLD_REG_MISC_INTR_STATUS   (0x08)
#  define CPLD_MAX6581_INTR           (1 << 0)
#  define CPLD_UPD720102GG_INTR       (1 << 1)

#define CPLD_REG_VERSION            (0x09)
#  define CPLD_VERSION_MASK           (0x1F)
#  define CPLD_RELEASE_VERSION        (1 << 5)

#define CPLD_REG_VMARG_CTRL_1       (0x0A)
#  define CPLD_VMARG_1_05V_CPU_MASK   (0x3)
#  define CPLD_VMARG_1_05V_CPU_SHIFT  (0)
#  define CPLD_VMARG_HIGH             (3)
#  define CPLD_VMARG_NORMAL           (2)
#  define CPLD_VMARG_LOW              (0)

#define CPLD_REG_VMARG_CTRL_2       (0x0B)
#  define CPLD_VMARG_3_3V_MASK        (0x3)
#  define CPLD_VMARG_3_3V_SHIFT       (0)
#  define CPLD_VMARG_3_3V_SFP_MASK    (0xc)
#  define CPLD_VMARG_3_3V_SFP_SHIFT   (2)

#define CPLD_REG_FAN_CTRL           (0x0D)
#  define CPLD_FAN_CTRL_MASK          (0x1F)

#define CPLD_REG_WATCH_DOG_CTRL     (0x0E)
#  define CPLD_WATCH_DOG_KICK         (1 << 0)
#  define CPLD_WATCH_DOG_ENABLE       (1 << 1)
#  define CPLD_WATCH_DOG_COUNT_MASK   (0x3c)
#  define CPLD_WATCH_DOG_COUNT_SHIFT  (2)

#define CPLD_REG_POWER_CTRL         (0x0F)
#  define CPLD_POWER_OT_PROTECT       (1 << 2)

#define CPLD_REG_RESET_CTRL_1       (0x10) // all resets active low
#  define CPLD_RESET_BCM56846_L	      (1 << 1)
#  define CPLD_RESET_MAX6581_L	      (1 << 4)

#define CPLD_REG_RESET_CTRL_2       (0x11) // all resets active low
#  define CPLD_RESET_BCM5482S_L	      (1 << 0)
#  define CPLD_RESET_I2C_SWITCH_L     (1 << 1)
#  define CPLD_RESET_I2C_GPIO_L	      (1 << 2)
#  define CPLD_RESET_USB_PHY_L	      (1 << 3)
#  define CPLD_RESET_USB_HUB_L	      (1 << 4)

#define CPLD_REG_INTERRUPT_MASK     (0x12)
#  define CPLD_INTR_MASK_MAX6581      (1 << 0)
#  define CPLD_INTR_MASK_BCM56846     (1 << 1)
#  define CPLD_INTR_MASK_I2C_GPIO     (1 << 3)
#  define CPLD_INTR_MASK_USB	      (1 << 4)
#  define CPLD_INTR_MASK_CF_CARD      (1 << 5)
#  define CPLD_INTR_MASK_WATCH_DOG    (1 << 6)
#  define CPLD_INTR_MASK_RX_LOS	      (1 << 7)

#define CPLD_REG_SYSTEM_LED_CTRL_1  (0x13)
#  define CPLD_SYS_LED_PS1_MASK	      (0x03)
#    define CPLD_SYS_LED_PS1_OFF        (3)
#    define CPLD_SYS_LED_PS1_GREEN	(2)
#    define CPLD_SYS_LED_PS1_YELLOW	(0)
#  define CPLD_SYS_LED_PS2_MASK	      (0x0c)
#    define CPLD_SYS_LED_PS2_OFF        (3 << 2)
#    define CPLD_SYS_LED_PS2_GREEN      (2 << 2)
#    define CPLD_SYS_LED_PS2_YELLOW     (0 << 2)
#  define CPLD_SYS_LED_DIAG_MASK      (0x30)
#    define CPLD_SYS_LED_DIAG_GREEN     (2 << 4)
#    define CPLD_SYS_LED_DIAG_YELLOW	(1 << 4)
#    define CPLD_SYS_LED_DIAG_OFF	(0 << 4)
#  define CPLD_SYS_LED_FAN_MASK	      (0xC0)
#    define CPLD_SYS_LED_FAN_OFF	(3 << 6)
#    define CPLD_SYS_LED_FAN_YELLOW	(2 << 6)
#    define CPLD_SYS_LED_FAN_GREEN	(1 << 6)

#define CPLD_REG_MISC_CTRL	      (0x14)
#  define CPLD_MISC_QSFP_LED_SHIFT_REG_CLEAR (1 << 0)

#define CPLD_REG_SYSTEM_LED_CTRL_2    (0x15)
#  define CPLD_SYS_LED_LOCATOR_MASK     (0x03)
#    define CPLD_SYS_LED_LOCATOR_YELLOW_BLINK	(3)
#    define CPLD_SYS_LED_LOCATOR_OFF		(1)

#endif /* ACCTON_AS5610_52X_H__ */
