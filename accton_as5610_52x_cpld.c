/*
 * CPLD sysfs driver for accton AS5610_52X.
 *
 * Copyright (C) 2014 Cumulus Networks, Inc.
 * Author: Puneet Shenoy <puneeet@cumulusnetworks.com>
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
#include <linux/hwmon.h>
#include <linux/of_platform.h>
#include <asm/io.h>

#include "accton_as5610_52x_cpld.h"
#include "platform_defs.h"

static const char driver_name[] = "accton_as5610_52x_cpld";
#define DRIVER_VERSION "1.0"

/*------------------------------------------------------------------------------
 *
 * Driver resident static variables
 *
 */

static uint8_t __iomem* accton_as5610_52x_cpld_regs;
static DEFINE_MUTEX(accton_as5610_52x_cpld_mutex);
static uint8_t accton_as5610_52x_pwm1 = 255;  // default value

/********************************************************************************
 *
 * CPLD I/O
 *
 */
static inline uint8_t cpld_rd(uint32_t reg)
{
	return readb(accton_as5610_52x_cpld_regs + reg);
}

static inline void cpld_wr(uint32_t reg, uint8_t data)
{
	writeb(data, accton_as5610_52x_cpld_regs + reg);
}

#define ACCTON_AS5610_52X_CPLD_STRING_NAME_SIZE 30

/*
 * board version
 */
static ssize_t board_revision_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{

	uint8_t model, version;
	uint8_t pcb_ver, cpld_ver, cpld_rev;

	model   = cpld_rd(CPLD_REG_MODEL_TYPE);
	version = cpld_rd(CPLD_REG_VERSION);

	pcb_ver  = (model & CPLD_PCB_VER_MASK) >> CPLD_PCB_VER_SHIFT;
	model    = model & CPLD_PCB_MODEL_TYPE_MASK;
	cpld_rev = version & CPLD_VERSION_MASK;
	cpld_ver = version & CPLD_RELEASE_VERSION;

	return sprintf(buf, "%d.%d:%d%s\n", model, pcb_ver, cpld_rev, cpld_ver ? "" : "-eng");
}
static SYSFS_ATTR_RO(board_revision, board_revision_show);

/*
 *   fan speed/PWM
 */
static ssize_t pwm1_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	return sprintf(buf, "%d\n", accton_as5610_52x_pwm1);
}

static ssize_t pwm1_store(struct device * dev,
			 struct device_attribute * dattr,
			 const char *buf, size_t count)
{
	uint32_t pwm = 0;

	if (sscanf(buf, "%d", &pwm) <= 0) {
		return -EINVAL;
	}
	pwm = clamp_val(pwm, 0, 255);
	accton_as5610_52x_pwm1 = pwm;
	pwm >>= 3;
	cpld_wr(CPLD_REG_FAN_CTRL, pwm);

	return count;
 }

static ssize_t pwm1_enable_show(struct device * dev,
				struct device_attribute * dattr,
				char * buf)
{
	return sprintf(buf, "1\n");
}

static ssize_t pwm1_enable_store(struct device * dev,
			 struct device_attribute * dattr,
			 const char *buf, size_t count)
{
	// Do nothing. Needed for pwmd
	return count;
}

static SYSFS_ATTR_RW(pwm1, pwm1_show, pwm1_store);
static SYSFS_ATTR_RW(pwm1_enable, pwm1_enable_show, pwm1_enable_store);

/*------------------------------------------------------------------------------
 *
 * PSU status definitions
 *
 * All the definition names use "positive" logic and return "1" for OK
 * and "0" for not OK.
 */
struct cpld_status {
	char name[ACCTON_AS5610_52X_CPLD_STRING_NAME_SIZE];
	uint8_t good_mask;  // set bits for "good behaviour"
	uint8_t bad_mask;   // set bits for "bad behaviour"
	char msg_good[ACCTON_AS5610_52X_CPLD_STRING_NAME_SIZE]; // positive string message
	char msg_bad[ACCTON_AS5610_52X_CPLD_STRING_NAME_SIZE]; // negative string message
};

