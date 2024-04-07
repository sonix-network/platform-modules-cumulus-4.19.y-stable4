/*
 * CPLD sysfs driver for cel_kennisis.
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

#include "cel_kennisis_cpld.h"
#include "platform_defs.h"

static const char driver_name[] = "cel_kennisis_cpld";
#define DRIVER_VERSION "1.0"

/*------------------------------------------------------------------------------
 *
 * Driver resident static variables
 *
 */

static uint8_t __iomem* cel_kennisis_cpld_regs;
static DEFINE_MUTEX(cel_kennisis_cpld_mutex);


/********************************************************************************
 *
 * CPLD I/O
 *
 */
static inline uint8_t cpld_rd(uint32_t reg)
{
	return readb(cel_kennisis_cpld_regs + reg);
}

static inline void cpld_wr(uint32_t reg, uint8_t data)
{
	writeb(data, cel_kennisis_cpld_regs + reg);
}

#define CEL_KENNISIS_CPLD_STRING_NAME_SIZE 30

/*
 * board version
 */
static ssize_t board_revision_show(struct device * dev,
                   struct device_attribute * dattr,
                   char * buf)
{
	uint8_t rev;

	rev   = cpld_rd(CPLD_BOARD_VERSION_OFFSET);

	return sprintf(buf, "%d\n", rev);
}
static SYSFS_ATTR_RO(board_revision, board_revision_show);

/*
 * cpld version
 */
static ssize_t cpld_version_show(struct device * dev,
                   struct device_attribute * dattr,
                   char * buf)
{
	uint8_t tmp;
	uint8_t type;
	uint8_t rev;

	tmp   = cpld_rd(CPLD_REG_VERSION_OFFSET);
	type = (tmp & CPLD_VERSION_H_MASK) >> CPLD_VERSION_H_SHIFT;
	rev  = (tmp & CPLD_VERSION_L_MASK)  >> CPLD_VERSION_L_SHIFT;

	return sprintf(buf, "%d.%d\n", type, rev);
}
static SYSFS_ATTR_RO(cpld_version, cpld_version_show);
/*------------------------------------------------------------------------------
 *
 * PSU status definitions
 *
 * All the definition names use "positive" logic and return "1" for OK
 * and "0" for not OK.
 */
struct cpld_status {
	char name[CEL_KENNISIS_CPLD_STRING_NAME_SIZE];
	uint8_t good_mask;  // set bits for "good behaviour"
	uint8_t bad_mask;   // set bits for "bad behaviour"
	char msg_good[CEL_KENNISIS_CPLD_STRING_NAME_SIZE]; // positive string message
	char msg_bad[CEL_KENNISIS_CPLD_STRING_NAME_SIZE]; // negative string message
};

static struct cpld_status cpld_psu_status[] = {
	{
		.name = "psu_pwr1_all_ok",
		.good_mask = CPLD_PSU_1_OK,
		.bad_mask = CPLD_PSU_1_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "psu_pwr1_present",
		.good_mask = 0,
		.bad_mask = CPLD_PSU_1_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "psu_pwr1_ac_ok",
		.good_mask = CPLD_PSU_1_OK,
		.bad_mask = 0,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "psu_pwr2_all_ok",
		.good_mask = CPLD_PSU_2_OK,
		.bad_mask = CPLD_PSU_2_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "psu_pwr2_present",
		.good_mask = 0,
		.bad_mask = CPLD_PSU_2_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "psu_pwr2_ac_ok",
		.good_mask = CPLD_PSU_2_OK,
		.bad_mask = 0,
		.msg_good = "1",
		.msg_bad = "0",
	},
};
static int n_psu_states = ARRAY_SIZE(cpld_psu_status);

/*
 * power supply status
 */

