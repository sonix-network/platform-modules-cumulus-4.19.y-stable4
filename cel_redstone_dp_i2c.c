/*
 * cel_redstone_dp_i2c.c - Celestica Redstone-DP I2C support
 *
 * Copyright (C) 2018 Cumulus Networks, Inc.  All Rights Reserved
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
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include "cel_redstone_dp.h"

#define BUS_DRV_NAME	"cel_redstone_dp_i2c_bus"
#define BUS_MUX_DRV_VERSION "1.0"

/*
 * The CPLD_I2C_XXX offsets are relative to the I2C_PORT_ID register within
 * each CPLD:
 */

#define CPLD_I2C_PORT_ID	   0x00
#define CPLD_I2C_OPCODE		   0x01
#define CPLD_I2C_DEV_ADDR	   0x02
#define CPLD_I2C_CMD_BYTE0	   0x03
#define CPLD_I2C_CMD_BYTE1	   0x04
#define CPLD_I2C_CMD_BYTE2	   0x05
#define CPLD_I2C_STATUS		   0x06
#define CPLD_I2C_WRITE_DATA	   0x10
#define WRITE_DATA(x) (CPLD_I2C_WRITE_DATA + (x))
#define CPLD_I2C_READ_DATA	   0x20
#define READ_DATA(x) (CPLD_I2C_READ_DATA + (x))
#define CPLD_REG_RANGE		   0x2F

#define MASTER_ERROR		   0x80
#define I2C_BUSY		   0x40
#define MASTER_RESET_L		   0x01

#define OPCODE_DATA_LENGTH_SHIFT   4
#define OPCODE_CMD_LENGTH	   1

#define DEV_ADDR_READ_OP	   0x1
#define DEV_ADDR_WRITE_OP	   0x0

struct cpld_i2c {
	struct device	  *m_dev;
	void __iomem	  *m_base;
	struct i2c_adapter m_adap;
	u8		   m_clk_freq;
};

static inline void cpld_set_mux_reg(struct cpld_i2c *i2c, int channel)
{
	iowrite8(i2c->m_clk_freq | (channel & 0x3F),
		 i2c->m_base + CPLD_I2C_PORT_ID);
}

/*
 * Wait up to 1 second for the controller to be come non-busy.
 *
 * Returns:
 *   - success:	 0
 *   - failure:	 negative status code
 */
static int cpld_wait(struct cpld_i2c *i2c)
{
	unsigned long orig_jiffies = jiffies;
	int ret = 0;
	u8 csr;

	/* Allow bus up to 1 second to become not busy */
	while ((csr = ioread8(i2c->m_base + CPLD_I2C_STATUS)) & I2C_BUSY) {
		if (signal_pending(current))
			return -EINTR;
		if (time_after(jiffies, orig_jiffies + HZ)) {
			dev_warn(i2c->m_dev, "Bus busy timeout\n");
			ret = -ETIMEDOUT;
			break;
		}
		schedule();
	}

	if (csr & MASTER_ERROR) {
		/* Typically this means the device is not present. */
		/* Clear master error with the master reset. */
		iowrite8(~MASTER_RESET_L, i2c->m_base + CPLD_I2C_STATUS);
		mdelay(3);
		iowrite8(MASTER_RESET_L, i2c->m_base + CPLD_I2C_STATUS);
		ret = ret ? ret : -EIO;
	}

	return ret;
}

static int cpld_i2c_write(struct cpld_i2c *i2c, int target, u8 offset,
			  const u8 *data, int length)
{
	u8 tmp, xfer_len, i;
	int ret, total_xfer = 0;

	/* The CPLD I2C master writes in units of 8 bytes */
	while (length > 0) {
		/* Configure the byte offset within the device */
		iowrite8(offset + total_xfer,
			 i2c->m_base + CPLD_I2C_CMD_BYTE0);

		/* Configure the transfer length (max of 8 bytes) */
		xfer_len = (length > 8) ? 8 : length;
		tmp = (xfer_len << OPCODE_DATA_LENGTH_SHIFT);
		tmp |= OPCODE_CMD_LENGTH;
		iowrite8(tmp, i2c->m_base + CPLD_I2C_OPCODE);

		/* Load the transmit data into the write buffer */
		for (i = 0; i < xfer_len; i++)
			iowrite8(data[total_xfer + i],
				 i2c->m_base + WRITE_DATA(i));

		/* Initiate the write transaction */
		tmp = (target << 1) | DEV_ADDR_WRITE_OP;
		iowrite8(tmp, i2c->m_base + CPLD_I2C_DEV_ADDR);

		ret = cpld_wait(i2c);
		if (ret)
			return ret;

		total_xfer += xfer_len;
		length -= xfer_len;
	}

	return 0;
}

