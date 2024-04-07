/* SPDX-License-Identifier: GPL-2.0+ */
/*******************************************************************************
 *
 * @file    dellemc-n32xx-n22xx-cplds.h
 * @brief   DellEMC CPLD definitions for N22XX and N32XX platforms
 * @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 * @copyright Copyright (C) 2020 Cumulus Networks, Inc. All rights reserved
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

#ifndef DELLEMC_N32XX_N22XX_CPLDS_H__
#define DELLEMC_N32XX_N22XX_CPLDS_H__

/*******************************************************************************
 *
 *                     DellEMC CPLD Register Definitions
 *
 *  These register definitions are taken from the Mt. Evans N16XX/N22XX/N32XX
 *  CPLD Register Specification, Revision: 07, April 30, 2019, Editor Ajith K
 *  Jacob
 *
 ******************************************************************************/

#define CPU_CPLD_DRIVER_NAME                    "n32xx_cpu_cpld"
#define SYSTEM_CPLD_DRIVER_NAME                 "n32xx_system_cpld"
#define LED_CPLD_DRIVER_NAME                    "n32xx_led_cpld"
#define PORT_CPLD_DRIVER_NAME                   "n32xx_port_cpld"

#define SYS_CPLD_MUX_DRIVER_NAME                "n32xx_cpld_mux"
#define PORT_MUX_DEVICE_NAME                    "n32xx_port_mux"
#define FAN_MUX_DEVICE_NAME                     "n32xx_fan_mux"
#define PSU_MUX_DEVICE_NAME                     "n32xx_psu_mux"

#define DELL_N32XX_N22XX_FAN_MUX_BUS_START      20
#define DELL_N32XX_N22XX_PSU_MUX_BUS_START      30
#define DELL_N32XX_N22XX_PORT_MUX_BUS_START     40
#define DELL_N3224F_PORT_MUX_BUS_START          48

struct system_cpld_mux_platform_data {
	struct i2c_client *cpld;
	char *dev_name;
};

//------------------------------------------------------------------------------
//
//                            CPU CPLD Registers
//
//------------------------------------------------------------------------------

#define DELL_N32XX_CPU_CPLD_REV_REG0_REG                                0x00

#define DELL_N32XX_CPU_CPLD_REV_REG1_REG                                0x01

#define DELL_N32XX_CPU_CPLD_GPR_REG                                     0x02

#define DELL_N32XX_CPU_BRD_REV_TYPE_REG                                 0x03
#  define DELL_N32XX_CPU_BRD_REV_TYPE_BRD_REV_MSB                       7
#  define DELL_N32XX_CPU_BRD_REV_TYPE_BRD_REV_LSB                       4
#  define DELL_N32XX_CPU_BRD_REV_TYPE_BRD_TYPE_MSB                      3
#  define DELL_N32XX_CPU_BRD_REV_TYPE_BRD_TYPE_LSB                      0

#define DELL_N32XX_CPU_SRR_REG                                          0x04
#  define DELL_N32XX_CPU_SRR_SPI_CS_SEL_BIT                             5
#  define DELL_N32XX_CPU_SRR_RST_BIOS_SWITCH_BIT                        4

#define DELL_N32XX_CPU_BIOS_WP_REG                                      0x05
#  define DELL_N32XX_CPU_BIOS_WP_SPI_BIOS_WP_BIT                        2
#  define DELL_N32XX_CPU_BIOS_WP_SPI_BAK_BIOS_WP_BIT                    1

#define DELL_N32XX_CPU_SYS_IRQ_REG                                      0x06
#  define DELL_N32XX_CPU_SYS_IRQ_EN_NMI_BIT                             1
#  define DELL_N32XX_CPU_SYS_IRQ_EN_SMI_BIT                             0

#define DELL_N32XX_CPU_SYS_WD_REG                                       0x07
#  define DELL_N32XX_CPU_SYS_WD_WD_TIMER_MSB                            6
#  define DELL_N32XX_CPU_SYS_WD_WD_TIMER_LSB                            4
#  define DELL_N32XX_CPU_SYS_WD_WD_EN_BIT                               3
#  define DELL_N32XX_CPU_SYS_WD_WD_PUNCH_BIT                            0

