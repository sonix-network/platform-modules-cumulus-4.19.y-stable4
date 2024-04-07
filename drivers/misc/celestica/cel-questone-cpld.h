/*
 * Celestica Questone CPLD Platform Definitions
 *
 * Copyright (c) 2017, 2018, 2019 Cumulus Networks, Inc.
 * All rights reserved.
 *
 * Alan Liebthal <alanl@cumulusnetworks.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef CEL_QUESTONE_H__
#define CEL_QUESTONE_H__

#define CEL_QUESTONE_NUM_PORTS      (56)
#define CEL_QUESTONE_NUM_SFP_PORTS  (48)
#define CEL_QUESTONE_NUM_QSFP_PORTS (6)
#define CPLD_IO_BASE                0xA100
#define CPLD_IO_SIZE                0x2df

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
	CL_I2C_MUX2_BUS0 = 20,
	CL_I2C_MUX2_BUS1,
	CL_I2C_MUX2_BUS2,
	CL_I2C_MUX2_BUS3,
	CL_I2C_MUX2_BUS4,
	CL_I2C_MUX2_BUS5,
	CL_I2C_MUX2_BUS6,
	CL_I2C_MUX2_BUS7,
	CL_I2C_CPLD_MUX_1 = 40,
	CL_I2C_CPLD_MUX_2,
	CL_I2C_CPLD_MUX_3,
	CL_I2C_CPLD_MUX_4,
	CL_I2C_CPLD_MUX_FIRST_PORT = 50,
};

/*-----------------------------------------------------------------------------
 *
 * register info from:
 *     R1032-M0024-01_Rev0.7_
 *       Questone_BaseBoard_CPLD_Design_Specification_20170105.pdf
 *
 */

/* Base board CPLD */

#define CPLD_BASE_REG_VERSION                    (0xA100)
#define   CPLDB_ID_MASK                          (0xf0)
#define   CPLDB_VERSION_MASK                     (0x0f)

#define CPLD_BASE_REG_SCRATCH                    (0xA101)

#define CPLD_BASE_REG_RESET_CTL                  (0xA102)
#define  CPLDB_RESET_BMC_L                       BIT(4)
#define  CPLDB_RESET_I210_L                      BIT(3)
#define  CPLDB_RESET_PCA9548_L                   BIT(2)
#define  CPLDB_RESET_PSOC_I2C_L                  BIT(1)
#define  CPLDB_RESET_SWITCH_BOARD_L              BIT(0)

#define CPLD_BASE_REG_CARD_PRESENT               (0xA105)
#define   CPLDB_SSD_ABSENT                       BIT(2)
#define   CPLDB_REAL_PANEL_CARD_ABSENT           BIT(1)
#define   CPLDB_BMC_CARD_ABSENT                  BIT(0)

#define CPLD_BASE_REG_SYS_LED                    (0xA106)
#define   CPLDB_SYS_LED_MASK                     (0x30)
#define   CPLDB_SYS_LED_AMBER                    (0x00)
#define   CPLDB_SYS_LED_GREEN                    (0x10)
#define   CPLDB_SYS_LED_YELLOW                   (0x20)
#define   CPLDB_SYS_LED_OFF                      (0x30)
#define   CPLDB_SYS_LED_BLINK_MASK               (0x03)
#define   CPLDB_SYS_LED_BLINK_OFF                (0x03)
#define   CPLDB_SYS_LED_BLINK_FAST               (0x02)
#define   CPLDB_SYS_LED_BLINK_SLOW               (0x01)
#define   CPLDB_SYS_LED_BLINK_ON                 (0x00)

#define CPLD_BASE_REG_BMC_CONSOLE_SWITCH         (0xA107)
#define   CPLDB_BCM_SEL_BMC                      BIT(0)

#define CPLD_BASE_REG_I2C_CTL                    (0xA108)
#define   CPLDB_I2C_SEL_L                        BIT(0)

