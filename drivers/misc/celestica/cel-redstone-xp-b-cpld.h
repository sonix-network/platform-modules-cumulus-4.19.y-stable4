/*
 * Celestica RedstoneXP-B CPLD Platform Definitions
 *
 * Alan Liebthal <alanl@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef CEL_REDSTONE_XP_B_H__
#define CEL_REDSTONE_XP_B_H__

#define CPLD_IO_BASE 0xA200
#define CPLD_IO_SIZE 0x2ff

enum {
	CL_I2C_I801_BUS = 0,
	CL_I2C_MUX1_BUS0 = 10,
	CL_I2C_MUX1_BUS1,
	CL_I2C_MUX1_BUS2,
	CL_I2C_MUX1_BUS3,
	CL_I2C_MUX1_BUS4,
	CL_I2C_MUX1_BUS5,
	CL_I2C_MUX1_BUS6,
	CL_I2C_MUX1_BUS7,
	CL_I2C_CPLD_MUX_1 = 20,
	CL_I2C_CPLD_MUX_2,
	CL_I2C_CPLD_MUX_3,
	CL_I2C_CPLD_MUX_4,
	CL_I2C_CPLD_MUX_FIRST_PORT = 30,
};

#define CEL_REDSTONE_XP_B_NUM_PORTS  (54)

/*-----------------------------------------------------------------------------
 *
 * register info from:
 *
 * R1089-M0021-01_Rev01_Redstone-XP_B_CPLD_Design_Specification_20161222
 *
 *  CPLD2
 */
#define CPLD2_REG_VERSION                    (0xA200)
  #define CPLD_ID_MASK                       (0xF0)
  #define CPLD_ID_SHIFT                      (4)
  #define CPLD_VERSION_MASK                  (0x0F)

#define CPLD2_REG_SCRATCH                    (0xA201)

#define CPLD2_REG_I2C_FREQ_DIV               (0xA210)

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

#define CPLD2_REG_SFP_1_8_RX_LOS_L           (0xA240)
#define CPLD2_REG_SFP_9_16_RX_LOS_L          (0xA241)
#define CPLD2_REG_SFP_17_18_RX_LOS_L         (0xA242)

#define CPLD2_REG_SFP_1_8_TX_DIS             (0xA250)
#define CPLD2_REG_SFP_9_16_TX_DIS            (0xA251)
#define CPLD2_REG_SFP_17_18_TX_DIS           (0xA252)

#define CPLD2_REG_SFP_1_8_RS                 (0xA253)
#define CPLD2_REG_SFP_9_16_RS                (0xA254)
#define CPLD2_REG_SFP_17_18_RS               (0xA255)

#define CPLD2_REG_SFP_1_8_TX_FAULT           (0xA256)
#define CPLD2_REG_SFP_9_16_TX_FAULT          (0xA257)
#define CPLD2_REG_SFP_17_18_TX_FAULT         (0xA258)

#define CPLD2_REG_SFP_1_8_PRES_L             (0xA259)
#define CPLD2_REG_SFP_9_16_PRES_L            (0xA25A)
#define CPLD2_REG_SFP_17_18_PRES_L           (0xA25B)

/*
 *  CPLD3
 */
#define CPLD3_REG_VERSION                    (0xA280)

#define CPLD3_REG_SCRATCH                    (0xA281)

#define CPLD3_REG_I2C_FREQ_DIV               (0xA290)

#define CPLD3_REG_I2C_CTL                    (0xA291)

#define CPLD3_REG_I2C_STATUS                 (0xA292)

#define CPLD3_REG_I2C_DATA                   (0xA293)

#define CPLD3_REG_I2C_PORT_ID                (0xA294)

#define CPLD3_REG_SFP_19_26_RX_LOS_L         (0xA2C0)
#define CPLD3_REG_SFP_27_34_RX_LOS_L         (0xA2C1)
#define CPLD3_REG_SFP_35_36_RX_LOS_L         (0xA2C2)

#define CPLD3_REG_SFP_19_26_TX_DIS           (0xA2D0)
#define CPLD3_REG_SFP_27_34_TX_DIS           (0xA2D1)
#define CPLD3_REG_SFP_35_36_TX_DIS           (0xA2D2)

