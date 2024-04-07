/*
 * DNI AG7648 CPLD Platform Definitions
 *
 * Copyright (C) 2017 Cumulus Networks, Inc.
 * Nikhil Dhar <ndhar@cumulusnetworks.com>
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
 */

#ifndef DNI_AG7648_CPLD_H__
#define DNI_AG7648_CPLD_H__

/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 */
#define DNI_AG7648_CPLD_STRING_NAME_SIZE  30

/* the AG7648 actually has three CPLDs: system, master, slave
 * the CPLD driver has been coded such that we have the appearance of a single
 * CPLD. We do this by embedding a two-bit index for which CPLD must be accessed
 * into each register definition. When the register is accessed, the register
 * definition is decoded into a CPLD index and register offset.
 */

#define CPLD_IDX_SHIFT             (6)
#define SYSTEM_CPLD_ID             (1)
#define MASTER_CPLD_ID             (2)
#define SLAVE_CPLD_ID              (3)
#define CPLD_IDX_MASK              (3 << CPLD_IDX_SHIFT)
#define SYSTEM_CPLD_IDX            (SYSTEM_CPLD_ID << CPLD_IDX_SHIFT)
#define MASTER_CPLD_IDX            (MASTER_CPLD_ID << CPLD_IDX_SHIFT)
#define SLAVE_CPLD_IDX             (SLAVE_CPLD_ID << CPLD_IDX_SHIFT)
#define NO_CPLD_I2C_CLIENTS        (3)

#define GET_CPLD_IDX(A)            (((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT) - 1)
#define STRIP_CPLD_IDX(A)          (A & ~CPLD_IDX_MASK)

#define SFP_REG_CONSTRUCT(str, regtype) \
	str ## regtype ## _REG

#define SFP_REG(str, regtype, _addr, _cpld_idx) \
	SFP_REG_CONSTRUCT(str, regtype) = (_addr | _cpld_idx)

#define SFP_ENUM(regtype, _addr, _cpld_idx) \
	enum { \
		SFP_REG(CPLD_SFP_1_8_MOD_, regtype, _addr, _cpld_idx), \
		SFP_REG(CPLD_SFP_9_16_MOD_, regtype, (_addr + 1), _cpld_idx), \
		SFP_REG(CPLD_SFP_17_24_MOD_, regtype, (_addr + 2), _cpld_idx), \
		SFP_REG(CPLD_SFP_25_32_MOD_, regtype, (_addr + 3), _cpld_idx), \
		SFP_REG(CPLD_SFP_33_40_MOD_, regtype, (_addr + 4), _cpld_idx), \
		SFP_REG(CPLD_SFP_41_48_MOD_, regtype, (_addr + 5), _cpld_idx), \
	}

/* System CPLD register definitions */
#define CPLD_SYSTEM_HW_REV_REG				(0x0 | SYSTEM_CPLD_IDX)

/* Master CPLD register definions */

#define CPLD_MASTER_HW_REV_REG				(0x01 | MASTER_CPLD_IDX)
/* 0 - Reset, 1 - Normal op */
#define CPLD_MASTER_RST_REG				(0x02 | MASTER_CPLD_IDX)
#define CPLD_MASTER_RST_REG_RST_PHY			BIT(0)
#define CPLD_MASTER_RST_REG_RST_TD2			BIT(1)
#define CPLD_MASTER_RST_REG_RST_PORT_LED1		BIT(2)
#define CPLD_MASTER_RST_REG_RST_PORT_LED2		BIT(3)
#define CPLD_MASTER_RST_REG_RST_USB			BIT(4)
#define CPLD_MASTER_RST_REG_RST_MAIN_BOARD		BIT(5)

#define CPLD_MASTER_POWER_SUPPLY_STATUS_REG		(0x03 | MASTER_CPLD_IDX)
#define CPLD_PSU1_MASK					(0xF0)
/* 0 - Present, 1 - Not Present */
#define CPLD_PSU1_PRESENT_L				BIT(7)
/* 0 - Power good, 1 - Power failed or not present */
#define CPLD_PSU1_GOOD_L				BIT(6)
/* 0 - Critical Event, 1 - No events */
#define CPLD_PSU1_ERROR					BIT(5)
/* 0 - Turn PSU1 on, 1 - Turn PSU1 off */
#define CPLD_PSU1_ON					BIT(4)
#define CPLD_PSU2_MASK					(0x0F)
#define CPLD_PSU2_PRESENT_L				BIT(3)
#define CPLD_PSU2_GOOD_L				BIT(2)
#define CPLD_PSU2_ERROR					BIT(1)
#define CPLD_PSU2_ON					BIT(0)

