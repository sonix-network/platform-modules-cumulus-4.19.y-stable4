/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Dell EMC S5048F CPLD Definitions
 *
 * Copyright (C) 2018, 2020 Cumulus Networks, Inc.  All rights reserved.
 * Author: David Yen (dhyen@cumulusnetworks.com)
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

#ifndef DELL_S5048F_CPLD_H__
#define DELL_S5048F_CPLD_H__

/*------------------------------------------------------------------------------
 *
 * This platform has four CPLD devices.  CPLD1 is accessed via the LPC bus;
 * and CPLD2, CPLD3, and CPLD4 are accessed via the I2C bus.
 *
 *------------------------------------------------------------------------------
 */

#define S5048F_PLATFORM_NAME "s5048f_platform"

#define S5048F_CPLD1_NAME    "s5048f_cpld1"
#define CPLD1_IO_BASE        0x100
#define CPLD1_IO_SIZE        0x0FF

#define S5048F_CPLD2_NAME    "s5048f_cpld2"
#define S5048F_CPLD3_NAME    "s5048f_cpld3"
#define S5048F_CPLD4_NAME    "s5048f_cpld4"

/*------------------------------------------------------------------------------
 *
 * This platform has two i2c busses:
 *  SMBus_0: SMBus I801 adapter at f000
 *  SMBus_1: SMBus iSMT adapter at dfff0000
 *
 *------------------------------------------------------------------------------
 */

enum {
	I2C_ISMT_BUS = 0,
	I2C_I801_BUS,

	I2C_SWITCH1_BUS0 = 10,
	I2C_SWITCH1_BUS1,
	I2C_SWITCH1_BUS2,
	I2C_SWITCH1_BUS3,
	I2C_SWITCH1_BUS4,
	I2C_SWITCH1_BUS5,
	I2C_SWITCH1_BUS6,
	I2C_SWITCH1_BUS7,

	I2C_MUX2_BUS0 = 20,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,
	I2C_MUX2_BUS4,
	I2C_MUX2_BUS5,
	I2C_MUX2_BUS6,
	I2C_MUX2_BUS7,

	I2C_MUX4_BUS0 = 79,
	I2C_MUX4_BUS1,
	I2C_MUX4_BUS2,
	I2C_MUX4_BUS3,
	I2C_MUX4_BUS4,
	I2C_MUX4_BUS5,
	I2C_MUX4_BUS6,
	I2C_MUX4_BUS7,

	I2C_MUX5_BUS0 = 39,
	I2C_MUX5_BUS1,
	I2C_MUX5_BUS2,
	I2C_MUX5_BUS3,
	I2C_MUX5_BUS4,
	I2C_MUX5_BUS5,
	I2C_MUX5_BUS6,
	I2C_MUX5_BUS7,

	I2C_MUX6_BUS0 = 31,
	I2C_MUX6_BUS1,
	I2C_MUX6_BUS2,
	I2C_MUX6_BUS3,
	I2C_MUX6_BUS4,
	I2C_MUX6_BUS5,
	I2C_MUX6_BUS6,
	I2C_MUX6_BUS7,

	I2C_MUX7_BUS0 = 47,
	I2C_MUX7_BUS1,
	I2C_MUX7_BUS2,
	I2C_MUX7_BUS3,
	I2C_MUX7_BUS4,
	I2C_MUX7_BUS5,
	I2C_MUX7_BUS6,
	I2C_MUX7_BUS7,

	I2C_MUX8_BUS0 = 71,
	I2C_MUX8_BUS1,
	I2C_MUX8_BUS2,
	I2C_MUX8_BUS3,
	I2C_MUX8_BUS4,
	I2C_MUX8_BUS5,
	I2C_MUX8_BUS6,
	I2C_MUX8_BUS7,

	I2C_MUX9_BUS0 = 63,
	I2C_MUX9_BUS1,
	I2C_MUX9_BUS2,
	I2C_MUX9_BUS3,
	I2C_MUX9_BUS4,
	I2C_MUX9_BUS5,
	I2C_MUX9_BUS6,
	I2C_MUX9_BUS7,

	I2C_MUX10_BUS0 = 55,
	I2C_MUX10_BUS1,
	I2C_MUX10_BUS2,
	I2C_MUX10_BUS3,
	I2C_MUX10_BUS4,
	I2C_MUX10_BUS5,
	I2C_MUX10_BUS6,
	I2C_MUX10_BUS7,
};

