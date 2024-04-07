/*
 * Celestica Seastone2 and Questone2 FPGA Port Management Definitions
 *
 * Copyright (c) 2019 Cumulus Networks, Inc.  All rights reserved.
 *
 */

#ifndef CEL_SEA2QUE2_H__
#define CEL_SEA2QUE2_H__

/*******************************************************************************
 *
 *                     Celestica FPGA Register Definitions
 *
 *  These register definitions are taken from the Seastone2 & Questone2 FPGA
 *  Design Specification, Revision: 0.6, dated May 23, 2018.
 *
 ******************************************************************************/

/*------------------------------------------------------------------------------
 *
 *                         PORT XCVR Registers
 *                           0x4000 - 0x4FFF
 *
 *----------------------------------------------------------------------------*/

#define CEL_SEA2QUE2_PORT_CTRL_REG                        0x0000
  #define CEL_SEA2QUE2_PORT_CTRL_LPMOD_BIT                6
  #define CEL_SEA2QUE2_PORT_CTRL_RST_BIT                  4
  #define CEL_SEA2QUE2_PORT_CTRL_TX_DIS_BIT               0

#define CEL_SEA2QUE2_PORT_STAT_REG                        0x0004
  #define CEL_SEA2QUE2_PORT_STAT_IRQ_BIT                  5
  #define CEL_SEA2QUE2_PORT_STAT_PRESENT_BIT              4
  #define CEL_SEA2QUE2_PORT_STAT_TXFAULT_BIT              2
  #define CEL_SEA2QUE2_PORT_STAT_RXLOS_BIT                1
  #define CEL_SEA2QUE2_PORT_STAT_MODABS_BIT               0

#endif /* CEL_SEA2QUE2_H__ */

