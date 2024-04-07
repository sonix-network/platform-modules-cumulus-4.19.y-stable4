/* SPDX-License-Identifier: GPL-2.0+ */
/*******************************************************************************
 *
 * @file    dellemc-s41xx-cplds.h
 * @brief   DellEMC CPLD definitions for S41XX platforms
 * @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 * @copyright Copyright (C) 2019 Cumulus Networks, Inc. All rights reserved
 *
 * @remark  This program is free software; you can redistribute it and/or modify
 *          it under the terms of the GNU General Public License as published by
 *          the Free Software Foundation; either version 2 of the License, or
 *          (at your option) any later version.
 *
 * @remark  This program is distributed in the hope that it will be useful,
 *          but WITHOUT ANY WARRANTY; without even the implied warranty of
 *          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *          General Public License for more details.
 *
 ******************************************************************************/

#ifndef DELLEMC_S41XX_CPLDS_H__
#define DELLEMC_S41XX_CPLDS_H__

/*******************************************************************************
 *
 *                     DellEMC CPLD Register Definitions
 *
 *  These register definitions are taken from the Mt. Baker S41xx CPLD Design
 *  Specification, Revision: 13, Dec 7, 2016, Editor Kelvin Wu, Ajith K Jacob &
 *  KK Lin
 *
 ******************************************************************************/

#define SYSTEM_CPLD_DRIVER_NAME                 "s41xx_system_cpld"
#define MASTER_CPLD_DRIVER_NAME                 "s41xx_master_cpld"
#define SLAVE_CPLD_DRIVER_NAME                  "s41xx_slave_cpld"
#define MUX_DRIVER_NAME                         "s41xx_port_mux"
#define DELL_S41XX_MUX_BUS_START                21

//------------------------------------------------------------------------------
//
//                         System CPLD Registers
//
//------------------------------------------------------------------------------

#define DELL_S41XX_SYS_CPLD_REV_REG                                     0x00
#  define DELL_S41XX_SYS_CPLD_REV_MJR_REV_MSB                           7
#  define DELL_S41XX_SYS_CPLD_REV_MJR_REV_LSB                           4
#  define DELL_S41XX_SYS_CPLD_REV_MNR_REV_MSB                           3
#  define DELL_S41XX_SYS_CPLD_REV_MNR_REV_LSB                           0

#define DELL_S41XX_SYS_CPLD_GPR_REG                                     0x01

#define DELL_S41XX_SYS_CPU_BRD_REV_TYPE_REG                             0x02
#  define DELL_S41XX_SYS_CPU_BRD_REV_TYPE_BRD_REV_MSB                   7
#  define DELL_S41XX_SYS_CPU_BRD_REV_TYPE_BRD_REV_LSB                   4
#  define DELL_S41XX_SYS_CPU_BRD_REV_TYPE_BRD_TYPE_MSB                  3
#  define DELL_S41XX_SYS_CPU_BRD_REV_TYPE_BRD_TYPE_LSB                  0

#define DELL_S41XX_SYS_SRR_REG                                          0x03
#  define DELL_S41XX_SYS_SRR_SSD_PRESNT_BIT                             7
#  define DELL_S41XX_SYS_SRR_SPI_CS_SEL_BIT                             5
#  define DELL_S41XX_SYS_SRR_RST_BIOS_SWITCH_BIT                        4
#  define DELL_S41XX_SYS_SRR_CPLD_UPGRADE_RST_BIT                       1

#define DELL_S41XX_SYS_EEPROM_WP_REG                                    0x04
#  define DELL_S41XX_SYS_EEPROM_WP_SYSTM_ID_EEPROM_WP_BIT               4
#  define DELL_S41XX_SYS_EEPROM_WP_SPI_WP_GBE_BIT                       3
#  define DELL_S41XX_SYS_EEPROM_WP_SPI_BIOS_WP_BIT                      2
#  define DELL_S41XX_SYS_EEPROM_WP_SPI_BAK_BIOS_WP_BIT                  1

