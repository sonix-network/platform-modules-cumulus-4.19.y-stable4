/*
 * Dell S3000 CPLD Platform Definitions
 *
 * Samer Nubani <samer@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef DELL_S3000_CPLD_H__
#define DELL_S3000_CPLD_H__

#define CPLD_IO_BASE 0x100
#define CPLD_IO_SIZE 0x155


/*------------------------------------------------------------------------------------
 *
 * register info from R0882-M0025-01 Rev0.3 S3000 MMC and SMC CPLD Logic design spec 2
 *
 *  CPLD_MMC (Module Management)
 *    - CPU boot backup
 *    - On-board power sequencing controller
 *    - On-board thermal alarm monitor
 *    - On-board reset circuitry
 *    - System reset distribution
 *    - Interrupt Logic
 */
#define CPLD_MMC_VERSION_REG                    (0x100)
#define  CPLD_MMC_VERSION_H_MASK                (0xF0)      /* Board version */
#define  CPLD_MMC_VERSION_H_SHIFT               (4)
#define  CPLD_MMC_VERSION_L_MASK                (0x0F)      /* MMC minor version */
#define  CPLD_MMC_VERSION_L_SHIFT               (0)

#define CPLD_MMC_SW_SCRATCH_REG                 (0x101)

#define CPLD_MMC_BOOT_OK_REG                    (0x102)
#define  CPLD_MMC_CPU_BOOT_STA                  (1 << 1)    /* 0: Boot OK */
#define  CPLD_MMC_CPU_BIOS                      (1 << 0)    /* 0: BIOS0, 1: BIOS1 */

#define CPLD_MMC_EEPROM_WP_REG                  (0x103)
#define  CPLD_MMC_SPD1_WP                       (1 << 1)    /* 0: Disable write protect */
#define  CPLD_MMC_SYSTEM_EEPROM_WP              (1 << 0)

#define CPLD_MMC_PRESENT_MB_REG                 (0x104)
#define  CPLD_MMC_PRESENT_MB                    (1 << 0)    /* 0: Mainboard present */

#define CPLD_MMC_WD_WDI_REG                     (0x110)
#define  CPLD_MMC_WD_WDI_MASK                   (0x03)
#define  CPLD_MMC_WD_WDI_SHIFT                  (0)
#define   CPLD_MMC_WD_WDI_200MS                 (0x00)
#define   CPLD_MMC_WD_WDI_30S                   (0x01)
#define   CPLD_MMC_WD_WDI_60S                   (0x02)
#define   CPLD_MMC_WD_WDI_180S                  (0x03)

#define CPLD_MMC_WD_MASK_REG                    (0x111)
#define  CPLD_MMC_WD                            (1 << 0)    /* 0: Enable WD */

#define CPLD_MMC_RST_SOURCE_REG                 (0x112)
#define  CPLD_MMC_RST_SOURCE_MASK               (0xFF)
#define  CPLD_MMC_RST_SOURCE_SHIFT              (8)
#define   CPLD_MMC_RST_SOURCE_PWR_ON            (0x11)
#define   CPLD_MMC_RST_SOURCE_XDP               (0x22)
#define   CPLD_MMC_RST_SOURCE_WD                (0x33)
#define   CPLD_MMC_RST_SOURCE_CPU_COLD          (0x44)
#define   CPLD_MMC_RST_SOURCE_CPU_WARM          (0x55)
#define   CPLD_MMC_RST_SOURCE_SOFT_COLD         (0x66)
#define   CPLD_MMC_RST_SOURCE_SOFT_WARM         (0x77)
#define   CPLD_MMC_RST_SOURCE_BUTTON            (0x88)

#define CPLD_MMC_RST_CTRL_REG                   (0x113)
#define  CPLD_MMC_RST_CTRL_MASK                 (0xFF)
#define  CPLD_MMC_RST_CTRL_SHIFT                (8)
#define   CPLD_MMC_RST_CTRL_SW_RESET            (0x5A)
#define   CPLD_MMC_RST_CTRL_SW_PWR_CYCLE        (0xAA)

#define CPLD_MMC_SEP_RST_REG                    (0x114)
#define  CPLD_MMC_SEP_RST_54616                 (1 << 0)    /* 0: Reset */