#define CPLD_BASE_REG_PORT_ISR                   (0xA110)
#define   CPLDB_INT_BMC_L                        BIT(4)
#define   CPLDB_INT_LM75_L                       BIT(3)
#define   CPLDB_INT_SWITCH_L                     BIT(2)
#define   CPLDB_INT_OPTICAL_L                    BIT(1)
#define   CPLDB_INT_THERMAL_L                    BIT(0)

#define CPLD_BASE_REG_INT0_ISR                   (0xA111)
#define   CPLDB_INT_PSU_R_L                      BIT(3)
#define   CPLDB_INT_PSU_L_L                      BIT(2)
#define   CPLDB_INT_LM75_2_L                     BIT(1)
#define   CPLDB_INT_LM75_1_L                     BIT(0)

#define CPLD_BASE_REG_EEPROM_WP                  (0xA130)
 #define  CPLDB_WP_SYSTEM_EEPROM                 BIT(0)
 #define  CPLDB_WP_BMC_BIOS                      BIT(1)
 #define  CPLDB_WP_BOARD_EEPROM                  BIT(2)

#define CPLD_BASE_REG_FAN1_PWM                   (0xA140)
#define CPLD_BASE_REG_FAN1_CTL                   (0xA141)
#define   CPLDB_FAN_DIR_B2F                      BIT(3)
#define   CPLDB_FAN_ABSENT                       BIT(2)
#define   CPLDB_FAN_LED_GREEN                    BIT(1)
#define   CPLDB_FAN_LED_YELLOW                   BIT(0)

#define CPLD_BASE_REG_FAN1_REAR_SPEED            (0xA142)
#define CPLD_BASE_REG_FAN1_FRONT_SPEED           (0xA143)

#define CPLD_BASE_REG_FAN2_PWM                   (0xA144)
#define CPLD_BASE_REG_FAN2_CTL                   (0xA145)

#define CPLD_BASE_REG_FAN2_REAR_SPEED            (0xA146)
#define CPLD_BASE_REG_FAN2_FRONT_SPEED           (0xA147)

#define CPLD_BASE_REG_FAN3_PWM                   (0xA148)
#define CPLD_BASE_REG_FAN3_CTL                   (0xA149)

#define CPLD_BASE_REG_FAN3_REAR_SPEED            (0xA14A)
#define CPLD_BASE_REG_FAN3_FRONT_SPEED           (0xA14B)

#define CPLD_BASE_REG_FAN4_PWM                   (0xA14C)
#define CPLD_BASE_REG_FAN4_CTL                   (0xA14D)

#define CPLD_BASE_REG_FAN4_REAR_SPEED            (0xA14E)
#define CPLD_BASE_REG_FAN4_FRONT_SPEED           (0xA14F)

#define CPLD_BASE_REG_FAN5_PWM                   (0xA150)
#define CPLD_BASE_REG_FAN5_CTL                   (0xA151)

#define CPLD_BASE_REG_FAN5_REAR_SPEED            (0xA152)
#define CPLD_BASE_REG_FAN5_FRONT_SPEED           (0xA153)

#define CPLD_BASE_REG_PSU_STATUS                 (0xA160)
#define   CPLDB_PSUL_ALERT_L                     BIT(7)
#define   CPLDB_PSUR_ALERT_L                     BIT(6)
#define   CPLDB_PSUL_ABSENT                      BIT(5)
#define   CPLDB_PSUR_ABSENT                      BIT(4)
#define   CPLDB_PSUL_OK                          BIT(3)
#define   CPLDB_PSUR_OK                          BIT(2)
#define   CPLDB_PSUL_ENABLE                      BIT(1)
#define   CPLDB_PSUR_ENABLE                      BIT(0)

#define CPLD_BASE_REG_PSU_LED_CTL                (0xA161)
#define   CPLDB_PSUL_CTL_HW                      BIT(1)
#define   CPLDB_PSUR_CTL_HW                      BIT(0)