#define DELL_S41XX_SYS_IRQ_REG                                          0x05
#  define DELL_S41XX_SYS_IRQ_LPC_CLK_FAIL_IRQ_EN_BIT                    7
#  define DELL_S41XX_SYS_IRQ_VRHOT_VCCP_IRQ_EN_BIT                      6
#  define DELL_S41XX_SYS_IRQ_CPU_THERMTRIP_IRQ_EN_BIT                   5
#  define DELL_S41XX_SYS_IRQ_TEMP_ALERT_IRQ_EN_BIT                      4
#  define DELL_S41XX_SYS_IRQ_LPC_CLK_FAIL_IRQ_BIT                       3
#  define DELL_S41XX_SYS_IRQ_VRHOT_VCCP_IRQ_BIT                         2
#  define DELL_S41XX_SYS_IRQ_CPU_THERMTRIP_IRQ_BIT                      1
#  define DELL_S41XX_SYS_IRQ_TEMP_ALERT_IRQ_BIT                         0

#define DELL_S41XX_SYS_WD_REG                                           0x06
#  define DELL_S41XX_SYS_WD_WD_TIMER_MSB                                6
#  define DELL_S41XX_SYS_WD_WD_TIMER_LSB                                4
#  define DELL_S41XX_SYS_WD_WD_EN_BIT                                   3
#  define DELL_S41XX_SYS_WD_WD_PUNCH_BIT                                0

#define DELL_S41XX_SYS_MB_RST_EN_REG                                    0x07
#  define DELL_S41XX_SYS_MB_RST_EN_BIT                                  0

#define DELL_S41XX_SYS_REBOOT_CAUSE_REG                                 0x08
#  define DELL_S41XX_SYS_REBOOT_CAUSE_COLD_RESET_BIT                    7
#  define DELL_S41XX_SYS_REBOOT_CAUSE_WARM_RESET_BIT                    6
#  define DELL_S41XX_SYS_REBOOT_CAUSE_THERMAL_SHUTDOWN_BIT              5
#  define DELL_S41XX_SYS_REBOOT_CAUSE_WD_FAIL_BIT                       4
#  define DELL_S41XX_SYS_REBOOT_CAUSE_BIOS_SWITCHOVER_BIT               3
#  define DELL_S41XX_SYS_REBOOT_CAUSE_BOOT_FAIL_BIT                     2
#  define DELL_S41XX_SYS_REBOOT_CAUSE_SHUT_DOWN_BIT                     1
#  define DELL_S41XX_SYS_REBOOT_CAUSE_PWR_ERR_BIT                       0

#define DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG                            0x09
#  define DELL_S41XX_SYS_CPU_PWR_EN_STATUS_V1P5_EN_BIT                  7
#  define DELL_S41XX_SYS_CPU_PWR_EN_STATUS_PWR_VDDR_EN_BIT              6
#  define DELL_S41XX_SYS_CPU_PWR_EN_STATUS_PWR_CORE_EN_BIT              5
#  define DELL_S41XX_SYS_CPU_PWR_EN_STATUS_V1P1_EN_BIT                  4
#  define DELL_S41XX_SYS_CPU_PWR_EN_STATUS_V1P0_EN_BIT                  3
#  define DELL_S41XX_SYS_CPU_PWR_EN_STATUS_V3P3_EN_BIT                  2
#  define DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG_1V8_EN_BIT               1
#  define DELL_S41XX_SYS_CPU_PWR_EN_STATUS_REG_1V35_EN_BIT              0

#define DELL_S41XX_SYS_CPU_PWR_STATUS_REG                               0x0A
#  define DELL_S41XX_SYS_CPU_PWR_STATUS_PG_DDR_VTT_BIT                  7
#  define DELL_S41XX_SYS_CPU_PWR_STATUS_PG_PVDDR_BIT                    6
#  define DELL_S41XX_SYS_CPU_PWR_STATUS_PG_PWR_CORE_BIT                 5
#  define DELL_S41XX_SYS_CPU_PWR_STATUS_PG_V1P1_BIT                     4
#  define DELL_S41XX_SYS_CPU_PWR_STATUS_PG_V1P0_BIT                     3
#  define DELL_S41XX_SYS_CPU_PWR_STATUS_PG_3V3_BIT                      2
#  define DELL_S41XX_SYS_CPU_PWR_STATUS_PG_1V8_BIT                      1
#  define DELL_S41XX_SYS_CPU_PWR_STATUS_PG_1V35_BIT                     0

//------------------------------------------------------------------------------
//
//                         Master CPLD Registers
//
//------------------------------------------------------------------------------

#define DELL_S41XX_MSTR_CPLD_REV_REG                                    0x00
#  define DELL_S41XX_MSTR_CPLD_REV_MJR_REV_MSB                          7
#  define DELL_S41XX_MSTR_CPLD_REV_MJR_REV_LSB                          4
#  define DELL_S41XX_MSTR_CPLD_REV_MNR_REV_MSB                          3
#  define DELL_S41XX_MSTR_CPLD_REV_MNR_REV_LSB                          0

