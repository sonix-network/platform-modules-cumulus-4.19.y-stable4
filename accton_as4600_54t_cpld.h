/*
 * Accton as4600_54t CPLD Platform Definitions
 *
 * Curt Brune <curt@cumulusnetworks.com>
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

#ifndef ACCTON_AS4600_54T_H__
#define ACCTON_AS4600_54T_H__

/*
 * Begin register defines.
 */

#define CPLD_REG_VERSION           (0x00)
#  define CPLD_VERSION_H_MASK        (0xF0)
#  define CPLD_VERSION_H_SHIFT       (4)
#  define CPLD_VERSION_L_MASK        (0x0F)
#  define CPLD_VERSION_L_SHIFT       (0)

#define CPLD_REG_RESET_CTRL_0      (0x01) /* all resets active low */
#  define CPLD_RESET_SYSTEM_L		(1 << 7)
#  define CPLD_RESET_CPU_HRESET_L	(1 << 6)
#  define CPLD_RESET_CPU_SRESET_L	(1 << 5)
#  define CPLD_RESET_CPU_TRST_L		(1 << 4)
#  define CPLD_RESET_NOR_FLASH_L	(1 << 3)
#  define CPLD_RESET_USB_PHY_L		(1 << 1)
#  define CPLD_RESET_BCM54616S_L	(1 << 0) /* Front panel mgmt PHY */

#define CPLD_REG_RESET_CTRL_1      (0x02) /* all resets active low */
#  define CPLD_RESET_I2C_BUFFER_L	(1 << 7)
#  define CPLD_RESET_RTC_L		(1 << 6)
#  define CPLD_RESET_POE_CONTROLLER_L	(1 << 5)
#  define CPLD_RESET_LED_SHIFTER_L	(1 << 4)
#  define CPLD_RESET_BCM56540_L		(1 << 3) /* Switch ASIC */
#  define CPLD_RESET_BCM84754_L		(1 << 2) /* 4x10G PHY */
#  define CPLD_RESET_BCM54280_0_L	(1 << 1) /* 8x1G PHY BCM54280 0, 1, 2 */
#  define CPLD_RESET_BCM54280_1_L	(1 << 0) /* 8x1G PHY BCM54280 3, 4, 5 */

#define CPLD_REG_RESET_CTRL_2      (0x03) /* all resets active low */
#  define CPLD_RESET_MODULE_0_L		(1 << 7)
#  define CPLD_RESET_MODULE_1_L		(1 << 6)
#  define CPLD_RESET_MODULE_0_QSFP_L	(1 << 5)
#  define CPLD_RESET_MODULE_1_QSFP_L	(1 << 4)
#  define CPLD_RESET_DDR_GROUP0_L	(1 << 3)
#  define CPLD_RESET_DDR_GROUP1_L	(1 << 2)
#  define CPLD_RESET_USB_FLASH_CTRL_L	(1 << 1)
#  define CPLD_RESET_USB_HUB_L		(1 << 0)

/* LEDs are active low */
#define CPLD_REG_SYSTEM_LED_CTRL_0  (0x04)
#  define CPLD_SYS_LED_DIAG_MASK	(0xC0)
#    define CPLD_SYS_LED_DIAG_GREEN			(0x40)
#    define CPLD_SYS_LED_DIAG_AMBER			(0x80)
#    define CPLD_SYS_LED_DIAG_OFF			(0xC0)
#  define CPLD_SYS_LED_POE_MASK		(0x30)
#    define CPLD_SYS_LED_POE_GREEN			(0x10)
#    define CPLD_SYS_LED_POE_AMBER			(0x20)
#    define CPLD_SYS_LED_POE_OFF			(0x30)
#  define CPLD_SYS_LED_PWR_0_MASK	(0x0C)
#    define CPLD_SYS_LED_PWR_0_GREEN		(0x04)
#    define CPLD_SYS_LED_PWR_0_AMBER		(0x08)
#    define CPLD_SYS_LED_PWR_0_OFF			(0x0C)
#  define CPLD_SYS_LED_PWR_1_MASK	(0x03)
#    define CPLD_SYS_LED_PWR_1_GREEN		(0x01)
#    define CPLD_SYS_LED_PWR_1_AMBER		(0x02)
#    define CPLD_SYS_LED_PWR_1_OFF			(0x03)

