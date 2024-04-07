/*
 * Celestica Smallstone XP-B CPLD Platform Definitions
 *
 * Alan Liebthal <alanl@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef CEL_SMALLSTONE_XP_B_H__
#define CEL_SMALLSTONE_XP_B_H__

#define CPLD_IO_BASE 0xA200
#define CPLD_IO_SIZE 0x2ff

enum {
	CL_I2C_I801_BUS=0,
	CL_I2C_MUX1_BUS0=10,
	CL_I2C_MUX1_BUS1,
	CL_I2C_MUX1_BUS2,
	CL_I2C_MUX1_BUS3,
	CL_I2C_MUX1_BUS4,
	CL_I2C_MUX1_BUS5,
	CL_I2C_MUX1_BUS6,
	CL_I2C_MUX1_BUS7,
	CL_I2C_CPLD_MUX_1=20,
	CL_I2C_CPLD_MUX_2,
	CL_I2C_CPLD_MUX_FIRST_PORT=30,
};

#define CEL_SMALLSTONE_XP_B_NUM_PORTS  (32)

/*----------------------------------------------------------------------------------
 *
 * register info from R1089-M0020-01_Rev0.1_Smallstone-XP_B_CPLD_Design_Specification_20161229
 *
 *  CPLD2
 */
#define CPLD2_REG_VERSION                    (0xA200)
  #define CPLD_ID_MASK                       (0xF0)
  #define CPLD_ID_SHIFT                      (4)
  #define CPLD_VERSION_MASK                  (0x0F)

#define CPLD2_REG_SCRATCH                    (0xA201)

#define CPLD2_REG_I2C_FREQ_DIV               (0xA210)
  #define CPLD_I2C_50KHZ                     (63)
  #define CPLD_I2C_100KHZ                    (31)
  #define CPLD_I2C_200KHZ                    (15)
  #define CPLD_I2C_400KHZ                    (7)

#define CPLD2_REG_I2C_CTL                    (0xA211)
  #define CPLD_CTL_MEN                       BIT(7)
  #define CPLD_MODULE_INT_ENABLE             BIT(6)
  #define CPLD_CTL_MSTA                      BIT(5)
  #define CPLD_CTL_MTX                       BIT(4)
  #define CPLD_MODULE_TX_ACK                 BIT(3)
  #define CPLD_MODULE_RSTA                   BIT(2)

#define CPLD2_REG_I2C_STATUS                 (0xA212)
  #define CPLD_STATUS_MCF                    BIT(7)
  #define CPLD_STATUS_MIF                    BIT(1)
  #define CPLD_STATUS_RXAK                   BIT(0)

#define CPLD2_REG_I2C_DATA                   (0xA213)

#define CPLD2_REG_I2C_PORT_ID                (0xA214)

#define CPLD2_REG_QSFP_1_8_RESET_L           (0xA250)
#define CPLD2_REG_QSFP_9_16_RESET_L          (0xA251)

#define CPLD2_REG_QSFP_1_8_LP_MOD            (0xA252)
#define CPLD2_REG_QSFP_9_16_LP_MOD           (0xA253)

#define CPLD2_REG_QSFP_1_8_PRESENT_L         (0xA254)
#define CPLD2_REG_QSFP_9_16_PRESENT_L        (0xA255)

#define CPLD2_REG_QSFP_1_8_I2C_READY         (0xA258)
#define CPLD2_REG_QSFP_9_16_I2C_READY        (0xA259)

/*
 *  CPLD3
 */
#define CPLD3_REG_VERSION                    (0xA280)
  #define CPLD_ID_MASK                       (0xF0)
  #define CPLD_ID_SHIFT                      (4)
  #define CPLD_VERSION_MASK                  (0x0F)

#define CPLD3_REG_SCRATCH                    (0xA281)

#define CPLD3_REG_I2C_FREQ_DIV               (0xA290)
  #define CPLD_I2C_50KHZ                     (63)
  #define CPLD_I2C_100KHZ                    (31)
  #define CPLD_I2C_200KHZ                    (15)
  #define CPLD_I2C_400KHZ                    (7)

#define CPLD3_REG_I2C_CTL                    (0xA291)
  #define CPLD_CTL_MEN                       BIT(7)
  #define CPLD_MODULE_INT_ENABLE             BIT(6)
  #define CPLD_CTL_MSTA                      BIT(5)
  #define CPLD_CTL_MTX                       BIT(4)
  #define CPLD_MODULE_TX_ACK                 BIT(3)
  #define CPLD_MODULE_RSTA                   BIT(2)

#define CPLD3_REG_I2C_STATUS                 (0xA292)
  #define CPLD_STATUS_MCF                    BIT(7)
  #define CPLD_STATUS_MIF                    BIT(1)
  #define CPLD_STATUS_RXAK                   BIT(0)

#define CPLD3_REG_I2C_DATA                   (0xA293)

#define CPLD3_REG_I2C_PORT_ID                (0xA294)

#define CPLD3_REG_QSFP_17_24_RESET_L         (0xA2D0)
#define CPLD3_REG_QSFP_25_32_RESET_L         (0xA2D1)

#define CPLD3_REG_QSFP_17_24_LP_MOD          (0xA2D2)
#define CPLD3_REG_QSFP_25_32_LP_MOD          (0xA2D3)

#define CPLD3_REG_QSFP_17_24_PRESENT_L       (0xA2D4)
#define CPLD3_REG_QSFP_25_32_PRESENT_L       (0xA2D5)

#define CPLD3_REG_QSFP_17_24_I2C_READY       (0xA2D8)
#define CPLD3_REG_QSFP_25_32_I2C_READY       (0xA2D9)

/*
 *  CPLD4
 */
#define CPLD4_REG_VERSION                    (0xA300)
  #define CPLD_ID_MASK                       (0xF0)
  #define CPLD_ID_SHIFT                      (4)
  #define CPLD_VERSION_MASK                  (0x0F)

#define CPLD4_REG_SCRATCH                    (0xA301)

#define CPLD4_REG_MISC_CTL                   (0xA304)
  #define CPLD_EEPROM_WP                     BIT(3)

/*
 *  CPLD5
 */
#define CPLD5_REG_VERSION                    (0xA380)

#define CPLD5_REG_SCRATCH                    (0xA381)


#endif // CEL_SMALLSTONE_XP_B_H__
