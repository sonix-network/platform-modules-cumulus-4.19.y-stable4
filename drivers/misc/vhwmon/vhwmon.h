/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * vhwmon.h - Virtual hwmon driver.
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

/*
 * ioctl interface
 *
 * There's a matching python version in vhwmon.py.
 * Any changes made here must be reflected there.
 */

#define VHWMON_MAX_DEVICES	512	// number of devices
#define VHWMON_MAX_ATTRS	700	// number of attributes per device
#define VHWMON_DEVICE_NAME_LEN	64	// device name length
#define VHWMON_ATTR_NAME_LEN	32	// attribute name length
#define VHWMON_ATTR_VALUE_LEN	32	// attribute value length

#define VHWMON_VERSION		_IO(1, 0)
#define VHWMON_ALLOC		_IO(1, 1)
#define VHWMON_FREE		_IO(1, 2)
#define VHWMON_SET		_IO(1, 3)

struct vhwmon_attr {
	char name[VHWMON_ATTR_NAME_LEN];
	char value[VHWMON_ATTR_VALUE_LEN];
};

union vhwmon_ioctl {
	struct {
		char name[VHWMON_DEVICE_NAME_LEN];
		int nattrs;
		struct vhwmon_attr attrs[VHWMON_MAX_ATTRS];
	} alloc;
	struct {
		int id;
	} free;
	struct {
		int id;
		char values[VHWMON_MAX_ATTRS][VHWMON_ATTR_VALUE_LEN];
	} set;
};