#define CPLD3_REG_SFP_19_26_RS               (0xA2D3)
#define CPLD3_REG_SFP_27_34_RS               (0xA2D4)
#define CPLD3_REG_SFP_35_36_RS               (0xA2D5)

#define CPLD3_REG_SFP_19_26_TX_FAULT         (0xA2D6)
#define CPLD3_REG_SFP_27_34_TX_FAULT         (0xA2D7)
#define CPLD3_REG_SFP_35_36_TX_FAULT         (0xA2D8)

#define CPLD3_REG_SFP_19_26_PRES_L           (0xA2D9)
#define CPLD3_REG_SFP_27_34_PRES_L           (0xA2DA)
#define CPLD3_REG_SFP_35_36_PRES_L           (0xA2DB)

/*
 *  CPLD4
 */
#define CPLD4_REG_VERSION                    (0xA300)

#define CPLD4_REG_SCRATCH                    (0xA301)

#define CPLD4_REG__RESET_CTL                 (0xA302)
  #define CPLD_RESET_BMC56850                BIT(3)

#define CPLD4_REG_MISC_CTL                   (0xA304)
  #define CPLD_EEPROM_WP                     BIT(3)

#define CPLD4_REG_I2C_FREQ_DIV               (0xA310)

#define CPLD4_REG_I2C_CTL                    (0xA311)

#define CPLD4_REG_I2C_STATUS                 (0xA312)

#define CPLD4_REG_I2C_DATA                   (0xA313)

#define CPLD4_REG_I2C_PORT_ID                (0xA314)

#define CPLD4_REG_SFP28_REV_ID               (0xA340)

#define CPLD4_REG_QSFP_1_6_RESET_L           (0xA360)
  #define CPLD4_QSFP_MASK                    (0x3f)
#define CPLD4_REG_QSFP_1_6_LP_MOD            (0xA361)
#define CPLD4_REG_QSFP_1_6_PRESENT_L         (0xA362)
#define CPLD4_REG_QSFP_1_6_I2C_READY         (0xA364)
#define CPLD4_REG_QSFP_1_6_PWR_GOOD          (0xA365)

#define CPLD4_REG_QSFP_1_6_SWITCH_TYPE       (0xA370)
  #define REDXP_B_TYPE                       (8)
  #define SMALLXP_B_TYPE                     (7)
  #define RDP_TYPE                           (6)
  #define MIDSTONE_TYPE                      (5)
  #define QUESSTONE_TYPE                     (4)
  #define SEASTONE_TYPE                      (3)
  #define SMALLXP_TYPE                       (2)
  #define REDXP_TYPE                         (1)

/*
 *  CPLD5
 */
#define CPLD5_REG_VERSION                    (0xA380)

#define CPLD5_REG_SCRATCH                    (0xA381)

#define CPLD5_REG_I2C_FREQ_DIV               (0xA390)

#define CPLD5_REG_I2C_CTL                    (0xA391)

#define CPLD5_REG_I2C_STATUS                 (0xA392)

#define CPLD5_REG_I2C_DATA                   (0xA393)

#define CPLD5_REG_I2C_PORT_ID                (0xA394)

#define CPLD5_REG_SFP_37_44_RX_LOS_L         (0xA3C0)
#define CPLD5_REG_SFP_45_48_RX_LOS_L         (0xA3C1)

#define CPLD5_REG_SFP_37_44_TX_DIS           (0xA3D0)
#define CPLD5_REG_SFP_45_48_TX_DIS           (0xA3D1)

#define CPLD5_REG_SFP_37_44_RS               (0xA3D2)
#define CPLD5_REG_SFP_45_48_RS               (0xA3D3)

#define CPLD5_REG_SFP_37_44_TX_FAULT         (0xA3D4)
#define CPLD5_REG_SFP_45_48_TX_FAULT         (0xA3D5)

#define CPLD5_REG_SFP_37_44_PRES_L           (0xA3D6)
#define CPLD5_REG_SFP_45_48_PRES_L           (0xA3D7)

#endif /* CEL_REDSTONE_XP_B_H__ */
