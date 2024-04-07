/*
 * CPLD glue driver for dni-7448 as described by a flattened OF device tree
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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <asm/io.h>

#include "dni_6448_cpld.h"
#include "platform_defs.h"

static const char driver_name[] = "dni_6448_cpld";
#define DRIVER_VERSION "1.0"

/*------------------------------------------------------------------------------
 *
 * Driver resident static variables
 *
 */

static uint8_t __iomem* dni6448_cpld_regs;
static DEFINE_MUTEX(dni6448_cpld_mutex);


/********************************************************************************
 *
 * CPLD I/O
 *
 */
static inline uint8_t cpld_rd( uint32_t reg)
{
	return readb( dni6448_cpld_regs + reg);
}

static inline void cpld_wr( uint32_t reg, uint8_t data)
{
	writeb( data, dni6448_cpld_regs + reg);
}

#define DNI6448_CPLD_STRING_NAME_SIZE 20

/*------------------------------------------------------------------------------
 *
 * PSU status definitions
 *
 * All the definition names use "positive" logic and return "1" for OK
 * and "0" for not OK.
 */
struct psu_status {
	char name[DNI6448_CPLD_STRING_NAME_SIZE];
	uint8_t good_mask;  // set bits for "good behaviour"
	uint8_t bad_mask;   // set bits for "bad behaviour"
};

static struct psu_status cpld_psu_status[] = {
	{
		.name = "all_ok",
		.good_mask = CPLD_PS_PRESENT | CPLD_PS_AC_GOOD | CPLD_PS_DC_GOOD,
		.bad_mask = CPLD_PS_TEMP_BAD | CPLD_PS_FAN_BAD,
	},
	{
		.name = "present",
		.good_mask = CPLD_PS_PRESENT,
		.bad_mask = 0,
	},
	{
		.name = "ac_ok",
		.good_mask = CPLD_PS_PRESENT | CPLD_PS_AC_GOOD,
		.bad_mask = 0,
	},
	{
		.name = "dc_ok",
		.good_mask = CPLD_PS_PRESENT | CPLD_PS_DC_GOOD,
		.bad_mask = 0,
	},
	{
		.name = "temp_ok",
		.good_mask = CPLD_PS_PRESENT,
		.bad_mask = CPLD_PS_TEMP_BAD,
	},
	{
		.name = "fan_ok",
		.good_mask = CPLD_PS_PRESENT,
		.bad_mask = CPLD_PS_FAN_BAD,
	},
};
static int n_psu_states = ARRAY_SIZE(cpld_psu_status);

/*
 * board version
 */
static ssize_t board_revision_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{

	uint8_t rev, ext_rev;
	uint8_t hw_ver, cpld_ver;
	uint8_t prod_id;

	rev     = cpld_rd( CPLD_REG_REVISION);
	ext_rev = cpld_rd( CPLD_REG_EXT_REVISION);
	prod_id = cpld_rd( CPLD_REG_PRODUCT_ID);

	hw_ver = rev & 0xF;
	if ( (rev & 0xF0) == 0xF0) {
		cpld_ver = ext_rev;
	}
	else	{
		cpld_ver = (rev >> 4) & 0xf;
	}

	return sprintf(buf, "%d.%d:%d\n", prod_id, hw_ver, cpld_ver);
}
static SYSFS_ATTR_RO(board_revision, board_revision_show);

/*
 * PSU status bits
 */
