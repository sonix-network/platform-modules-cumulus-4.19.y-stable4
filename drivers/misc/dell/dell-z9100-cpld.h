/*
 * DELL Z9100 CPLD Platform Definitions
 *
 * Puneet Shenoy <puneet@cumulusnetworks.com>
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

#ifndef DELL_Z9100_CPLD_H__
#define DELL_Z9100_CPLD_H__

/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 * register info from "Z9100 CPLD Design Specification.pdf"
 * The DELL Z9100 has 4 CPLDs : CPLD 1, CPLD 2, CPLD 3 and CPLD 4.
 *
 * CPLD1 -- System Leds
 * CPLD2 -- ZQSFP ports 1-12
 * CPLD3 -- ZQSFP ports 13-22
 * CPLD2 -- ZQSFP ports 23-32 and SFP+ ports 33-34
 *
 * The CPLD driver has been coded such that we have the appearance of
 * a single CPLD. We do this by embedding a two-bit index for which
 * CPLD must be accessed into each register definition. When the
 * register is accessed, the register definition is decoded into a
 * CPLD index and register offset.
 */


#define CPLD_IDX_SHIFT                    (8)
#define CPLD_IDX_MASK                     (7 << CPLD_IDX_SHIFT)
#define CPLD_CPLD2_IDX                    (0)
#define CPLD_CPLD3_IDX                    (1)
#define CPLD_CPLD4_IDX                    (2)
#define NUM_CPLD_I2C_CLIENTS              (3)

/* These registers are common for all 4 CPLDS */
#define CPLD_VERSION                           (0x0)
#  define CPLD_VERSION_H_MASK                  (0xF0)
#  define CPLD_VERSION_H_SHIFT                 (4)
#  define CPLD_VERSION_L_MASK                  (0x0F)
#  define CPLD_VERSION_L_SHIFT                 (0)


#define CPLD_BOARD_TYPE                        (0x1)
#  define CPLD_BOARD_TYPE_Z9100                (0x1)
#  define CPLD_BOARD_TYPE_Z6100                (0x2)

#define CPLD_SW_SCRATCH                        (0x2)

#define CPLD_ID                                (0x3)
#define CPLD_ID_CPLD1                          (1)
#define CPLD_ID_CPLD2                          (2)
#define CPLD_ID_CPLD3                          (3)
#define CPLD_ID_CPLD4                          (4)

/*
 * Follwing registers are common to CPLD2, CPLD3 and CPLD4.
 * Contents of these registers are identical on all 3 CPLDS.
 */

#define CPLD_PORT_LED_OPMOD                    (0x9)
#  define CPLD_PORT_LED_OPMOD_MASK             (0x1)
#  define CPLD_PORT_LED_OPMOD_NORMAL           (0x0)
#  define CPLD_PORT_LED_OPMOD_TEST             (0x1)

#define CPLD_PORT_LED_TEST                     (0xA)
#  define CPLD_PORT_LED_AMBER_MASK             (0x0C)
#  define CPLD_PORT_LED_AMBER_OFF              (0x00)
#  define CPLD_PORT_LED_AMBER_FLASH_30MS       (0x04)
#  define CPLD_PORT_LED_AMBER_ON               (0x08)
#  define CPLD_PORT_LED_AMBER_FLASH_1S         (0x0C)
#  define CPLD_PORT_LED_GREEN_MASK             (0x03)
#  define CPLD_PORT_LED_GREEN_OFF              (0x00)
#  define CPLD_PORT_LED_GREEN_FLASH_30MS       (0x01)
#  define CPLD_PORT_LED_GREEN_ON               (0x02)
#  define CPLD_PORT_LED_GREEN_FLASH_1S         (0x03)

