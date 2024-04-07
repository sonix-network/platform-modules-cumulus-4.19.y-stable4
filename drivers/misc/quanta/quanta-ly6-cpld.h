/*
 * Quanta LY6 QSFP+ I/O CPLD Platform Definitions
 *
 * Vidya Ravipati <vidya@cumulusnetworks.com>
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

#ifndef QUANTA_LY6_QSFP_IO_CPLD_H__
#define QUANTA_LY6_QSFP_IO_CPLD_H__

#define QUANATA_LY6_CPLD_STRING_NAME_SIZE 30

/*
 * Begin register defines.
 */


/* The Quanta LY6 has two CPLDs:  PORT0 and PORT1.
 *
 * PORT0  -- QSFP+ I/Os for ports 1-16
 * PORT1  -- QSFP+ I/Os for ports 17-32
 *
 * The CPLD driver has been coded such that we have the appearance of
 * a single CPLD. We do this by embedding a two-bit index for which
 * CPLD must be accessed into each register definition. When the
 * register is accessed, the register definition is decoded into a
 * CPLD index and register offset.
 */

/*
 * Port0 CPLD register definitions -- ports 1 to 16 
 */
#define CPLD_QSFP1_4_INFO_REG       (0x01)
#define CPLD_QSFP5_8_INFO_REG       (0x02)
#define CPLD_QSFP9_12_INFO_REG      (0x03)
#define CPLD_QSFP13_16_INFO_REG     (0x04)

/*
 * End register defines.
 */

#endif /* QUANTA_LY6_QSFP_IO_CPLD_H__ */
