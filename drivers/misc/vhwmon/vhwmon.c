// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * vhwmon.c - Virtual hwmon driver.
 *
 * Copyright (C) 2017,2019 Cumulus Networks, Inc.  All Rights Reserved
 * Author: Ellen Wang (support@cumulusnetworks.com)
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

#include <stddef.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/hwmon.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/atomic.h>
#include <linux/uaccess.h>
#include "vhwmon.h"

/*
 * Set VHWMON_DEBUG to the desired debug level.
 * There are four debug printk macros: D0, D1, D2, and D3.
 * D[123] are controlled by VHWMON_DEBUG and print with KERN_DEBUG.
 * D0 is always on, and uses KERN_WARNING.
 */
#define VHWMON_DEBUG 0

#define D0(a, ...) \
	pr_warn("%s: " a "\n", __func__, ## __VA_ARGS__)

#if VHWMON_DEBUG < 1
#define D1(a...) ((void)0)
#else
#define D1(a, ...) \
	pr_debug("%s: " a "\n", __func__, ## __VA_ARGS__)
#endif

#if VHWMON_DEBUG < 2
#define D2(a...) ((void)0)
#else
#define D2(a, ...) \
	pr_debug("%s: " a "\n", __func__, ## __VA_ARGS__)
#endif

#if VHWMON_DEBUG < 3
#define D3(a...) ((void)0)
#else
#define D3(a, ...) \
	pr_debug("%s: " a "\n", __func__, ## __VA_ARGS__)
#endif

#define ASSERT(c) ({ \
	if (!(c)) { \
		pr_err("%s:%d: assertion \"%s\" failed", \
		       __FILE__, __LINE__, #c); \
	} \
})

/*
 * Global state
 */

static const char driver_name[] = "vhwmon";
#define DRIVER_VERSION "1.0"

struct vhwmon_info {
	struct device *hdev;
	char name[VHWMON_DEVICE_NAME_LEN];
	int id;
	int nattrs;
	struct vhwmon_attr *attrs;
	struct attribute_group *sysfs_group[2];
	struct device_attribute *sysfs_attrs;
};

static struct vhwmon_info *vhwmon_info[VHWMON_MAX_DEVICES];

DEFINE_IDA(vhwmon_ida);

static void vhwmon_info_init(void);
static int vhwmon_info_alloc(char *name, int nattrs,
			     struct vhwmon_attr *attrs);
static int vhwmon_info_free(int id);
static void vhwmon_info_freeall(void);
static int vhwmon_info_set_values(int id,
				  char values[][VHWMON_ATTR_VALUE_LEN]);
static int vhwmon_info_find(int id, struct vhwmon_info **infop);

static int vhwmon_hwmon_register(struct vhwmon_info *info);
static void vhwmon_hwmon_unregister(struct vhwmon_info *info);

static ssize_t vhwmon_show(struct device *dev,
			   struct device_attribute *attr,
			   char *buf);

/*
 * Device node
 */

static struct miscdevice vhwmon_dev;
static atomic_t vhwmon_dev_opened;
static union vhwmon_ioctl vhwmon_iocbuf;

static int
vhwmon_open(struct inode *ino, struct file *filp)
{
	if (atomic_xchg(&vhwmon_dev_opened, 1) != 0) {
		D1("device already opened");
		return -EBUSY;
	}
	D1("device opened");
	return 0;
}

static int
vhwmon_release(struct inode *ino, struct file *filp)
{
	vhwmon_info_freeall();
	atomic_set(&vhwmon_dev_opened, 0);
	D1("device closed");
	return 0;
}

static long
vhwmon_ioctl(struct file *filp, unsigned int op, unsigned long arg)
{
	union vhwmon_ioctl *ioc = &vhwmon_iocbuf;
	int retval = 0;

	D3("op %u, arg %lu (%#lx)", op, arg, arg);

	// XXX revisit locking here and at other entry points
	device_lock(vhwmon_dev.this_device);
	switch (op) {
	case VHWMON_VERSION:
		/* arg is the incoming version, which we don't use yet */
		retval = 1000;		/* version 1.0 */
		break;
	case VHWMON_ALLOC:
		if (copy_from_user(&ioc->alloc, (void *)arg,
				   sizeof(ioc->alloc)) != 0) {
			retval = -EFAULT;
			break;
		}
		retval = vhwmon_info_alloc(ioc->alloc.name, ioc->alloc.nattrs,
					   ioc->alloc.attrs);
		/* the allocated hwmon name is returned in ioc->alloc.name */
		if (retval >= 0 &&
		    copy_to_user(((union vhwmon_ioctl *)arg)->alloc.name,
				 ioc->alloc.name,
				 sizeof(ioc->alloc.name)) != 0) {
			/* this really shouldn't happen at all */
			vhwmon_info_free(retval);
			retval = -EFAULT;
		}
		break;
	case VHWMON_FREE:
		if (copy_from_user(&ioc->free, (void *)arg,
				   sizeof(ioc->free)) != 0) {
			retval = -EFAULT;
			break;
		}
		retval = vhwmon_info_free(ioc->free.id);
		break;
	case VHWMON_SET:
		if (copy_from_user(&ioc->set, (void *)arg,
				   sizeof(ioc->set)) != 0) {
			retval = -EFAULT;
			break;
		}
		retval = vhwmon_info_set_values(ioc->set.id, ioc->set.values);
		break;
	default:
		D0("unknown ioctl op %d", op);
		retval = -EINVAL;
	}

	device_unlock(vhwmon_dev.this_device);
	return retval;
}

static const struct file_operations vhwmon_fops = {
	.owner = THIS_MODULE,
	.open = vhwmon_open,
	.release = vhwmon_release,
	.unlocked_ioctl = vhwmon_ioctl,
};

/*
 * Initialization and cleanup
 */

static int __init
vhwmon_init(void)
{
	int retval;

	D1("start");
	vhwmon_info_init();

	D1("register");
	vhwmon_dev.minor = MISC_DYNAMIC_MINOR;
	vhwmon_dev.name = driver_name;
	vhwmon_dev.fops = &vhwmon_fops;
	retval = misc_register(&vhwmon_dev);
	if (retval != 0) {
		pr_err("%s: unable to register device: errno %d\n",
		       driver_name, retval);
		goto out;
	}
	dev_info(vhwmon_dev.this_device,
		 "registered device, minor number %d\n", vhwmon_dev.minor);

out:
	D1("return %d", retval);
	return retval;
}

static void __exit
vhwmon_exit(void)
{
	vhwmon_info_freeall();
	misc_deregister(&vhwmon_dev);
}

/*
 * vhwmon_info management
 */

static void
vhwmon_info_init(void)
{
	ida_init(&vhwmon_ida);
}

static int
vhwmon_info_alloc(char *name,
		  int nattrs,
		  struct vhwmon_attr *attrs)
{
	struct vhwmon_info *info = NULL;
	int i;
	int retval;

	if (nattrs < 0 || nattrs > VHWMON_MAX_ATTRS) {
		retval = -EINVAL;
		goto err;
	}

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		retval = -ENOMEM;
		goto err;
	}
	info->id = -1;
	info->attrs = kmalloc_array(nattrs, sizeof(*info->attrs), GFP_KERNEL);
	if (!info->attrs) {
		retval = -ENOMEM;
		goto err;
	}

	retval = ida_simple_get(&vhwmon_ida, 0, VHWMON_MAX_DEVICES, GFP_KERNEL);
	if (retval < 0)
		goto err;
	info->id = retval;
	/*
	 * All the strings are supposed to be null terminated,
	 * but we force them anyway for safety.
	 * Yes, that means they may get silently truncated.
	 * We'll call it a user error.
	 */
	strncpy(info->name, name, VHWMON_DEVICE_NAME_LEN);
	info->name[VHWMON_DEVICE_NAME_LEN - 1] = '\0';
	info->nattrs = nattrs;
	memcpy(info->attrs, attrs, nattrs * sizeof(*info->attrs));
	for (i = 0; i < nattrs; i++) {
		info->attrs[i].name[VHWMON_ATTR_NAME_LEN - 1] = '\0';
		info->attrs[i].value[VHWMON_ATTR_VALUE_LEN - 1] = '\0';
	}

	retval = vhwmon_hwmon_register(info);
	if (retval < 0)
		goto err;

	/*
	 * Return the hwmon name
	 */
	strncpy(name, dev_name(info->hdev), VHWMON_DEVICE_NAME_LEN);
	name[VHWMON_DEVICE_NAME_LEN - 1] = '\0';

	vhwmon_info[info->id] = info;
	return info->id;
err:
	if (info) {
		if (info->hdev)
			vhwmon_hwmon_unregister(info);
		if (info->id >= 0)
			ida_simple_remove(&vhwmon_ida, info->id);
		kfree(info->attrs);
		kfree(info);
	}
	return retval;
}

static int
vhwmon_info_free(int id)
{
	struct vhwmon_info *info;
	int retval;

	retval = vhwmon_info_find(id, &info);
	if (retval)
		goto out;
	vhwmon_info[info->id] = NULL;
	vhwmon_hwmon_unregister(info);
	ida_simple_remove(&vhwmon_ida, info->id);
	kfree(info->attrs);
	kfree(info);

out:
	return retval;
}

static void
vhwmon_info_freeall(void)
{
	int i;

	for (i = 0; i < VHWMON_MAX_DEVICES; i++)
		if (vhwmon_info[i])
			vhwmon_info_free(i);
}

static int
vhwmon_info_set_values(int id,
		       char values[][VHWMON_ATTR_VALUE_LEN])
{
	struct vhwmon_info *info;
	int i;
	int retval;

	retval = vhwmon_info_find(id, &info);
	if (retval)
		goto out;

	// avoid inconsistent sysfs access
	device_lock(info->hdev);
	for (i = 0; i < info->nattrs; i++) {
		struct vhwmon_attr *n = info->attrs + i;

		memcpy(n->value, values[i], sizeof(n->value));
		/* force null termination for safety. */
		n->value[VHWMON_ATTR_VALUE_LEN - 1] = '\0';
	}
	device_unlock(info->hdev);

out:
	return retval;
}

int
vhwmon_info_find(int id, struct vhwmon_info **infop)
{
	struct vhwmon_info *info;

	if (id < 0 || id >= VHWMON_MAX_DEVICES) {
		D0("invalid id %d", id);
		return -EINVAL;
	}
	info = vhwmon_info[id];
	if (!info) {
		D0("id %d not allocated", id);
		return -EINVAL;
	}
	*infop = info;

	return 0;
}

/*
 * hwmon interface
 */

static int
vhwmon_hwmon_register(struct vhwmon_info *info)
{
	struct device *hdev;
	int i;
	int retval;

	/*
	 * Allocate memory first.
	 */
	info->sysfs_attrs = kzalloc(sizeof(*info->sysfs_attrs) *
					   (info->nattrs + 1), GFP_KERNEL);
	if (!info->sysfs_attrs) {
		retval = -ENOMEM;
		goto bad;
	}
	info->sysfs_group[0] = kzalloc(sizeof(*info->sysfs_group[0]),
				       GFP_KERNEL);
	if (!info->sysfs_group[0]) {
		retval = -ENOMEM;
		goto bad;
	}
	info->sysfs_group[0]->attrs =
		kzalloc(sizeof(*info->sysfs_group[0]->attrs) *
			(info->nattrs + 1), GFP_KERNEL);
	if (!info->sysfs_group[0]->attrs) {
		retval = -ENOMEM;
		goto bad;
	}

	/*
	 * Create sysfs group
	 */
	for (i = 0; i < info->nattrs; i++) {
		struct device_attribute *a = info->sysfs_attrs + i;
		struct vhwmon_attr *n = info->attrs + i;

		a->attr.name = n->name;
		a->attr.mode = 0444;
		a->show = vhwmon_show;
		info->sysfs_group[0]->attrs[i] = &a->attr;
	}
	/* info->sysfs_group[0]->attrs list is NULL terminated by the kzalloc */
	info->sysfs_group[1] = NULL;

	/*
	 * Hook ourselves up to hwmon.
	 *
	 * We're hijacking the hwmon device instance to hang our state
	 * and sysfs attributes.  So we try to tread lightly.
	 * If you can't be good, be careful.
	 *
	 * The reason is we don't want to create a device instance
	 * for every sensor we publish.  Besides laziness, the device
	 * we create will have to satisfy lm-sensors.
	 * Specifically, the device's sysfs group is pointed
	 * to by the symlink /sys/class/hwmon/hwmon<n>/device.
	 * And lm-sensors uses the link to determine the type
	 * of bus we're on, which has to be among the ones it
	 * knows about.  And of those, the only one we can reasonably
	 * pretend to be is a platform ISA device.
	 *
	 * So instead, we don't supply a struct device to
	 * hwmon_device_register, and the .../hwmon<n>/device
	 * link is not created, and lm-sensors treats us as a
	 * virtual device (which we are) and is not too fussy
	 * about the details.
	 *
	 * However, that means we don't have place to put our
	 * sysfs attributes (temp1*, etc.) except in the .../hwmon<n>
	 * directory, which in turn means using the hwmon device
	 * instance to do our business.
	 *
	 * One down side of this is that the lm-sensors user-level
	 * code appends "-virtual-0" to our sensor names.
	 * (Yes, the index is always 0.  The comment in the code:
	 * "for now we assume that virtual devices are unique".)
	 * We'll live with that and limit our creativity to the
	 * part of the name we do control.
	 */

	ASSERT(!info->hdev);
	hdev = hwmon_device_register_with_groups(NULL, info->name, info,
						 (const struct attribute_group
						  **)&info->sysfs_group[0]);
	D2("%s: registered hwmon %s", info->name, dev_name(hdev));
	if (IS_ERR(hdev)) {
		retval = PTR_ERR(hdev);
		dev_err(vhwmon_dev.this_device,
			"unable to register hwmon device for %s, errno %d\n",
			info->name, retval);
		goto bad;
	}
	info->hdev = hdev;

	retval = sysfs_create_link(&vhwmon_dev.this_device->kobj,
				   &info->hdev->kobj, info->name);
	if (retval != 0) {
		dev_err(info->hdev, "unable to create sysfs link: errno %d\n",
			retval);
		goto bad;
	}
	D2("%s: created sysfs link %s", info->name, dev_name(info->hdev));

	return 0;
bad:
	kfree(info->sysfs_group[0]->attrs);
	kfree(info->sysfs_group[0]);
	info->sysfs_group[0] = NULL;
	kfree(info->sysfs_attrs);
	info->sysfs_attrs = NULL;
	if (info->hdev) {
		hwmon_device_unregister(info->hdev);
		info->hdev = NULL;
	}
	return retval;
}

static void
vhwmon_hwmon_unregister(struct vhwmon_info *info)
{
	ASSERT(info->hdev);

	D2("%s: removing sysfs link %s", info->name, dev_name(info->hdev));
	sysfs_remove_link(&vhwmon_dev.this_device->kobj, info->name);
	D2("%s: unregistering hwmon %s", info->name, dev_name(info->hdev));
	hwmon_device_unregister(info->hdev);
	info->hdev = NULL;
	kfree(info->sysfs_group[0]->attrs);
	kfree(info->sysfs_group[0]);
	info->sysfs_group[0] = NULL;
	kfree(info->sysfs_attrs);
	info->sysfs_attrs = NULL;
}

/*
 * sysfs interface
 */

static ssize_t
vhwmon_show(struct device *dev,
	    struct device_attribute *attr,
	    char *buf)
{
	struct vhwmon_info *info = dev_get_drvdata(dev);
	int n = attr - info->sysfs_attrs;

	ASSERT(info->hdev == dev);
	ASSERT(n >= 0 && n < info->nattrs);

	return scnprintf(buf, PAGE_SIZE, "%s\n", info->attrs[n].value);
}

MODULE_AUTHOR("Cumulus Networks, Inc.");
MODULE_DESCRIPTION("Virtual hwmon driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(vhwmon_init);
module_exit(vhwmon_exit);
