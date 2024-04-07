/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Dell EMC S5048F SmartFusion Controller Definitions
 *
 * David Yen <dhyen@cumulusnetworks.com>
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
 */

#ifndef DELLEMC_S5048F_SMF_H
#define DELLEMC_S5048F_SMF_H

#define S5048F_SMF_NAME "s5048f_smf"
#define SMF_IO_BASE     0x200
#define SMF_IO_SIZE     0x1ff

/* SMF LPC Mapped Registers */
#define SMF_VER            0x200
#define SMF_BOARD_TYPE     0x201
#define SMF_SW_SCRATCH     0x202
#define SMF_BOOT_OK        0x203
#define SMF_MSS_STA        0x205
#define SMF_WD_WID         0x206
#define SMF_WD_MASK        0x207
#define SMF_POR_SOURCE     0x209
#define SMF_RST_SOURCE     0x20A
#define SMF_SEP_RST        0x20B
#define SMF_RAM_ADDR_H     0x210
#define SMF_RAM_ADDR_L     0x211
#define SMF_RAM_R_DATA     0x212
#define SMF_RAM_W_DATA     0x213
#define SMF_CPU_EEPROM_WP  0x220
#define SMF_TPM_STA_ID     0x221

/* Mailbox Registers */
#define SMF_PROTOCOL_VER                0x0000
#define SMF_MSS_VER                     0x0002
#define SMF_MAX_POWER                   0x0011

#define SMF_CPU_POWER_STATUS            0x02a6
#define SMF_SWITCH_POWER_STATUS         0x02a7
#define SMF_MB_SYSTEM_STATUS            0x04d9
#define SMF_SMF_FLAG                    0x0606
#define SMF_CPU_FLAG                    0x0607
#define SMF_DEVICE_STATUS               0x0608
#define SMF_MB_SCAN_LM75_BASE           0x02ea

/* Temp Sensor Registers */
#define SMF_MAX_TEMP_SENSORS            0x0013
#define SMF_TEMP_SENSOR_BASE            0x0014
#define SMF_TEMP_HW_SHUT_BASE           0x003c
#define SMF_TEMP_SW_SHUT_BASE           0x003e
#define SMF_TEMP_MAJOR_ALARM_BASE       0x0040
#define SMF_TEMP_MINOR_ALARM_BASE       0x0042
#define SMF_TEMP_STATUS_BASE            0x00dc

/* Fan Sensor Registers */
#define SMF_MAX_FAN_TRAYS               0x00f0
#define SMF_FANS_PER_TRAY               0x00f1
#define SMF_MAX_FAN_SET_SPEED           0x00f2
#define SMF_FAN_TRAY_FAN_SPEED_BASE     0x00f3
#define SMF_FAN_PRESENT                 0x0113
#define SMF_FAN_STATUS                  0x0114
#define SMF_FAN_F2B                     0x0116
#define SMF_FAN_TRAY_SERIAL_NUM_BASE    0x0117
#define SMF_FAN_TRAY_SERIAL_NUM_SIZE    20
#define SMF_FAN_TRAY_PART_NUM_BASE      0x012B
#define SMF_FAN_TRAY_PART_NUM_SIZE      6
#define SMF_FAN_TRAY_LABEL_REV_BASE     0x0131
#define SMF_FAN_TRAY_LABEL_REV_SIZE     3
#define SMF_FAN_TRAY_MFG_DATA_BASE      0x0134
#define SMF_FAN_TRAY_MFG_DATA_SAZE      6
#define SMF_FAN_CONTROL_ALGO_FLAG       0x022f
#define SMF_FAN_SPEED_CHANGE_FLAG       0x0230

#define SMF_FAN_TRAY1_ALL_OK_MASK       0x0003
#define SMF_FAN_TRAY2_ALL_OK_MASK       0x0018
#define SMF_FAN_TRAY3_ALL_OK_MASK       0x0060
#define SMF_FAN_TRAY4_ALL_OK_MASK       0x0300
#define SMF_FAN_BLOCK_SIZE              35

/* PSU sensor registers */
#define SMF_MAX_PSUS                    0x0231
#define SMF_TOTAL_POWER                 0x0232

#define SMF_PSU_MAX_POWER_BASE          0x0234
#define SMF_PSU_FUNCTION_SUPPORT_BASE   0x0236
#define SMF_PSU_STATUS_BASE             0x0237
#define SMF_PSU_TEMP_BASE               0x0239
#define SMF_PSU_FAN_SPEED_BASE          0x023B
#define SMF_PSU_FAN_STATUS_BASE         0x023D
#define SMF_PSU_INPUT_VOLTAGE_BASE      0x023E
#define SMF_PSU_OUTPUT_VOLTAGE_BASE     0x0240
#define SMF_PSU_INPUT_CURRENT_BASE      0x0232
#define SMF_PSU_OUTPUT_CURRENT_BASE     0x0244
#define SMF_PSU_INPUT_POWER_BASE        0x0246
#define SMF_PSU_OUTPUT_POWER_BASE       0x0248

#define SMF_PSU_COUNTRY_CODE_BASE       0x024A
#define SMF_PSU_COUNTRY_CODE_SIZE       2
#define SMF_PSU_PART_NUM_BASE           0x024C
#define SMF_PSU_PART_NUM_SIZE           6
#define SMF_PSU_MFG_ID_BASE             0x0252
#define SMF_PSU_MFG_ID_SIZE             5
#define SMF_PSU_MFG_DATE_BASE           0x0257
#define SMF_PSU_MFG_DATE_SIZE           8
#define SMF_PSU_SERIAL_NUM_BASE         0x025F
#define SMF_PSU_SERIAL_NUM_SIZE         4
#define SMF_PSU_SERVICE_TAG_BASE        0x0263
#define SMF_PSU_SERVICE_TAG_SIZE        7
#define SMF_PSU_LABEL_REV_BASE          0x026A
#define SMF_PSU_LABEL_REV_SIZE          3

#define SMF_PSU_BLOCK_SIZE              57
#define SMF_PSU_PRESENT_N_MASK          BIT(0)
#define SMF_PSU_DC_MASK                 BIT(1)
#define SMF_PSU_ALL_OK_N_MASK           0x1d

#define SMF_PSU_FAN_PRESENT_N_MASK      BIT(2)
#define SMF_PSU_FAN_ALL_OK_N_MASK       BIT(1)
#define SMF_PSU_FAN_F2B_MASK            BIT(0)

#endif /* DELLEMC_S5048F_SMF_H */

