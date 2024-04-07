/*
 * Copyright (C) 2013 Cumulus Networks, Inc.
 * Author: Curt Brune <curt@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

#include "cel-xp-platform.h"
#include "cel-xp-muxpld.h"

#define BUS_DRV_NAME    "cel_xp_i2c_bus"
#define BUS_MUX_DRV_VERSION "1.0"

/*
 * The CEL_CPLD_I2C_XXX offsets are relative to the start of the "I2C Port Id reg"
 * within each CPLD:
 *
 *
 */
#define CEL_CPLD_I2C_PORT_ID        0x00
#define CEL_CPLD_I2C_OPCODE         0x01
#define CEL_CPLD_I2C_DEV_ADDR       0x02
#define CEL_CPLD_I2C_CMD_BYTE0      0x03
#define CEL_CPLD_I2C_CMD_BYTE1      0x04
#define CEL_CPLD_I2C_CMD_BYTE2      0x05
#define CEL_CPLD_I2C_CSR            0x06
#define CEL_CPLD_I2C_WRITE_DATA     0x10
#define WRITE_DATA(x) (CEL_CPLD_I2C_WRITE_DATA + (x))
#define CEL_CPLD_I2C_READ_DATA      0x20
#define READ_DATA(x) (CEL_CPLD_I2C_READ_DATA + (x))
#define CEL_CPLD_REG_RANGE          0x2f

#define CEL_CPLD_I2C_CLK_50KHZ      0x00
#define CEL_CPLD_I2C_CLK_100KHZ     0x40

#define CSR_MASTER_ERROR            0x80
#define CSR_BUSY                    0x40
#define CSR_MASTER_RESET_L          0x01

#define OPCODE_DATA_LENGTH_SHIFT    4
#define OPCODE_CMD_LENGTH           1

#define DEV_ADDR_READ_OP            0x1
#define DEV_ADDR_WRITE_OP           0x0

struct cel_cpld_i2c {
	struct device     *m_dev;
	void __iomem      *m_base;
	struct i2c_adapter m_adap;
	u8                 m_clk_freq;
};

static inline void cel_cpld_set_mux_reg(struct cel_cpld_i2c *i2c, int channel)
{
	iowrite8(i2c->m_clk_freq | (channel & 0x3F),
	       i2c->m_base + CEL_CPLD_I2C_PORT_ID);
}

/*
 * Wait up to 1 second for the controller to be come non-busy.
 *
 * Returns:
 *   - success:  0
 *   - failure:  negative status code
 */
static int cel_cpld_wait(struct cel_cpld_i2c *i2c)
{
	unsigned long orig_jiffies = jiffies;
	int rc = 0;
	u8 csr;

	/* Allow bus up to 1s to become not busy */
	while ((csr = ioread8(i2c->m_base + CEL_CPLD_I2C_CSR)) & CSR_BUSY) {
			if (signal_pending(current)) {
			return -EINTR;
		}
		if (time_after(jiffies, orig_jiffies + HZ)) {
			dev_warn(i2c->m_dev, "Bus busy timeout\n");
			rc = -ETIMEDOUT;
			break;
		}
		schedule();
	}

	if (csr & CSR_MASTER_ERROR) {
		/* Typically this means the SFP+ device is not present. */
		/* Clear master error with the master reset. */
		iowrite8(~CSR_MASTER_RESET_L,
		       i2c->m_base + CEL_CPLD_I2C_CSR);
		udelay(3000);
		iowrite8(CSR_MASTER_RESET_L,
		       i2c->m_base + CEL_CPLD_I2C_CSR);
		rc = rc ? rc : -EIO;
	}

	return rc;
}

