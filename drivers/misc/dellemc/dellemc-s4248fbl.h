/*
 * Copyright (C) 2019 Cumulus Networks, Inc.  All Rights Reserved.
 *
 * Dell EMC S4248FBL CPLD Platform Definitions
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

#ifndef DELL_S4248FBL_CPLD_H__
#define DELL_S4248FBL_CPLD_H__

/*
 * This platform has two i2c busses:
 *  SMBus_1: SMBus I801 adapter at f000
 *  SMBus_0: SMBus iSMT adapter at dfff0000
 *
 * SMBus_1 is connected to the following devices:
 *    pca9547 8-channel switch (0x70)
 *	 0 eeprom (0x50-0x57)
 *	 1 bus master selector (0x74)
 *	 2 pca9548 8-channel mux (0x71)
 *	     0 eeprom (0x50)
 *	     4 cpld#2 (0x3e)
 *	     5 cpld#3 (0x3e)
 *	     6 cpld#4 (0x3e)
 *	     7 cpld#1 (0x3e)
 *	 4 pca9548 8-channel mux for qsfp28 41-48 (0x72)
 *	 5 pca9548 8-channel mux for  sfp28 33-40 (0x71)
 *	 5 pca9548 8-channel mux for  sfp28 25-32 (0x72)
 *	 6 pca9548 8-channel mux for  sfp28 17-24 (0x71)
 *	 6 pca9548 8-channel mux for  sfp28  9-16 (0x72)
 *	 7 pca9548 8-channel mux for  sfp28  1- 8 (0x72)
 *
 * SMBus_0 is connected to the following devices:
 *    i2c host (0x08)
 *    so-dimm temp sensor (0x18)
 *    isl90727 ddr3 vref tuning (0x2e)
 *    so-dimm eeprom (0x50)
 *    clock gen (0x69)
 */

/* i2c bus adapter numbers for the down stream i2c busses */
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
 * Device data and driver data structures
 *
 */
#define CPLD_STRING_NAME_SIZE  30

/* The S4248FBL has three CPLDs: cpld2, cpld3, cpld4.  The optional cpld1 does
 * not appear to be populated.
 *
 * The CPLD driver has been coded such that we have the appearance of a single
 * CPLD. We do this by embedding a two-bit index for which CPLD must be
 * accessed into each register definition. When the register is accessed, the
 * register definition is decoded into a CPLD index and register offset.
 */
#define NUM_CPLD_DEVICES                      (3)
#define CPLD2_ID                              (0)
#define CPLD3_ID                              (1)
#define CPLD4_ID                              (2)

#define CPLD_IDX_SHIFT                        (12)
#define CPLD_IDX_MASK                         (3 << CPLD_IDX_SHIFT)
#define CPLD2_IDX                             (CPLD2_ID << CPLD_IDX_SHIFT)
#define CPLD3_IDX                             (CPLD3_ID << CPLD_IDX_SHIFT)
#define CPLD4_IDX                             (CPLD4_ID << CPLD_IDX_SHIFT)

#define GET_CPLD_ID(A)                  ((A & CPLD_IDX_MASK) >> CPLD_IDX_SHIFT)
#define STRIP_CPLD_IDX(A)                     (A & ~CPLD_IDX_MASK)

#define MAX_REGS                              (8)

/*
 * These registers are common to all CPLDs
 */

#define CPLD_VERSION_REG                      (0x00)
#define  CPLD_MAJOR_VER_MASK                  (0XF0)
#define  CPLD_MAJOR_VER_SHIFT                 (4)
#define  CPLD_MINOR_VER_MASK                  (0X0F)
#define  CPLD_MINOR_VER_SHIFT                 (0)

#define BOARD_TYPE_REG                        (0x01)
#define  Z9100_BOARD                          (0X01)
#define  S6100_BOARD                          (0x02)
#define  S4200_BOARD                          (0x03)

#define CPLD_ID_REG                           (0x03)
#define  CPLD1                                (0x01)
#define  CPLD2                                (0x02)
#define  CPLD3                                (0x03)
#define  CPLD4                                (0x04)

#define PORT_LED_OPMOD_REG                    (0x09)
#define  OP_MOD_MASK                          (0x01)

#define PORT_LED_TEST_REG                     (0x0A)
#define  PORTS_AMBER_MASK                     (0x0C)
#define  PORTS_GREEN_MASK                     (0x03)

/*
 * CPLD2 register definitions
 */