#define CPLD_BASE_REG_BMC_LED_CTL                (0xA165)
#define   CPLDB_BMC_AMBER_CTL_HW                 BIT(1)
#define   CPLDB_BMC_GREEN_CTL_HW                 BIT(0)

/* Switch card CPLD1 */

#define CPLD_SW1_REG_VERSION                     (0xA300)
#define   CPLD1_VERSION_MASK                     (0x0f)

#define CPLD_SW1_REG_SCRATCH                     (0xA301)

#define CPLD_SW1_REG_RESET_CTL1                  (0xA302)

#define CPLD_SW1_REG_RESET_CTL2                  (0xA303)

#define CPLD_SW1_REG_I2C_PORT_ID                 (0xA310)
  #define CPLD2_I2C_BAUD_50KHZ                   (0)
  #define CPLD2_I2C_BAUD_100KHZ                  BIT(6)
  #define CPLD2_I2C_BAUD_MASK                    BIT(6)
  #define CPLD2_I2C_PORT_ID_MASK                 (0x3f)

#define CPLD_SW1_REG_I2C_OP_CODE                 (0xA311)
  #define CPLD2_I2C_DATA_LEN_MASK                (0xf0)
  #define CPLD2_I2C_CMD_LEN_MASK                 (0x03)

#define CPLD_SW1_REG_DEV_ADDR                    (0xA312)
  #define CPLD2_I2C_DEV_ADDR_MASK                (0xfe)
  #define CPLD2_I2C_DEV_RW_MASK                  BIT(0)
  #define CPLD2_I2C_DEV_READ                     BIT(0)
  #define CPLD2_I2C_DEV_WRITE                    (0)

#define CPLD_SW1_REG_I2C_CMD_BYTE0               (0xA313)
#define CPLD_SW1_REG_I2C_CMD_BYTE1               (0xA314)
#define CPLD_SW1_REG_I2C_CMD_BYTE2               (0xA315)

#define CPLD_SW1_REG_I2C_STATUS                  (0xA316)
  #define CPLD_I2C_MASTER_ERROR                  BIT(7)
  #define CPLD_I2C_BUSY                          BIT(6)
  #define CPLD_I2C_RESET                         BIT(0)

#define CPLD_SW1_REG_I2C_WRITE_DATA_BYTE0        (0xA320)

#define CPLD_SW1_REG_I2C_READ_DATA_BYTE0         (0xA330)

#define CPLD_SW1_REG_GPIO_CONTROL                (0xA340)

#define CPLD_SW1_REG_GENERAL_STATUS              (0xA341)

#define CPLD_SW1_SPI_CS_CONTROL                  (0xA350)

#define CPLD_SW1_REG_ZQSFP_1_6_RESET             (0xA360)
  #define ZQSFP_6_RESET_L                        BIT(5)
  #define ZQSFP_5_RESET_L                        BIT(4)
  #define ZQSFP_4_RESET_L                        BIT(3)
  #define ZQSFP_3_RESET_L                        BIT(2)
  #define ZQSFP_2_RESET_L                        BIT(1)
  #define ZQSFP_1_RESET_L                        BIT(0)

#define CPLD_SW1_REG_ZQSFP_1_6_LPMOD             (0xA361)
  #define ZQSFP_6_LPMOD                          BIT(5)
  #define ZQSFP_5_LPMOD                          BIT(4)
  #define ZQSFP_4_LPMOD                          BIT(3)
  #define ZQSFP_3_LPMOD                          BIT(2)
  #define ZQSFP_2_LPMOD                          BIT(1)
  #define ZQSFP_1_LPMOD                          BIT(0)

#define CPLD_SW1_REG_ZQSFP_1_6_ABS               (0xA362)
  #define ZQSFP_6_ABS                            BIT(5)
  #define ZQSFP_5_ABS                            BIT(4)
  #define ZQSFP_4_ABS                            BIT(3)
  #define ZQSFP_3_ABS                            BIT(2)
  #define ZQSFP_2_ABS                            BIT(1)
  #define ZQSFP_1_ABS                            BIT(0)