static int cpld_i2c_read(struct cpld_i2c *i2c, int target, u8 offset,
			 u8 *data, int length)
{
	u8 tmp, xfer_len, i;
	int ret, total_xfer = 0;

	/* The CPLD master read in units of 8 bytes */
	while (length > 0) {
		/* Configure the byte offset within the device */
		iowrite8(offset + total_xfer,
			 i2c->m_base + CPLD_I2C_CMD_BYTE0);

		/* Configure the transfer length (max of 8 bytes) */
		xfer_len = (length > 8) ? 8 : length;
		tmp = (xfer_len << OPCODE_DATA_LENGTH_SHIFT);
		tmp |= OPCODE_CMD_LENGTH;
		iowrite8(tmp, i2c->m_base + CPLD_I2C_OPCODE);

		/* Initiate the read transaction */
		tmp = (target << 1) | DEV_ADDR_READ_OP;
		iowrite8(tmp, i2c->m_base + CPLD_I2C_DEV_ADDR);

		/* Wait for transfer completion */
		ret = cpld_wait(i2c);
		if (ret)
			return ret;

		/* Gather of the read data */
		for (i = 0; i < xfer_len; i++)
			data[total_xfer + i] =
				ioread8(i2c->m_base + READ_DATA(i));

		total_xfer += xfer_len;
		length -= xfer_len;
	}

	return 0;
}

static int cpld_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	struct i2c_msg *pmsg;
	int ret = 0;
	struct cpld_i2c *i2c = i2c_get_adapdata(adap);
	u8 offset;

	/* Allow bus to become not busy */
	ret = cpld_wait(i2c);
	if (ret)
		return ret;

	/*
	 * The CPLD I2C master is special built to read/write module EEPROMs.
	 * It is *not* a general purpose I2C master.  The clients of this
	 * master are *always* expected to be at,24c04 or sff-8436 compatible
	 * EEPROMs.
	 *
	 * For READ operations, we expect:
	 *
	 *    - number of messages is "2"
	 *    - msg[0] contains info about the offset within the EEPROM
	 *	- msg[0].len = 1
	 *	- msg[0].buf[0] contains the offset
	 *    - msg[1] contains info about the read payload
	 *
	 * For WRITE operations, we expect:
	 *
	 *    - number of messages is "1"
	 *    - msg[0] contains info about the offset within the EEPROM
	 *    - msg[0].buf[0] contains the offset
	 *    - msg[0] also contains info about the write payload
	 */

	/*
	 * The offset within the EEPROM is stored in msg[0].buf[0].
	 */
	offset = msgs[0].buf[0];

	if (num == 1)
		pmsg = &msgs[0];
	else
		pmsg = &msgs[1];

	if ((offset + pmsg->len) > 0x200)
		return -EINVAL;

	if (pmsg->flags & I2C_M_RD) {
		if (num != 2) {
			dev_warn(i2c->m_dev,
				 "Expected two i2c messages. Got %d\n", num);
			return -EINVAL;
		}

		if (msgs[0].len != 1) {
			dev_warn(i2c->m_dev,
				 "Expected mgs[0].len == 1. Got %d\n",
					msgs[0].len);
			return -EINVAL;
		}

		ret = cpld_i2c_read(i2c, pmsg->addr, offset, pmsg->buf,
				    pmsg->len);
	} else {
		ret = cpld_i2c_write(i2c, pmsg->addr, offset,
				     &pmsg->buf[1], (pmsg->len - 1));
	}

	return (ret < 0) ? ret : num;
}

