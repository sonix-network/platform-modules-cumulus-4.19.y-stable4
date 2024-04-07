/*
 * Smartfusion register definitions.
 *
 * Copyright (C) 2017 Cumulus Networks, Inc.
 * Author: Curt Brune <curt@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * https://www.gnu.org/licenses/gpl-2.0-standalone.html
 */

#ifndef S6100_SMF_REG_H__
#define S6100_SMF_REG_H__

#include <linux/regmap.h>

/**
 * smf_reg_rd(): Read smf lpc register
 *
 * @smf_map: regmap configured for the smf lpc registers
 * @reg: register offset
 * @val: pointer to hold 8-bit register read value
 *
 * Read an 8-bit smf lpc value from register @reg and return the
 * value in @val.
 *
 * A value of zero is returned on success, a negative errno is
 * returned in error cases.
 */
static inline int
smf_reg_rd(struct regmap *smf_map, unsigned int reg, uint8_t *val)
{
	int rc;
	unsigned int val32 = 0;

	rc = regmap_read(smf_map, reg, &val32);
	WARN(rc, "regmap_read(smf_map, 0x%x,...) failed: (%d)\n", reg, rc);

	*val = val32 & 0xFF;
	return rc;
}

/**
 * smf_reg_wr(): Write smf lpc register
 *
 * @smf_map: regmap configured for the smf lpc registers
 * @reg: register offset
 * @val: 8-bit value to write
 *
 * Writes @val to the 8-bit smf lpc register specified by @reg.
 *
 * A value of zero is returned on success, a negative errno is
 * returned in error cases.
 */
static inline int
smf_reg_wr(struct regmap *smf_map, unsigned int reg, uint8_t val)
{
	int rc;

	rc = regmap_write(smf_map, reg, val);
	WARN(rc, "regmap_write(smf_map, 0x%x, 0x%x) failed: (%d)\n",
	     reg, val, rc);

	return rc;
}

/**
 * smf_mb_reg_rd(): Read 8-bit smf mailbox register
 *
 * @smf_mb_map: regmap configured for the smf mailbox registers
 * @reg: register offset
 * @val: pointer to hold 8-bit register read value
 *
 * Read an 8-bit smf mailbox value from register @reg and return the
 * value in @val.
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
static inline int
smf_mb_reg_rd(struct regmap *smf_mb_map, unsigned int reg, uint8_t *val)
{
	int rc;
	unsigned int val32 = 0;

	rc = regmap_read(smf_mb_map, reg, &val32);
	WARN(rc, "regmap_read(smf_mb_map, 0x%x,...) failed: (%i)\n", reg, rc);

	*val = val32 & 0xff;
	return rc;
}

/**
 * smf_mb_reg_wr(): Write 8-bit smf mailbox register
 *
 * @smf_mb_map: regmap configured for the smf mailbox registers
 * @reg: register offset
 * @val: 8-bit value to write
 *
 * Writes @val to the 8-bit smf mailbox the register specified by
 * @reg.
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
static inline int
smf_mb_reg_wr(struct regmap *smf_mb_map, unsigned int reg, uint8_t val)
{
	int rc;

	rc = regmap_write(smf_mb_map, reg, val);
	WARN(rc, "regmap_write(smf_mb_map, 0x%x, 0x%x) failed: (%i)\n",
	     reg, val, rc);

	return rc;
}

/**
 * smf_mb_reg_rd16(): Read 16-bit smf mailbox register
 *
 * @smf_mb_map: regmap configured for the smf mailbox registers
 * @reg: register offset
 * @val: pointer to hold 16-bit register read value
 *
 * Read two consecutive 8-bit smf mailbox registers and return the
 * 16-bit result in @val.
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
static inline int
smf_mb_reg_rd16(struct regmap *smf_mb_map, unsigned int reg, uint16_t *val)
{
	int rc;
	uint8_t val1, val2;

	rc = smf_mb_reg_rd(smf_mb_map, reg, &val1);
	if (rc)
		return rc;

	rc = smf_mb_reg_rd(smf_mb_map, reg + 1, &val2);
	if (rc)
		return rc;

	*val = ((val1 & 0xFF) << 8) | (val2 & 0xFF);
	return rc;
}

/**
 * smf_mb_reg_rd_array(): Read N 8-bit smf mailbox registers
 *
 * @smf_mb_map: regmap configured for the smf mailbox registers
 * @reg: base register offset
 * @val: storage to hold register data
 * @len: length of @val array in bytes
 *
 * Read @len consecutive 8-bit smf mailbox registers and return the
 * result in @val.
 *
 * A value of zero will be returned on success, a negative errno will
 * be returned in error cases.
 */