#define DELL_S41XX_MSTR_CPLD_GPR_REG                                    0x01

#define DELL_S41XX_MSTR_MB_BRD_REV_TYPE_REG                             0x02
#  define DELL_S41XX_MSTR_MB_BRD_REV_TYPE_BRD_REV_MSB                   7
#  define DELL_S41XX_MSTR_MB_BRD_REV_TYPE_BRD_REV_LSB                   4
#  define DELL_S41XX_MSTR_MB_BRD_REV_TYPE_BRD_TYPE_MSB                  3
#  define DELL_S41XX_MSTR_MB_BRD_REV_TYPE_BRD_TYPE_LSB                  0

#define DELL_S41XX_MSTR_SRR_REG                                         0x03
#  define DELL_S41XX_MSTR_SRR_PORT_LED_RESET_MSB                        7
#  define DELL_S41XX_MSTR_SRR_PORT_LED_RESET_LSB                        6
#  define DELL_S41XX_MSTR_SRR_MB_RST_BIT                                5
#  define DELL_S41XX_MSTR_SRR_MICRO_USB_RST_BIT                         4
#  define DELL_S41XX_MSTR_SRR_1588_RST_BIT                              2
#  define DELL_S41XX_MSTR_SRR_NPU_RST_BIT                               1
#  define DELL_S41XX_MSTR_SRR_MGMT_PHY_RST_BIT                          0

#define DELL_S41XX_MSTR_FAN_EEPROM_WP_REG                               0x04
#  define DELL_S41XX_MSTR_FAN_EEPROM_WP_FAN4_EEPROM_WP_BIT              3
#  define DELL_S41XX_MSTR_FAN_EEPROM_WP_FAN3_EEPROM_WP_BIT              2
#  define DELL_S41XX_MSTR_FAN_EEPROM_WP_FAN2_EEPROM_WP_BIT              1
#  define DELL_S41XX_MSTR_FAN_EEPROM_WP_FAN1_EEPROM_WP_BIT              0

#define DELL_S41XX_MSTR_IRQ_REG                                         0x05
#  define DELL_S41XX_MSTR_IRQ_PS2_INT_BIT                               7
#  define DELL_S41XX_MSTR_IRQ_PS1_INT_BIT                               6
#  define DELL_S41XX_MSTR_IRQ_USB_FAULT_BIT                             5
#  define DELL_S41XX_MSTR_IRQ_BCM54616S_INT_BIT                         4
#  define DELL_S41XX_MSTR_IRQ_1588_MODULE_INT_BIT                       3
#  define DELL_S41XX_MSTR_IRQ_HOT_SWAP_INT2_BIT                         2
#  define DELL_S41XX_MSTR_IRQ_HOT_SWAP_INT1_BIT                         1
#  define DELL_S41XX_MSTR_IRQ_FAN_ALERT_INT_BIT                         0

#define DELL_S41XX_MSTR_SYSTEM_LED_REG                                  0x06
#  define DELL_S41XX_MSTR_SYSTEM_LED_FAN_LED_MSB                        7
#  define DELL_S41XX_MSTR_SYSTEM_LED_FAN_LED_LSB                        6
#  define DELL_S41XX_MSTR_SYSTEM_LED_SYSTEM_MSB                         5
#  define DELL_S41XX_MSTR_SYSTEM_LED_SYSTEM_LSB                         4
#  define DELL_S41XX_MSTR_SYSTEM_LED_BEACON_BIT                         3
#  define DELL_S41XX_MSTR_SYSTEM_LED_POWER_MSB                          2
#  define DELL_S41XX_MSTR_SYSTEM_LED_POWER_LSB                          1
#  define DELL_S41XX_MSTR_SYSTEM_LED_STACK_LED_BIT                      0

#define DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_REG                         0x07
#  define DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_LED_BLNK_BIT              6
#  define DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_LED_OFF_BIT               5
#  define DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_DOT_BIT                   4
#  define DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_DGT_MSB                   3
#  define DELL_S41XX_MSTR_SEVEN_DGT_STACK_LED_DGT_LSB                   0

