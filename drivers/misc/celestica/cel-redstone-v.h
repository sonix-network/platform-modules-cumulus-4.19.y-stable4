/*
 * Celestica Redstone-V CPLD Platform Definitions
 *
 * David Yen <dhyen@cumulusnetworks.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef CEL_REDSTONE_V_H__
#define CEL_REDSTONE_V_H__

#define CPLD_STRING_NAME_SIZE 30
#define SFP_LABEL_SIZE  8
#define QSFP_LABEL_SIZE  8
#define NUM_SFP_REGS 8
#define REG_NAME_LEN 16

#define CEL_REDSTONE_V_NUM_PORTS            (52)
#define CEL_REDSTONE_V_NUM_SFP_PORTS        (48)

/*
 * The platform has one i801 and one ismt adapter.
 *
 * The i801 is connected to the following:
 *    eeprom (0x50)
 *    24lc64t serial eeprrom (0x56)
 *    ir3584 voltage regulator (0x08)
 *    ir3584 voltage regulator (0x70)
 *    ir3584 voltage regulator (0x71)
 *    pca9548a 8-channel mux (0x76)
 *	 0 psu1 (0x58)
 *	   psu1 eeprom (0x50)
 *	 1 lm75 (0x49)
 *	   lm75 (0x4a)
 *	   lm75 (0x4b)
 *	   lm75 (0x4c)
 *	 2 psu2 (0x58)
 *	   psu2 eeprom (0x50)
 *	 3 CPLD2 (0x3a)
 *	   CPLD3 (0x3b)
 *	   CPLD4 (0x3c)
 *	 4 24lc64t serial eeprom (0x42)
 *	 5 3584 voltage regulator (0x10)
 *	   3584 voltage regulator (0x72)
 *	   3584 voltage regulator (0x73)
 *	 6 pi6c20800b clock buf (0x6e)
 *	 7 3581 voltage regulator (0x11)
 *	   3581 voltage regulator (0x74)
 *	   3581 voltage regulator (0x75)
 *    pca9548a 8-channel mux (0x77)
 *	 0 eeprom (0x50)
 *	 1 eeprom (0x50)
 *	 2 eeprom (0x50)
 *	 3 eeprom (0x50)
 *	 4 eeprom (0x50)
 *    CPLD1 (0x39)
 *    zl30362 ieee 1588 (0x53)
 */

enum {
	I2C_I801_BUS = -2,
	I2C_ISMT_BUS,

	/* mux #1 */
	I2C_MUX1_BUS0 = 10,
	I2C_MUX1_BUS1,
	I2C_MUX1_BUS2,
	I2C_MUX1_BUS3,
	I2C_MUX1_BUS4,
	I2C_MUX1_BUS5,
	I2C_MUX1_BUS6,
	I2C_MUX1_BUS7,

	/* mux #2 */
	I2C_MUX2_BUS0 = 20,
	I2C_MUX2_BUS1,
	I2C_MUX2_BUS2,
	I2C_MUX2_BUS3,
	I2C_MUX2_BUS4,
	I2C_MUX2_BUS5,
	I2C_MUX2_BUS6,
	I2C_MUX2_BUS7,

	/* CPLD muxes */
	I2C_CPLD_MUX1 = 30,
	I2C_CPLD_MUX2,
	I2C_CPLD_MUX3,
	I2C_CPLD_MUX4,
	I2C_CPLD_MUX_FIRST_PORT = 50,

};

/*
 * From R1097-M0008-01 Rev 0.2 Redstone-V CPLD Design Specification
 */

#define CPLD_1_4_OFFSET 0x200
#define CPLD_2_3_OFFSET 0x100
#define CPLD_IO_BASE 0xA100
#define CPLD_IO_SIZE 0x02ff

/* CPLD1 */

#define CPLD1_VERSION_REG                              (0x200)
#define  CPLD1_CPLD_ID_MASK                            (0xF0)
#define  CPLD1_CPLD_ID_SHIFT                           (4)
#define  CPLD1_CPLD_ID                                 (0x10)
#define  CPLD1_CPLD_MINOR_VERSION_MASK                 (0x0F)
#define  CPLD1_CPLD_MINOR_VERSION_SHIFT                (0)

#define CPLD1_SOFTWARE_SCRATCH_REG                     (0x201)

/* resets are active-L */
#define CPLD1_RESET_CONTROL_1_REG                      (0x202)
#define  CPLD1_ZL30253_RESET_CONTROL_MASK              (0x80)
#define  CPLD1_PCA9548_RESET_CONTROL_MASK              (0x40)
#define  CPLD1_I210_RESET_CONTROL_MASK                 (0x20)
#define  CPLD1_DPLL_RESET_CONTROL_MASK                 (0x10)
#define  CPLD1_BCM56760_RESET_CONTROL_MASK             (0x08)
#define  CPLD1_CPLD4_RESET_CONTROL_MASK                (0x04)
#define  CPLD1_CPLD3_RESET_CONTROL_MASK                (0x02)
#define  CPLD1_CPLD2_RESET_CONTROL_MASK                (0x01)

/* resets are active-L */
#define CPLD1_RESET_CONTROL_2_REG                      (0x203)
#define  CPLD1_BMC_RESET_CONTROL_MASK                  (0x04)
#define  CPLD1_BMC_JTAG_RESET_CONTROL_MASK             (0x02)
#define  CPLD1_COME_RESET_CONTROL_MASK                 (0x01)

#define CPLD1_INTERRUPT_PORT_STATUS_REG                (0x205)
#define  CPLD1_SWITCH_INT_N_MASK                       (0x04)
#define  CPLD1_OPTICAL_RX_LOS_INT_MASK                 (0x02)
#define  CPLD1_OPTICAL_ABS_INT_N_MASK                  (0x01)

#define CPLD1_INT0_SOURCE_STATUS_REG                   (0x206)
#define  CPLD1_STATUS_CPLD4_ABS_INT_N_MASK             (0x04)
#define  CPLD1_STATUS_CPLD3_ABS_INT_N_MASK             (0x02)
#define  CPLD1_STATUS_CPLD2_ABS_INT_N_MASK             (0x01)

#define CPLD1_INT1_SOURCE_STATUS_REG                   (0x207)
#define  CPLD1_STATUS_CPLD4_RX_LOS_INT_N_MASK          (0x04)
#define  CPLD1_STATUS_CPLD3_RX_LOS_INT_N_MASK          (0x02)
#define  CPLD1_STATUS_CPLD2_RX_LOS_INT_N_MASK          (0x01)

#define CPLD1_INT2_SOURCE_STATUS_REG                   (0x208)
#define  CPLD1_STATUS_BMC_INT_OUT_MASK                 (0x80)
#define  CPLD1_STATUS_CB_THRMTRIP_N_MASK               (0x40)
#define  CPLD1_STATUS_PSU_R_ALERT_N_MASK               (0x20)
#define  CPLD1_STATUS_PSU_L_ALERT_N_MASK               (0x10)
#define  CPLD1_STATUS_IR3584_3R3V_ALERT_N_MASK         (0x08)
#define  CPLD1_STATUS_IR3581_ROV_ALERT_N_MASK          (0x04)
#define  CPLD1_STATUS_DPLL_INT_MASK                    (0x02)
#define  CPLD1_STATUS_LM75_INT_N_MASK                  (0x01)

#define CPLD1_INT0_SOURCE_INTERRUPT_REG                (0x209)
#define  CPLD1_INTERRUPT_CPLD4_ABS_INT_N_MASK          (0x04)
#define  CPLD1_INTERRUPT_CPLD3_ABS_INT_N_MASK          (0x02)
#define  CPLD1_INTERRUPT_CPLD2_ABS_INT_N_MASK          (0x01)

#define CPLD1_INT1_SOURCE_INTERRUPT_REG                (0x20A)
#define  CPLD1_INTERRUPT_CPLD4_RX_LOS_INT_N_MASK       (0x04)
#define  CPLD1_INTERRUPT_CPLD3_RX_LOS_INT_N_MASK       (0x02)
#define  CPLD1_INTERRUPT_CPLD2_RX_LOS_INT_N_MASK       (0x01)

#define CPLD1_INT2_SOURCE_INTERRUPT_REG                (0x20B)
#define  CPLD1_INTERRUPT_BMC_INT_OUT_MASK              (0x80)
#define  CPLD1_INTERRUPT_CB_THRMTRIP_N_MASK            (0x40)
#define  CPLD1_INTERRUPT_PSU_R_ALERT_N_MASK            (0x20)
#define  CPLD1_INTERRUPT_PSU_L_ALERT_N_MASK            (0x10)
#define  CPLD1_INTERRUPT_IR3584_3R3V_ALERT_N_MASK      (0x08)
#define  CPLD1_INTERRUPT_IR3581_ROV_ALERT_N_MASK       (0x04)
#define  CPLD1_INTERRUPT_DPLL_INT_MASK                 (0x02)
#define  CPLD1_INTERRUPT_LM75_INT_N_MASK               (0x01)

