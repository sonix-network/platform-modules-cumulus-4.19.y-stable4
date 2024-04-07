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
#include <linux/err.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/watchdog.h>

#include <asm/io.h>

#include "platform_defs.h"
#include "dni_7448_cpld.h"

static const char driver_name[] = "dni_7448_cpld";
#define DRIVER_VERSION "1.0"
static struct device *cpld_hwmon_dev;

static bool dni7448_wdt_enabled;
module_param(dni7448_wdt_enabled, bool, 0);
dni7448_wdt_period = DNI7449_CPLD_WATCHDOG_MAX;
module_param(dni7448_wdt_period, int, 0);

/*------------------------------------------------------------------------------
 *
 * Driver resident static variables
 *
 */

static DEFINE_MUTEX(dni7448_cpld_mutex);

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
	uint8_t crev;
	uint8_t * regs;

	mutex_lock(&dni7448_cpld_mutex);

	regs = dev_get_drvdata(dev);

	tmp = readb(regs + DNI7448_CPLD_BOARD_REV_OFFSET);
	type = (tmp & DNI7448_CPLD_BOARD_TYPE_MASK) >> DNI7448_CPLD_BOARD_TYPE_SHIFT;
	rev  = (tmp & DNI7448_CPLD_BOARD_REV_MASK)  >> DNI7448_CPLD_BOARD_REV_SHIFT;

	crev = readb(regs + DNI7448_CPLD_REV_OFFSET);

	mutex_unlock(&dni7448_cpld_mutex);

	return sprintf(buf, "%d.%d:%d\n", type, rev, crev);
}
static SYSFS_ATTR_RO(board_revision, board_revision_show);

static ssize_t name_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
    return sprintf(buf, "%s\n", driver_name);
}
static SYSFS_ATTR_RO(name, name_show);

/*
 * bulk power supply status
 */
static ssize_t bulk_power_show(struct device * dev,
			       struct device_attribute * dattr,
			       char * buf)
{
	uint8_t tmp;
	uint8_t mask;
	uint8_t present_l;
	uint8_t pwr_ok_l;
	uint8_t fan_ok;
	uint8_t temp_ok;
	uint8_t bad = 0;
	uint8_t * regs;

	mutex_lock(&dni7448_cpld_mutex);

	regs = dev_get_drvdata(dev);
	tmp = readb(regs + DNI7448_CPLD_BULK_PS_OFFSET);

	mutex_unlock(&dni7448_cpld_mutex);

	if (strcmp(dattr->attr.name, xstr(PLATFORM_PS_NAME_0)) == 0) {
		mask      = DNI7448_CPLD_BULK_PS_0_MASK;
		present_l = DNI7448_CPLD_BULK_PS_0_PRESENT_L;
		pwr_ok_l  = DNI7448_CPLD_BULK_PS_0_OK_L;
		fan_ok	  = DNI7448_CPLD_BULK_PS_0_TEMP_OK;
		temp_ok   = DNI7448_CPLD_BULK_PS_0_FAN_OK;
	} else {
		mask      = DNI7448_CPLD_BULK_PS_1_MASK;
		present_l = DNI7448_CPLD_BULK_PS_1_PRESENT_L;
		pwr_ok_l  = DNI7448_CPLD_BULK_PS_1_OK_L;
		fan_ok	  = DNI7448_CPLD_BULK_PS_1_TEMP_OK;
		temp_ok   = DNI7448_CPLD_BULK_PS_1_FAN_OK;
	}

	tmp &= mask;

	if ( ~tmp & present_l) {
		sprintf(buf, PLATFORM_INSTALLED);
		if (!(~tmp & pwr_ok_l)) {
			bad++;
			strcat( buf, ", "PLATFORM_PS_POWER_BAD);
		}
		if (!(tmp & fan_ok)) {
			bad++;
			strcat( buf, ", "PLATFORM_PS_FAN_BAD);
		}
		if (!(tmp & temp_ok)) {
			bad++;
			strcat( buf, ", "PLATFORM_PS_TEMP_BAD);
		}
		if ( bad == 0) {
			strcat( buf, ", "PLATFORM_OK);
		}
	}
	else {
		// Not present
		sprintf(buf, PLATFORM_NOT_INSTALLED);
	}

	strcat( buf, "\n");

	return strlen(buf);

}
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_0, bulk_power_show);
static SYSFS_ATTR_RO(PLATFORM_PS_NAME_1, bulk_power_show);


/*
 * board power status (kind of silly... nothing would work if this was bad)
 */