#define DELL_S41XX_MSTR_FAN_TRAY_LED_REG                                0x08
#  define DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY4_LED_MSB                7
#  define DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY4_LED_LSB                6
#  define DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY3_LED_MSB                5
#  define DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY3_LED_LSB                4
#  define DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY2_LED_MSB                3
#  define DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY2_LED_LSB                2
#  define DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY1_LED_MSB                1
#  define DELL_S41XX_MSTR_FAN_TRAY_LED_FAN_TRAY1_LED_LSB                0

#define DELL_S41XX_MSTR_FAN_TRAY_STATUS_REG                             0x09
#  define DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY4_PRESENT_BIT         3
#  define DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY3_PRESENT_BIT         2
#  define DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY2_PRESENT_BIT         1
#  define DELL_S41XX_MSTR_FAN_TRAY_STATUS_FAN_TRAY1_PRESENT_BIT         0

#define DELL_S41XX_MSTR_MISC_CTRL_REG                                   0x0A
#  define DELL_S41XX_MSTR_MISC_CTRL_LED_PT_REGEN_BIT                    7
#  define DELL_S41XX_MSTR_MISC_CTRL_LED_TEST_MSB                        6
#  define DELL_S41XX_MSTR_MISC_CTRL_LED_TEST_LSB                        5
#  define DELL_S41XX_MSTR_MISC_CTRL_CPLD_SPI_WP_BIT                     4
#  define DELL_S41XX_MSTR_MISC_CTRL_MICRO_USB_SUSPEND_BIT               3
#  define DELL_S41XX_MSTR_MISC_CTRL_MSTR_IRQ_EN_BIT                     2
#  define DELL_S41XX_MSTR_MISC_CTRL_MICRO__BIT                          1
#  define DELL_S41XX_MSTR_MISC_CTRL_CNSL_SEL_BIT                        0

#define DELL_S41XX_MSTR_PSU_EN_STATUS_REG                               0x0B
#  define DELL_S41XX_MSTR_PSU_EN_STATUS_PS1_PS_BIT                      7
#  define DELL_S41XX_MSTR_PSU_EN_STATUS_PS1_PG_BIT                      6
#  define DELL_S41XX_MSTR_PSU_EN_STATUS_PS1_INT_BIT                     5
#  define DELL_S41XX_MSTR_PSU_EN_STATUS_PS1_ON_BIT                      4
#  define DELL_S41XX_MSTR_PSU_EN_STATUS_PS2_PS_BIT                      3
#  define DELL_S41XX_MSTR_PSU_EN_STATUS_PS2_PG_BIT                      2
#  define DELL_S41XX_MSTR_PSU_EN_STATUS_PS2_INT_BIT                     1
#  define DELL_S41XX_MSTR_PSU_EN_STATUS_PS2_ON_BIT                      0

#define DELL_S41XX_MSTR_MB_PWR_EN_STATUS_REG                            0x0C
#  define DELL_S41XX_MSTR_MB_PWR_EN_STATUS_USB1_VBUS_EN_BIT             7
#  define DELL_S41XX_MSTR_MB_PWR_EN_STATUS_VCC_1V25_EN_BIT              6
#  define DELL_S41XX_MSTR_MB_PWR_EN_STATUS_HOT_SWAP1_EN_BIT             5
#  define DELL_S41XX_MSTR_MB_PWR_EN_STATUS_HOT_SWAP2_EN_BIT             4
#  define DELL_S41XX_MSTR_MB_PWR_EN_STATUS_MAC_AVS1V_EN_BIT             3
#  define DELL_S41XX_MSTR_MB_PWR_EN_STATUS_MAC1V_EN_BIT                 2
#  define DELL_S41XX_MSTR_MB_PWR_EN_STATUS_V3P3_EN_BIT                  1
#  define DELL_S41XX_MSTR_MB_PWR_EN_STATUS_V5P0_EN_BIT                  0

#define DELL_S41XX_MSTR_MB_PWR_STATUS_REG                               0x0D
#  define DELL_S41XX_MSTR_MB_PWR_STATUS_VCC_3P3_CPLD_BIT                7
#  define DELL_S41XX_MSTR_MB_PWR_STATUS_VCC_1V25_PG_BIT                 6
#  define DELL_S41XX_MSTR_MB_PWR_STATUS_HOT_SWAP_PG1_BIT                5
#  define DELL_S41XX_MSTR_MB_PWR_STATUS_HOT_SWAP_PG2_BIT                4
#  define DELL_S41XX_MSTR_MB_PWR_STATUS_MAC_AVS1V_PG_BIT                3
#  define DELL_S41XX_MSTR_MB_PWR_STATUS_MAC1V_PG_BIT                    2
#  define DELL_S41XX_MSTR_MB_PWR_STATUS_VCC3V3_PG_BIT                   1
#  define DELL_S41XX_MSTR_MB_PWR_STATUS_VCC5V_PG_BIT                    0