#define CPLD1_INT0_SOURCE_MASK_REG                     (0x20C)
#define  CPLD1_MASK_CPLD4_ABS_INT_N_MASK               (0x04)
#define  CPLD1_MASK_CPLD3_ABS_INT_N_MASK               (0x02)
#define  CPLD1_MASK_CPLD2_ABS_INT_N_MASK               (0x01)

#define CPLD1_INT1_SOURCE_MASK_REG                     (0x20D)
#define  CPLD1_MASK_CPLD4_RX_LOS_INT_N_MASK            (0x04)
#define  CPLD1_MASK_CPLD3_RX_LOS_INT_N_MASK            (0x02)
#define  CPLD1_MASK_CPLD2_RX_LOS_INT_N_MASK            (0x01)

#define CPLD1_INT2_SOURCE_MASK_REG                     (0x20E)
#define  CPLD1_MASK_BMC_INT_OUT_MASK                   (0x80)
#define  CPLD1_MASK_CB_THRMTRIP_N_MASK                 (0x40)
#define  CPLD1_MASK_PSU_R_ALERT_N_MASK                 (0x20)
#define  CPLD1_MASK_PSU_L_ALERT_N_MASK                 (0x10)
#define  CPLD1_MASK_IR3584_3R3V_ALERT_N_MASK           (0x08)
#define  CPLD1_MASK_IR3581_ROV_ALERT_N_MASK            (0x04)
#define  CPLD1_MASK_DPLL_INT_MASK                      (0x02)
#define  CPLD1_MASK_LM75_INT_N_MASK                    (0x01)

#define CPLD1_FAN1_PWM_CONTROL_REG                     (0x210)
#define  CPLD1_FAN1_PWM_CONTROL_MASK                   (0xFF)

#define CPLD1_FAN1_MISC_CONTROL_AND_STATUS_REG         (0x211)
#define  CPLD1_FAN1_DIRECTION_MASK                     (0x08)
#define  CPLD1_FAN1_PRESENT_STATUS_MASK                (0x04)
#define  CPLD1_FAN1_GREEN_LED_CONTROL_MASK             (0x02)
#define  CPLD1_FAN1_RED_LED_CONTROL_MASK               (0x01)

#define CPLD1_FAN1_REAR_FAN_SPEED_REG                  (0x212)
#define  CPLD1_FAN1_REAR_FAN_SPEED_MASK                (0xFF)
#define  CPLD1_FAN_SPEED_MULT                          (150)

#define CPLD1_FAN1_FRONT_FAN_SPEED_REG                 (0x213)
#define  CPLD1_FAN1_FRONT_FAN_SPEED_MASK               (0xFF)

#define CPLD1_FAN2_PWM_CONTROL_REG                     (0x214)
#define  CPLD1_FAN2_PWM_CONTROL_MASK                   (0xFF)

#define CPLD1_FAN2_MISC_CONTROL_AND_STATUS_REG         (0x215)
#define  CPLD1_FAN2_DIRECTION_MASK                     (0x08)
#define  CPLD1_FAN2_PRESENT_STATUS_MASK                (0x04)
#define  CPLD1_FAN2_GREEN_LED_CONTROL_MASK             (0x02)
#define  CPLD1_FAN2_RED_LED_CONTROL_MASK               (0x01)

#define CPLD1_FAN2_REAR_FAN_SPEED_REG                  (0x216)
#define  CPLD1_FAN2_REAR_FAN_SPEED_MASK                (0xFF)

#define CPLD1_FAN2_FRONT_FAN_SPEED_REG                 (0x217)
#define  CPLD1_FAN2_FRONT_FAN_SPEED_MASK               (0xFF)

#define CPLD1_FAN3_PWM_CONTROL_REG                     (0x218)
#define  CPLD1_FAN3_PWM_CONTROL_MASK                   (0xFF)

#define CPLD1_FAN3_MISC_CONTROL_AND_STATUS_REG         (0x219)
#define  CPLD1_FAN3_DIRECTION_MASK                     (0x08)
#define  CPLD1_FAN3_PRESENT_STATUS_MASK                (0x04)
#define  CPLD1_FAN3_GREEN_LED_CONTROL_MASK             (0x02)
#define  CPLD1_FAN3_RED_LED_CONTROL_MASK               (0x01)

#define CPLD1_FAN3_REAR_FAN_SPEED_REG                  (0x21A)
#define  CPLD1_FAN3_REAR_FAN_SPEED_MASK                (0xFF)

#define CPLD1_FAN3_FRONT_FAN_SPEED_REG                 (0x21B)
#define  CPLD1_FAN3_FRONT_FAN_SPEED_MASK               (0xFF)

#define CPLD1_FAN4_PWM_CONTROL_REG                     (0x21C)
#define  CPLD1_FAN4_PWM_CONTROL_MASK                   (0xFF)

#define CPLD1_FAN4_MISC_CONTROL_AND_STATUS_REG         (0x21D)
#define  CPLD1_FAN4_DIRECTION_MASK                     (0x08)
#define  CPLD1_FAN4_PRESENT_STATUS_MASK                (0x04)
#define  CPLD1_FAN4_GREEN_LED_CONTROL_MASK             (0x02)
#define  CPLD1_FAN4_RED_LED_CONTROL_MASK               (0x01)

#define CPLD1_FAN4_REAR_FAN_SPEED_REG                  (0x21E)
#define  CPLD1_FAN4_REAR_FAN_SPEED_MASK                (0xFF)

#define CPLD1_FAN4_FRONT_FAN_SPEED_REG                 (0x21F)
#define  CPLD1_FAN4_FRONT_FAN_SPEED_MASK               (0xFF)

#define CPLD1_FAN5_PWM_CONTROL_REG                     (0x220)
#define  CPLD1_FAN5_PWM_CONTROL_MASK                   (0xFF)

#define CPLD1_FAN5_MISC_CONTROL_AND_STATUS_REG         (0x221)
#define  CPLD1_FAN5_DIRECTION_MASK                     (0x08)
#define  CPLD1_FAN5_PRESENT_STATUS_MASK                (0x04)
#define  CPLD1_FAN5_GREEN_LED_CONTROL_MASK             (0x02)
#define  CPLD1_FAN5_RED_LED_CONTROL_MASK               (0x01)

#define CPLD1_FAN5_REAR_FAN_SPEED_REG                  (0x222)
#define  CPLD1_FAN5_REAR_FAN_SPEED_MASK                (0xFF)

#define CPLD1_FAN5_FRONT_FAN_SPEED_REG                 (0x223)
#define  CPLD1_FAN5_FRONT_FAN_SPEED_MASK               (0xFF)

#define CPLD1_PSU_CONTROL_AND_STATUS_REG               (0x230)
#define  CPLD1_LEFT_PSU_ALERT_STATUS_MASK              (0x80)
#define  CPLD1_RIGHT_PSU_ALERT_STATUS_MASK             (0x40)
#define  CPLD1_LEFT_PSU_PRESENT_STATUS_MASK            (0x20)
#define  CPLD1_RIGHT_PSU_PRESENT_STATUS_MASK           (0x10)
#define  CPLD1_LEFT_PSU_POWER_OK_STATUS_MASK           (0x08)
#define  CPLD1_RIGHT_PSU_POWER_OK_STATUS_MASK          (0x04)
#define  CPLD1_LEFT_PSU_ENABLE_CONTROL_MASK            (0x02)
#define  CPLD1_RIGHT_PSU_ENABLE_CONTROL_MASK           (0x01)

#define CPLD1_PSU_LED_SOFTWARE_CONTROL_REG             (0x231)
#define  CPLD1_ENABLE_SW_CONTROL_OF_RIGHT_PSU_LED_MASK (0x20)
#define  CPLD1_ENABLE_SW_CONTROL_OF_LEFT_PSU_LED_MASK  (0x10)
#define  CPLD1_RIGHT_PSU_LED_MASK                      (0x0C)
#define   CPLD1_RIGHT_PSU_LED_AMBER                    (0x08)
#define   CPLD1_RIGHT_PSU_LED_GREEN                    (0x04)
#define   CPLD1_RIGHT_PSU_LED_OFF                      (0x00)
#define  CPLD1_LEFT_PSU_LED_MASK                       (0x03)
#define   CPLD1_LEFT_PSU_LED_AMBER                     (0x02)
#define   CPLD1_LEFT_PSU_LED_GREEN                     (0x01)
#define   CPLD1_LEFT_PSU_LED_OFF                       (0x00)

#define CPLD1_FRONT_PANEL_LED_CONTROL_REG              (0x232)
#define  CPLD1_BMC_STATUS_LED_CONTROL_MASK             (0xC0)
#define   CPLD1_BMC_STATUS_LED_GREEN                   (0x40)
#define   CPLD1_BMC_STATUS_LED_YELLOW                  (0x80)
#define   CPLD1_BMC_STATUS_LED_OFF                     (0xC0)
#define  CPLD1_SYSTEM_LED_CONTROL_MASK                 (0x30)
#define   CPLD1_SYSTEM_LED_GREEN                       (0x10)
#define   CPLD1_SYSTEM_LED_YELLOW                      (0x20)
#define   CPLD1_SYSTEM_LED_OFF                         (0x30)
#define  CPLD1_SYSTEM_STATUS_LED_CONTROL_MASK          (0x03)
#define   CPLD1_SYSTEM_STATUS_LED_OFF                  (0x03)
#define   CPLD1_SYSTEM_STATUS_LED_5HZ_BLINK            (0x02)
#define   CPLD1_SYSTEM_STATUS_LED_1HZ_BLINK            (0x01)
#define   CPLD1_SYSTEM_STATUS_LED_ON                   (0x00)