static u32 cpld_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm cpld_algo = {
	.master_xfer = cpld_xfer,
	.functionality = cpld_functionality,
};

static struct i2c_adapter cpld_ops = {
	.owner = THIS_MODULE,
	.name = "CEL_CPLD adapter",
	.algo = &cpld_algo,
	.timeout = HZ,
};

static void cpld_i2c_bus_setup(struct cpld_i2c *i2c, u32 clock)
{
	u8 clk_freq = CPLD_I2C_CLK_50KHZ;

	if (clock == 100000)
		clk_freq = CPLD_I2C_CLK_100KHZ;

	i2c->m_clk_freq = clk_freq;
	iowrite8(clk_freq, i2c->m_base + CPLD_I2C_PORT_ID);

	/* Reset the I2C master logic */
	iowrite8(~MASTER_RESET_L, i2c->m_base + CPLD_I2C_STATUS);
	mdelay(3);
	iowrite8(MASTER_RESET_L, i2c->m_base + CPLD_I2C_STATUS);
}

static int i2c_bus_probe(struct platform_device *op)
{
	struct cpld_i2c *i2c;
	struct cpld_bus_data *bus_data;
	u32 clock = CPLD_I2C_CLK_100KHZ;
	int result = 0;

	i2c = kzalloc(sizeof(*i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	i2c->m_dev = &op->dev; /* for debug and error output */

	if (!op->dev.platform_data) {
		pr_err("error: no platform data\n");
		goto fail_map;
	}
	bus_data = op->dev.platform_data;
	i2c->m_base = ioport_map(bus_data->io_base, CPLD_REG_RANGE);

	cpld_ops.nr = bus_data->bus;
	i2c->m_adap = cpld_ops;
	i2c_set_adapdata(&i2c->m_adap, i2c);
	i2c->m_adap.dev.parent = &op->dev;
	i2c->m_adap.nr = cpld_ops.nr;

	result = i2c_add_numbered_adapter(&i2c->m_adap);
	if (result < 0) {
		dev_err(i2c->m_dev, "failed to add adapter %u\n",
			bus_data->bus);
		goto fail_map;
	}
	if (bus_data->clock)
		clock = bus_data->clock;

	cpld_i2c_bus_setup(i2c, clock);

	if (bus_data->timeout) {
		cpld_ops.timeout = bus_data->timeout * HZ / 1000000;
		if (cpld_ops.timeout < 5)
			cpld_ops.timeout = 5;
	}

	dev_set_drvdata(&op->dev, i2c);
	i2c->m_adap.dev.parent = &op->dev;

	return result;

 fail_map:
	kfree(i2c);
	return result;
};

static int i2c_bus_remove(struct platform_device *op)
{
	struct cpld_i2c *i2c = dev_get_drvdata(&op->dev);

	i2c_del_adapter(&i2c->m_adap);
	dev_set_drvdata(&op->dev, NULL);

	iounmap(i2c->m_base);
	kfree(i2c);
	return 0;
};

/* Structure for a device driver */
static struct platform_driver i2c_bus_driver = {
	.driver = {
		.name  = BUS_DRV_NAME,
		.owner = THIS_MODULE,
	},
	.probe	= i2c_bus_probe,
	.remove	= i2c_bus_remove,
};

/*
 * MUX Driver starts here
 */

#define MUX_DRV_NAME	"cel_redstone_dp_i2c_mux"

#define INVALID_CHANNEL		(0xFF)

struct cpld_vadapter {
	struct i2c_adapter *m_adapter;	/* virtual i2c adapter struct */
	u32		    m_channel;	/* MUX channel */
	struct llist_node   m_llnode;
};

struct cpld_i2c_mux {
	struct device	    *m_dev;	      /* for debug and error output */
	struct llist_head    m_vadapter_list; /* list of virtual i2c adapters */
	struct cpld_i2c	    *m_i2c_bus;	      /* parent I2C master device */
	struct i2c_adapter **m_adap_list;     /* list of adapters on this mux */
	u8		     m_last_channel;  /* last channel selected	  */
	u32		     m_mux_id;	      /* mux ID */
	int		     m_num_ports;     /* number of ports on this mux */
};

static int cpld_i2c_mux_select_chan(struct i2c_adapter *adap,
				    void *data, u32 channel)
{
	struct cpld_i2c_mux *mux = data;

	if (channel != mux->m_last_channel) {
		cpld_set_mux_reg(mux->m_i2c_bus, channel);
		mux->m_last_channel = channel;
	}

	return 0;
}

static void free_mux_data(struct cpld_i2c_mux *mux)
{
	int i;

	for (i = 0; i < mux->m_num_ports; i++)
		if (mux->m_adap_list[i])
			i2c_del_mux_adapter(mux->m_adap_list[i]);
	kfree(mux);
}

/*
 * I2C init/probing/exit functions
 */

static int i2c_mux_probe(struct platform_device *client)
{
	struct i2c_adapter  *adap = to_i2c_adapter(client->dev.parent);
	struct cpld_i2c *i2c = i2c_get_adapdata(adap);
	struct cpld_i2c_mux *mux;
	struct cpld_mux_data *mux_data;
	int rc = -ENODEV;
	int i;

	if (!client->dev.platform_data)
		pr_err("mux: no platform data\n");
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
	mux->m_adap_list = kzalloc(sizeof(struct i2c_adapter *) *
				   mux_data->mux_num_ports, GFP_KERNEL);

	for (i = 0; i < mux_data->mux_num_ports; i++) {
		struct i2c_adapter *adapter;
		int bus;
		int channel;

		channel = mux_data->mux_base_port_num + i;
		bus = mux_data->mux_ports_base_bus + channel;
		adapter = i2c_add_mux_adapter(adap, &client->dev, mux, bus,
					      channel, 0,
					      cpld_i2c_mux_select_chan,
					      NULL);
		if (!adapter) {
			pr_err("failed to register multiplexed adapter %u\n",
			       bus);
			kfree(mux);
			rc = -ENODEV;
			goto dealloc_exit;
		}
		mux->m_adap_list[i] = adapter;
	}
	dev_set_drvdata(&client->dev, mux);

	return 0;

dealloc_exit:
	free_mux_data(mux);
err_exit:
	return rc;
}

static int i2c_mux_remove(struct platform_device *client)
{
	free_mux_data(dev_get_drvdata(&client->dev));
	return 0;
}

static const struct platform_device_id i2c_mux_ids[] = {
	{ "cel_red_dp_cpld_mux", 0 },
	{ /* end of list */ },
};

static struct platform_driver i2c_mux_driver = {
	.driver = {
		.name  = MUX_DRV_NAME,
		.owner = THIS_MODULE,
	},
	.probe	  = i2c_mux_probe,
	.remove	  = i2c_mux_remove,
	.id_table = i2c_mux_ids,
};

/*
 * Module init/exit methods begin here.	 The init and exit methods
 * register/de-register two devices:
 *
 * - The i2c bus device
 * - The i2c mux device
 *
 */
static int __init cel_redstone_dp_i2c_init(void)
{
	int ret;

	/* First register the i2c adapter */
	ret = platform_driver_register(&i2c_bus_driver);
	if (ret) {
		pr_err("failed to register CPLD I2C driver: %d\n", ret);
		return ret;
	}

	/* Next register the i2c mux */
	ret = platform_driver_register(&i2c_mux_driver);
	if (ret) {
		pr_err("failed to register CPLD mux driver: %d\n", ret);
		platform_driver_unregister(&i2c_bus_driver);
		return ret;
	}

	return 0;
}

static void __exit cel_redstone_dp_i2c_exit(void)
{
	/* First remove the i2c mux driver */
	platform_driver_unregister(&i2c_mux_driver);

	/* Next remove the i2c adapter */
	platform_driver_unregister(&i2c_bus_driver);
}

module_init(cel_redstone_dp_i2c_init);
module_exit(cel_redstone_dp_i2c_exit);

MODULE_AUTHOR("David Yen <dhyen@cumulusnetworks.com>");
MODULE_DESCRIPTION("I2C Adapter/Mux Driver for Celestica Redstone-DP");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(BUS_MUX_DRV_VERSION);