#define DELL_S41XX_MSTR_NPU_ROV_REG                                     0x0E
#  define DELL_S41XX_MSTR_NPU__ROV_MSB                                  2
#  define DELL_S41XX_MSTR_NPU__ROV_LSB                                  0

#define DELL_S41XX_MSTR_MB_REBOOT_CAUSE_REG                             0x0F
#  define DELL_S41XX_MSTR_MB_REBOOT_CAUSE_COLD_RESET_BIT                7
#  define DELL_S41XX_MSTR_MB_REBOOT_CAUSE_WARM_RESET_BIT                6
#  define DELL_S41XX_MSTR_MB_REBOOT_CAUSE_THERMAL_SHUTDOWN_BIT          5
#  define DELL_S41XX_MSTR_MB_REBOOT_CAUSE_MB_PWR_EN_BIT                 4
#  define DELL_S41XX_MSTR_MB_REBOOT_CAUSE_FAN_PRESENT_BIT               3
#  define DELL_S41XX_MSTR_MB_REBOOT_CAUSE_PSU_FAIL_BIT                  2
#  define DELL_S41XX_MSTR_MB_REBOOT_CAUSE_SHUT_DOWN_BIT                 1
#  define DELL_S41XX_MSTR_MB_REBOOT_CAUSE_PWR_ERR_BIT                   0

#define DELL_S41XX_MSTR_PORT_I2C_MUX_REG                                0x10
#  define DELL_S41XX_MSTR_PORT_I2C_MUX_MSB                              5
#  define DELL_S41XX_MSTR_PORT_I2C_MUX_LSB                              0

#define DELL_S41XX_MSTR_IEEE_1588_REG                                   0x11
#  define DELL_S41XX_MSTR_IEEE_1588_GPIO_BIT                            7
#  define DELL_S41XX_MSTR_IEEE_1588_1588_PG_BIT                         6
#  define DELL_S41XX_MSTR_IEEE_1588_BK_CLK_VLD_BIT                      5
#  define DELL_S41XX_MSTR_IEEE_1588_CLK_VLD_BIT                         4
#  define DELL_S41XX_MSTR_IEEE_1588_FSEL_MSB                            3
#  define DELL_S41XX_MSTR_IEEE_1588_FSEL_LSB                            2
#  define DELL_S41XX_MSTR_IEEE_1588_MODULE_ID_MSB                       1
#  define DELL_S41XX_MSTR_IEEE_1588_MODULE_ID_LSB                       0

#define DELL_S41XX_MSTR_IRQ_MSK_REG                                     0x12
#  define DELL_S41XX_MSTR_IRQ_MSK_PS2_INT_MSK_BIT                       7
#  define DELL_S41XX_MSTR_IRQ_MSK_PS1_INT_MSK_BIT                       6
#  define DELL_S41XX_MSTR_IRQ_MSK_USB_FAULT_MSK_BIT                     5
#  define DELL_S41XX_MSTR_IRQ_MSK_BCM54616S_INT_MSK_BIT                 4
#  define DELL_S41XX_MSTR_IRQ_MSK_1588_MODULE_INT_MSK_BIT               3
#  define DELL_S41XX_MSTR_IRQ_MSK_HOT_SWAP_INT2_MSK_BIT                 2
#  define DELL_S41XX_MSTR_IRQ_MSK_HOT_SWAP_INT1_MSK_BIT                 1
#  define DELL_S41XX_MSTR_IRQ_MSK_FAN_ALERT_INT_MSK_BIT                 0

#define DELL_S41XX_MSTR_QSFP_PRESENT_STATUS_REG                         0x18
#  define DELL_S41XX_MSTR_QSFP_PRESENT_STATUS_PORT30_PRESENT_BIT        5
#  define DELL_S41XX_MSTR_QSFP_PRESENT_STATUS_PORT29_PRESENT_BIT        4
#  define DELL_S41XX_MSTR_QSFP_PRESENT_STATUS_PORT28_PRESENT_BIT        3
#  define DELL_S41XX_MSTR_QSFP_PRESENT_STATUS_PORT27_PRESENT_BIT        2
#  define DELL_S41XX_MSTR_QSFP_PRESENT_STATUS_PORT26_PRESENT_BIT        1
#  define DELL_S41XX_MSTR_QSFP_PRESENT_STATUS_PORT25_PRESENT_BIT        0