static ssize_t board_power_show(struct device * dev,
				struct device_attribute * dattr,
				char * buf)
{
	uint8_t tmp;
	uint8_t * regs;

	mutex_lock(&dni7448_cpld_mutex);

	regs = dev_get_drvdata(dev);
	tmp = readb(regs + DNI7448_CPLD_BOARD_PWR_STATUS_OFFSET);

	mutex_unlock(&dni7448_cpld_mutex);

	if (tmp == (( DNI7448_CPLD_VCC_3V3_GOOD
		    | DNI7448_CPLD_NET_SOC_1V_GOOD
		    | DNI7448_CPLD_DDR_1V5_GOOD
		    | DNI7448_CPLD_PHY_1V_1to24_GOOD
		    | DNI7448_CPLD_PHY_1V_25to52_GOOD)
		    & ~DNI7448_CPLD_CPU_1V_GOOD_L
		    & ~DNI7448_CPLD_PHY_1V5_GOOD_L)) {
		return sprintf(buf, PLATFORM_OK "\n");
	} else {
		return sprintf(buf, "bad - 0x%02x\n", tmp);
	}
}
static SYSFS_ATTR_RO(board_power, board_power_show);

/*
 * fan status
 */
static ssize_t fan_show(struct device * dev,
			struct device_attribute * dattr,
			char * buf)
{
	uint8_t tmp;
	uint8_t mask;
	uint8_t present_l;
	uint8_t fan_0_ok;
	uint8_t fan_1_ok;
	uint8_t * regs;

	mutex_lock(&dni7448_cpld_mutex);

	regs = dev_get_drvdata(dev);
	tmp = readb(regs + DNI7448_CPLD_FAN_STATUS_OFFSET);

	mutex_unlock(&dni7448_cpld_mutex);

	if (strcmp(dattr->attr.name, "fan_0") == 0) {
		mask      = DNI7448_CPLD_FAN_0_MASK;
		present_l = DNI7448_CPLD_FAN_0_PRESENT_L;
		fan_0_ok  = DNI7448_CPLD_FAN_0_A_OK;
		fan_1_ok  = DNI7448_CPLD_FAN_0_B_OK;
	} else {
		mask      = DNI7448_CPLD_FAN_1_MASK;
		present_l = DNI7448_CPLD_FAN_1_PRESENT_L;
		fan_0_ok  = DNI7448_CPLD_FAN_1_A_OK;
		fan_1_ok  = DNI7448_CPLD_FAN_1_B_OK;
	}

	tmp &= mask;

	if (tmp & present_l) {
		return sprintf(buf, PLATFORM_NOT_INSTALLED "\n");
	} else if ((!(tmp & fan_0_ok)) |
		   (!(tmp & fan_1_ok))) {
		return sprintf(buf, PLATFORM_FAN_NOT_SPINNING "\n");
	} else {
		return sprintf(buf, PLATFORM_OK "\n");
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
	char name[DNI7448_CPLD_STRING_NAME_SIZE];
	unsigned int offset;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_fan",
		.offset = DNI7448_CPLD_SYSTEM_LED_OFFSET,
		.mask = DNI7448_CPLD_FAN_LED,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, DNI7448_CPLD_FAN_LED_GREEN },
			{ PLATFORM_LED_YELLOW, DNI7448_CPLD_FAN_LED_YELLOW },
			{ PLATFORM_LED_OFF, DNI7448_CPLD_FAN_LED_OFF },
		},
	},
	{
		.name = "led_power",
		.offset = DNI7448_CPLD_SYSTEM_LED_OFFSET,
		.mask = DNI7448_CPLD_POWER_LED,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN, DNI7448_CPLD_POWER_LED_GREEN },
			{ PLATFORM_LED_YELLOW, DNI7448_CPLD_POWER_LED_YELLOW },
			{ PLATFORM_LED_YELLOW_BLINKING, DNI7448_CPLD_POWER_LED_YELLOW_BLINK },
			{ PLATFORM_LED_OFF, DNI7448_CPLD_POWER_LED_OFF },
		},
	},
	{
		.name = "led_master",
		.offset = DNI7448_CPLD_SYSTEM_LED_OFFSET,
		.mask = DNI7448_CPLD_MASTER_LED,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN, DNI7448_CPLD_MASTER_LED_GREEN },
			{ PLATFORM_LED_OFF, DNI7448_CPLD_MASTER_LED_OFF },
		},
	},
	{
		.name = "led_status",
		.offset = DNI7448_CPLD_SYSTEM_LED_OFFSET,
		.mask = DNI7448_CPLD_STATUS_LED,
		.n_colors = 5,
		.colors = {
			{ PLATFORM_LED_GREEN, DNI7448_CPLD_STATUS_LED_GREEN },
			{ PLATFORM_LED_GREEN_BLINKING, DNI7448_CPLD_STATUS_LED_GREEN_BLINK },
			{ PLATFORM_LED_RED, DNI7448_CPLD_STATUS_LED_RED },
			{ PLATFORM_LED_RED_BLINKING, DNI7448_CPLD_STATUS_LED_RED_BLINK },
			{ PLATFORM_LED_OFF, DNI7448_CPLD_STATUS_LED_OFF },
		},
	},
	{
		.name = "led_fan_tray_0",
		.offset = DNI7448_CPLD_FAN_TRAY_LED_CTRL_OFFSET,
		.mask = DNI7448_CPLD_FAN_TRAY_0_LED,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, DNI7448_CPLD_FAN_TRAY_0_LED_GREEN },
			{ PLATFORM_LED_RED, DNI7448_CPLD_FAN_TRAY_0_LED_RED },
			{ PLATFORM_LED_OFF, DNI7448_CPLD_FAN_TRAY_0_LED_OFF },
		},
	},
	{
		.name = "led_fan_tray_1",
		.offset = DNI7448_CPLD_FAN_TRAY_LED_CTRL_OFFSET,
		.mask = DNI7448_CPLD_FAN_TRAY_1_LED,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_GREEN, DNI7448_CPLD_FAN_TRAY_1_LED_GREEN },
			{ PLATFORM_LED_RED, DNI7448_CPLD_FAN_TRAY_1_LED_RED },
			{ PLATFORM_LED_OFF, DNI7448_CPLD_FAN_TRAY_1_LED_OFF },
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
	uint8_t * regs;

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
	mutex_lock(&dni7448_cpld_mutex);

	regs = dev_get_drvdata(dev);
	tmp = readb(regs + target->offset);

	mutex_unlock(&dni7448_cpld_mutex);

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
	uint8_t * regs;

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

	mutex_lock(&dni7448_cpld_mutex);

	regs = dev_get_drvdata(dev);

	tmp = readb(regs + target->offset);
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;

	writeb(tmp, (regs + target->offset));

	mutex_unlock(&dni7448_cpld_mutex);

	return count;
}

