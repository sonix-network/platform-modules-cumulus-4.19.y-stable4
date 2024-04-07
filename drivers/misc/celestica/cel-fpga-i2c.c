/*
 * cel_fpga_i2c.c - Celestica FPGA I2C bus support
 *
 * Copyright (C) 2019 Cumulus Networks, Inc.  All rights reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched/signal.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include "cel-fpga-i2c.h"

#define DRIVER_VERSION "1.0"

/* The CEL_FPGA_I2C_X offsets are relative to the start of each FPGA core */

#define CEL_FPGA_I2C_FREQ_DIV       0x00
#define CEL_FPGA_I2C_CR             0x04
#define CEL_FPGA_I2C_SR             0x08
#define CEL_FPGA_I2C_DATA           0x0c
#define CEL_FPGA_I2C_PORT_ID        0x10
#define CEL_FPGA_REG_RANGE          0x1f

#define CCR_MEN                     0x80
#define CCR_MIEN                    0x40
#define CCR_MSTA                    0x20
#define CCR_MTX                     0x10
#define CCR_TXAK                    0x08
#define CCR_RSTA                    0x04

#define CSR_MCF                     0x80
#define CSR_BUSY                    0x40 /* always 0 */
#define CSR_MBB                     0x20
#define CSR_MAL                     0x10 /* always 0 */
#define CSR_MIF                     0x02
#define CSR_RXAK                    0x01

struct cel_fpga_i2c {
	struct device     *dev;
	void __iomem      *base;
	struct mutex       lock;
	struct i2c_adapter adap;
	u32                clk_freq;
	u32                timeout;
};

static inline void cel_fpga_set_mux_reg(struct cel_fpga_i2c *i2c, int channel)
{
	iowrite32(channel & 0x3F, i2c->base + CEL_FPGA_I2C_PORT_ID);
}

/*
 * Wait up to 1 second for the controller to be come non-busy.
 *
 * Returns:
 *   - success:  0
 *   - failure:  negative status code
 */
static int cel_fpga_wait(struct cel_fpga_i2c *i2c, int writing)
{
	unsigned long orig_jiffies = jiffies;
	u32 cmd_err;
	int result = 0;

	while (!(ioread32(i2c->base + CEL_FPGA_I2C_SR) & CSR_MIF)) {
		schedule();
		if (time_after(jiffies, orig_jiffies + i2c->timeout)) {
			iowrite32(0, i2c->base + CEL_FPGA_I2C_CR);
			result = -ETIMEDOUT;
			dev_warn(i2c->dev, "wait timeout.");
			break;
		}
	}
	cmd_err = ioread32(i2c->base + CEL_FPGA_I2C_SR);
	iowrite32(0, i2c->base + CEL_FPGA_I2C_SR);

	if (result < 0)
		return result;

	if (!(cmd_err & CSR_MCF))
		return -EIO;

	if (cmd_err & CSR_MAL)
		return -EAGAIN;

	if (writing && (cmd_err & CSR_RXAK)) {
		/* generate stop */
		iowrite32(CCR_MEN, i2c->base + CEL_FPGA_I2C_CR);
		return -ENXIO;
	}
	return 0;
}

/* Sometimes 9th clock pulse isn't generated, and slave doesn't release
 * the bus, because it wants to send ACK.
 * Following sequence of enabling/disabling and sending start/stop generates
 * the 9 pulses, so it's all OK.
 */
static void cel_fpga_i2c_fixup(struct cel_fpga_i2c *i2c)
{
	int k;
	u32 delay_val = 1000000 / i2c->clk_freq + 1;

	if (delay_val < 2)
		delay_val = 2;

	for (k = 9; k; k--) {
		iowrite32(0, i2c->base + CEL_FPGA_I2C_CR);
		iowrite32(CCR_MSTA | CCR_MTX | CCR_MEN,
			 i2c->base + CEL_FPGA_I2C_CR);
		ioread32(i2c->base + CEL_FPGA_I2C_DATA);
		iowrite32(CCR_MEN, i2c->base + CEL_FPGA_I2C_CR);
		udelay(delay_val << 1);
	}
}

static int cel_fpga_i2c_write(struct cel_fpga_i2c *i2c, int addr,
			      u8 *data, int length, int restart)
{
	int i, result;
	u32 flags = restart ? CCR_RSTA : 0;

	/* Start as master */
	iowrite32(CCR_MIEN | CCR_MEN | CCR_MSTA | CCR_MTX | flags,
		 i2c->base + CEL_FPGA_I2C_CR);
	/* Write target byte */
	iowrite32(addr << 1, i2c->base + CEL_FPGA_I2C_DATA);

	result = cel_fpga_wait(i2c, 1);
	if (result < 0)
		return result;

	for (i = 0; i < length; i++) {
		/* Write data byte */
		iowrite32(data[i], i2c->base + CEL_FPGA_I2C_DATA);

		result = cel_fpga_wait(i2c, 1);
		if (result < 0)
			return result;
	}

	return 0;
}