#define DELL_S41XX_MSTR_QSFP_RST_REG                                    0x19
#  define DELL_S41XX_MSTR_QSFP_RST_PORT30_RST_BIT                       5
#  define DELL_S41XX_MSTR_QSFP_RST_PORT29_RST_BIT                       4
#  define DELL_S41XX_MSTR_QSFP_RST_PORT28_RST_BIT                       3
#  define DELL_S41XX_MSTR_QSFP_RST_PORT27_RST_BIT                       2
#  define DELL_S41XX_MSTR_QSFP_RST_PORT26_RST_BIT                       1
#  define DELL_S41XX_MSTR_QSFP_RST_PORT25_RST_BIT                       0

#define DELL_S41XX_MSTR_QSFP_MODSEL_REG                                 0x1A
#  define DELL_S41XX_MSTR_QSFP_MODSEL_PORT30_MODSEL_BIT                 5
#  define DELL_S41XX_MSTR_QSFP_MODSEL_PORT29_MODSEL_BIT                 4
#  define DELL_S41XX_MSTR_QSFP_MODSEL_PORT28_MODSEL_BIT                 3
#  define DELL_S41XX_MSTR_QSFP_MODSEL_PORT27_MODSEL_BIT                 2
#  define DELL_S41XX_MSTR_QSFP_MODSEL_PORT26_MODSEL_BIT                 1
#  define DELL_S41XX_MSTR_QSFP_MODSEL_PORT25_MODSEL_BIT                 0

#define DELL_S41XX_MSTR_QSFP_LPMODE_REG                                 0x1B
#  define DELL_S41XX_MSTR_QSFP_LPMODE_PORT30_LPMODE_BIT                 5
#  define DELL_S41XX_MSTR_QSFP_LPMODE_PORT29_LPMODE_BIT                 4
#  define DELL_S41XX_MSTR_QSFP_LPMODE_PORT28_LPMODE_BIT                 3
#  define DELL_S41XX_MSTR_QSFP_LPMODE_PORT27_LPMODE_BIT                 2
#  define DELL_S41XX_MSTR_QSFP_LPMODE_PORT26_LPMODE_BIT                 1
#  define DELL_S41XX_MSTR_QSFP_LPMODE_PORT25_LPMODE_BIT                 0

#define DELL_S41XX_MSTR_QSFP_IRQ_STATUS_REG                             0x1C
#  define DELL_S41XX_MSTR_QSFP_IRQ_STATUS_PORT30__BIT                   5
#  define DELL_S41XX_MSTR_QSFP_IRQ_STATUS_PORT29__BIT                   4
#  define DELL_S41XX_MSTR_QSFP_IRQ_STATUS_PORT28__BIT                   3
#  define DELL_S41XX_MSTR_QSFP_IRQ_STATUS_PORT27__BIT                   2
#  define DELL_S41XX_MSTR_QSFP_IRQ_STATUS_PORT26__BIT                   1
#  define DELL_S41XX_MSTR_QSFP_IRQ_STATUS_PORT25_IRQ_STATUS_BIT         0

#define DELL_S41XX_MSTR_QSFP_IRQ_MSK_REG                                0x1D
#  define DELL_S41XX_MSTR_QSFP_IRQ_MSK_PORT30__BIT                      5
#  define DELL_S41XX_MSTR_QSFP_IRQ_MSK_PORT29__BIT                      4
#  define DELL_S41XX_MSTR_QSFP_IRQ_MSK_PORT28__BIT                      3
#  define DELL_S41XX_MSTR_QSFP_IRQ_MSK_PORT27__BIT                      2
#  define DELL_S41XX_MSTR_QSFP_IRQ_MSK_PORT26__BIT                      1
#  define DELL_S41XX_MSTR_QSFP_IRQ_MSK_PORT25_IRQ_MSK_BIT               0

#define DELL_S41XX_MSTR_PORT_8_1_PRESENT_STATUS_REG                     0x20

#define DELL_S41XX_MSTR_PORT_16_9_PRESENT_STATUS_REG                    0x21

#define DELL_S41XX_MSTR_PORT_8_1_TX_DISABLE_REG                         0x22

#define DELL_S41XX_MSTR_PORT_16_9_TX_DISABLE_REG                        0x23