#define CPLD_ZQSFP_TRIG_MOD                    (0x20)
#   define CPLD_ZQSFP_TRIG_PRESENT_MASK        (0x0c)
#   define CPLD_ZQSFP_TRIG_PRESENT_FALL        (0x0)
#   define CPLD_ZQSFP_TRIG_PRESENT_RISE        (0x1)
#   define CPLD_ZQSFP_TRIG_PRESENT_FALL_RISE   (0x2)
#   define CPLD_ZQSFP_TRIG_PRESENT_LEVEL       (0x3)
#   define CPLD_ZQSFP_TRIG_INT_MASK            (0x03)
#   define CPLD_ZQSFP_TRIG_INT_FALL            (0x0)
#   define CPLD_ZQSFP_TRIG_INT_RISE            (0x1)
#   define CPLD_ZQSFP_TRIG_INT_FALL_RISE       (0x2)
#   define CPLD_ZQSFP_TRIG_INT_LEVEL           (0x3)

#define CPLD_ZQSFP_COMBINE                     (0x21)
#   define CPLD_ZQSFP_ALL_PRESENT              (1 << 1)  
#   define CPLD_ZQSFP_ALL_INT                  (1 << 0)

/*
 * Follwing registers are common to CPLD2, CPLD3 and CPLD4. 
 * However, the contents of the registers are not identical
 * in the CPLDs.
 */

#define CPLD_ZQSFP_RESET_CTRL0                 (0x10)
#define CPLD_ZQSFP_RESET_CTRL1                 (0x11)
#define CPLD2_ZQSFP_1_MASK                     (0xff)

#define CPLD_ZQSFP_LPMOD_CTRL0                 (0x12)
#define CPLD_ZQSFP_LPMOD_CTRL1                 (0x13)

#define CPLD_ZQSFP_INT_STA0                    (0x14)
#define CPLD_ZQSFP_INT_STA1                    (0x15)

#define CPLD_ZQSFP_PRESENT_STA0                (0x16)
#define CPLD_ZQSFP_PRESENT_STA1                (0x17)


#define CPLD_ZQSFP_INT_INT0                    (0x22)
#define CPLD_ZQSFP_INT_INT1                    (0x23)

#define CPLD_ZQSFP_PRESENT_INT0                (0x24)
#define CPLD_ZQSFP_PRESENT_INT1                (0x25)

#define CPLD_ZQSFP_INT_MASK0                   (0x26)
#define CPLD_ZQSFP_INT_MASK1                   (0x27)

#define CPLD_ZQSFP_PRESENT_MASK0               (0x28)
#define CPLD_ZQSFP_PRESENT_MASK1               (0x29)

/* 
 * The following definitions are used to make 32-bit values
 * of the above registers
 */
#define CPLD2_ZQSFP_1_OFF                      (0)
#define CPLD2_ZQSFP_1_MASK                     (0xff)
#define CPLD2_ZQSFP_2_OFF                      (8)
#define CPLD2_ZQSFP_2_MASK                     (0xf)

#define CPLD3_ZQSFP_1_MASK                     (0xff)
#define CPLD3_ZQSFP_1_OFF                      (12)
#define CPLD3_ZQSFP_2_MASK                     (0x3)
#define CPLD3_ZQSFP_2_OFF                      (20)

#define CPLD4_ZQSFP_1_MASK                     (0xff)
#define CPLD4_ZQSFP_1_OFF                      (22)
#define CPLD4_ZQSFP_2_MASK                     (0x3)
#define CPLD4_ZQSFP_2_OFF                      (30)


/* Present only in CPLD 4 */
#define CPLD_SFPP_TXDISABLE_CTRL               (0x30)

#define CPLD_SFPP_RS_CTRL                      (0x31)

#define CPLD_SFPP_RXLOS_STA                    (0x32)

#define CPLD_SFPP_TXFAULT_STA                  (0x33)

#define CPLD_SFPP_PRESENT_STA                  (0x34)

#define CPLD_SFPP_TRIG_MOD                     (0x40)

#define CPLD_SFPP_COMBINE                      (0x41)

#define CPLD_SFPP_RXLOS_INT                    (0x42)

#define CPLD_SFPP_PRESENT_INT                  (0x43)

#define CPLD_SFPP_RXLOS_MASK                   (0x44)

#define CPLD_SFPP_PRESENT_MASK                 (0x45)

#define CPLD_SFPP_MASK                         (0x3)
#endif  // DELL_Z9100_CPLD_H__