static ssize_t psu_power_show(struct device * dev,
			      struct device_attribute * dattr,
			      char * buf)
{
	uint8_t tmp, i;
	uint8_t bad = 0;
	uint8_t name_len = strlen(xstr(PLATFORM_PS_NAME_0));
	struct psu_status* target = NULL;

	/* find the target PSU */
	if (strncmp(dattr->attr.name, xstr(PLATFORM_PS_NAME_0), name_len) == 0) {
		tmp = cpld_rd( CPLD_REG_PS1_STATUS);
	} else {
		tmp = cpld_rd( CPLD_REG_PS2_STATUS);
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
	if ( (tmp & target->good_mask) == target->good_mask) {
		if ( tmp & target->bad_mask) {
			bad++;
		}
	}
	else {
		bad++;
	}

	return sprintf( buf, "%c\n", bad ? '0' : '1');

}

static SYSFS_ATTR_RO(psu_pwr1_all_ok,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_present, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_ac_ok,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_dc_ok,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_temp_ok, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_fan_ok,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_all_ok,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_present, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_ac_ok,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_dc_ok,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_temp_ok, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_fan_ok,	psu_power_show);

/*
 * fan status
 */
static ssize_t fan_show(struct device * dev,
			struct device_attribute * dattr,
			char * buf)
{
	uint8_t tmp;
	uint8_t mask;

	tmp = cpld_rd( CPLD_REG_FAN_TRAY);
	if (strcmp(dattr->attr.name, "fan_0") == 0) {
		mask      = CPLD_FAN1_PRESENT_L;
	} else {
		mask      = CPLD_FAN2_PRESENT_L;
	}

	if (tmp & mask) {
		return sprintf(buf, PLATFORM_NOT_INSTALLED "\n");
	}
	else {
		return sprintf(buf, PLATFORM_INSTALLED "," PLATFORM_OK "\n");
	}

}
static SYSFS_ATTR_RO(fan_0, fan_show);
static SYSFS_ATTR_RO(fan_1, fan_show);


/*------------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[DNI6448_CPLD_STRING_NAME_SIZE];
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_locator",
		.mask = CPLD_SYS_LED_LOCATOR_MSK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN,          CPLD_SYS_LED_LOCATOR_GREEN},
			{ PLATFORM_LED_GREEN_BLINKING, CPLD_SYS_LED_LOCATOR_GREEN_BLINKING},
			{ PLATFORM_LED_OFF,            CPLD_SYS_LED_LOCATOR_OFF},
		},
	},
	{
		.name = "led_fan",
		.mask = 1 << CPLD_SYS_LED_FAN,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYS_LED_FAN_GREEN},
			{ PLATFORM_LED_RED,   CPLD_SYS_LED_FAN_RED},
		},
	},
	{
		.name = "led_master",
		.mask = 1 << CPLD_SYS_LED_MASTER,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_SYS_LED_MASTER_GREEN},
			{ PLATFORM_LED_OFF,   CPLD_SYS_LED_MASTER_OFF},
		},
	},
	{
		.name = "led_status",
		.mask = CPLD_SYS_LED_STATUS_MSK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_RED,            CPLD_SYS_LED_STATUS_RED},
			{ PLATFORM_LED_RED_BLINKING,   CPLD_SYS_LED_STATUS_RED_BLINKING},
			{ PLATFORM_LED_GREEN,          CPLD_SYS_LED_STATUS_GREEN},
			{ PLATFORM_LED_GREEN_BLINKING, CPLD_SYS_LED_STATUS_GREEN_BLINKING},
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
		return sprintf(buf, "undefined\n");
	}

	/* read the register */
	tmp = cpld_rd( CPLD_REG_SYSTEM_LED);

	/* find the color */
	tmp &= target->mask;
	for (i = 0; i < target->n_colors; i++) {
		if (tmp == target->colors[i].value) {
			break;
		}
	}
	if (i == target->n_colors) {
		return sprintf(buf, "undefined\n");
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

	mutex_lock(&dni6448_cpld_mutex);

	tmp = cpld_rd( CPLD_REG_SYSTEM_LED);
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;
	cpld_wr( CPLD_REG_SYSTEM_LED, tmp);

	mutex_unlock(&dni6448_cpld_mutex);

	return count;
}
static SYSFS_ATTR_RW(led_fan,     led_show, led_store);
static SYSFS_ATTR_RW(led_locator, led_show, led_store);
static SYSFS_ATTR_RW(led_master,  led_show, led_store);
static SYSFS_ATTR_RW(led_status,  led_show, led_store);

/*
 * 7-segment LEDs
 */

static ssize_t seg7_value_show(struct device * dev,
			       struct device_attribute * dattr,
			       char * buf)
{
	uint8_t msb, lsb;
	uint8_t val;

	/* read the value registers */
	mutex_lock(&dni6448_cpld_mutex);
	msb = cpld_rd( CPLD_REG_7SEGMENT_LED_MSB);
	lsb = cpld_rd( CPLD_REG_7SEGMENT_LED_LSB);
	mutex_unlock(&dni6448_cpld_mutex);

	/*
	 * Each 7-segment LED has 3 attributes:
	 *   on/off
	 *   green or green_blinking
	 *   hex digit, 0-f
	 */

	val = ((msb & CPLD_7SEGMENT_DIGIT_MASK) << 4) |
		(lsb & CPLD_7SEGMENT_DIGIT_MASK);
	return sprintf( buf, "%02X\n", val);

}

static ssize_t seg7_value_store(struct device * dev,
				struct device_attribute * dattr,
				const char * buf, size_t count)
{
	uint8_t msb, lsb;
	uint32_t msb_val, lsb_val;

	/*
	 * Each 7-segment LED has 3 attributes:
	 *   on/off
	 *   green or green_blinking
	 *   hex digit, 0-f
	 *
	 * Assume input is a two byte string containing two hex
	 * digits.
	 */

	if ( (buf[2] != 0) && (buf[2] != '\n')) {
		return -EINVAL;
	}

	if ( (buf[0] >= '0') && (buf[0] <= '9') ) {
		msb_val = buf[0] - '0';
	}
	else if ( (buf[0] >= 'a') && (buf[0] <= 'f') ) {
		msb_val = 10 + buf[0] - 'a';
	}
	else if ( (buf[0] >= 'A') && (buf[0] <= 'F') ) {
		msb_val = 10 + buf[0] - 'A';
	}
	else {
		return -EINVAL;
	}

	if ( (buf[1] >= '0') && (buf[1] <= '9') ) {
		lsb_val = buf[1] - '0';
	}
	else if ( (buf[1] >= 'a') && (buf[1] <= 'f') ) {
		lsb_val = 10 + buf[1] - 'a';
	}
	else if ( (buf[1] >= 'A') && (buf[1] <= 'F') ) {
		lsb_val = 10 + buf[1] - 'A';
	}
	else {
		return -EINVAL;
	}

	mutex_lock(&dni6448_cpld_mutex);

	msb = cpld_rd( CPLD_REG_7SEGMENT_LED_MSB);
	lsb = cpld_rd( CPLD_REG_7SEGMENT_LED_LSB);

	msb &= ~CPLD_7SEGMENT_DIGIT_MASK;
	msb |= msb_val;
	lsb &= ~CPLD_7SEGMENT_DIGIT_MASK;
	lsb |= lsb_val;

	cpld_wr( CPLD_REG_7SEGMENT_LED_MSB, msb);
	cpld_wr( CPLD_REG_7SEGMENT_LED_LSB, lsb);

	mutex_unlock(&dni6448_cpld_mutex);

	return count;
}
static SYSFS_ATTR_RW(display_value, seg7_value_show, seg7_value_store);

static ssize_t seg7_color_show(struct device * dev,
			       struct device_attribute * dattr,
			       char * buf)
{
	uint8_t msb;

	/* read the value registers */
	msb = cpld_rd( CPLD_REG_7SEGMENT_LED_MSB);

	/*
	 * Each 7-segment LED has 3 attributes:
	 *   on/off
	 *   green or green_blinking
	 *   hex digit, 0-f
	 *
	 * Both registers are programmed the same for color, so just
	 * look at the MSB register.
	 */

	if ( msb & CPLD_7SEGMENT_ON) {
		return sprintf( buf, "%s\n", (msb & CPLD_7SEGMENT_BLINK) ?
				PLATFORM_LED_GREEN_BLINKING : PLATFORM_LED_GREEN);
	}
	else {
		return sprintf( buf, "%s\n", PLATFORM_LED_OFF);
	}

}

static struct led_color seg7_color[3] = {
	{ PLATFORM_LED_GREEN,          CPLD_7SEGMENT_ON},
	{ PLATFORM_LED_GREEN_BLINKING, CPLD_7SEGMENT_ON | CPLD_7SEGMENT_BLINK},
	{ PLATFORM_LED_OFF,            0},
};

static ssize_t seg7_color_store(struct device * dev,
				struct device_attribute * dattr,
				const char * buf, size_t count)
{
	uint8_t msb, lsb, i;
	char raw[PLATFORM_LED_COLOR_NAME_SIZE];

	/*
	 * Each 7-segment LED has 3 attributes:
	 *   on/off
	 *   green or green_blinking
	 *   hex digit, 0-f
	 *
	 * Both registers are programmed the same for color.
	 */

	/* find the color */
	if (sscanf(buf, "%19s", raw) <= 0) {
		return -EINVAL;
	}
	for (i = 0; i < 3; i++) {
		if (strcmp(raw, seg7_color[i].name) == 0) {
			break;
		}
	}

	if ( i == 3) {
		return -EINVAL;
	}

	mutex_lock(&dni6448_cpld_mutex);

	msb = cpld_rd( CPLD_REG_7SEGMENT_LED_MSB);
	lsb = cpld_rd( CPLD_REG_7SEGMENT_LED_LSB);

	msb &= ~CPLD_7SEGMENT_COLOR_MASK;
	msb |= seg7_color[i].value;
	lsb &= ~CPLD_7SEGMENT_COLOR_MASK;
	lsb |= seg7_color[i].value;

	cpld_wr( CPLD_REG_7SEGMENT_LED_MSB, msb);
	cpld_wr( CPLD_REG_7SEGMENT_LED_LSB, lsb);

	mutex_unlock(&dni6448_cpld_mutex);

	return count;
}

static SYSFS_ATTR_RW(display_color, seg7_color_show, seg7_color_store);

/*------------------------------------------------------------------------------
 *
 * Mux-able ports 45, 46, 47, 48 sysfs functions
 *
 * Also used for the 10G ports on the hot-swap modules, just they
 * don't use the mux field.
 *
 */

#define DNI6448_CPLD_PORT_NAME_SIZE 20
struct port_info {
	char name[DNI6448_CPLD_PORT_NAME_SIZE];
	uint8_t  mux;
	uint32_t tx_dis_reg;
	uint8_t  tx_dis;
	uint32_t rx_los_reg;
	uint8_t  rx_los;
	uint32_t pres_reg;
	uint8_t  pres;
};

/* this register is read only... we need to keep state */
static uint8_t sfp_tx_ctrl;

/* All names in ports[] must have the same length */
#define PORT_NAME_LEN (6)

static struct port_info ports[] = {
	{
		.name       = "port45",
		.mux        = CPLD_PORT_MUX_45,
		.tx_dis_reg = CPLD_REG_SPF_TX_CTRL,
		.tx_dis     = CPLD_TX_DIS_45,
		.rx_los_reg = CPLD_REG_SPF_RX_LOS,
		.rx_los     = CPLD_RX_LOS_45,
		.pres_reg   = CPLD_REG_SPF_PRESENT,
		.pres       = CPLD_SFP_PRESENT_L_45,
	},
	{
		.name       = "port46",
		.mux        = CPLD_PORT_MUX_46,
		.tx_dis_reg = CPLD_REG_SPF_TX_CTRL,
		.tx_dis     = CPLD_TX_DIS_46,
		.rx_los_reg = CPLD_REG_SPF_RX_LOS,
		.rx_los     = CPLD_RX_LOS_46,
		.pres_reg   = CPLD_REG_SPF_PRESENT,
		.pres       = CPLD_SFP_PRESENT_L_46,
	},
	{
		.name       = "port47",
		.mux        = CPLD_PORT_MUX_47,
		.tx_dis_reg = CPLD_REG_SPF_TX_CTRL,
		.tx_dis     = CPLD_TX_DIS_47,
		.rx_los_reg = CPLD_REG_SPF_RX_LOS,
		.rx_los     = CPLD_RX_LOS_47,
		.pres_reg   = CPLD_REG_SPF_PRESENT,
		.pres       = CPLD_SFP_PRESENT_L_47,
	},
	{
		.name       = "port48",
		.mux        = CPLD_PORT_MUX_48,
		.tx_dis_reg = CPLD_REG_SPF_TX_CTRL,
		.tx_dis     = CPLD_TX_DIS_48,
		.rx_los_reg = CPLD_REG_SPF_RX_LOS,
		.rx_los     = CPLD_RX_LOS_48,
		.pres_reg   = CPLD_REG_SPF_PRESENT,
		.pres       = CPLD_SFP_PRESENT_L_48,
	},
	/*
	** Ports 49-52 are on the 10G modules.
	*/
	{
		.name       = "port49",
		.tx_dis_reg = CPLD_REG_SPF_PLUS_TX_CTRL,
		.tx_dis     = CPLD_TX_DIS_49,
		.rx_los_reg = CPLD_REG_SPF_PLUS_STATUS,
		.rx_los     = CPLD_RX_LOS_49,
		.pres_reg   = CPLD_REG_SPF_PLUS_STATUS,
		.pres       = CPLD_SFP_PRESENT_L_49,
	},
	{
		.name       = "port50",
		.tx_dis_reg = CPLD_REG_SPF_PLUS_TX_CTRL,
		.tx_dis     = CPLD_TX_DIS_50,
		.rx_los_reg = CPLD_REG_SPF_PLUS_STATUS,
		.rx_los     = CPLD_RX_LOS_50,
		.pres_reg   = CPLD_REG_SPF_PLUS_STATUS,
		.pres       = CPLD_SFP_PRESENT_L_50,
	},
	{
		.name       = "port51",
		.tx_dis_reg = CPLD_REG_SPF_PLUS_TX_CTRL,
		.tx_dis     = CPLD_TX_DIS_51,
		.rx_los_reg = CPLD_REG_SPF_PLUS_STATUS,
		.rx_los     = CPLD_RX_LOS_51,
		.pres_reg   = CPLD_REG_SPF_PLUS_STATUS,
		.pres       = CPLD_SFP_PRESENT_L_51,
	},
	{
		.name       = "port52",
		.tx_dis_reg = CPLD_REG_SPF_PLUS_TX_CTRL,
		.tx_dis     = CPLD_TX_DIS_52,
		.rx_los_reg = CPLD_REG_SPF_PLUS_STATUS,
		.rx_los     = CPLD_RX_LOS_52,
		.pres_reg   = CPLD_REG_SPF_PLUS_STATUS,
		.pres       = CPLD_SFP_PRESENT_L_52,
	},
};
static int n_port = sizeof(ports) / sizeof(ports[0]);

#define TYPE_MUX     (0)
#define TYPE_TX_CTRL (1)
#define TYPE_RX_LOS  (2)
#define TYPE_PRESENT (3)
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
	if ( strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "mux") == 0) {
		type = TYPE_MUX;
		offset = CPLD_REG_PORT_MUX;
	}
	else if ( strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "tx_ctrl") == 0) {
		type = TYPE_TX_CTRL;
		offset = target->tx_dis_reg;
	}
	else if ( strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "rx_status") == 0) {
		type = TYPE_RX_LOS;
		offset = target->rx_los_reg;
	}
	else if ( strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "sfp_present") == 0) {
		type = TYPE_PRESENT;
		offset = target->pres_reg;
	}
	else {
		return sprintf(buf, "undefined\n");
	}

	/* read the register */
	tmp = cpld_rd( offset);

	switch (type) {
	case TYPE_MUX:
		if ( tmp & target->mux) {
			str = "fiber";
		}
		else {
			str = "copper";
		}
		break;
	case TYPE_TX_CTRL:
		/* combo port reg is write only, use local state for the read*/
		if (target->tx_dis_reg == CPLD_REG_SPF_TX_CTRL) {
			tmp = sfp_tx_ctrl;
		}
		if ( tmp & target->tx_dis) {
			str = "disable";
		}
		else {
			str = "enable";
		}
		break;
	case TYPE_RX_LOS:
		if ( tmp & target->rx_los) {
			str = "link_down";
		}
		else {
			str = "link_up";
		}
		break;
	case TYPE_PRESENT:
		if ( tmp & target->pres) {
			str = "0";
		}
		else {
			str = "1";
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
	if ( strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "mux") == 0) {
		offset = CPLD_REG_PORT_MUX;
	}
	else if ( strcmp(dattr->attr.name + PORT_NAME_LEN + 1, "tx_ctrl") == 0) {
		offset = target->tx_dis_reg;
	}
	else {
		return -EINVAL;
	}

	/* find the setting */
	if (sscanf(buf, "%19s", raw) <= 0) {
		return -EINVAL;
	}

	mutex_lock(&dni6448_cpld_mutex);
	tmp = cpld_rd( offset);

	switch (offset) {
	case CPLD_REG_PORT_MUX:
		if ( strcmp( raw, "fiber") == 0) {
			tmp |= target->mux;
		}
		else if ( strcmp( raw, "copper") == 0) {
			tmp &= ~target->mux;
		}
		else {
			goto fail;
		}
		break;
	case CPLD_REG_SPF_TX_CTRL:
	case CPLD_REG_SPF_PLUS_TX_CTRL:
		/* combo port reg is write only, use local state for the read*/
		if (target->tx_dis_reg == CPLD_REG_SPF_TX_CTRL) {
			tmp = sfp_tx_ctrl;
		}
		if ( strcmp( raw, "disable") == 0) {
			tmp |= target->tx_dis;
		}
		else if ( strcmp( raw, "enable") == 0) {
			tmp &= ~target->tx_dis;
		}
		else {
			goto fail;
		}
		if (target->tx_dis_reg == CPLD_REG_SPF_TX_CTRL) {
			sfp_tx_ctrl = tmp;
		}
		break;
	default:
		goto fail;
	};

	cpld_wr( offset, tmp);
	mutex_unlock(&dni6448_cpld_mutex);

	return count;

fail:
	mutex_unlock(&dni6448_cpld_mutex);
	return -EINVAL;
}