static inline int
smf_mb_reg_rd_array(struct regmap *smf_mb_map, unsigned int reg,
		    uint8_t *val, size_t len)
{
	int rc = 0;
	int i;

	for (i = 0; i < len; i++) {
		rc = smf_mb_reg_rd(smf_mb_map, reg + i, val++);
		if (rc)
			return rc;
	}

	return rc;
}

#define S6100_SMF_IO_BASE 0x200
#define S6100_SMF_IO_SIZE 0x80

/* SMF LPC Mapped Registers */
#define S6100_SMF_VER            0x00
#define S6100_SMF_BOARD_TYPE	 0x01
#define S6100_SMF_SW_SCRATCH	 0x02
#define S6100_SMF_BOOT_OK	 0x03
#define S6100_SMF_UART_STA	 0x04
#define S6100_SMF_MSS_STA	 0x05
#define S6100_SMF_WD_WID	 0x06
#define S6100_SMF_WD_MASK	 0x07
#define S6100_SMF_POR_SOURCE	 0x09
#define S6100_SMF_RST_SOURCE	 0x0A
#define S6100_SMF_SEP_RST	 0x0B
#define S6100_SMF_CPU_RST_CTRL	 0x0C
#define S6100_SMF_RAM_ADDR_H	 0x10
#define S6100_SMF_RAM_ADDR_L	 0x11
#define S6100_SMF_RAM_R_DATA	 0x12
#define S6100_SMF_RAM_W_DATA	 0x13
#define S6100_SMF_CPU_EEPROM_WP	 0x20
#  define CPU_EEPROM_WP_EN         0x0
#define S6100_SMF_TPM_STA_ID	 0x21