#define CPLD_MASTER_INT_REG				(0x06 | MASTER_CPLD_IDX)
#define CPLD_POWER_LED_MASK				(0xC0)
#define CPLD_POWER_LED_OFF				(0x00)
#define CPLD_POWER_LED_YELLOW				(0x40)
#define CPLD_POWER_LED_GREEN				(0x80)
#define CPLD_POWER_LED_YELLOW_BLINK			(0xC0)

#define CPLD_MASTER_LED_CONTROL_REG1			(0x07 | MASTER_CPLD_IDX)
#define CPLD_SYSTEM_LED_MASK				(0x60)
#define CPLD_SYSTEM_LED_GREEN_BLINK			(0x0)
#define CPLD_SYSTEM_LED_GREEN				(0x1)
#define CPLD_SYSTEM_LED_YELLOW				(0x2)
#define CPLD_SYSTEM_LED_YELLOW_BLINK			(0x3)
#define CPLD_LOC_LED_MASK				(0x18)
#define CPLD_LOC_LED_OFF				(0x0)
#define CPLD_LOC_LED_GREEN_BLINK			(0x1)
#define CPLD_LOC_LED_GREEN				(0x2)

#define CPLD_MASTER_LED_CONTROL_REG2			(0x08 | MASTER_CPLD_IDX)
#define CPLD_FAN_TRAY_2_PRESENT_L			BIT(7)
#define CPLD_FAN_TRAY_1_PRESENT_L			BIT(6)
#define CPLD_FAN_TRAY_3_LED_MASK			(0x30)
#define CPLD_FAN_TRAY_2_LED_MASK			(0x0C)
#define CPLD_FAN_TRAY_1_LED_MASK			(0x03)
#define CPLD_FAN_TRAY_1_LED_OFF				(0x0)
#define CPLD_FAN_TRAY_1_LED_GREEN			(0x1)
#define CPLD_FAN_TRAY_1_LED_YELLOW			(0x2)
#define CPLD_FAN_TRAY_2_LED_OFF				(0x0)
#define CPLD_FAN_TRAY_2_LED_GREEN			(0x4)
#define CPLD_FAN_TRAY_2_LED_YELLOW			(0x8)
#define CPLD_FAN_TRAY_3_LED_OFF				(0x0)
#define CPLD_FAN_TRAY_3_LED_GREEN			(0x10)
#define CPLD_FAN_TRAY_3_LED_YELLOW			(0x20)

#define CPLD_MASTER_FAN_STATUS_REG			(0x09 | MASTER_CPLD_IDX)
#define CPLD_FRONT_FAN_LED_MASK				(0x18)
#define CPLD_FRONT_FAN_LED_OFF				(0x0)
#define CPLD_FRONT_FAN_LED_YELLOW			(0x08)
#define CPLD_FRONT_FAN_LED_GREEN			(0x10)
#define CPLD_FRONT_FAN_LED_YELLOW_BLINK			(0x18)
/* 0 - Fan present, 1 - absent */
#define CPLD_FAN_TRAY_3_PRESENT_L			BIT(0)

#define CPLD_QSFP_MOD_SELECT_REG			(0x0A | MASTER_CPLD_IDX)
#define CPLD_QSFP_LP_MODE_REG				(0x0B | MASTER_CPLD_IDX)
#define CPLD_QSFP_PRESENT_L_REG				(0x0C | MASTER_CPLD_IDX)
#define CPLD_QSFP_RESET_L_REG				(0x0D | MASTER_CPLD_IDX)
#define CPLD_QSFP_INTERRUPT_L_REG			(0x0E | MASTER_CPLD_IDX)

#define CPLD_SFP_MUX_SELECT_REG				(0x11 | MASTER_CPLD_IDX)

/* Slave CPLD register definitions */
#define CPLD_SLAVE_REV_REG				(0x0 | SLAVE_CPLD_IDX)

SFP_ENUM(PRESENT_L, 0x02, SLAVE_CPLD_IDX);
SFP_ENUM(TX_DISABLE, 0x08, SLAVE_CPLD_IDX);
SFP_ENUM(RX_LOS, 0x0E, SLAVE_CPLD_IDX);
SFP_ENUM(TX_FAULT, 0x14, SLAVE_CPLD_IDX);

#endif
