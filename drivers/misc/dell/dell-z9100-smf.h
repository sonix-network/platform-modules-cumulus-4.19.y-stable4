/*
 * Dell Z9100 SmartFusion Controller Definitions
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

#ifndef DELL_Z9100_SMF_H
#define DELL_Z9100_SMF_H

#define Z9100_SMF_IO_BASE 0x200 
#define Z9100_SMF_IO_SIZE 0x1ff


/* SMF LPC Mapped Registers */
#define Z9100_SMF_VER            0x200
#define Z9100_SMF_BOARD_TYPE     0x201
#define Z9100_SMF_SW_SCRATCH     0x202
#define Z9100_SMF_BOOT_OK        0x203
#define Z9100_SMF_UART_MUX_CTRL  0x204
#define Z9100_SMF_WD_WID         0x206
#define Z9100_SMF_WD_MASK        0x207
#define Z9100_SMF_RST_SOURCE     0x20A
#define Z9100_SMF_SEP_RST        0x20B
#define Z9100_SMF_CPU_RST_CTRL   0x20C
#define Z9100_SMF_RAM_ADDR_H     0x210
#define Z9100_SMF_RAM_ADDR_L     0x211
#define Z9100_SMF_RAM_R_DATA     0x212
#define Z9100_SMF_RAM_W_DATA     0x213
#define Z9100_SMF_TPM_STA_ID     0x221
#define Z9100_SMF_THERM_PWR_CTRL 0x230
#define Z9100_SMF_THERM_TIMESEL  0x231
#define Z9100_SMF_THERM_TIMEOUT  0x232


/* Mailbox Registers */
#define Z9100_SMF_MB_PROTO_VER                0x0
#define Z9100_SMF_MB_FIREWALL_VER             0x2
#define Z9100_SMF_MB_POWER_MAX                0x11
#define Z9100_SMF_MB_SYSTEM_STATUS            0x04d9
#define Z9100_SMF_MB_SMF_FLAG                 0x0606
#define Z9100_SMF_MB_CPU_FLAG                 0x0607
#define Z9100_SMF_MB_DEV_STATUS               0x0608
#define Z9100_SMF_MB_SCAN_LM75_1              0x02ea


/* Temp Sensor Registers */
#define Z9100_SMF_MB_TEMP1_SENSOR             0x14
#define Z9100_SMF_MB_TEMP1_HW_SHUT_LIMIT      0x3c
#define Z9100_SMF_MB_TEMP1_SW_SHUT_LIMIT      0x3e
#define Z9100_SMF_MB_TEMP1_MJR_ALARM_LIMIT    0x40
#define Z9100_SMF_MB_TEMP1_MNR_ALARM_LIMIT    0x42
#define Z9100_SMF_MB_TEMP1_FAULT              0xdc


/* Fan Sensor Registers */
#define Z9100_SMF_MB_FANS_TRAYS_CNT             0xf0
#define Z9100_SMF_MB_FANS_PER_TRAY              0xf1
#define Z9100_SMF_MB_FANS_MAX_SPEED             0xf2
#define Z9100_SMF_MB_FAN_FAN1_SPEED             0xf3
#define Z9100_SMF_MB_FAN_TRAYS_PRESENT          0x113
#define Z9100_SMF_MB_FAN_STATUS                 0x114
#define Z9100_SMF_MB_FAN_TRAYS_F2B              0x116

#define Z9100_SMF_MB_FAN1_SERIAL_NUM             0x117
#define Z9100_SMF_MB_FAN1_SERIAL_NUM_SIZE        20
#define Z9100_SMF_MB_FAN1_PART_NUM               0x12B
#define Z9100_SMF_MB_FAN1_PART_NUM_SIZE          6
#define Z9100_SMF_MB_FAN1_LABEL_REV              0x131
#define Z9100_SMF_MB_FAN1_LABEL_REV_SIZE         3
#define Z9100_SMF_MB_FAN_ALGO                    0x22f

#define Z9100_SMF_MB_FAN_OFFSET                  35

/* PSU sensor registers */
#define Z9100_SMF_MB_PSU1_MAX                    0x0234

#define Z9100_SMF_MB_PSU1_FUNC                   0x0236
#define Z9100_SMF_MB_PSU1_STATUS                 0x0237
#define Z9100_SMF_MB_PSU1_TEMP                   0x0239
#define Z9100_SMF_MB_PSU1_FAN_SPEED              0x023B
#define Z9100_SMF_MB_PSU1_FAN_STATUS             0x023D
#define Z9100_SMF_MB_PSU1_INP_VOLT               0x023E
#define Z9100_SMF_MB_PSU1_OUT_VOLT               0x0240
#define Z9100_SMF_MB_PSU1_INP_CURR               0x0232
#define Z9100_SMF_MB_PSU1_OUT_CURR               0x0244
#define Z9100_SMF_MB_PSU1_INP_POWER              0x0246
#define Z9100_SMF_MB_PSU1_OUT_POWER              0x0248

#define Z9100_SMF_MB_PSU1_COUNTY                 0x024A
#define Z9100_SMF_MB_PSU1_COUNTY_SIZE            2 
#define Z9100_SMF_MB_PSU1_PART_NUM               0x024C
#define Z9100_SMF_MB_PSU1_PART_NUM_SIZE          6 
#define Z9100_SMF_MB_PSU1_MFG_ID                 0x0252
#define Z9100_SMF_MB_PSU1_MFG_ID_SIZE            5 
#define Z9100_SMF_MB_PSU1_MFG_DATE               0x0257
#define Z9100_SMF_MB_PSU1_MFG_DATE_SIZE          8
#define Z9100_SMF_MB_PSU1_SERIAL_NUM             0x025F
#define Z9100_SMF_MB_PSU1_SERIAL_NUM_SIZE        4
#define Z9100_SMF_MB_PSU1_SERVICE_TAG            0x0263
#define Z9100_SMF_MB_PSU1_SERVICE_TAG_SIZE       8
#define Z9100_SMF_MB_PSU1_LABEL_REV              0x026A
#define Z9100_SMF_MB_PSU1_LABEL_REV_SIZE         3

#define Z9100_SMF_MB_PSU1_BASE     Z9100_SMF_MB_PSU1_MAX
#define Z9100_SMF_MB_PSU_OFFSET                  57
#define Z9100_SMF_MB_PSU_PRESENT_FLAG            0x1 
#define Z9100_SMF_MB_PSU_OK_FLAG                 0x1c 
#define Z9100_SMF_MB_PSU_FAN_PRESENT_FLAG        (1 << 2)
#define Z9100_SMF_MB_PSU_FAN_OK_FLAG             (1 << 1) 
#define Z9100_SMF_MB_PSU_FAN_F2B_FLAG            (1 << 0) 



#endif // DELL_Z9100_SMF_H

