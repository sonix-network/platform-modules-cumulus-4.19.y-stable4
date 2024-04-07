/*
 * CPLD sysfs driver for accton_as4600_54t.
 *
 * Copyright (C) 2013 Cumulus Networks, Inc.
 * Author: Vidya Sagar Ravipati <vidya@cumulusnetworks.com>
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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <asm/io.h>

#include "accton_as4600_54t_cpld.h"
#include "platform_defs.h"

static const char driver_name[] = "accton_as4600_54t_cpld";
#define DRIVER_VERSION "1.0"

/*------------------------------------------------------------------------------
 *
 * Driver resident static variables
 *
 */

static uint8_t __iomem* accton_as4600_54t_cpld_regs;
static DEFINE_MUTEX(accton_as4600_54t_cpld_mutex);


/********************************************************************************
 *
 * CPLD I/O
 *
 */
static inline uint8_t cpld_rd(uint32_t reg)
{
	return readb(accton_as4600_54t_cpld_regs + reg);
}

static inline void cpld_wr(uint32_t reg, uint8_t data)
{
	writeb(data, accton_as4600_54t_cpld_regs + reg);
}

#define ACCTON_AS4600_54T_CPLD_STRING_NAME_SIZE 30

/*
 * board version
 */
static ssize_t board_revision_show(struct device * dev,
                   struct device_attribute * dattr,
                   char * buf)
{
	uint8_t tmp;
	uint8_t type;
	uint8_t rev;

	tmp   = cpld_rd(CPLD_REG_VERSION);
	type = (tmp & CPLD_VERSION_H_MASK) >> CPLD_VERSION_H_SHIFT;
	rev  = (tmp & CPLD_VERSION_L_MASK)  >> CPLD_VERSION_L_SHIFT;

	return sprintf(buf, "%d.%d\n", type, rev);
}
static SYSFS_ATTR_RO(board_revision, board_revision_show);
/*------------------------------------------------------------------------------
 *
 * PSU status definitions
 *
 * All the definition names use "positive" logic and return "1" for OK
 * and "0" for not OK.
 */
struct cpld_status {
	char name[ACCTON_AS4600_54T_CPLD_STRING_NAME_SIZE];
	uint8_t good_mask;  // set bits for "good behaviour"
	uint8_t bad_mask;   // set bits for "bad behaviour"
	char msg_good[ACCTON_AS4600_54T_CPLD_STRING_NAME_SIZE]; // positive string message
	char msg_bad[ACCTON_AS4600_54T_CPLD_STRING_NAME_SIZE]; // negative string message
};

