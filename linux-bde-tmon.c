/*
 * Sysfs thermal monitoring support for Broadcom silicon
 *
 * Copyright (C) 2011 Cumulus Networks, LLC
 * Author: JR Rivers <jrrivers@cumulusnetworks.com>
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
#include <linux/hwmon.h>
#include <linux/swab.h>

#include <asm/io.h>

/*------------------------------------------------------------------------------
 *
 * Export thermal monitoring to sysfs for StrataSwitchV devices
 *
 * We export one temperature value for each device after finding the worst
 * case value out of all available monitors.  We also maintain and export
 * a "min" and "max" value that are used for integration with lm-sensors
 * based programs.
 *
 * Temperature output follows the linux hwmon convention of milli-degree
 * Celcius, and if the thermal monitoring is disabled or we don't know the
 * device, we output the maximum temp of 125C (125000).
 *
 * This code is specific to the devices known at the time of writing,
 * it can get more modular if/when the chip thermal monitoring implementation
 * changes.
 *
 * offsets are hard coded to avoid incorrect #include polluting #defines
 */


/*------------------------------------------------------------------------------
 *
 * Broadcom Linux BDE peering information
 *
 * simplified down to the part we need/use to avoid contamination
 */
struct ibde {
	const char *(*name)(void);
	int     (*num_devices)(int type);
#define BDE_ALL_DEVICES		0
	uint32_t * padding1;
	uint32_t (*get_dev_type)(int d);
#define BDE_PCI_DEV_TYPE	0x00000001
	uint32_t * padding[19];
};

static struct ibde * tmon_bde = NULL;

extern int linux_bde_create(void * bus, struct ibde ** bde);
extern void * lkbde_get_hw_dev(int d);
extern void * lkbde_get_dev_virt(int d);
extern void * lkbde_get_dev_phys(int d);


/* Supported PCI devices */
static uint16_t trident_devid[] = {
	/* Trident */
	0xb840,
	0xb841,
	0xb843,
	0xb845,
	/* Trident Plus */
	0xb842,
	0xb844,
	0xb846,
	0xb549,
	/* Titan */
	0xb743,
	0xb745,
	/* Titan Plus */
	0xb744,
	0xb746,
};

static int ntrident_devids = ARRAY_SIZE(trident_devid);


/*------------------------------------------------------------------------------
 *
 * Device data and driver data structures
 *
 */

static const char driver_name[] = "linux_bde_tmon";
#define DRIVER_VERSION "1.0"

/*
 * Information that we need for each device
 */
enum tmon_type_t {
     none = 0,
     trident = 1
};

struct tmon_dev_info {
	enum tmon_type_t type;
	volatile void * base;
	volatile void * phys;
	struct device * dev;
	struct device * hdev;
	unsigned long max;	/* non-functional storage for lm-sensors */
	unsigned long hyst;	/* non-functional storage for lm-sensors */
};

#define MAX_DEVICES 16
static struct tmon_dev_info tmon_info[MAX_DEVICES];
static int ntmon;

static int get_tmon_idx(struct device * dev)
{
	int i;

	for (i = 0; i < ntmon; i++) {
		if (dev == tmon_info[i].dev) {
			return i;
		}
	}
	return -ENODEV;
};

#define TMON_MAX_VALUE  150000
#define TMON_MAX_THRESH 105000
#define TMON_MAX_HYST    50000


/*------------------------------------------------------------------------------
 *
 * Attribute and attribute_group declarations
 *
 * - one set of attributes pointing to the steering function
 * - a group per like-device type with the correct number of attributes
 */

static ssize_t name(struct device * dev, struct device_attribute * dattr,
		    char * buf)
{
     return sprintf(buf, "%s\n", driver_name);
};

static ssize_t tmon_show(struct device *, struct device_attribute *, char *);
static ssize_t tmon_get(struct device *, struct device_attribute *, char *);
static ssize_t tmon_set(struct device *, struct device_attribute *, const char *,
			size_t count);

static DEVICE_ATTR(name, S_IRUGO,  name, NULL);
static DEVICE_ATTR(temp1_input, S_IRUGO, tmon_show, NULL);
static DEVICE_ATTR(temp1_max, S_IRUGO | S_IWUSR, tmon_get, tmon_set);
static DEVICE_ATTR(temp1_max_hyst, S_IRUGO | S_IWUSR, tmon_get, tmon_set);