#define CPLD1_UART_MUX_CONTROL_REG                     (0x233)
#define  CPLD1_UART_MUX_CONTROL_MASK                   (0x01)
#define   CPLD1_UART_CONNECT_TO_BMC                    (0x00)
#define   CPLD1_UART_CONNECT_TO_COME                   (0x01)

#define CPLD1_COME_POWER_CONTROL_REG                   (0x235)
#define  CPLD1_PWR_COME_FORCE_CONTROL_MASK             (0x02)
#define   CPLD1_FORCE_COME_POWER_OFF                   (0x00)
#define  CPLD1_PWR_COME_EN_MASK                        (0x01)
#define   CPLD1_COME_POWER_OFF                         (0x00)
#define   CPLD1_COME_POWER_ON                          (0x01)

#define CPLD1_COME_BIOS_DISABLE_CONTROL_REG            (0x236)
#define  CPLD1_CONTROL_COME_BIOS_DIS1_MASK             (0x02)
#define  CPLD1_CONTROL_COME_BIOS_DIS0_MASK             (0x01)

#define CPLD1_COME_STATUS_REG                          (0x237)
#define  CPLD1_COME_SUS_STAT_N_MASK                    (0x08)
#define  CPLD1_COME_SUS_S5_N_MASK                      (0x04)
#define  CPLD1_COME_SUS_S4_N_MASK                      (0x02)
#define  CPLD1_COME_SUS_S3_N_MASK                      (0x01)

#define CPLD1_COME_POWER_BUTTON_CONTROL_REG            (0x238)
#define  CPLD1_CB_PWR_BTN_N_MASK                       (0x01)

#define CPLD1_COME_BOARD_TYPE_REG                      (0x239)
#define  CPLD1_CB_TYPE_N_MASK                          (0x07)

#define CPLD1_GENERAL_STATUS_REG                       (0x241)
#define  CPLD1_STATUS_DPLL1_LOCK_MASK                  (0x04)
#define  CPLD1_STATUS_L1_RCVRD_CLK_VID_MASK            (0x02)
#define  CPLD1_STATUS_L1_RCVRD_CLK_BKUP_VID_MASK       (0x01)

#define CPLD1_MISC_CONTROL_REG                         (0x242)
#define  CPLD1_BMC_BIOS_SPI_WP_MASK                    (0x10)
#define  CPLD1_MAINBOARD_EEPROM_WP_MASK                (0x08)
#define  CPLD1_TLV_EEPROM_WP_MASK                      (0x04)
#define  CPLD1_APLL_156M25_CLK_SEL_MASK                (0x02)
#define  CPLD1_DPLL_25M_CLK_SEL_MASK                   (0x01)

#define CPLD1_I2C_CONTROL_SELECT_REG                   (0x245)
#define  CPLD1_DRIVE_I2C_SEL_MASK                      (0x01)

#define CPLD1_SWITCH_BOARD_TYPE_REG                    (0x270)
#define  CPLD1_SWITCH_BOARD_TYPE_MASK                  (0xFF)
#define   CPLD1_RED_V_BOARD                            (0x09)
#define   CPLD1_RXP_B_BOARD                            (0x08)
#define   CPLD1_SXP_B_BOARD                            (0x07)
#define   CPLD1_RDP_BOARD                              (0x06)
#define   CPLD1_MIDSTONE_BOARD                         (0x05)
#define   CPLD1_REDSTONE_V_BOARD                       (0x04)
#define   CPLD1_SEASTONE_BOARD                         (0x03)
#define   CPLD1_SMALLSTONE_XP_BOARD                    (0x02)
#define   CPLD1_REDSTONE_XP_BOARD                      (0x01)

/* CPLD2 */

#define CPLD2_VERSION_REG                              (0x100)
#define  CPLD2_CPLD_ID_MASK                            (0xF0)
#define  CPLD2_CPLD_ID_SHIFT                           (4)
#define  CPLD2_CPLD_ID                                 (0x20)
#define  CPLD2_CPLD_MINOR_VERSION_MASK                 (0x0F)
#define  CPLD2_CPLD_MINOR_VERSION_SHIFT                (0)

#define CPLD2_SOFTWARE_SCRATCH_REG                     (0x101)

#define CPLD2_I2C_FREQUENCY_DIVIDER_REG                (0x110)
#define  CPLD2_BAUD_RATE_SETTING_MASK                  (0x3F)
#define   CPLD2_BAUD_RATE_50KHZ                        (0x3F)
#define   CPLD2_BAUD_RATE_100KHZ                       (0x1F)
#define   CPLD2_BAUD_RATE_200KHZ                       (0x0F)
#define   CPLD2_BAUD_RATE_400KHZ                       (0x07)

#define CPLD2_I2C_CONTROL_REG                          (0x111)
#define  CPLD2_MODULE_ENABLE_MASK                      (0x80)
#define  CPLD2_MODULE_INTERRUPT_ENABLE_MASK            (0x40)
#define  CPLD2_MASTER_SLAVE_MODE_START_MASK            (0x20)
#define  CPLD2_TRANSMIT_RECEIVE_MODE_SELECT_MASK       (0x10)
#define  CPLD2_TRANSFER_ACKNOWLEDGE_MASK               (0x08)
#define  CPLD2_REPEATED_START_MASK                     (0x04)
#define  CPLD2_BROADCAST_MASK                          (0x01)

#define CPLD2_I2C_STATUS_REG                           (0x112)
#define  CPLD2_DATA_TRANSFER_MASK                      (0x80)
#define  CPLD2_ADDRESS_AS_A_SLAVE_MASK                 (0x40)
#define  CPLD2_BUS_BUSY_MASK                           (0x20)
#define  CPLD2_ARBITRATION_LOST_MASK                   (0x10)
#define  CPLD2_BROADCAST_MATCH_MASK                    (0x08)
#define  CPLD2_SLAVE_READ_WRITE_MASK                   (0x04)
#define  CPLD2_MODULE_INTERRUPT_MASK                   (0x02)
#define  CPLD2_RECEIVE_ACKNOWLEDGE_MASK                (0x01)

#define CPLD2_I2C_DATA_REG                             (0x113)

#define CPLD2_I2C_PORT_ID_REG                          (0x114)
#define  CPLD2_I2C_OPERATION_ID_MASK                   (0x3F)

#define CPLD2_SPI_CS_CONTROL_REG                       (0x120)
#define  CPLD2_SLAVE_DEVICE_SELECT_MASK                (0x0F)
#define   CPLD2_SELECT_CS1                             (0x01)
#define   CPLD2_SELECT_CS2                             (0x02)
#define   CPLD2_SELECT_CS3                             (0x03)
#define   CPLD2_SELECT_CS4                             (0x04)

#define CPLD2_SPI_OP_CONTROL_REG                       (0x121)
#define  CPLD2_OP_COMMAND_MASK                         (0xFF)
#define   CPLD2_OP_WRITE                               (0x02)
#define   CPLD2_OP_READ                                (0x03)

#define CPLD2_SPI_CLOCK_DIVIDER_CONTROL_REG            (0x122)

#define CPLD2_SPI_DEVICE_ADDRESS_BYTE0_REG             (0x123)
#define CPLD2_SPI_DEVICE_ADDRESS_BYTE1_REG             (0x124)
#define CPLD2_SPI_DEVICE_ADDRESS_BYTE2_REG             (0x125)
#define CPLD2_SPI_DEVICE_ADDRESS_BYTE3_REG             (0x126)

#define CPLD2_SPI_STATUS_AND_SOFT_CONTROL_REG          (0x127)
#define  CPLD2_SPI_BUSY_INDICATOR_MASK                 (0x40)
#define  CPLD2_START_SPI_MASTER_OPERATION_MASK         (0x20)
#define  CPLD2_SPI_MASTER_SOFTWARE_RESET_MASK          (0x10)
#define  CPLD2_SPI_OPERATION_DATA_BYTE_LENGTH_MASK     (0x0C)
#define  CPLD2_SPI_OPERATION_ADDRESS_BYTE_LENGTH_MASK  (0x03)

#define CPLD2_SPI_WRITE_DATA_BYTE0_REG                 (0x128)
#define CPLD2_SPI_WRITE_DATA_BYTE1_REG                 (0x129)
#define CPLD2_SPI_WRITE_DATA_BYTE2_REG                 (0x12A)
#define CPLD2_SPI_WRITE_DATA_BYTE3_REG                 (0x12B)

#define CPLD2_SPI_READ_DATA_BYTE0_REG                  (0x12C)
#define CPLD2_SPI_READ_DATA_BYTE1_REG                  (0x12D)
#define CPLD2_SPI_READ_DATA_BYTE2_REG                  (0x12E)
#define CPLD2_SPI_READ_DATA_BYTE3_REG                  (0x12F)

