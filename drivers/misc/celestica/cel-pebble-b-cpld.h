/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Celestica Pebble BMC E1052 CPLD Platform Definitions
 *
 * Dave Olson <olson@cumulusnetworks.com>
 *
 */

#ifndef CEL_PEBBLE_B_CPLD_H__
#define CEL_PEBBLE_B_CPLD_H__

#define CPLD_IO_BASE 0x100
#define CPLD_IO_SIZE 0x55

/*
 * register info from R1085-M0011-1 Pebble BMC CPLD Logic design spec
 *
 *  CPLD_MMC (Module Management)
 *    - CPU boot backup
 *    - On-board power sequencing controller
 *    - On-board reset circuitry
 *    - Interrupt Logic
 *    - PWM fan controller
 *    - LED control
 */
#define CPLD_MMC_VERSION_REG               (0x00)
#define  CPLD_MMC_VERSION_H_MASK           (0xF0)    /* Board version */
#define  CPLD_MMC_VERSION_H_SHIFT          (4)
#define  CPLD_MMC_VERSION_L_MASK           (0x0F)    /* MMC minor version */
#define  CPLD_MMC_VERSION_L_SHIFT          (0)

#define CPLD_MMC_SW_SCRATCH_REG            (0x01)

#define CPLD_MMC_BOOT_OK_REG               (0x02)
#define  CPLD_MMC_CPU_BOOT_STA             BIT(1)  /* 0: Boot OK */
#define  CPLD_MMC_CPU_BIOS                 BIT(0)  /* 0: BIOS0, 1: BIOS1 */

#define CPLD_MMC_FLASH_WP_REG              (0x03)
#define  CPLD_MMC_FLASH_WP                 BIT(3)  /* 0: Disable write prot */
#define  CPLD_MMC_BACKUP_FLASH_WP          BIT(2)
#define  CPLD_MMC_EEPROM_WP                BIT(0)

#define CPLD_MMC_PRESENT_MB_REG            (0x04)
#define  CPLD_MMC_PRESENT_MB               BIT(0)  /* 0: Mainboard present */

#define CPLD_MMC_BIOS_SELECT               (0x05)
#define  CPLD_MMC_BIOS_SEL                 BIT(0)  /* 0, bios 0, 1, bios 1 */
#define  CPLD_MMC_BIOS_SELSTAT             BIT(0)  /* 0: h/w ctrl, 1 s/w */

#define CPLD_MMC_WD_MASK_REG               (0x11)
#define  CPLD_MMC_WD                       BIT(0)  /* 0: Enable WD */

#define CPLD_MMC_SEP_RST_REG               (0x12)
#define  CPLD_MMC_SEP_RST1_PCIE_56150_2     BIT(3)  /* 0: Reset */
#define  CPLD_MMC_SEP_RST1_PCIE_56150_1     BIT(2)  /* 0: Reset */
#define  CPLD_MMC_SEP_RST1_PCA9546          BIT(1)  /* 0: Reset */
#define  CPLD_MMC_SEP_RST1_B50210PHY        BIT(0)  /* 0: Reset */

#define CPLD_MMC_SEP_RST2_REG               (0x13)
#define  CPLD_MMC_SEP_RST2_I210             BIT(7)  /* 0: Reset */
#define  CPLD_MMC_SEP_RST2_USB              BIT(6)  /* 0: Reset */
#define  CPLD_MMC_SEP_RST2_BCM56150_2       BIT(5)  /* 0: Reset */
#define  CPLD_MMC_SEP_RST2_BCM56150_1       BIT(4)  /* 0: Reset */
#define  CPLD_MMC_SEP_RST2_B50282_2         BIT(3)  /* 0: Reset */
#define  CPLD_MMC_SEP_RST2_B50282_1         BIT(2)  /* 0: Reset */
#define  CPLD_MMC_SEP_RST2_PCA9548          BIT(1)  /* 0: Reset */
#define  CPLD_MMC_SEP_RST2_PCA9506          BIT(0)  /* 0: Reset */

#define CPLD_MMC_RST_SOURCE_REG            (0x14)
#define  CPLD_MMC_RST_SOURCE_MASK          (0xFF)
#define  CPLD_MMC_RST_SOURCE_SHIFT         (8)
#define   CPLD_MMC_RST_SOURCE_PWR_ON       (0x11)
#define   CPLD_MMC_RST_SOURCE_XDP          (0x22)
#define   CPLD_MMC_RST_SOURCE_WD           (0x33)
#define   CPLD_MMC_RST_SOURCE_CPU_COLD     (0x44)
#define   CPLD_MMC_RST_SOURCE_CPU_WARM     (0x55)
#define   CPLD_MMC_RST_SOURCE_SOFT_COLD    (0x66)
#define   CPLD_MMC_RST_SOURCE_SOFT_WARM    (0x77)
#define   CPLD_MMC_RST_SOURCE_BUTTON       (0x88)