static struct cpld_status cpld_psu_status[] = {
	{
		.name = "all_ok",
		.good_mask = CPLD_PSU_AC_FAIL_L | CPLD_PSU_FAN_FAIL_L | CPLD_PSU_ALERT_L,
		.bad_mask = CPLD_PSU_PRESENT_L | CPLD_PSU_12V_GOOD_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "present",
		.good_mask = 0,
		.bad_mask = CPLD_PSU_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "ac_ok",
		.good_mask = CPLD_PSU_AC_FAIL_L,
		.bad_mask = CPLD_PSU_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "dc_ok",
		.good_mask = 0,
		.bad_mask = CPLD_PSU_PRESENT_L | CPLD_PSU_12V_GOOD_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan_ok",
		.good_mask = CPLD_PSU_FAN_FAIL_L,
		.bad_mask = CPLD_PSU_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
};
static int n_psu_states = ARRAY_SIZE(cpld_psu_status);

/*
 * bulk power supply status
 */

static ssize_t psu_power_show(struct device * dev,
                  struct device_attribute * dattr,
                  char * buf)
{
	uint8_t tmp, i;
	uint8_t bad = 0;
	uint8_t name_len = strlen(xstr(PLATFORM_PS_NAME_0));
	struct cpld_status* target = NULL;

	/* find the target PSU */
	if (strncmp(dattr->attr.name, xstr(PLATFORM_PS_NAME_0), name_len) == 0) {
		tmp = cpld_rd(CPLD_REG_PSU_0_STATUS);
	} else {
		tmp = cpld_rd(CPLD_REG_PSU_1_STATUS);
	}

	for (i = 0; i < n_psu_states; i++) {
		if (strcmp(dattr->attr.name + name_len + 1, cpld_psu_status[i].name) == 0) {
			target = &cpld_psu_status[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined\n");
	}

	/*
	** All of the "good" bits must be set.
	** None of the "bad" bits can be set.
	*/
	if ((tmp & target->good_mask) == target->good_mask) {
		if (tmp & target->bad_mask) {
			bad++;
		}
	} else {
		bad++;
	}

	return sprintf(buf, "%s\n", bad ? target->msg_bad : target->msg_good);

}

static SYSFS_ATTR_RO(psu_pwr1_all_ok,  psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_present,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_ac_ok,   psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_dc_ok,   psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_fan_ok,  psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_all_ok,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_present,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_ac_ok,   psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_dc_ok,   psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_fan_ok,  psu_power_show);

/*
** System status register
*/
static struct cpld_status cpld_fan_status[] = {
	{
		.name = "fan0_present",
		.good_mask = 0,
		.bad_mask = CPLD_FAN_PRESENT_0_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan0_air_flow",
		.good_mask = CPLD_FAN_DIRECTION_0_L,
		.bad_mask = CPLD_FAN_PRESENT_0_L,
		.msg_good = "front-to-back",
		.msg_bad = "back-to-front",
	},
	{
		.name = "fan0_all_ok",
		.good_mask = 0,
		.bad_mask = CPLD_FAN_PRESENT_0_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan1_present",
		.good_mask = 0,
		.bad_mask = CPLD_FAN_PRESENT_1_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan1_air_flow",
		.good_mask = CPLD_FAN_DIRECTION_1_L,
		.bad_mask = CPLD_FAN_PRESENT_1_L,
		.msg_good = "front-to-back",
		.msg_bad = "back-to-front",
	},
	{
		.name = "fan1_all_ok",
		.good_mask = 0,
		.bad_mask = CPLD_FAN_PRESENT_1_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
};
static int n_fan_states = ARRAY_SIZE(cpld_fan_status);

/*
 * fan status
 */
static ssize_t fan_show(struct device * dev,
            struct device_attribute * dattr,
            char * buf)
{
	uint8_t tmp;
	uint8_t i;
	uint8_t air_flow = 0;
	uint8_t present_l = 0;
	uint8_t fan_direction = 0;
	uint8_t bad = 0;
	struct cpld_status* target = NULL;

	tmp = cpld_rd(CPLD_REG_FAN_STATUS);

	for (i = 0; i < n_fan_states; i++) {
		if (strcmp(dattr->attr.name, cpld_fan_status[i].name) == 0) {
			target = &cpld_fan_status[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined\n");
	}

	if (strcmp(target->name, "fan0_air_flow") == 0) {
		air_flow = 1;
		present_l = CPLD_FAN_PRESENT_0_L;
		fan_direction = CPLD_FAN_DIRECTION_0_L;
	} else if (strcmp(target->name, "fan1_air_flow") == 0) {
		air_flow = 1;
		present_l = CPLD_FAN_PRESENT_1_L;
		fan_direction = CPLD_FAN_DIRECTION_1_L;
	}
	if (air_flow) {
		if (tmp & present_l) {
			return sprintf(buf, "not present\n");
		} else {
			return sprintf(buf, "%s\n", (tmp & fan_direction) ?
				"front-to-back" : "back-to-front");
		}
	} else {
		/*
		** All of the "good" bits must be set.
		** None of the "bad" bits can be set.
		*/
		if ((tmp & target->good_mask) == target->good_mask) {
			if (tmp & target->bad_mask) {
				bad++;
			}
		} else {
			bad++;
		}

		return sprintf(buf, "%s\n", bad ? target->msg_bad : target->msg_good);
	}
}

static SYSFS_ATTR_RO(fan0_present, fan_show);
static SYSFS_ATTR_RO(fan0_air_flow, fan_show);
static SYSFS_ATTR_RO(fan0_all_ok, fan_show);
static SYSFS_ATTR_RO(fan1_present, fan_show);
static SYSFS_ATTR_RO(fan1_air_flow, fan_show);
static SYSFS_ATTR_RO(fan1_all_ok, fan_show);

/*------------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[ACCTON_AS4600_54T_CPLD_STRING_NAME_SIZE];
	uint8_t reg;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_psu1",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_0,
		.mask = CPLD_SYS_LED_PWR_0_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYS_LED_PWR_0_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_PWR_0_AMBER},
			{ PLATFORM_LED_OFF, CPLD_SYS_LED_PWR_0_OFF}
		},
	},
	{
		.name = "led_psu2",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_0,
		.mask = CPLD_SYS_LED_PWR_1_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYS_LED_PWR_1_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_PWR_1_AMBER},
			{ PLATFORM_LED_OFF, CPLD_SYS_LED_PWR_1_OFF}
		},
	},
	{
		.name = "led_poe",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_0,
		.mask = CPLD_SYS_LED_POE_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYS_LED_POE_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_POE_AMBER},
			{ PLATFORM_LED_OFF, CPLD_SYS_LED_POE_OFF}
		},
	},
	{
		.name = "led_diag",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_0,
		.mask = CPLD_SYS_LED_DIAG_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYS_LED_DIAG_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_DIAG_AMBER},
			{ PLATFORM_LED_OFF, CPLD_SYS_LED_DIAG_OFF}
		},
	},
	{
		.name = "led_fan0",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_1,
		.mask = CPLD_SYS_LED_FAN_1_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN,  CPLD_SYS_LED_FAN_1_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_FAN_1_AMBER},
			{ PLATFORM_LED_OFF,    CPLD_SYS_LED_FAN_1_OFF},
		},
	},
	{
		.name = "led_fan1",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_1,
		.mask = CPLD_SYS_LED_FAN_2_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN,  CPLD_SYS_LED_FAN_2_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_FAN_2_AMBER},
			{ PLATFORM_LED_OFF,    CPLD_SYS_LED_FAN_2_OFF},
		},
	},
	{
		.name = "led_module0",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_1,
		.mask = CPLD_SYS_LED_MODULE_0_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN,  CPLD_SYS_LED_MODULE_0_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_MODULE_0_AMBER},
			{ PLATFORM_LED_OFF,    CPLD_SYS_LED_MODULE_0_OFF},
		},
	},
	{
		.name = "led_module1",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_1,
		.mask = CPLD_SYS_LED_MODULE_1_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN,  CPLD_SYS_LED_MODULE_1_GREEN},
			{ PLATFORM_LED_YELLOW, CPLD_SYS_LED_MODULE_1_AMBER},
			{ PLATFORM_LED_OFF,    CPLD_SYS_LED_MODULE_1_OFF},
		},
	},
};
static int n_leds = ARRAY_SIZE(cpld_leds);

/*
 * LEDs
 */
static ssize_t led_show(struct device * dev,
			struct device_attribute * dattr,
			char * buf)
{
	uint8_t tmp;
	int i;
	struct led * target = NULL;

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined target\n");
	}

	/* read the register */
	tmp = cpld_rd(target->reg);

	/* find the color */
	tmp &= target->mask;
	for (i = 0; i < target->n_colors; i++) {
		if (tmp == target->colors[i].value) {
			break;
		}
	}
	if (i == target->n_colors) {
		return sprintf(buf, "undefined color\n");
	} else {
		return sprintf(buf, "%s\n", target->colors[i].name);
	}
}

static ssize_t led_store(struct device * dev,
			 struct device_attribute * dattr,
			 const char * buf, size_t count)
{
	uint8_t tmp;
	int i;
	struct led * target = NULL;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];

	/* find the target led */
	for (i = 0; i < n_leds; i++) {
		if (strcmp(dattr->attr.name, cpld_leds[i].name) == 0) {
			target = &cpld_leds[i];
			break;
		}
	}
	if (target == NULL) {
		return -EINVAL;
	}

	/* find the color */
	if (sscanf(buf, "%19s", raw) <= 0) {
		return -EINVAL;
	}
	for (i = 0; i < target->n_colors; i++) {
		if (strcmp(raw, target->colors[i].name) == 0) {
			break;
		}
	}
	if (i == target->n_colors) {
		return -EINVAL;
	}

	/* set the new value */

	mutex_lock(&accton_as4600_54t_cpld_mutex);

	tmp = cpld_rd(target->reg);
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;
	cpld_wr(target->reg, tmp);

	mutex_unlock(&accton_as4600_54t_cpld_mutex);

	return count;
}
static SYSFS_ATTR_RW(led_psu1,    led_show, led_store);
static SYSFS_ATTR_RW(led_psu2,    led_show, led_store);
static SYSFS_ATTR_RW(led_poe,     led_show, led_store);
static SYSFS_ATTR_RW(led_diag,    led_show, led_store);
static SYSFS_ATTR_RW(led_fan0,    led_show, led_store);
static SYSFS_ATTR_RW(led_fan1,    led_show, led_store);
static SYSFS_ATTR_RW(led_module0, led_show, led_store);
static SYSFS_ATTR_RW(led_module1, led_show, led_store);


/*------------------------------------------------------------------------------
 *
 * SFP+ ports 49, 50, 51, 52 sysfs functions
 *
 */

#define accton_as4600_54t_CPLD_PORT_NAME_SIZE 20
struct port_info {
	char name[accton_as4600_54t_CPLD_PORT_NAME_SIZE];
	uint32_t pres_reg;
	uint8_t  pres;
	uint32_t rx_los_reg;
	uint8_t  rx_los;
	uint32_t tx_fail_reg;
	uint8_t  tx_fail;
	uint32_t tx_dis_reg;
	uint8_t  tx_dis;
	uint32_t tx_speed_reg;
	uint8_t  tx_speed;
};

// All names in ports[] must have the same length
#define PORT_NAME_LEN (6)

static struct port_info ports[] = {
	/*
	** Ports 49-52 are SFP+ modules.
	*/
	{
		.name           = "port49",
		.pres_reg       = CPLD_REG_SFPP_PRESENT,
		.pres           = CPLD_SFPP_PRESENT_PORT_0,
		.rx_los_reg     = CPLD_REG_SFPP_RX_LOS,
		.rx_los         = CPLD_SFPP_RX_LOS_PORT_0,
		.tx_fail_reg    = CPLD_REG_SFPP_TX_FAIL,
		.tx_fail        = CPLD_SFPP_TX_FAIL_PORT_0,
		.tx_dis_reg     = CPLD_REG_SFPP_TX_DISABLE,
		.tx_dis         = CPLD_SFPP_TX_DISABLE_PORT_0,
		.tx_speed_reg   = CPLD_REG_SFPP_SPEED,
		.tx_speed       = CPLD_SFPP_SPEED_PORT_0,
	},
	{
		.name          = "port50",
		.pres_reg      = CPLD_REG_SFPP_PRESENT,
		.pres          = CPLD_SFPP_PRESENT_PORT_1,
		.rx_los_reg    = CPLD_REG_SFPP_RX_LOS,
		.rx_los        = CPLD_SFPP_RX_LOS_PORT_1,
		.tx_fail_reg   = CPLD_REG_SFPP_TX_FAIL,
		.tx_fail       = CPLD_SFPP_TX_FAIL_PORT_1,
		.tx_dis_reg    = CPLD_REG_SFPP_TX_DISABLE,
		.tx_dis        = CPLD_SFPP_TX_DISABLE_PORT_1,
		.tx_speed_reg  = CPLD_REG_SFPP_SPEED,
		.tx_speed      = CPLD_SFPP_SPEED_PORT_1,
	},
	{
		.name          = "port51",
		.pres_reg      = CPLD_REG_SFPP_PRESENT,
		.pres          = CPLD_SFPP_PRESENT_PORT_2,
		.rx_los_reg    = CPLD_REG_SFPP_RX_LOS,
		.rx_los        = CPLD_SFPP_RX_LOS_PORT_2,
		.tx_fail_reg   = CPLD_REG_SFPP_TX_FAIL,
		.tx_fail       = CPLD_SFPP_TX_FAIL_PORT_2,
		.tx_dis_reg    = CPLD_REG_SFPP_TX_DISABLE,
		.tx_dis        = CPLD_SFPP_TX_DISABLE_PORT_2,
		.tx_speed_reg  = CPLD_REG_SFPP_SPEED,
		.tx_speed      = CPLD_SFPP_SPEED_PORT_2,
	},
	{
		.name          = "port52",
		.pres_reg      = CPLD_REG_SFPP_PRESENT,
		.pres          = CPLD_SFPP_PRESENT_PORT_3,
		.rx_los_reg    = CPLD_REG_SFPP_RX_LOS,
		.rx_los        = CPLD_SFPP_RX_LOS_PORT_3,
		.tx_fail_reg   = CPLD_REG_SFPP_TX_FAIL,
		.tx_fail       = CPLD_SFPP_TX_FAIL_PORT_3,
		.tx_dis_reg    = CPLD_REG_SFPP_TX_DISABLE,
		.tx_dis        = CPLD_SFPP_TX_DISABLE_PORT_3,
		.tx_speed_reg  = CPLD_REG_SFPP_SPEED,
		.tx_speed      = CPLD_SFPP_SPEED_PORT_3,
	},
};
static int n_port = sizeof(ports) / sizeof(ports[0]);

static ssize_t port_show(struct device * dev,
			 struct device_attribute * dattr,
			 char * buf)
{
	uint8_t tmp;
	unsigned int offset;
	int i;
	struct port_info * target = NULL;
	char* str;
	int type = -1;

	/* find the target port */
	for (i = 0; i < n_port; i++) {
		if (strncmp(dattr->attr.name,
			    ports[i].name, PORT_NAME_LEN) == 0) {
			target = &ports[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined\n");
	}

	/* find the target operation */
	if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "sfp_present") == 0) {
		type = TYPE_PRESENT;
		offset = target->pres_reg;
	}
	else if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "rx_status") == 0) {
		type = TYPE_RX_LOS;
		offset = target->rx_los_reg;
	}
	else if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "tx_status") == 0) {
		type = TYPE_TX_FAIL;
		offset = target->tx_fail_reg;
	}
	else if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "tx_ctrl") == 0) {
		type = TYPE_TX_CTRL;
		offset = target->tx_dis_reg;
	}
	else if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "tx_speed") == 0) {
		type = TYPE_TX_SPEED;
		offset = target->tx_speed_reg;
	}
	else {
		return sprintf(buf, "undefined\n");
	}

	/* read the register */
	tmp = cpld_rd(offset);

	switch (type) {
	case TYPE_PRESENT:
		if (tmp & target->pres) {
			str = "0";
		}
		else {
			str = "1";
		}
		break;
	case TYPE_RX_LOS:
		if (tmp & target->rx_los) {
			str = "link_down";
		}
		else {
			str = "link_up";
		}
		break;
	case TYPE_TX_FAIL:
		if (tmp & target->tx_fail) {
			str = "link_down";
		}
		else {
			str = "link_up";
		}
		break;
	case TYPE_TX_CTRL:
		if (tmp & target->tx_dis) {
			str = "disable";
		}
		else {
			str = "enable";
		}
		break;
	case TYPE_TX_SPEED:
		if (tmp & target->tx_speed) {
			str = "10G";
		}
		else {
			str = "1G";
		}
		break;
	default:
		str = "undefined";
	};

	return sprintf(buf, "%s\n", str);
}