#define DELL_N32XX_CPU_REBOOT_CAUSE_REG                                 0x09
#  define DELL_N32XX_CPU_REBOOT_CAUSE_COLD_RESET_BIT                    7
#  define DELL_N32XX_CPU_REBOOT_CAUSE_WARM_RESET_BIT                    6
#  define DELL_N32XX_CPU_REBOOT_CAUSE_THERMAL_SHUTDOWN_BIT              5
#  define DELL_N32XX_CPU_REBOOT_CAUSE_WD_FAIL_BIT                       4
#  define DELL_N32XX_CPU_REBOOT_CAUSE_BIOS_SWITCHOVER_BIT               3
#  define DELL_N32XX_CPU_REBOOT_CAUSE_BOOT_FAIL_BIT                     2
#  define DELL_N32XX_CPU_REBOOT_CAUSE_SHUT_DOWN_BIT                     1
#  define DELL_N32XX_CPU_REBOOT_CAUSE_PWR_ERR_BIT                       0

#define DELL_N32XX_CPU_PWR_EN_STS1_REG                                  0x0A
#  define DELL_N32XX_CPU_PWR_EN_STS1_EN_VCCREF_BIT                      7
#  define DELL_N32XX_CPU_PWR_EN_STS1_EN_V2P5_BIT                        6
#  define DELL_N32XX_CPU_PWR_EN_STS1_EN_VDDQ_BIT                        5
#  define DELL_N32XX_CPU_PWR_EN_STS1_EN_V1P05_BIT                       4
#  define DELL_N32XX_CPU_PWR_EN_STS1_EN_VNN_BIT                         3
#  define DELL_N32XX_CPU_PWR_EN_STS1_EN_V1P8_BIT                        2
#  define DELL_N32XX_CPU_PWR_EN_STS1_EN_V3P3_BIT                        1
#  define DELL_N32XX_CPU_PWR_EN_STS1_EN_V3P3E_BIT                       0

#define DELL_N32XX_CPU_PWR_EN_STS2_REG                                  0x0B
#  define DELL_N32XX_CPU_PWR_EN_STS2_EN_VCC_BIT                         2
#  define DELL_N32XX_CPU_PWR_EN_STS2_EN_VTT_BIT                         1
#  define DELL_N32XX_CPU_PWR_EN_STS2_EN_VCCRAM_BIT                      0

#define DELL_N32XX_CPU_PWR_STS1_REG                                     0x0C
#  define DELL_N32XX_CPU_PWR_STS1_PG_VTT_BIT                            7
#  define DELL_N32XX_CPU_PWR_STS1_PG_VDDQ_BIT                           6
#  define DELL_N32XX_CPU_PWR_STS1_PG_VNN_BIT                            5
#  define DELL_N32XX_CPU_PWR_STS1_PG_V1P05_BIT                          4
#  define DELL_N32XX_CPU_PWR_STS1_PG_V1P8_BIT                           3
#  define DELL_N32XX_CPU_PWR_STS1_PG_V3P3_BIT                           2
#  define DELL_N32XX_CPU_PWR_STS1_PG_V3P3E_BIT                          1
#  define DELL_N32XX_CPU_PWR_STS1_PG_V3P3_AUX_BIT                       0

#define DELL_N32XX_CPU_PWR_STS2_REG                                     0x0D
#  define DELL_N32XX_CPU_PWR_STS2_PG_UCD_BIT                            4
#  define DELL_N32XX_CPU_PWR_STS2_PG_VCC_BIT                            3
#  define DELL_N32XX_CPU_PWR_STS2_PG_VCCRAM_BIT                         2
#  define DELL_N32XX_CPU_PWR_STS2_PG_VCCREF_BIT                         1
#  define DELL_N32XX_CPU_PWR_STS2_PG_V2P5_BIT                           0