/*------------------------------------------------------------------------------
 *
 * These registers are common to all four CPLDs.  For CPLD1, the register
 * addresses listed are the offsets from IO_BASE.
 *
 *------------------------------------------------------------------------------
 */

#define CPLD_VERSION_REG						 0x00
#  define CPLD_MAJOR_VER_MSB						 7
#  define CPLD_MAJOR_VER_LSB						 4
#  define CPLD_MINOR_VER_MSB						 3
#  define CPLD_MINOR_VER_LSB						 0

/* note: restricting this to three bits for now */
#define BOARD_TYPE_REG							 0x01
#  define BOARD_TYPE_MSB						 2
#  define BOARD_TYPE_LSB						 0

#define SW_SCRATCH_REG							 0x02
#  define SW_SCRATCH_MSB						 7
#  define SW_SCRATCH_LSB						 0

/* note: restricting this to three bits for now */
#define CPLD_ID_REG							 0x03
#  define CPLD_ID_MSB							 2
#  define CPLD_ID_LSB							 0

/*------------------------------------------------------------------------------
 *
 * These registers are unique to CPLD1.
 *
 *------------------------------------------------------------------------------
 */

#define CPLD_SET_RST0_REG						 0x10
#  define PCA9548_RST10_BIT						 7
#  define PCA9548_RST9_BIT						 6
#  define PCA9548_RST8_BIT						 5
#  define PCA9548_RST7_BIT						 4
#  define PCA9548_RST6_BIT						 3
#  define PCA9548_RST5_BIT						 2
#  define PCA9548_RST4_BIT						 1
#  define PCA9548_RST2_BIT						 0

#define CPLD_SET_RST1_REG						 0x11
#  define PCIE_RST_BIT							 7
#  define DPLL_RST_BIT							 6
#  define SW_RST_BIT							 5
#  define FPGA_RST_BIT							 4
#  define CPLD_RST4_BIT							 2
#  define CPLD_RST3_BIT							 1
#  define CPLD_RST2_BIT							 0

#define SW_EEPROM_WP_REG						 0x20
#  define SW_EEPROM_WP_BIT						 0

#define SYS_LED_CTRL_REG						 0x30
#  define SYS_STA_LED_MSB						 7
#  define SYS_STA_LED_LSB						 6
#  define PWR_STA_LED_MSB						 5
#  define PWR_STA_LED_LSB						 4
#  define FAN_STA_LED_MSB						 3
#  define FAN_STA_LED_LSB						 2
#  define BEACON_LED_BIT						 1
#  define STACKING_LED_BIT						 0

#define SEG7_LED_CTRL_REG						 0x31
#  define SEG7_LED_CTL_MSB						 7
#  define SEG7_LED_CTL_LSB						 0

#define MISC_TRIG_MOD_REG						 0x40
#  define MISC_INT_TRIG_MSB						 1
#  define MISC_INT_TRIG_LSB						 0

#define MISC_COMBINE_REG						 0x41
#  define MISC_INT_COMBINE_BIT						 0

#define MISC_STA_REG							 0x42
#  define IR3581_ALERT_N_STA_BIT					 4
#  define IR3584_1P0V_ALERT_N_STA_BIT					 3
#  define IR3584_3P3V_ALERT_N_STA_BIT					 2
#  define IR3584_3P3V_VRHOT_STA_BIT					 1
#  define BCM54616S_INT_STA_BIT						 0

#define MISC_INT_REG							 0x43
#  define IR3581_ALERT_N_INT_BIT					 4
#  define IR3584_1P0V_ALERT_N_INT_BIT					 3
#  define IR3584_3P3V_ALERT_N_INT_BIT					 2
#  define IR3584_3P3V_VRHOT_INT_BIT					 1
#  define BCM54616S_INT_INT_BIT						 0

#define MISC_INT_MASK_REG						 0x44
#  define IR3581_ALERT_N_MASK_BIT					 4
#  define IR3584_1P0V_ALERT_N_MASK_BIT					 3
#  define IR3584_3P3V_ALERT_N_MASK_BIT					 2
#  define IR3584_3P3V_VRHOT_MASK_BIT					 1
#  define BCM54616S_INT_MASK_BIT					 0

#define OPTICAL_INT_STA_REG						 0x50
#  define CPLD4_SFP28_RXLOS_INT_STA_BIT					 7
#  define CPLD4_SFP28_ABS_INT_STA_BIT					 6
#  define CPLD4_INT_INT_STA_BIT						 5
#  define CPLD4_ABS_INT_STA_BIT						 4
#  define CPLD3_SFP28_RXLOS_INT_STA_BIT					 3
#  define CPLD3_ABS_INT_STA_BIT						 2
#  define CPLD2_SFP28_RXLOS_INT_STA_BIT					 1
#  define CPLD2_ABS_INT_STA_BIT						 0