#define CPLD2_SFP_1_8_RXLOS_REG                        (0x140)
#define  CPLD2_SFP_8_RXLOS                             (1 << 7)
#define  CPLD2_SFP_7_RXLOS                             (1 << 6)
#define  CPLD2_SFP_6_RXLOS                             (1 << 5)
#define  CPLD2_SFP_5_RXLOS                             (1 << 4)
#define  CPLD2_SFP_4_RXLOS                             (1 << 3)
#define  CPLD2_SFP_3_RXLOS                             (1 << 2)
#define  CPLD2_SFP_2_RXLOS                             (1 << 1)
#define  CPLD2_SFP_1_RXLOS                             (1 << 0)

#define CPLD2_SFP_9_16_RXLOS_REG                       (0x141)
#define  CPLD2_SFP_16_RXLOS                            (1 << 7)
#define  CPLD2_SFP_15_RXLOS                            (1 << 6)
#define  CPLD2_SFP_14_RXLOS                            (1 << 5)
#define  CPLD2_SFP_13_RXLOS                            (1 << 4)
#define  CPLD2_SFP_12_RXLOS                            (1 << 3)
#define  CPLD2_SFP_11_RXLOS                            (1 << 2)
#define  CPLD2_SFP_10_RXLOS                            (1 << 1)
#define  CPLD2_SFP_9_RXLOS                             (1 << 0)

#define CPLD2_SFP_17_18_RXLOS_REG                      (0x142)
#define  CPLD2_SFP_18_RXLOS                            (1 << 1)
#define  CPLD2_SFP_17_RXLOS                            (1 << 0)

#define CPLD2_SFP_1_8_RXLOS_INTERRUPT_REG              (0x143)
#define  CPLD2_SFP_8_RXLOS_INTERRUPT                   (1 << 7)
#define  CPLD2_SFP_7_RXLOS_INTERRUPT                   (1 << 6)
#define  CPLD2_SFP_6_RXLOS_INTERRUPT                   (1 << 5)
#define  CPLD2_SFP_5_RXLOS_INTERRUPT                   (1 << 4)
#define  CPLD2_SFP_4_RXLOS_INTERRUPT                   (1 << 3)
#define  CPLD2_SFP_3_RXLOS_INTERRUPT                   (1 << 2)
#define  CPLD2_SFP_2_RXLOS_INTERRUPT                   (1 << 1)
#define  CPLD2_SFP_1_RXLOS_INTERRUPT                   (1 << 0)

#define CPLD2_SFP_9_16_RXLOS_INTERRUPT_REG             (0x144)
#define  CPLD2_SFP_16_RXLOS_INTERRUPT                  (1 << 7)
#define  CPLD2_SFP_15_RXLOS_INTERRUPT                  (1 << 6)
#define  CPLD2_SFP_14_RXLOS_INTERRUPT                  (1 << 5)
#define  CPLD2_SFP_13_RXLOS_INTERRUPT                  (1 << 4)
#define  CPLD2_SFP_12_RXLOS_INTERRUPT                  (1 << 3)
#define  CPLD2_SFP_11_RXLOS_INTERRUPT                  (1 << 2)
#define  CPLD2_SFP_10_RXLOS_INTERRUPT                  (1 << 1)
#define  CPLD2_SFP_9_RXLOS_INTERRUPT                   (1 << 0)

#define CPLD2_SFP_17_18_RXLOS_INTERRUPT_REG            (0x145)
#define  CPLD2_SFP_18_RXLOS_INTERRUPT                  (1 << 1)
#define  CPLD2_SFP_17_RXLOS_INTERRUPT                  (1 << 0)

#define CPLD2_SFP_1_8_RXLOS_MASK_REG                   (0x146)
#define  CPLD2_SFP_8_RXLOS_MASK                        (1 << 7)
#define  CPLD2_SFP_7_RXLOS_MASK                        (1 << 6)
#define  CPLD2_SFP_6_RXLOS_MASK                        (1 << 5)
#define  CPLD2_SFP_5_RXLOS_MASK                        (1 << 4)
#define  CPLD2_SFP_4_RXLOS_MASK                        (1 << 3)
#define  CPLD2_SFP_3_RXLOS_MASK                        (1 << 2)
#define  CPLD2_SFP_2_RXLOS_MASK                        (1 << 1)
#define  CPLD2_SFP_1_RXLOS_MASK                        (1 << 0)

#define CPLD2_SFP_9_16_RXLOS_MASK_REG                  (0x147)
#define  CPLD2_SFP_16_RXLOS_MASK                       (1 << 7)
#define  CPLD2_SFP_15_RXLOS_MASK                       (1 << 6)
#define  CPLD2_SFP_14_RXLOS_MASK                       (1 << 5)
#define  CPLD2_SFP_13_RXLOS_MASK                       (1 << 4)
#define  CPLD2_SFP_12_RXLOS_MASK                       (1 << 3)
#define  CPLD2_SFP_11_RXLOS_MASK                       (1 << 2)
#define  CPLD2_SFP_10_RXLOS_MASK                       (1 << 1)
#define  CPLD2_SFP_9_RXLOS_MASK                        (1 << 0)

#define CPLD2_SFP_17_18_RXLOS_MASK_REG                 (0x148)
#define  CPLD2_SFP_18_RXLOS_MASK                       (1 << 1)
#define  CPLD2_SFP_17_RXLOS_MASK                       (1 << 0)

#define CPLD2_SFP_1_8_TXDISABLE_REG                    (0x150)
#define  CPLD2_SFP_8_TXDISABLE                         (1 << 7)
#define  CPLD2_SFP_7_TXDISABLE                         (1 << 6)
#define  CPLD2_SFP_6_TXDISABLE                         (1 << 5)
#define  CPLD2_SFP_5_TXDISABLE                         (1 << 4)
#define  CPLD2_SFP_4_TXDISABLE                         (1 << 3)
#define  CPLD2_SFP_3_TXDISABLE                         (1 << 2)
#define  CPLD2_SFP_2_TXDISABLE                         (1 << 1)
#define  CPLD2_SFP_1_TXDISABLE                         (1 << 0)

#define CPLD2_SFP_9_16_TXDISABLE_REG                   (0x151)
#define  CPLD2_SFP_16_TXDISABLE                        (1 << 7)
#define  CPLD2_SFP_15_TXDISABLE                        (1 << 6)
#define  CPLD2_SFP_14_TXDISABLE                        (1 << 5)
#define  CPLD2_SFP_13_TXDISABLE                        (1 << 4)
#define  CPLD2_SFP_12_TXDISABLE                        (1 << 3)
#define  CPLD2_SFP_11_TXDISABLE                        (1 << 2)
#define  CPLD2_SFP_10_TXDISABLE                        (1 << 1)
#define  CPLD2_SFP_9_TXDISABLE                         (1 << 0)

#define CPLD2_SFP_17_18_TXDISABLE_REG                  (0x152)
#define  CPLD2_SFP_18_TXDISABLE                        (1 << 1)
#define  CPLD2_SFP_17_TXDISABLE                        (1 << 0)

#define CPLD2_SFP_1_8_RS_CONTROL_REG                   (0x153)
#define  CPLD2_SFP_8_RS_CONTROL                        (1 << 7)
#define  CPLD2_SFP_7_RS_CONTROL                        (1 << 6)
#define  CPLD2_SFP_6_RS_CONTROL                        (1 << 5)
#define  CPLD2_SFP_5_RS_CONTROL                        (1 << 4)
#define  CPLD2_SFP_4_RS_CONTROL                        (1 << 3)
#define  CPLD2_SFP_3_RS_CONTROL                        (1 << 2)
#define  CPLD2_SFP_2_RS_CONTROL                        (1 << 1)
#define  CPLD2_SFP_1_RS_CONTROL                        (1 << 0)

#define CPLD2_SFP_9_16_RS_CONTROL_REG                  (0x154)
#define  CPLD2_SFP_16_RS_CONTROL                       (1 << 7)
#define  CPLD2_SFP_15_RS_CONTROL                       (1 << 6)
#define  CPLD2_SFP_14_RS_CONTROL                       (1 << 5)
#define  CPLD2_SFP_13_RS_CONTROL                       (1 << 4)
#define  CPLD2_SFP_12_RS_CONTROL                       (1 << 3)
#define  CPLD2_SFP_11_RS_CONTROL                       (1 << 2)
#define  CPLD2_SFP_10_RS_CONTROL                       (1 << 1)
#define  CPLD2_SFP_9_RS_CONTROL                        (1 << 0)

#define CPLD2_SFP_17_18_RS_CONTROL_REG                 (0x155)
#define  CPLD2_SFP_18_RS_CONTROL                       (1 << 1)
#define  CPLD2_SFP_17_RS_CONTROL                       (1 << 0)