static int cel_fpga_i2c_read(struct cel_fpga_i2c *i2c, int addr,
			     u8 *data, int length, int restart, bool recv_len)
{
	int i, result;
	u32 flags = restart ? CCR_RSTA : 0;

	/* Switch to read - restart */
	iowrite32(CCR_MIEN | CCR_MEN | CCR_MSTA | CCR_MTX | flags,
		 i2c->base + CEL_FPGA_I2C_CR);
	/* Write target address byte - this time with the read flag set */
	iowrite32(addr << 1 | 1, i2c->base + CEL_FPGA_I2C_DATA);

	result = cel_fpga_wait(i2c, 1);
	if (result < 0)
		return result;

	if (length) {
		if (length == 1 && !recv_len)
			iowrite32(CCR_MIEN | CCR_MEN | CCR_MSTA | CCR_TXAK,
				 i2c->base + CEL_FPGA_I2C_CR);
		else
			iowrite32(CCR_MIEN | CCR_MEN | CCR_MSTA,
				 i2c->base + CEL_FPGA_I2C_CR);
		/* Dummy read */
		ioread32(i2c->base + CEL_FPGA_I2C_DATA);
	}

	for (i = 0; i < length; i++) {
		u8 byte;

		result = cel_fpga_wait(i2c, 0);
		if (result < 0)
			return result;

		/*
		 * For block reads, we have to know the total length (1st byte)
		 * before we can determine if we are done.
		 */
		if (i || !recv_len) {
			/* Generate txack on next to last byte */
			if (i == length - 2)
				iowrite32(CCR_MIEN | CCR_MEN | CCR_MSTA |
					 CCR_TXAK,
					 i2c->base + CEL_FPGA_I2C_CR);
			/* Do not generate stop on last byte */
			if (i == length - 1)
				iowrite32(CCR_MIEN | CCR_MEN | CCR_MSTA |
					 CCR_MTX,
					 i2c->base + CEL_FPGA_I2C_CR);
		}

		byte = ioread32(i2c->base + CEL_FPGA_I2C_DATA);

		/*
		 * Adjust length if first received byte is length.
		 * The length is 1 length byte plus actually data length
		 */
		if (i == 0 && recv_len) {
			if (byte == 0 || byte > I2C_SMBUS_BLOCK_MAX)
				return -EPROTO;
			length += byte;
			/*
			 * For block reads, generate txack here if data length
			 * is 1 byte (total length is 2 bytes).
			 */
			if (length == 2)
				iowrite32(CCR_MIEN | CCR_MEN | CCR_MSTA |
					 CCR_TXAK,
					 i2c->base + CEL_FPGA_I2C_CR);
		}
		data[i] = byte;
	}

	return length;
}

static int cel_fpga_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs,
			 int num)
{
	struct i2c_msg *pmsg;
        int ret = 0;
        int i;
	unsigned long orig_jiffies = jiffies;
        struct cel_fpga_i2c *i2c = i2c_get_adapdata(adap);
        u8 status;
        bool recv_len;

        /* Clear arbitration */
        iowrite32(0, i2c->base + CEL_FPGA_I2C_SR);
	/* Start with MEN */
        iowrite32(CCR_MEN, i2c->base + CEL_FPGA_I2C_CR);

        /* Allow bus up to 1s to become not busy */
        while (ioread32(i2c->base + CEL_FPGA_I2C_SR) & CSR_MBB) {
                if (signal_pending(current)) {
                        iowrite32(0, i2c->base + CEL_FPGA_I2C_CR);
			return -EINTR;
                }
                if (time_after(jiffies, orig_jiffies + HZ)) {
                        status = ioread32(i2c->base + CEL_FPGA_I2C_SR);
			if ((status & (CSR_MCF | CSR_MBB | CSR_RXAK)) != 0) {
                                iowrite32(status & ~CSR_MAL,
                                         i2c->base + CEL_FPGA_I2C_SR);
				cel_fpga_i2c_fixup(i2c);
                        }
                        return -EIO;
                }
                schedule();
	}

        for (i = 0; ret >= 0 && i < num; i++) {
                pmsg = &msgs[i];
                if (pmsg->flags & I2C_M_RD) {
                        recv_len = pmsg->flags & I2C_M_RECV_LEN;
                        ret = cel_fpga_i2c_read(i2c, pmsg->addr, pmsg->buf,
                                                pmsg->len, i, recv_len);
                        if (recv_len && ret > 0)
                                pmsg->len = ret;
                } else {
                        ret = cel_fpga_i2c_write(i2c, pmsg->addr, pmsg->buf,
                                                 pmsg->len, i);
                }
        }

        /* initiate stop */
        iowrite32(CCR_MEN, i2c->base + CEL_FPGA_I2C_CR);
        orig_jiffies = jiffies;
        /* Wait until STOP is seen, allow up to 1 s */
        while (ioread32(i2c->base + CEL_FPGA_I2C_SR) & CSR_MBB) {
                if (time_after(jiffies, orig_jiffies + HZ)) {
                        status = ioread32(i2c->base + CEL_FPGA_I2C_SR);
                        if ((status & (CSR_MCF | CSR_MBB | CSR_RXAK)) != 0) {
                                iowrite32(status & ~CSR_MAL,
                                         i2c->base + CEL_FPGA_I2C_SR);
                                cel_fpga_i2c_fixup(i2c);
                        }
                        return -EIO;
                }
                cond_resched();
        }

        return (ret < 0) ? ret : num;
}

