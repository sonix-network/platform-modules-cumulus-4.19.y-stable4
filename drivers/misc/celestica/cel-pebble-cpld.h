/*
 * Celestica Pebble CPLD Platform Definitions
 *
 * Vidya Ravipati <vidya@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef CEL_PEBBLE_CPLD_H__
#define CEL_PEBBLE_CPLD_H__

#define CPLD_IO_BASE 0x100
#define CPLD_IO_SIZE 0x155


/*------------------------------------------------------------------------------------
 *
 * register info from R0882-M0025-01 Rev0.4 Pebble MMC CPLD Logic design spec 
 *
 *  CPLD_MMC (Module Management)
 *    - CPU boot backup
 *    - On-board power sequencing controller
 *    - On-board reset circuitry
 *    - Interrupt Logic
 *    - PWM fan controller
 *    - LED control
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

#define CPLD_MMC_FLASH_WP_REG                   (0x103)
#define  CPLD_MMC_FLASH_WP                      (1 << 1)    /* 0: Disable write protect */
#define  CPLD_MMC_BACKUP_FLASH_WP               (1 << 0)

#define CPLD_MMC_PRESENT_MB_REG                 (0x104)
#define  CPLD_MMC_PRESENT_MB                    (1 << 0)    /* 0: Mainboard present */

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

#define CPLD_MMC_eUSB_RST_REG                    (0x114)
#define  CPLD_MMC_eUSB_RST                       (1 << 0)    /* 0: Reset */

#define CPLD_MMC_SEP_RST_REG                    (0x115)
#define  CPLD_MMC_SEP_RST_I210                  (1 << 7)    /* 0: Reset */
#define  CPLD_MMC_SEP_RST_PD96200               (1 << 6)    /* 0: Reset */
#define  CPLD_MMC_SEP_RST_BCM56150_2            (1 << 5)    /* 0: Reset */
#define  CPLD_MMC_SEP_RST_BCM56150_1            (1 << 4)    /* 0: Reset */
#define  CPLD_MMC_SEP_RST_B50282_2              (1 << 3)    /* 0: Reset */
#define  CPLD_MMC_SEP_RST_B50282_1              (1 << 2)    /* 0: Reset */
#define  CPLD_MMC_SEP_RST_PCA9548               (1 << 1)    /* 0: Reset */
#define  CPLD_MMC_SEP_RST_PCA9506               (1 << 0)    /* 0: Reset */

#define CPLD_MMC_SUS0_TRIG_MOD_REG              (0x121)
#define  CPLD_MMC_SUS0_FANTRIP_TRIG_MASK        (0x30)
#define  CPLD_MMC_SUS0_FANTRIP_TRIG_SHIFT       (4)
#define  CPLD_MMC_SUS0_PCA9506_TRIG_MASK        (0x0C)
#define  CPLD_MMC_SUS0_PCA9506_TRIG_SHIFT       (2)
#define  CPLD_MMC_SUS0_LM75B_TRIG_MASK          (0x03)
#define  CPLD_MMC_SUS0_LM75B_TRIG_SHIFT         (0)

#define CPLD_MMC_SUS0_STA_REG                   (0x122)
#define  CPLD_MMC_SUS0_FAN_STA                  (1 << 2)    /* 0: Int active */
#define  CPLD_MMC_SUS0_PCA9506_STA              (1 << 1)    /* 0: Int active */
#define  CPLD_MMC_SUS0_LM75B_STA                (1 << 0)    /* 0: Int active */

#define CPLD_MMC_SUS0_INT_REG                   (0x123)
#define  CPLD_MMC_SUS0_FAN_INT                  (1 << 2)    /* 1: Int active */
#define  CPLD_MMC_SUS0_PCA9506_INT              (1 << 1)    /* 1: Int active */
#define  CPLD_MMC_SUS0_LM75B_ALERT_INT          (1 << 0)    /* 1: Int active */

#define CPLD_MMC_SUS0_MASK_REG                  (0x124)
#define  CPLD_MMC_SUS0_FAN_MASK                 (1 << 2)    /* 1: Masked */
#define  CPLD_MMC_SUS0_PCA9506_MASK             (1 << 1)    /* 1: Masked */
#define  CPLD_MMC_SUS0_LM75B_ALERT_MASK         (1 << 0)    /* 1: Masked */

#define CPLD_MMC_SYS_LED_REG                    (0x128)
#define  CPLD_MMC_SYS_LED_MASK                  (0x0F)
#define  CPLD_MMC_SYS_LED_SHIFT                 (0)
#define   CPLD_MMC_SYS_LED_OFF                  (0x00)
#define   CPLD_MMC_SYS_LED_GREEN                (0x03)
#define   CPLD_MMC_SYS_LED_AMBER_BLINK          (0x05)
#define   CPLD_MMC_SYS_LED_AMBER                (0x0F)

#define CPLD_MMC_PWM1_CTRL_REG                  (0x130)
#define  CPLD_MMC_PWM1_MASK                     (0xFF)
#define  CPLD_MMC_PWM1_SHIFT                    (0)


#define CPLD_MMC_PWM2_CTRL_REG                  (0x131)
#define  CPLD_MMC_PWM2_MASK                     (0xFF)
#define  CPLD_MMC_PWM2_SHIFT                    (0)

#define CPLD_MMC_FAN1_TACH_REG                  (0x132)
#define  CPLD_MMC_FAN1_TACH_MASK                (0xFF)
#define  CPLD_MMC_FAN1_TACH_SHIFT               (0)

#define CPLD_MMC_FAN2_TACH_REG                  (0x133)
#define  CPLD_MMC_FAN2_TACH_MASK                (0xFF)
#define  CPLD_MMC_FAN2_TACH_SHIFT               (0)

#endif /* CEL_PEBBLE_CPLD_H__ */
