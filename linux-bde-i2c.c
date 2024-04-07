/*
 * I2C bus driver for Broadcom silicon
 *
 * Copyright (C) 2012 Cumulus Networks, LLC
 * Author: Cumulus Networks
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
 */

#include <stddef.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/i2c.h>
#include <linux/swab.h>
#include <asm/io.h>
#include <asm/delay.h>

/*
 * Compilation options
 */

/*
 * Define BDE_I2C_FORCEBIGENDIAN to configure the hardware for big-endian,
 * undefine to leave it alone and run adaptively.
 * However, we should always define it because linux-kernel-bde only
 * works in big-endian mode but doesn't set the hardware.
 */
#define BDE_I2C_FORCEBIGENDIAN

/*
 * Set BDE_I2C_DEBUG to the desired debug level.
 * There are four debug printk macros: D0, D1, D2, and D3.
 * D[123] are controlled by BDE_I2C_DEBUG and print with KERN_DEBUG.
 * D0 is always on, and uses KERN_WARNING.
 */
#define BDE_I2C_DEBUG 0

#define D0(a, ...) \
        printk(KERN_WARNING "%s: " a "\n", __FUNCTION__, ## __VA_ARGS__)

#if BDE_I2C_DEBUG < 1
#define D1(a...) ((void) 0)
#else
#define D1(a, ...) \
        printk(KERN_DEBUG "%s: " a "\n", __FUNCTION__, ## __VA_ARGS__)
#endif

#if BDE_I2C_DEBUG < 2
#define D2(a...) ((void) 0)
#else
#define D2(a, ...) \
        printk(KERN_DEBUG "%s: " a "\n", __FUNCTION__, ## __VA_ARGS__)
#endif

#if BDE_I2C_DEBUG < 3
#define D3(a...) ((void) 0)
#else
#define D3(a, ...) \
        printk(KERN_DEBUG "%s: " a "\n", __FUNCTION__, ## __VA_ARGS__)
#endif

/*
 * Stripped down inteface to Broadcom SDK linux-kernel-bde module
 */

struct ibde {
	const char *(*name)(void);
	int (*num_devices)(int type);
#define BDE_ALL_DEVICES	 0
	void *pad1[1];
	uint32_t (*get_dev_type)(int d);
#define BDE_PCI_DEV_TYPE 0x00000001
	void *pad2[9];
	int (*interrupt_connect)(int d, void (*isr)(void *), void *data);
	int (*interrupt_disconnect)(int d);
#define LKBDE_ISR2_DEV 0x8000
	void *pad3[8];
};

struct linux_bde_bus;

extern int linux_bde_create(struct linux_bde_bus *bus, struct ibde **bde);
extern int linux_bde_destroy(struct ibde *bde);
extern void *lkbde_get_hw_dev(int d);
extern void *lkbde_get_dev_virt(int d);
extern void *lkbde_get_dev_phys(int d);
extern int lkbde_irq_mask_set(int d, uint32_t addr,
                              uint32_t mask, uint32_t fmask);

static struct ibde *bde = NULL;

/*
 * Device state
 */

struct bde_i2c {
	struct i2c_adapter adapter;
	int n;				// device number for lkbde_* funcs
	struct pci_dev *pdev;
	volatile void *addr;
	volatile int intr;
	wait_queue_head_t wq;
	int skip_intr;
};

static struct bde_i2c *devs[4];
static int ndevs;

/*
 * Local functions
 */

static int __init bde_i2c_init(void);
static void bde_i2c_exit(void);

static u32 bde_i2c_functionality(struct i2c_adapter *a);
static int bde_i2c_master_xfer(struct i2c_adapter *a,
                               struct i2c_msg *msgs,
			       int nmsgs);
static int bde_i2c_xfer(struct bde_i2c *d, uint16_t addr, bool ten, bool read,
	                uint8_t *buf, int len);
static int bde_i2c_send_start(struct bde_i2c *d);
static int bde_i2c_send_addr(struct bde_i2c *d, uint16_t addr,
                             int ten, int read);
static int bde_i2c_recv_data(struct bde_i2c *d, uint8_t *buf, int len);
static int bde_i2c_send_data(struct bde_i2c *d, uint8_t *buf, int len);
static void bde_i2c_send_stop(struct bde_i2c *d);
static void bde_i2c_reset(struct bde_i2c *d);
static int bde_i2c_wait(struct bde_i2c *d);
static void bde_i2c_intr(void *data);

/*
 * I2C bus driver infrastructure
 */

static struct i2c_algorithm bde_i2c_algorithm = {
	.master_xfer = &bde_i2c_master_xfer,
	.functionality = &bde_i2c_functionality,
};

/*
 * Hardware
 */

#define REG_ADDR	0x120
#define REG_DATA	0x124
#define REG_CTRL	0x128
#define REG_STAT	0x12c
#define REG_CCR		0x12c
#define REG_XADDR	0x130
#define REG_RESET	0x13c

#define REG_CTRL_IEN	0x80		// interrupt enable
#define REG_CTRL_ENAB	0x40		// bus enable
#define REG_CTRL_STA	0x20		// master mode start
#define REG_CTRL_STP	0x10		// master mode stop
#define REG_CTRL_IFLG	0x08		// interrupt flag
#define REG_CTRL_AAK	0x04		// assert acknowledge

#define REG_CONFIG	0x10c
#define REG_IRQ_STAT	0x144
#define REG_IRQ_MASK	0x148
#define REG_ENDIAN_SEL	0x174
#define REG_RATE_ADJUST	0x1b4

#define REG_CONFIG_I2C	(1 << 11)
#define REG_IRQ_I2C	(1 << 18)	// I2C bit in both STAT and MASK

/*
 * Read and write hardware register
 */

static inline uint32_t
readreg(struct bde_i2c *d, unsigned reg)
{
	uint32_t data = __raw_readl(d->addr + reg);

#ifdef BDE_I2C_FORCEBIGENDIAN
	data = be32_to_cpu(data);
#else
	if ((__raw_readl(d->addr + REG_ENDIAN_SEL) & 0x01) != 0) {
		data = be32_to_cpu(data);
	} else {
		data = le32_to_cpu(data);
	}
#endif
	return data;
}

static inline void
writereg(struct bde_i2c *d, unsigned reg, uint32_t data)
{
#ifdef BDE_I2C_FLEXENDIAN
	data = cpu_to_be32(data);
#else
	if ((__raw_readl(d->addr + REG_ENDIAN_SEL) & 0x01) != 0) {
		data = cpu_to_be32(data);
	} else {
		data = cpu_to_le32(data);
	}
#endif
	__raw_writel(data, d->addr + reg);
}

/*
 * Module load & unload
 */

static int __init
bde_i2c_init(void)
{
	int i;
	int n;
	int error;

	D1("");
	if (linux_bde_create(NULL, &bde) < 0 || bde == NULL) {
		D0("linux_bde_create failed");
		return -ENODEV;
	}
	D2("linux_bde_create %p", bde);
	n = bde->num_devices(BDE_ALL_DEVICES);
	D2("%d device(s)", n);
	for (i = 0; i < n; i++) {
		struct bde_i2c *d;

		/*
		 * Allocate device structure, initialize it,
		 * hook it up to linux-kernel-bde and i2c infrastructure.
		 */

		D2("device %d type %#010x", i, bde->get_dev_type(i));
		if ((bde->get_dev_type(i) & BDE_PCI_DEV_TYPE) == 0) {
			D2("device %d not PCI", i);
			continue;
		}
		if (ndevs >= ARRAY_SIZE(devs)) {
			D0("too many devices");
			error = -ENODEV;
			goto bad;
		}
		if ((d = kzalloc(sizeof *d, GFP_KERNEL)) == NULL) {
			D0("kzalloc failed");
			error = -ENOMEM;
			goto bad;
		}
		d->adapter.owner = THIS_MODULE;
		strncpy(d->adapter.name, "BCM56845 I2C",
			ARRAY_SIZE(d->adapter.name));
		d->adapter.class = I2C_CLASS_HWMON;
		d->adapter.algo = &bde_i2c_algorithm;
		d->adapter.algo_data = d;
		d->n = i;
		d->pdev = lkbde_get_hw_dev(i);
		d->addr = lkbde_get_dev_virt(i);
		init_waitqueue_head(&d->wq);
		D1("device %d pci %s phys %p virt %p",
		   i,
		   d->pdev->bus == NULL ? NULL : d->pdev->bus->name,
		   lkbde_get_dev_phys(i),
		   d->addr);
		if (bde->interrupt_connect(i | LKBDE_ISR2_DEV, bde_i2c_intr, d) < 0) {
			D0("interrupt_connect failed");
			kfree(d);
			error = -EBUSY;		// XXX better errno?
			goto bad;
		}
		if ((error = i2c_add_adapter(&d->adapter)) != 0) {
			D0("i2c_add_adapter failed");
			bde->interrupt_disconnect(i | LKBDE_ISR2_DEV);
			kfree(d);
			goto bad;
		}
		devs[ndevs++] = d;
		printk(KERN_INFO "adding i2c bus %d on BCM56845 unit %d\n",
		       i2c_adapter_id(&d->adapter), d->n);

		/*
		 * Initialize hardware
		 */

		// endian
		D3("ENDIAN_SEL %#010x", readreg(d, REG_ENDIAN_SEL));
#ifdef BDE_I2C_FORCEBIGENDIAN
		writereg(d, REG_ENDIAN_SEL,
			 readreg(d, REG_ENDIAN_SEL) | 0x05050505);
		// XXX apply delay to every register write?
		readreg(d, REG_ENDIAN_SEL);
		readreg(d, REG_ENDIAN_SEL);
		readreg(d, REG_ENDIAN_SEL);
		readreg(d, REG_ENDIAN_SEL);
		D3("ENDIAN_SEL %#010x", readreg(d, REG_ENDIAN_SEL));
#endif

		// i2c enable
		D3("CONFIG %#010x", readreg(d, REG_CONFIG));
		writereg(d, REG_CONFIG,
			 readreg(d, REG_CONFIG) | REG_CONFIG_I2C);
		// XXX apply delay to every register write?
		readreg(d, REG_CONFIG);
		readreg(d, REG_CONFIG);
		readreg(d, REG_CONFIG);
		readreg(d, REG_CONFIG);
		D3("CONFIG %#010x", readreg(d, REG_CONFIG));

		// i2c rate
		D3("RATE_ADJUST %#010x", readreg(d, REG_RATE_ADJUST));

		bde_i2c_reset(d);
	}

	D1("done");
	return 0;
bad:
	D0("error %d", error);
	bde_i2c_exit();
	return error;
}

static void
bde_i2c_exit(void)
{
	int i;
	int error;

	D1("");
	for (i = 0; i < ndevs; i++) {
		struct bde_i2c *d = devs[i];
		if (d == NULL)
			continue;
		devs[i] = NULL;
		printk(KERN_INFO "removing i2c bus %d on BCM56845 unit %d\n",
		       i2c_adapter_id(&d->adapter), d->n);

		i2c_del_adapter(&d->adapter);
		if ((error = bde->interrupt_disconnect(d->n | LKBDE_ISR2_DEV)) != 0) {
			D0("%d interrupt_disconnect error %d", d->n, error);
			// can't really happen, ignore
		}

		// disable i2c registers
		D3("CONFIG %#010x", readreg(d, REG_CONFIG));
		writereg(d, REG_CONFIG,
			 readreg(d, REG_CONFIG) & ~REG_CONFIG_I2C);
		D3("CONFIG %#010x", readreg(d, REG_CONFIG));

		kfree(d);
	}
	ndevs = 0;
	linux_bde_destroy(bde);
	bde = NULL;
	D1("done");
}

/*
 * I2C "algorithm" functions
 */

static u32
bde_i2c_functionality(struct i2c_adapter *a)
{
	D3("%d", ((struct bde_i2c *) a->algo_data)->n);
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static int
bde_i2c_master_xfer(struct i2c_adapter *a,
                    struct i2c_msg *msgs,
                    int nmsgs)
{
	struct bde_i2c *d = (struct bde_i2c *) a->algo_data;
	int i;
	int error;

	D1("%d nmsgs %d", d->n, nmsgs);
	for (i = 0; i < nmsgs; i++) {
		struct i2c_msg *m = msgs + i;
		int ten = (m->flags & I2C_M_TEN) != 0;
		int read = (m->flags & I2C_M_RD) != 0;

		D1("msg %d addr %#x flags %#x(%s%s%s%s) len %u",
		   i, m->addr, m->flags,
		   m->flags & I2C_M_RD ? "r" : "w",
		   m->flags & I2C_M_TEN ? "10" : "7",
		   m->flags & I2C_M_RECV_LEN ? "l" : "",
		   m->flags & ~(I2C_M_TEN|I2C_M_RD|I2C_M_RECV_LEN) ? "?" : "",
		   m->len);
		if ((m->flags & I2C_M_RECV_LEN) != 0) {
			D0("I2C_M_RECV_LEN not supported");
			error = -EINVAL;
			goto out;
		}
		error = bde_i2c_xfer(d, m->addr, ten, read, m->buf, m->len);
		if (error != 0) {
			// we can't return partial success
			goto out;
		}
	}
	bde_i2c_send_stop(d);

	error = nmsgs;

out:
	D1("return %d", error);
	return error;
}

/*
 * Read and write
 * Stop is sent by caller, so don't do it here.
 */

static int
bde_i2c_xfer(struct bde_i2c *d,
	     uint16_t addr,
	     bool ten,
	     bool read,
	     uint8_t *buf,
	     int len)
{
	int error;

	if ((error = bde_i2c_send_start(d)) != 0)
		return error;
	if ((error = bde_i2c_send_addr(d, addr, ten, read)) != 0)
		return error;
	return (read ? bde_i2c_recv_data : bde_i2c_send_data)(d, buf, len);

#if 0 // XXX which is prettier?
	(error = bde_i2c_send_start(d)) == 0 &&
	(error = bde_i2c_send_addr(d, addr, ten, read)) == 0 &&
	(error = (read ? bde_i2c_recv_data : bde_i2c_send_data)(d, buf, len));
#endif
}

static int
bde_i2c_send_start(struct bde_i2c *d)
{
	uint8_t b;
	int error = 0;

	b = readreg(d, REG_CTRL);
	b &= ~REG_CTRL_IFLG;
	b |= REG_CTRL_STA;
	writereg(d, REG_CTRL, b);
	D2("CTRL %#04x %#04x", b, readreg(d, REG_CTRL));

	if ((error = bde_i2c_wait(d)) != 0) {
		bde_i2c_reset(d);
		goto out;
	}

	switch (b = readreg(d, REG_STAT)) {
	case 0x08:	// sent start
	case 0x10:	// sent repeated start
		break;
	default:
		D0("STAT %#04x unexpected", b);
		error = -EIO;
		bde_i2c_reset(d);
	}

out:
	D2("return %d", error);
	return error;
}

static int
bde_i2c_send_addr(struct bde_i2c *d,
                  uint16_t addr,
		  int ten,
		  int read)
{
	uint8_t b, a;
	int error = 0;

	if (ten) {
		D0("10-bit address not supported");
		error = -EINVAL;
		goto out;
	}
	//ASSERT((read & ~1) == 0);
	a = addr << 1 | read;
	writereg(d, REG_DATA, a);
	b = readreg(d, REG_CTRL);
	b &= ~REG_CTRL_IFLG;
	writereg(d, REG_CTRL, b);
	D2("CTRL %#04x %#04x DATA %#04x %#04x",
	   b, readreg(d, REG_CTRL), a, readreg(d, REG_DATA));

	if ((error = bde_i2c_wait(d)) != 0) {
		bde_i2c_reset(d);
		goto out;
	}

	switch (b = readreg(d, REG_STAT)) {
	case 0x18:	// ack (write)
	case 0x40:	// ack (read)
		break;
	case 0x20:	// no ack (write)
	case 0x48:	// no ack (read)
		D2("STAT %#04x no ack", b);
		error = -ENXIO;
		bde_i2c_send_stop(d);
		break;
	case 0x38:	// lost arbitration
		D2("STAT %#04x lost arbitration", b);
		error = -EAGAIN;
		break;
	case 0x68:	// lost arbitration, received slave addr, write
	case 0x78:	// lost arbitration, received general-call addr
	case 0xB0:	// lost arbitration, received slave addr, read
		// XXX these should be impossible for us
		D0("STAT %#04x unexpected", b);
		error = -EIO;
		bde_i2c_reset(d);
		break;
	default:
		D0("STAT %#04x unexpected", b);
		error = -EIO;
		bde_i2c_reset(d);
	}

out:
	D2("return %d", error);
	return error;
}

static int
bde_i2c_recv_data(struct bde_i2c *d,
                  uint8_t *buf,
		  int len)
{
	int i;
	uint8_t b, c;
	int error = 0;

	for (i = 0; i < len; i++) {
		b = readreg(d, REG_CTRL);
		b &= ~REG_CTRL_IFLG;
		if (i < len - 1) {
			b |= REG_CTRL_AAK;
		} else {
			b &= ~REG_CTRL_AAK;
		}
		writereg(d, REG_CTRL, b);
		D2("CTRL %#04x %#04x", b, readreg(d, REG_CTRL));

		if ((error = bde_i2c_wait(d)) != 0) {
			bde_i2c_reset(d);
			break;
		}

		switch (b = readreg(d, REG_STAT)) {
		case 0x50:	// received data, sent ack
		case 0x58:	// received data, sent no ack
			c = readreg(d, REG_DATA);
			D2("DATA %#04x", c);
			buf[i] = c;
			continue;
		case 0x38:	// lost arbitration
			D2("STAT %#04x lost arbitration", b);
			/*
			 * We can actually call this a success because
			 * we can only lose on nack so we're, like, done.
			 */
			error = -EAGAIN;
			break;
		default:
			D0("STAT %#04x unexpected", b);
			error = -EIO;
			bde_i2c_reset(d);
		}
		break;
	}

	D2("return %d", error);
	return error;
}

static int
bde_i2c_send_data(struct bde_i2c *d,
                  uint8_t *buf,
		  int len)
{
	int i;
	uint8_t b, c;
	int error = 0;

	for (i = 0; i < len; i++) {
		c = buf[i];
		writereg(d, REG_DATA, c);
		b = readreg(d, REG_CTRL);
		b &= ~REG_CTRL_IFLG;
		writereg(d, REG_CTRL, b);
		D2("CTRL %#04x %#04x DATA %#04x %#04x",
		   b, readreg(d, REG_CTRL), c, readreg(d, REG_DATA));

		if ((error = bde_i2c_wait(d)) != 0) {
			bde_i2c_reset(d);
			break;
		}

		switch (b = readreg(d, REG_STAT)) {
		case 0x28:	// ack
			continue;
		case 0x30:	// no ack
			D2("STAT %#04x no ack", b);
			// XXX how to return partial write?
			error = -EIO;
			bde_i2c_send_stop(d);
			break;
		case 0x38:	// lost arbitration
			D2("STAT %#04x lost arbitration", b);
			error = -EAGAIN;
			break;
		default:
			D0("STAT %#04x unexpected", b);
			error = -EIO;
			bde_i2c_reset(d);
		}
		break;
	}

	D2("return %d", error);
	return error;
}

static void
bde_i2c_send_stop(struct bde_i2c *d)
{
	uint8_t b;

	b = readreg(d, REG_CTRL);
	b &= ~REG_CTRL_IFLG;
	b |= REG_CTRL_STP;
	writereg(d, REG_CTRL, b);
	D2("send stop CTRL %#04x", readreg(d, REG_CTRL));
	// no completion interrupt for this operation
	// XXX but should we wait and if so how?

#if BDE_I2C_DEBUG >= 3
	udelay(100);
	D3("CTRL %#04x", readreg(d, REG_CTRL));
	D3("STAT %#04x", readreg(d, REG_STAT));
#endif
}

/*
 * Hardware reset, used on initialization and errors
 */

static void
bde_i2c_reset(struct bde_i2c *d)
{
	uint8_t b;

	// block interrupt
	D3("IRQ_STAT %#010x", readreg(d, REG_IRQ_STAT));
	D3("IRQ_MASK %#010x", readreg(d, REG_IRQ_MASK));
	lkbde_irq_mask_set(d->n | LKBDE_ISR2_DEV, REG_IRQ_MASK,
			   0, REG_IRQ_I2C);
	D3("IRQ_MASK %#010x", readreg(d, REG_IRQ_MASK));

	// dump registers
	D3("ADDR %#04x", readreg(d, REG_ADDR));
	D3("DATA %#04x", readreg(d, REG_DATA));
	D3("CTRL %#04x", readreg(d, REG_CTRL));
	D3("STAT %#04x", readreg(d, REG_STAT));
	D3("XADDR %#04x", readreg(d, REG_XADDR));

	// reset
	D2("reset");
	writereg(d, REG_RESET, 0xff);
	udelay(1000);

	// dump registers again
	D3("ADDR %#04x", readreg(d, REG_ADDR));
	D3("DATA %#04x", readreg(d, REG_DATA));
	D3("CTRL %#04x", readreg(d, REG_CTRL));
	D3("STAT %#04x", readreg(d, REG_STAT));
	D3("XADDR %#04x", readreg(d, REG_XADDR));

	// there should be no pending interrupt at this point
	D3("IRQ_STAT %#010x", readreg(d, REG_IRQ_STAT));

	// initialize REG_CTRL
	b = 0;
	b |= REG_CTRL_IEN;
	b |= REG_CTRL_ENAB;
	writereg(d, REG_CTRL, b);
	D3("write CTRL %#04x %#04x", b, readreg(d, REG_CTRL));

	// initialize REG_ADDR
	b = 0;
	writereg(d, REG_ADDR, b);
	D3("write ADDR %#04x %#04x", b, readreg(d, REG_ADDR));

	// initialize REG_CCR
	b = 2 << 3 | 0; // 83333Hz
	D3("write CCR %#04x", b);
	writereg(d, REG_CCR, b); // 83333Hz
}

/*
 * Wait for completion interrupt
 */

static int
bde_i2c_wait(struct bde_i2c *d)
{
	int error = 0;

	/*
	 * Delay a bit before reenabling interrupt.
	 * XXX apply delay to every register write?
	 *
	 * We're generally called right after clearing IFLG.
	 * Apparently, the hardware doesn't immediately lower
	 * the interrupt line.  So we hang around a bit before
	 * reenabling interrupt.  Otherwise, we get a spurious
	 * interrupt immediately.
	 *
	 * There doesn't appear to be an easy way to do this
	 * adaptively.  While we can observe this condition
	 * by watching the REG_IRQ_I2C bit in REG_IRQ_STAT
	 * (initially high, then low, then high again, which is
	 * how we diagnosed it in the first place),
	 * we can't do it in a reliable, race-free way.
	 *
	 * The number 7 is arrived at empirically.
	 */
	{
		int i;
		for (i = 7; --i >= 0;)
			readreg(d, REG_CTRL);
	}

	d->intr = 0;
	D3("%d IRQ_MASK %#010x", d->n, readreg(d, REG_IRQ_MASK));
	lkbde_irq_mask_set(d->n | LKBDE_ISR2_DEV, REG_IRQ_MASK,
			   REG_IRQ_I2C, REG_IRQ_I2C);
	D3("%d IRQ_MASK %#010x", d->n, readreg(d, REG_IRQ_MASK));

	error = wait_event_interruptible_timeout(d->wq, d->intr, HZ * 10);
	if (!d->intr) {
		if (error >= 0) {
			D2("%d timout %d", d->n, error);
			error = -EBUSY;
		}
		goto out;
	}
	error = 0;

out:
	D3("%d return %d STAT %#04x", d->n, error, readreg(d, REG_STAT));
	return error;
}

static void
bde_i2c_intr(void *data)
{
	struct bde_i2c *d = (struct bde_i2c *) data;
	uint32_t s, m;

	s = readreg(d, REG_IRQ_STAT);
	m = readreg(d, REG_IRQ_MASK);
	D3("%d IRQ_STAT %#010x IRQ_MASK %#010x", d->n, s, m);
	if ((s & m & REG_IRQ_I2C) == 0) {
		D3("%d not this unit", d->n);
		return;
	}

	/*
	 * XXX This looks for the symptom of the spurious interrupt
	 * mentioned in bde_i2c_wait().  We know IFLG is lowered before
	 * bde_i2c_wait() is called, so if it's high now, then
	 * everything is good.  If it's low, then this interrupt
	 * is spurious and we just try again.
	 */
	{
		uint8_t b;
		b = readreg(d, REG_CTRL);
		if ((b & REG_CTRL_IFLG) == 0 && ++d->skip_intr < 10) {
			D3("%d CTRL %#04x skip %d", d->n, b, d->skip_intr);
			return;
		}
		if (d->skip_intr > 0) {
			D0("%d %d spurious interrupt(s)", d->n, d->skip_intr);
			d->skip_intr = 0;
		}
	}

	lkbde_irq_mask_set(d->n | LKBDE_ISR2_DEV, REG_IRQ_MASK,
			   0, REG_IRQ_I2C);
	D3("%d IRQ_MASK %#010x", d->n, readreg(d, REG_IRQ_MASK));
	D3("%d intr %d -> 1", d->n, d->intr);
	d->intr = 1;
	wake_up(&d->wq);
}


MODULE_AUTHOR("Cumulus Networks, LLC");
MODULE_DESCRIPTION("I2C bus driver for Broadcom BCM56845");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

module_init(bde_i2c_init);
module_exit(bde_i2c_exit);