static int cel_cpld_i2c_write(struct cel_cpld_i2c *i2c, int target,
			      u8 offset, const u8 *data, int length)
{
	u8 tmp, xfer_len, i;
	int ret, total_xfer = 0;

	/* The CEL-CPLD I2C master writes in units of 8 bytes */
	while (length > 0) {

		/* Configure byte offset within device */
		iowrite8(offset + total_xfer,
		       i2c->m_base + CEL_CPLD_I2C_CMD_BYTE0);

		/* Configure transfer length - max of 8 bytes */
		xfer_len = (length > 8) ? 8 : length;
		tmp = (xfer_len << OPCODE_DATA_LENGTH_SHIFT);
		tmp |= OPCODE_CMD_LENGTH;
		iowrite8(tmp, i2c->m_base + CEL_CPLD_I2C_OPCODE);

		/* Load the transmit data into the send buffer */
		for (i = 0; i < xfer_len; i++)
			iowrite8(data[total_xfer + i], i2c->m_base + WRITE_DATA(i));

		/* Initiate write transaction */
		tmp = (target << 1) | DEV_ADDR_WRITE_OP;
		iowrite8(tmp, i2c->m_base + CEL_CPLD_I2C_DEV_ADDR);

		/* Wait for transfer completion */
		ret = cel_cpld_wait(i2c);
		if (ret)
			return ret;

		total_xfer += xfer_len;
		length -= xfer_len;
	}

	return 0;
}

static int cel_cpld_i2c_read(struct cel_cpld_i2c *i2c, int target,
			     u8 offset, u8 *data, int length)
{
	u8 tmp, xfer_len, i;
	int ret, total_xfer = 0;

	/* The CEL-CPLD I2C master reads in units of 8 bytes */
	while (length > 0) {

		/* Configure byte offset within device */
		iowrite8(offset + total_xfer,
		       i2c->m_base + CEL_CPLD_I2C_CMD_BYTE0);

		/* Configure transfer length - max of 8 bytes */
		xfer_len = (length > 8) ? 8 : length;
		tmp = (xfer_len << OPCODE_DATA_LENGTH_SHIFT);
		tmp |= OPCODE_CMD_LENGTH;
		iowrite8(tmp, i2c->m_base + CEL_CPLD_I2C_OPCODE);

		/* Initiate read transaction */
		tmp = (target << 1) | DEV_ADDR_READ_OP;
		iowrite8(tmp, i2c->m_base + CEL_CPLD_I2C_DEV_ADDR);

		/* Wait for transfer completion */
		ret = cel_cpld_wait(i2c);
		if (ret)
			return ret;

		/* Gather up the results */
		for (i = 0; i < xfer_len; i++)
			data[total_xfer + i] = ioread8(i2c->m_base + READ_DATA(i));

		total_xfer += xfer_len;
		length -= xfer_len;
	}

	return 0;
}