#define CPLD_MMC_SUS0_TRIG_MOD_REG         (0x21)
#define  CPLD_MMC_SUS0_TRIG_FALLING         (0x00)
#define  CPLD_MMC_SUS0_TRIG_RISING          (0x01)
#define  CPLD_MMC_SUS0_TRIG_EDGE            (0x02)
#define  CPLD_MMC_SUS0_TRIG_LEVEL_LOW       (0x03)

#define CPLD_MMC_SUS0_STA_REG              (0x22)
#define  CPLD_MMC_SUS0_BMC_LM75B_STA       BIT(4)    /* 0: Int active */
#define  CPLD_MMC_SUS0_FAN2_STA            BIT(3)    /* 0: Int active */
#define  CPLD_MMC_SUS0_FAN1_STA            BIT(2)    /* 0: Int active */
#define  CPLD_MMC_SUS0_PCA9506_STA         BIT(1)    /* 0: Int active */
#define  CPLD_MMC_SUS0_LM75B_STA           BIT(0)    /* 0: Int active */

#define CPLD_MMC_SUS0_INT_REG              (0x23)
#define  CPLD_MMC_SUS0_SW_LM75B_ALERT_INT  BIT(4)    /* 1: Int active */
#define  CPLD_MMC_SUS0_FAN2_INT            BIT(3)    /* 1: Int active */
#define  CPLD_MMC_SUS0_FAN1_INT            BIT(2)    /* 1: Int active */
#define  CPLD_MMC_SUS0_PCA9506_INT         BIT(1)    /* 1: Int active */
#define  CPLD_MMC_SUS0_LM75B_ALERT_INT     BIT(0)    /* 1: Int active */

#define CPLD_MMC_SUS0_MASK_REG             (0x24)
#define  CPLD_MMC_SUS0_SW_LM75B_ALERT_MASK BIT(4)    /* 1: Masked */
#define  CPLD_MMC_SUS0_FAN2_MASK           BIT(3)    /* 1: Masked */
#define  CPLD_MMC_SUS0_FAN1_MASK           BIT(2)    /* 1: Masked */
#define  CPLD_MMC_SUS0_PCA9506_MASK        BIT(1)    /* 1: Masked */
#define  CPLD_MMC_SUS0_LM75B_ALERT_MASK    BIT(0)    /* 1: Masked */

#define CPLD_MMC_PHY_LED_REG               (0x26) /* mgmt ether LED */
#define  CPLD_MMC_PHY_LED_1G               (0x3) /* mgmt 1Gb */
#define  CPLD_MMC_PHY_LED_10_100           (0x2) /* mgmt 10M or 100M */
#define  CPLD_MMC_PHY_LED_NOLINK_ACT       (0x0) /* no link or activity */

#define CPLD_MMC_PORT_LED_CTRL             (0x27) /* mgmt ether LED */
#define  CPLD_MMC_PORT_LED_CTRL_RJ45       BIT(5) /* 0 off, 1 on */
#define  CPLD_MMC_PORT_LED_CTRL_SFP        BIT(4) /* 0 off, 1 on */
#define  CPLD_MMC_PORT_LED_CTRL_RJ45_GR    BIT(3) /* 1 green on */
#define  CPLD_MMC_PORT_LED_CTRL_RJ45_YEL   BIT(2) /* 1 yellow on */
						  /* both off, LED off */
#define  CPLD_MMC_PORT_LED_CTRL_SFP_GR     BIT(1) /* 1 green on */
#define  CPLD_MMC_PORT_LED_CTRL_SFP_YEL    BIT(0) /* 1 yellow on */
						  /* both off, LED off */