#define DELL_N32XX_CPU_GPIO_STS1_REG                                    0x0E
#  define DELL_N32XX_CPU_GPIO_STS1_CPU_GPIO99_BIT                       7
#  define DELL_N32XX_CPU_GPIO_STS1_CPU_GPIO91_BIT                       6
#  define DELL_N32XX_CPU_GPIO_STS1_CPU_GPIO45_BIT                       5
#  define DELL_N32XX_CPU_GPIO_STS1_CPU_GPIO40_BIT                       4
#  define DELL_N32XX_CPU_GPIO_STS1_CPU_GPIO38_BIT                       3
#  define DELL_N32XX_CPU_GPIO_STS1_CPU_GPIO23_BIT                       2
#  define DELL_N32XX_CPU_GPIO_STS1_CPU_GPIO22_BIT                       1
#  define DELL_N32XX_CPU_GPIO_STS1_CPU_GPIO10_BIT                       0

#define DELL_N32XX_CPU_GPIO_STS2_REG                                    0x0F
#  define DELL_N32XX_CPU_GPIO_STS2_CPU_GPIO0_BIT                        3
#  define DELL_N32XX_CPU_GPIO_STS2_CPU_GPIO4_BIT                        2
#  define DELL_N32XX_CPU_GPIO_STS2_CPU_GPIO37_BIT                       1
#  define DELL_N32XX_CPU_GPIO_STS2_CPU_GPIO100_BIT                      0

#define DELL_N32XX_CPU_ERR_STS_REG                                      0x10
#  define DELL_N32XX_CPU_ERR_STS_MCERR_BIT                              4
#  define DELL_N32XX_CPU_ERR_STS_IERR_BIT                               3
#  define DELL_N32XX_CPU_ERR_STS_ERROR2_BIT                             2
#  define DELL_N32XX_CPU_ERR_STS_ERROR1_BIT                             1
#  define DELL_N32XX_CPU_ERR_STS_ERROR0_BIT                             0

#define DELL_N32XX_CPU_TEMP_STS_REG                                     0x11
#  define DELL_N32XX_CPU_TEMP_STS_TMP75_HOT_BIT                         5
#  define DELL_N32XX_CPU_TEMP_STS_VDDQ_HOT_BIT                          4
#  define DELL_N32XX_CPU_TEMP_STS_VCCRAM_HOT_BIT                        3
#  define DELL_N32XX_CPU_TEMP_STS_VCC_HOT_BIT                           2
#  define DELL_N32XX_CPU_TEMP_STS_VNN_HOT_BIT                           1
#  define DELL_N32XX_CPU_TEMP_STS_CPU_THERMTRIP_BIT                     0

#define DELL_N32XX_CPU_MISC_STS1_REG                                    0x12
#  define DELL_N32XX_CPU_MISC_STS1_TPM_ID_MSB                           7
#  define DELL_N32XX_CPU_MISC_STS1_TPM_ID_LSB                           5
#  define DELL_N32XX_CPU_MISC_STS1_UCD_ALERT_BIT                        4
#  define DELL_N32XX_CPU_MISC_STS1_VDDQ_FLT_BIT                         3
#  define DELL_N32XX_CPU_MISC_STS1_VCC_FLT_BIT                          2
#  define DELL_N32XX_CPU_MISC_STS1_WAKE1_BIT                            1
#  define DELL_N32XX_CPU_MISC_STS1_WAKE0_BIT                            0

#define DELL_N32XX_CPU_MISC_STS2_REG                                    0x13
#  define DELL_N32XX_CPU_MISC_STS2_CPU_SUS_STS_BIT                      5
#  define DELL_N32XX_CPU_MISC_STS2_PWR_BTN_STS_BIT                      4
#  define DELL_N32XX_CPU_MISC_STS2_XDP_SUS_STS_BIT                      3
#  define DELL_N32XX_CPU_MISC_STS2_USBA_HOST_PRENT_BIT                  2
#  define DELL_N32XX_CPU_MISC_STS2_SYSTEM_RESET_BIT                     1
#  define DELL_N32XX_CPU_MISC_STS2_CLK_VLD_BIT                          0

//------------------------------------------------------------------------------
//
//                         System CPLD Registers
//
//------------------------------------------------------------------------------

#define DELL_N32XX_SYS_CPLD_REV_REG0_REG                                0x00

#define DELL_N32XX_SYS_CPLD_REV_REG1_REG                                0x01

