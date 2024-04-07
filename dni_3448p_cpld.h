/*
 * DNI 3448P CPLD Platform Definitions
 *
 * Samer Nubani <samer@cumulusnetworks.com>
 * Alan Liebthal <alanl@cumulusnetworks.com>
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

#ifndef DNI_3448P_H__
#define DNI_3448P_H__

/*
 * Begin register defines.
 */
#define CPLD_REG_HW_REVISION                       (0x01)
  #define CPLD_BOARD_TYPE_MASK                     (0xf0)
  #define CPLD_HW_REV_MASK                         (0x07)

#define CPLD_REG_SW_RESET                          (0x02)
  #define CPLD_FORCE_SW_RESET_L                    (1 << 7)
  #define CPLD_RESET_MAC_L                         (1 << 5)
  #define CPLD_RESET_OOB_L                         (1 << 4)
  #define CPLD_RESET_10G_SLOT1_L                   (1 << 3)
  #define CPLD_RESET_PHY_L                         (1 << 2)
  #define CPLD_RESET_POE_L                         (1 << 0)

#define CPLD_REG_MODULE_CTL_STATUS                 (0x03)
  #define CPLD_10G_SLOT1_TYPE_MASK                 (0xf0)
  #define CPLD_10G_SLOT1_POWER_GOOD                (1 << 2)
  #define CPLD_10G_SLOT1_MODULE_PRESENT_L          (1 << 1)
  #define CPLD_10G_SLOT1_POWER_ENABLE_L            (1 << 0)

#define CPLD_REG_SYS_POWER_STATUS                  (0x04)
  #define CPLD_VCN3V3_PG                           (1 << 7)
  #define CPLD_DDR1V5_PG                           (1 << 6)
  #define CPLD_MAC1V_AVS_PG                        (1 << 5)
  #define CPLD_VCC1V_PG                            (1 << 4)
  #define CPLD_MODULE_PG                           (1 << 2)
  #define CPLD_HOT_SWAP_PG                         (1 << 0)

#define CPLD_REG_CPU_IRQ                           (0x05)
  #define CPLD_POE_INT                             (1 << 7)
  #define CPLD_RTC_INT                             (1 << 6)
  #define CPLD_FAN_INT                             (1 << 5)
  #define CPLD_HOT_SWAP_INT                        (1 << 4)
  #define CPLD_THERMAL_INT                         (1 << 3)
  #define CPLD_PS_POWER_INT                        (1 << 2)
  #define CPLD_MODULE_INT                          (1 << 1)
  #define CPLD_PHY_INT                             (1 << 0)

#define CPLD_REG_CPU_IRQ_MASK                      (0x06)
  #define CPLD_POE_INT_MASK                        (1 << 7)
  #define CPLD_RTC_INT_MASK                        (1 << 6)
  #define CPLD_FAN_INT_MASK                        (1 << 5)
  #define CPLD_HOT_SWAP_INT_MASK                   (1 << 4)
  #define CPLD_THERMAL_INT_MASK                    (1 << 3)
  #define CPLD_PS_POWER_INT_MASK                   (1 << 2)
  #define CPLD_MODULE_INT_MASK                     (1 << 1)
  #define CPLD_PHY_INT_MASK                        (1 << 0)

#define CPLD_REG_CPLD_PROGRAM_REV                  (0x07)
  #define CPLD_REV_MASK                            (0xff)

#define CPLD_REG_N30XX_PSU_STATUS                  (0x08)
  #define CPLD_PS2_PRESENT_L                       (1 << 7)
  #define CPLD_PS2_GOOD                            (1 << 6)
  #define CPLD_PS2_ALERT_L                         (1 << 5)
  #define CPLD_PS2_ENABLE_L                        (1 << 4)
  #define CPLD_PS1_PRESENT_L                       (1 << 3)
  #define CPLD_PS1_GOOD                            (1 << 2)
  #define CPLD_PS1_ALERT_L                         (1 << 1)
  #define CPLD_PS1_ENABLE_L                        (1 << 0)

#define CPLD_REG_N20XX_PSU_STATUS                  (0x09)
  #define CPLD_RPS_PRESENT_L                       (1 << 7)
  #define CPLD_RPS_GOOD                            (1 << 6)
  #define CPLD_MAIN_PSU_GOOD_L                     (1 << 5)
  #define CPLD_MAIN_PSU_OVERTEMP_L                 (1 << 4)
  #define CPLD_MAIN_PSU_FAN_FAULT_L                (1 << 3)