#define CPLD_SW1_REG_ZQSFP_1_6_INT_N             (0xA363)
  #define ZQSFP_6_INT_L                          BIT(5)
  #define ZQSFP_5_INT_L                          BIT(4)
  #define ZQSFP_4_INT_L                          BIT(3)
  #define ZQSFP_3_INT_L                          BIT(2)
  #define ZQSFP_2_INT_L                          BIT(1)
  #define ZQSFP_1_INT_L                          BIT(0)

#define CPLD_SW1_REG_ZQSFP_1_6_I2C_READY         (0xA364)
  #define ZQSFP_6_I2C_READY                      BIT(5)
  #define ZQSFP_5_I2C_READY                      BIT(4)
  #define ZQSFP_4_I2C_READY                      BIT(3)
  #define ZQSFP_3_I2C_READY                      BIT(2)
  #define ZQSFP_2_I2C_READY                      BIT(1)
  #define ZQSFP_1_I2C_READY                      BIT(0)

#define CPLD_SW1_REG_ZQSFP_1_6_POWER_GOOD        (0xA365)
  #define ZQSFP_6_POWER_GOOD                     BIT(5)
  #define ZQSFP_5_POWER_GOOD                     BIT(4)
  #define ZQSFP_4_POWER_GOOD                     BIT(3)
  #define ZQSFP_3_POWER_GOOD                     BIT(2)
  #define ZQSFP_2_POWER_GOOD                     BIT(1)
  #define ZQSFP_1_POWER_GOOD                     BIT(0)

/* Switch card CPLD2 */

#define CPLD_SW2_REG_VERSION                     (0xA200)
  #define CPLD2_VERSION_MASK                     (0x0f)

#define CPLD_SW2_REG_SCRATCH                     (0xA201)

#define CPLD_SW2_REG_I2C_PORT_ID                 (0xA210)

#define CPLD_SW2_REG_I2C_OP_CODE                 (0xA211)

#define CPLD_SW2_REG_DEV_ADDR                    (0xA212)

#define CPLD_SW2_REG_CMD_BYTE0                   (0xA213)
#define CPLD_SW2_REG_CMD_BYTE1                   (0xA214)
#define CPLD_SW2_REG_CMD_BYTE2                   (0xA215)

#define CPLD_SW2_REG_I2C_STATUS                  (0xA216)

#define CPLD_SW2_REG_WRITE_DATA_BYTE0            (0xA220)
#define CPLD_SW2_REG_READ_DATA_BYTE0             (0xA230)

#define CPLD_SW2_REG_SFP_1_8_RXLOS_L             (0xA240)
#define CPLD_SW2_REG_SFP_9_16_RXLOS_L            (0xA241)
#define CPLD_SW2_REG_SFP_17_18_RXLOS_L           (0xA242)

#define CPLD_SW2_REG_SFP_1_8_TX_DISABLE          (0xA250)
#define CPLD_SW2_REG_SFP_9_16_TX_DISABLE         (0xA251)
#define CPLD_SW2_REG_SFP_17_18_TX_DISABLE        (0xA252)

#define CPLD_SW2_REG_SFP_1_8_TX_FAULT            (0xA256)
#define CPLD_SW2_REG_SFP_9_16_TX_FAULT           (0xA257)
#define CPLD_SW2_REG_SFP_17_18_TX_FAULT          (0xA258)

#define CPLD_SW2_REG_SFP_1_8_ABS                 (0xA259)
#define CPLD_SW2_REG_SFP_9_16_ABS                (0xA25A)
#define CPLD_SW2_REG_SFP_17_18_ABS               (0xA25B)

/* Switch card CPLD3 */