#define DELL_N32XX_SYS_CPLD_GPR_REG                                     0x02

#define DELL_N32XX_SYS_MB_BRD_REV_TYPE_REG                              0x03
#  define DELL_N32XX_SYS_MB_BRD_REV_TYPE_BRD_REV_MSB                    7
#  define DELL_N32XX_SYS_MB_BRD_REV_TYPE_BRD_REV_LSB                    5
#  define DELL_N32XX_SYS_MB_BRD_REV_TYPE_BRD_TYPE_MSB                   4
#  define DELL_N32XX_SYS_MB_BRD_REV_TYPE_BRD_TYPE_LSB                   0

#define DELL_N32XX_SYS_SRR_REG                                          0x04
#  define DELL_N32XX_SYS_SRR_PORT_LED_RESET_BIT                         7
#  define DELL_N32XX_SYS_SRR_POE_RST_DSBL_BIT                           6
#  define DELL_N32XX_SYS_SRR_CPLD_RST_BIT                               3
#  define DELL_N32XX_SYS_SRR_NPU2_RST_BIT                               2
#  define DELL_N32XX_SYS_SRR_NPU1_RST_BIT                               1
#  define DELL_N32XX_SYS_SRR_MGMT_PHY_RST_BIT                           0

#define DELL_N32XX_SYS_EEPROM_WP_REG                                    0x05
#  define DELL_N32XX_SYS_EEPROM_WP_CPLD_SPI_WP_BIT                      4
#  define DELL_N32XX_SYS_EEPROM_WP_SYS_EEPROM_WP_BIT                    3
#  define DELL_N32XX_SYS_EEPROM_WP_FAN3_EEPROM_WP_BIT                   2
#  define DELL_N32XX_SYS_EEPROM_WP_FAN2_EEPROM_WP_BIT                   1
#  define DELL_N32XX_SYS_EEPROM_WP_FAN1_EEPROM_WP_BIT                   0

#define DELL_N32XX_SYS_IRQ_STS_REG                                      0x06
#  define DELL_N32XX_SYS_IRQ_STS_TPM_INT_BIT                            6
#  define DELL_N32XX_SYS_IRQ_STS_USB_FAULT_BIT                          5
#  define DELL_N32XX_SYS_IRQ_STS_BCM54216S_INT_BIT                      4
#  define DELL_N32XX_SYS_IRQ_STS_UCD_IRQ_BIT                            3
#  define DELL_N32XX_SYS_IRQ_STS_FAN_ALERT_INT_BIT                      0

#define DELL_N32XX_SYS_SYSTEM_LED_REG                                   0x07
#  define DELL_N32XX_SYS_SYSTEM_LED_FAN_LED_MSB                         7
#  define DELL_N32XX_SYS_SYSTEM_LED_FAN_LED_LSB                         6
#  define DELL_N32XX_SYS_SYSTEM_LED_SYSTEM_MSB                          5
#  define DELL_N32XX_SYS_SYSTEM_LED_SYSTEM_LSB                          4
#  define DELL_N32XX_SYS_SYSTEM_LED_BEACON_BIT                          3
#  define DELL_N32XX_SYS_SYSTEM_LED_POWER_MSB                           2
#  define DELL_N32XX_SYS_SYSTEM_LED_POWER_LSB                           1
#  define DELL_N32XX_SYS_SYSTEM_LED_STACK_LED_BIT                       0

#define DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_REG                          0x08
#  define DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_LED_BLNK_BIT               6
#  define DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_LED_OFF_BIT                5
#  define DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_DOT_BIT                    4
#  define DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_DGT_MSB                    3
#  define DELL_N32XX_SYS_SEVEN_DGT_STACK_LED_DGT_LSB                    0

#define DELL_N32XX_SYS_FAN_TRAY_LED_REG                                 0x09
#  define DELL_N32XX_SYS_FAN_TRAY_LED3_FAN_MSB                          5
#  define DELL_N32XX_SYS_FAN_TRAY_LED3_FAN_LSB                          4
#  define DELL_N32XX_SYS_FAN_TRAY_LED2_FAN_MSB                          3
#  define DELL_N32XX_SYS_FAN_TRAY_LED2_FAN_LSB                          2
#  define DELL_N32XX_SYS_FAN_TRAY_LED1_FAN_MSB                          1
#  define DELL_N32XX_SYS_FAN_TRAY_LED1_FAN_LSB                          0