#define CPLD_REG_LED_CTL_1                         (0x0a)
  #define CPLD_PWR1_LED_MASK                       (0xc0)
  #define CPLD_PWR1_LED_OFF                        (0x00)
  #define CPLD_PWR1_LED_GREEN                      (0x40)
  #define CPLD_PWR1_LED_GREEN_BLINK                (0x80)
  #define CPLD_PWR2_LED_MASK                       (0x30)
  #define CPLD_PWR2_LED_OFF                        (0x00)
  #define CPLD_PWR2_LED_GREEN                      (0x10)
  #define CPLD_PWR2_LED_GREEN_BLINK                (0x20)
  #define CPLD_FAN_LED_MASK                        (0x0c)
  #define CPLD_FAN_LED_GREEN                       (0x04)
  #define CPLD_FAN_LED_RED                         (0x08)
  #define CPLD_SYSTEM_LED_MASK                     (0x03)
  #define CPLD_SYSTEM_LED_GREEN_BLINK              (0x00)
  #define CPLD_SYSTEM_LED_GREEN                    (0x01)
  #define CPLD_SYSTEM_LED_RED                      (0x02)
  #define CPLD_SYSTEM_LED_RED_BLINK                (0x03)

#define CPLD_REG_LED_CTL_2                         (0x0b)
  #define CPLD_TEMP_LED_MASK                       (0x0c)
  #define CPLD_TEMP_LED_RED                        (0x08)
  #define CPLD_TEMP_LED_OFF                        (0x00)
  #define CPLD_TEMP_LED_GREEN                      (0x04)
  #define CPLD_MASTER_LED_MASK                     (0x02)
  #define CPLD_MASTER_LED_GREEN                    (0x02)
  #define CPLD_MASTER_LED_OFF                      (0x00)


#define CPLD_REG_FAN_STATUS                        (0x0c)
  #define CPLD_FAN1_TRAY_PRESENT_L                 (1 << 1)
  #define CPLD_FAN1_GOOD                           (1 << 0)

#define CPLD_REG_WATCHDOG                          (0x0d)
  #define CPLD_WATCHDOG_TIMER_MASK                 (0x70)
  #define CPLD_WATCHDOG_TIMER_15S                  (0x00)
  #define CPLD_WATCHDOG_TIMER_20S                  (0x01)
  #define CPLD_WATCHDOG_TIMER_30S                  (0x02)
  #define CPLD_WATCHDOG_TIMER_40S                  (0x03)
  #define CPLD_WATCHDOG_TIMER_50S                  (0x04)
  #define CPLD_WATCHDOG_TIMER_60S                  (0x05)
  #define CPLD_WATCHDOG_ENABLE                     (1 << 3)
  #define CPLD_WATCHDOG_RESET_RTC                  (1 << 2)
  #define CPLD_WATCHDOG_WDI_FLAG                   (1 << 0)

#define CPLD_REG_DEVICE_SHUTDOWN                   (0x0e)
  #define CPLD_SHUTDOWN_MODULE_L                   (1 << 7)
  #define CPLD_SHUTDOWN_OOB_L                      (1 << 6)
  #define CPLD_SHUTDOWN_PHY_L                      (1 << 4)
  #define CPLD_SHUTDOWN_POE_L                      (1 << 3)
  #define CPLD_SHUTDOWN_SFP_L                      (1 << 2)
  #define CPLD_SHUTDOWN_ALL_L                      (1 << 0)


#define CPLD_REG_N30XX_I2C_MUX_CTL                 (0x19)
  #define CPLD_I2C_SELECT_MASK                     (0x60)
  #define CPLD_I2C_PSU1_SELECT                     (0x00)
  #define CPLD_I2C_PSU2_SELECT                     (0x20)
  #define CPLD_I2C_SFP_PORT_23_47                  (0x00)
  #define CPLD_I2C_SFP_PORT_24_48                  (0x01)

#define CPLD_REG_N3024F_I2C_MUX_CTL                (0x1a)

#define CPLD_REG_7DIGIT_LED_CTL                    (0x1b)

#define CPLD_REG_USB_REGISTER                      (0x1c)



#endif /* DNI_3448P_H__ */