static SYSFS_ATTR_RW(led_fan, led_show, led_store);
static SYSFS_ATTR_RW(led_power, led_show, led_store);
static SYSFS_ATTR_RW(led_master, led_show, led_store);
static SYSFS_ATTR_RW(led_status, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_0, led_show, led_store);
static SYSFS_ATTR_RW(led_fan_tray_1, led_show, led_store);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */

static struct attribute *dni7448_cpld_attrs[] = {
	&dev_attr_name.attr,
	&dev_attr_board_revision.attr,
	&dev_attr_psu_pwr1.attr,
	&dev_attr_psu_pwr2.attr,
	&dev_attr_board_power.attr,
	&dev_attr_fan_0.attr,
	&dev_attr_fan_1.attr,
	&dev_attr_led_fan.attr,
	&dev_attr_led_power.attr,
	&dev_attr_led_master.attr,
	&dev_attr_led_status.attr,
	&dev_attr_led_fan_tray_0.attr,
	&dev_attr_led_fan_tray_1.attr,
	NULL,
};

static struct attribute_group dni7448_cpld_attr_group = {
	.attrs = dni7448_cpld_attrs,
};

/*------------------------------------------------------------------------------
 *
 * Watchdog driver implementation
 *
 */

struct dni7449_wdt_private_data {
	struct platform_device *ofdev;  /* platform device object */
	uint8_t *regs;                  /* CPLD register access */
	uint8_t  wd_ctrl;               /* Watchdog control register value */
};

/*
 * The DNI-7448 CPLD only supports discrete watchdog periods.  This
 * routine maps the hardware register values to human readable values.
 */
static inline uint8_t period_to_sec(unsigned int period)
{
	uint8_t sec = 0;

	switch (period) {
	case DNI7448_CPLD_WATCHDOG_15_SEC:
		sec = 15;
		break;
	case DNI7448_CPLD_WATCHDOG_20_SEC:
		sec = 20;
		break;
	case DNI7448_CPLD_WATCHDOG_30_SEC:
		sec = 30;
		break;
	case DNI7448_CPLD_WATCHDOG_40_SEC:
		sec = 40;
		break;
	case DNI7448_CPLD_WATCHDOG_50_SEC:
		sec = 50;
		break;
	case DNI7448_CPLD_WATCHDOG_60_SEC:
	default:
		sec = 60;
		break;
	}

	return sec;
}

