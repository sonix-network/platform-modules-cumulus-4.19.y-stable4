/*
 * cel_xp_b_i2c.c - Celestica I2C mux support
 *
 * Copyright (C) 2017, 2018, 2019 Cumulus Networks, Inc. all rights reserved
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include "cel-xp-b-muxpld.h"

#define BUS_DRV_NAME    "cel_xp_b_i2c_bus"
#define BUS_MUX_DRV_VERSION "1.0"

/*
 * The CEL_CPLD_I2C_XXX offsets are relative to the
 * start of the "I2C Freq Div" register within each CPLD:
 */
#define CEL_CPLD_I2C_FREQ_DIV       0x00
#define CEL_CPLD_I2C_CR             0x01
#define CEL_CPLD_I2C_SR             0x02
#define CEL_CPLD_I2C_DATA           0x03
#define CEL_CPLD_I2C_PORT_ID        0x04
#define CEL_CPLD_REG_RANGE          0x2f

#define CCR_MEN                     0x80
#define CCR_MIEN                    0x40
#define CCR_MSTA                    0x20
#define CCR_MTX                     0x10
#define CCR_TXAK                    0x08
#define CCR_RSTA                    0x04

#define CSR_MCF                     0x80
#define CSR_BUSY                    0x40
#define CSR_MBB                     0x20
#define CSR_MAL                     0x10
#define CSR_MIF                     0x02
#define CSR_RXAK                    0x01

struct cel_cpld_i2c {
	struct device     *m_dev;
	void __iomem      *m_base;
	struct i2c_adapter m_adap;
	u32                m_clk_freq;
	u32                m_timeout;
};

static inline void cel_cpld_set_mux_reg(struct cel_cpld_i2c *i2c, int channel)
{
	iowrite8(channel & 0x3F, i2c->m_base + CEL_CPLD_I2C_PORT_ID);
}

/*
 * Wait up to 1 second for the controller to be come non-busy.
 *
 * Returns:
 *   - success:  0
 *   - failure:  negative status code
 */
static int cel_cpld_wait(struct cel_cpld_i2c *i2c, int writing)
{
	unsigned long orig_jiffies = jiffies;
	u32 cmd_err;
	int result = 0;

	while (!(ioread8(i2c->m_base + CEL_CPLD_I2C_SR) & CSR_MIF)) {
		schedule();
		if (time_after(jiffies, orig_jiffies + i2c->m_timeout)) {
			iowrite8(0, i2c->m_base + CEL_CPLD_I2C_CR);
			result = -ETIMEDOUT;
			dev_warn(i2c->m_dev, "wait timeout.");
			break;
		}
	}
	cmd_err = ioread8(i2c->m_base + CEL_CPLD_I2C_SR);
	iowrite8(0, i2c->m_base + CEL_CPLD_I2C_SR);

	if (result < 0)
		return result;

	if (!(cmd_err & CSR_MCF))
		return -EIO;

	if (cmd_err & CSR_MAL)
		return -EAGAIN;

	if (writing && (cmd_err & CSR_RXAK)) {
		/* generate stop */
		iowrite8(CCR_MEN, i2c->m_base + CEL_CPLD_I2C_CR);
		return -ENXIO;
	}
	return 0;
}

/* Sometimes 9th clock pulse isn't generated, and slave doesn't release
 * the bus, because it wants to send ACK.
 * Following sequence of enabling/disabling and sending start/stop generates
 * the 9 pulses, so it's all OK.
 */
static void cls_i2c_fixup(struct cel_cpld_i2c *i2c)
{
	int k;
	u32 delay_val = 1000000 / i2c->m_clk_freq + 1;

	if (delay_val < 2)
		delay_val = 2;

	for (k = 9; k; k--) {
		iowrite8(0, i2c->m_base + CEL_CPLD_I2C_CR);
		iowrite8(CCR_MSTA | CCR_MTX | CCR_MEN,
			 i2c->m_base + CEL_CPLD_I2C_CR);
		ioread8(i2c->m_base + CEL_CPLD_I2C_DATA);
		iowrite8(CCR_MEN, i2c->m_base + CEL_CPLD_I2C_CR);
		udelay(delay_val << 1);
	}
}