#define CPLD2_SFP_1_8_TXFAULT_REG                      (0x156)
#define  CPLD2_SFP_8_TXFAULT                           (1 << 7)
#define  CPLD2_SFP_7_TXFAULT                           (1 << 6)
#define  CPLD2_SFP_6_TXFAULT                           (1 << 5)
#define  CPLD2_SFP_5_TXFAULT                           (1 << 4)
#define  CPLD2_SFP_4_TXFAULT                           (1 << 3)
#define  CPLD2_SFP_3_TXFAULT                           (1 << 2)
#define  CPLD2_SFP_2_TXFAULT                           (1 << 1)
#define  CPLD2_SFP_1_TXFAULT                           (1 << 0)

#define CPLD2_SFP_9_16_TXFAULT_REG                     (0x157)
#define  CPLD2_SFP_16_TXFAULT                          (1 << 7)
#define  CPLD2_SFP_15_TXFAULT                          (1 << 6)
#define  CPLD2_SFP_14_TXFAULT                          (1 << 5)
#define  CPLD2_SFP_13_TXFAULT                          (1 << 4)
#define  CPLD2_SFP_12_TXFAULT                          (1 << 3)
#define  CPLD2_SFP_11_TXFAULT                          (1 << 2)
#define  CPLD2_SFP_10_TXFAULT                          (1 << 1)
#define  CPLD2_SFP_9_TXFAULT                           (1 << 0)

#define CPLD2_SFP_17_18_TXFAULT_REG                    (0x158)
#define  CPLD2_SFP_18_TXFAULT                          (1 << 1)
#define  CPLD2_SFP_17_TXFAULT                          (1 << 0)

#define CPLD2_SFP_1_8_ABS_REG                          (0x159)
#define  CPLD2_SFP_8_ABS                               (1 << 7)
#define  CPLD2_SFP_7_ABS                               (1 << 6)
#define  CPLD2_SFP_6_ABS                               (1 << 5)
#define  CPLD2_SFP_5_ABS                               (1 << 4)
#define  CPLD2_SFP_4_ABS                               (1 << 3)
#define  CPLD2_SFP_3_ABS                               (1 << 2)
#define  CPLD2_SFP_2_ABS                               (1 << 1)
#define  CPLD2_SFP_1_ABS                               (1 << 0)

#define CPLD2_SFP_9_16_ABS_REG                         (0x15A)
#define  CPLD2_SFP_16_ABS                              (1 << 7)
#define  CPLD2_SFP_15_ABS                              (1 << 6)
#define  CPLD2_SFP_14_ABS                              (1 << 5)
#define  CPLD2_SFP_13_ABS                              (1 << 4)
#define  CPLD2_SFP_12_ABS                              (1 << 3)
#define  CPLD2_SFP_11_ABS                              (1 << 2)
#define  CPLD2_SFP_10_ABS                              (1 << 1)
#define  CPLD2_SFP_9_ABS                               (1 << 0)

#define CPLD2_SFP_17_18_ABS_REG                        (0x15B)
#define  CPLD2_SFP_18_ABS                              (1 << 1)
#define  CPLD2_SFP_17_ABS                              (1 << 0)

#define CPLD2_PORT_LED_OPERATING_MODE_REG              (0x160)
#define  CPLD2_PORT_LED_OPERATING_MODE_MASK            (0x01)
#define   CPLD2_PORT_LED_NORMAL_MODE                   (0x00)
#define   CPLD2_PORT_LED_TEST_MODE                     (0x01)

#define CPLD2_PORT_LED_TEST_REG                        (0x161)
#define  CPLD2_PORT_LEDS_AMBER_CONTROL_MASK            (0x02)
#define   CPLD2_ALL_PORTS_LED_AMBER_ON                 (0x00)
#define   CPLD2_ALL_PORTS_LED_AMBER_OFF                (0x02)
#define  CPLD2_PORT_LEDS_GREEN_CONTROL_MASK            (0x01)
#define   CPLD2_ALL_PORTS_LED_GREEN_ON                 (0x00)
#define   CPLD2_ALL_PORTS_LED_GREEN_OFF                (0x01)

/* CPLD3 */

#define CPLD3_VERSION_REG                              (0x180)
#define  CPLD3_CPLD_ID_MASK                            (0xF0)
#define  CPLD3_CPLD_ID_SHIFT                           (4)
#define  CPLD3_CPLD_ID                                 (0x30)
#define  CPLD3_CPLD_MINOR_VERSION_MASK                 (0x0F)
#define  CPLD3_CPLD_MINOR_VERSION_SHIFT                (0)

#define CPLD3_SOFTWARE_SCRATCH_REG                     (0x181)

#define CPLD3_MISC_CONTROL_REG                         (0x182)
#define  CPLD3_CONTROL_ZL30253_GPIO2_MASK              (0x04)
#define  CPLD3_CONTROL_ZL30253_GPIO1_MASK              (0x02)
#define  CPLD3_CONTROL_ZL30253_GPIO0_MASK              (0x01)

#define CPLD3_I2C_FREQUENCY_DIVIDER_REG                (0x190)
#define  CPLD3_BAUD_RATE_SETTING_MASK                  (0x3F)
#define   CPLD3_BAUD_RATE_50KHZ                        (0x3F)
#define   CPLD3_BAUD_RATE_100KHZ                       (0x1F)
#define   CPLD3_BAUD_RATE_200KHZ                       (0x0F)
#define   CPLD3_BAUD_RATE_400KHZ                       (0x07)

#define CPLD3_I2C_CONTROL_REG                          (0x191)
#define  CPLD3_MODULE_ENABLE_MASK                      (0x80)
#define  CPLD3_MODULE_INTERRUPT_ENABLE_MASK            (0x40)
#define  CPLD3_MASTER_SLAVE_MODE_START_MASK            (0x20)
#define  CPLD3_TRANSMIT_RECEIVE_MODE_SELECT_MASK       (0x10)
#define  CPLD3_TRANSFER_ACKNOWLEDGE_MASK               (0x08)
#define  CPLD3_REPEATED_START_MASK                     (0x04)
#define  CPLD3_BROADCAST_MASK                          (0x)

#define CPLD3_I2C_STATUS_REG                           (0x192)
#define  CPLD3_DATA_TRANSFER_MASK                      (0x80)
#define  CPLD3_ADDRESS_AS_A_SLAVE_MASK                 (0x40)
#define  CPLD3_BUS_BUSY_MASK                           (0x20)
#define  CPLD3_ARBITRATION_LOST_MASK                   (0x10)
#define  CPLD3_BROADCAST_MATCH_MASK                    (0x08)
#define  CPLD3_SLAVE_READ_WRITE_MASK                   (0x04)
#define  CPLD3_MODULE_INTERRUPT_MASK                   (0x02)
#define  CPLD3_RECEIVE_ACKNOWLEDGE_MASK                (0x01)

#define CPLD3_I2C_DATA_REG                             (0x193)

#define CPLD3_I2C_PORT_ID_REG                          (0x194)
#define  CPLD3_I2C_OPERATION_ID_MASK                   (0x3F)

#define CPLD3_SFP_19_26_RXLOS_REG                      (0x1C0)
#define  CPLD3_SFP_26_RXLOS                            (1 << 7)
#define  CPLD3_SFP_25_RXLOS                            (1 << 6)
#define  CPLD3_SFP_24_RXLOS                            (1 << 5)
#define  CPLD3_SFP_23_RXLOS                            (1 << 4)
#define  CPLD3_SFP_22_RXLOS                            (1 << 3)
#define  CPLD3_SFP_21_RXLOS                            (1 << 2)
#define  CPLD3_SFP_20_RXLOS                            (1 << 1)
#define  CPLD3_SFP_19_RXLOS                            (1 << 0)

#define CPLD3_SFP_27_34_RXLOS_REG                      (0x1C1)
#define  CPLD3_SFP_34_RXLOS                            (1 << 7)
#define  CPLD3_SFP_33_RXLOS                            (1 << 6)
#define  CPLD3_SFP_32_RXLOS                            (1 << 5)
#define  CPLD3_SFP_31_RXLOS                            (1 << 4)
#define  CPLD3_SFP_30_RXLOS                            (1 << 3)
#define  CPLD3_SFP_29_RXLOS                            (1 << 2)
#define  CPLD3_SFP_28_RXLOS                            (1 << 1)
#define  CPLD3_SFP_27_RXLOS                            (1 << 0)

#define CPLD3_SFP_35_36_RXLOS_REG                      (0x1C2)
#define  CPLD3_SFP_36_RXLOS                            (1 << 1)
#define  CPLD3_SFP_35_RXLOS                            (1 << 0)

#define CPLD3_SFP_19_26_RXLOS_INTERRUPT_REG            (0x1C3)
#define  CPLD3_SFP_26_RXLOS_INTERRUPT                  (1 << 7)
#define  CPLD3_SFP_25_RXLOS_INTERRUPT                  (1 << 6)
#define  CPLD3_SFP_24_RXLOS_INTERRUPT                  (1 << 5)
#define  CPLD3_SFP_23_RXLOS_INTERRUPT                  (1 << 4)
#define  CPLD3_SFP_22_RXLOS_INTERRUPT                  (1 << 3)
#define  CPLD3_SFP_21_RXLOS_INTERRUPT                  (1 << 2)
#define  CPLD3_SFP_20_RXLOS_INTERRUPT                  (1 << 1)
#define  CPLD3_SFP_19_RXLOS_INTERRUPT                  (1 << 0)