static struct attribute * tmon_attrs[] = {
	&dev_attr_name.attr,
	&dev_attr_temp1_input.attr,
	&dev_attr_temp1_max.attr,
	&dev_attr_temp1_max_hyst.attr,
	NULL,
};

static struct attribute_group tmon_attr_group = {
	.attrs = tmon_attrs,
};


/*------------------------------------------------------------------------------
 *
 * Trident, Titan, and other StrataSwitchV devices
 *
 * Implementation specific
 */

/*
 * Enable monitoring
 */
void trident_tmon_enable(volatile void * base )
{
	uint32_t tmp;
	int swap;

	/* Determine endianness of PIO (CMIC_ENDIANESS_SEL) */
	tmp = readl(base + 0x0174);
	swap = ((tmp & 0x01010101) != 0);

	/* reset and enable thermal monitor (CMIC_THERMAL_MON_CTRL)*/
	tmp = readl(base + 0x0088);
	if (swap) tmp = swab32(tmp);
	tmp &= ~0x00020000;	/* clear POWER_DOWN */
	tmp |=  0x00010000;	/* set VTMON_RSTB (active low reset) */
	tmp &= ~0x00000007;	/* set BG_ADJ to 1.184V */
	tmp |=  0x00000001;
	if (swap) tmp = swab32(tmp);
	writel(tmp, base + 0x0088);
};

/*
 * show temperature values (in millidegree C)
 */
static ssize_t trident_tmon_show(struct device * dev,
				 struct device_attribute * dattr,
				 char * buf)
{
	int idx;
	int i;
	struct tmon_dev_info * t;
	uint32_t offset;
	uint32_t tmp;
	int swap;
	uint32_t max = 0;

	/* find the device's information */
	idx =  get_tmon_idx(dev);
	BUG_ON(idx < 0);
	t = &tmon_info[idx];

	/* Determine endianness of PIO (CMIC_ENDIANESS_SEL) */
	tmp = readl(t->base + 0x0174);
	swap = ((tmp & 0x01010101) != 0);
	
	/* is tmon enabled and out of reset (CMIC_THERMAL_MON_CTRL) */
	tmp = readl(t->base + 0x0088);
	if (swap) tmp = swab32(tmp);
	if (((tmp & 0x00020000) != 0x00000000) ||
	    ((tmp & 0x00010000) != 0x00010000)) {
		trident_tmon_enable(t->base);
		return sprintf(buf, "%d\n", TMON_MAX_VALUE);
	}

	for (i = 0; i < 8; i++) {
		/* reg offset of target diode (CMIC_THERMAL_MON_RESULT_x) */
		switch (i) {
		case 0 : offset = 0x0090; break;
		case 1 : offset = 0x0094; break;
		case 2 : offset = 0x095c; break; 
		case 3 : offset = 0x0960; break;
		case 4 : offset = 0x0964; break;
		case 5 : offset = 0x0968; break;
		case 6 : offset = 0x0e40; break;
		case 7 : offset = 0x0e44; break;
		}
	
		/* CMIC_THERMAL_MON_RESULT_x.value */
		tmp = readl(t->base + offset);
		if (swap) tmp = swab32(tmp);
		tmp &= 0x000003ff;

		/* calculate per programmer's guide (CMIC_THERMAL_MON_RESULT_x)
		 * - t = 410 - (0.5424 * register)
		 * - implement millidegree C to (max of 400mC rounding error)
		 */
		tmp *= 542;
		if (tmp > 410000) {
			/* ignore negative readings */
			tmp = 0;
		} else {
			tmp = 410000 - tmp;
		}
		
		/* keep the largest reading */
		if (tmp > TMON_MAX_VALUE) {
			max = TMON_MAX_VALUE;
		} else if (tmp > max) {
			max = tmp;
		}
	}
	return sprintf(buf, "%d\n", max);
};

/*------------------------------------------------------------------------------
 *
 * Main entry points for sysfs interface
 *
 */

static ssize_t tmon_get(struct device * dev,
			struct device_attribute * dattr,
			char * buf)
{
	int idx;

	idx = get_tmon_idx(dev);
	BUG_ON(idx < 0);
	
	if (strcmp(dattr->attr.name, "temp1_max") == 0) {
		return sprintf(buf, "%lu\n", tmon_info[idx].max);
	} else if (strcmp(dattr->attr.name, "temp1_max_hyst") == 0) {
		return sprintf(buf, "%lu\n", tmon_info[idx].hyst);
	} else {
		return -EINVAL;
	}
};