#define DELL_N32XX_SYS_FAN_TRAY_STATUS_REG                              0x0A
#  define DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN3_TYPE_BIT                  6
#  define DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN2_TYPE_BIT                  5
#  define DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN1_TYPE_BIT                  4
#  define DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN3_PRESENT_BIT               2
#  define DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN2_PRESENT_BIT               1
#  define DELL_N32XX_SYS_FAN_TRAY_STATUS_FAN1_PRESENT_BIT               0

#define DELL_N32XX_SYS_MISC_CTRL_REG                                    0x0B
#  define DELL_N32XX_SYS_MISC_CTRL_RST_LED_CLR_BIT                      7
#  define DELL_N32XX_SYS_MISC_CTRL_LED_TEST_MSB                         6
#  define DELL_N32XX_SYS_MISC_CTRL_LED_TEST_LSB                         5
#  define DELL_N32XX_SYS_MISC_CTRL_MICRO_USB_SUSPEND_BIT                3
#  define DELL_N32XX_SYS_MISC_CTRL_SYS_IRQ_EN_BIT                       2
#  define DELL_N32XX_SYS_MISC_CTRL_MICRO__BIT                           1
#  define DELL_N32XX_SYS_MISC_CTRL_FAN_EN_BIT                           0

#define DELL_N32XX_SYS_PSU_EN_STATUS_REG                                0x0C
#  define DELL_N32XX_SYS_PSU_EN_STATUS_PS1_PS_BIT                       7
#  define DELL_N32XX_SYS_PSU_EN_STATUS_PS1_PG_BIT                       6
#  define DELL_N32XX_SYS_PSU_EN_STATUS_PS1_INT_BIT                      5
#  define DELL_N32XX_SYS_PSU_EN_STATUS_PS1_ON_BIT                       4
#  define DELL_N32XX_SYS_PSU_EN_STATUS_PS2_PS_BIT                       3
#  define DELL_N32XX_SYS_PSU_EN_STATUS_PS2_PG_BIT                       2
#  define DELL_N32XX_SYS_PSU_EN_STATUS_PS2_INT_BIT                      1
#  define DELL_N32XX_SYS_PSU_EN_STATUS_PS2_ON_BIT                       0

#define DELL_N32XX_SYS_EPS_STATUS_REG                                   0x0e
#  define DELL_N32XX_SYS_EPS_STATUS_EPS_GPIO_MSB                        7
#  define DELL_N32XX_SYS_EPS_STATUS_EPS_GPIO_LSB                        4
#  define DELL_N32XX_SYS_EPS_STATUS_EPS_INT_BIT                         3
#  define DELL_N32XX_SYS_EPS_STATUS_EPS_PRSNT_BIT                       2
#  define DELL_N32XX_SYS_EPS_STATUS_EPS_PG_BIT                          1
#  define DELL_N32XX_SYS_EPS_STATUS_EPS_ALRM_BIT                        0

#define DELL_N32XX_SYS_POE_EN_STATUS_REG                                0x0D
#  define DELL_N32XX_SYS_POE_EN_STATUS_POE_CTRL_RST_BIT                 5
#  define DELL_N32XX_SYS_POE_EN_STATUS_POE_PORT_DIS_BIT                 4
#  define DELL_N32XX_SYS_POE_EN_STATUS_POE_I2C_RDY_BIT                  3
#  define DELL_N32XX_SYS_POE_EN_STATUS_POE_INT_BIT                      2
#  define DELL_N32XX_SYS_POE_EN_STATUS_POE_TMP_ALRM_BIT                 1
#  define DELL_N32XX_SYS_POE_EN_STATUS_POE_SYSTEM_OK_BIT                0