#define DELL_S41XX_MSTR_PORT_8_1_RX_LOS_STATUS_REG                      0x24

#define DELL_S41XX_MSTR_PORT_16_9_RX_LOS_STATUS_REG                     0x25

#define DELL_S41XX_MSTR_PORT_8_1_TX_FAULT_STATUS_REG                    0x26

#define DELL_S41XX_MSTR_PORT_16_9_TX_FAULT_STATUS_REG                   0x27

#define DELL_S41XX_MSTR_PHY_8_1_RST_REG                                 0x28

#define DELL_S41XX_MSTR_PHY_12_9_RST_REG                                0x29
#  define DELL_S41XX_MSTR_PHY_12_9_RST_PHY_RST_MSB                      3
#  define DELL_S41XX_MSTR_PHY_12_9_RST_PHY_RST_LSB                      0

#define DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_REG                           0x2A
#  define DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_VCC2V5_EN_BIT               7
#  define DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_CT1V9_EN_BIT                6
#  define DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_PHY_1V_R_EN_BIT             5
#  define DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_PHY_1V_L_EN_BIT             4
#  define DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_VCC2V5_PG_BIT               3
#  define DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_CT1V9_PG_BIT                2
#  define DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_PHY_1V_R_PG_BIT             1
#  define DELL_S41XX_MSTR_PHY_PWR_EN_STATUS_PHY_1V_L_PG_BIT             0

#define DELL_S41XX_MSTR_PHY_8_1_IRQ_STATUS_REG                          0x2B

#define DELL_S41XX_MSTR_PHY_12_9_IRQ_STATUS_MSK_REG                     0x2C
#  define DELL_S41XX_MSTR_PHY_12_9_IRQ_STATUS_MSK_PHY_IRQ_MSK_MSB       7
#  define DELL_S41XX_MSTR_PHY_12_9_IRQ_STATUS_MSK_PHY_IRQ_MSK_LSB       4
#  define DELL_S41XX_MSTR_PHY_12_9_IRQ_STATUS_MSK_PHY_IRQ_STATUS_MSB    3
#  define DELL_S41XX_MSTR_PHY_12_9_IRQ_STATUS_MSK_PHY_IRQ_STATUS_LSB    0

#define DELL_S41XX_MSTR_PLL_STATUS_REG                                  0x2D
#  define DELL_S41XX_MSTR_PLL_STATUS_CNSL_MUX_MSB                       7
#  define DELL_S41XX_MSTR_PLL_STATUS_CNSL_MUX_LSB                       4
#  define DELL_S41XX_MSTR_PLL_STATUS_PLL_STS_BIT                        0

#define DELL_S41XX_MSTR_UPORT_CTRL_REG                                  0x2E
#  define DELL_S41XX_MSTR_U_PORT_CTRL_BIT                               0

#define DELL_S41XX_MSTR_PHY_8_1_IRQ_MSK_REG                             0x2F

#define DELL_S41XX_MSTR_ACCURACY_7_0_REG                                0x30

#define DELL_S41XX_MSTR_NANOSEC_7_0_REG                                 0x31

#define DELL_S41XX_MSTR_NANOSEC_15_8_REG                                0x32

#define DELL_S41XX_MSTR_NANOSEC_23_16_REG                               0x33

#define DELL_S41XX_MSTR_NANOSEC_32_24_REG                               0x34
#  define DELL_S41XX_MSTR_NANOSEC_32_24_NANOSEC_MSB                     5
#  define DELL_S41XX_MSTR_NANOSEC_32_24_NANOSEC_LSB                     0

#define DELL_S41XX_MSTR_SEC_7_0_REG                                     0x35

#define DELL_S41XX_MSTR_SEC_15_8_REG                                    0x36

#define DELL_S41XX_MSTR_SEC_23_16_REG                                   0x37

#define DELL_S41XX_MSTR_SEC_32_24_REG                                   0x38

#define DELL_S41XX_MSTR_EPOCH_7_0_REG                                   0x39

#define DELL_S41XX_MSTR_EPOCH_15_8_REG                                  0x3A

#define DELL_S41XX_MSTR_LOCK_7_0_REG                                    0x3B
#  define DELL_S41XX_MSTR_LOCK_7_0_LOCK_BIT                             0

//------------------------------------------------------------------------------
//
//                         Slave CPLD Registers
//
//------------------------------------------------------------------------------

