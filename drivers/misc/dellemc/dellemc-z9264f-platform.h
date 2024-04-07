/*
 * Dell EMC Z9264F platform Definitions
 *
 * Copyright (C) 2018 Cumulus Networks, Inc.  All Rights Reserved.
 * Nikhil Dhar (ndhar@cumulusnetworks.com)
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
 */
#ifndef __DELL_Z9264_PLATFORM_H
#define __DELL_Z9264_PLATFORM_H

#define FPGA_I2C_BUS_MAX 8

/* FPGA Hardware Descriptor */
struct fpga_desc {
	u8 tgtaddr_rw;	/* target address & r/w bit */
	u8 wr_len_cmd;	/* write length in bytes or a command */
	u8 rd_len;	/* read length */
	u8 control;	/* control bits */
	u8 status;	/* status bits */
	u8 retry;	/* collision retry and retry count */
	u8 rxbytes;	/* received bytes */
	u8 txbytes;	/* transmitted bytes */
	u32 dptr_low;	/* lower 32 bit of the data pointer */
	u32 dptr_high;	/* upper 32 bit of the data pointer */
} __packed;

struct fpga_priv {
	u8 __iomem *pbar;                       /* PCIe base address register */
	struct pci_dev *pci_dev;
	struct fpga_desc *hw;			/* descriptor virt base addr */
	dma_addr_t io_rng_dma;			/* descriptor HW base addr */
};
#endif