#define CPLD_MMC_THERMAL_POWEROFF_CTRL_REG      (0x120)
#define  CPLD_MMC_THERMAL_POWEROFF_CPU          (1 << 0)    /* 1: Power off CPU rail */

#define CPLD_MMC_SUS0_TRIG_MOD_REG              (0x121)
#define  CPLD_MMC_SUS0_THERMTRIP_TRIG_MASK      (0x30)
#define  CPLD_MMC_SUS0_THERMTRIP_TRIG_SHIFT     (4)
#define  CPLD_MMC_SUS0_54616_TRIG_MASK          (0x0C)
#define  CPLD_MMC_SUS0_54616_TRIG_SHIFT         (2)
#define  CPLD_MMC_SUS0_SENSOR_TRIG_MASK         (0x03)
#define  CPLD_MMC_SUS0_SENSOR_TRIG_SHIFT        (0)

#define CPLD_MMC_SUS0_COMBINE_REG               (0x122)
#define  CPLD_MMC_SUS0_THERMTRIP_COMBINE        (1 << 3)    /* 0: Int active */
#define  CPLD_MMC_SUS0_54616_COMBINE            (1 << 2)    /* 0: Int active */
#define  CPLD_MMC_SUS0_SENSOR_PROCHOT_COMBINE   (1 << 1)    /* 0: Int active */
#define  CPLD_MMC_SUS0_SENSOR_ALERT_COMBINE     (1 << 0)    /* 0: Int active */

#define CPLD_MMC_SUS0_STA_REG                   (0x123)
#define CPLD_MMC_SUS0_STA_THERMAL_MASK          (0x0F)
#define CPLD_MMC_SUS0_STA_THERMAL_SHIFT         (0)
#define  CPLD_MMC_SUS0_THERMTRIP_STA            (1 << 3)    /* 0: Alert */
#define  CPLD_MMC_SUS0_54616_STA                (1 << 2)    /* 0: Int active */
#define  CPLD_MMC_SUS0_SENSOR_PROCHOT_STA       (1 << 1)    /* 0: Overhot */
#define  CPLD_MMC_SUS0_SENSOR_ALERT_STA         (1 << 0)    /* 0: Alert */

#define CPLD_MMC_SUS0_INT_REG                   (0x124)
#define  CPLD_MMC_SUS0_THERMTRIP_INT            (1 << 3)    /* 1: Int active */
#define  CPLD_MMC_SUS0_54616_INT                (1 << 2)    /* 1: Int active */
#define  CPLD_MMC_SUS0_SENSOR_PROCHOT_INT       (1 << 1)    /* 1: Int active */
#define  CPLD_MMC_SUS0_SENSOR_ALERT_INT         (1 << 0)    /* 1: Int active */

#define CPLD_MMC_SUS0_MASK_REG                  (0x125)
#define  CPLD_MMC_SUS0_THERMTRIP_MASK           (1 << 3)    /* 1: Masked */
#define  CPLD_MMC_SUS0_54616_MASK               (1 << 2)    /* 1: Masked */
#define  CPLD_MMC_SUS0_SENSOR_PROCHOT_MASK      (1 << 1)    /* 1: Masked */
#define  CPLD_MMC_SUS0_SENSOR_ALERT_MASK        (1 << 0)    /* 1: Masked */

/*
 * CPLD_SMC (System Management)
 *  - Main board reset logic
 *  - PSU interface
 *  - Optical module control
 *  - LED control
 *  - On-board alarm monitor
 *  - Interrupt logic
 */
#define CPLD_SMC_VERSION_REG                    (0x200)
#define  CPLD_SMC_VERSION_H_MASK                (0xF0)      /* Board version */
#define  CPLD_SMC_VERSION_H_SHIFT               (4)
#define  CPLD_SMC_VERSION_L_MASK                (0x0F)      /* SMC minor version */
#define  CPLD_SMC_VERSION_L_SHIFT               (0)

#define CPLD_SMC_SW_SCRATCH_REG                 (0x201)

#define CPLD_SMC_BOARD_ID_REG                   (0x202)
#define  CPLD_SMC_BOARD_ID_MASK                 (0x0f)
#define  CPLD_SMC_BOARD_ID_SHIFT                (0)