static ssize_t port_store(struct device * dev,
			  struct device_attribute * dattr,
			  const char * buf, size_t count)
{
	uint8_t tmp;
	unsigned int offset;
	int i;
	struct port_info* target = NULL;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];
	int type = -1;

	/* find the target port */
	for (i = 0; i < n_port; i++) {
		if (strncmp(dattr->attr.name,
			    ports[i].name, PORT_NAME_LEN) == 0) {
			target = &ports[i];
			break;
		}
	}
	if (target == NULL) {
		return -EINVAL;
	}

	/* find the target operation */
	if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "tx_ctrl") == 0) {
		type = TYPE_TX_CTRL;
		offset = target->tx_dis_reg;
	}
	else if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "tx_speed") == 0) {
		type = TYPE_TX_SPEED;
		offset = target->tx_speed_reg;
	}
	else {
		return -EINVAL;
	}

	/* find the setting */
	if (sscanf(buf, "%19s", raw) <= 0) {
		return -EINVAL;
	}

	mutex_lock(&accton_as4600_54t_cpld_mutex);
	tmp = cpld_rd( offset);

	switch (type) {
	case TYPE_TX_CTRL:
		if (strcmp(raw, "disable") == 0) {
			tmp |= target->tx_dis;
		}
		else if (strcmp(raw, "enable") == 0) {
			tmp &= ~target->tx_dis;
		}
		else {
			goto fail;
		}
		break;
	case TYPE_TX_SPEED:
		if (strcmp(raw, "10G") == 0) {
			tmp |= target->tx_speed;
		}
		else if (strcmp(raw, "1G") == 0) {
			tmp &= ~target->tx_speed;
		}
		else {
			goto fail;
		}
		break;
	default:
		goto fail;
	};

	cpld_wr( offset, tmp);
	mutex_unlock(&accton_as4600_54t_cpld_mutex);

	return count;