static int cel_cpld_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	struct i2c_msg *pmsg;
	int ret = 0;
	struct cel_cpld_i2c *i2c = i2c_get_adapdata(adap);
	u8 offset;

	/* Allow bus to become not busy */
	ret = cel_cpld_wait(i2c);
	if (ret)
		return ret;

	/*
	 * This is somewhat gimpy.
	 *
	 * The CEL-CPLD I2C master is special built to read/write SFP+
	 * EEPROMs only.  It is *not* a general purpose I2C master.
	 * The clients of this master are *always* expected to be
	 * "at,24c04" or "sff-8436 based qsfp" compatible EEPROMs.
	 *
	 * As such we have the following expectations for READ operation:
	 *
	 *    - number of messages is "2"
	 *    - msg[0] contains info about the offset within the 512-byte EEPROM
	 *      - msg[0].len = 1
	 *      - msg[0].buf[0] contains the offset
	 *    - msg[1] contains info about the read payload
	 *
	 * As such we have the following expectations for WRITE operation:
	 *
	 *    - number of messages is "1"
	 *    - msg[0] contains info about the offset within the 512-byte EEPROM
	 *    - msg[0].buf[0] contains the offset
	 *    - msg[0] also contains info about the write payload
	 */

	/*
	 * The offset within the EEPROM is stored in msg[0].buf[0].
	 */
	offset = msgs[0].buf[0];

	if (num == 1) {
		pmsg = &msgs[0];
	} else {
		pmsg = &msgs[1];
	}

	if ((offset + pmsg->len) > 0x200)
		return -EINVAL;

	if (pmsg->flags & I2C_M_RD) {
		if (num != 2) {
			dev_warn(i2c->m_dev, "Expecting 2 i2c messages. Got %d\n", num);
			return -EINVAL;
		}

		if (msgs[0].len != 1) {
			dev_warn(i2c->m_dev, "Expecting mgs[0].len == 1. Got %d\n",
					msgs[0].len);
			return -EINVAL;
		}

		ret = cel_cpld_i2c_read(i2c, pmsg->addr, offset, pmsg->buf, pmsg->len);
	} else {
		ret = cel_cpld_i2c_write(i2c, pmsg->addr, offset, &(pmsg->buf[1]), (pmsg->len - 1));
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

static const struct i2c_adapter cel_cpld_ops = {
	.owner = THIS_MODULE,
	.name = "CEL_CPLD adapter",
	.algo = &cel_cpld_algo,
	.timeout = HZ,
};

static void cel_cpld_i2c_bus_setup(struct cel_cpld_i2c *i2c, u32 clock)
{
	u8 clk_freq = CEL_CPLD_I2C_CLK_50KHZ;

	if (clock == 100000)
		clk_freq = CEL_CPLD_I2C_CLK_100KHZ;

	i2c->m_clk_freq = clk_freq;
	iowrite8(clk_freq, i2c->m_base + CEL_CPLD_I2C_PORT_ID);

	/* Reset the I2C master logic */
	iowrite8(~CSR_MASTER_RESET_L, i2c->m_base + CEL_CPLD_I2C_CSR);
	udelay(3000);
	iowrite8(CSR_MASTER_RESET_L, i2c->m_base + CEL_CPLD_I2C_CSR);

}

static int cel_cpld_i2c_bus_probe(struct platform_device *op)
{
	struct cel_cpld_i2c *i2c;
	struct cpld_bus_data *bus_data;
	u32 clock = 100000;
	int result = 0;

	i2c = kzalloc(sizeof(*i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	if (!op->dev.platform_data) {
		pr_err("error: no platform data\n");
		goto fail_map;
	}
	bus_data = op->dev.platform_data;

	i2c->m_dev = &op->dev; /* for debug and error output */
	i2c->m_base = ioport_map(bus_data->io_base, CEL_CPLD_REG_RANGE);
	i2c->m_adap = cel_cpld_ops;
	i2c->m_adap.dev.parent = &op->dev;
	i2c->m_adap.nr = bus_data->bus;
	i2c->m_adap.timeout = bus_data->timeout * HZ / 1000000;
	if (i2c->m_adap.timeout < 5) {
		i2c->m_adap.timeout = 5;
		dev_warn(i2c->m_dev, "clipped adapter timeout to 5\n");
	}
	i2c_set_adapdata(&i2c->m_adap, i2c);
	if (bus_data->clock) {
		clock = bus_data->clock;
	}
	cel_cpld_i2c_bus_setup(i2c, clock);

	result = i2c_add_numbered_adapter(&i2c->m_adap);
	if (result < 0) {
		dev_err(i2c->m_dev, "failed to add adapter %u\n", bus_data->bus);
	}

	dev_set_drvdata(&op->dev, i2c);

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

#define MUX_DRV_NAME    "cel-xp-cpld-i2c-mux"

#define INVALID_CHANNEL		(0xFF)

struct cel_cpld_vadapter {
	struct i2c_adapter *m_adapter;  /* virtual i2c adapter struct */
	u32                 m_channel;  /* MUX channel */
	struct llist_node   m_llnode;
};

struct cel_cpld_i2c_mux {
	struct llist_head    m_vadapter_list; /* list of virtual i2c adapters */
	struct cel_cpld_i2c *m_i2c_bus;       /* parent I2C master device */
	u8                   m_last_channel;  /* last channel selected    */
	u32                  m_mux_id;        /* mux ID */
	int                  m_num_ports;     /* number of ports on this mux */
};

static int cel_cpld_i2c_mux_select_chan(struct i2c_mux_core *muxc, u32 channel)
{
	struct cel_cpld_i2c_mux *mux = i2c_mux_priv(muxc);

	if (channel != mux->m_last_channel) {
		cel_cpld_set_mux_reg(mux->m_i2c_bus, channel);
		mux->m_last_channel = channel;
	}

	return 0;
}

static int cel_cpld_i2c_mux_deselect_chan(struct i2c_mux_core *muxc, u32 channel)
{
	struct cel_cpld_i2c_mux *mux = i2c_mux_priv(muxc);

	cel_cpld_set_mux_reg(mux->m_i2c_bus, INVALID_CHANNEL);
	mux->m_last_channel = INVALID_CHANNEL;

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
		pr_err("mux: no platform data\n");
		return -EINVAL;
	}
	mux_data = client->dev.platform_data;

	mux = kmalloc(sizeof(struct cel_cpld_i2c_mux), GFP_KERNEL);
	if (!mux) {
		rc = -ENOMEM;
		goto err_exit;
	}
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
			     cel_cpld_i2c_mux_deselect_chan);
	if (!muxc) {
		rc = -ENOMEM;
		goto err_exit;
	}
	muxc->priv = mux;
	platform_set_drvdata(client, muxc);

	for (i = 0; i < mux_data->mux_num_ports; i++) {
		int bus;
		int channel;

		channel = mux_data->mux_base_port_num + i;
		bus = RXP_I2C_CPLD_MUX_FIRST_PORT + channel;
		if (i2c_mux_add_adapter(muxc, bus, channel, 0)) {
		     pr_err("failed to register multiplexed adapter %u\n", bus);
		     rc = -ENODEV;
		     goto dealloc_exit;
		}
	}
	dev_set_drvdata(&client->dev, mux);

	return 0;

dealloc_exit:
	i2c_mux_del_adapters(muxc);
	i2c_put_adapter(muxc->parent);
err_exit:
	kfree(mux);
	return rc;
}

static int cel_cpld_i2c_mux_remove(struct platform_device *client)
{
	struct i2c_mux_core *muxc = platform_get_drvdata(client);

	if(muxc) {
		kfree(muxc->priv);
		i2c_mux_del_adapters(muxc);
		i2c_put_adapter(muxc->parent);
	}

	return 0;
}

static const struct platform_device_id cel_cpld_i2c_mux_ids[] = {
	{ "cel_xp_cpld_i2c_mux", 0 },
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
		printk(KERN_ERR"%s(): platform_driver_register() failed: %d\n",
		       __func__, rc);
		return rc;
	}

	/* Next register the i2c mux */
	rc = platform_driver_register(&cel_cpld_mux_driver);
	if (rc) {
		printk(KERN_ERR"%s(): i2c_add_driver() failed: %d\n",
		       __func__, rc);
		platform_driver_unregister(&cel_cpld_i2c_bus_driver);
	}

	return rc;
}

static void __exit cel_cpld_i2c_exit(void)
{
	/* First remove the i2c mux driver */
	platform_driver_unregister( &cel_cpld_mux_driver);

	/* Next remove the i2c adapter */
	platform_driver_unregister(&cel_cpld_i2c_bus_driver);
}

module_init(cel_cpld_i2c_init);
module_exit(cel_cpld_i2c_exit);

MODULE_AUTHOR("Alan Liebthal <alanl@cumulusnetworks.com>");
MODULE_DESCRIPTION("I2C Adapter/Mux Driver for Celestica XP CPLD");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(BUS_MUX_DRV_VERSION);