#define CPLD3_SFP_27_34_RXLOS_INTERRUPT_REG            (0x1C4)
#define  CPLD3_SFP_34_RXLOS_INTERRUPT                  (1 << 7)
#define  CPLD3_SFP_33_RXLOS_INTERRUPT                  (1 << 6)
#define  CPLD3_SFP_32_RXLOS_INTERRUPT                  (1 << 5)
#define  CPLD3_SFP_31_RXLOS_INTERRUPT                  (1 << 4)
#define  CPLD3_SFP_30_RXLOS_INTERRUPT                  (1 << 3)
#define  CPLD3_SFP_29_RXLOS_INTERRUPT                  (1 << 2)
#define  CPLD3_SFP_28_RXLOS_INTERRUPT                  (1 << 1)
#define  CPLD3_SFP_27_RXLOS_INTERRUPT                  (1 << 0)

#define CPLD3_SFP_35_36_RXLOS_INTERRUPT_REG            (0x1C5)
#define  CPLD3_SFP_36_RXLOS_INTERRUPT                  (1 << 1)
#define  CPLD3_SFP_35_RXLOS_INTERRUPT                  (1 << 0)

#define CPLD3_SFP_19_26_RXLOS_MASK_REG                 (0x1C6)
#define  CPLD3_SFP_26_RXLOS_MASK                       (1 << 7)
#define  CPLD3_SFP_25_RXLOS_MASK                       (1 << 6)
#define  CPLD3_SFP_24_RXLOS_MASK                       (1 << 5)
#define  CPLD3_SFP_23_RXLOS_MASK                       (1 << 4)
#define  CPLD3_SFP_22_RXLOS_MASK                       (1 << 3)
#define  CPLD3_SFP_21_RXLOS_MASK                       (1 << 2)
#define  CPLD3_SFP_20_RXLOS_MASK                       (1 << 1)
#define  CPLD3_SFP_19_RXLOS_MASK                       (1 << 0)

#define CPLD3_SFP_27_34_RXLOS_MASK_REG                 (0x1C7)
#define  CPLD3_SFP_34_RXLOS_MASK                       (1 << 7)
#define  CPLD3_SFP_33_RXLOS_MASK                       (1 << 6)
#define  CPLD3_SFP_32_RXLOS_MASK                       (1 << 5)
#define  CPLD3_SFP_31_RXLOS_MASK                       (1 << 4)
#define  CPLD3_SFP_30_RXLOS_MASK                       (1 << 3)
#define  CPLD3_SFP_29_RXLOS_MASK                       (1 << 2)
#define  CPLD3_SFP_28_RXLOS_MASK                       (1 << 1)
#define  CPLD3_SFP_27_RXLOS_MASK                       (1 << 0)

#define CPLD3_SFP_35_36_RXLOS_MASK_REG                 (0x1C8)
#define  CPLD3_SFP_36_RXLOS_MASK                       (1 << 1)
#define  CPLD3_SFP_35_RXLOS_MASK                       (1 << 0)

#define CPLD3_SFP_19_26_TXDISABLE_REG                  (0x1D0)
#define  CPLD3_SFP_26_TXDISABLE                        (1 << 7)
#define  CPLD3_SFP_25_TXDISABLE                        (1 << 6)
#define  CPLD3_SFP_24_TXDISABLE                        (1 << 5)
#define  CPLD3_SFP_23_TXDISABLE                        (1 << 4)
#define  CPLD3_SFP_22_TXDISABLE                        (1 << 3)
#define  CPLD3_SFP_21_TXDISABLE                        (1 << 2)
#define  CPLD3_SFP_20_TXDISABLE                        (1 << 1)
#define  CPLD3_SFP_19_TXDISABLE                        (1 << 0)

#define CPLD3_SFP_27_34_TXDISABLE_REG                  (0x1D1)
#define  CPLD3_SFP_34_TXDISABLE                        (1 << 7)
#define  CPLD3_SFP_33_TXDISABLE                        (1 << 6)
#define  CPLD3_SFP_32_TXDISABLE                        (1 << 5)
#define  CPLD3_SFP_31_TXDISABLE                        (1 << 4)
#define  CPLD3_SFP_30_TXDISABLE                        (1 << 3)
#define  CPLD3_SFP_29_TXDISABLE                        (1 << 2)
#define  CPLD3_SFP_28_TXDISABLE                        (1 << 1)
#define  CPLD3_SFP_27_TXDISABLE                        (1 << 0)

#define CPLD3_SFP_35_36_TXDISABLE_REG                  (0x1D2)
#define  CPLD3_SFP_36_TXDISABLE                        (1 << 1)
#define  CPLD3_SFP_35_TXDISABLE                        (1 << 0)

#define CPLD3_SFP_19_26_RS_CONTROL_REG                 (0x1D3)
#define  CPLD3_SFP_26_RS_CONTROL                       (1 << 7)
#define  CPLD3_SFP_25_RS_CONTROL                       (1 << 6)
#define  CPLD3_SFP_24_RS_CONTROL                       (1 << 5)
#define  CPLD3_SFP_23_RS_CONTROL                       (1 << 4)
#define  CPLD3_SFP_22_RS_CONTROL                       (1 << 3)
#define  CPLD3_SFP_21_RS_CONTROL                       (1 << 2)
#define  CPLD3_SFP_20_RS_CONTROL                       (1 << 1)
#define  CPLD3_SFP_19_RS_CONTROL                       (1 << 0)

#define CPLD3_SFP_27_34_RS_CONTROL_REG                 (0x1D4)
#define  CPLD3_SFP_34_RS_CONTROL                       (1 << 7)
#define  CPLD3_SFP_33_RS_CONTROL                       (1 << 6)
#define  CPLD3_SFP_32_RS_CONTROL                       (1 << 5)
#define  CPLD3_SFP_31_RS_CONTROL                       (1 << 4)
#define  CPLD3_SFP_30_RS_CONTROL                       (1 << 3)
#define  CPLD3_SFP_29_RS_CONTROL                       (1 << 2)
#define  CPLD3_SFP_28_RS_CONTROL                       (1 << 1)
#define  CPLD3_SFP_27_RS_CONTROL                       (1 << 0)

#define CPLD3_SFP_35_36_RS_CONTROL_REG                 (0x1D5)
#define  CPLD3_SFP_36_RS_CONTROL                       (1 << 1)
#define  CPLD3_SFP_35_RS_CONTROL                       (1 << 0)

#define CPLD3_SFP_19_26_TXFAULT_REG                    (0x1D6)
#define  CPLD3_SFP_26_TXFAULT                          (1 << 7)
#define  CPLD3_SFP_25_TXFAULT                          (1 << 6)
#define  CPLD3_SFP_24_TXFAULT                          (1 << 5)
#define  CPLD3_SFP_23_TXFAULT                          (1 << 4)
#define  CPLD3_SFP_22_TXFAULT                          (1 << 3)
#define  CPLD3_SFP_21_TXFAULT                          (1 << 2)
#define  CPLD3_SFP_20_TXFAULT                          (1 << 1)
#define  CPLD3_SFP_19_TXFAULT                          (1 << 0)

#define CPLD3_SFP_27_34_TXFAULT_REG                    (0x1D7)
#define  CPLD3_SFP_34_TXFAULT                          (1 << 7)
#define  CPLD3_SFP_33_TXFAULT                          (1 << 6)
#define  CPLD3_SFP_32_TXFAULT                          (1 << 5)
#define  CPLD3_SFP_31_TXFAULT                          (1 << 4)
#define  CPLD3_SFP_30_TXFAULT                          (1 << 3)
#define  CPLD3_SFP_29_TXFAULT                          (1 << 2)
#define  CPLD3_SFP_28_TXFAULT                          (1 << 1)
#define  CPLD3_SFP_27_TXFAULT                          (1 << 0)

#define CPLD3_SFP_35_36_TXFAULT_REG                    (0x1D8)
#define  CPLD3_SFP_36_TXFAULT                          (1 << 1)
#define  CPLD3_SFP_35_TXFAULT                          (1 << 0)

#define CPLD3_SFP_19_26_ABS_REG                        (0x1D9)
#define  CPLD3_SFP_26_ABS                              (1 << 7)
#define  CPLD3_SFP_25_ABS                              (1 << 6)
#define  CPLD3_SFP_24_ABS                              (1 << 5)
#define  CPLD3_SFP_23_ABS                              (1 << 4)
#define  CPLD3_SFP_22_ABS                              (1 << 3)
#define  CPLD3_SFP_21_ABS                              (1 << 2)
#define  CPLD3_SFP_20_ABS                              (1 << 1)
#define  CPLD3_SFP_19_ABS                              (1 << 0)