static int cel_cpld_i2c_write(struct cel_cpld_i2c *i2c, int addr,
			      u8 *data, int length, int restart)
{
	int i, result;
	u32 flags = restart ? CCR_RSTA : 0;

	/* Start as master */
	iowrite8(CCR_MIEN | CCR_MEN | CCR_MSTA | CCR_MTX | flags,
		 i2c->m_base + CEL_CPLD_I2C_CR);
	/* Write target byte */
	iowrite8(addr << 1, i2c->m_base + CEL_CPLD_I2C_DATA);

	result = cel_cpld_wait(i2c, 1);
	if (result < 0)
		return result;

	for (i = 0; i < length; i++) {
		/* Write data byte */
		iowrite8(data[i], i2c->m_base + CEL_CPLD_I2C_DATA);

		result = cel_cpld_wait(i2c, 1);
		if (result < 0)
			return result;
	}

	return 0;
}

static int cel_cpld_i2c_read(struct cel_cpld_i2c *i2c, int addr,
			     u8 *data, int length, int restart, bool recv_len)
{
	int i, result;
	u32 flags = restart ? CCR_RSTA : 0;

	/* Switch to read - restart */
	iowrite8(CCR_MIEN | CCR_MEN | CCR_MSTA | CCR_MTX | flags,
		 i2c->m_base + CEL_CPLD_I2C_CR);
	/* Write target address byte - this time with the read flag set */
	iowrite8((addr << 1) | 1, i2c->m_base + CEL_CPLD_I2C_DATA);

	result = cel_cpld_wait(i2c, 1);
	if (result < 0)
		return result;

	if (length) {
		if (length == 1 && !recv_len)
			iowrite8(CCR_MIEN | CCR_MEN | CCR_MSTA | CCR_TXAK,
				 i2c->m_base + CEL_CPLD_I2C_CR);
		else
			iowrite8(CCR_MIEN | CCR_MEN | CCR_MSTA,
				 i2c->m_base + CEL_CPLD_I2C_CR);
		/* Dummy read */
		ioread8(i2c->m_base + CEL_CPLD_I2C_DATA);
	}

	for (i = 0; i < length; i++) {
		u8 byte;

		result = cel_cpld_wait(i2c, 0);
		if (result < 0)
			return result;

		/*
		 * For block reads, we have to know the total length (1st byte)
		 * before we can determine if we are done.
		 */
		if (i || !recv_len) {
			/* Generate txack on next to last byte */
			if (i == length - 2)
				iowrite8(CCR_MIEN | CCR_MEN | CCR_MSTA |
					 CCR_TXAK,
					 i2c->m_base + CEL_CPLD_I2C_CR);
			/* Do not generate stop on last byte */
			if (i == length - 1)
				iowrite8(CCR_MIEN | CCR_MEN | CCR_MSTA |
					 CCR_MTX,
					 i2c->m_base + CEL_CPLD_I2C_CR);
		}

		byte = ioread8(i2c->m_base + CEL_CPLD_I2C_DATA);

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
				iowrite8(CCR_MIEN | CCR_MEN | CCR_MSTA |
					 CCR_TXAK,
					 i2c->m_base + CEL_CPLD_I2C_CR);
		}
		data[i] = byte;
	}

	return length;
}