#define DELL_N32XX_SYS_MB_REBOOT_CAUSE_REG                              0x10
#  define DELL_N32XX_SYS_MB_REBOOT_CAUSE_COLD_RESET_BIT                 7
#  define DELL_N32XX_SYS_MB_REBOOT_CAUSE_WARM_RESET_BIT                 6
#  define DELL_N32XX_SYS_MB_REBOOT_CAUSE_THERMAL_SHUTDOWN_BIT           5
#  define DELL_N32XX_SYS_MB_REBOOT_CAUSE_WD_FAIL_BIT                    4
#  define DELL_N32XX_SYS_MB_REBOOT_CAUSE_CPU_THRMTRIP_BIT               3
#  define DELL_N32XX_SYS_MB_REBOOT_CAUSE_SHUT_DOWN_BIT                  2
#  define DELL_N32XX_SYS_MB_REBOOT_CAUSE_PWR_ERR_BIT                    1
#  define DELL_N32XX_SYS_MB_REBOOT_CAUSE_POR_RST_BIT                    0

#define DELL_N32XX_SYS_PORT_I2C_MUX_REG                                 0x11
#  define DELL_N32XX_SYS_PORT_I2C_MUX_MSB                               4
#  define DELL_N32XX_SYS_PORT_I2C_MUX_LSB                               0

#define DELL_N32XX_SYS_IRQ_MSK_REG                                      0x14
#  define DELL_N32XX_SYS_IRQ_MSK_PS2_INT_MSK_BIT                        7
#  define DELL_N32XX_SYS_IRQ_MSK_PS1_INT_MSK_BIT                        6
#  define DELL_N32XX_SYS_IRQ_MSK_USB_FAULT_MSK_BIT                      5
#  define DELL_N32XX_SYS_IRQ_MSK_BCM54216S_INT_MSK_IRP32XX_INT_MX_BIT   4
#  define DELL_N32XX_SYS_IRQ_MSK_POE_INT_MSK_BIT                        3
#  define DELL_N32XX_SYS_IRQ_MSK_EPS_INT_MSK_BIT                        2
#  define DELL_N32XX_SYS_IRQ_MSK_TPM_INT_MSK_UCD_INT_MSK_BIT            1
#  define DELL_N32XX_SYS_IRQ_MSK_FAN_ALERT_INT_MSK_BIT                  0

#define DELL_N32XX_SYS_FAN_I2C_MUX_REG                                  0x13
#  define DELL_N32XX_SYS_FAN_I2C_MUX_MSB                                1
#  define DELL_N32XX_SYS_FAN_I2C_MUX_LSB                                0

#define DELL_N32XX_SYS_PSU_I2C_MUX_REG                                  0x12
#  define DELL_N32XX_SYS_PSU_I2C_MUX_MSB                                1
#  define DELL_N32XX_SYS_PSU_I2C_MUX_LSB                                0

#define DELL_N32XX_SYS_CTRL_REG                                         0x15
#  define DELL_N32XX_SYS_CTRL_USBA_PWR_EN_BIT                           2
#  define DELL_N32XX_SYS_CTRL_PWR_DN_BIT                                1
#  define DELL_N32XX_SYS_CTRL_PWR_CYC_SYS_BIT                           0

#define DELL_N32XX_SYS_MGMT_GPIO_REG                                    0x16
#  define DELL_N32XX_SYS_MGMT_GPIO_MSB                                  3
#  define DELL_N32XX_SYS_MGMT_GPIO_LSB                                  1
#  define DELL_N32XX_SYS_MGMT_GPIO_BIT                                  0

#define DELL_N32XX_SYS_WD_REG                                           0x17
#  define DELL_N32XX_SYS_WD_WD_TIMER_MSB                                6
#  define DELL_N32XX_SYS_WD_WD_TIMER_LSB                                4
#  define DELL_N32XX_SYS_WD_WD_EN_BIT                                   3
#  define DELL_N32XX_SYS_WD_WD_PUNCH_BIT                                0

#define DELL_N32XX_SYS_FRNT_PNL_RST_STS_REG                             0x18
#  define DELL_N32XX_SYS_FRNT_PNL_RST_STS_RST_12S_VLD_BIT               2
#  define DELL_N32XX_SYS_FRNT_PNL_RST_STS_RST_8S_VLD_BIT                1
#  define DELL_N32XX_SYS_FRNT_PNL_RST_STS_RST_4S_VLD_BIT                0