/* Mailbox CPU Read-Only Registers */
#define S6100_SMF_MB_PROTO_VER			0x0000
#define S6100_SMF_MB_FIRMWARE_VER		0x0002
/*   Temp Sensor Registers */
#define S6100_SMF_MB_NUM_TEMP_SENSORS		0x0013
#define S6100_SMF_MB_TEMP01_SENSOR		0x0014
#define S6100_SMF_MB_TEMP01_HW_SHUT_LIMIT	0x003c
#define S6100_SMF_MB_TEMP01_SW_SHUT_LIMIT	0x003e
#define S6100_SMF_MB_TEMP01_MJR_ALARM_LIMIT	0x0040
#define S6100_SMF_MB_TEMP01_MNR_ALARM_LIMIT	0x0042
#define S6100_SMF_MB_TEMP01_STATUS		0x00dc
#  define S6100_SMF_MB_TEMP_STATUS_NORMAL	0x0
#  define S6100_SMF_MB_TEMP_STATUS_LT_HW_L	0x1
#  define S6100_SMF_MB_TEMP_STATUS_LT_SW_L	0x2
#  define S6100_SMF_MB_TEMP_STATUS_GT_HW_H	0x3
#  define S6100_SMF_MB_TEMP_STATUS_GT_SW_H	0x4
#  define S6100_SMF_MB_TEMP_STATUS_FAULT	0x5
#define S6100_SMF_MB_TEMP16_STATUS		0x00eb
/*   Fan Sensor Registers */
#define S6100_SMF_MB_FAN_TRAY_CNT		0x00f0
#define S6100_SMF_MB_FANS_PER_TRAY		0x00f1
#define S6100_SMF_MB_FAN_MAX_SPEED		0x00f2
#define S6100_SMF_MB_FAN_TRAY1_FAN1_SPEED_H	0x00f3
#define S6100_SMF_MB_FAN_TRAY1_FAN1_SPEED_L	0x00f4
#define S6100_SMF_MB_FAN_TRAY1_FAN2_SPEED_H	0x00f5
#define S6100_SMF_MB_FAN_TRAY1_FAN2_SPEED_L	0x00f6
#define S6100_SMF_MB_FAN_TRAYS_PRESENT		0x0113
#define S6100_SMF_MB_FAN_STATUS			0x0114
#define S6100_SMF_MB_FAN_TRAYS_F2B		0x0116
#define S6100_SMF_MB_FAN_TRAY1_SERIAL_NUM	0x0117
#define   S6100_SMF_MB_FAN_TRAY_SERIAL_NUM_SIZE	20
#define S6100_SMF_MB_FAN_TRAY1_PART_NUM		0x012B
#define   S6100_SMF_MB_FAN_TRAY_PART_NUM_SIZE	6
#define S6100_SMF_MB_FAN_TRAY1_LABEL_REV	0x0131
#define   S6100_SMF_MB_FAN_TRAY_LABEL_REV_SIZE	3
#define S6100_SMF_MB_FAN_TRAY1_MFG_DATE		0x0134
#define   S6100_SMF_MB_FAN_TRAY_MFG_DATE_SIZE	6
/*   Fan trays 2 through 8 .... */
#define S6100_SMF_MB_FAN_TRAY8_MFG_DATE		0x0134
/*   Power and PSU sensor registers */
#define S6100_SMF_MB_PSU_CNT			0x0231
#define S6100_SMF_MB_PSU_TOTAL_POWER_H		0x0232
#define S6100_SMF_MB_PSU_TOTAL_POWER_L		0x0233
#define S6100_SMF_MB_PSU1_MAX_H			0x0234
#define S6100_SMF_MB_PSU1_MAX_L			0x0235
#define S6100_SMF_MB_PSU1_FUNC			0x0236
#define S6100_SMF_MB_PSU1_STATUS		0x0237
#  define PSU_STATUS_OUTPUT_VOLTAGE_OK_L	GENMASK(3, 2)
#  define PSU_STATUS_INPUT_TYPE_DC		BIT(1)
#  define PSU_STATUS_PRESENT_L			BIT(0)
#define S6100_SMF_MB_PSU1_TEMP			0x0239
#define S6100_SMF_MB_PSU1_FAN_SPEED		0x023B
#define S6100_SMF_MB_PSU1_FAN_STATUS		0x023D
#  define PSU_FAN_STATUS_PRESENT_L		BIT(2)
#  define PSU_FAN_STATUS_OK_L			BIT(1)
#  define PSU_FAN_STATUS_F2B			BIT(0)
#define S6100_SMF_MB_PSU1_INPUT_VOLT		0x023E
#define S6100_SMF_MB_PSU1_OUTPUT_VOLT		0x0240
#define S6100_SMF_MB_PSU1_INPUT_CURRENT		0x0232
#define S6100_SMF_MB_PSU1_OUTPUT_CURRENT	0x0244
#define S6100_SMF_MB_PSU1_INPUT_POWER		0x0246
#define S6100_SMF_MB_PSU1_OUTPUT_POWER		0x0248
#define S6100_SMF_MB_PSU1_COUNTY_CODE		0x024A
#define   S6100_SMF_MB_PSU_COUNTY_CODE_SIZE	2
#define S6100_SMF_MB_PSU1_PART_NUM		0x024C
#define   S6100_SMF_MB_PSU_PART_NUM_SIZE	6
#define S6100_SMF_MB_PSU1_MFG_ID		0x0252
#define   S6100_SMF_MB_PSU_MFG_ID_SIZE		5
#define S6100_SMF_MB_PSU1_MFG_DATE		0x0257
#define   S6100_SMF_MB_PSU_MFG_DATE_SIZE	8
#define S6100_SMF_MB_PSU1_SERIAL_NUM		0x025F
#define   S6100_SMF_MB_PSU_SERIAL_NUM_SIZE	4
#define S6100_SMF_MB_PSU1_SERVICE_TAG		0x0263
#define   S6100_SMF_MB_PSU_SERVICE_TAG_SIZE	8
#define S6100_SMF_MB_PSU1_LABEL_REV		0x026A
#define   S6100_SMF_MB_PSU_LABEL_REV_SIZE	3
#define S6100_SMF_MB_INTR_SOURCE		0x030c
#define S6100_SMF_MB_SENSOR_SHUTDOWN_REASON	0x030d
#define S6100_SMF_MB_PSU_MISMATCH		0x030e
#define S6100_SMF_MB_FAN_TRAY_MISMATCH		0x030f
#define S6100_SMF_MB_IO_MODULE_POWER_STATUS	0x0310
#define S6100_SMF_MB_IO_MODULE_PRESENT		0x0311
#define S6100_SMF_MB_FAN_EEPROM_NOT_EMPTY	0x0312
#define S6100_SMF_MB_SCAN_RESULT_1		0x0313
#define S6100_SMF_MB_SCAN_RESULT_2		0x0314
#define S6100_SMF_MB_SCAN_RESULT_3		0x0315
#define S6100_SMF_MB_SCAN_RESULT_4		0x0316
#define S6100_SMF_MB_RESERVED1			0x0317
/* S6100_SMF_MB_RESERVED1 extends through 0x3FF */
#define   S6100_SMF_MB_RESERVED1_SIZE		0xe8