#define CPLD3_SFP_27_34_ABS_REG                        (0x1DA)
#define  CPLD3_SFP_34_ABS                              (1 << 7)
#define  CPLD3_SFP_33_ABS                              (1 << 6)
#define  CPLD3_SFP_32_ABS                              (1 << 5)
#define  CPLD3_SFP_31_ABS                              (1 << 4)
#define  CPLD3_SFP_30_ABS                              (1 << 3)
#define  CPLD3_SFP_29_ABS                              (1 << 2)
#define  CPLD3_SFP_28_ABS                              (1 << 1)
#define  CPLD3_SFP_27_ABS                              (1 << 0)

#define CPLD3_SFP_35_36_ABS_REG                        (0x1DB)
#define  CPLD3_SFP_36_ABS                              (1 << 1)
#define  CPLD3_SFP_35_ABS                              (1 << 0)

#define CPLD3_PORT_LED_OPERATING_MODE_REG              (0x1E0)
#define  CPLD3_PORT_LED_OPERATING_MODE_MASK            (0x01)
#define   CPLD3_PORT_LED_NORMAL_MODE                   (0x00)
#define   CPLD3_PORT_LED_TEST_MODE                     (0x01)

#define CPLD3_PORT_LED_TEST_REG                        (0x1E1)
#define  CPLD3_PORT_LEDS_AMBER_CONTROL_MASK            (0x02)
#define   CPLD3_ALL_PORTS_LED_AMBER_ON                 (0x00)
#define   CPLD3_ALL_PORTS_LED_AMBER_OFF                (0x02)
#define  CPLD3_PORT_LEDS_GREEN_CONTROL_MASK            (0x01)
#define   CPLD3_ALL_PORTS_LED_GREEN_ON                 (0x00)
#define   CPLD3_ALL_PORTS_LED_GREEN_OFF                (0x01)

/* CPLD4 */

#define CPLD4_VERSION_REG                              (0x280)
#define  CPLD4_CPLD_ID_MASK                            (0xF0)
#define  CPLD4_CPLD_ID_SHIFT                           (4)
#define  CPLD4_CPLD_ID                                 (0x40)
#define  CPLD4_CPLD_MINOR_VERSION_MASK                 (0x0F)
#define  CPLD4_CPLD_MINOR_VERSION_SHIFT                (0)

#define CPLD4_SOFTWARE_SCRATCH_REG                     (0x281)

#define CPLD4_IR_POWER_STATUS_REG                      (0x282)
#define  CPLD4_STATUS_IR3581_VRHOT_ROV_MASK            (0x04)
#define  CPLD4_STATUS_IR3584_3R3V_VRHOT_MASK           (0x02)
#define  CPLD4_STATUS_IR3584_3R3V_VRDT1_MASK           (0x01)

#define CPLD4_I2C_FREQUENCY_DIVIDER_REG                (0x290)
#define  CPLD4_BAUD_RATE_SETTING_MASK                  (0x3F)
#define   CPLD4_BAUD_RATE_50KHZ                        (0x3F)
#define   CPLD4_BAUD_RATE_100KHZ                       (0x1F)
#define   CPLD4_BAUD_RATE_200KHZ                       (0x0F)
#define   CPLD4_BAUD_RATE_400KHZ                       (0x07)

#define CPLD4_I2C_CONTROL_REG                          (0x291)
#define  CPLD4_MODULE_ENABLE_MASK                      (0x80)
#define  CPLD4_MODULE_INTERRUPT_ENABLE_MASK            (0x40)
#define  CPLD4_MASTER_SLAVE_MODE_START_MASK            (0x20)
#define  CPLD4_TRANSMIT_RECEIVE_MODE_SELECT_MASK       (0x10)
#define  CPLD4_TRANSFER_ACKNOWLEDGE_MASK               (0x08)
#define  CPLD4_REPEATED_START_MASK                     (0x04)
#define  CPLD4_BROADCAST_MASK                          (0x01)

#define CPLD4_I2C_STATUS_REG                           (0x292)
#define  CPLD4_DATA_TRANSFER_MASK                      (0x80)
#define  CPLD4_ADDRESS_AS_A_SLAVE_MASK                 (0x40)
#define  CPLD4_BUS_BUSY_MASK                           (0x20)
#define  CPLD4_ARBITRATION_LOST_MASK                   (0x10)
#define  CPLD4_BROADCAST_MATCH_MASK                    (0x08)
#define  CPLD4_SLAVE_READ_WRITE_MASK                   (0x04)
#define  CPLD4_MODULE_INTERRUPT_MASK                   (0x02)
#define  CPLD4_RECEIVE_ACKNOWLEDGE_MASK                (0x01)

#define CPLD4_I2C_DATA_REG                             (0x293)

#define CPLD4_I2C_PORT_ID_REG                          (0x294)
#define  CPLD4_I2C_OPERATION_ID_MASK                   (0x3F)

#define CPLD4_LM75_INT_SOURCE_STATUS_REG               (0x298)
#define  CPLD4_STATUS_LM75_INT_N4_MASK                 (0x08)
#define  CPLD4_STATUS_LM75_INT_N3_MASK                 (0x04)
#define  CPLD4_STATUS_LM75_INT_N2_MASK                 (0x02)
#define  CPLD4_STATUS_LM75_INT_N1_MASK                 (0x01)

#define CPLD4_LM75_INT_SOURCE_INTERRUPT_REG            (0x299)
#define  CPLD4_INTERRUPT_STATUS_LM75_INT_N4_MASK       (0x08)
#define  CPLD4_INTERRUPT_STATUS_LM75_INT_N3_MASK       (0x04)
#define  CPLD4_INTERRUPT_STATUS_LM75_INT_N2_MASK       (0x02)
#define  CPLD4_INTERRUPT_STATUS_LM75_INT_N1_MASK       (0x01)

#define CPLD4_LM75_INT_SOURCE_MASK_REG                 (0x29A)
#define  CPLD4_MASK_LM75_INT_N4_MASK                   (0x08)
#define  CPLD4_MASK_LM75_INT_N3_MASK                   (0x04)
#define  CPLD4_MASK_LM75_INT_N2_MASK                   (0x02)
#define  CPLD4_MASK_LM75_INT_N1_MASK                   (0x01)

#define CPLD4_SFP_37_44_RXLOS_REG                      (0x2C0)
#define  CPLD4_SFP_44_RXLOS                            (1 << 7)
#define  CPLD4_SFP_43_RXLOS                            (1 << 6)
#define  CPLD4_SFP_42_RXLOS                            (1 << 5)
#define  CPLD4_SFP_41_RXLOS                            (1 << 4)
#define  CPLD4_SFP_40_RXLOS                            (1 << 3)
#define  CPLD4_SFP_39_RXLOS                            (1 << 2)
#define  CPLD4_SFP_38_RXLOS                            (1 << 1)
#define  CPLD4_SFP_37_RXLOS                            (1 << 0)

#define CPLD4_SFP_45_48_RXLOS_REG                      (0x2C1)
#define  CPLD4_SFP_48_RXLOS                            (1 << 3)
#define  CPLD4_SFP_47_RXLOS                            (1 << 2)
#define  CPLD4_SFP_46_RXLOS                            (1 << 1)
#define  CPLD4_SFP_45_RXLOS                            (1 << 0)

#define CPLD4_SFP_37_44_RXLOS_INTERRUPT_REG            (0x2C2)
#define  CPLD4_SFP_44_RXLOS_INTERRUPT                  (1 << 7)
#define  CPLD4_SFP_43_RXLOS_INTERRUPT                  (1 << 6)
#define  CPLD4_SFP_42_RXLOS_INTERRUPT                  (1 << 5)
#define  CPLD4_SFP_41_RXLOS_INTERRUPT                  (1 << 4)
#define  CPLD4_SFP_40_RXLOS_INTERRUPT                  (1 << 3)
#define  CPLD4_SFP_39_RXLOS_INTERRUPT                  (1 << 2)
#define  CPLD4_SFP_38_RXLOS_INTERRUPT                  (1 << 1)
#define  CPLD4_SFP_37_RXLOS_INTERRUPT                  (1 << 0)

#define CPLD4_SFP_45_48_RXLOS_INTERRUPT_REG            (0x2C3)
#define  CPLD4_SFP_48_RXLOS_INTERRUPT                  (1 << 3)
#define  CPLD4_SFP_47_RXLOS_INTERRUPT                  (1 << 2)
#define  CPLD4_SFP_46_RXLOS_INTERRUPT                  (1 << 1)
#define  CPLD4_SFP_45_RXLOS_INTERRUPT                  (1 << 0)

#define CPLD4_SFP_37_44_RXLOS_MASK_REG                 (0x2C4)
#define  CPLD4_SFP_44_RXLOS_MASK                       (1 << 7)
#define  CPLD4_SFP_43_RXLOS_MASK                       (1 << 6)
#define  CPLD4_SFP_42_RXLOS_MASK                       (1 << 5)
#define  CPLD4_SFP_41_RXLOS_MASK                       (1 << 4)
#define  CPLD4_SFP_40_RXLOS_MASK                       (1 << 3)
#define  CPLD4_SFP_39_RXLOS_MASK                       (1 << 2)
#define  CPLD4_SFP_38_RXLOS_MASK                       (1 << 1)
#define  CPLD4_SFP_37_RXLOS_MASK                       (1 << 0)

