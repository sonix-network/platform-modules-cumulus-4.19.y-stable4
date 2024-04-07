/* SPDX-License-Identifier: GPL-2.0+ */
/*******************************************************************************
 *
 * @file    dellemc-z9xxx-s52xx-fpga.h
 * @brief   DellEMC FPGA definitions for Z9XXX/S52XX platforms
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

#ifndef DELLEMC_Z9XXX_S52XX_FPGA_H__
#define DELLEMC_Z9XXX_S52XX_FPGA_H__

/*******************************************************************************
 *
 *                     DellEMC FPGA Register Definitions
 *
 *  These register definitions are taken from the Z9XXX/S52XX Programmable Logic
 *  Design Specification, Revision: 14, Sept 17, 2018, Editor Ajith K Jacob.
 *
 ******************************************************************************/

//------------------------------------------------------------------------------
//
//                        CTRL - Control Registers
//                             0x0000 - 0x03FF
//
//------------------------------------------------------------------------------

#define DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_REG                              0x0000
#  define DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_VNDR_MSB                       31
#  define DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_VNDR_LSB                       16
#  define DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_MJR_REV_MSB                    15
#  define DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_MJR_REV_LSB                    8
#  define DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_MNR_REV_MSB                    7
#  define DELL_Z9S52_CTRL_FPGA_VNDR_VRSN_MNR_REV_LSB                    0

#define DELL_Z9S52_CTRL_FPGA_GPR_REG                                    0x0004

#define DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_REG                             0x0008
#  define DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_COME_TYPE_MSB                 10
#  define DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_COME_TYPE_LSB                 8
#  define DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_BRD_REV_MSB                   7
#  define DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_BRD_REV_LSB                   4
#  define DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_BRD_TYPE_MSB                  3
#  define DELL_Z9S52_CTRL_MB_BRD_REV_TYPE_BRD_TYPE_LSB                  0

#define DELL_Z9S52_CTRL_BUILD_TIMESTAMP1_REG                            0x000C

#define DELL_Z9S52_CTRL_BUILD_TIMESTAMP2_REG                            0x0010

#define DELL_Z9S52_CTRL_PLL_STS_REG                                     0x0014
#  define DELL_Z9S52_CTRL_PLL_STS_BS_CLK_PLL_LCK_BIT                    5
#  define DELL_Z9S52_CTRL_PLL_STS_OOB_CLK_PLL_LCK_BIT                   4
#  define DELL_Z9S52_CTRL_PLL_STS_JA_LCK_BIT                            3
#  define DELL_Z9S52_CTRL_PLL_STS_DPLL_LCK_BIT                          2
#  define DELL_Z9S52_CTRL_PLL_STS_25MHZ_PLL_LCK_BIT                     1
#  define DELL_Z9S52_CTRL_PLL_STS_PCIE_PLL_LCK_BIT                      0

#define DELL_Z9S52_CTRL_BMC_GPR_REG                                     0x0020

#define DELL_Z9S52_CTRL_SYSTEM_LED_REG                                  0x0024
#  define DELL_Z9S52_CTRL_SYSTEM_LED_CPU_CTRL_BIT                       8
#  define DELL_Z9S52_CTRL_SYSTEM_LED_FAN_LED_MSB                        7
#  define DELL_Z9S52_CTRL_SYSTEM_LED_FAN_LED_LSB                        6
#  define DELL_Z9S52_CTRL_SYSTEM_LED_SYSTEM_MSB                         5
#  define DELL_Z9S52_CTRL_SYSTEM_LED_SYSTEM_LSB                         4
#  define DELL_Z9S52_CTRL_SYSTEM_LED_BEACON_BIT                         3
#  define DELL_Z9S52_CTRL_SYSTEM_LED_POWER_MSB                          2
#  define DELL_Z9S52_CTRL_SYSTEM_LED_POWER_LSB                          1
#  define DELL_Z9S52_CTRL_SYSTEM_LED_STACK_LED_BIT                      0

#define DELL_Z9S52_CTRL_SVN_SEG_STK_LED_REG                             0x0028
#  define DELL_Z9S52_CTRL_SVN_SEG_STK_LED_LED_BLNK_BIT                  6
#  define DELL_Z9S52_CTRL_SVN_SEG_STK_LED_LED_OFF_BIT                   5
#  define DELL_Z9S52_CTRL_SVN_SEG_STK_LED_DOT_BIT                       4
#  define DELL_Z9S52_CTRL_SVN_SEG_STK_LED_DGT_MSB                       3
#  define DELL_Z9S52_CTRL_SVN_SEG_STK_LED_DGT_LSB                       0