static u32 cel_fpga_i2c_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm cel_fpga_i2c_algorithm = {
	.master_xfer = cel_fpga_i2c_xfer,
	.functionality = cel_fpga_i2c_functionality,
};

static struct i2c_adapter cel_fpga_i2c_adapter = {
	.owner = THIS_MODULE,
	.name = "fpga-i2c-adapter",
	.class = I2C_CLASS_DEPRECATED,
	.algo = &cel_fpga_i2c_algorithm,
	.timeout = HZ,
};


/*
static int cel_fpga_i2c_bus_setup(struct cel_fpga_i2c *i2c, u32 clock)
{
	switch (clock) {
	case FPGA_I2C_50KHZ:
		i2c->clk_freq = 50000l;
		break;
	case FPGA_I2C_100KHZ:
		i2c->clk_freq = 100000l;
		break;
	case FPGA_I2C_200KHZ:
		i2c->clk_freq = 200000l;
		break;
	case FPGA_I2C_400KHZ:
		i2c->clk_freq = 400000l;
		break;
	default:
		dev_err(i2c->dev, "error: unsupported bus speed.\n");
		return -EINVAL;
	}

	pr_info("IOWRITE32 #24; clock = %d\n", clock);
	iowrite32(clock, i2c->base + CEL_FPGA_I2C_FREQ_DIV);

	return 0;
}
*/
static int cel_fpga_i2c_probe(struct platform_device *pdev)
{
	struct cel_fpga_i2c *i2c;
	struct fpga_i2c_platform_data *platdata;
	struct resource *res;
	int ret;

	i2c = devm_kzalloc(&pdev->dev, sizeof(*i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENOMEM;

	i2c->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(i2c->base))
		return PTR_ERR(i2c->base);

	mutex_init(&i2c->lock);

	platdata = dev_get_platdata(&pdev->dev);
	if (platdata) {
		i2c->clk_freq = platdata->clock_khz;
		i2c->timeout = HZ;
	}

	/* hook up driver to tree */
	platform_set_drvdata(pdev, i2c);
	i2c->adap = cel_fpga_i2c_adapter;
	i2c_set_adapdata(&i2c->adap, i2c);
	i2c->adap.dev.parent = &pdev->dev;
	i2c->adap.dev.of_node = pdev->dev.of_node;

	/* add i2c adapter to i2c tree */
	if (platdata)
		i2c->adap.nr = (platdata->devices)->bus;
	else
		i2c->adap.nr = -1;

	ret = i2c_add_numbered_adapter(&i2c->adap);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to add numbered adapter %d\n",
			i2c->adap.nr);
		return ret;
	}

	return 0;
};

static int cel_fpga_i2c_remove(struct platform_device *pdev)
{
	struct cel_fpga_i2c *i2c;

	i2c = platform_get_drvdata(pdev);

	i2c_del_adapter(&i2c->adap);

	dev_set_drvdata(&pdev->dev, NULL);

	iounmap(i2c->base);
	kfree(i2c);
	return 0;
};

/* Structure for a device driver */

static struct platform_driver cel_fpga_i2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name  = CEL_FPGA_I2C_DRIVER_NAME,
	},
	.probe		= cel_fpga_i2c_probe,
	.remove		= cel_fpga_i2c_remove,
};

/* Module init/exit */

static int __init cel_fpga_i2c_init(void)
{
	int ret;

	/* register the i2c adapter */

	ret = platform_driver_register(&cel_fpga_i2c_driver);
	if (ret) {
		pr_err("%s(): platform_driver_register() failed: %d\n",
		       __func__, ret);
		return ret;
	}

	return ret;
}

static void __exit cel_fpga_i2c_exit(void)
{
	/* remove the i2c adapter */

	platform_driver_unregister(&cel_fpga_i2c_driver);
}

module_init(cel_fpga_i2c_init);
module_exit(cel_fpga_i2c_exit);

/*module_platform_driver(cel_fpga_i2c_driver);*/

MODULE_AUTHOR("David Yen  <dhyen@cumulusnetworks.com>");
MODULE_DESCRIPTION("I2C Adapter/Mux Driver for Celestica FPGA");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DRIVER_VERSION);