#define CPLD_SW3_REG_VERSION                     (0xA280)
  #define CPLD3_VERSION_MASK                     (0x0f)

#define CPLD_SW3_REG_SCRATCH                     (0xA281)

#define CPLD_SW3_REG_I2C_PORT_ID                 (0xA290)

#define CPLD_SW3_REG_I2C_OP_CODE                 (0xA291)

#define CPLD_SW3_REG_DEV_ADDR                    (0xA292)

#define CPLD_SW3_REG_CMD_BYTE0                   (0xA293)
#define CPLD_SW3_REG_CMD_BYTE1                   (0xA294)
#define CPLD_SW3_REG_CMD_BYTE2                   (0xA295)

#define CPLD_SW3_REG_I2C_STATUS                  (0xA296)

#define CPLD_SW3_REG_WRITE_DATA_BYTE0            (0xA2A0)
#define CPLD_SW3_REG_READ_DATA_BYTE0             (0xA2B0)

#define CPLD_SW3_REG_SFP_19_26_RXLOS_L           (0xA2C0)
#define CPLD_SW3_REG_SFP_27_34_RXLOS_L           (0xA2C1)
#define CPLD_SW3_REG_SFP_35_36_RXLOS_L           (0xA2C2)

#define CPLD_SW3_REG_SFP_19_26_TX_DISABLE        (0xA2D0)
#define CPLD_SW3_REG_SFP_27_34_TX_DISABLE        (0xA2D1)
#define CPLD_SW3_REG_SFP_35_36_TX_DISABLE        (0xA2D2)

#define CPLD_SW3_REG_SFP_19_26_TX_FAULT          (0xA2D6)
#define CPLD_SW3_REG_SFP_27_34_TX_FAULT          (0xA2D7)
#define CPLD_SW3_REG_SFP_35_36_TX_FAULT          (0xA2D8)

#define CPLD_SW3_REG_SFP_19_26_ABS               (0xA2D9)
#define CPLD_SW3_REG_SFP_27_34_ABS               (0xA2DA)
#define CPLD_SW3_REG_SFP_35_36_ABS               (0xA2DB)

/* Switch card CPLD4 */

#define CPLD_SW4_REG_VERSION                     (0xA380)
  #define CPLD4_VERSION_MASK                     (0x0f)

#define CPLD_SW4_REG_SCRATCH                     (0xA381)

#define CPLD_SW4_REG_I2C_PORT_ID                 (0xA390)

#define CPLD_SW4_REG_I2C_OP_CODE                 (0xA391)

#define CPLD_SW4_REG_DEV_ADDR                    (0xA392)

#define CPLD_SW4_REG_CMD_BYTE0                   (0xA393)
#define CPLD_SW4_REG_CMD_BYTE1                   (0xA394)
#define CPLD_SW4_REG_CMD_BYTE2                   (0xA395)

#define CPLD_SW4_REG_I2C_STATUS                  (0xA396)

#define CPLD_SW4_REG_WRITE_DATA_BYTE0            (0xA3A0)
#define CPLD_SW4_REG_READ_DATA_BYTE0             (0xA3B0)

#define CPLD_SW4_REG_SFP_37_44_RXLOS_L           (0xA3C0)
#define CPLD_SW4_REG_SFP_45_48_RXLOS_L           (0xA3C1)

#define CPLD_SW4_REG_SFP_37_44_TX_DISABLE        (0xA3D0)
#define CPLD_SW4_REG_SFP_45_48_TX_DISABLE        (0xA3D1)

#define CPLD_SW4_REG_SFP_37_44_TX_FAULT          (0xA3D4)
#define CPLD_SW4_REG_SFP_45_48_TX_FAULT          (0xA3D5)

#define CPLD_SW4_REG_SFP_37_44_ABS               (0xA3D6)
#define CPLD_SW4_REG_SFP_45_48_ABS               (0xA3D7)

#endif /* CEL_QUESTONE_H__ */