#define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_REG                            0x0018
#  define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_COLD_RESET_BIT               10
#  define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_WARM_RESET_BIT               9
#  define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_RST_BTN_COLD_REBOOT_BIT      8
#  define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_RST_BTN_SHUT_DOWN_BIT        7
#  define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_HOTSWP_SHUT_DOWN_BIT         6
#  define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_BMC_SHUTDOWN_BIT             5
#  define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_WD_FAIL_BIT                  4
#  define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_CPU_THRMTRIP_BIT             3
#  define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_PSU_SHUT_DOWN_BIT            2
#  define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_SHUT_DOWN_BIT                1
#  define DELL_Z9S52_CTRL_SYS_REBOOT_CAUSE_PWR_ERR_BIT                  0

#define DELL_Z9S52_CTRL_CPU_STATE_REG                                   0x001C
#  define DELL_Z9S52_CTRL_CPU_STATE_SYS_PWR_DN_BIT                      9
#  define DELL_Z9S52_CTRL_CPU_STATE_MB_PWR_UP_BIT                       8
#  define DELL_Z9S52_CTRL_CPU_STATE_WARM_BIT                            5
#  define DELL_Z9S52_CTRL_CPU_STATE_S0_BIT                              4
#  define DELL_Z9S52_CTRL_CPU_STATE_S3_BIT                              3
#  define DELL_Z9S52_CTRL_CPU_STATE_S4_BIT                              2
#  define DELL_Z9S52_CTRL_CPU_STATE_S5_BIT                              1
#  define DELL_Z9S52_CTRL_CPU_STATE_G3_BIT                              0

#define DELL_Z9S52_CTRL_BMC_PWR_CTRL_REG                                0x002C
#  define DELL_Z9S52_CTRL_BMC_PWR_CTRL_PWR_DWN_BIT                      0

#define DELL_Z9S52_CTRL_BMC_EEPRM_CTRL_REG                              0x0030
#  define DELL_Z9S52_CTRL_BMC_EEPRM_CTRL_EEPRM_SEL_BIT                  2

#define DELL_Z9S52_CTRL_MSTR_SRR_REG                                    0x0040
#  define DELL_Z9S52_CTRL_MSTR_SRR_I2C_RST_16_8_MSB                     30
#  define DELL_Z9S52_CTRL_MSTR_SRR_I2C_RST_16_8_LSB                     22
#  define DELL_Z9S52_CTRL_MSTR_SRR_MB_RST_EN_BIT                        21
#  define DELL_Z9S52_CTRL_MSTR_SRR_I2C_RST_7_1_MSB                      20
#  define DELL_Z9S52_CTRL_MSTR_SRR_I2C_RST_7_1_LSB                      14
#  define DELL_Z9S52_CTRL_MSTR_SRR_SLV_CPLD_RST_MSB                     13
#  define DELL_Z9S52_CTRL_MSTR_SRR_SLV_CPLD_RST_LSB                     10
#  define DELL_Z9S52_CTRL_MSTR_SRR_BMC_URT1_FIFO_RST_BIT                9
#  define DELL_Z9S52_CTRL_MSTR_SRR_CPU_URT0_FIFO_RST_BIT                8
#  define DELL_Z9S52_CTRL_MSTR_SRR_USBA_RST_BIT                         7
#  define DELL_Z9S52_CTRL_MSTR_SRR_I210_RST_BIT                         6
#  define DELL_Z9S52_CTRL_MSTR_SRR_MICRO_USB_RST_BIT                    4
#  define DELL_Z9S52_CTRL_MSTR_SRR_JA_RST_BIT                           3
#  define DELL_Z9S52_CTRL_MSTR_SRR_DPLL_RST_BIT                         2
#  define DELL_Z9S52_CTRL_MSTR_SRR_NPU_RST_BIT                          1
#  define DELL_Z9S52_CTRL_MSTR_SRR_BMC_RST_BIT                          0

#define DELL_Z9S52_CTRL_MSTR_CTRL_REG                                   0x0044
#  define DELL_Z9S52_CTRL_MSTR_CTRL_PWR_CYCLE_SYSTEM_BIT                0

#define DELL_Z9S52_CTRL_MSTR_IRQ_REG                                    0x0048
#  define DELL_Z9S52_CTRL_MSTR_IRQ_NPU_INT_STS_BIT                      4
#  define DELL_Z9S52_CTRL_MSTR_IRQ_I210_IRQ_STS_BIT                     2
#  define DELL_Z9S52_CTRL_MSTR_IRQ_IR3595_IRQ_STS_BIT                   1
#  define DELL_Z9S52_CTRL_MSTR_IRQ_TMG_M_IRQ_STS_BIT                    0