static ssize_t tmon_set(struct device * dev,
			struct device_attribute * dattr,
			const char * buf, size_t count)
{
	int idx;
	unsigned long v = simple_strtoul(buf, NULL, 10);

	v = clamp_val(v, 0, TMON_MAX_VALUE);

	idx = get_tmon_idx(dev);
	BUG_ON(idx < 0);
	
	if (strcmp(dattr->attr.name, "temp1_max") == 0) {
		tmon_info[idx].max = v;
	} else if (strcmp(dattr->attr.name, "temp1_max_hyst") == 0) {
		tmon_info[idx].hyst = v;
	} else {
		return -EINVAL;
	}
	return count;
};

static enum tmon_type_t tmon_type(struct device * dev)
{
	struct pci_dev * pdev;
	int i;
	
	pdev = to_pci_dev(dev);
	
	for (i = 0; i < ntrident_devids; i++) {
		if (pdev->device == trident_devid[i]) {
			return trident;
		}
	}
	return none;
};

/* show function steering based on device type */
static ssize_t tmon_show(struct device * dev,
			 struct device_attribute * dattr,
			 char * buf)
{
	if (tmon_type(dev) == trident) {
		return trident_tmon_show(dev, dattr, buf);
	} else {
		return sprintf(buf, "%d\n", TMON_MAX_VALUE);
	}
};


/*------------------------------------------------------------------------------
 *
 * Module interface
 *
 * Connect to the linux-kernel-bde driver to get access to the list of 
 * devices and then initialize the interesting ones.
 *
 */

static int __init tmon_probe(void)
{
	int i;
	int ret_val;
	struct pci_dev * pdev;
	struct tmon_dev_info * t;

	if ((linux_bde_create(NULL, &tmon_bde) < 0) || (tmon_bde == NULL)) {
		return -ENODEV;
	}

	ntmon = 0;
	for (i = 0; i < tmon_bde->num_devices(BDE_ALL_DEVICES); i++) {
		if ((tmon_bde->get_dev_type(i) & BDE_PCI_DEV_TYPE) == 0) {
			continue;
		}

		t = &tmon_info[ntmon];

		pdev    = lkbde_get_hw_dev(i);
		t->dev  = &pdev->dev;
		t->base = lkbde_get_dev_virt(i);
		t->phys = lkbde_get_dev_phys(i);

		if ((t->type = tmon_type(t->dev)) == none) {
			dev_warn(t->dev, "thermal monitoring not available "
				 "for PCI device %#x\n", pdev->device);
			continue;
		} else {
			ret_val = sysfs_create_group(&t->dev->kobj, &tmon_attr_group);
			if (ret_val) {
				dev_err(t->dev, "unable to create sysfs attrs"
					"for PCI device %#x errno %d\n",
					pdev->device, ret_val);
				continue;
			}
			
			t->hdev = hwmon_device_register(t->dev);
			if (IS_ERR(t->hdev)) {
				ret_val = PTR_ERR(t->hdev);
				sysfs_remove_group(&t->dev->kobj, &tmon_attr_group);
				dev_err(t->dev, "unable to create hwmon group "
					"for PCI device %#x errno %d\n",
					pdev->device, ret_val);
				continue;
			}
			
			dev_info(t->dev, "created sysfs groups as %s\n",
				 dev_name(t->hdev));

			t->max  = TMON_MAX_THRESH;
			t->hyst = TMON_MAX_HYST;
			if (t->type == trident) {
				trident_tmon_enable(t->base);
			}
			ntmon++;
		}
	}
	return 0;
}

static void __exit tmon_remove(void)
{
	int i;
	struct tmon_dev_info * t;

	for (i = 0; i < ntmon; i++) {
		t = &tmon_info[i];
		dev_info(t->dev, "removing %s sysfs group\n", dev_name(t->hdev));
		hwmon_device_unregister(t->hdev);
		sysfs_remove_group(&t->dev->kobj, &tmon_attr_group);
	}
}


MODULE_AUTHOR("JR Rivers <jrrivers@cumulusnetworks.com>");
MODULE_DESCRIPTION("Thermal monitoring driver for Broadcom networks SOCs");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(tmon_probe);
module_exit(tmon_remove);