#define CPLD_MMC_SYS_LED_REG               (0x28)
#define  CPLD_MMC_SYS_PWRLED_AUTO          BIT(6) /* 0 auto, 1 reg ctrl */
#define  CPLD_MMC_SYS_PWRLED_SHIFT         (4)
#define   CPLD_MMC_SYS_PWRLED_OFF          (0x00)
#define   CPLD_MMC_SYS_PWRLED_GREEN        (0x01)
#define   CPLD_MMC_SYS_PWRLED_YELLOW       (0x02)
#define   CPLD_MMC_SYS_LED_RED             (0x03)
/* POE bits 2-3 unused */
#define  CPLD_MMC_SYS_LED_GREEN_MASK       (0x03)
#define  CPLD_MMC_SYS_LED_GREEN_SHIFT      (0)
#define   CPLD_MMC_SYS_LED_GREEN_OFF       (0x00)
#define   CPLD_MMC_SYS_LED_GREEN_SLOW_BLINK (0x01)
#define   CPLD_MMC_SYS_LED_GREEN_ON        (0x02)
#define   CPLD_MMC_SYS_LED_YELLOW          (0x03)

#define CPLD_MMC_PORT_LED_SEL_REG          (0x29)
#define  CPLD_MMC_PORT_LED_SEL_SFP         (0x03)
#define  CPLD_MMC_PORT_LED_SEL_RJ45        (0x02)
#define  CPLD_MMC_PORT_LED_SEL_TEST        (0x01)
#define  CPLD_MMC_PORT_LED_SEL_DFLT        (0x00) /* all off */

/* Fan speed is set and monitored by BMC */

#define CPLD_MMC_PWM1_CTRL_REG             (0x30)
#define  CPLD_MMC_PWM1_MASK                (0xFF)
#define  CPLD_MMC_PWM1_SHIFT               (0)

#define CPLD_MMC_PWM2_CTRL_REG             (0x31)
#define  CPLD_MMC_PWM2_MASK                (0xFF)
#define  CPLD_MMC_PWM2_SHIFT               (0)

#define CPLD_MMC_FAN1_TACH_REG             (0x32)
#define  CPLD_MMC_FAN1_TACH_MASK           (0xFF)
#define  CPLD_MMC_FAN1_TACH_SHIFT          (0)

#define CPLD_MMC_FAN2_TACH_REG             (0x33)
#define  CPLD_MMC_FAN2_TACH_MASK           (0xFF)
#define  CPLD_MMC_FAN2_TACH_SHIFT          (0)

/* The rest are currently unused, but here for completeness */

#define CPLD_MMC_FAN_WDOG                  (0x34)
#define  CPLD_MMC_FAN2_WDOG_ENAB           BIT(0) /* fan watchdog enab if 1 */

#define CPLD_MMC_UART_MUX                  (0x35)
#define  CPLD_MMC_UART_CPUG_ENAB           BIT(0) /* 0 for bmc, 1 cpu */

#define CPLD_MMC_MGMT_SFP_STA              (0x40)
#define  CPLD_MMC_MGMT_SFP_ABS             BIT(3)
#define  CPLD_MMC_MGMT_SFP_TX_FAULT        BIT(2)
#define  CPLD_MMC_MGMT_SFP_RX_LOS          BIT(1)
#define  CPLD_MMC_MGMT_SFP_TX_DIS_CTRL     BIT(0) /* 1 enable, 0 disable */

#define CPLD_MMC_BTN_CTRL                  (0x50) /* power/reset button */
#define  CPLD_MMC_BTN_CTRL_PWR_CYC         BIT(2)
#define  CPLD_MMC_BTN_CTRL_RST             BIT(1)
#define  CPLD_MMC_BTN_CTRL_PWR             BIT(0)

#define CPLD_MMC_CPU_STA                   (0x51)
#define  CPLD_MMC_BTN_CPU_STA_PMC_SUS_STAT BIT(2)
#define  CPLD_MMC_BTN_CPU_STA_PMC_S4       BIT(1)
#define  CPLD_MMC_BTN_CPU_STA_PMC_S3       BIT(0)

#define CPLD_MMC_SKU                       (0x52)
#define  CPLD_MMC_SKU_MASK                 (0x03) /* 0 - 75W+175W POE */
						  /* 2 - 100W PSU + BMC */
						  /* 3 - 65W PSU - no BMC */

#define CPLD_MMC_PSU_STA                   (0x53)
#define  CPLD_MMC_PSU_STA_2                BIT(1) /* 1 - OK, 0 no output */
#define  CPLD_MMC_PSU_STA_1                BIT(0) /* 1 - OK, 0 no output */

#define CPLD_MMC_HW_VERS                   (0x54)
#define  CPLD_MMC_PSU_VERS_MASK            (0x07)

#endif /* CEL_PEBBLE_B_CPLD_H__ */
