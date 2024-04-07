/*
 * Accton AS670x-32x SYSPLD Platform Definitions
 *
 * Dustin Byford <dustin@cumulusnetworks.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef ACCTON_AS670X_32X_SYSPLD_H__
#define ACCTON_AS670X_32X_SYSPLD_H__

/*
 * Begin register defines.
 */

#define CPLD_REG_VERSION		(0x00) /* read only */
#  define CPLD_VERSION_CODE_REL_MASK		(0x80)
#  define CPLD_VERSION_CODE_MASK		(0x70)
#  define CPLD_VERSION_CODE_SHIFT		(4)
#  define CPLD_VERSION_HW_REL_MASK		(0x08)
#  define CPLD_VERSION_HW_MASK			(0x07)
#  define CPLD_VERSION_HW_SHIFT			(0)

#define CPLD_REG_RESET_STATUS      	(0x02) /* read only */
#  define CPLD_RESET_STATUS_POR_RESET_L		(1 << 1)

#define CPLD_REG_PSU_STATUS        	(0x02) /* read only */
#  define CPLD_PSU_STATUS_PS2_PRESENT_L		(1 << 2)
#  define CPLD_PSU_STATUS_PS2_POWER_FAIL	(1 << 3)
#  define CPLD_PSU_STATUS_PS2_12V_PG		(1 << 4)
#  define CPLD_PSU_STATUS_PS1_PRESENT_L		(1 << 5)
#  define CPLD_PSU_STATUS_PS1_POWER_FAIL	(1 << 6)
#  define CPLD_PSU_STATUS_PS1_12V_PG		(1 << 7)

#define CPLD_REG_MODULE_PRESENT		(0x03) /* read only */
#  define CPLD_MODULE_PRESENT_M1_PRESENT_L	(1 << 0)
#  define CPLD_MODULE_PRESENT_M2_PRESENT_L	(1 << 1)

/*
 * Three registers concatenated for the first 20 front panel switch ports,
 * module ports are not represented here.
 *
 * One active low bit per port.
 */
#define CPLD_REG_QSFP_PRESENT_L_1	(0x04) /* read only */
#define CPLD_REG_QSFP_PRESENT_L_2	(0x05) /* read only */
#define CPLD_REG_QSFP_PRESENT_L_3	(0x06) /* read only */
/*
 * Three registers concatenated for the first 20 front panel switch ports,
 * module ports are not represented here.
 *
 * One active low bit per port.
 */
#define CPLD_REG_QSFP_IRQ_L_1		(0x07) /* read only */
#define CPLD_REG_QSFP_IRQ_L_2		(0x08) /* read only */
#define CPLD_REG_QSFP_IRQ_L_3		(0x09) /* read only */

#define CPLD_REG_MODULE_STATUS		(0x0d) /* read only */
#  define CPLD_MODULE_STATUS_M1_MASK		(0x03)
#  define CPLD_MODULE_STATUS_M1_SHIFT		(0)
#  define CPLD_MODULE_STATUS_M2_MASK		(0x0c)
#  define CPLD_MODULE_STATUS_M2_SHIFT		(2)
#  define CPLD_MODULE_STATUS_M1_PWR_OK		(1 << 4)
#  define CPLD_MODULE_STATUS_M2_PWR_OK		(1 << 5)

#  define CPLD_MODULE_STATUS_2CXP		(0)
#  define CPLD_MODULE_STATUS_2CFP		(1)
#  define CPLD_MODULE_STATUS_4TENGBASET		(2)
#  define CPLD_MODULE_STATUS_6QSFP		(3)

#define CPLD_REG_MODULE_PWR_CONTROL_M1	(0x21) /* read write */
#  define CPLD_MODULE_PWR_CONTROL_M1_EN		(1 << 4)
#define CPLD_REG_MODULE_PWR_CONTROL_M2	(0x22) /* read write */
#  define CPLD_MODULE_PWR_CONTROL_M2_EN		(1 << 4)