#define CPLD4_SFP_45_48_RXLOS_MASK_REG                 (0x2C5)
#define  CPLD4_SFP_48_RXLOS_MASK                       (1 << 3)
#define  CPLD4_SFP_47_RXLOS_MASK                       (1 << 2)
#define  CPLD4_SFP_46_RXLOS_MASK                       (1 << 1)
#define  CPLD4_SFP_45_RXLOS_MASK                       (1 << 0)

#define CPLD4_SFP_37_44_TXDISABLE_REG                  (0x2D0)
#define  CPLD4_SFP_44_TXDISABLE                        (1 << 7)
#define  CPLD4_SFP_43_TXDISABLE                        (1 << 6)
#define  CPLD4_SFP_42_TXDISABLE                        (1 << 5)
#define  CPLD4_SFP_41_TXDISABLE                        (1 << 4)
#define  CPLD4_SFP_40_TXDISABLE                        (1 << 3)
#define  CPLD4_SFP_39_TXDISABLE                        (1 << 2)
#define  CPLD4_SFP_38_TXDISABLE                        (1 << 1)
#define  CPLD4_SFP_37_TXDISABLE                        (1 << 0)

#define CPLD4_SFP_45_48_TXDISABLE_REG                  (0x2D1)
#define  CPLD4_SFP_48_TXDISABLE                        (1 << 3)
#define  CPLD4_SFP_47_TXDISABLE                        (1 << 2)
#define  CPLD4_SFP_46_TXDISABLE                        (1 << 1)
#define  CPLD4_SFP_45_TXDISABLE                        (1 << 0)

#define CPLD4_SFP_37_44_RS_CONTROL_REG                 (0x2D2)
#define  CPLD4_SFP_44_RS_CONTROL                       (1 << 7)
#define  CPLD4_SFP_43_RS_CONTROL                       (1 << 6)
#define  CPLD4_SFP_42_RS_CONTROL                       (1 << 5)
#define  CPLD4_SFP_41_RS_CONTROL                       (1 << 4)
#define  CPLD4_SFP_40_RS_CONTROL                       (1 << 3)
#define  CPLD4_SFP_39_RS_CONTROL                       (1 << 2)
#define  CPLD4_SFP_38_RS_CONTROL                       (1 << 1)
#define  CPLD4_SFP_37_RS_CONTROL                       (1 << 0)

#define CPLD4_SFP_45_48_RS_CONTROL_REG                 (0x2D3)
#define  CPLD4_SFP_48_RS_CONTROL                       (1 << 3)
#define  CPLD4_SFP_47_RS_CONTROL                       (1 << 2)
#define  CPLD4_SFP_46_RS_CONTROL                       (1 << 1)
#define  CPLD4_SFP_45_RS_CONTROL                       (1 << 0)

#define CPLD4_SFP_37_44_TXFAULT_REG                    (0x2D4)
#define  CPLD4_SFP_44_TXFAULT                          (1 << 7)
#define  CPLD4_SFP_43_TXFAULT                          (1 << 6)
#define  CPLD4_SFP_42_TXFAULT                          (1 << 5)
#define  CPLD4_SFP_41_TXFAULT                          (1 << 4)
#define  CPLD4_SFP_40_TXFAULT                          (1 << 3)
#define  CPLD4_SFP_39_TXFAULT                          (1 << 2)
#define  CPLD4_SFP_38_TXFAULT                          (1 << 1)
#define  CPLD4_SFP_37_TXFAULT                          (1 << 0)

#define CPLD4_SFP_45_48_TXFAULT_REG                    (0x2D5)
#define  CPLD4_SFP_48_TXFAULT                          (1 << 3)
#define  CPLD4_SFP_47_TXFAULT                          (1 << 2)
#define  CPLD4_SFP_46_TXFAULT                          (1 << 1)
#define  CPLD4_SFP_45_TXFAULT                          (1 << 0)

#define CPLD4_SFP_37_44_ABS_REG                        (0x2D6)
#define  CPLD4_SFP_44_ABS                              (1 << 7)
#define  CPLD4_SFP_43_ABS                              (1 << 6)
#define  CPLD4_SFP_42_ABS                              (1 << 5)
#define  CPLD4_SFP_41_ABS                              (1 << 4)
#define  CPLD4_SFP_40_ABS                              (1 << 3)
#define  CPLD4_SFP_39_ABS                              (1 << 2)
#define  CPLD4_SFP_38_ABS                              (1 << 1)
#define  CPLD4_SFP_37_ABS                              (1 << 0)

#define CPLD4_SFP_45_48_ABS_REG                        (0x2D7)
#define  CPLD4_SFP_48_ABS                              (1 << 3)
#define  CPLD4_SFP_47_ABS                              (1 << 2)
#define  CPLD4_SFP_46_ABS                              (1 << 1)
#define  CPLD4_SFP_45_ABS                              (1 << 0)

#define CPLD4_QSFP28_1_4_RESET_REG                     (0x2E0)
#define  CPLD4_QSFP28_1_4_RESET_MASK                   (0x0F)
#define  CPLD4_QSFP28_4_RESET                          (1 << 3)
#define  CPLD4_QSFP28_3_RESET                          (1 << 2)
#define  CPLD4_QSFP28_2_RESET                          (1 << 1)
#define  CPLD4_QSFP28_1_RESET                          (1 << 0)

#define CPLD4_QSFP28_1_4_LPMOD_REG                     (0x2E1)
#define  CPLD4_QSFP28_1_4_LPMOD_MASK                   (0x0F)
#define  CPLD4_QSFP28_4_LPMOD                          (1 << 3)
#define  CPLD4_QSFP28_3_LPMOD                          (1 << 2)
#define  CPLD4_QSFP28_2_LPMOD                          (1 << 1)
#define  CPLD4_QSFP28_1_LPMOD                          (1 << 0)

#define CPLD4_QSFP28_1_4_ABS_REG                       (0x2E2)
#define  CPLD4_QSFP28_1_4_ABS_MASK                     (0x0F)
#define  CPLD4_QSFP28_4_ABS                            (1 << 3)
#define  CPLD4_QSFP28_3_ABS                            (1 << 2)
#define  CPLD4_QSFP28_2_ABS                            (1 << 1)
#define  CPLD4_QSFP28_1_ABS                            (1 << 0)

#define CPLD4_QSFP28_1_4_INT_N_REG                     (0x2E3)
#define  CPLD4_QSFP28_1_4_INT_N_MASK                   (0x0F)
#define  CPLD4_QSFP28_4_INT_N                          (1 << 3)
#define  CPLD4_QSFP28_3_INT_N                          (1 << 2)
#define  CPLD4_QSFP28_2_INT_N                          (1 << 1)
#define  CPLD4_QSFP28_1_INT_N                          (1 << 0)

#define CPLD4_QSFP28_1_4_INT_N_INTERRUPT_REG           (0x2E4)
#define  CPLD4_QSFP28_4_INT_N_INTERRUPT                (1 << 3)
#define  CPLD4_QSFP28_3_INT_N_INTERRUPT                (1 << 2)
#define  CPLD4_QSFP28_2_INT_N_INTERRUPT                (1 << 1)
#define  CPLD4_QSFP28_1_INT_N_INTERRUPT                (1 << 0)

#define CPLD4_QSFP28_1_4_INT_N_MASK_REG                (0x2E5)
#define  CPLD4_QSFP28_4_INT_N_MASK                     (1 << 3)
#define  CPLD4_QSFP28_3_INT_N_MASK                     (1 << 2)
#define  CPLD4_QSFP28_2_INT_N_MASK                     (1 << 1)
#define  CPLD4_QSFP28_1_INT_N_MASK                     (1 << 0)

#define CPLD4_QSFP28_1_4_I2C_READY_REG                 (0x2E6)
#define  CPLD4_QSFP28_4_I2C_READY                      (1 << 3)
#define  CPLD4_QSFP28_3_I2C_READY                      (1 << 2)
#define  CPLD4_QSFP28_2_I2C_READY                      (1 << 1)
#define  CPLD4_QSFP28_1_I2C_READY                      (1 << 0)

#define CPLD4_PORT_LED_OPERATING_MODE_REG              (0x2F0)
#define  CPLD4_PORT_LED_OPERATING_MODE_MASK            (0x01)
#define   CPLD4_PORT_LED_NORMAL_MODE                   (0x00)
#define   CPLD4_PORT_LED_TEST_MODE                     (0x01)

#define CPLD4_PORT_LED_TEST_REG                        (0x2F1)
#define  CPLD4_PORT_LEDS_AMBER_CONTROL_MASK            (0x02)
#define   CPLD4_ALL_PORTS_LED_AMBER_ON                 (0x00)
#define   CPLD4_ALL_PORTS_LED_AMBER_OFF                (0x02)
#define  CPLD4_PORT_LEDS_GREEN_CONTROL_MASK            (0x01)
#define   CPLD4_ALL_PORTS_LED_GREEN_ON                 (0x00)
#define   CPLD4_ALL_PORTS_LED_GREEN_OFF                (0x01)

#endif /* CEL_REDSTONE_V_H__ */