#define CPLD_SMC_PSU_CONTROL_REG                (0x203)
#define  CPLD_SMC_PSUR_CTRL                     (1 << 1)    /* 0: On */
#define  CPLD_SMC_PSUL_CTRL                     (1 << 0)    /* 0: On */

#define CPLD_SMC_PSU_STATUS_REG                 (0x204)
#define  CPLD_SMC_PSUR_AC                       (1 << 7)    /* 0: AC fail */
#define  CPLD_SMC_PSUR_POWER                    (1 << 6)    /* 0: Power fail */
#define  CPLD_SMC_PSUR_ALERT                    (1 << 5)    /* 0: Alert */
#define  CPLD_SMC_PSUR_PRESENT                  (1 << 4)    /* 0: Present */
#define  CPLD_SMC_PSUL_AC                       (1 << 3)    /* 0: AC fail */
#define  CPLD_SMC_PSUL_POWER                    (1 << 2)    /* 0: Power fail */
#define  CPLD_SMC_PSUL_ALERT                    (1 << 1)    /* 0: Alert */
#define  CPLD_SMC_PSUL_PRESENT                  (1 << 0)    /* 0: Present */

#define CPLD_SMC_FAN1_LED_REG                   (0x205)
#define  CPLD_SMC_FAN1_LED_MASK                 (0x07)
#define  CPLD_SMC_FAN1_LED_SHIFT                (0)
#define   CPLD_SMC_FAN1_LED_GREEN               (0x00)
#define   CPLD_SMC_FAN1_LED_GREEN_SLOW_BLINK    (0x01)
#define   CPLD_SMC_FAN1_LED_YELLOW              (0x02)
#define   CPLD_SMC_FAN1_LED_YELLOW_SLOW_BLINK   (0x03)
#define   CPLD_SMC_FAN1_LED_OFF                 (0x07)

#define CPLD_SMC_FAN2_LED_REG                   (0x206)
#define  CPLD_SMC_FAN2_LED_MASK                 (0x07)
#define  CPLD_SMC_FAN2_LED_SHIFT                (0)
#define   CPLD_SMC_FAN2_LED_GREEN               (0x00)
#define   CPLD_SMC_FAN2_LED_GREEN_SLOW_BLINK    (0x01)
#define   CPLD_SMC_FAN2_LED_YELLOW              (0x02)
#define   CPLD_SMC_FAN2_LED_YELLOW_SLOW_BLINK   (0x03)
#define   CPLD_SMC_FAN2_LED_OFF                 (0x07)

#define CPLD_SMC_FAN3_LED_REG                   (0x207)
#define  CPLD_SMC_FAN3_LED_MASK                 (0x07)
#define  CPLD_SMC_FAN3_LED_SHIFT                (0)
#define   CPLD_SMC_FAN3_LED_GREEN               (0x00)
#define   CPLD_SMC_FAN3_LED_GREEN_SLOW_BLINK    (0x01)
#define   CPLD_SMC_FAN3_LED_YELLOW              (0x02)
#define   CPLD_SMC_FAN3_LED_YELLOW_SLOW_BLINK   (0x03)
#define   CPLD_SMC_FAN3_LED_OFF                 (0x07)

#define CPLD_SMC_LED_OPMOD_REG                  (0x208)
#define  CPLD_SMC_LED_OPMOD                     (1 << 0)    /* 0: Normal, 1: Test */

#define CPLD_SMC_LED_TEST_REG                   (0x209)
#define  CPLD_SMC_LED_TEST_OPCTRL_MASK          (0x03)
#define  CPLD_SMC_LED_TEST_OPCTRL_SHIFT         (0)
#define   CPLD_SMC_LED_TEST_OPCTRL_OFF          (0x00)
#define   CPLD_SMC_LED_TEST_OPCTRL_GREEN_BLINK  (0x01)
#define   CPLD_SMC_LED_TEST_OPCTRL_GREEN        (0x02)
#define   CPLD_SMC_LED_TEST_OPCTRL_GREEN_SLOW_BLINK   (0x03)