#define DELL_Z9S52_CTRL_MSTR_IRQ_EN_REG                                 0x004C
#  define DELL_Z9S52_CTRL_MSTR_IRQ_EN_NPU_INT_EN_BIT                    4
#  define DELL_Z9S52_CTRL_MSTR_IRQ_EN_I210_IRQ_EN_BIT                   2
#  define DELL_Z9S52_CTRL_MSTR_IRQ_EN_IR3595_IRQ_EN_BIT                 1
#  define DELL_Z9S52_CTRL_MSTR_IRQ_EN_TMG_M_IRQ_EN_BIT                  0

#define DELL_Z9S52_CTRL_MISC_CTRL_REG                                   0x0050
#  define DELL_Z9S52_CTRL_MISC_CTRL_EEPRM_SEL_BIT                       14
#  define DELL_Z9S52_CTRL_MISC_CTRL_RVSD_BIT                            12
#  define DELL_Z9S52_CTRL_MISC_CTRL_LED_TEST_MSB                        9
#  define DELL_Z9S52_CTRL_MISC_CTRL_LED_TEST_LSB                        8
#  define DELL_Z9S52_CTRL_MISC_CTRL_CNSL_FIFO_MOD_MSB                   7
#  define DELL_Z9S52_CTRL_MISC_CTRL_CNSL_FIFO_MOD_LSB                   6
#  define DELL_Z9S52_CTRL_MISC_CTRL_CNSL_BMC_FIFO_EN_BIT                5
#  define DELL_Z9S52_CTRL_MISC_CTRL_CNSL_CPU_FIFO_EN_BIT                4
#  define DELL_Z9S52_CTRL_MISC_CTRL_CLK_SEL1_BIT                        3
#  define DELL_Z9S52_CTRL_MISC_CTRL_CLK_SEL0_BIT                        2
#  define DELL_Z9S52_CTRL_MISC_CTRL_USBA_PWREN_BIT                      1
#  define DELL_Z9S52_CTRL_MISC_CTRL_SYS_EEPROM_WP_BIT                   0

#define DELL_Z9S52_CTRL_MISC_STS_REG                                    0x0054
#  define DELL_Z9S52_CTRL_MISC_STS_COM_E_GPI3_BIT                       8
#  define DELL_Z9S52_CTRL_MISC_STS_COM_E_GPI2_BIT                       7
#  define DELL_Z9S52_CTRL_MISC_STS_DBG_HDR_BIT                          6
#  define DELL_Z9S52_CTRL_MISC_STS_TMP75_STS_BIT                        5
#  define DELL_Z9S52_CTRL_MISC_STS_USBA_HS_BIT                          4
#  define DELL_Z9S52_CTRL_MISC_STS_USBA_FAULT_BIT                       3
#  define DELL_Z9S52_CTRL_MISC_STS_TMG_M_PRSNT_BIT                      2
#  define DELL_Z9S52_CTRL_MISC_STS_MICRO_USB_PRESENT_BIT                1
#  define DELL_Z9S52_CTRL_MISC_STS_USBA_PRSNT_BIT                       0

#define DELL_Z9S52_CTRL_PWR_EN_REG                                      0x0060
#  define DELL_Z9S52_CTRL_PWR_EN_HOT_SWAP2_EN_BIT                       9
#  define DELL_Z9S52_CTRL_PWR_EN_HOT_SWAP1_EN_BIT                       8
#  define DELL_Z9S52_CTRL_PWR_EN_PSU2_ON_BIT                            1
#  define DELL_Z9S52_CTRL_PWR_EN_PSU1_ON_BIT                            0

#define DELL_Z9S52_CTRL_PWR_STS_REG                                     0x0038
#  define DELL_Z9S52_CTRL_PWR_STS_FTR_PG_BIT                            15
#  define DELL_Z9S52_CTRL_PWR_STS_UCD_LAT_PG_BIT                        13
#  define DELL_Z9S52_CTRL_PWR_STS_UCD_ERLY_PG_BIT                       12
#  define DELL_Z9S52_CTRL_PWR_STS_HOT_SWAP_PG2_BIT                      9
#  define DELL_Z9S52_CTRL_PWR_STS_HOT_SWAP_PG1_BIT                      8
#  define DELL_Z9S52_CTRL_PWR_STS_PSU2_PG_BIT                           3
#  define DELL_Z9S52_CTRL_PWR_STS_PSU2_PRSNT_BIT                        2
#  define DELL_Z9S52_CTRL_PWR_STS_PSU1_PG_BIT                           1
#  define DELL_Z9S52_CTRL_PWR_STS_PSU1_PRSNT_BIT                        0