#define DELL_S41XX_SLV_CPLD_REV_REG                                     0x00
#  define DELL_S41XX_SLV_CPLD_REV_MJR_REV_MSB                           7
#  define DELL_S41XX_SLV_CPLD_REV_MJR_REV_LSB                           4
#  define DELL_S41XX_SLV_CPLD_REV_MNR_REV_MSB                           3
#  define DELL_S41XX_SLV_CPLD_REV_MNR_REV_LSB                           0

#define DELL_S41XX_SLV_CPLD_GPR_REG                                     0x01

/*
 *  This is really how the datasheet explains these register offsets.  No joke.
 *
 *  Note: ports 25-30 are QSFP so they don't show up here. Except on the S4128
 *        where the QSFP ports are 25-26.
 *
 *  PORT_[j+8*(i+1)-1]_[j+8*i]_PRESENT_STATUS offset=0x10+i*1
 *      0<=i<=3, j=17+k where k=0 when i=0 else 6
 *    except on the S4128:
 *      0<=i<=1, j=17+k where k=0 when i=0 else 2
 *
 *  If my math is correct that, expands to:
 *
 *  i=0: (k=0,j=17): [17+8*(0+1)-1]-[17+8*0] offset=0x10+0
 *                    24:17 offset 0x10
 *  i=1: (k=6,j=23): [23+8*(1+1)-1]-[23+8*1] offset=0x10+1
 *                    38:31 offset 0x11
 *  i=2: (k=6,j=23): [23+8*(2+1)-1]-[23+8*2] offset=0x10+2
 *                    46:39 offset 0x12
 *  i=3: (k=6,j=23): [23+8*(3+1)-1]-[23+8*3] offset=0x10+3
 *                    54:47 offset 0x13
 * and on the S4148:
 *  i=1: (k=2,j=19): [19+8*(1+1)-1]-[19+8*1] offset=0x10+1
 *                    34:27 offset 0x11
 */
#define DELL_S41XX_SLV_CPLD_PORT_24_17_PRESENT_STATUS_REG               0x10
/* 25-30 are QSFP */
#define DELL_S41XX_SLV_CPLD_PORT_38_31_PRESENT_STATUS_REG               0x11
#define DELL_S41XX_SLV_CPLD_PORT_46_39_PRESENT_STATUS_REG               0x12
#define DELL_S41XX_SLV_CPLD_PORT_54_47_PRESENT_STATUS_REG               0x13

#define DELL_S41XX_SLV_CPLD_PORT_24_17_TX_DISABLE_REG                   0x14
/* 25-30 are QSFP */
#define DELL_S41XX_SLV_CPLD_PORT_38_31_TX_DISABLE_REG                   0x15
#define DELL_S41XX_SLV_CPLD_PORT_46_39_TX_DISABLE_REG                   0x16
#define DELL_S41XX_SLV_CPLD_PORT_54_47_TX_DISABLE_REG                   0x17

#define DELL_S41XX_SLV_CPLD_PORT_24_17_RX_LOS_STATUS_REG                0x18
/* 25-30 are QSFP */
#define DELL_S41XX_SLV_CPLD_PORT_38_31_RX_LOS_STATUS_REG                0x19
#define DELL_S41XX_SLV_CPLD_PORT_46_39_RX_LOS_STATUS_REG                0x1A
#define DELL_S41XX_SLV_CPLD_PORT_54_47_RX_LOS_STATUS_REG                0x1B

#define DELL_S41XX_SLV_CPLD_PORT_24_17_TX_FAULT_STATUS_REG              0x1C
/* 25-30 are QSFP */
#define DELL_S41XX_SLV_CPLD_PORT_38_31_TX_FAULT_STATUS_REG              0x1D
#define DELL_S41XX_SLV_CPLD_PORT_46_39_TX_FAULT_STATUS_REG              0x1E
#define DELL_S41XX_SLV_CPLD_PORT_54_47_TX_FAULT_STATUS_REG              0x1F

/* S4128 only */
/* 25-26 are QSFP */
#define DELL_S41XX_SLV_CPLD_PORT_34_27_PRESENT_STATUS_REG               0x11
#define DELL_S41XX_SLV_CPLD_PORT_34_27_TX_DISABLE_REG                   0x15
#define DELL_S41XX_SLV_CPLD_PORT_34_27_RX_LOS_STATUS_REG                0x19
#define DELL_S41XX_SLV_CPLD_PORT_34_27_TX_FAULT_STATUS_REG              0x1D

#endif /* DELLEMC_S41XX_CPLDS_H__ */