#define CPLD_SMC_FPS_LED1_REG                    (0x20a)
#define  CPLD_SMC_FPS_LED1_POWER_MASK            (0xC0)
#define  CPLD_SMC_FPS_LED1_POWER_SHIFT           (6)
#define   CPLD_SMC_FPS_LED1_POWER_GREEN          (0x03)
#define   CPLD_SMC_FPS_LED1_POWER_AMBER_BLINK    (0x02)
#define   CPLD_SMC_FPS_LED1_POWER_AMBER          (0x01)
#define   CPLD_SMC_FPS_LED1_POWER_OFF            (0x00)
#define  CPLD_SMC_FPS_LED1_FAN_MASK              (0x30)
#define  CPLD_SMC_FPS_LED1_FAN_SHIFT             (4)
#define   CPLD_SMC_FPS_LED1_FAN_GREEN            (0x00)
#define   CPLD_SMC_FPS_LED1_FAN_AMBER            (0x01)
#define   CPLD_SMC_FPS_LED1_FAN_OFF              (0x03)
#define  CPLD_SMC_FPS_LED1_STA_MASK              (0x0e)
#define  CPLD_SMC_FPS_LED1_STA_SHIFT             (1)
#define   CPLD_SMC_FPS_LED1_STA_GREEN            (0x00)
#define   CPLD_SMC_FPS_LED1_STA_GREEN_BLINK      (0x01)
#define   CPLD_SMC_FPS_LED1_STA_AMBER            (0x02)
#define   CPLD_SMC_FPS_LED1_STA_AMBER_BLINK      (0x03)
#define   CPLD_SMC_FPS_LED1_STA_OFF              (0x07)
#define  CPLD_SMC_FPS_LED1_MASTER                (1 << 0)    /* 0: Green, 1: Off */

#define CPLD_SMC_FPS_LED2_REG                    (0x20b)
#define  CPLD_SMC_FPS_LED2_LOCATOR               (1 << 0)    /* 0: Blue, 1: Off */

#define CPLD_SMC_DEV_STA_REG                    (0x20c)
#define  CPLD_SMC_DEV_STA_USB_HUB               (1 << 3)    /* 1: Active */
#define  CPLD_SMC_DEV_STA_FAN_DIR_MASK          (0x07)
#define  CPLD_SMC_DEV_STA_FAN_DIR_SHIFT         (0)
#define   CPLD_SMC_DEV_STA_FAN3_DIR             (1 << 2)    /* 0: F2B, 1: B2F */
#define   CPLD_SMC_DEV_STA_FAN2_DIR             (1 << 1)    /* 0: F2B, 1: B2F */
#define   CPLD_SMC_DEV_STA_FAN1_DIR             (1 << 0)    /* 0: F2B, 1: B2F */

#define CPLD_SMC_FAN_EEPROM_WP_REG              (0x20d)
#define  CPLD_SMC_FAN_EEPROM_WP                 (1 << 0)    /* 0: Disable Write Protect */

#define CPLD_SMC_EEPROM_WC_N_REG                (0X20e)
#define  CPLD_SMC_EEPROM_WC                     (1 << 0)    /* 0: Disable */

#define CPLD_SMC_POWER_EN_REG                   (0x20f)
#define  CPLD_SMC_POWER_USB                     (1 << 2)    /* 0: Enable */
#define  CPLD_SMC_POWER_XP1R5V                  (1 << 1)    /* 1: Enable */
#define  CPLD_SMC_POWER_XP1R0V                  (1 << 0)    /* 1: Enable */

#define CPLD_SMC_WD_WDI_REG                     (0x220)
#define  CPLD_SMC_WD_WDI_MASK                   (0x03)
#define  CPLD_SMC_WD_WDI_SHIFT                  (0)
#define   CPLD_SMC_WD_WDI_200MS                 (0x00)
#define   CPLD_SMC_WD_WDI_30S                   (0x01)
#define   CPLD_SMC_WD_WDI_60S                   (0x02)
#define   CPLD_SMC_WD_WDI_180S                  (0x03)

#define CPLD_SMC_WD_MASK_REG                    (0x221)
#define  CPLD_SMC_WD                            (1 << 0)    /* 0: Enable WD */

