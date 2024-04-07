/*
 * Quanta IX Rangeley CPLD Platform Definitions
 *
 * Copyright (c) 2015 Cumulus Networks, Inc.  All rights reserved.
 *
 * Vidya Sagar Ravipati <vidya@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 */

#ifndef QUANTA_IX_CPLD_H__
#define QUANTA_IX_CPLD_H__

#define QUANATA_IX_CPLD_STRING_NAME_SIZE 30

#define IX1_PLATFORM_NAME "IX1"
#define IX2_PLATFORM_NAME "IX2"
#define QUANTA_IX_REG_MAX		8

/*
 * CPLD Indexes for Quanta IX Rangeley platforms
 */
/* CPLD1- LED function of QSFP28 1-16 */
#define IX1_LED_QSFP28_1_16_CPLD_ID	(0x01)
/* CPLD2-IO expander of QSFP28 1-16 */
#define IX1_IO_QSFP28_1_16_CPLD_ID	(0x02)
/* CPLD3- LED function of QSFP28 17-32 */
#define IX1_LED_QSFP28_17_32_CPLD_ID	(0x03)
/* CPLD4-IO expander of QSFP28 17-32 */
#define IX1_IO_QSFP28_17_32_CPLD_ID	(0x04)
/* CPLD5- I2C Bus monitor function for QSFP28 1-32 */
#define IX1_I2C_BUS_MNTR_CPLD_ID	(0x05)
/*
 * IX2 CPLD(s)
 */
/* CPLD1-IO expander of SFP28 33-48 */
#define IX2_IO_SFP28_33_48_CPLD_ID	(0x06)
/* CPLD2-IO expander of SFP28 1-16 */
#define IX2_IO_SFP28_1_16_CPLD_ID	(0x07)
/* CPLD3- LED function of SFP28 & QSFP28 */
#define IX2_LED_1_52_CPLD_ID		(0x08)
/* CPLD4-IO expander of SFP28 1-16 */
#define IX2_IO_SFP28_17_32_CPLD_ID	(0x09)
/* CPLD5- I2C monitor function of PCA9548_1-7 */
#define IX2_I2C_BUS_MNTR_CPLD_ID	(0x0A)
/* CPLD6- UART Switch */
#define IX2_UART_SWITCH_CPLD_ID		(0x0B)


/*
 * Begin register defines.
 */


/* The Quanta IX1 has two I/O CPLDs:  PORT0 and PORT1.
 *
 * PORT0  -- QSFP28 I/Os for ports 1-16
 * PORT1  -- QSFP28 I/Os for ports 17-32
 *
 */

#define NUM_QSFP_IO_GROUPS	4
#define NUM_QSFPS_PER_IO_GROUP	4
/*
 * Port0 CPLD register definitions -- ports 1 to 16
 */
#define IX1_IO_QSFP_1_4_INFO_REG       (0x01)
#define IX1_IO_QSFP_5_8_INFO_REG       (0x02)
#define IX1_IO_QSFP_9_12_INFO_REG      (0x03)
#define IX1_IO_QSFP_13_16_INFO_REG     (0x04)

/*
 * CPLD5/port monitor register definitions
 */
#define IX1_MNTR_ENABLE_REG            (0x04)
#define IX1_MNTR_RESET_TIME_LO_REG     (0x05)
#define IX1_MNTR_RESET_TIME_HI_REG     (0x06)

/* The Quanta IX1 has two LED CPLDs:  PORT0 and PORT1.
 *
 * PORT0  -- QSFP28 LEDs for ports 1-16
 * PORT1  -- QSFP28 LEDs for ports 17-32
 *
 */
#define QUANTA_IX_LED_BIT_MAX		8

#define IX1_LED_100G_1_8_INFO_REG		(0x00)
#define IX1_LED_100G_9_16_INFO_REG		(0x01)
#define IX1_LED_40G_1_8_INFO_REG		(0x02)
#define IX1_LED_40G_9_16_INFO_REG		(0x03)
#define IX1_LED_25G_1_8_INFO_REG		(0x04)
#define IX1_LED_25G_9_16_INFO_REG		(0x05)
#define IX1_LED_10G_1_8_INFO_REG		(0x06)
#define IX1_LED_10G_9_16_INFO_REG		(0x07)
#define IX1_LED_50G_1_4_INFO_REG		(0x08)
#define IX1_LED_50G_5_8_INFO_REG		(0x09)
#define IX1_LED_50G_9_12_INFO_REG		(0x0A)
#define IX1_LED_50G_13_16_INFO_REG		(0x0B)

#define IX2_LED_100G_49_56_INFO_REG		(0x00)
#define IX2_LED_40G_49_56_INFO_REG		(0x01)
#define IX2_LED_25G_1_8_INFO_REG		(0x02)
#define IX2_LED_25G_9_16_INFO_REG		(0x03)
#define IX2_LED_25G_17_24_INFO_REG		(0x04)
#define IX2_LED_25G_25_32_INFO_REG		(0x05)
#define IX2_LED_25G_33_40_INFO_REG		(0x06)
#define IX2_LED_25G_41_48_INFO_REG		(0x07)
#define IX2_LED_25G_49_56_INFO_REG		(0x08)
#define IX2_LED_10G_1_8_INFO_REG		(0x09)
#define IX2_LED_10G_9_16_INFO_REG		(0x0A)
#define IX2_LED_10G_17_24_INFO_REG		(0x0B)
#define IX2_LED_10G_25_32_INFO_REG		(0x0C)
#define IX2_LED_10G_33_40_INFO_REG		(0x0D)
#define IX2_LED_10G_41_48_INFO_REG		(0x0E)
#define IX2_LED_10G_49_56_INFO_REG		(0x0F)
#define IX2_LED_50G_49_52_INFO_REG		(0x10)
#define IX2_LED_50G_53_56_INFO_REG		(0x11)
#define IX2_LED_10G_1_8_BLINK_REG		(0x12)
#define IX2_LED_10G_9_16_BLINK_REG		(0x13)
#define IX2_LED_10G_17_24_BLINK_REG		(0x14)
#define IX2_LED_10G_25_32_BLINK_REG		(0x15)
#define IX2_LED_10G_33_40_BLINK_REG		(0x16)
#define IX2_LED_10G_41_48_BLINK_REG		(0x17)
#define IX2_LED_10G_49_56_BLINK_REG		(0x18)

/* The Quanta IX2 has three I/O CPLDs:  PORT0, PORT1 and PORT2.
 *
 * PORT0  -- SFP28 I/Os for ports 1-16
 * PORT1  -- SFP28 I/Os for ports 17-32
 * PORT2  -- SFP28 I/Os for ports 33-48
 *
 */

#define NUM_SFP_IO_GROUPS	4
#define NUM_SFPS_PER_IO_GROUP	4
/*
 * Port0 CPLD register definitions -- ports 1 to 16
 */
#define IX2_IO_SFP_1_4_INFO_REG       (0x01)
#define IX2_IO_SFP_5_8_INFO_REG       (0x02)
#define IX2_IO_SFP_9_12_INFO_REG      (0x03)
#define IX2_IO_SFP_13_16_INFO_REG     (0x04)
/*
 * End register defines.
 */

#endif