/*
 * The DNI-7448 CPLD only supports discrete watchdog periods.  Map the
 * requested timeout to an equal or greater timeout value supported by
 * the hardware.
 */
static inline uint8_t sec_to_period(unsigned int secs)
{
	uint8_t period;

	if (secs < 16)
		period = DNI7448_CPLD_WATCHDOG_15_SEC;
	else if (secs < 21)
		period = DNI7448_CPLD_WATCHDOG_20_SEC;
	else if (secs < 31)
		period = DNI7448_CPLD_WATCHDOG_30_SEC;
	else if (secs < 41)
		period = DNI7448_CPLD_WATCHDOG_40_SEC;
	else if (secs < 51)
		period = DNI7448_CPLD_WATCHDOG_50_SEC;
	else
		period = DNI7448_CPLD_WATCHDOG_60_SEC;

	return period;
}

static int dni7448_wdt_ping(struct watchdog_device *wdog)
{
	struct dni7449_wdt_private_data *wdt = (struct dni7449_wdt_private_data *)watchdog_get_drvdata(wdog);

	writeb(wdt->wd_ctrl, wdt->regs + DNI7448_CPLD_WATCHDOG_OFFSET);

	return 0;
}

static int dni7448_wdt_start(struct watchdog_device *wdog)
{
	struct dni7449_wdt_private_data *wdt = (struct dni7449_wdt_private_data *)watchdog_get_drvdata(wdog);

	dev_notice(&wdt->ofdev->dev,
		   "watchdog enabled (requested timeout: %u sec, actual: %u sec)\n",
		   wdog->timeout, period_to_sec(sec_to_period(wdog->timeout)));

	wdt->wd_ctrl |= DNI7448_CPLD_WATCHDOG_EN;
	writeb(wdt->wd_ctrl, wdt->regs + DNI7448_CPLD_WATCHDOG_OFFSET);

	return 0;
}

static int dni7448_wdt_stop(struct watchdog_device *wdog)
{
	struct dni7449_wdt_private_data *wdt = (struct dni7449_wdt_private_data *)watchdog_get_drvdata(wdog);

	wdt->wd_ctrl &= ~DNI7448_CPLD_WATCHDOG_EN;
	writeb(wdt->wd_ctrl, wdt->regs + DNI7448_CPLD_WATCHDOG_OFFSET);

	return 0;
}

static int dni7448_wdt_set_timeout(struct watchdog_device *wdog,
				   unsigned int timeout)
{
	struct dni7449_wdt_private_data *wdt = (struct dni7449_wdt_private_data *)watchdog_get_drvdata(wdog);
	wdog->timeout = timeout;
	dev_notice(&wdt->ofdev->dev,
		   "watchdog requested timeout: %u sec, actual: %u sec.\n",
		   wdog->timeout, period_to_sec(sec_to_period(wdog->timeout)));

	wdt->wd_ctrl &= ~DNI7448_CPLD_WATCHDOG_TIME_MASK;
	wdt->wd_ctrl |= sec_to_period(wdog->timeout);
	writeb(wdt->wd_ctrl, wdt->regs + DNI7448_CPLD_WATCHDOG_OFFSET);

	return 0;
}

static struct watchdog_info dni7448_wdt_info = {
	.options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE,
	.identity = "DNI-7448 Watchdog",
};

static struct watchdog_ops dni7448_wdt_ops = {
	.owner = THIS_MODULE,
	.start = dni7448_wdt_start,
	.stop = dni7448_wdt_stop,
	.ping = dni7448_wdt_ping,
	.set_timeout = dni7448_wdt_set_timeout,
};

static struct watchdog_device dni7448_wdt_dev = {
	.info = &dni7448_wdt_info,
	.ops = &dni7448_wdt_ops,
	.min_timeout = DNI7449_CPLD_WATCHDOG_MIN,
	.max_timeout = DNI7449_CPLD_WATCHDOG_MAX
};

static struct dni7449_wdt_private_data dni7449_wdt_private;