#define CPLD_SMC_SEP_RST_REG                    (0x222)
#define  CPLD_SMC_SEP_RST_PCIE                  (1 << 4)    /* 0: Reset */
#define  CPLD_SMC_SEP_RST_USB                   (1 << 3)    /* 0: Reset */
#define  CPLD_SMC_SEP_RST_50282                 (1 << 2)    /* 0: Reset */
#define  CPLD_SMC_SEP_RST_9548A                 (1 << 1)    /* 0: Reset */
#define  CPLD_SMC_SEP_RST_54616                 (1 << 0)    /* 0: Reset */

#define CPLD_SMC_SUS6_TRIG_MOD1_REG             (0x230)
#define  CPLD_SMC_SUS6_THERMAL_TRIG_MASK        (0x30)
#define  CPLD_SMC_SUS6_THERMAL_TRIG_SHIFT       (4)
#define  CPLD_SMC_SUS6_54616_TRIG_MASK          (0x0C))
#define  CPLD_SMC_SUS6_54616_TRIG_SHIFT         (2)
#define  CPLD_SMC_SUS6_50282_TRIG_MASK          (0x03)
#define  CPLD_SMC_SUS6_50282_TRIG_SHIFT         (0)

#define CPLD_SMC_SUS6_TRIG_MOD2_REG             (0x231)
#define  CPLD_SMC_SUS6_SWITCH_TRIG_MASK         (0xC0)
#define  CPLD_SMC_SUS6_SWITCH_TRIG_SHIFT        (6)
#define  CPLD_SMC_SUS6_FANTRAY_TRIG_MASK        (0x30)
#define  CPLD_SMC_SUS6_FANTRAY_TRIG_SHIFT       (4)
#define  CPLD_SMC_SUS6_FANCTRL_TRIG_MASK        (0x0C)
#define  CPLD_SMC_SUS6_FANCTRL_TRIG_SHIFT       (2)
#define  CPLD_SMC_SUS6_PSU_TRIG_MASK            (0x03)
#define  CPLD_SMC_SUS6_PSU_TRIG_SHIFT           (0)

#define CPLD_SMC_SUS6_COMBINE_REG               (0x232)
#define  CPLD_SMC_SUS6_SWITCH_COMBINE           (1 << 6)    /* 0: Int */
#define  CPLD_SMC_SUS6_FANTRAY_COMBINE          (1 << 5)    /* 0: Int */
#define  CPLD_SMC_SUS6_FANCTRL_COMBINE          (1 << 4)    /* 0: Int */
#define  CPLD_SMC_SUS6_PSU_COMBINE              (1 << 3)    /* 0: Int */
#define  CPLD_SMC_SUS6_THERMAL_COMBINE          (1 << 2)    /* 0: Int */
#define  CPLD_SMC_SUS6_54616_COMBINE            (1 << 1)    /* 0: Int */
#define  CPLD_SMC_SUS6_50282_COMBINE            (1 << 0)    /* 0: Int */

#define CPLD_SMC_SUS6_STA1_REG                  (0x233)
#define  CPLD_SMC_SUS6_THERMAL_INT_STA          (1 << 7)    /* 0: Int */
#define  CPLD_SMC_SUS6_54616_INT_STA            (1 << 6)    /* 0: Int */
#define  CPLD_SMC_SUS6_50282_6_INT_STA          (1 << 5)    /* 0: Int */
#define  CPLD_SMC_SUS6_50282_5_INT_STA          (1 << 4)    /* 0: Int */
#define  CPLD_SMC_SUS6_50282_4_INT_STA          (1 << 3)    /* 0: Int */
#define  CPLD_SMC_SUS6_50282_3_INT_STA          (1 << 2)    /* 0: Int */
#define  CPLD_SMC_SUS6_50282_2_INT_STA          (1 << 1)    /* 0: Int */
#define  CPLD_SMC_SUS6_50282_1_INT_STA          (1 << 0)    /* 0: Int */

#define CPLD_SMC_SUS6_STA2_REG                  (0x234)
#define  CPLD_SMC_SUS6_FAN3_PRESENT_STA         (1 << 7)    /* 0: Present */
#define  CPLD_SMC_SUS6_FAN2_PRESENT_STA         (1 << 6)    /* 0: Present */
#define  CPLD_SMC_SUS6_FAN1_PRESENT_STA         (1 << 5)    /* 0: Present */
#define  CPLD_SMC_SUS6_FANCTRL_INT_STA          (1 << 4)    /* 0: Int */
#define  CPLD_SMC_SUS6_PSUR_ALERT_STA           (1 << 3)    /* 0: Alert */
#define  CPLD_SMC_SUS6_PSUL_ALERT_STA           (1 << 2)    /* 0: Alert */
#define  CPLD_SMC_SUS6_PSUR_PRESENT_STA         (1 << 1)    /* 0: Present */
#define  CPLD_SMC_SUS6_PSUL_PRESENT_STA         (1 << 0)    /* 0: Present */