#define DELL_Z9S52_CTRL_NPU_AVS_REG                                     0x0068
#  define DELL_Z9S52_CTRL_NPU_AVS_AVS_MSB                               7
#  define DELL_Z9S52_CTRL_NPU_AVS_AVS_LSB                               0

#define DELL_Z9S52_CTRL_ACCURACY_REG                                    0x0070
#  define DELL_Z9S52_CTRL_ACCURACY_ACCRCY_MSB                           7
#  define DELL_Z9S52_CTRL_ACCURACY_ACCRCY_LSB                           0

#define DELL_Z9S52_CTRL_NANOSEC_REG                                     0x0074
#  define DELL_Z9S52_CTRL_NANOSEC_NS_MSB                                29
#  define DELL_Z9S52_CTRL_NANOSEC_NS_LSB                                0

#define DELL_Z9S52_CTRL_SEC_REG                                         0x0078

#define DELL_Z9S52_CTRL_LOCK_EPOCH_REG                                  0x007C
#  define DELL_Z9S52_CTRL_LOCK_EPOCH_LOCK_BIT                           16
#  define DELL_Z9S52_CTRL_LOCK_EPOCH_EPOCH_MSB                          15
#  define DELL_Z9S52_CTRL_LOCK_EPOCH_EPOCH_LSB                          0

#define DELL_Z9S52_CTRL_CNSL_CPU_STS_REG                                0x0080
#  define DELL_Z9S52_CTRL_CNSL_CPU_STS_CPU_DROP_COUNT_MSB               30
#  define DELL_Z9S52_CTRL_CNSL_CPU_STS_CPU_DROP_COUNT_LSB               16
#  define DELL_Z9S52_CTRL_CNSL_CPU_STS_CPU_COUNT_MSB                    14
#  define DELL_Z9S52_CTRL_CNSL_CPU_STS_CPU_COUNT_LSB                    0

#define DELL_Z9S52_CTRL_CNSL_CPU_DT_REG                                 0x0084
#  define DELL_Z9S52_CTRL_CNSL_CPU_DT_CPU_DT_MSB                        7
#  define DELL_Z9S52_CTRL_CNSL_CPU_DT_CPU_DT_LSB                        0

#define DELL_Z9S52_CTRL_CNSL_BMC_STS_REG                                0x0088
#  define DELL_Z9S52_CTRL_CNSL_BMC_STS_BMC_DROP_COUNT_MSB               30
#  define DELL_Z9S52_CTRL_CNSL_BMC_STS_BMC_DROP_COUNT_LSB               16
#  define DELL_Z9S52_CTRL_CNSL_BMC_STS_BMC_COUNT_MSB                    14
#  define DELL_Z9S52_CTRL_CNSL_BMC_STS_BMC_COUNT_LSB                    0

#define DELL_Z9S52_CTRL_CNSL_BMC_DT_REG                                 0x008C
#  define DELL_Z9S52_CTRL_CNSL_BMC_DT_BMC_DT_MSB                        7
#  define DELL_Z9S52_CTRL_CNSL_BMC_DT_BMC_DT_LSB                        0

#define DELL_Z9S52_CTRL_CRC_ERR_CTR_REG                                 0x0090

#define DELL_Z9S52_CTRL_JTAG_REG                                        0x0094
#  define DELL_Z9S52_CTRL_JTAG_JTAG_EN_BIT                              5
#  define DELL_Z9S52_CTRL_JTAG_JTAG_TDO_BIT                             4
#  define DELL_Z9S52_CTRL_JTAG_JTAG_TRST_BIT                            3
#  define DELL_Z9S52_CTRL_JTAG_JTAG_TDI_BIT                             2
#  define DELL_Z9S52_CTRL_JTAG_JTAG_TMS_BIT                             1
#  define DELL_Z9S52_CTRL_JTAG_JTAG_TCK_BIT                             0

#define DELL_Z9S52_CTRL_FEATUR_GPIO_REG                                 0x0098
#  define DELL_Z9S52_CTRL_FEATUR_GPIO_FEATUE_GPIO_15_11_MSB             15
#  define DELL_Z9S52_CTRL_FEATUR_GPIO_FEATUE_GPIO_15_11_LSB             11
#  define DELL_Z9S52_CTRL_FEATUR_GPIO_FEATUE_GPIO_10_BIT                10
#  define DELL_Z9S52_CTRL_FEATUR_GPIO_FEATUE_GPIO_9_BIT                 9
#  define DELL_Z9S52_CTRL_FEATUR_GPIO_FEATUE_GPIO_8_BIT                 8
#  define DELL_Z9S52_CTRL_FEATUR_GPIO_FEATUE_GPIO_7_1_MSB               7
#  define DELL_Z9S52_CTRL_FEATUR_GPIO_FEATUE_GPIO_7_1_LSB               1
#  define DELL_Z9S52_CTRL_FEATUR_GPIO_FEATUE_GPIO_0_BIT                 0