#define DELL_N32XX_SYS_MISC_STS_REG                                     0x19
#  define DELL_N32XX_SYS_MISC_STS_NPU_GPIO9_BIT                         7
#  define DELL_N32XX_SYS_MISC_STS_NPU_GPIO2_BIT                         6
#  define DELL_N32XX_SYS_MISC_STS_NPU_GPIO1_BIT                         5
#  define DELL_N32XX_SYS_MISC_STS_NPU_GPIO11_BIT                        4
#  define DELL_N32XX_SYS_MISC_STS_PSU2_AC_GOOD_BIT                      3
#  define DELL_N32XX_SYS_MISC_STS_PSU1_AC_GOOD_BIT                      2
#  define DELL_N32XX_SYS_MISC_STS_EPS_GPIO6_BIT                         1
#  define DELL_N32XX_SYS_MISC_STS_EPS_GPIO5_BIT                         0

#define DELL_N32XX_SYS_QSFP_PRESENT_STATUS_REG                          0x20
#  define DELL_N32XX_SYS_QSFP_PRESENT_STATUS_PORT2_PRESENT_BIT          1
#  define DELL_N32XX_SYS_QSFP_PRESENT_STATUS_PORT1_PRESENT_BIT          0
#  define DELL_N32XX_SYS_QSFP_PRESENT_STATUS_PORT_PRESENT_MSB           1
#  define DELL_N32XX_SYS_QSFP_PRESENT_STATUS_PORT_PRESENT_LSB           0

#define DELL_N32XX_SYS_QSFP_RST_REG                                     0x21
#  define DELL_N32XX_SYS_QSFP_RST_PORT2_RST_BIT                         1
#  define DELL_N32XX_SYS_QSFP_RST_PORT1_RST_BIT                         0
#  define DELL_N32XX_SYS_QSFP_RST_PORT_RST_MSB                          1
#  define DELL_N32XX_SYS_QSFP_RST_PORT_RST_LSB                          0

#define DELL_N32XX_SYS_QSFP_LPMODE_REG                                  0x22
#  define DELL_N32XX_SYS_QSFP_LPMODE_PORT2_LPMODE_BIT                   1
#  define DELL_N32XX_SYS_QSFP_LPMODE_PORT1_LPMODE_BIT                   0
#  define DELL_N32XX_SYS_QSFP_LPMODE_PORT_LPMODE_MSB                    1
#  define DELL_N32XX_SYS_QSFP_LPMODE_PORT_LPMODE_LSB                    0

#define DELL_N32XX_SYS_QSFP_IRQ_STATUS_REG                              0x23
#  define DELL_N32XX_SYS_QSFP_IRQ_STATUS_PORT2_IRQ_STATUS_BIT           1
#  define DELL_N32XX_SYS_QSFP_IRQ_STATUS_PORT1_IRQ_STATUS_BIT           0
#  define DELL_N32XX_SYS_QSFP_IRQ_STATUS_PORT_IRQ_STATUS_MSB            1
#  define DELL_N32XX_SYS_QSFP_IRQ_STATUS_PORT_IRQ_STATUS_LSB            0

#define DELL_N32XX_SYS_QSFP_IRQ_MSK_REG                                 0x24
#  define DELL_N32XX_SYS_QSFP_IRQ_MSK_PORT2_IRQ_MSK_BIT                 1
#  define DELL_N32XX_SYS_QSFP_IRQ_MSK_PORT1_IRQ_MSK_BIT                 0
#  define DELL_N32XX_SYS_QSFP_IRQ_MSK_PORT_IRQ_MSK_MSB                  1
#  define DELL_N32XX_SYS_QSFP_IRQ_MSK_PORT_IRQ_MSK_LSB                  1

#define DELL_N32XX_SYS_UPLINK_PORT_4_1_PRESENT_STATUS_REG               0x30
#  define DELL_N32XX_SYS_UPLINK_PORT_4_1_PRESENT_STATUS_PRSNT_STS_MSB   3
#  define DELL_N32XX_SYS_UPLINK_PORT_4_1_PRESENT_STATUS_PRSNT_STS_LSB   0