static ssize_t psu_power_show(struct device * dev,
                  struct device_attribute * dattr,
                  char * buf)
{
	uint8_t tmp, i;
	uint8_t bad = 0;
	struct cpld_status* target = NULL;


	tmp = cpld_rd(CPLD_REG_PSU_STATUS_2_OFFSET);

	for (i = 0; i < n_psu_states; i++) {
		if (strcmp(dattr->attr.name, cpld_psu_status[i].name) == 0) {
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

static SYSFS_ATTR_RO(psu_pwr1_all_ok, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_present, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_ac_ok, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_all_ok, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_present, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_ac_ok, psu_power_show);

/*
** System status register
*/
static struct cpld_status cpld_fan_status[] = {
	{
		.name = "fan1_present",
		.good_mask = 0,
		.bad_mask = CPLD_FAN_1_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan1_all_ok",
		.good_mask = 0,
		.bad_mask = CPLD_FAN_1_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan2_present",
		.good_mask = 0,
		.bad_mask = CPLD_FAN_2_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan2_all_ok",
		.good_mask = 0,
		.bad_mask = CPLD_FAN_2_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan3_present",
		.good_mask = 0,
		.bad_mask = CPLD_FAN_3_PRESENT_L,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan3_all_ok",
		.good_mask = 0,
		.bad_mask = CPLD_FAN_3_PRESENT_L,
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
	uint8_t bad = 0;
	struct cpld_status* target = NULL;

	tmp = cpld_rd(CPLD_REG_FAN_STATUS_OFFSET);

	for (i = 0; i < n_fan_states; i++) {
		if (strcmp(dattr->attr.name, cpld_fan_status[i].name) == 0) {
			target = &cpld_fan_status[i];
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

static SYSFS_ATTR_RO(fan1_present, fan_show);
static SYSFS_ATTR_RO(fan1_all_ok, fan_show);
static SYSFS_ATTR_RO(fan2_present, fan_show);
static SYSFS_ATTR_RO(fan2_all_ok, fan_show);
static SYSFS_ATTR_RO(fan3_present, fan_show);
static SYSFS_ATTR_RO(fan3_all_ok, fan_show);

/*------------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[CEL_KENNISIS_CPLD_STRING_NAME_SIZE];
	uint8_t reg;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_status",
		.reg  = CPLD_REG_SYS_LED_CTRL_OFFSET,
		.mask = CPLD_SYS_LED_MASK,
		.n_colors = 7,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYS_LED_GREEN},
			{ PLATFORM_LED_GREEN_SLOW_BLINKING, CPLD_SYS_LED_GREEN_SLOW_BLINK},
			{ PLATFORM_LED_GREEN_BLINKING, CPLD_SYS_LED_GREEN_FAST_BLINK},
			{ PLATFORM_LED_RED, CPLD_SYS_LED_RED},
			{ PLATFORM_LED_RED_SLOW_BLINKING, CPLD_SYS_LED_RED_SLOW_BLINK},
			{ PLATFORM_LED_RED_BLINKING, CPLD_SYS_LED_RED_FAST_BLINK},
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

	mutex_lock(&cel_kennisis_cpld_mutex);

	tmp = cpld_rd(target->reg);
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;
	cpld_wr(target->reg, tmp);

	mutex_unlock(&cel_kennisis_cpld_mutex);

	return count;
}
static SYSFS_ATTR_RW(led_status,    led_show, led_store);


/*------------------------------------------------------------------------------
 *
 * SFP+ ports 49, 50, 51, 52 sysfs functions
 *
 */

#define CEL_KENNISIS_CPLD_PORT_NAME_SIZE 20
struct port_info {
	char name[CEL_KENNISIS_CPLD_PORT_NAME_SIZE];
	uint32_t tx_enable_reg;
	uint8_t  tx_enable;
	uint32_t cmu_lock_reg;
	uint8_t  cmu_lock;
	uint32_t cdr_lock_reg;
	uint8_t  cdr_lock;
	uint32_t rx_los_reg;
	uint8_t  rx_los;
	uint32_t lnk_alarm_reg;
	uint8_t  lnk_alarm;
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
		.tx_enable_reg  = CPLD_REG_SFP_DRVR_CTRL_OFFSET,
		.tx_enable      = CPLD_SFP_TXONOFF_1,
		.cmu_lock_reg   = CPLD_REG_SFP_STATUS_1_OFFSET,
		.cmu_lock       = CPLD_SFP_PCMULK_1,
		.cdr_lock_reg   = CPLD_REG_SFP_STATUS_1_OFFSET,
		.cdr_lock       = CPLD_SFP_PCDRLK_1,
		.rx_los_reg     = CPLD_REG_SFP_STATUS_2_OFFSET,
		.rx_los         = CPLD_SFP_PLOSB_1_L,
		.lnk_alarm_reg  = CPLD_REG_SFP_STATUS_2_OFFSET,
		.lnk_alarm      = CPLD_SFP_LASI_1_L,
		.tx_speed_reg   = CPLD_REG_SFP_RS_CTRL_OFFSET,
		.tx_speed       = CPLD_SFP_PLUS_RS1_0 | CPLD_SFP_PLUS_RS1_1,
	},
	{
		.name           = "port50",
		.tx_enable_reg  = CPLD_REG_SFP_DRVR_CTRL_OFFSET,
		.tx_enable      = CPLD_SFP_TXONOFF_2,
		.cmu_lock_reg   = CPLD_REG_SFP_STATUS_1_OFFSET,
		.cmu_lock       = CPLD_SFP_PCMULK_2,
		.cdr_lock_reg   = CPLD_REG_SFP_STATUS_1_OFFSET,
		.cdr_lock       = CPLD_SFP_PCDRLK_2,
		.rx_los_reg     = CPLD_REG_SFP_STATUS_2_OFFSET,
		.rx_los         = CPLD_SFP_PLOSB_2_L,
		.lnk_alarm_reg  = CPLD_REG_SFP_STATUS_2_OFFSET,
		.lnk_alarm      = CPLD_SFP_LASI_2_L,
		.tx_speed_reg   = CPLD_REG_SFP_RS_CTRL_OFFSET,
		.tx_speed       = CPLD_SFP_PLUS_RS2_0 | CPLD_SFP_PLUS_RS2_1,
	},
	{
		.name           = "port51",
		.tx_enable_reg  = CPLD_REG_SFP_DRVR_CTRL_OFFSET,
		.tx_enable      = CPLD_SFP_TXONOFF_3,
		.cmu_lock_reg   = CPLD_REG_SFP_STATUS_1_OFFSET,
		.cmu_lock       = CPLD_SFP_PCMULK_3,
		.cdr_lock_reg   = CPLD_REG_SFP_STATUS_1_OFFSET,
		.cdr_lock       = CPLD_SFP_PCDRLK_3,
		.rx_los_reg     = CPLD_REG_SFP_STATUS_2_OFFSET,
		.rx_los         = CPLD_SFP_PLOSB_3_L,
		.lnk_alarm_reg  = CPLD_REG_SFP_STATUS_2_OFFSET,
		.lnk_alarm      = CPLD_SFP_LASI_3_L,
		.tx_speed_reg   = CPLD_REG_SFP_RS_CTRL_OFFSET,
		.tx_speed       = CPLD_SFP_PLUS_RS3_0 | CPLD_SFP_PLUS_RS3_1,
	},
	{
		.name           = "port52",
		.tx_enable_reg  = CPLD_REG_SFP_DRVR_CTRL_OFFSET,
		.tx_enable      = CPLD_SFP_TXONOFF_4,
		.cmu_lock_reg   = CPLD_REG_SFP_STATUS_1_OFFSET,
		.cmu_lock       = CPLD_SFP_PCMULK_4,
		.cdr_lock_reg   = CPLD_REG_SFP_STATUS_1_OFFSET,
		.cdr_lock       = CPLD_SFP_PCDRLK_4,
		.rx_los_reg     = CPLD_REG_SFP_STATUS_2_OFFSET,
		.rx_los         = CPLD_SFP_PLOSB_4_L,
		.lnk_alarm_reg  = CPLD_REG_SFP_STATUS_2_OFFSET,
		.lnk_alarm      = CPLD_SFP_LASI_4_L,
		.tx_speed_reg   = CPLD_REG_SFP_RS_CTRL_OFFSET,
		.tx_speed       = CPLD_SFP_PLUS_RS4_0 | CPLD_SFP_PLUS_RS4_1,
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
	if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "tx_ctrl") == 0) {
		type = TYPE_TX_CTRL;
		offset = target->tx_enable_reg;
	}
	else if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "cmu_lock_detect") == 0) {
		type = TYPE_CMU_LOCK_DETECT;
		offset = target->cmu_lock_reg;
	}
	else if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "cdr_lock_detect") == 0) {
		type = TYPE_CDR_LOCK_DETECT;
		offset = target->cdr_lock_reg;
	}
	else if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "rx_status") == 0) {
		type = TYPE_RX_LOS;
		offset = target->rx_los_reg;
	}
	else if (strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "link_alarm_status_int") == 0) {
		type = TYPE_LNK_ALARM_INT;
		offset = target->lnk_alarm_reg;
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
	case TYPE_TX_CTRL:
		if (tmp & target->tx_enable) {
			str = "enable";
		}
		else {
			str = "disable";
		}
		break;
	case TYPE_CMU_LOCK_DETECT:
		if (tmp & target->cmu_lock) {
			str = "1";
		}
		else {
			str = "0";
		}
		break;
	case TYPE_CDR_LOCK_DETECT:
		if (tmp & target->cdr_lock) {
			str = "1";
		}
		else {
			str = "0";
		}
		break;
	case TYPE_RX_LOS:
		if (tmp & target->rx_los) {
			str = "link_up";
		}
		else {
			str = "link_down";
		}
		break;
	case TYPE_LNK_ALARM_INT:
		if (tmp & target->lnk_alarm) {
			str = "link_up";
		}
		else {
			str = "link_down";
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
		offset = target->tx_enable_reg;
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

	mutex_lock(&cel_kennisis_cpld_mutex);
	tmp = cpld_rd( offset);

	switch (type) {
	case TYPE_TX_CTRL:
		if (strcmp(raw, "enable") == 0) {
			tmp |= target->tx_enable;
		}
		else if (strcmp(raw, "disable") == 0) {
			tmp &= ~target->tx_enable;
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
	mutex_unlock(&cel_kennisis_cpld_mutex);

	return count;

fail:
	mutex_unlock(&cel_kennisis_cpld_mutex);
	return -EINVAL;
}

static SYSFS_ATTR_RO(port49_rx_status, port_show);
static SYSFS_ATTR_RO(port50_rx_status, port_show);
static SYSFS_ATTR_RO(port51_rx_status, port_show);
static SYSFS_ATTR_RO(port52_rx_status, port_show);
static SYSFS_ATTR_RO(port49_cmu_lock_detect, port_show);
static SYSFS_ATTR_RO(port50_cmu_lock_detect, port_show);
static SYSFS_ATTR_RO(port51_cmu_lock_detect, port_show);
static SYSFS_ATTR_RO(port52_cmu_lock_detect, port_show);
static SYSFS_ATTR_RO(port49_cdr_lock_detect, port_show);
static SYSFS_ATTR_RO(port50_cdr_lock_detect, port_show);
static SYSFS_ATTR_RO(port51_cdr_lock_detect, port_show);
static SYSFS_ATTR_RO(port52_cdr_lock_detect, port_show);
static SYSFS_ATTR_RO(port49_link_alarm_status_int, port_show);
static SYSFS_ATTR_RO(port50_link_alarm_status_int, port_show);
static SYSFS_ATTR_RO(port51_link_alarm_status_int, port_show);
static SYSFS_ATTR_RO(port52_link_alarm_status_int, port_show);
static SYSFS_ATTR_RW(port49_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port50_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port51_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port52_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port49_tx_speed, port_show, port_store);
static SYSFS_ATTR_RW(port50_tx_speed, port_show, port_store);
static SYSFS_ATTR_RW(port51_tx_speed, port_show, port_store);
static SYSFS_ATTR_RW(port52_tx_speed, port_show, port_store);

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
  Board version register,in the following format:\n\
\n\
  Example: 1\n\
\n\
\n\
cpld_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register,in the following format:\n\
\n\
  cpld_version\n\
\n\
  Example: 1.6\n\
\n\
led_status\n\
\n\
  Read-Write:\n\
\n\
  System State LED color.\n\
  The following values are possible: green, green blinking,\n\
  green slow blinking, red, red blinking and red slow blinking\n\
\n\
\n\
psu_pwr1_all_ok\n\
psu_pwr1_ac_ok\n\
psu_pwr1_present\n\
psu_pwr2_all_ok\n\
psu_pwr2_ac_ok\n\
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
      psu_pwr1_present && psu_pwr1_ac_ok\n\
\n\
\n\
fan1_all_ok\n\
fan1_present\n\
fan2_all_ok\n\
fan2_present\n\
fan3_all_ok\n\
fan3_present\n\
\n\
  Read-Only:\n\
\n\
  System fan status.\n\
  The following values are possible: 1, 0\n\
\n\
  1 means OK.  0 means not OK.\n\
\n\
  The fan[123]_all_ok value is calculated from the fan_present\n\
  values as follows:\n\
\n\
    fan0_all_ok = fan0_present;\n\
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
port49_cmu_lock_detect\n\
port50_cmu_lock_detect\n\
port51_cmu_lock_detect\n\
port52_cmu_lock_detect\n\
\n\
  Read-Only:\n\
\n\
  SFP+ CMU Lock Detect.\n\
  The following values are possible: 1, 0\n\
\n\
\n\
port49_cdr_lock_detect\n\
port50_cdr_lock_detect\n\
port51_cdr_lock_detect\n\
port52_cdr_lock_detect\n\
\n\
  Read-Only:\n\
\n\
  SFP+ CDR Lock Detect.\n\
  The following values are possible: 1, 0\n\
\n\
\n\
port49_link_alarm_status_int\n\
port50_link_alarm_status_int\n\
port51_link_alarm_status_int\n\
port52_link_alarm_status_int\n\
\n\
  Read-Only:\n\
\n\
  SFP+ Link Fault Status.\n\
  The following values are possible: link_up, link_down\n\
\n\
  link_up means SFP+ Link is working well.\n\
  link_down means SFP+ Link Fault.\n\
\n\
\n\
port49_tx_ctrl\n\
port49_tx_ctrl\n\
port50_tx_ctrl\n\
port51_tx_ctrl\n\
port52_tx_ctrl\n\
\n\
  Read-Write:\n\
\n\
  SFP+ Transmit Enable Status.\n\
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
static struct attribute *cel_kennisis_cpld_attrs[] = {
	&dev_attr_board_revision.attr,
	&dev_attr_cpld_version.attr,
	&dev_attr_psu_pwr1_all_ok.attr,
	&dev_attr_psu_pwr1_present.attr,
	&dev_attr_psu_pwr1_ac_ok.attr,
	&dev_attr_psu_pwr2_all_ok.attr,
	&dev_attr_psu_pwr2_present.attr,
	&dev_attr_psu_pwr2_ac_ok.attr,
	&dev_attr_fan1_present.attr,
	&dev_attr_fan1_all_ok.attr,
	&dev_attr_fan2_present.attr,
	&dev_attr_fan2_all_ok.attr,
	&dev_attr_fan3_present.attr,
	&dev_attr_fan3_all_ok.attr,
	&dev_attr_led_status.attr,
	&dev_attr_port49_rx_status.attr,
	&dev_attr_port50_rx_status.attr,
	&dev_attr_port51_rx_status.attr,
	&dev_attr_port52_rx_status.attr,
	&dev_attr_port49_cmu_lock_detect.attr,
	&dev_attr_port50_cmu_lock_detect.attr,
	&dev_attr_port51_cmu_lock_detect.attr,
	&dev_attr_port52_cmu_lock_detect.attr,
	&dev_attr_port49_cdr_lock_detect.attr,
	&dev_attr_port50_cdr_lock_detect.attr,
	&dev_attr_port51_cdr_lock_detect.attr,
	&dev_attr_port52_cdr_lock_detect.attr,
	&dev_attr_port49_link_alarm_status_int.attr,
	&dev_attr_port50_link_alarm_status_int.attr,
	&dev_attr_port51_link_alarm_status_int.attr,
	&dev_attr_port52_link_alarm_status_int.attr,
	&dev_attr_port49_tx_ctrl.attr,
	&dev_attr_port50_tx_ctrl.attr,
	&dev_attr_port51_tx_ctrl.attr,
	&dev_attr_port52_tx_ctrl.attr,
	&dev_attr_port49_tx_speed.attr,
	&dev_attr_port50_tx_speed.attr,
	&dev_attr_port51_tx_speed.attr,
	&dev_attr_port52_tx_speed.attr,
	&dev_attr_help.attr,
	&dev_attr_README.attr,
	NULL,
};

static struct attribute_group cel_kennisis_cpld_attr_group = {
	.attrs = cel_kennisis_cpld_attrs,
};


/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static int cel_kennisis_cpld_setup(void)
{
	// Put some interesting, one-time initializations here.
	return 0;
}

static int cel_kennisis_cpld_probe(struct platform_device * ofdev)
{
	int retval = 0;
	struct device_node * np = ofdev->dev.of_node;
	struct kobject * kobj = &ofdev->dev.kobj;

	if (dev_get_drvdata(&ofdev->dev)) {
		dev_info(&ofdev->dev, "already probed\n");
		return 0;
	}

	cel_kennisis_cpld_regs = of_iomap(np,0);
	if (!cel_kennisis_cpld_regs) {
		return -EIO;
	}

	retval = sysfs_create_group(kobj, &cel_kennisis_cpld_attr_group);
	if (retval) {
		return retval;
	}

	if (cel_kennisis_cpld_setup()) {
	     return -EIO;
	}

	dev_info(&ofdev->dev, "probed & iomapped @ 0x%p\n", cel_kennisis_cpld_setup);

	return 0;
}

static int cel_kennisis_cpld_remove(struct platform_device * ofdev)
{
	struct kobject * kobj = &ofdev->dev.kobj;

	/* don't iounmap(regs)... the platform driver uses it for reset	*/
	sysfs_remove_group(kobj, &cel_kennisis_cpld_attr_group);

	dev_info(&ofdev->dev, "removed\n");
	return 0;
}

static struct of_device_id cel_kennisis_cpld_ids[] = {
	{
		.compatible = "cel,kennisis-cpld",
	},
	{ /* end of list */ },
};

static struct platform_driver cel_kennisis_cpld_driver = {
	.probe = cel_kennisis_cpld_probe,
	.remove = cel_kennisis_cpld_remove,
	.driver = {
		.name  = driver_name,
		.owner = THIS_MODULE,
		.of_match_table = cel_kennisis_cpld_ids,
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

static int __init cel_kennisis_cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&cel_kennisis_cpld_driver);
	if (rv) {
		printk(KERN_ERR
		       "%s platform_driver_register failed (%i)\n",
		       driver_name, rv);
	}
	return rv;
}

static void __exit cel_kennisis_cpld_exit(void)
{
	return platform_driver_unregister(&cel_kennisis_cpld_driver);
}

MODULE_AUTHOR("Vidya Sagar Ravipati <vidya@cumulusnetworks.com>");
MODULE_DESCRIPTION("CPLD driver for Celestica Inc., Kennisis");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(cel_kennisis_cpld_init);
module_exit(cel_kennisis_cpld_exit);