#define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_REG                             0x0058
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH6_MSI_VCTR_MSB          29
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH6_MSI_VCTR_LSB          25
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH5_MSI_VCTR_MSB          24
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH5_MSI_VCTR_LSB          20
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH4_MSI_VCTR_MSB          19
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH4_MSI_VCTR_LSB          15
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH3_MSI_VCTR_MSB          14
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH3_MSI_VCTR_LSB          10
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH2_MSI_VCTR_MSB          9
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH2_MSI_VCTR_LSB          5
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH1_MSI_VCTR_MSB          4
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP1_I2C_CH1_MSI_VCTR_LSB          0

#define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_REG                             0x005C
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_PORT_33_64_MSI_VCTR_MSB       29
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_PORT_33_64_MSI_VCTR_LSB       25
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_PORT_1_32_MSI_VCTR_MSB        24
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_PORT_1_32_MSI_VCTR_LSB        20
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_SPI_EXT_MSI_VCTR_MSB          19
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_SPI_EXT_MSI_VCTR_LSB          15
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_SPI_INT_MSI_VCTR_MSB          14
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_SPI_INT_MSI_VCTR_LSB          10
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_I2C_EXTRNL_MSI_VCTR_MSB       9
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_I2C_EXTRNL_MSI_VCTR_LSB       5
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_I2C_CH7_MSI_VCTR_MSB          4
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP2_I2C_CH7_MSI_VCTR_LSB          0

#define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_REG                             0x009C
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_I2C_CH9_MSI_VCTR_MSB          29
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_I2C_CH9_MSI_VCTR_LSB          25
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_I2C_CH8_MSI_VCTR_MSB          24
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_I2C_CH8_MSI_VCTR_LSB          20
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_NPU_MSI_VCTR_MSB              19
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_NPU_MSI_VCTR_LSB              15
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_DMA_MSI_VCTR_MSB              14
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_DMA_MSI_VCTR_LSB              10
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_PORT_97_104_MSI_VCTR_MSB      9
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_PORT_97_104_MSI_VCTR_LSB      5
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_PORT_65_96_MSI_VCTR_MSB       4
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP3_PORT_65_96_MSI_VCTR_LSB       0

#define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_REG                             0x00A0
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH15_MSI_VCTR_MSB         29
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH15_MSI_VCTR_LSB         25
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH14_MSI_VCTR_MSB         24
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH14_MSI_VCTR_LSB         20
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH13_MSI_VCTR_MSB         19
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH13_MSI_VCTR_LSB         15
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH12_MSI_VCTR_MSB         14
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH12_MSI_VCTR_LSB         10
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH11_MSI_VCTR_MSB         9
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH11_MSI_VCTR_LSB         5
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH10_MSI_VCTR_MSB         4
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP4_I2C_CH10_MSI_VCTR_LSB         0

#define DELL_Z9S52_CTRL_MSI_VECTOR_MAP5_REG                             0x00A4
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP5_I2C_CH16_MSI_VCTR_MSB         4
#  define DELL_Z9S52_CTRL_MSI_VECTOR_MAP5_I2C_CH16_MSI_VCTR_LSB         0

#define DELL_Z9S52_CTRL_EEPRM_INFO_REG                                  0x0100

//------------------------------------------------------------------------------
//
//                         PCIe GTP_DRP Registers
//                             0x0800 - 0x0FFF
//
//------------------------------------------------------------------------------

#define DELL_Z9S52_PCIE_GTP_DRP_TXMARGIN_FULL_REG                       0x09D4
#  define DELL_Z9S52_PCIE_GTP_DRP_TXMARGIN_FULL_TX_MARGIN_FULL_1_MSB    14
#  define DELL_Z9S52_PCIE_GTP_DRP_TXMARGIN_FULL_TX_MARGIN_FULL_1_LSB    8
#  define DELL_Z9S52_PCIE_GTP_DRP_TXMARGIN_FULL_TX_MARGIN_FULL_0_MSB    7
#  define DELL_Z9S52_PCIE_GTP_DRP_TXMARGIN_FULL_TX_MARGIN_FULL_0_LSB    0

