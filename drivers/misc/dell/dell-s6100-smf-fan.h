/*
 * Smartfusion fan tray FRU driver definitions for dell_s6100 platforms.
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

#ifndef S6100_SMF_FAN_TRAY_H
#define S6100_SMF_FAN_TRAY_H

#define S6100_SMF_FAN_DRIVER_NAME	"dell-s6100-smf-fan"

struct s6100_smf_fan_platform_data {
	struct regmap          *smf_mb_map;
};

#endif /* S6100_SMF_FAN_TRAY_H */