/* Mailbox CPU Read-Only Register Helper constants */
#define S6100_SMF_MB_FAN_TRAY_SPEED_BASE \
	S6100_SMF_MB_FAN_TRAY1_FAN1_SPEED_H
#define S6100_SMF_MB_FAN_TRAY_SPEED_OFFSET	   \
	(S6100_SMF_MB_FAN_TRAY1_FAN2_SPEED_L + 1 - \
	 S6100_SMF_MB_FAN_TRAY1_FAN1_SPEED_H)
#define S6100_SMF_MB_FAN_TRAY_VPD_OFFSET \
	(S6100_SMF_MB_FAN_TRAY_SERIAL_NUM_SIZE + \
	 S6100_SMF_MB_FAN_TRAY_PART_NUM_SIZE   + \
	 S6100_SMF_MB_FAN_TRAY_LABEL_REV_SIZE  + \
	 S6100_SMF_MB_FAN_TRAY_MFG_DATE_SIZE)
#define S6100_SMF_MB_PSU_BASE			S6100_SMF_MB_PSU1_MAX_H
#define S6100_SMF_MB_PSU_OFFSET		   \
	(S6100_SMF_MB_PSU1_LABEL_REV +	   \
	 S6100_SMF_MB_PSU_LABEL_REV_SIZE - \
	 S6100_SMF_MB_PSU_BASE)
#define S6100_SMF_MB_MAX_STRING_SIZE \
	(S6100_SMF_MB_FAN_TRAY_SERIAL_NUM_SIZE + 1)