fail:
	mutex_unlock(&accton_as4600_54t_cpld_mutex);
	return -EINVAL;
}

/*
** Define a more programmer friendly format for the port control
** and status registers.
**
** Ports 49-52 are the SFP+ modules
*/
static ssize_t port_ctrl_read(struct device * dev,
		struct device_attribute * dattr,
		char * buf)
{
	uint8_t ge_pres, ge_rx_los, ge_tx_fail, ge_tx_dis, ge_tx_speed;
	uint32_t val, p;

	mutex_lock(&accton_as4600_54t_cpld_mutex);

	ge_pres     = cpld_rd(CPLD_REG_SFPP_PRESENT);
	ge_rx_los   = cpld_rd(CPLD_REG_SFPP_RX_LOS);
	ge_tx_fail  = cpld_rd(CPLD_REG_SFPP_TX_FAIL);
	ge_tx_dis   = cpld_rd(CPLD_REG_SFPP_TX_DISABLE);
	ge_tx_speed = cpld_rd(CPLD_REG_SFPP_SPEED);

	mutex_unlock(&accton_as4600_54t_cpld_mutex);

	val = 0x0;

	// SFP+ ports 49 -52
	for ( p = 49; p < 53; p++) {
		accton_as4600_54t_set_sfp_present( &val, p, /* inverted */
			!(ge_pres & (1 << (7 - (p - 49)))));
		accton_as4600_54t_set_rx_los( &val, p,
			ge_rx_los & (1 << (7 - (p - 49))));
		accton_as4600_54t_set_tx_fail( &val, p,
			(ge_tx_fail & (1 << (7 - (p - 49)))));
		accton_as4600_54t_set_tx_enable( &val, p, /* inverted */
			!(ge_tx_dis & (1 << (7 - (p - 49)))));
		accton_as4600_54t_set_tx_speed_10g( &val, p,
			(ge_tx_speed & (1 << (7 - (p - 49)))));
	}


    *((uint32_t*)buf) = val;
	return sizeof(val);
}