#define CPLD_SMC_SUS6_STA3_REG                  (0x235)
#define  CPLD_SMC_SUS6_SWITCH_STA               (1 << 0)    /* 0: Int */

#define CPLD_SMC_SUS6_INT1_REG                  (0x236)
#define  CPLD_SMC_SUS6_THERMAL_INT              (1 << 7)    /* 1: Int */
#define  CPLD_SMC_SUS6_54616_INT                (1 << 6)    /* 1: Int */
#define  CPLD_SMC_SUS6_50282_6_INT              (1 << 5)    /* 1: Int */
#define  CPLD_SMC_SUS6_50282_5_INT              (1 << 4)    /* 1: Int */
#define  CPLD_SMC_SUS6_50282_4_INT              (1 << 3)    /* 1: Int */
#define  CPLD_SMC_SUS6_50282_3_INT              (1 << 2)    /* 1: Int */
#define  CPLD_SMC_SUS6_50282_2_INT              (1 << 1)    /* 1: Int */
#define  CPLD_SMC_SUS6_50282_1_INT              (1 << 0)    /* 1: Int */

#define CPLD_SMC_SUS6_INT2_REG                  (0x237)
#define  CPLD_SMC_SUS6_FAN3_INT                 (1 << 7)    /* 1: Int */
#define  CPLD_SMC_SUS6_FAN2_INT                 (1 << 6)    /* 1: Int */
#define  CPLD_SMC_SUS6_FAN1_INT                 (1 << 5)    /* 1: Int */
#define  CPLD_SMC_SUS6_FANCTRL_INT              (1 << 4)    /* 1: Int */
#define  CPLD_SMC_SUS6_PSUR_ALERT_INT           (1 << 3)    /* 1: Int */
#define  CPLD_SMC_SUS6_PSUL_ALERT_INT           (1 << 2)    /* 1: Int */
#define  CPLD_SMC_SUS6_PSUR_PRESENT_INT         (1 << 1)    /* 1: Int */
#define  CPLD_SMC_SUS6_PSUL_PRESENT_INT         (1 << 0)    /* 1: Int */

#define CPLD_SMC_SUS6_INT3_REG                  (0x238)
#define  CPLD_SMC_SUS6_SWITCH_INT               (1 << 0)    /* 1: Int */

#define CPLD_SMC_SUS6_MASK1_REG                 (0x239)
#define  CPLD_SMC_SUS6_THERMAL_MASK             (1 << 7)    /* 1: Mask */
#define  CPLD_SMC_SUS6_54616_MASK               (1 << 6)    /* 1: Mask */
#define  CPLD_SMC_SUS6_50282_6_MASK             (1 << 5)    /* 1: Mask */
#define  CPLD_SMC_SUS6_50282_5_MASK             (1 << 4)    /* 1: Mask */
#define  CPLD_SMC_SUS6_50282_4_MASK             (1 << 3)    /* 1: Mask */
#define  CPLD_SMC_SUS6_50282_3_MASK             (1 << 2)    /* 1: Mask */
#define  CPLD_SMC_SUS6_50282_2_MASK             (1 << 1)    /* 1: Mask */
#define  CPLD_SMC_SUS6_50282_1_MASK             (1 << 0)    /* 1: Mask */