#define DELL_Z9S52_PCIE_GTP_DRP_TXDEEMPH_REG                            0x09E8
#  define DELL_Z9S52_PCIE_GTP_DRP_TXDEEMPH_TX_DEEMPH1_MSB               14
#  define DELL_Z9S52_PCIE_GTP_DRP_TXDEEMPH_TX_DEEMPH1_LSB               8
#  define DELL_Z9S52_PCIE_GTP_DRP_TXDEEMPH_TX_DEEMPH0_MSB               7
#  define DELL_Z9S52_PCIE_GTP_DRP_TXDEEMPH_TX_DEEMPH0_LSB               0

//------------------------------------------------------------------------------
//
//                            CFG SPI Registers
//                             0x1000 - 0x1FFF
//
//------------------------------------------------------------------------------

#define DELL_Z9S52_CFG_SPI_SRR_REG                                      0x1040
#define DELL_Z9S52_CFG_SPI_CTRL_REG                                     0x1060
#define DELL_Z9S52_CFG_SPI_STATUS_REG                                   0x1064
#define DELL_Z9S52_CFG_SPI_DATA_TRANSMIT_REG                            0x1068
#define DELL_Z9S52_CFG_SPI_DATA_RECEIVE_REG                             0x106C
#define DELL_Z9S52_CFG_SPI_SLAVE_SELECT_REG                             0x1070
#define DELL_Z9S52_CFG_SPI_TX_FIFO_OCCUPANCY_REG                        0x1074
#define DELL_Z9S52_CFG_SPI_RX_FIFO_OCCUPANCY_REG                        0x1078
#define DELL_Z9S52_CFG_SPI_DGIER_REG                                    0x101C
#define DELL_Z9S52_CFG_SPI_IP_ISR_REG                                   0x1020
#define DELL_Z9S52_CFG_SPI_IP_IER_REG                                   0x1028

//------------------------------------------------------------------------------
//
//                             DMA Registers
//                            0x2000 - 0x2FFF
//
//------------------------------------------------------------------------------

#define DELL_Z9S52_DMA_DMA_CTRL_REG                                     0x2000
#  define DELL_Z9S52_DMA_DMA_CTRL_BD_CNT_MSB                            15
#  define DELL_Z9S52_DMA_DMA_CTRL_BD_CNT_LSB                            8
#  define DELL_Z9S52_DMA_DMA_CTRL_BD_CHK_DSBLE_BIT                      6
#  define DELL_Z9S52_DMA_DMA_CTRL_DMA_TEST_BIT                          4
#  define DELL_Z9S52_DMA_DMA_CTRL_DMA_DBG_CLR_BIT                       3
#  define DELL_Z9S52_DMA_DMA_CTRL_DMA_IRQ_EN_BIT                        2
#  define DELL_Z9S52_DMA_DMA_CTRL_DMA_RESET_BIT                         1
#  define DELL_Z9S52_DMA_DMA_CTRL_DMA_FIFO_EN_BIT                       0

#define DELL_Z9S52_DMA_BFR_SZ_REG                                       0x2004
#  define DELL_Z9S52_DMA_BFR_SZ_BUF_SZ_MSB                              24
#  define DELL_Z9S52_DMA_BFR_SZ_BUF_SZ_LSB                              0

#define DELL_Z9S52_DMA_STRT_PNTR_REG                                    0x2008
#  define DELL_Z9S52_DMA_STRT_PNTR_STRT_PNTR_MSB                        10
#  define DELL_Z9S52_DMA_STRT_PNTR_STRT_PNTR_LSB                        0

#define DELL_Z9S52_DMA_TAIL_PNTR_REG                                    0x200C
#  define DELL_Z9S52_DMA_TAIL_PNTR_TAIL_PNTR_MSB                        10
#  define DELL_Z9S52_DMA_TAIL_PNTR_TAIL_PNTR_LSB                        0

#define DELL_Z9S52_DMA_DMA_STS_REG                                      0x2010
#  define DELL_Z9S52_DMA_DMA_STS_DMA_IDLE_BIT                           8
#  define DELL_Z9S52_DMA_DMA_STS_DMA_TM_ERR_BIT                         7
#  define DELL_Z9S52_DMA_DMA_STS_DST_ADD_ERR_BIT                        6
#  define DELL_Z9S52_DMA_DMA_STS_DST_HAD_ERR_BIT                        5
#  define DELL_Z9S52_DMA_DMA_STS_DST_ADL_ERR_BIT                        4
#  define DELL_Z9S52_DMA_DMA_STS_BFR_ERR_BIT                            3
#  define DELL_Z9S52_DMA_DMA_STS_DECODE_ERR_BIT                         2
#  define DELL_Z9S52_DMA_DMA_STS_XFR_ERR_BIT                            1
#  define DELL_Z9S52_DMA_DMA_STS_XFR_DN_BIT                             0

#define DELL_Z9S52_DMA_DST_BAR_REG                                      0x2014