/*
** For ports 49-52 i.e. SFP+ modules only the TX disable and speed
** bits are writable.
**
*/
static ssize_t port_ctrl_write(struct device * dev,
		struct device_attribute * dattr,
		const char * buf, size_t count)
{
	uint8_t ge_tx_dis, ge_tx_speed;
	uint32_t val, p;

	val = *((uint32_t*)buf);

	mutex_lock(&accton_as4600_54t_cpld_mutex);

	ge_tx_dis = cpld_rd(CPLD_REG_SFPP_TX_DISABLE);
	ge_tx_speed = cpld_rd(CPLD_REG_SFPP_SPEED);

	// 10G ports 49 - 52
	for ( p = 49; p < 53; p++) {
		ge_tx_dis &= ~(1 << (7 - (p - 49)));
        /* inverted */
		ge_tx_dis |= !accton_as4600_54t_get_tx_enable( val, p) << (7 - (p - 49));
		ge_tx_speed &= ~(1 << (7 - (p - 49)));
		ge_tx_speed |= accton_as4600_54t_get_tx_speed_10g( val, p) << (7 - (p - 49));
	}

	// 10G ports 49 - 52
	cpld_wr(CPLD_REG_SFPP_TX_DISABLE, ge_tx_dis);
	cpld_wr(CPLD_REG_SFPP_SPEED, ge_tx_speed);

	mutex_unlock(&accton_as4600_54t_cpld_mutex);

	return count;
}

