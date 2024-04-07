/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Celestica Pebble BMC E1052 Platform Definitions
 *
 * Vidya Ravipati <vidya@cumulusnetworks.com>
 *
 */

#ifndef CEL_PEBBLE_B_PLATFORM_H__
#define CEL_PEBBLE_B_PLATFORM_H__

#define CEL_PEBBLE_B_NUM_PORTS          (52)

#define CEL_PEBBLE_B_GPIO_1_BASE              (472)
#define CEL_PEBBLE_B_GPIO_1_PIN_COUNT         (40)
#define CEL_PEBBLE_B_GPIO_SFP_1_RS            (0)
#define CEL_PEBBLE_B_GPIO_SFP_1_RX_LOS        (1)
#define CEL_PEBBLE_B_GPIO_SFP_1_PRESENT_L     (2)
#define CEL_PEBBLE_B_GPIO_SFP_1_TX_DISABLE    (3)
#define CEL_PEBBLE_B_GPIO_SFP_1_TX_FAULT      (4)
#define CEL_PEBBLE_B_GPIO_SFP_2_RS            (8)
#define CEL_PEBBLE_B_GPIO_SFP_2_RX_LOS        (9)
#define CEL_PEBBLE_B_GPIO_SFP_2_PRESENT_L     (10)
#define CEL_PEBBLE_B_GPIO_SFP_2_TX_DISABLE    (11)
#define CEL_PEBBLE_B_GPIO_SFP_2_TX_FAULT      (12)
#define CEL_PEBBLE_B_GPIO_SFP_3_RS            (16)
#define CEL_PEBBLE_B_GPIO_SFP_3_RX_LOS        (17)
#define CEL_PEBBLE_B_GPIO_SFP_3_PRESENT_L     (18)
#define CEL_PEBBLE_B_GPIO_SFP_3_TX_DISABLE    (19)
#define CEL_PEBBLE_B_GPIO_SFP_3_TX_FAULT      (20)
#define CEL_PEBBLE_B_GPIO_SFP_4_RS            (24)
#define CEL_PEBBLE_B_GPIO_SFP_4_RX_LOS        (25)
#define CEL_PEBBLE_B_GPIO_SFP_4_PRESENT_L     (26)
#define CEL_PEBBLE_B_GPIO_SFP_4_TX_DISABLE    (27)
#define CEL_PEBBLE_B_GPIO_SFP_4_TX_FAULT      (28)
#define CEL_PEBBLE_B_GPIO_RTC_INT_L           (33)
#define CEL_PEBBLE_B_GPIO_LM75B1_OS           (34)
#define CEL_PEBBLE_B_GPIO_LM75B2_OS           (35)
#define CEL_PEBBLE_B_GPIO_B50282_1_INT_L      (36)
#define CEL_PEBBLE_B_GPIO_B50282_2_INT_L      (37)
#define CEL_PEBBLE_B_GPIO_PCIE_INT_L          (39)

/*
 * This platform has one i2c busses:$
 *  SMBus_0: SMBus I801 adapter at PCIe address 0000:00:1f.3$
 *  A 9546 connects to that, and the 9546 port 2 connects to the 9548
 */
/* i2c bus adapter numbers for the down stream i2c busses */
enum {
	CEL_PEBBLE_B_I2C_I801_BUS = 0,
	CEL_PEBBLE_B_I2C_PCA9546_BUS_0 = 10,
	CEL_PEBBLE_B_I2C_PCA9546_BUS_1,
	CEL_PEBBLE_B_I2C_PCA9546_BUS_2,
	CEL_PEBBLE_B_I2C_PCA9546_BUS_3,
	CEL_PEBBLE_B_I2C_PCA9548_BUS_0 = 20,
	CEL_PEBBLE_B_I2C_PCA9548_BUS_1,
	CEL_PEBBLE_B_I2C_PCA9548_BUS_2,
	CEL_PEBBLE_B_I2C_PCA9548_BUS_3,
	CEL_PEBBLE_B_I2C_PCA9548_BUS_4,
	CEL_PEBBLE_B_I2C_PCA9548_BUS_5,
	CEL_PEBBLE_B_I2C_PCA9548_BUS_6,
};

#endif /* CEL_PEBBLE_B_PLATFORM_H__ */
