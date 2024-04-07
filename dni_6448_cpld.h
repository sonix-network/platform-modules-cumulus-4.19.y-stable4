/*
 * DNI 6448 CPLD Platform Definitions
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

#ifndef DNI_6448_H__
#define DNI_6448_H__

/*
 * Begin register defines.
 */

#define CPLD_REG_REVISION	  (0x01000)

#define CPLD_REG_RESET		  (0x02000) // all resets active low
#  define CPLD_RESET_10G_SLOT_B_L	(7)
#  define CPLD_RESET_SYSTEM_L		(5)
#  define CPLD_RESET_CPU_L		(4)
#  define CPLD_RESET_BCM56634_L		(3)
#  define CPLD_RESET_BCM5481_L		(2)
#  define CPLD_RESET_BCM54680_PHY_L	(1)
#  define CPLD_RESET_10G_SLOT_A_L	(0)

#define CPLD_REG_DEBUG_LED	  (0x03000)

#define CPLD_REG_SPF_PLUS_TX_CTRL (0x04000)
#  define CPLD_TX_DIS_49	      (1 << 0)
#  define CPLD_TX_DIS_50	      (1 << 1)
#  define CPLD_TX_DIS_51	      (1 << 2)
#  define CPLD_TX_DIS_52	      (1 << 3)

#define CPLD_REG_SPF_PLUS_STATUS  (0x05000)
#  define CPLD_RX_LOS_49	      (1 << 0)
#  define CPLD_RX_LOS_50	      (1 << 1)
#  define CPLD_SFP_PRESENT_L_49	      (1 << 2)
#  define CPLD_SFP_PRESENT_L_50	      (1 << 3)
#  define CPLD_RX_LOS_51	      (1 << 4)
#  define CPLD_RX_LOS_52	      (1 << 5)
#  define CPLD_SFP_PRESENT_L_51	      (1 << 6)
#  define CPLD_SFP_PRESENT_L_52	      (1 << 7)

#define CPLD_REG_FLASH_PROTECT	  (0x06000)
#  define CPLD_FLASH_WP		  (1)
#  define CPLD_FLASH_VPP	  (0)

#define CPLD_REG_PRODUCT_ID	  (0x07000)

#define CPLD_REG_FAN_TRAY	  (0x08000)
#  define CPLD_FAN2_PRESENT_L        (1 << 3)
#  define CPLD_FAN1_PRESENT_L        (1 << 1)

#define CPLD_REG_POE_PLUS_CTRL_RESET    (0x09000)
#  define CPLD_POE_PLUS_PWR_ENABLE  (1 << 1)
#  define CPLD_POE_PLUS_RESET_L     (1 << 0)

#define CPLD_REG_SYSTEM_LED	  (0x0a000)
#  define CPLD_SYS_LED_LOCATOR_MSK   (0x30)
#  define CPLD_SYS_LED_LOCATOR_SHIFT (4)
#    define CPLD_SYS_LED_LOCATOR_GREEN          (0x00)
#    define CPLD_SYS_LED_LOCATOR_GREEN_BLINKING (0x10)
#    define CPLD_SYS_LED_LOCATOR_OFF            (0x30)
#  define CPLD_SYS_LED_FAN	     (3)
#    define CPLD_SYS_LED_FAN_GREEN          (1 << 3)
#    define CPLD_SYS_LED_FAN_RED            (0)
#  define CPLD_SYS_LED_MASTER	     (2)
#    define CPLD_SYS_LED_MASTER_GREEN       (1 << 2)
#    define CPLD_SYS_LED_MASTER_OFF         (0)
#  define CPLD_SYS_LED_STATUS_MSK    (0x03)
#  define CPLD_SYS_LED_STATUS_SHIFT  (0)
#    define CPLD_SYS_LED_STATUS_RED            (0)
#    define CPLD_SYS_LED_STATUS_RED_BLINKING   (1)
#    define CPLD_SYS_LED_STATUS_GREEN_BLINKING (2)
#    define CPLD_SYS_LED_STATUS_GREEN          (3)

#define CPLD_REG_7SEGMENT_LED_MSB (0x0b000)
#define CPLD_REG_7SEGMENT_LED_LSB (0x0c000)
#  define CPLD_7SEGMENT_BLINK        (1 << 5)
#  define CPLD_7SEGMENT_ON           (1 << 4)
#  define CPLD_7SEGMENT_COLOR_MASK   (0x30)
#  define CPLD_7SEGMENT_DIGIT_MASK   (0xF)