#define CPLD2_SFP28_TXDISABLE_CTRL0_REG       (0x10 | CPLD2_IDX)
#define CPLD2_SFP28_TXDISABLE_CTRL1_REG       (0x11 | CPLD2_IDX)
#define CPLD2_SFP28_TXDISABLE_CTRL2_REG       (0x12 | CPLD2_IDX)

#define CPLD2_SFP28_RS_CTRL0_REG              (0x13 | CPLD2_IDX)
#define CPLD2_SFP28_RS_CTRL1_REG              (0x14 | CPLD2_IDX)
#define CPLD2_SFP28_RS_CTRL2_REG              (0x15 | CPLD2_IDX)

#define CPLD2_SFP28_RXLOS_STA0_REG            (0x16 | CPLD2_IDX)
#define CPLD2_SFP28_RXLOS_STA1_REG            (0x17 | CPLD2_IDX)
#define CPLD2_SFP28_RXLOS_STA2_REG            (0x18 | CPLD2_IDX)

#define CPLD2_SFP28_TXFAULT_STA0_REG          (0x19 | CPLD2_IDX)
#define CPLD2_SFP28_TXFAULT_STA1_REG          (0x1A | CPLD2_IDX)
#define CPLD2_SFP28_TXFAULT_STA2_REG          (0x1B | CPLD2_IDX)

#define CPLD2_SFP28_ABS_STA0_REG              (0x1C | CPLD2_IDX)
#define CPLD2_SFP28_ABS_STA1_REG              (0x1D | CPLD2_IDX)
#define CPLD2_SFP28_ABS_STA2_REG              (0x1E | CPLD2_IDX)

/*
 * CPLD3 register definitions
 */
#define CPLD3_SFP28_TXDISABLE_CTRL0_REG       (0x10 | CPLD3_IDX)
#define CPLD3_SFP28_TXDISABLE_CTRL1_REG       (0x11 | CPLD3_IDX)
#define CPLD3_SFP28_TXDISABLE_CTRL2_REG       (0x12 | CPLD3_IDX)

#define CPLD3_SFP28_RS_CTRL0_REG              (0x13 | CPLD3_IDX)
#define CPLD3_SFP28_RS_CTRL1_REG              (0x14 | CPLD3_IDX)
#define CPLD3_SFP28_RS_CTRL2_REG              (0x15 | CPLD3_IDX)

#define CPLD3_SFP28_RXLOS_STA0_REG            (0x16 | CPLD3_IDX)
#define CPLD3_SFP28_RXLOS_STA1_REG            (0x17 | CPLD3_IDX)
#define CPLD3_SFP28_RXLOS_STA2_REG            (0x18 | CPLD3_IDX)

#define CPLD3_SFP28_TXFAULT_STA0_REG          (0x19 | CPLD3_IDX)
#define CPLD3_SFP28_TXFAULT_STA1_REG          (0x1A | CPLD3_IDX)
#define CPLD3_SFP28_TXFAULT_STA2_REG          (0x1B | CPLD3_IDX)

#define CPLD3_SFP28_ABS_STA0_REG              (0x1C | CPLD3_IDX)
#define CPLD3_SFP28_ABS_STA1_REG              (0x1D | CPLD3_IDX)
#define CPLD3_SFP28_ABS_STA2_REG              (0x1E | CPLD3_IDX)

/*
 * CPLD4 register definitions
 */
#define CPLD4_SFP28_TXDISABLE_CTRL0_REG       (0x10 | CPLD4_IDX)

#define CPLD4_SFP28_RS_CTRL0_REG              (0x13 | CPLD4_IDX)

#define CPLD4_SFP28_RXLOS_STA0_REG            (0x16 | CPLD4_IDX)

#define CPLD4_SFP28_TXFAULT_STA0_REG          (0x19 | CPLD4_IDX)

#define CPLD4_SFP28_ABS_STA0_REG              (0x1C | CPLD4_IDX)

#define CPLD4_QSFP28_RESET_CTRL0_REG          (0x30 | CPLD4_IDX)
#define CPLD4_QSFP28_LPMOD_CTRL0_REG          (0x31 | CPLD4_IDX)
#define CPLD4_QSFP28_INT_STA0_REG             (0x32 | CPLD4_IDX)
#define CPLD4_QSFP28_ABS_STA0_REG             (0x33 | CPLD4_IDX)

#endif  /* DELL_S4248FBL_CPLD_H__ */
