/*
 * Celestica Pebble Platform Definitions
 *
 * Vidya Ravipati <vidya@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef CEL_PEBBLE_PLATFORM_H__
#define CEL_PEBBLE_PLATFORM_H__

#define CEL_PEBBLE_NUM_PORTS          (52)

#define CEL_PEBBLE_GPIO_1_BASE          (472)
#define CEL_PEBBLE_GPIO_1_PIN_COUNT     (40)
#define CEL_PEBBLE_GPIO_SFP_1_RS            (CEL_PEBBLE_GPIO_1_BASE+0)
#define CEL_PEBBLE_GPIO_SFP_1_RX_LOS        (CEL_PEBBLE_GPIO_1_BASE+1)
#define CEL_PEBBLE_GPIO_SFP_1_PRESENT_L     (CEL_PEBBLE_GPIO_1_BASE+2)
#define CEL_PEBBLE_GPIO_SFP_1_TX_DISABLE    (CEL_PEBBLE_GPIO_1_BASE+3)
#define CEL_PEBBLE_GPIO_SFP_1_TX_FAULT      (CEL_PEBBLE_GPIO_1_BASE+4)
#define CEL_PEBBLE_GPIO_SFP_2_RS            (CEL_PEBBLE_GPIO_1_BASE+8)
#define CEL_PEBBLE_GPIO_SFP_2_RX_LOS        (CEL_PEBBLE_GPIO_1_BASE+9)
#define CEL_PEBBLE_GPIO_SFP_2_PRESENT_L     (CEL_PEBBLE_GPIO_1_BASE+10)
#define CEL_PEBBLE_GPIO_SFP_2_TX_DISABLE    (CEL_PEBBLE_GPIO_1_BASE+11)
#define CEL_PEBBLE_GPIO_SFP_2_TX_FAULT      (CEL_PEBBLE_GPIO_1_BASE+12)
#define CEL_PEBBLE_GPIO_SFP_3_RS            (CEL_PEBBLE_GPIO_1_BASE+16)
#define CEL_PEBBLE_GPIO_SFP_3_RX_LOS        (CEL_PEBBLE_GPIO_1_BASE+17)
#define CEL_PEBBLE_GPIO_SFP_3_PRESENT_L     (CEL_PEBBLE_GPIO_1_BASE+18)
#define CEL_PEBBLE_GPIO_SFP_3_TX_DISABLE    (CEL_PEBBLE_GPIO_1_BASE+19)
#define CEL_PEBBLE_GPIO_SFP_3_TX_FAULT      (CEL_PEBBLE_GPIO_1_BASE+20)
#define CEL_PEBBLE_GPIO_SFP_4_RS            (CEL_PEBBLE_GPIO_1_BASE+24)
#define CEL_PEBBLE_GPIO_SFP_4_RX_LOS        (CEL_PEBBLE_GPIO_1_BASE+25)
#define CEL_PEBBLE_GPIO_SFP_4_PRESENT_L     (CEL_PEBBLE_GPIO_1_BASE+26)
#define CEL_PEBBLE_GPIO_SFP_4_TX_DISABLE    (CEL_PEBBLE_GPIO_1_BASE+27)
#define CEL_PEBBLE_GPIO_SFP_4_TX_FAULT      (CEL_PEBBLE_GPIO_1_BASE+28)
#define CEL_PEBBLE_GPIO_RTC_INT_L           (CEL_PEBBLE_GPIO_1_BASE+33)
#define CEL_PEBBLE_GPIO_LM75B1_OS           (CEL_PEBBLE_GPIO_1_BASE+34)
#define CEL_PEBBLE_GPIO_LM75B2_OS           (CEL_PEBBLE_GPIO_1_BASE+35)
#define CEL_PEBBLE_GPIO_B50282_1_INT_L      (CEL_PEBBLE_GPIO_1_BASE+36)
#define CEL_PEBBLE_GPIO_B50282_2_INT_L      (CEL_PEBBLE_GPIO_1_BASE+37)
#define CEL_PEBBLE_GPIO_PCIE_INT_L          (CEL_PEBBLE_GPIO_1_BASE+39)

/*
 * This platform has one i2c busses:$
 *  SMBus_0: SMBus I801 adapter at PCIe address 0000:00:1f.3$
 */
/* i2c bus adapter numbers for the down stream i2c busses */
enum {
	CEL_PEBBLE_I2C_I801_BUS=0,
    CEL_PEBBLE_I2C_PCA9548_BUS_0=10,
    CEL_PEBBLE_I2C_PCA9548_BUS_1,
    CEL_PEBBLE_I2C_PCA9548_BUS_2,
    CEL_PEBBLE_I2C_PCA9548_BUS_3,
    CEL_PEBBLE_I2C_PCA9548_BUS_4,
    CEL_PEBBLE_I2C_PCA9548_BUS_5,
    CEL_PEBBLE_I2C_PCA9548_BUS_6,
    CEL_PEBBLE_I2C_PCA9548_BUS_7,
};


#endif /* CEL_PEBBLE_PLATFORM_H__ */