#define CPLD_REG_PORT_MUX	  (0x0d000)
#  define CPLD_PORT_MUX_45            (1 << 3)
#  define CPLD_PORT_MUX_46            (1 << 2)
#  define CPLD_PORT_MUX_47            (1 << 1)
#  define CPLD_PORT_MUX_48            (1 << 0)

#define CPLD_REG_I2C_1_CTRL	  (0x0e000)
#  define CPLD_I2C_1_EN_L	     (4)
#  define CPLD_I2C_1_SELECT_MSK	     (0x3)

#define CPLD_REG_I2C_2_CTRL	  (0x0f000)
#  define CPLD_I2C_2_SELECT_MSK	     (0x7)

#define CPLD_REG_SPF_TX_CTRL	  (0x12000)
#  define CPLD_TX_DIS_45	      (1 << 4)
#  define CPLD_TX_DIS_46	      (1 << 5)
#  define CPLD_TX_DIS_47	      (1 << 6)
#  define CPLD_TX_DIS_48	      (1 << 7)

#define CPLD_REG_SPF_RX_LOS	  (0x15000)
#  define CPLD_RX_LOS_45	      (1 << 4)
#  define CPLD_RX_LOS_46	      (1 << 5)
#  define CPLD_RX_LOS_47	      (1 << 6)
#  define CPLD_RX_LOS_48	      (1 << 7)

#define CPLD_REG_SPF_PRESENT	  (0x18000)
#  define CPLD_SFP_PRESENT_L_45	      (1 << 4)
#  define CPLD_SFP_PRESENT_L_46	      (1 << 5)
#  define CPLD_SFP_PRESENT_L_47	      (1 << 6)
#  define CPLD_SFP_PRESENT_L_48	      (1 << 7)

#define CPLD_REG_USB_CTRL	  (0x19000)
#  define CPLD_USB_ERROR_L            (1 << 4)
#  define CPLD_USB_RESET              (1 << 0)

#define CPLD_REG_PS1_STATUS	  (0x1a000)
#define CPLD_REG_PS2_STATUS	  (0x1b000)
#  define CPLD_PS_TEMP_BAD           (1 << 4)
#  define CPLD_PS_FAN_BAD            (1 << 3)
#  define CPLD_PS_PRESENT            (1 << 2)
#  define CPLD_PS_AC_GOOD            (1 << 1)
#  define CPLD_PS_DC_GOOD            (1 << 0)

#define CPLD_REG_10G_MOD_ENABLE	  (0x1d000)
#  define CPLD_10G_49_50_EN_L        (1 << 1)
#  define CPLD_10G_51_52_EN_L        (1 << 0)

#define CPLD_REG_MOD_49_50_STATUS (0x1e000)
#define CPLD_REG_MOD_51_52_STATUS (0x1f000)
#  define CPLD_10G_MOD_READY_L       (1 << 4)
#  define CPLD_10G_MOD_PRESENT_L     (1 << 3)
#  define CPLD_10G_MOD_ID_MASK       (0x7)
#  define CPLD_10G_MOD_ID_SFP        (0x1)

#define CPLD_REG_EXT_REVISION	  (0x20000)
#define CPLD_REG_SERIAL_CTRL	  (0x21000)

/*
 * End register defines.
 */

// Expected product ID
#define DNI_6448_PRODUCT_ID   (0xa0)