static SYSFS_ATTR_RO(port49_sfp_present, port_show);
static SYSFS_ATTR_RO(port50_sfp_present, port_show);
static SYSFS_ATTR_RO(port51_sfp_present, port_show);
static SYSFS_ATTR_RO(port52_sfp_present, port_show);
static SYSFS_ATTR_RO(port49_rx_status, port_show);
static SYSFS_ATTR_RO(port50_rx_status, port_show);
static SYSFS_ATTR_RO(port51_rx_status, port_show);
static SYSFS_ATTR_RO(port52_rx_status, port_show);
static SYSFS_ATTR_RO(port49_tx_status, port_show);
static SYSFS_ATTR_RO(port50_tx_status, port_show);
static SYSFS_ATTR_RO(port51_tx_status, port_show);
static SYSFS_ATTR_RO(port52_tx_status, port_show);
static SYSFS_ATTR_RW(port49_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port50_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port51_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port52_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port49_tx_speed, port_show, port_store);
static SYSFS_ATTR_RW(port50_tx_speed, port_show, port_store);
static SYSFS_ATTR_RW(port51_tx_speed, port_show, port_store);
static SYSFS_ATTR_RW(port52_tx_speed, port_show, port_store);
static SYSFS_ATTR_RW(port_raw_ctrl, port_ctrl_read, port_ctrl_write);
/*
 * Help / README
 *
 * The string length must be less than 4K.
 *
 */
#define HELP_STR "Description of the CPLD driver files:\n\
\n\
board_revision\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register,in the following format:\n\
\n\
  cpld_version\n\
\n\
  Example: 0.2\n\
\n\
led_diag\n\
\n\
  Read-Write:\n\
\n\
  Diag LED color.\n\
  The following values are possible: green, amber, off\n\
\n\
led_fan0\n\
led_fan1\n\
\n\
  Read-Write:\n\
\n\
  System FAN LED color.\n\
  The following values are possible: green, amber, off\n\
\n\
led_psu1\n\
led_psu2\n\
\n\
  Read-Write:\n\
\n\
  PSU LED colors.\n\
  The following values are possible: green, amber, off\n\
\n\
led_module0\n\
led_module1\n\
\n\
  Read-Write:\n\
\n\
  Module LED colors.\n\
  The following values are possible: green, amber, off\n\