#define CPLD_REG_SYSTEM_LED_CTRL_1  (0x05)
#  define CPLD_SYS_LED_FAN_1_MASK	(0xC0)
#    define CPLD_SYS_LED_FAN_1_GREEN		(0x40)
#    define CPLD_SYS_LED_FAN_1_AMBER		(0x80)
#    define CPLD_SYS_LED_FAN_1_OFF			(0xC0)
#  define CPLD_SYS_LED_FAN_2_MASK	(0x30)
#    define CPLD_SYS_LED_FAN_2_GREEN		(0x10)
#    define CPLD_SYS_LED_FAN_2_AMBER		(0x20)
#    define CPLD_SYS_LED_FAN_2_OFF			(0x30)
#  define CPLD_SYS_LED_MODULE_0_MASK	(0x0C)
#    define CPLD_SYS_LED_MODULE_0_GREEN		(0x04)
#    define CPLD_SYS_LED_MODULE_0_AMBER		(0x08)
#    define CPLD_SYS_LED_MODULE_0_OFF		(0x08)
#  define CPLD_SYS_LED_MODULE_1_MASK	(0x03)
#    define CPLD_SYS_LED_MODULE_1_GREEN		(0x01)
#    define CPLD_SYS_LED_MODULE_1_AMBER		(0x02)
#    define CPLD_SYS_LED_MODULE_1_OFF		(0x03)

#define CPLD_REG_SYSTEM_LED_CTRL_2  (0x06)
#  define CPLD_SYS_LED_STACKING_1_MASK	(0xC0)
#    define CPLD_SYS_LED_STACKING_GREEN		(0x40)
#    define CPLD_SYS_LED_STACKING_AMBER		(0x80)
#    define CPLD_SYS_LED_STACKING_OFF		(0xC0)
#  define CPLD_SYS_LED_STACK_LINK_MASK	(0x30)
#    define CPLD_SYS_LED_STACK_LINK_GREEN	(0x10)
#    define CPLD_SYS_LED_STACK_LINK_AMBER	(0x20)

#define CPLD_REG_SYS_CONTROL		(0x07)
#  define CPLD_WDT_ENABLE			(1 << 7)
#  define CPLD_WDT_STATUS			(1 << 6)
#  define CPLD_WDT_KICK				(1 << 5)

#define CPLD_REG_7SEGMENT_LED		(0x09)

#define CPLD_REG_SFPP_PRESENT		(0x0a)
#  define CPLD_SFPP_PRESENT_PORT_0		(1 << 7)
#  define CPLD_SFPP_PRESENT_PORT_1		(1 << 6)
#  define CPLD_SFPP_PRESENT_PORT_2		(1 << 5)
#  define CPLD_SFPP_PRESENT_PORT_3		(1 << 4)

#define CPLD_REG_SFPP_RX_LOS		(0x0b)
#  define CPLD_SFPP_RX_LOS_PORT_0		(1 << 7)
#  define CPLD_SFPP_RX_LOS_PORT_1		(1 << 6)
#  define CPLD_SFPP_RX_LOS_PORT_2		(1 << 5)
#  define CPLD_SFPP_RX_LOS_PORT_3		(1 << 4)

#define CPLD_REG_SFPP_TX_FAIL		(0x0c)
#  define CPLD_SFPP_TX_FAIL_PORT_0		(1 << 7)
#  define CPLD_SFPP_TX_FAIL_PORT_1		(1 << 6)
#  define CPLD_SFPP_TX_FAIL_PORT_2		(1 << 5)
#  define CPLD_SFPP_TX_FAIL_PORT_3		(1 << 4)

#define CPLD_REG_SFPP_TX_DISABLE	(0x0d)
#  define CPLD_SFPP_TX_DISABLE_PORT_0		(1 << 7)
#  define CPLD_SFPP_TX_DISABLE_PORT_1		(1 << 6)
#  define CPLD_SFPP_TX_DISABLE_PORT_2		(1 << 5)
#  define CPLD_SFPP_TX_DISABLE_PORT_3		(1 << 4)

#define CPLD_REG_SFPP_SPEED		(0x0e)
#  define CPLD_SFPP_SPEED_PORT_0		(1 << 7)  /* 0 - 1G, 1 - 10G */
#  define CPLD_SFPP_SPEED_PORT_1		(1 << 6)
#  define CPLD_SFPP_SPEED_PORT_2		(1 << 5)
#  define CPLD_SFPP_SPEED_PORT_3		(1 << 4)