/*
** Define a more programmer friendly format for the port control and
** status registers.
**
** 4-bits of control/status for each port 45 - 52, packed into a
** uint32_t.
**
** Ports 45 - 48 are muxable (copper or fiber) 1G ports.
** Ports 49 - 52 are 10G SFP+ on the hot-swap modules.
**
** [00]:RW - port 45 fiber mode  -- 0 - copper, 1 - fiber(SFP)
** [01]:RW - port 45 tx_enable   -- For SFP: 0 - disable, 1 - enable
** [02]:RO - port 45 sfp_present -- For SFP, Read-Only. 0 - not present, 1 - present
** [03]:RO - port 45 rx_los      -- For SFP, Read-Only. 0 - link up, 1 - link down
** [04]:RW - port 46 fiber mode  -- 0 - copper, 1 - fiber(SFP)
** [05]:RW - port 46 tx_enable   -- For SFP: 0 - disable, 1 - enable
** [06]:RO - port 46 sfp_present -- For SFP, Read-Only. 0 - not present, 1 - present
** [07]:RO - port 46 rx_los      -- For SFP, Read-Only. 0 - link up, 1 - link down
** [08]:RW - port 47 fiber mode  -- 0 - copper, 1 - fiber(SFP)
** [09]:RW - port 47 tx_enable   -- For SFP: 0 - disable, 1 - enable
** [10]:RO - port 47 sfp_present -- For SFP, Read-Only. 0 - not present, 1 - present
** [11]:RO - port 47 rx_los      -- For SFP, Read-Only. 0 - link up, 1 - link down
** [12]:RW - port 48 fiber mode  -- 0 - copper, 1 - fiber(SFP)
** [13]:RW - port 48 tx_enable   -- For SFP: 0 - disable, 1 - enable
** [14]:RO - port 48 sfp_present -- For SFP, Read-Only. 0 - not present, 1 - present
** [15]:RO - port 48 rx_los      -- For SFP, Read-Only. 0 - link up, 1 - link down
** [16]:RO - port 49 fiber mode  -- Read-Only. Always reads 1 - fiber(SFP+)
** [17]:RW - port 49 tx_enable   -- 0 - disable, 1 - enable
** [18]:RO - port 49 sfp_present -- Read-Only. 0 - not present, 1 - present
** [19]:RO - port 49 rx_los      -- Read-Only. 0 - link up, 1 - link down
** [20]:RO - port 50 fiber mode  -- Read-Only. Always reads 1 - fiber(SFP+)
** [21]:RW - port 50 tx_enable   -- 0 - disable, 1 - enable
** [22]:RO - port 50 sfp_present -- Read-Only. 0 - not present, 1 - present
** [23]:RO - port 50 rx_los      -- Read-Only. 0 - link up, 1 - link down
** [24]:RO - port 51 fiber mode  -- Read-Only. Always reads 1 - fiber(SFP+)
** [25]:RW - port 51 tx_enable   -- 0 - disable, 1 - enable
** [26]:RO - port 51 sfp_present -- Read-Only. 0 - not present, 1 - present
** [27]:RO - port 51 rx_los      -- Read-Only. 0 - link up, 1 - link down
** [28]:RO - port 52 fiber mode  -- Read-Only. Always reads 1 - fiber(SFP+)
** [29]:RW - port 52 tx_enable   -- 0 - disable, 1 - enable
** [30]:RO - port 52 sfp_present -- Read-Only. 0 - not present, 1 - present
** [31]:RO - port 52 rx_los      -- Read-Only. 0 - link up, 1 - link down

*/

/* Internal routine */
static inline uint32_t
__dni6448_get_n( uint32_t val, uint16_t port, uint16_t offset)
{
#ifdef __KERNEL__
	BUG_ON( (port < 45) || (port > 52));
#else
	assert( (port >= 45) && (port <= 52));
#endif
	return (val & (1 << (offset + (4 * (port - 45))))) ? 1 : 0;
}

/* External get fiber mode routine */
static inline uint32_t
dni6448_get_fiber_mode( uint32_t val, uint16_t port)
{
	return __dni6448_get_n( val, port, 0);
}

/* External get tx enable */
static inline uint32_t
dni6448_get_tx_enable( uint32_t val, uint16_t port)
{
	return __dni6448_get_n( val, port, 1);
}

/* External get SFP present */
static inline uint32_t
dni6448_get_sfp_present( uint32_t val, uint16_t port)
{
	return __dni6448_get_n( val, port, 2);
}

/* External get rx los */
static inline uint32_t
dni6448_get_rx_los( uint32_t val, uint16_t port)
{
	return __dni6448_get_n( val, port, 3);
}

/*
** Set bit routines
*/

/* Internal routine */
static inline void
__dni6448_set_n( uint32_t* val, uint16_t port, uint16_t bit, uint16_t offset)
{
#ifdef __KERNEL__
	BUG_ON( (port < 45) || (port > 52));
#else
	assert( (port >= 45) && (port <= 52));
#endif
	if ( bit) {
		// set it
		*val |= (1 << (offset + (4 * (port - 45))));
	}
	else {
		// clear it
		*val &= ~(1 << (offset + (4 * (port - 45))));
	}
}

/* External set fiber mode routine */
static inline void
dni6448_set_fiber_mode( uint32_t* val, uint16_t port, uint16_t bit)
{
	__dni6448_set_n( val, port, bit, 0);
}

/* External set tx enable */
static inline void
dni6448_set_tx_enable( uint32_t* val, uint16_t port, uint16_t bit)
{
	__dni6448_set_n( val, port, bit, 1);
}

/* External set SFP present */
static inline void
dni6448_set_sfp_present( uint32_t* val, uint16_t port, uint16_t bit)
{
	__dni6448_set_n( val, port, bit, 2);
}

/* External set rx los */
static inline void
dni6448_set_rx_los( uint32_t* val, uint16_t port, uint16_t bit)
{
	__dni6448_set_n( val, port, bit, 3);
}

#endif /* DNI_6448_H__ */