\n\
psu_pwr1_all_ok\n\
psu_pwr1_ac_ok\n\
psu_pwr1_dc_ok\n\
psu_pwr1_fan_ok\n\
psu_pwr1_present\n\
psu_pwr2_all_ok\n\
psu_pwr2_ac_ok\n\
psu_pwr2_dc_ok\n\
psu_pwr2_fan_ok\n\
psu_pwr2_present\n\
\n\
  Read-Only:\n\
\n\
  Hot swap power supply status info.\n\
  The following values are possible: 1, 0\n\
\n\
  1 means OK.  0 means not OK.\n\
\n\
  The psu_pwr[12]_all_ok values are calculated from the other\n\
  psu_pwr[12]_* values as follows:\n\
\n\
    psu_pwr1_all_ok =\n\
      psu_pwr1_present && psu_pwr1_ac_ok && psu_pwr1_dc_ok &&\n\
      psu_pwr1_fan_ok;\n\
\n\
\n\
fan0_all_ok\n\
fan0_air_flow\n\
fan0_present\n\
fan1_all_ok\n\
fan1_air_flow\n\
fan1_present\n\
\n\
  Read-Only:\n\
\n\
  System fan status.\n\
  The following values are possible: 1, 0\n\
\n\
  1 means OK.  0 means not OK.\n\
\n\
  The fan[01]_all_ok value is calculated from the fan_present\n\
  values as follows:\n\
\n\
    fan0_all_ok = fan0_present;\n\
\n\
fan[0]_air_flow\n\
\n\
  Read-Only:\n\
\n\
  System fan air flow direction.\n\
  The following values are possible: front-to-back, back-to-front\n\
\n\
\n\
port49_sfp_present\n\
port50_sfp_present\n\
port51_sfp_present\n\
port52_sfp_present\n\
\n\
  Read-Only:\n\
\n\
  SFP+ Present status.\n\
  The following values are possible: 1, 0\n\
\n\
  1 means PRESENT. 0 means ABSENT.\n\
\n\
\n\
port49_rx_status\n\
port50_rx_status\n\
port51_rx_status\n\
port52_rx_status\n\
\n\
  Read-Only:\n\
\n\
  SFP+ Reciever Loss Status.\n\
  The following values are possible: link_up, link_down\n\
\n\
  link_up means SFP+ Receiver is working well.\n\
  link_down means SFP+ Receive Loss.\n\
\n\
\n\
port49_tx_status\n\
port50_tx_status\n\
port51_tx_status\n\
port52_tx_status\n\
\n\
  Read-Only:\n\
\n\
  SFP+ Transmit Fault Status.\n\
  The following values are possible: link_up, link_down\n\
\n\
  link_up means SFP+ Transmitter is working well.\n\
  link_down means SFP+ Transmit Fault.\n\
\n\
\n\
port49_tx_ctrl\n\
port50_tx_ctrl\n\
port51_tx_ctrl\n\
port52_tx_ctrl\n\
\n\
  Read-Write:\n\
\n\
  SFP+ Transmit Disable Status.\n\
  The following values are possible: enable, disable\n\
\n\
port49_tx_speed\n\
port50_tx_speed\n\
port51_tx_speed\n\
port52_tx_speed\n\
\n\
  Read-Write:\n\
\n\
  SFP+ Transmit Speed Status.\n\
  The following values are possible: 1G, 10G\n\
\n\
help\n\
README\n\
\n\
  Read-Only:\n\
\n\
  The text you are reading now.\n\
\n\
"

static ssize_t help_show(struct device * dev,
			 struct device_attribute * dattr,
			 char * buf)
{
	return sprintf(buf, HELP_STR);
}

