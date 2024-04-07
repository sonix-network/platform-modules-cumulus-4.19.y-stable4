/*
 * Quanta LY7 Platform Definitions
 *
 * Copyright (C) 2017, 2018, 2019, 2020 Cumulus Networks, Inc.  All Rights Reserved
 * David Yen <dhyen@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 */

#ifndef QUANTA_LY7_H__
#define QUANTA_LY7_H__

#define LY7_PORT_COUNT	    52

#define LY7_GPIO_BASE       100
#define LY7_GPIO_23_BASE    LY7_GPIO_BASE
#define LY7_GPIO_21_BASE    (LY7_GPIO_23_BASE + PCA_9555_GPIO_COUNT)

#define QUANTA_LY7_CPLD_STRING_NAME_SIZE 30

#define LY7_PLATFORM_NAME                "LY7"
#define QUANTA_LY7_REG_MAX               8

/* LY7 CPLDs */

#define LY7_IO_SFP28_1_16_CPLD_ID        (0x0C)
#define LY7_IO_SFP28_17_32_CPLD_ID       (0x0D)
#define LY7_IO_SFP28_33_48_CPLD_ID       (0x0E)

#define LY7_LED_CPLD_ID                  (0x0F)

#define LY7_IO_SFP_1_4_INFO_REG          (0x01)
#define LY7_IO_SFP_5_8_INFO_REG          (0x02)
#define LY7_IO_SFP_9_12_INFO_REG         (0x03)
#define LY7_IO_SFP_13_16_INFO_REG        (0x04)

#define LY7_LED_DECODER_REG              (0x04)

/*
 * Structure definitions
 */
struct quanta_ly7_platform_data {
	int idx;
};

#endif