static int dni7448_wdt_init(struct platform_device *ofdev, uint8_t* regs)
{
	bool nowayout = WATCHDOG_NOWAYOUT;

	dni7448_wdt_info.firmware_version = readb(regs + DNI7448_CPLD_REV_OFFSET);
	dni7449_wdt_private.ofdev = ofdev;
	dni7449_wdt_private.regs = regs;
	dni7449_wdt_private.wd_ctrl = readb(regs + DNI7448_CPLD_WATCHDOG_OFFSET) & ~DNI7448_CPLD_WATCHDOG_KICK_L;
	watchdog_set_drvdata(&dni7448_wdt_dev, (void*)&dni7449_wdt_private);
	dni7448_wdt_set_timeout(&dni7448_wdt_dev, dni7448_wdt_period);
	dni7448_wdt_dev.timeout = dni7448_wdt_period;
	watchdog_set_nowayout(&dni7448_wdt_dev, nowayout);
	if (dni7448_wdt_enabled)
		dni7448_wdt_start(&dni7448_wdt_dev);

	return watchdog_register_device(&dni7448_wdt_dev);
}

static void dni7448_wdt_remove()
{
	dni7448_wdt_stop(&dni7448_wdt_dev);
	watchdog_unregister_device(&dni7448_wdt_dev);
}

/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static int dni7448_cpld_setup(uint8_t* regs)
{
	unsigned int offset;
	uint8_t tmp;

	/* enable Trident to drive LEDs */
	offset = DNI7448_CPLD_FAN_TRAY_LED_CTRL_OFFSET;
	tmp =  readb(regs + offset);
	tmp |= DNI7448_CPLD_PORT_LED_EN;
	writeb(tmp, (regs + offset));

	return 0;
}

static int dni7448_cpld_probe(struct platform_device * ofdev)
{
	int retval = 0;
	int err;
	struct device_node * np = ofdev->dev.of_node;
	struct kobject * kobj = &ofdev->dev.kobj;
	uint8_t * regs;

	if (dev_get_drvdata(&ofdev->dev)) {
		dev_info(&ofdev->dev, "already probed\n");
		regs = dev_get_drvdata(&ofdev->dev);
	}
	else {
		regs = of_iomap(np,0);
		if (!regs) {
			dev_err(&ofdev->dev, "I/O Mapping CPLD registers failed\n");
			return -EIO;
		}
	}

	retval = sysfs_create_group(kobj, &dni7448_cpld_attr_group);
	if (retval) {
		dev_err(&ofdev->dev, "Creating sysfs group failed\n");
		return retval;
	}

	if (dni7448_cpld_setup(regs)) {
	     return -EIO;
	}
	cpld_hwmon_dev = hwmon_device_register(&ofdev->dev);
	if (IS_ERR(cpld_hwmon_dev)) {
		err = PTR_ERR(cpld_hwmon_dev);
		dev_err(&ofdev->dev, "hwmon registration failed\n");
	}

	dev_set_drvdata(&ofdev->dev, regs);
	dev_info(&ofdev->dev, "probed & iomapped @ %p\n", regs);

	retval = dni7448_wdt_init(ofdev, regs);
	if (retval) {
		dev_err(&ofdev->dev, "Watchdog driver registration failed\n");
		return retval;
	}

	return retval;
}

static int dni7448_cpld_remove(struct platform_device * ofdev)
{
	struct kobject * kobj = &ofdev->dev.kobj;

	dni7448_wdt_remove();
	hwmon_device_unregister(cpld_hwmon_dev);
	/* don't iounmap(regs)... the platform driver uses it for reset	*/
	sysfs_remove_group(kobj, &dni7448_cpld_attr_group);

	dev_info(&ofdev->dev, "removed\n");
	return 0;
}

static struct of_device_id dni7448_cpld_ids[] = {
	{
		.compatible = "dni,7448-cpld",
	},
	{ /* end of list */ },
};

static struct platform_driver dni7448_cpld_driver = {
	.probe = dni7448_cpld_probe,
	.remove = dni7448_cpld_remove,
	.driver = {
		.name  = driver_name,
		.owner = THIS_MODULE,
		.of_match_table = dni7448_cpld_ids,
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

static int __init dni7448_cpld_init(void)
{
	int rv;

	rv = platform_driver_register(&dni7448_cpld_driver);
	if (rv) {
		printk(KERN_ERR
		       "%s platform_driver_register failed (%i)\n",
		       driver_name, rv);
	}
	return rv;
}

static void __exit dni7448_cpld_exit(void)
{
	return platform_driver_unregister(&dni7448_cpld_driver);
}

MODULE_AUTHOR("JR Rivers <jrrivers@cumulusnetworks.com>");
MODULE_DESCRIPTION("CPLD driver for Delta Networks Inc. ET7448");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(dni7448_cpld_init);
module_exit(dni7448_cpld_exit);