#define CPLD_SMC_SUS6_MASK2_REG                 (0x23a)
#define  CPLD_SMC_SUS6_FAN3_MASK                (1 << 7)    /* 1: Mask */
#define  CPLD_SMC_SUS6_FAN2_MASK                (1 << 6)    /* 1: Mask */
#define  CPLD_SMC_SUS6_FAN1_MASK                (1 << 5)    /* 1: Mask */
#define  CPLD_SMC_SUS6_FANCTRL_MASK             (1 << 4)    /* 1: Mask */
#define  CPLD_SMC_SUS6_PSUR_ALERT_MARK          (1 << 3)    /* 1: Mask */
#define  CPLD_SMC_SUS6_PSUL_ALERT_MARK          (1 << 2)    /* 1: Mask */
#define  CPLD_SMC_SUS6_PSUR_PRESENT_MASK        (1 << 1)    /* 1: Mask */
#define  CPLD_SMC_SUS6_PSUL_PRESENT_MASK        (1 << 0)    /* 1: Mask */

#define CPLD_SMC_SUS6_MASK3_REG                 (0x23b)
#define  CPLD_SMC_SUS6_SWITCH_MASK              (1 << 0)    /* 1: Mask */

#define CPLD_SMC_SUS7_TRIG_MODE_REG             (0x240)
#define  CPLD_SMC_SUS7_SFP_RXLOS_TRIG_MASK      (0x30)
#define  CPLD_SMC_SUS7_SFP_RXLOS_TRIG_SHIFT     (4)
#define  CPLD_SMC_SUS7_SFP_ABS_TRIG_MASK        (0x0C)
#define  CPLD_SMC_SUS7_SFP_ABS_TRIG_SHIFT       (2)
#define  CPLD_SMC_SUS7_SFP_TXFLT_TRIG_MASK      (0x03)
#define  CPLD_SMC_SUS7_SFP_TXFL_TRIG_SHIFT      (0)

#define CPLD_SMC_SUS7_COMBINE_REG               (0x241)
#define  CPLD_SMC_SUS7_SFP_RXLOS_COMBINE        (1 << 2)    /* 0: Int */
#define  CPLD_SMC_SUS7_SFP_ABS_COMBINE          (1 << 1)    /* 0: Int */
#define  CPLD_SMC_SUS7_SFP_TXFLT_COMBINE        (1 << 0)    /* 0: Int */

#define CPLD_SMC_SUS7_STA1_REG                  (0x242)
#define  CPLD_SMC_SUS7_SFP4_TXFLT_STA           (1 << 3)    /* 1: Fault */
#define  CPLD_SMC_SUS7_SFP3_TXFLT_STA           (1 << 2)    /* 1: Fault */
#define  CPLD_SMC_SUS7_SFP2_TXFLT_STA           (1 << 1)    /* 1: Fault */
#define  CPLD_SMC_SUS7_SFP1_TXFLT_STA           (1 << 0)    /* 1: Fault */

#define CPLD_SMC_SUS7_STA2_REG                  (0x243)
#define  CPLD_SMC_SUS7_SFP4_ABS_STA             (1 << 3)    /* 0: Present */
#define  CPLD_SMC_SUS7_SFP3_ABS_STA             (1 << 2)    /* 0: Present */
#define  CPLD_SMC_SUS7_SFP2_ABS_STA             (1 << 1)    /* 0: Present */
#define  CPLD_SMC_SUS7_SFP1_ABS_STA             (1 << 0)    /* 0: Present */

#define CPLD_SMC_SUS7_STA3_REG                  (0x244)
#define  CPLD_SMC_SUS7_SFP4_RXLOS_STA           (1 << 3)    /* 1: Loss */
#define  CPLD_SMC_SUS7_SFP3_RXLOS_STA           (1 << 2)    /* 1: Loss */
#define  CPLD_SMC_SUS7_SFP2_RXLOS_STA           (1 << 1)    /* 1: Loss */
#define  CPLS_SMC_SUS7_SFP1_RXLOS_STA           (1 << 0)    /* 1: Loss */

#define CPLD_SMC_SUS7_INT1_REG                  (0x246)
#define  CPLD_SMC_SUS7_SFP4_TXFLT_INT           (1 << 3)    /* 1: Int */
#define  CPLD_SMC_SUS7_SFP3_TXFLT_INT           (1 << 2)    /* 1: Int */
#define  CPLD_SMC_SUS7_SFP2_TXFLT_INT           (1 << 1)    /* 1: Int */
#define  CPLD_SMC_SUS7_SFP1_TXFLT_INT           (1 << 0)    /* 1: Int */