#define CPLD_REG_FAN_STATUS		(0x0f)
#  define CPLD_FAN_PRESENT_0_L			(1 << 7) /* 0 - present */
#  define CPLD_FAN_PRESENT_1_L			(1 << 6) /* 1 - not present */
#  define CPLD_FAN_DIRECTION_0_L		(1 << 5) /* 0 - back to front */
#  define CPLD_FAN_DIRECTION_1_L		(1 << 4) /* 1 - front to back */

#define CPLD_REG_PUSH_BUTTON_STATUS	(0x10)
#  define CPLD_PB_STATUS_NON_STACKING		(1 << 7)
#  define CPLD_PB_STATUS_STACKING_MASTER	(1 << 6)
#  define CPLD_PB_STATUS_STACKING_SLAVE		(1 << 5)

#define CPLD_REG_POE_STATUS		(0x11)
#  define CPLD_POE_SYSTEM_VALID_L		(1 << 7)

#define CPLD_REG_POE_CONTROL		(0x12)
#  define CPLD_POE_ENABLE			(1 << 7)

#define CPLD_REG_PSU_0_STATUS		(0x13)
#  define CPLD_PSU_PRESENT_L			(1 << 7)
#  define CPLD_PSU_AC_FAIL_L			(1 << 6) /* 0 - input fail */
#  define CPLD_PSU_56V_GOOD_L			(1 << 5)
#  define CPLD_PSU_12V_GOOD_L			(1 << 4)
#  define CPLD_PSU_FAN_FAIL_L			(1 << 3) /* 0 - fan fail */
#  define CPLD_PSU_ALERT_L			(1 << 2) /* 0 - need to read module EEPROM */

#define CPLD_REG_PSU_1_STATUS		(0x14)
#  define CPLD_PSU_PRESENT_L			(1 << 7)
#  define CPLD_PSU_AC_FAIL_L			(1 << 6) /* 0 - input fail */
#  define CPLD_PSU_56V_GOOD_L			(1 << 5)
#  define CPLD_PSU_12V_GOOD_L			(1 << 4)
#  define CPLD_PSU_FAN_FAIL_L			(1 << 3) /* 0 - fan fail */
#  define CPLD_PSU_ALERT_L			(1 << 2) /* 0 - need to read module EEPROM */

#define CPLD_REG_RPS_POWER_STATUS	(0x15)
#  define CPLD_RPS_PRESENT_L			(1 << 7)
#  define CPLD_RPS_FAIL_L			(1 << 6) /* 0 - RPS fail */

#define CPLD_REG_MODULE_0_STATUS	(0x16)
#  define CPLD_MODULE_PRESENT_L			(1 << 7)

#define CPLD_REG_MODULE_1_STATUS	(0x17)
#  define CPLD_MODULE_PRESENT_L			(1 << 7)

/*
 * End register defines.
 */

/*
** Define a more programmer friendly format for the port control and
** status registers.
**
** 5-bits of control/status for each port 49 - 52, packed into a
** uint32_t.
**
** [00]:RO - port 49 sfp_present -- Read-Only. 0 - not present, 1 - present
** [01]:RO - port 49 rx_los      -- Read-Only. 0 - link up, 1 - link down
** [02]:RO - port 49 tx_fail     -- Read-Only. 0 - link up, 1 - link down
** [03]:RW - port 49 tx_enable   -- 0 - disable, 1 - enable
** [04]:RW - port 49 fiber mode  -- 0 - 1G, 1 - 10G
** [05]:RO - port 50 sfp_present -- Read-Only. 0 - not present, 1 - present
** [06]:RO - port 50 rx_los      -- Read-Only. 0 - link up, 1 - link down
** [07]:RO - port 50 tx_fail     -- Read-Only. 0 - link up, 1 - link down
** [08]:RW - port 50 tx_enable   -- 0 - disable, 1 - enable
** [09]:RW - port 50 fiber mode  -- 0 - 1G, 1 - 10G
** [10]:RO - port 51 sfp_present -- Read-Only. 0 - not present, 1 - present
** [11]:RO - port 51 rx_los      -- Read-Only. 0 - link up, 1 - link down
** [12]:RO - port 51 tx_fail     -- Read-Only. 0 - link up, 1 - link down
** [13]:RW - port 51 tx_enable   -- 0 - disable, 1 - enable
** [14]:RW - port 51 fiber mode  -- 0 - 1G, 1 - 10G
** [15]:RO - port 52 sfp_present -- Read-Only. 0 - not present, 1 - present
** [16]:RO - port 52 rx_los      -- Read-Only. 0 - link up, 1 - link down
** [17]:RO - port 52 tx_fail     -- Read-Only. 0 - link up, 1 - link down
** [18]:RW - port 52 tx_enable   -- 0 - disable, 1 - enable
** [19]:RW - port 52 fiber mode  -- 0 - 1G, 1 - 10G
*/