#define DELL_Z9S52_DMA_DST_HAD_REG                                      0x2018

#define DELL_Z9S52_DMA_DBG_REG1_REG                                     0x201C
#  define DELL_Z9S52_DMA_DBG_REG1_NXT_BD_MSB                            31
#  define DELL_Z9S52_DMA_DBG_REG1_NXT_BD_LSB                            16
#  define DELL_Z9S52_DMA_DBG_REG1_CRNT_BD_MSB                           15
#  define DELL_Z9S52_DMA_DBG_REG1_CRNT_BD_LSB                           0

#define DELL_Z9S52_DMA_DBG_REG2_REG                                     0x2020

#define DELL_Z9S52_DMA_DBG_REG3_REG                                     0x2024
#  define DELL_Z9S52_DMA_DBG_REG3_CRNT_BD_ADD_MSB                       27
#  define DELL_Z9S52_DMA_DBG_REG3_CRNT_BD_ADD_LSB                       16
#  define DELL_Z9S52_DMA_DBG_REG3_BD_SM_MSB                             3
#  define DELL_Z9S52_DMA_DBG_REG3_BD_SM_LSB                             0

#define DELL_Z9S52_DMA_DBG_REG4_REG                                     0x2028
#  define DELL_Z9S52_DMA_DBG_REG4_XFR_CNT_REM_MSB                       30
#  define DELL_Z9S52_DMA_DBG_REG4_XFR_CNT_REM_LSB                       12
#  define DELL_Z9S52_DMA_DBG_REG4_DMA_SM_MSB                            11
#  define DELL_Z9S52_DMA_DBG_REG4_DMA_SM_LSB                            4
#  define DELL_Z9S52_DMA_DBG_REG4_AXI_SM_MSB                            3
#  define DELL_Z9S52_DMA_DBG_REG4_AXI_SM_LSB                            0

#define DELL_Z9S52_DMA_DBG_REG5_REG                                     0x202C
#  define DELL_Z9S52_DMA_DBG_REG5_DMA_FIFO_CNT_MSB                      10
#  define DELL_Z9S52_DMA_DBG_REG5_DMA_FIFO_CNT_LSB                      0

#define DELL_Z9S52_DMA_DBG_REG6_REG                                     0x2030

#define DELL_Z9S52_DMA_DBG_REG7_REG                                     0x2034

#define DELL_Z9S52_DMA_DBG_REG8_REG                                     0x2038

#define DELL_Z9S52_DMA_DBG_REG9_REG                                     0x203C

#define DELL_Z9S52_DMA_DBG_REG10_REG                                    0x2040

#define DELL_Z9S52_DMA_DBG_REG11_REG                                    0x2044

#define DELL_Z9S52_DMA_NXT_PNTR_REG                                     0x2800
#  define DELL_Z9S52_DMA_NXT_PNTR_NXT_BD_ADD_MSB                        10
#  define DELL_Z9S52_DMA_NXT_PNTR_NXT_BD_ADD_LSB                        0

#define DELL_Z9S52_DMA_DST_ADL_REG                                      0x2804

#define DELL_Z9S52_DMA_XSFR_STS_REG                                     0x280C
#  define DELL_Z9S52_DMA_XSFR_STS_DMA_IDLE_BIT                          8
#  define DELL_Z9S52_DMA_XSFR_STS_BD_DMA_TM_ERR_BIT                     7
#  define DELL_Z9S52_DMA_XSFR_STS_BD_DST_ADD_ERR_BIT                    6
#  define DELL_Z9S52_DMA_XSFR_STS_BD_DST_HAD_ERR_BIT                    5
#  define DELL_Z9S52_DMA_XSFR_STS_BD_DST_ADL_ERR_BIT                    4
#  define DELL_Z9S52_DMA_XSFR_STS_BD_BFR_ERR_BIT                        3
#  define DELL_Z9S52_DMA_XSFR_STS_BD_DECODE_ERR_BIT                     2
#  define DELL_Z9S52_DMA_XSFR_STS_BD_XFR_ERR_BIT                        1
#  define DELL_Z9S52_DMA_XSFR_STS_BD_XFR_DN_BIT                         0

//------------------------------------------------------------------------------
//
//                           PCIE CFG Registers
//                            0x3000 - 0x3FFF
//
//------------------------------------------------------------------------------

#define DELL_Z9S52_PCIE_CFG_AXIBARTOPCIBAR0_U_REG                       0x3208

#define DELL_Z9S52_PCIE_CFG_AXIBARTOPCIBAR0_L_REG                       0x320C

//------------------------------------------------------------------------------
//
//                           PORT XCVR Registers
//                             0x4000 - 0x5FFF
//
//------------------------------------------------------------------------------