static int cel_cpld_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs,
			 int num)
{
	struct i2c_msg *pmsg;
	int ret = 0;
	int i;
	unsigned long orig_jiffies = jiffies;
	struct cel_cpld_i2c *i2c = i2c_get_adapdata(adap);
	u8 status;
	bool recv_len;

	/* Clear arbitration */
	iowrite8(0, i2c->m_base + CEL_CPLD_I2C_SR);
	/* Start with MEN */
	iowrite8(CCR_MEN, i2c->m_base + CEL_CPLD_I2C_CR);

	/* Allow bus up to 1s to become not busy */
	while (ioread8(i2c->m_base + CEL_CPLD_I2C_SR) & CSR_MBB) {
		if (signal_pending(current)) {
			iowrite8(0, i2c->m_base + CEL_CPLD_I2C_CR);
			return -EINTR;
		}
		if (time_after(jiffies, orig_jiffies + HZ)) {
			status = ioread8(i2c->m_base + CEL_CPLD_I2C_SR);
			if ((status & (CSR_MCF | CSR_MBB | CSR_RXAK)) != 0) {
				iowrite8(status & ~CSR_MAL,
					 i2c->m_base + CEL_CPLD_I2C_SR);
				cls_i2c_fixup(i2c);
			}
			return -EIO;
		}
		schedule();
	}

	for (i = 0; ret >= 0 && i < num; i++) {
		pmsg = &msgs[i];
		if (pmsg->flags & I2C_M_RD) {
			recv_len = pmsg->flags & I2C_M_RECV_LEN;
			ret = cel_cpld_i2c_read(i2c, pmsg->addr, pmsg->buf,
						pmsg->len, i, recv_len);
			if (recv_len && ret > 0)
				pmsg->len = ret;
		} else {
			ret = cel_cpld_i2c_write(i2c, pmsg->addr, pmsg->buf,
						 pmsg->len, i);
		}
	}

	/* initiate stop */
	iowrite8(CCR_MEN, i2c->m_base + CEL_CPLD_I2C_CR);
	orig_jiffies = jiffies;
	/* Wait until STOP is seen, allow up to 1 s */
	while (ioread8(i2c->m_base + CEL_CPLD_I2C_SR) & CSR_MBB) {
		if (time_after(jiffies, orig_jiffies + HZ)) {
			status = ioread8(i2c->m_base + CEL_CPLD_I2C_SR);
			if ((status & (CSR_MCF | CSR_MBB | CSR_RXAK)) != 0) {
				iowrite8(status & ~CSR_MAL,
					 i2c->m_base + CEL_CPLD_I2C_SR);
				cls_i2c_fixup(i2c);
			}
			return -EIO;
		}
		cond_resched();
	}

	return (ret < 0) ? ret : num;
}

static u32 cel_cpld_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm cel_cpld_algo = {
	.master_xfer = cel_cpld_xfer,
	.functionality = cel_cpld_functionality,
};

static struct i2c_adapter cel_cpld_ops = {
	.owner = THIS_MODULE,
	.name = "CEL_CPLD adapter",
	.algo = &cel_cpld_algo,
	.timeout = HZ,
};

static int cel_cpld_i2c_bus_setup(struct cel_cpld_i2c *i2c, u32 clock)
{
	switch (clock) {
	case CPLD_I2C_50KHZ:
		i2c->m_clk_freq = 50000l;
		break;
	case CPLD_I2C_100KHZ:
		i2c->m_clk_freq = 100000l;
		break;
	case CPLD_I2C_200KHZ:
		i2c->m_clk_freq = 200000l;
		break;
	case CPLD_I2C_400KHZ:
		i2c->m_clk_freq = 400000l;
		break;
	default:
		dev_err(i2c->m_dev, "error: unsupported bus speed.\n");
		return -EINVAL;
	}
	iowrite8(clock, i2c->m_base + CEL_CPLD_I2C_FREQ_DIV);
	return 0;
}