/* Internal routine */
#define TYPE_PRESENT    (0)
#define TYPE_RX_LOS     (1)
#define TYPE_TX_FAIL    (2)
#define TYPE_TX_CTRL    (3)
#define TYPE_TX_SPEED   (4)

static inline uint32_t
__accton_as4600_54t_get_n( uint32_t val, uint16_t port, uint16_t offset)
{
#ifdef __KERNEL__
	BUG_ON( (port < 49) || (port > 52));
#else
	assert( (port >= 49) && (port <= 52));
#endif
	return (val & (1 << (offset + (5 * (port - 49))))) ? 1 : 0;
}

/* External get SFP present */
static inline uint32_t
accton_as4600_54t_get_sfp_present( uint32_t val, uint16_t port)
{
	return __accton_as4600_54t_get_n( val, port, TYPE_PRESENT);
}

/* External get rx los */
static inline uint32_t
accton_as4600_54t_get_rx_los( uint32_t val, uint16_t port)
{
	return __accton_as4600_54t_get_n( val, port, TYPE_RX_LOS);
}

/* External get tx fail */
static inline uint32_t
accton_as4600_54t_get_tx_fail( uint32_t val, uint16_t port)
{
	return __accton_as4600_54t_get_n( val, port, TYPE_TX_FAIL);
}

/* External get tx enable */
static inline uint32_t
accton_as4600_54t_get_tx_enable( uint32_t val, uint16_t port)
{
	return __accton_as4600_54t_get_n( val, port, TYPE_TX_CTRL);
}

/* External get transmit speed routine */
static inline uint32_t
accton_as4600_54t_get_tx_speed_10g( uint32_t val, uint16_t port)
{
	return __accton_as4600_54t_get_n( val, port, TYPE_TX_SPEED);
}

/*
** Set bit routines
*/

/* Internal routine */
static inline void
__accton_as4600_54t_set_n( uint32_t* val, uint16_t port, uint16_t bit, uint16_t offset)
{
#ifdef __KERNEL__
	BUG_ON( (port < 49) || (port > 52));
#else
	assert( (port >= 49) && (port <= 52));
#endif

	if (bit) {
		// set it
		*val |= (1 << (offset + (5 * (port - 49))));
	}
	else {
		// clear it
		*val &= ~(1 << (offset + (5 * (port - 49))));
	}
}

/* External set SFP present */
static inline void
accton_as4600_54t_set_sfp_present( uint32_t* val, uint16_t port, uint16_t bit)
{
	__accton_as4600_54t_set_n( val, port, bit, TYPE_PRESENT);
}

/* External set rx los */
static inline void
accton_as4600_54t_set_rx_los( uint32_t* val, uint16_t port, uint16_t bit)
{
	__accton_as4600_54t_set_n( val, port, bit, TYPE_RX_LOS);
}

/* External set tx fail */
static inline void
accton_as4600_54t_set_tx_fail( uint32_t* val, uint16_t port, uint16_t bit)
{
	__accton_as4600_54t_set_n( val, port, bit, TYPE_TX_FAIL);
}

/* External set tx enable */
static inline void
accton_as4600_54t_set_tx_enable( uint32_t* val, uint16_t port, uint16_t bit)
{
	__accton_as4600_54t_set_n( val, port, bit, TYPE_TX_CTRL);
}

/* External set transmit speed routine */
static inline void
accton_as4600_54t_set_tx_speed_10g( uint32_t* val, uint16_t port, uint16_t bit)
{
	__accton_as4600_54t_set_n( val, port, bit, TYPE_TX_SPEED);
}

#endif /* ACCTON_AS4600_54T_H__ */