static struct cpld_status cpld_psu_status[] = {
	{
		.name = "all_ok",
		.good_mask = CPLD_PS_DC_OK,
		.bad_mask = CPLD_PS_ABSENT,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "present",
		.good_mask = 0,
		.bad_mask = CPLD_PS_ABSENT,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "dc_ok",
		.good_mask = CPLD_PS_DC_OK,
		.bad_mask = CPLD_PS_ABSENT,
		.msg_good = "1",
		.msg_bad = "0",
	},
};
static int n_psu_states = ARRAY_SIZE(cpld_psu_status);

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
	struct cpld_status* target = NULL;

	/* find the target PSU */
	if (strncmp(dattr->attr.name, xstr(PLATFORM_PS_NAME_0), name_len) == 0) {
		tmp = cpld_rd(CPLD_REG_PS1_STATUS);
	} else {
		tmp = cpld_rd(CPLD_REG_PS2_STATUS);
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

static SYSFS_ATTR_RO(psu_pwr1_all_ok,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_present,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_dc_ok,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_all_ok,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_present,	psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_dc_ok,	psu_power_show);

/*
 * System status register
 */
static struct cpld_status cpld_system_status[] = {
	{
		.name = "all_ok",
		.good_mask = CPLD_SYS_PWR_GOOD,
		.bad_mask = CPLD_SYS_FAN_ABSENT | CPLD_SYS_FAN_BAD,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "dc_power_ok",
		.good_mask = CPLD_SYS_PWR_GOOD,
		.bad_mask = 0,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan_present",
		.good_mask = 0,
		.bad_mask = CPLD_SYS_FAN_ABSENT,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan_ok",
		.good_mask = 0,
		.bad_mask = CPLD_SYS_FAN_ABSENT | CPLD_SYS_FAN_BAD,
		.msg_good = "1",
		.msg_bad = "0",
	},
	{
		.name = "fan_air_flow",
		.good_mask = CPLD_SYS_FAN_AIR_FLOW,
		.bad_mask = CPLD_SYS_FAN_ABSENT | CPLD_SYS_FAN_BAD,
		.msg_good = "front-to-back",
		.msg_bad = "back-to-front",
	},
};
static int n_system_states = ARRAY_SIZE(cpld_system_status);

/*
 * system status bits
 */
static ssize_t system_status_show(struct device * dev,
				  struct device_attribute * dattr,
				  char * buf)
{
	uint8_t tmp, i;
	uint8_t bad = 0;
	uint8_t name_len = strlen("system");
	struct cpld_status* target = NULL;

	tmp = cpld_rd(CPLD_REG_SYSTEM_STATUS);

	for (i = 0; i < n_system_states; i++) {
		if (strcmp(dattr->attr.name + name_len + 1, cpld_system_status[i].name) == 0) {
			target = &cpld_system_status[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined\n");
	}

	if (strcmp(target->name, "fan_air_flow") == 0) {
		if (tmp & CPLD_SYS_FAN_ABSENT) {
			return sprintf(buf, "not present\n");
		}
		else {
			return sprintf(buf, "%s\n", (tmp & CPLD_SYS_FAN_AIR_FLOW) ?
				       "front-to-back" : "back-to-front");
		}
	}
	else {
		/*
		** All of the "good" bits must be set.
		** None of the "bad" bits can be set.
		*/
		if ((tmp & target->good_mask) == target->good_mask) {
			if (tmp & target->bad_mask) {
				bad++;
			}
		}
		else {
			bad++;
		}

		return sprintf(buf, "%s\n", bad ? target->msg_bad : target->msg_good);
	}
	return sprintf(buf, "undefined\n");
}

static SYSFS_ATTR_RO(system_all_ok,	   system_status_show);
static SYSFS_ATTR_RO(system_dc_power_ok,  system_status_show);
static SYSFS_ATTR_RO(system_fan_present,  system_status_show);
static SYSFS_ATTR_RO(system_fan_ok,	   system_status_show);
static SYSFS_ATTR_RO(system_fan_air_flow, system_status_show);


/*------------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[ACCTON_AS5610_52X_CPLD_STRING_NAME_SIZE];
	uint8_t reg;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_psu1",
  		.reg  = CPLD_REG_SYSTEM_LED_CTRL_1,
  		.mask = CPLD_SYS_LED_PS1_MASK,
  		.n_colors = 3,
  		.colors = {
  			{ PLATFORM_LED_GREEN,   CPLD_SYS_LED_PS1_GREEN},
  			{ PLATFORM_LED_YELLOW,  CPLD_SYS_LED_PS1_YELLOW},
 			{ PLATFORM_LED_OFF,     CPLD_SYS_LED_PS1_OFF},
 		},
 	},
	{
		.name = "led_psu2",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_1,
		.mask = CPLD_SYS_LED_PS2_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN,   CPLD_SYS_LED_PS2_GREEN},
			{ PLATFORM_LED_YELLOW,  CPLD_SYS_LED_PS2_YELLOW},
			{ PLATFORM_LED_OFF,     CPLD_SYS_LED_PS2_OFF},
		},
	},
	{
		.name = "led_diag",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_1,
		.mask = CPLD_SYS_LED_DIAG_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN,   CPLD_SYS_LED_DIAG_GREEN},
			{ PLATFORM_LED_YELLOW,  CPLD_SYS_LED_DIAG_YELLOW},
			{ PLATFORM_LED_OFF,     CPLD_SYS_LED_DIAG_OFF},
		},
	},
	{
		.name = "led_fan",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_1,
		.mask = CPLD_SYS_LED_FAN_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN,   CPLD_SYS_LED_FAN_GREEN},
			{ PLATFORM_LED_YELLOW,  CPLD_SYS_LED_FAN_YELLOW},
			{ PLATFORM_LED_OFF,     CPLD_SYS_LED_FAN_OFF},
		},
	},
	{
		.name = "led_locator",
		.reg  = CPLD_REG_SYSTEM_LED_CTRL_2,
		.mask = CPLD_SYS_LED_LOCATOR_MASK,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_YELLOW_BLINKING,
			  CPLD_SYS_LED_LOCATOR_YELLOW_BLINK},
			{ PLATFORM_LED_OFF,
			  CPLD_SYS_LED_LOCATOR_OFF},
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

	mutex_lock(&accton_as5610_52x_cpld_mutex);

	tmp = cpld_rd(target->reg);
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;
	cpld_wr(target->reg, tmp);

	mutex_unlock(&accton_as5610_52x_cpld_mutex);

	return count;
}
static SYSFS_ATTR_RW(led_psu1,    led_show, led_store);
static SYSFS_ATTR_RW(led_psu2,    led_show, led_store);
static SYSFS_ATTR_RW(led_diag,    led_show, led_store);
static SYSFS_ATTR_RW(led_fan,     led_show, led_store);
static SYSFS_ATTR_RW(led_locator, led_show, led_store);

/*
 * Watch Dog Control
 */

struct watch_dog {
	char name[ACCTON_AS5610_52X_CPLD_STRING_NAME_SIZE];
	uint8_t bit;
};

struct watch_dog watch_dog_bits[] = {
	{
		.name = "watch_dog_keep_alive",
		.bit  = 0,
	},
	{
		.name = "watch_dog_enable",
		.bit  = 1,
	},
	{
		.name = "watch_dog_timeout",
	},
};

static int n_watch_dog_bits = ARRAY_SIZE(watch_dog_bits);

static int wd_cnt_tbl[] = {
  8,
 16,
 32,
 48,
 64,
 72,
 88,
 96,
128,
136,
149,
192,
256,
320,
448,
512,
};

static ssize_t watch_dog_show(struct device * dev,
			      struct device_attribute * dattr,
			      char * buf)
{
	uint8_t tmp;
	int i;
	struct watch_dog* target = NULL;

	/* find the target bit */
	for (i = 0; i < n_watch_dog_bits; i++) {
		if (strcmp(dattr->attr.name, watch_dog_bits[i].name) == 0) {
			target = &watch_dog_bits[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined target\n");
	}

	/* read the register */
	tmp = cpld_rd(CPLD_REG_WATCH_DOG_CTRL);

	if (strcmp(target->name, "watch_dog_timeout") == 0) {
		tmp = (tmp & CPLD_WATCH_DOG_COUNT_MASK) >> CPLD_WATCH_DOG_COUNT_SHIFT;
		return sprintf(buf, "%u seconds to NMI.\n", wd_cnt_tbl[tmp]);
	}
	else {
		return sprintf(buf, "%d\n", (tmp & (1 << target->bit)) ? 1 : 0);
	}

}

static ssize_t watch_dog_store(struct device * dev,
			       struct device_attribute * dattr,
			       const char * buf, size_t count)
{
	uint8_t tmp;
	int i;
	int bit_val = -1;
	struct watch_dog* target = NULL;

	/* find the target bit */
	for (i = 0; i < n_watch_dog_bits; i++) {
		if (strcmp(dattr->attr.name, watch_dog_bits[i].name) == 0) {
			target = &watch_dog_bits[i];
			break;
		}
	}
	if (target == NULL) {
		return -EINVAL;
	}

	if (sscanf(buf, "%d", &bit_val) <= 0) {
		return -EINVAL;
	}

	if (strcmp(target->name, "watch_dog_timeout") == 0) {
		if ((bit_val < 8) || (bit_val > 512)) {
			return -EINVAL;
		}
		for (i = 0; i < ARRAY_SIZE(wd_cnt_tbl); i++) {
			if (bit_val <= wd_cnt_tbl[i]) {
				break;
			}
		}

		/* set the new value */
		mutex_lock(&accton_as5610_52x_cpld_mutex);

		tmp = cpld_rd(CPLD_REG_WATCH_DOG_CTRL);
		tmp &= ~CPLD_WATCH_DOG_COUNT_MASK;
		tmp |= i << CPLD_WATCH_DOG_COUNT_SHIFT;
		cpld_wr(CPLD_REG_WATCH_DOG_CTRL, tmp);

		mutex_unlock(&accton_as5610_52x_cpld_mutex);
	}
	else {
		if (strcmp(target->name, "watch_dog_keep_alive") == 0) {
			/* always write a one */
			bit_val = 1;
		}
		else {
			/* enable must 0 or 1 */
			if ((bit_val != 0) && (bit_val != 1)) {
				return -EINVAL;
			}
		}

		/* set the new value */
		mutex_lock(&accton_as5610_52x_cpld_mutex);

		tmp = cpld_rd(CPLD_REG_WATCH_DOG_CTRL);
		tmp &= ~(1 << target->bit);
		tmp |= bit_val ? (1 << target->bit) : 0;
		cpld_wr(CPLD_REG_WATCH_DOG_CTRL, tmp);

		mutex_unlock(&accton_as5610_52x_cpld_mutex);
	}

	return count;
}

static SYSFS_ATTR_RW(watch_dog_keep_alive, watch_dog_show, watch_dog_store);
static SYSFS_ATTR_RW(watch_dog_enable,     watch_dog_show, watch_dog_store);
static SYSFS_ATTR_RW(watch_dog_timeout,    watch_dog_show, watch_dog_store);

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
  Concatenation of model type, HW revision and cpld version registers,\n\
  using the following format:\n\
\n\
  model.hw_revision:cpld_version\n\
\n\
  Example: 0.2:2\n\
\n\
  For a 'pre-production/engineering release version' the suffix '-eng'\n\
  is added.\n\
\n\
  Example: 0.2:2-eng\n\
\n\
led_psu1\n\
\n\
  Read-Write:\n\
\n\
  Psu supply 1 LED color.\n\
  The following values are possible: green, yellow, off\n\
\n\
led_psu2\n\
\n\
  Read-Write:\n\
\n\
  Psu supply 2 LED color.\n\
  The following values are possible: green, yellow, off\n\
\n\
led_diag\n\
\n\
  Read-Write:\n\
\n\
  Diag LED color.\n\
  The following values are possible: green, yellow, off\n\
\n\
led_fan\n\
\n\
  Read-Write:\n\
\n\
  System FAN LED color.\n\
  The following values are possible: green, yellow, off\n\
\n\
led_locator\n\
\n\
  Read-Write:\n\
\n\
  Locator LED color.\n\
  The following values are possible: amber_blinking, off\n\
\n\
psu[12]_all_ok\n\
psu[12]_present\n\
psu[12]_dc_ok\n\
\n\
  Read-Only:\n\
\n\
  Hot swap psu supply status info.\n\
  The following values are possible: 1, 0\n\
\n\
  1 means OK.  0 means not OK.\n\
\n\
  The psu[12]_all_ok values are calculated from the other\n\
  psu[12]_* values as follows:\n\
\n\
    psu1_all_ok =\n\
      psu1_present && psu1_dc_ok;\n\
\n\
\n\
system_all_ok\n\
system_fan_present\n\
system_fan_ok\n\
system_fan_present\n\
system_dc_power_ok\n\
\n\
  Read-Only:\n\
\n\
  System fan and power status.\n\
  The following values are possible: 1, 0\n\
\n\
  1 means OK.  0 means not OK.\n\
\n\
  The system_all_ok value is calculated from the other system_*\n\
  values as follows:\n\
\n\
    system_all_ok =\n\
      system_fan_present && system_fan_ok && system_dc_power_ok;\n\
\n\
system_fan_air_flow\n\
\n\
  Read-Only:\n\
\n\
  System fan air flow direction.\n\
  The following values are possible: front-to-back, back-to-front\n\
\n\
watch_dog_enable\n\
\n\
  Read-Write:\n\
\n\
  set/get the watch dog enable bit in the CPLD.\n\
\n\
  0 -- Disable the watch dog.\n\
  1 -- Enable the watch dog feature.\n\
\n\
  If 'watch_dog_keep_alive' is not written within the number of seconds\n\
  specified in 'watch_dog_timeout' a non-maskable interrupt (NMI) is\n\
  triggered.\n\
\n\
  If, after the NMI, an additional timeout period expires then the\n\
  entire board is reset and reboots.\n\
\n\
watch_dog_keep_alive\n\
\n\
  Read-Write:\n\
\n\
  Restarts the watch dog count down timer, marking the system as alive.\n\
\n\
  Write any integer to restart the count down timer.\n\
\n\
watch_dog_timeout\n\
\n\
  Read-Write:\n\
\n\
  The watch dog NMI timeout value in seconds.  See the description of\n\
  'watch_dog_enable' for details.\n\
\n\
  The values in the following table are valid.  Values specified\n\
  outside the table are considered errors.  Values specified between\n\
  valid values are rounded up to the next larger valid value.\n\
\n\
  All times are in seconds:\n\
\n\
     8   64   128   256\n\
    16	 72   136   320\n\
    32	 88   149   448\n\
    48	 96   192   512\n\
\n\
pwm1\n\
\n\
  Read-Write:\n\
\n\
  Set the fan speed. Values range from 0 to 248.\n\
\n\
pwm1_enable\n\
\n\
  Read-Write:\n\
\n\
  Indicates that pwm1 is enabled. Write does nothing, as value is always 1.\n\
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

static struct attribute *accton_as5610_52x_cpld_attrs[] = {
	&dev_attr_board_revision.attr,
	&dev_attr_psu_pwr1_all_ok.attr,
	&dev_attr_psu_pwr1_present.attr,
	&dev_attr_psu_pwr1_dc_ok.attr,
	&dev_attr_psu_pwr2_all_ok.attr,
	&dev_attr_psu_pwr2_present.attr,
	&dev_attr_psu_pwr2_dc_ok.attr,
	&dev_attr_system_all_ok.attr,
	&dev_attr_system_dc_power_ok.attr,
	&dev_attr_system_fan_present.attr,
	&dev_attr_system_fan_ok.attr,
	&dev_attr_system_fan_air_flow.attr,
        &dev_attr_pwm1.attr,
        &dev_attr_pwm1_enable.attr,
	&dev_attr_led_psu1.attr,
	&dev_attr_led_psu2.attr,
	&dev_attr_led_diag.attr,
	&dev_attr_led_fan.attr,
	&dev_attr_led_locator.attr,
	&dev_attr_help.attr,
	&dev_attr_README.attr,
	&dev_attr_watch_dog_keep_alive.attr,
	&dev_attr_watch_dog_enable.attr,
	&dev_attr_watch_dog_timeout.attr,
	NULL,
};

static struct attribute_group accton_as5610_52x_cpld_attr_group = {
	.attrs = accton_as5610_52x_cpld_attrs,
};


/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static int accton_as5610_52x_cpld_setup(void)
{
	// Put some interesting, one-time initializations here.
	return 0;
}

static int accton_as5610_52x_cpld_probe(struct platform_device * ofdev)
{
	int retval = 0;
	struct device_node * np = ofdev->dev.of_node;
	struct kobject * kobj = &ofdev->dev.kobj;

	if (dev_get_drvdata(&ofdev->dev)) {
		dev_info(&ofdev->dev, "already probed\n");
		return 0;
	}

	accton_as5610_52x_cpld_regs = of_iomap(np,0);
	if (!accton_as5610_52x_cpld_regs) {
		return -EIO;
	}

	retval = sysfs_create_group(kobj, &accton_as5610_52x_cpld_attr_group);
	if (retval) {
		return retval;
	}

	if (accton_as5610_52x_cpld_setup()) {
	     return -EIO;
	}

	dev_info(&ofdev->dev, "probed & iomapped @ 0x%p\n", accton_as5610_52x_cpld_setup);

	return 0;
}

static int accton_as5610_52x_cpld_remove(struct platform_device * ofdev)
{
	struct kobject * kobj = &ofdev->dev.kobj;

	/* don't iounmap(regs)... the platform driver uses it for reset	*/
	sysfs_remove_group(kobj, &accton_as5610_52x_cpld_attr_group);

	dev_info(&ofdev->dev, "removed\n");
	return 0;
}

static struct of_device_id accton_as5610_52x_cpld_ids[] = {
	{
		.compatible = "accton,accton_as5610_52x-cpld",
	},
	{ /* end of list */ },
};

static struct platform_driver accton_as5610_52x_cpld_driver = {
	.probe = accton_as5610_52x_cpld_probe,
	.remove = accton_as5610_52x_cpld_remove,
	.driver = {
		.name  = driver_name,
		.owner = THIS_MODULE,
		.of_match_table = accton_as5610_52x_cpld_ids,
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

static int __init accton_as5610_52x_cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&accton_as5610_52x_cpld_driver);
	if (rv) {
		printk(KERN_ERR
		       "%s platform_driver_register failed (%i)\n",
		       driver_name, rv);
	}
	return rv;
}

static void __exit accton_as5610_52x_cpld_exit(void)
{
	return platform_driver_unregister(&accton_as5610_52x_cpld_driver);
}

MODULE_AUTHOR("Puneet Shenoy <puneet@cumulusnetworks.com>");
MODULE_DESCRIPTION("CPLD driver for Accton Technology Corporation, AS5610_52X");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(accton_as5610_52x_cpld_init);
module_exit(accton_as5610_52x_cpld_exit);