static int cel_cpld_i2c_bus_probe(struct platform_device *op)
{
	struct cel_cpld_i2c *i2c;
	struct cpld_bus_data *bus_data;
	u32 clock = CPLD_I2C_400KHZ;
	int result = 0;

	i2c = kzalloc(sizeof(*i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	i2c->m_dev = &op->dev; /* for debug and error output */

	if (!op->dev.platform_data) {
		pr_err(BUS_DRV_NAME "error: no platform data\n");
		goto fail_map;
	}
	bus_data = op->dev.platform_data;
	i2c->m_base = ioport_map(bus_data->io_base, CEL_CPLD_REG_RANGE);

	cel_cpld_ops.nr = bus_data->bus;
	i2c->m_adap = cel_cpld_ops;
	i2c_set_adapdata(&i2c->m_adap, i2c);
	i2c->m_adap.dev.parent = &op->dev;
	i2c->m_adap.nr = cel_cpld_ops.nr;

	result = i2c_add_numbered_adapter(&i2c->m_adap);
	if (result < 0) {
		dev_err(i2c->m_dev, "failed to add adapter %u\n",
			bus_data->bus);
		goto fail_map;
	}
	if (bus_data->clock)
		clock = bus_data->clock;
	result = cel_cpld_i2c_bus_setup(i2c, clock);
	if (result < 0)
		goto fail_map;

	if (bus_data->timeout) {
		cel_cpld_ops.timeout = bus_data->timeout * HZ / 1000000;
		if (cel_cpld_ops.timeout < 5)
			cel_cpld_ops.timeout = 5;
	}
	i2c->m_timeout = 1000; /* 1ms */

	dev_set_drvdata(&op->dev, i2c);
	i2c->m_adap.dev.parent = &op->dev;

	return result;

 fail_map:
	kfree(i2c);
	return result;
};

static int cel_cpld_i2c_bus_remove(struct platform_device *op)
{
	struct cel_cpld_i2c *i2c = dev_get_drvdata(&op->dev);

	i2c_del_adapter(&i2c->m_adap);
	dev_set_drvdata(&op->dev, NULL);

	iounmap(i2c->m_base);
	kfree(i2c);
	return 0;
};

/* Structure for a device driver */
static struct platform_driver cel_cpld_i2c_bus_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = BUS_DRV_NAME,
	},
	.probe		= cel_cpld_i2c_bus_probe,
	.remove		= cel_cpld_i2c_bus_remove,
};

/*
 * MUX Driver starts here
 */

#define MUX_DRV_NAME    "cel-xp-b-cpld-i2c-mux"

#define INVALID_CHANNEL		(0xFF)

struct cel_cpld_vadapter {
	struct i2c_adapter *m_adapter;  /* virtual i2c adapter struct */
	u32                 m_channel;  /* MUX channel */
	struct llist_node   m_llnode;
};

struct cel_cpld_i2c_mux {
	struct device       *m_dev;           /* for debug and error output */
	struct llist_head    m_vadapter_list; /* list of virtual i2c adapters */
	struct cel_cpld_i2c *m_i2c_bus;       /* parent I2C master device */
	u8                   m_last_channel;  /* last channel selected    */
	u32                  m_mux_id;        /* mux ID */
	int                  m_num_ports;     /* number of ports on this mux */
};

static int cel_cpld_i2c_mux_select_chan(struct i2c_mux_core *muxc,
					u32 channel)
{
	struct cel_cpld_i2c_mux *mux =  i2c_mux_priv(muxc);

	if (channel != mux->m_last_channel) {
		cel_cpld_set_mux_reg(mux->m_i2c_bus, channel);
		mux->m_last_channel = channel;
	}

	return 0;
}

/*
 * I2C init/probing/exit functions
 */