static SYSFS_ATTR_RW(port45_mux, port_show, port_store);
static SYSFS_ATTR_RW(port46_mux, port_show, port_store);
static SYSFS_ATTR_RW(port47_mux, port_show, port_store);
static SYSFS_ATTR_RW(port48_mux, port_show, port_store);
static SYSFS_ATTR_RW(port45_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port46_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port47_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port48_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port49_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port50_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port51_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RW(port52_tx_ctrl, port_show, port_store);
static SYSFS_ATTR_RO(port45_rx_status, port_show);
static SYSFS_ATTR_RO(port46_rx_status, port_show);
static SYSFS_ATTR_RO(port47_rx_status, port_show);
static SYSFS_ATTR_RO(port48_rx_status, port_show);
static SYSFS_ATTR_RO(port49_rx_status, port_show);
static SYSFS_ATTR_RO(port50_rx_status, port_show);
static SYSFS_ATTR_RO(port51_rx_status, port_show);
static SYSFS_ATTR_RO(port52_rx_status, port_show);
static SYSFS_ATTR_RO(port45_sfp_present, port_show);
static SYSFS_ATTR_RO(port46_sfp_present, port_show);
static SYSFS_ATTR_RO(port47_sfp_present, port_show);
static SYSFS_ATTR_RO(port48_sfp_present, port_show);
static SYSFS_ATTR_RO(port49_sfp_present, port_show);
static SYSFS_ATTR_RO(port50_sfp_present, port_show);
static SYSFS_ATTR_RO(port51_sfp_present, port_show);
static SYSFS_ATTR_RO(port52_sfp_present, port_show);


/*
** Define a more programmer friendly format for the port control
** and status registers.
**
** Ports 49-52 are the 10G SFP+ modules and do not support the mux
** bit.
**
*/
static ssize_t port_ctrl_read(struct device * dev,
			      struct device_attribute * dattr,
			      char * buf)
{
	uint8_t mux, ge_tx_dis, ge_rx_los, ge_pres;
	uint8_t xe_tx_dis, xe_status;
	uint32_t val, p;

	mutex_lock(&dni6448_cpld_mutex);

	mux       = cpld_rd( CPLD_REG_PORT_MUX);
	/* the combo port reg is write only, use internal state */
	ge_tx_dis = sfp_tx_ctrl;
	ge_rx_los = cpld_rd( CPLD_REG_SPF_RX_LOS);
	ge_pres   = cpld_rd( CPLD_REG_SPF_PRESENT);
	xe_tx_dis = cpld_rd( CPLD_REG_SPF_PLUS_TX_CTRL);
	xe_status = cpld_rd( CPLD_REG_SPF_PLUS_STATUS);

	mutex_unlock(&dni6448_cpld_mutex);

	val = 0x0;
	// 1G ports 45 - 48
	for ( p = 45; p < 49; p++) {
		dni6448_set_fiber_mode ( &val, p, (mux & (1 << (48 - p))));
		dni6448_set_tx_enable  ( &val, p, /* inverted */
					 !(ge_tx_dis & (1 << (4 + p - 45))));
		dni6448_set_rx_los     ( &val, p,
					 ge_tx_dis & (1 << (4 + p - 45)));
		dni6448_set_sfp_present( &val, p, /* inverted */
					 !(ge_pres & (1 << (4 + p - 45))));
	}

	// 10G ports 49 - 50
	for ( p = 49; p < 51; p++) {
		dni6448_set_fiber_mode ( &val, p, 1); // always fiber
		dni6448_set_tx_enable  ( &val, p, /* inverted */
					 !(xe_tx_dis & (1 << (p - 49))));
		dni6448_set_rx_los     ( &val, p,
					 (xe_status & (1 << (p - 49))));
		dni6448_set_sfp_present( &val, p, /* inverted */
					 !(xe_status & (1 << (2 + p - 49))));
	}

	// 10G ports 51 - 52
	for ( p = 51; p < 53; p++) {
		dni6448_set_fiber_mode ( &val, p, 1); // always fiber
		dni6448_set_tx_enable  ( &val, p, /* inverted */
					 !(xe_tx_dis & (1 << (p - 49))));
		dni6448_set_rx_los     ( &val, p,
					 (xe_status & (1 << (4 + p - 51))));
		dni6448_set_sfp_present( &val, p, /* inverted */
					 !(xe_status & (1 << (6 + p - 51))));
	}

	*((uint32_t*)buf) = val;
	return sizeof(val);
}

/*
** For ports 44-48 only the MUX and TX disable bits are writable.
**
** Ports 49-52 are the 10G SFP+ modules and do not support the mux
** bit, only the TX disable bits are writable.
**
*/
static ssize_t port_ctrl_write(struct device * dev,
			       struct device_attribute * dattr,
			       const char * buf, size_t count)
{
	uint8_t mux, ge_tx_dis, xe_tx_dis;
	uint32_t val, p;

	val = *((uint32_t*)buf);

	mutex_lock(&dni6448_cpld_mutex);

	mux    = cpld_rd( CPLD_REG_PORT_MUX);
	/* the combo port reg is write only, use internal state */
	ge_tx_dis = sfp_tx_ctrl;
	xe_tx_dis = cpld_rd( CPLD_REG_SPF_PLUS_TX_CTRL);

	// 1G ports 45 - 48
	for ( p = 45; p < 49; p++) {
		mux &= ~(1 << (48 - p));
		mux |= dni6448_get_fiber_mode( val, p) << (48 - p);
		ge_tx_dis &= ~(1 << (4 + p - 45));
		ge_tx_dis |= !dni6448_get_tx_enable( val, p) << (4 + p - 45); /* inverted */
	}

	// 10G ports 49 - 52
	for ( p = 49; p < 53; p++) {
		xe_tx_dis &= ~(1 << (p - 49));
		xe_tx_dis |= !dni6448_get_tx_enable( val, p) << (p - 49); /* inverted */
	}

	cpld_wr( CPLD_REG_PORT_MUX, mux);
	cpld_wr( CPLD_REG_SPF_TX_CTRL, ge_tx_dis);
	sfp_tx_ctrl = ge_tx_dis;
	cpld_wr( CPLD_REG_SPF_PLUS_TX_CTRL, xe_tx_dis);

	mutex_unlock(&dni6448_cpld_mutex);

	return count;
}

static SYSFS_ATTR_RW(port_raw_ctrl, port_ctrl_read, port_ctrl_write);

static ssize_t usb_status_read(struct device * dev,
			       struct device_attribute * dattr,
			       char * buf)
{
	uint8_t tmp;

	tmp = cpld_rd( CPLD_REG_USB_CTRL);

	if ( tmp & CPLD_USB_ERROR_L) {
		return sprintf(buf, PLATFORM_OK "\n");
	}
	else {
		return sprintf(buf, "over_current\n");
	}

}
static SYSFS_ATTR_RO(usb_status, usb_status_read);

/*------------------------------------------------------------------------------
 *
 * Hot Swap Module status and control registers.
 *
 */

#define DNI6448_CPLD_MODULE_NAME_SIZE 20
struct mod_info {
	char name[DNI6448_CPLD_MODULE_NAME_SIZE];
	uint8_t  pwr_en_l;
	uint32_t status_reg;
};

// All names in mods[] must have the same length
#define MOD_NAME_LEN (8)

static struct mod_info mods[] = {
	{
		.name       = "mod49_50",
		.pwr_en_l   = CPLD_10G_49_50_EN_L,
		.status_reg = CPLD_REG_MOD_49_50_STATUS,
	},
	{
		.name       = "mod51_52",
		.pwr_en_l   = CPLD_10G_51_52_EN_L,
		.status_reg = CPLD_REG_MOD_51_52_STATUS,
	},
};
static int n_mod = sizeof(mods) / sizeof(mods[0]);


#define MOD_OP_POWER   (0)
#define MOD_OP_PRESENT (1)
#define MOD_OP_READY   (2)
#define MOD_OP_TYPE    (3)
static ssize_t mod_show(struct device * dev,
			struct device_attribute * dattr,
			char * buf)
{
	uint8_t tmp;
	unsigned int offset;
	int i;
	struct mod_info * target = NULL;
	int op;
	char* str;

	/* find the target module */
	for (i = 0; i < n_mod; i++) {
		if (strncmp(dattr->attr.name,
			    mods[i].name, MOD_NAME_LEN) == 0) {
			target = &mods[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined\n");
	}

	/* find the target operation */
	if ( strcmp(dattr->attr.name + MOD_NAME_LEN + 1, "power") == 0) {
		op = MOD_OP_POWER;
		offset = CPLD_REG_10G_MOD_ENABLE;
	}
	else if ( strcmp(dattr->attr.name + MOD_NAME_LEN + 1, "present") == 0) {
		op = MOD_OP_PRESENT;
		offset = target->status_reg;
	}
	else if ( strcmp(dattr->attr.name + MOD_NAME_LEN + 1, "ready") == 0) {
		op = MOD_OP_READY;
		offset = target->status_reg;
	}
	else if ( strcmp(dattr->attr.name + MOD_NAME_LEN + 1, "type") == 0) {
		op = MOD_OP_TYPE;
		offset = target->status_reg;
	}
	else {
		return sprintf(buf, "undefined\n");
	}

	/* read the register */
	tmp = cpld_rd( offset);

	switch (op) {
	case MOD_OP_POWER:
		if ( tmp & target->pwr_en_l) {
			str = "0";
		}
		else {
			str = "1";
		}
		break;
	case MOD_OP_PRESENT:
		if ( tmp & CPLD_10G_MOD_PRESENT_L) {
			str = "0";
		}
		else {
			str = "1";
		}
		break;
	case MOD_OP_READY:
		if ( tmp & CPLD_10G_MOD_READY_L) {
			str = "0";
		}
		else {
			str = "1";
		}
		break;
	case MOD_OP_TYPE:
		return sprintf(buf, "%d\n", tmp & CPLD_10G_MOD_ID_MASK);
	default:
		str = "undefined";
	};

	return sprintf(buf, "%s\n", str);
}

static ssize_t mod_store(struct device * dev,
			  struct device_attribute * dattr,
			  const char * buf, size_t count)
{
	uint8_t tmp;
	int i;
	struct mod_info* target = NULL;
	int enable = 0;

	/* find the target mod */
	for (i = 0; i < n_mod; i++) {
		if (strncmp(dattr->attr.name,
			    mods[i].name, MOD_NAME_LEN) == 0) {
			target = &mods[i];
			break;
		}
	}
	if (target == NULL) {
		return -EINVAL;
	}

	/* find the setting - expect 0 or 1 */
	if (sscanf(buf, "%d", &enable) <= 0) {
		return -EINVAL;
	}

	/* Only one operation, set the power on/off */
	mutex_lock(&dni6448_cpld_mutex);
	tmp = cpld_rd( CPLD_REG_10G_MOD_ENABLE);

	if ( enable) {
		tmp &= ~target->pwr_en_l;
	}
	else {
		tmp |= target->pwr_en_l;
	}

	cpld_wr( CPLD_REG_10G_MOD_ENABLE, tmp);
	mutex_unlock(&dni6448_cpld_mutex);

	return count;
}

static SYSFS_ATTR_RW(mod49_50_power, mod_show, mod_store);
static SYSFS_ATTR_RW(mod51_52_power, mod_show, mod_store);
static SYSFS_ATTR_RO(mod49_50_present, mod_show);
static SYSFS_ATTR_RO(mod51_52_present, mod_show);
static SYSFS_ATTR_RO(mod49_50_ready, mod_show);
static SYSFS_ATTR_RO(mod51_52_ready, mod_show);
static SYSFS_ATTR_RO(mod49_50_type, mod_show);
static SYSFS_ATTR_RO(mod51_52_type, mod_show);

/*------------------------------------------------------------------------------
 *
 * POE+ Reset and Control Register Show.
 *
 */
static ssize_t poe_plus_power_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	uint8_t poe_status, poe_pwr_status;

	poe_status = cpld_rd( CPLD_REG_POE_PLUS_CTRL_RESET);

	if (poe_status & (CPLD_POE_PLUS_PWR_ENABLE)) {
		poe_pwr_status = 1;
	} else {
		poe_pwr_status = 0;
	}

	return sprintf(buf, "%d\n", poe_pwr_status);
}

static ssize_t poe_plus_power_store(struct device * dev,
			  struct device_attribute * dattr,
			  const char * buf, size_t count)
{
	uint8_t tmp;
	int enable = 0;

	if (sscanf(buf, "%d", &enable) <= 0) {
		return -EINVAL;
	}

	/* Only one operation, set the power on/off */
	mutex_lock(&dni6448_cpld_mutex);

	tmp = cpld_rd(CPLD_REG_POE_PLUS_CTRL_RESET);

	if (enable) {
		tmp |= (CPLD_POE_PLUS_PWR_ENABLE);
	} else {
		tmp &= ~(CPLD_POE_PLUS_PWR_ENABLE);
	}
	cpld_wr(CPLD_REG_POE_PLUS_CTRL_RESET, tmp);
	tmp = cpld_rd(CPLD_REG_POE_PLUS_CTRL_RESET);

	mutex_unlock(&dni6448_cpld_mutex);

	return count;
}


static ssize_t poe_plus_reset_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	uint8_t poe_status, poe_reset_status;

	poe_status = cpld_rd(CPLD_REG_POE_PLUS_CTRL_RESET);

	if (poe_status & (CPLD_POE_PLUS_RESET_L)) {
		poe_reset_status = 0;
	} else {
		poe_reset_status = 1;
	}

	return sprintf(buf, "%d\n", poe_reset_status);
}

static ssize_t poe_plus_reset_store(struct device * dev,
			  struct device_attribute * dattr,
			  const char * buf, size_t count)
{
	uint8_t tmp;
	int reset = 0;

	if (sscanf(buf, "%d", &reset) <= 0) {
		return -EINVAL;
	}

	/* Only one operation, set the reset on/off */
	mutex_lock(&dni6448_cpld_mutex);

	tmp = cpld_rd(CPLD_REG_POE_PLUS_CTRL_RESET);

	if (reset) {
		tmp &= ~(CPLD_POE_PLUS_RESET_L);
	} else {
		tmp |= (CPLD_POE_PLUS_RESET_L);
	}

	cpld_wr(CPLD_REG_POE_PLUS_CTRL_RESET, tmp);
	mutex_unlock(&dni6448_cpld_mutex);

	return count;
}
static SYSFS_ATTR_RW(poe_plus_power, poe_plus_power_show, poe_plus_power_store);
static SYSFS_ATTR_RW(poe_plus_reset, poe_plus_reset_show, poe_plus_reset_store);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */

static struct attribute *dni6448_cpld_attrs[] = {
	&dev_attr_board_revision.attr,
	&dev_attr_psu_pwr1_all_ok.attr,
	&dev_attr_psu_pwr1_present.attr,
	&dev_attr_psu_pwr1_ac_ok.attr,
	&dev_attr_psu_pwr1_dc_ok.attr,
	&dev_attr_psu_pwr1_temp_ok.attr,
	&dev_attr_psu_pwr1_fan_ok.attr,
	&dev_attr_psu_pwr2_all_ok.attr,
	&dev_attr_psu_pwr2_present.attr,
	&dev_attr_psu_pwr2_ac_ok.attr,
	&dev_attr_psu_pwr2_dc_ok.attr,
	&dev_attr_psu_pwr2_temp_ok.attr,
	&dev_attr_psu_pwr2_fan_ok.attr,
	&dev_attr_fan_0.attr,
	&dev_attr_fan_1.attr,
	&dev_attr_led_fan.attr,
	&dev_attr_led_locator.attr,
	&dev_attr_led_master.attr,
	&dev_attr_led_status.attr,
	&dev_attr_display_value.attr,
	&dev_attr_display_color.attr,
	&dev_attr_port45_mux.attr,
	&dev_attr_port46_mux.attr,
	&dev_attr_port47_mux.attr,
	&dev_attr_port48_mux.attr,
	&dev_attr_port45_tx_ctrl.attr,
	&dev_attr_port46_tx_ctrl.attr,
	&dev_attr_port47_tx_ctrl.attr,
	&dev_attr_port48_tx_ctrl.attr,
	&dev_attr_port49_tx_ctrl.attr,
	&dev_attr_port50_tx_ctrl.attr,
	&dev_attr_port51_tx_ctrl.attr,
	&dev_attr_port52_tx_ctrl.attr,
	&dev_attr_port45_rx_status.attr,
	&dev_attr_port46_rx_status.attr,
	&dev_attr_port47_rx_status.attr,
	&dev_attr_port48_rx_status.attr,
	&dev_attr_port49_rx_status.attr,
	&dev_attr_port50_rx_status.attr,
	&dev_attr_port51_rx_status.attr,
	&dev_attr_port52_rx_status.attr,
	&dev_attr_port45_sfp_present.attr,
	&dev_attr_port46_sfp_present.attr,
	&dev_attr_port47_sfp_present.attr,
	&dev_attr_port48_sfp_present.attr,
	&dev_attr_port49_sfp_present.attr,
	&dev_attr_port50_sfp_present.attr,
	&dev_attr_port51_sfp_present.attr,
	&dev_attr_port52_sfp_present.attr,
	&dev_attr_port_raw_ctrl.attr,
	&dev_attr_usb_status.attr,
	&dev_attr_mod49_50_power.attr,
	&dev_attr_mod51_52_power.attr,
	&dev_attr_mod49_50_present.attr,
	&dev_attr_mod51_52_present.attr,
	&dev_attr_mod49_50_ready.attr,
	&dev_attr_mod51_52_ready.attr,
	&dev_attr_mod49_50_type.attr,
	&dev_attr_mod51_52_type.attr,
	&dev_attr_poe_plus_power.attr,
	&dev_attr_poe_plus_reset.attr,
	NULL,
};

static struct attribute_group dni6448_cpld_attr_group = {
	.attrs = dni6448_cpld_attrs,
};


/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static int dni6448_cpld_setup(void)
{
	// Put some interesting, one-time initializations here.
	/*
	 * Configure ports 45-48 for copper.
	 */
	cpld_wr( CPLD_REG_PORT_MUX, 0x0);

	/* enable tx_ctrl to the SFP and SFP+ slots and initialize internal state */
	sfp_tx_ctrl = 0x00;
	cpld_wr(CPLD_REG_SPF_TX_CTRL, sfp_tx_ctrl);
	cpld_wr(CPLD_REG_SPF_PLUS_TX_CTRL, 0x0);

	return 0;
}

static int dni6448_cpld_probe(struct platform_device * ofdev)
{
	int retval = 0;
	struct device_node * np = ofdev->dev.of_node;
	struct kobject * kobj = &ofdev->dev.kobj;

	if (dev_get_drvdata(&ofdev->dev)) {
		dev_info(&ofdev->dev, "already probed\n");
		return 0;
	}

	dni6448_cpld_regs = of_iomap(np,0);
	if (!dni6448_cpld_regs) {
		return -EIO;
	}

	retval = sysfs_create_group(kobj, &dni6448_cpld_attr_group);
	if (retval) {
		return retval;
	}

	if (dni6448_cpld_setup()) {
	     return -EIO;
	}

	dev_info(&ofdev->dev, "probed & iomapped @ 0x%p\n", dni6448_cpld_setup);

	return 0;
}

static int dni6448_cpld_remove(struct platform_device * ofdev)
{
	struct kobject * kobj = &ofdev->dev.kobj;

	/* don't iounmap(regs)... the platform driver uses it for reset	*/
	sysfs_remove_group(kobj, &dni6448_cpld_attr_group);

	dev_info(&ofdev->dev, "removed\n");
	return 0;
}

static struct of_device_id dni6448_cpld_ids[] = {
	{
		.compatible = "dni,6448-cpld",
	},
	{ /* end of list */ },
};

static struct platform_driver dni6448_cpld_driver = {
	.probe = dni6448_cpld_probe,
	.remove = dni6448_cpld_remove,
	.driver = {
		.name  = driver_name,
		.owner = THIS_MODULE,
		.of_match_table = dni6448_cpld_ids,
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

static int __init dni6448_cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&dni6448_cpld_driver);
	if (rv) {
		printk(KERN_ERR
		       "%s platform_driver_register failed (%i)\n",
		       driver_name, rv);
	}
	return rv;
}

static void __exit dni6448_cpld_exit(void)
{
	return platform_driver_unregister(&dni6448_cpld_driver);
}

MODULE_AUTHOR("Curt Brune <curt@cumulusnetworks.com>");
MODULE_DESCRIPTION("CPLD driver for Delta Networks Inc. ET-6448");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(dni6448_cpld_init);
module_exit(dni6448_cpld_exit);