#define DELL_N32XX_SYS_UPLINK_PORT_4_1_TX_DISABLE_REG                   0x31
#  define DELL_N32XX_SYS_UPLINK_PORT_4_1__TX_DISABLE_MSB                3
#  define DELL_N32XX_SYS_UPLINK_PORT_4_1__TX_DISABLE_LSB                0

#define DELL_N32XX_SYS_UPLINK_PORT_4_1_RX_LOS_STATUS_REG                0x32
#  define DELL_N32XX_SYS_UPLINK_PORT_4_1_RX_LOS_STATUS_RX_LOS_MSB       3
#  define DELL_N32XX_SYS_UPLINK_PORT_4_1_RX_LOS_STATUS_RX_LOS_LSB       0

#define DELL_N32XX_SYS_UPLINK_PORT_4_1_TX_FAULT_STATUS_REG              0x33
#  define DELL_N32XX_SYS_UPLINK_PORT_4_1_TX_FAULT_STATUS_TX_FAULT_MSB   3
#  define DELL_N32XX_SYS_UPLINK_PORT_4_1_TX_FAULT_STATUS_TX_FAULT_LSB   0

#define DELL_N32XX_SYS_PHY_7_1_RST_REG                                  0x40
#  define DELL_N32XX_SYS_PHY_7_1_RST_PHY_RST_MSB                        6
#  define DELL_N32XX_SYS_PHY_7_1_RST_PHY_RST_LSB                        0

#define DELL_N32XX_SYS_PHY_7_1_IRQ_STATUS_REG                           0x41
#  define DELL_N32XX_SYS_PHY_7_1_IRQ_STATUS_PHY_IRQ_STATUS_MSB          6
#  define DELL_N32XX_SYS_PHY_7_1_IRQ_STATUS_PHY_IRQ_STATUS_LSB          0

#define DELL_N32XX_SYS_PHY_7_1_IRQ_MSK_REG                              0x42
#  define DELL_N32XX_SYS_PHY_7_1_IRQ_MSK_PHY_IRQ_MSK_MSB                6
#  define DELL_N32XX_SYS_PHY_7_1_IRQ_MSK_PHY_IRQ_MSK_LSB                0

//------------------------------------------------------------------------------
//
//                            LED CPLD Registers
//
//------------------------------------------------------------------------------

#define DELL_N32XX_LED_CPLD_REV_REG0_REG                                0x00

#define DELL_N32XX_LED_CPLD_REV_REG1_REG                                0x01

#define DELL_N32XX_LED_CPLD_GPR_REG                                     0x02

//------------------------------------------------------------------------------
//
//                            PORT CPLD Registers
//
//------------------------------------------------------------------------------

#define DELL_N32XX_PORT_CPLD_REV_REG0_REG                               0x00

#define DELL_N32XX_PORT_CPLD_REV_REG1_REG                               0x01

#define DELL_N32XX_PORT_CPLD_GPR_REG                                    0x02

#define DELL_N32XX_PORT_8_1_PRESENT_STATUS_REG                          0x10
#define DELL_N32XX_PORT_16_9_PRESENT_STATUS_REG                         0x11
#define DELL_N32XX_PORT_24_17_PRESENT_STATUS_REG                        0x12

#define DELL_N32XX_PORT_8_1_TX_DISABLE_REG                              0x14
#define DELL_N32XX_PORT_16_9_TX_DISABLE_REG                             0x15
#define DELL_N32XX_PORT_24_17_TX_DISABLE_REG                            0x16

#define DELL_N32XX_PORT_8_1_RX_LOS_STATUS_REG                           0x18
#define DELL_N32XX_PORT_16_9_RX_LOS_STATUS_REG                          0x19
#define DELL_N32XX_PORT_24_17_RX_LOS_STATUS_REG                         0x1A

#define DELL_N32XX_PORT_8_1_TX_FAULT_STATUS_REG                         0x1C
#define DELL_N32XX_PORT_16_9_TX_FAULT_STATUS_REG                        0x1D
#define DELL_N32XX_PORT_24_17_TX_FAULT_STATUS_REG                       0x1E

#endif /* DELLEMC_N32XX_N22XX_CPLDS_H__ */