#define CPLD_SMC_SUS7_INT2_REG                  (0x247)
#define  CPLD_SMC_SUS7_SFP4_ABS_INT             (1 << 3)    /* 1: Int */
#define  CPLD_SMC_SUS7_SFP3_ABS_INT             (1 << 2)    /* 1: Int */
#define  CPLD_SMC_SUS7_SFP2_ABS_INT             (1 << 1)    /* 1: Int */
#define  CPLD_SMC_SUS7_SFP1_ABS_INT             (1 << 0)    /* 1: Int */

#define CPLD_SMC_SUS7_INT3_REG                  (0x248)
#define  CPLD_SMC_SUS7_SFP4_RXLOS_INT           (1 << 3)    /* 1: Int */
#define  CPLD_SMC_SUS7_SFP3_RXLOS_INT           (1 << 2)    /* 1: Int */
#define  CPLD_SMC_SUS7_SFP2_RXLOS_INT           (1 << 1)    /* 1: Int */
#define  CPLS_SMC_SUS7_SFP1_RXLOS_INT           (1 << 0)    /* 1: Int */

#define CPLD_SMC_SUS7_MASK1_REG                 (0x24a)
#define  CPLD_SMC_SUS7_SFP4_TXFLT_MASK          (1 << 3)    /* 1: Mask */
#define  CPLD_SMC_SUS7_SFP3_TXFLT_MASK          (1 << 2)    /* 1: Mask */
#define  CPLD_SMC_SUS7_SFP2_TXFLT_MASK          (1 << 1)    /* 1: Mask */
#define  CPLD_SMC_SUS7_SFP1_TXFLT_MASK          (1 << 0)    /* 1: Mask */

#define CPLD_SMC_SUS7_MASK2_REG                 (0x24b)
#define  CPLD_SMC_SUS7_SFP4_ABS_MASK            (1 << 3)    /* 1: Mask */
#define  CPLD_SMC_SUS7_SFP3_ABS_MASK            (1 << 2)    /* 1: Mask */
#define  CPLD_SMC_SUS7_SFP2_ABS_MASK            (1 << 1)    /* 1: Mask */
#define  CPLD_SMC_SUS7_SFP1_ABS_MASK            (1 << 0)    /* 1: Mask */

#define CPLD_SMC_SUS7_MASK3_REG                 (0x24c)
#define  CPLD_SMC_SUS7_SFP4_RXLOS_MASK          (1 << 3)    /* 1: Mask */
#define  CPLD_SMC_SUS7_SFP3_RXLOS_MASK          (1 << 2)    /* 1: Mask */
#define  CPLD_SMC_SUS7_SFP2_RXLOS_MASK          (1 << 1)    /* 1: Mask */
#define  CPLS_SMC_SUS7_SFP1_RXLOS_MASK          (1 << 0)    /* 1: Mask */

#define CPLD_SMC_SFPTX_CTRL_REG                 (0x255)
#define  CPLD_SMC_SFPTX_RATE_SEL_MASK           (0xF0)
#define  CPLD_SMC_SFPTX_RATE_SEL_SHIFT          (4)
#define   CPLD_SMC_SFPTX_SFP4_RATE_SEL          (1 << 7)    /* 0: 1G, 1: 10G */
#define   CPLD_SMC_SFPTX_SFP3_RATE_SEL          (1 << 6)    /* 0: 1G, 1: 10G */
#define   CPLD_SMC_SFPTX_SFP2_RATE_SEL          (1 << 5)    /* 0: 1G, 1: 10G */
#define   CPLD_SMC_SFPTX_SFP1_RATE_SEL          (1 << 4)    /* 0: 1G, 1: 10G */
#define  CPLD_SMC_SFPTX_TXEN_MASK               (0x0F)
#define  CPLD_SMC_SFPTX_TXEN_SHIFT              (0)
#define   CPLD_SMC_SFPTX_SFP4_TXEN              (1 << 3)    /* 0: Enable */
#define   CPLD_SMC_SFPTX_SFP3_TXEN              (1 << 2)    /* 0: Enable */
#define   CPLD_SMC_SFPTX_SFP2_TXEN              (1 << 1)    /* 0: Enable */
#define   CPLD_SMC_SFPTX_SFP1_TXEN              (1 << 0)    /* 0: Enable */

#endif /* DELL_S3000_CPLD_H__ */