static int cel_cpld_i2c_mux_probe(struct platform_device *client)
{
	struct i2c_adapter  *adap = to_i2c_adapter(client->dev.parent);
	struct cel_cpld_i2c *i2c = i2c_get_adapdata(adap);
	struct cel_cpld_i2c_mux *mux;
	struct cpld_mux_data *mux_data;
	struct i2c_mux_core *muxc = NULL;
	int rc = -ENODEV;
	int i;

	if (!client->dev.platform_data) {
		pr_err(BUS_DRV_NAME "mux: no platform data\n");
		return -EINVAL;
	}
	mux_data = client->dev.platform_data;

	mux = kmalloc(sizeof(*mux), GFP_KERNEL);
	if (!mux) {
		rc = -ENOMEM;
		goto err_exit;
	}
	mux->m_dev = &client->dev;
	mux->m_i2c_bus = i2c;
	mux->m_mux_id = adap->nr;
	mux->m_last_channel = INVALID_CHANNEL;
	mux->m_num_ports = mux_data->mux_num_ports;
	muxc = i2c_mux_alloc(adap,
			     &client->dev,
			     mux_data->mux_num_ports,
			     0,
			     0,
			     cel_cpld_i2c_mux_select_chan,
			     NULL);

	if (!muxc) {
	  pr_err(BUS_DRV_NAME "i2c_mux_alloc() failed .\n");
	  rc = -ENOMEM;
	  goto err_exit;
	}

	muxc->priv = mux;
	platform_set_drvdata(client, muxc);

	for (i = 0; i < mux_data->mux_num_ports; i++) {
		int bus;
		int channel;

		channel = mux_data->mux_base_port_num + i;
		bus =  mux_data->mux_ports_base_bus + channel;
		rc = i2c_mux_add_adapter(muxc, bus, channel, 0);
		if (rc) {
			pr_err(BUS_DRV_NAME
			       "failed to register multiplexed adapter %u\n",
			       bus);
			goto dealloc_exit;
		}
	}
	dev_set_drvdata(&client->dev, mux);

	return 0;

dealloc_exit:
	i2c_mux_del_adapters(muxc);
	i2c_put_adapter(muxc->parent);
err_exit:
	return rc;
}

static int cel_cpld_i2c_mux_remove(struct platform_device *client)
{
	struct i2c_mux_core *muxc = platform_get_drvdata(client);

	if (muxc) {
		kfree(muxc->priv);
		i2c_mux_del_adapters(muxc);
		i2c_put_adapter(muxc->parent);
	}

	return 0;
}

static const struct platform_device_id cel_cpld_i2c_mux_ids[] = {
	{ "cel_xp_b_cpld_mux", 0 },
	{ /* end of list */ },
};

static struct platform_driver cel_cpld_mux_driver = {
	.driver = {
		.name  = MUX_DRV_NAME,
		.owner = THIS_MODULE,
	},
	.probe    = cel_cpld_i2c_mux_probe,
	.remove   = cel_cpld_i2c_mux_remove,
	.id_table = cel_cpld_i2c_mux_ids,
};

/*
 * Module init/exit methods begin here.  The init and exit methods
 * register/de-register two devices:
 *
 * - The i2c bus adapter device
 * - The i2c mux device
 *
 */
static int __init cel_cpld_i2c_init(void)
{
	int rc;

	/* First register the i2c adapter */
	rc = platform_driver_register(&cel_cpld_i2c_bus_driver);
	if (rc) {
		pr_err("%s(): platform_driver_register() failed: %d\n",
		       __func__, rc);
		return rc;
	}

	/* Next register the i2c mux */
	rc = platform_driver_register(&cel_cpld_mux_driver);
	if (rc) {
		pr_err("%s(): i2c_add_driver() failed: %d\n",
		       __func__, rc);
		platform_driver_unregister(&cel_cpld_i2c_bus_driver);
	}

	return rc;
}

static void __exit cel_cpld_i2c_exit(void)
{
	/* First remove the i2c mux driver */
	platform_driver_unregister(&cel_cpld_mux_driver);

	/* Next remove the i2c adapter */
	platform_driver_unregister(&cel_cpld_i2c_bus_driver);
}

module_init(cel_cpld_i2c_init);
module_exit(cel_cpld_i2c_exit);

MODULE_AUTHOR("Alan Liebthal <alanl@cumulusnetworks.com>");
MODULE_DESCRIPTION("I2C Adapter/Mux Driver for Celestica XP-B CPLD");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(BUS_MUX_DRV_VERSION);
