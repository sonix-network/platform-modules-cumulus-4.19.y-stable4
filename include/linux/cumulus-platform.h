/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common Cumulus platform module APIs.
 *
 * Copyright (C) 2017 Cumulus Networks, Inc.
 * Author: Curt Brune <curt@cumulusnetworks.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * https://www.gnu.org/licenses/gpl-2.0-standalone.html
 */

#ifndef CUMULUS_PLATFORM_H__
#define CUMULUS_PLATFORM_H__

#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/gpio/driver.h>
#include "platform-bitfield.h"

typedef int bf_read_func(struct device *dev, int reg, int nregs, u32 *val);
typedef int bf_write_func(struct device *dev, int reg, int nregs, u32 val);

int cumulus_i2c_find_adapter(const char *name);

struct i2c_client *
cumulus_i2c_add_client(int bus, struct i2c_board_info *info);

ssize_t cumulus_gpio_map_show(struct device *dev,
			      struct gpio_chip *chip,
			      char *buf);

ssize_t cumulus_bf_show32(struct device *dev,
			  struct device_attribute *dattr,
			  char *buf,
			  struct bf *bif,
			  bf_read_func *read);

ssize_t cumulus_bf_store32(struct device *dev,
			   struct device_attribute *dattr,
			   const char *buf,
			   size_t size,
			   struct bf *bif,
			   bf_read_func *read,
			   bf_write_func *write);

ssize_t cumulus_bf_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf,
			struct bf *bif,
			bf_read_func *read);

ssize_t cumulus_bf_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf,
			 size_t size,
			 struct bf *bif,
			 bf_read_func *read,
			 bf_write_func *write);

int cumulus_bf_i2c_read_reg(struct device *dev,
			    int reg,
			    int nregs,
			    u32 *val);

int cumulus_bf_i2c_write_reg(struct device *dev,
			     int reg,
			     int nregs,
			     u32 val);

#endif /* CUMULUS_PLATFORM_H__ */