#define DELL_Z9S52_PORT_XCVR_PORT_CTRL_REG                              0x4000
#  define DELL_Z9S52_PORT_XCVR_PORT_CTRL_LPMOD_BIT                      6
#  define DELL_Z9S52_PORT_XCVR_PORT_CTRL_MODSEL_BIT                     5
#  define DELL_Z9S52_PORT_XCVR_PORT_CTRL_RST_BIT                        4
#  define DELL_Z9S52_PORT_XCVR_PORT_CTRL_TX_DIS_BIT                     0

#define DELL_Z9S52_PORT_XCVR_PORT_STS_REG                               0x4004
#  define DELL_Z9S52_PORT_XCVR_PORT_STS_IRQ_BIT                         5
#  define DELL_Z9S52_PORT_XCVR_PORT_STS_PRSNT_BIT                       4
#  define DELL_Z9S52_PORT_XCVR_PORT_STS_TXFAULT_BIT                     2
#  define DELL_Z9S52_PORT_XCVR_PORT_STS_RXLOS_BIT                       1
#  define DELL_Z9S52_PORT_XCVR_PORT_STS_MODABS_BIT                      0

#define DELL_Z9S52_PORT_XCVR_PORT_IRQ_STS_REG                           0x4008
#  define DELL_Z9S52_PORT_XCVR_PORT_IRQ_STS_IRQ_LTCH_STS_BIT            5
#  define DELL_Z9S52_PORT_XCVR_PORT_IRQ_STS_PRSNT_LTCH_STS_BIT          4
#  define DELL_Z9S52_PORT_XCVR_PORT_IRQ_STS_TXFAULT_LTCH_STS_BIT        2
#  define DELL_Z9S52_PORT_XCVR_PORT_IRQ_STS_RXLOS_LTCH_STS_BIT          1
#  define DELL_Z9S52_PORT_XCVR_PORT_IRQ_STS_MODABS_LTCH_STS_BIT         0

#define DELL_Z9S52_PORT_XCVR_PORT_IRQ_EN_REG                            0x400C
#  define DELL_Z9S52_PORT_XCVR_PORT_IRQ_EN_IRQ_EN_BIT                   5
#  define DELL_Z9S52_PORT_XCVR_PORT_IRQ_EN_PRSNT_IRQ_EN_BIT             4
#  define DELL_Z9S52_PORT_XCVR_PORT_IRQ_EN_TXFAULT_IRQ_EN_BIT           2
#  define DELL_Z9S52_PORT_XCVR_PORT_IRQ_EN_RXLOS_IRQ_EN_BIT             1
#  define DELL_Z9S52_PORT_XCVR_PORT_IRQ_EN_MODABS_IRQ_EN_BIT            0

//------------------------------------------------------------------------------
//
//                              I2C Registers
//                             0x6000 - 0x60FF
//
//------------------------------------------------------------------------------

#define DELL_Z9S52_I2C_PREPLO_REG                                       0x6000
#define DELL_Z9S52_I2C_PREPHO_REG                                       0x6001
#define DELL_Z9S52_I2C_CTR_REG                                          0x6002
#define DELL_Z9S52_I2C_TXR_REG                                          0x6003
#define DELL_Z9S52_I2C_RXR_REG                                          0x6003
#define DELL_Z9S52_I2C_CR_REG                                           0x6004
#define DELL_Z9S52_I2C_SR_REG                                           0x6004

//------------------------------------------------------------------------------
//
//                             SPI Registers
//                            0x7000 - 0x7FFF
//
//------------------------------------------------------------------------------

#define DELL_Z9S52_SPI_SRR_REG                                          0x7040
#define DELL_Z9S52_SPI_CTRL_REG                                         0x7060
#define DELL_Z9S52_SPI_STATUS_REG                                       0x7064
#define DELL_Z9S52_SPI_DATA_TRANSMIT_REG                                0x7068
#define DELL_Z9S52_SPI_DATA_RECEIVE_REG                                 0x706C
#define DELL_Z9S52_SPI_SLAVE_SELECT_REG                                 0x7070
#define DELL_Z9S52_SPI_TX_FIFO_OCCUPANCY_REG                            0x7074
#define DELL_Z9S52_SPI_RX_FIFO_OCCUPANCY_REG                            0x7078
#define DELL_Z9S52_SPI_DGIER_REG                                        0x701C
#define DELL_Z9S52_SPI_IP_ISR_REG                                       0x7020
#define DELL_Z9S52_SPI_IP_IER_REG                                       0x7028

#endif /* DELLEMC_Z9XXX_S52XX_FPGA_H__ */