static SYSFS_ATTR_RO(help, help_show);
static SYSFS_ATTR_RO(README, help_show);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */
static struct attribute *accton_as4600_54t_cpld_attrs[] = {
	&dev_attr_board_revision.attr,
	&dev_attr_psu_pwr1_all_ok.attr,
	&dev_attr_psu_pwr1_present.attr,
	&dev_attr_psu_pwr1_ac_ok.attr,
	&dev_attr_psu_pwr1_dc_ok.attr,
	&dev_attr_psu_pwr1_fan_ok.attr,
	&dev_attr_psu_pwr2_all_ok.attr,
	&dev_attr_psu_pwr2_present.attr,
	&dev_attr_psu_pwr2_ac_ok.attr,
	&dev_attr_psu_pwr2_dc_ok.attr,
	&dev_attr_psu_pwr2_fan_ok.attr,
	&dev_attr_fan0_present.attr,
	&dev_attr_fan0_air_flow.attr,
	&dev_attr_fan0_all_ok.attr,
	&dev_attr_fan1_present.attr,
	&dev_attr_fan1_air_flow.attr,
	&dev_attr_fan1_all_ok.attr,
	&dev_attr_led_psu1.attr,
	&dev_attr_led_psu2.attr,
	&dev_attr_led_poe.attr,
	&dev_attr_led_diag.attr,
	&dev_attr_led_fan0.attr,
	&dev_attr_led_fan1.attr,
	&dev_attr_led_module0.attr,
	&dev_attr_led_module1.attr,
	&dev_attr_port49_sfp_present.attr,
	&dev_attr_port50_sfp_present.attr,
	&dev_attr_port51_sfp_present.attr,
	&dev_attr_port52_sfp_present.attr,
	&dev_attr_port49_rx_status.attr,
	&dev_attr_port50_rx_status.attr,
	&dev_attr_port51_rx_status.attr,
	&dev_attr_port52_rx_status.attr,
	&dev_attr_port49_tx_status.attr,
	&dev_attr_port50_tx_status.attr,
	&dev_attr_port51_tx_status.attr,
	&dev_attr_port52_tx_status.attr,
	&dev_attr_port49_tx_ctrl.attr,
	&dev_attr_port50_tx_ctrl.attr,
	&dev_attr_port51_tx_ctrl.attr,
	&dev_attr_port52_tx_ctrl.attr,
	&dev_attr_port49_tx_speed.attr,
	&dev_attr_port50_tx_speed.attr,
	&dev_attr_port51_tx_speed.attr,
	&dev_attr_port52_tx_speed.attr,
	&dev_attr_port_raw_ctrl.attr,
	&dev_attr_help.attr,
	&dev_attr_README.attr,
	NULL,
};

static struct attribute_group accton_as4600_54t_cpld_attr_group = {
	.attrs = accton_as4600_54t_cpld_attrs,
};


/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static int accton_as4600_54t_cpld_setup(void)
{
	// Put some interesting, one-time initializations here.
	return 0;
}

static int accton_as4600_54t_cpld_probe(struct platform_device * ofdev)
{
	int retval = 0;
	struct device_node * np = ofdev->dev.of_node;
	struct kobject * kobj = &ofdev->dev.kobj;

	if (dev_get_drvdata(&ofdev->dev)) {
		dev_info(&ofdev->dev, "already probed\n");
		return 0;
	}

	accton_as4600_54t_cpld_regs = of_iomap(np,0);
	if (!accton_as4600_54t_cpld_regs) {
		return -EIO;
	}

	retval = sysfs_create_group(kobj, &accton_as4600_54t_cpld_attr_group);
	if (retval) {
		return retval;
	}

	if (accton_as4600_54t_cpld_setup()) {
	     return -EIO;
	}

	dev_info(&ofdev->dev, "probed & iomapped @ 0x%p\n", accton_as4600_54t_cpld_setup);

	return 0;
}

static int accton_as4600_54t_cpld_remove(struct platform_device * ofdev)
{
	struct kobject * kobj = &ofdev->dev.kobj;

	/* don't iounmap(regs)... the platform driver uses it for reset	*/
	sysfs_remove_group(kobj, &accton_as4600_54t_cpld_attr_group);

	dev_info(&ofdev->dev, "removed\n");
	return 0;
}

static struct of_device_id accton_as4600_54t_cpld_ids[] = {
	{
		.compatible = "accton,as4600_54t-cpld",
	},
	{ /* end of list */ },
};

static struct platform_driver accton_as4600_54t_cpld_driver = {
	.probe = accton_as4600_54t_cpld_probe,
	.remove = accton_as4600_54t_cpld_remove,
	.driver = {
		.name  = driver_name,
		.owner = THIS_MODULE,
		.of_match_table = accton_as4600_54t_cpld_ids,
	},
};


/*------------------------------------------------------------------------------
 *
 * module interface
 *
 */

/* move the action of this to probe and remove and change these to
   of_register_platform_driver/register_platform_driver
   and
   of_unregister_platform_driver/unregister_platform_driver
*/

static int __init accton_as4600_54t_cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&accton_as4600_54t_cpld_driver);
	if (rv) {
		printk(KERN_ERR
		       "%s platform_driver_register failed (%i)\n",
		       driver_name, rv);
	}
	return rv;
}

static void __exit accton_as4600_54t_cpld_exit(void)
{
	return platform_driver_unregister(&accton_as4600_54t_cpld_driver);
}

MODULE_AUTHOR("Vidya Sagar Ravipati <vidya@cumulusnetworks.com>");
MODULE_DESCRIPTION("CPLD driver for Accton Technology Corporation, AS4600_54T");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(accton_as4600_54t_cpld_init);
module_exit(accton_as4600_54t_cpld_exit);