/* Mailbox CPU Read-Write Registers */
#define S6100_SMF_MB_TEMP_UPDATE_FLAG		0x0400
#define S6100_SMF_MB_MAX_NUM_CPU		0x0401
#define S6100_SMF_MB_CPU_1_TEMP_H		0x0402
#define S6100_SMF_MB_CPU_1_TEMP_L		0x0403
#define S6100_SMF_MB_CPU_2_TEMP_H		0x0404
#define S6100_SMF_MB_CPU_2_TEMP_L		0x0405
#define S6100_SMF_MB_MAX_NUM_SWITCH_CHIP	0x0406
#define S6100_SMF_MB_SWITCH_CHIP_1_TEMP_H	0x0407
#define S6100_SMF_MB_SWITCH_CHIP_1_TEMP_L	0x0408
#define S6100_SMF_MB_SWITCH_CHIP_2_TEMP_H	0x0409
#define S6100_SMF_MB_SWITCH_CHIP_2_TEMP_L	0x040a
#define S6100_SMF_MB_MAX_NUM_DIMM		0x040b
#define S6100_SMF_MB_DIMM_1_TEMP_H		0x040c
#define S6100_SMF_MB_DIMM_1_TEMP_L		0x040d
#define S6100_SMF_MB_DIMM_2_TEMP_H		0x040e
#define S6100_SMF_MB_DIMM_2_TEMP_L		0x040f
#define S6100_SMF_MB_RESERVED2			0x0410
#define   S6100_SMF_MB_RESERVED2_SIZE		0xc7
#define S6100_SMF_MB_SMF_SCAN_I2C_DEVICE	0x04d7
#define S6100_SMF_MB_SMF_RELEASE_I2C_DEVICE	0x04d8
#define S6100_SMF_MB_IO_MODULE1_POWER_CTRL	0x04d9
#  define S6100_IO_MODULE_POWER_OK		  0x0
#  define S6100_IO_MODULE_POWER_ON		  0x1
#  define S6100_IO_MODULE_POWER_OFF		  0x2
#  define S6100_IO_MODULE_POWER_FAIL		  0x3
#  define S6100_IO_MODULE_POWER_MASK		  0x3
#define S6100_SMF_MB_IO_MODULE2_POWER_CTRL	0x04da
#define S6100_SMF_MB_IO_MODULE3_POWER_CTRL	0x04db
#define S6100_SMF_MB_IO_MODULE4_POWER_CTRL	0x04dc
#define S6100_SMF_MB_RESERVED3			0x04dd
#define S6100_SMF_MB_RESERVED4			0x04de
#define S6100_SMF_MB_SYS_STATUS_LED_CTRL	0x04df
#  define SYS_LED_STATUS_GREEN			  0x0
#  define SYS_LED_STATUS_GREEN_BLINKING		  0x1
#  define SYS_LED_STATUS_AMBER			  0x2
#  define SYS_LED_STATUS_AMBER_BLINKING		  0x8
#define S6100_SMF_MB_SYS_BEACON_LED_CTRL	0x04e0
#define S6100_SMF_MB_SYS_STACKING_LED_CTRL	0x04e1
#define S6100_SMF_MB_RESERVED5			0x04e2
#define S6100_SMF_MB_RESERVED6			0x04e3
#define S6100_SMF_MB_PSU_FAN_ALARM_LED_CTRL	0x04e4
#define S6100_SMF_MB_PSU_FAN_STATUS_LED_CTRL	0x04e4
#define S6100_SMF_MB_FAN_TRAY1_INTR		0x04f1
#define S6100_SMF_MB_FAN_TRAY2_INTR		0x04f2
#define S6100_SMF_MB_FAN_TRAY3_INTR		0x04f3
#define S6100_SMF_MB_FAN_TRAY4_INTR		0x04f4
#define S6100_SMF_MB_PSU1_INTR			0x04f5
#define S6100_SMF_MB_PSU2_INTR			0x04f6
#define S6100_SMF_MB_SWITCH_BOARD_INTR		0x04f7
#define S6100_SMF_MB_CPU_BOARD_NTERRUPT		0x04f8
#define S6100_SMF_MB_SENSOR_INTR_MASK		0x04fa
#define S6100_SMF_MB_FAN_TRAY_INTR_MASK		0x04fb
#define S6100_SMF_MB_PSU_INTR_MASK		0x04fc
#define S6100_SMF_MB_SWITCH_BOARD_INTR_MASK	0x04fd
#define S6100_SMF_MB_CPU_BOARD_INTR_MASK	0x04fe
#define S6100_SMF_MB_RESERVED7			0x04ff
#define S6100_SMF_MB_POWER_CYCLE_CTRL		0x0500

/**
 * Mailbox CPU Read-Write Raw I2C Registers
 *
 * Registers for raw i2c access from the CPU begin at 0x0600.  Only
 * implement these if necessary.
*/

/**
 * S6100_SMF_MB_BAD_READ
 *
 * SMF mailbox reads can fail for a number of reasons:
 *
 * - device (e.g. PSU) is not attached
 * - temperature sensor reading is not ready
 * - register out of bounds
 * - other failure
 *
 * Seemingly the SMF returns the value 0xffff to indicate an
 * undesirable outcome.
*/

#define S6100_SMF_MB_BAD_READ			0xffff

#endif /* S6100_SMF_REG_H__ */