/*------------------------------------------------------------------------------
 *
 * These registers are (mostly) common to CPLD2, CPLD3, and CPLD4.
 *
 *------------------------------------------------------------------------------
 */

#define NUM_CPLD2_SFP28_REGS						 3
#define NUM_CPLD3_SFP28_REGS						 3
#define NUM_CPLD4_SFP28_REGS						 2
#define NUM_CPLD4_QSFP28_REGS						 1

#define PORT_LED_OPMOD_REG						 0x09
#  define OP_MOD_BIT							 0

#define PORT_LED_TEST_REG						 0x0a
#  define OP_CTRL_AMBER_MSB						 3
#  define OP_CTRL_AMBER_LSB						 2
#  define OP_CTRL_GREEN_MSB						 1
#  define OP_CTRL_GREEN_LSB						 0

#define SFP28_TXDISABLE_CTRL0_REG					 0x10
#define SFP28_TXDISABLE_CTRL1_REG					 0x11
#define SFP28_TXDISABLE_CTRL2_REG					 0x12

#define SFP28_RS_CTRL0_REG						 0x13
#define SFP28_RS_CTRL1_REG						 0x14
#define SFP28_RS_CTRL2_REG						 0x15

#define SFP28_RXLOS_STA0_REG						 0x16
#define SFP28_RXLOS_STA1_REG						 0x17
#define SFP28_RXLOS_STA2_REG						 0x18

#define SFP28_TXFAULT_STA0_REG						 0x19
#define SFP28_TXFAULT_STA1_REG						 0x1a
#define SFP28_TXFAULT_STA2_REG						 0x1b

#define SFP28_ABS_STA0_REG						 0x1c
#define SFP28_ABS_STA1_REG						 0x1d
#define SFP28_ABS_STA2_REG						 0x1e

#define SFP28_TRIG_MOD_REG						 0x20
#  define SFP28_PRS_TRIG_MSB						 3
#  define SFP28_PRS_TRIG_LSB						 2
#  define SFP28_RXLOS_TRIG_MSB						 1
#  define SFP28_RXLOS_TRIG_LSB						 0

#define SFP28_COMBINE_REG						 0x21
#  define SFP28_ABS_COMBINE_BIT						 1
#  define SFP28_RXLOS_COMBINE_BIT					 0

#define SFP28_RXLOS_INT0_REG						 0x30
#define SFP28_RXLOS_INT1_REG						 0x31
#define SFP28_RXLOS_INT2_REG						 0x32

#define SFP28_ABS_INT0_REG						 0x33
#define SFP28_ABS_INT1_REG						 0x34
#define SFP28_ABS_INT2_REG						 0x35

#define SFP28_RXLOS_MASK0_REG						 0x36
#define SFP28_RXLOS_MASK1_REG						 0x37
#define SFP28_RXLOS_MASK2_REG						 0x38

#define SFP28_ABS_MASK0_REG						 0x39
#define SFP28_ABS_MASK1_REG						 0x3a
#define SFP28_ABS_MASK2_REG						 0x3b

/*------------------------------------------------------------------------------
 *
 * These registers are unique to CPLD4.
 *
 *------------------------------------------------------------------------------
 */

#define QSFP28_RESET_CTRL0_REG						 0x40

#define QSFP28_LPMOD_CTRL0_REG						 0x41

#define QSFP28_INT_STA0_REG						 0x42

#define QSFP28_ABS_STA0_REG						 0x43

#define QSFP28_TRIG_MOD_REG						 0x50
#  define QSFP28_PRS_TRIG_MSB						 3
#  define QSFP28_PRS_TRIG_LSB						 2
#  define QSFP28_INT_TRIG_MSB						 1
#  define QSFP28_INT_TRIG_LSB						 0

#define QSFP28_COMBINE_REG						 0x51
#  define QSFP28_PRES_COMBINE_BIT					 1
#  define QSFP28_INT_COMBINE_BIT					 0

#define QSFP28_INT_INT0_REG						 0x60

#define QSFP28_ABS_INT0_REG						 0x61

#define QSFP28_INT_MASK0_REG						 0x62

#define QSFP28_ABS_MASK0_REG						 0x63

#endif  /* DELL_S5048F_CPLD_H__ */