/*
 * Three registers concatenated for the first 20 front panel switch ports,
 * module ports are not represented here.
 *
 * One active low bit per port.  Default is 1.
 *
 * The last 4 bits in this range control the QSFP voltage regulator.
 */
#define CPLD_REG_QSFP_RST_L_1		(0x24) /* read write */
#define CPLD_REG_QSFP_RST_L_2		(0x25) /* read write */
#define CPLD_REG_QSFP_RST_L_3		(0x26) /* read write */

/*
 * Three registers concatenated for the first 20 front panel switch ports,
 * module ports are not represented here.
 *
 * One bit per port.  Default is 0.
 */
#define CPLD_REG_QSFP_LPMODE_1		(0x27) /* read write */
#define CPLD_REG_QSFP_LPMODE_2		(0x28) /* read write */
#define CPLD_REG_QSFP_LPMODE_3		(0x29) /* read write */

/*
 * Three registers concatenated for the first 20 front panel switch ports,
 * module ports are not represented here.
 *
 * One active low bit per port.  Default is 0.
 */
#define CPLD_REG_QSFP_MODSEL_L_1	(0x2a) /* read write */
#define CPLD_REG_QSFP_MODSEL_L_2	(0x2b) /* read write */
#define CPLD_REG_QSFP_MODSEL_L_3	(0x2c) /* read write */

#define CPLD_REG_SYS_ALARM_LED		(0x2d) /* read write */
#  define CPLD_SYS_ALARM_LED_SYS_LED0		(1 << 0)
#  define CPLD_SYS_ALARM_LED_SYS_LED1		(1 << 1)
#  define CPLD_SYS_ALARM_LED_SYS_MASK		(CPLD_SYS_ALARM_LED_SYS_LED0|CPLD_SYS_ALARM_LED_SYS_LED1)
#  define CPLD_SYS_ALARM_LED_ALRM_LED0		(1 << 2)
#  define CPLD_SYS_ALARM_LED_ALRM_LED1		(1 << 3)
#  define CPLD_SYS_ALARM_LED_ALRM_MASK		(CPLD_SYS_ALARM_LED_ALRM_LED0|CPLD_SYS_ALARM_LED_ALRM_LED1)
#  define CPLD_SYS_ALARM_LED_SYS_GREEN_BLINK	(CPLD_SYS_ALARM_LED_SYS_LED0)
#  define CPLD_SYS_ALARM_LED_SYS_GREEN		(CPLD_SYS_ALARM_LED_SYS_LED1)
#  define CPLD_SYS_ALARM_LED_ALRM_RED_BLINK	(CPLD_SYS_ALARM_LED_ALRM_LED0)
#  define CPLD_SYS_ALARM_LED_ALRM_GREEN		(CPLD_SYS_ALARM_LED_ALRM_LED1)

#define CPLD_REG_I2C_SWITCH_CONTROL	(0x60) /* read write */
/*
 * This is awesome.  Can I call this hex-coded decimal?
 */
#  define CPLD_REG_I2C_SWITCH_CONTROL_DISABLE	(0x0)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT1	(0x1)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT2	(0x2)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT3	(0x3)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT4	(0x4)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT5	(0x5)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT6	(0x6)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT7	(0x7)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT8	(0x8)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT9	(0x9)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT10	(0x10)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT11	(0x11)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT12	(0x12)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT13	(0x13)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT14	(0x14)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT15	(0x15)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT16	(0x16)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT17	(0x17)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT18	(0x18)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT19	(0x19)
#  define CPLD_REG_I2C_SWITCH_CONTROL_PORT20	(0x20)
#  define CPLD_REG_I2C_SWITCH_CONTROL_M1	(0x21)
#  define CPLD_REG_I2C_SWITCH_CONTROL_M2	(0x22)
#define CPLD_I2C_SWITCH_NUM_CHANNELS	(22)

/*
 * End register defines.
 */

#endif /* ACCTON_AS670X_32X_SYSPLD_H__ */
