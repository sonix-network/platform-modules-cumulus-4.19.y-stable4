/*
 * CPLD sysfs driver for cel_pebble.
 *
 * Copyright (C) 2015 Cumulus Networks, Inc.
 * Author: Vidya Ravipati <vidya@cumulusnetworks.com>
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
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <asm/io.h>

#include "cel-pebble-cpld.h"
#include "platform-defs.h"

static const char driver_name[] = "cel_pebble_cpld";
#define DRIVER_VERSION "1.0"

#define CPLDNAMSZ 20

static uint8_t* cel_pebble_cpld_regs;

/********************************************************************************
 *
 * CPLD I/O
 *
 */
static uint8_t cpld_reg_read(uint32_t reg)
{
    uint8_t data;

    data = ioread8(cel_pebble_cpld_regs + reg - CPLD_IO_BASE);
    return data;
}

static void cpld_reg_write(uint32_t reg, uint8_t data)
{
    iowrite8(data, cel_pebble_cpld_regs + reg - CPLD_IO_BASE);
}

#define cel_pebble_CPLD_STRING_NAME_SIZE 30

/*
 * mmc write protect
 */
static ssize_t flash_write_protect_show(struct device * dev,
                 struct device_attribute * dattr,
                 char * buf)
{
    uint8_t data;

    data = cpld_reg_read(CPLD_MMC_FLASH_WP_REG);
    return sprintf(buf, "0x%02X\n", data);
}

static ssize_t flash_write_protect_store(struct device * dev,
                   struct device_attribute * dattr,
                   const char * buf, size_t count)
{
    int tmp;

    if (sscanf(buf, "%x", &tmp) != 1) {
        return -EINVAL;
    }
    cpld_reg_write(CPLD_MMC_FLASH_WP_REG, tmp);

    return count;
}
static SYSFS_ATTR_RW(flash_write_protect, flash_write_protect_show, flash_write_protect_store);

/*
 * MMC (Module Management) cpld version
 */
static ssize_t cpld_mmc_version_show(struct device * dev,
                 struct device_attribute * dattr,
                 char * buf)
{
    uint8_t tmp;
    uint8_t type;
    uint8_t rev;

    tmp = cpld_reg_read(CPLD_MMC_VERSION_REG);
    type = (tmp & CPLD_MMC_VERSION_H_MASK) >> CPLD_MMC_VERSION_H_SHIFT;
    rev  = (tmp & CPLD_MMC_VERSION_L_MASK) >> CPLD_MMC_VERSION_L_SHIFT;

    return sprintf(buf, "%d.%d\n", type, rev);
}
static SYSFS_ATTR_RO(cpld_mmc_version, cpld_mmc_version_show);

/*
 * Fan speed read in PWM
 */
static ssize_t fan_tach_show(struct device * dev,
                 struct device_attribute * dattr,
                 char * buf)
{
    int fan = 0;
    uint32_t reg;
    s32 speed = 0;
    int rpm;

    if ((sscanf(dattr->attr.name, "fan%d_", &fan) < 1) || (fan < 1) || (fan > 10))
        return -EINVAL;

    if (fan & 0x1) {
        reg = CPLD_MMC_FAN1_TACH_REG;
    } else {
        reg = CPLD_MMC_FAN2_TACH_REG;
    }

    speed = cpld_reg_read(reg);
    if ((speed < 0) || (speed > 255))
        return speed;

    if (speed == 0) {
        /* To catch errors in sysfs node */
        rpm = speed;
    } else {
        rpm =(1000000)/(speed);   //15us*4  for 255 PWM
    }
    return sprintf(buf, "%d\n", rpm);
}

static SYSFS_ATTR_RO(fan1_input, fan_tach_show);
static SYSFS_ATTR_RO(fan2_input, fan_tach_show);

static ssize_t fan_pwm_input_show(struct device * dev,
                 struct device_attribute * dattr,
                 char * buf)
{
    int fan = 0;
    uint32_t reg;
    s32 val;
    int pwm;

    if ((sscanf(dattr->attr.name, "pwm%d", &fan) < 1) || (fan < 1) || (fan > 2))
        return -EINVAL;

    if (fan & 0x1) {
        reg = CPLD_MMC_PWM1_CTRL_REG;
    } else {
        reg = CPLD_MMC_PWM2_CTRL_REG;
    }

    val = cpld_reg_read(reg);
    if (val < 0)
        return val;

    pwm = (uint8_t)val;

    return sprintf(buf, "%d\n", pwm);
}

static ssize_t fan_pwm_input_store(struct device * dev,
                   struct device_attribute * dattr,
                   const char * buf, size_t count)
{
    int fan = 0;
    uint32_t reg;
    int tmp;

    if ((sscanf(buf, "%d", &tmp) != 1) || (tmp < 0) || (tmp > 255)) {
        return -EINVAL;
    }

    if ((sscanf(dattr->attr.name, "pwm%d", &fan) < 1) || (fan < 1) || (fan > 2))
        return -EINVAL;

    if (fan & 0x1) {
        reg = CPLD_MMC_PWM1_CTRL_REG;
    } else {
        reg = CPLD_MMC_PWM2_CTRL_REG;
    }

    cpld_reg_write(reg, tmp);

    return count;
}
static SYSFS_ATTR_RW(pwm1, fan_pwm_input_show, fan_pwm_input_store);
static SYSFS_ATTR_RW(pwm2, fan_pwm_input_show, fan_pwm_input_store);

/* pwm_en is for implementing the hwmon interface.
 *
 * These routines actually do nothing.
 */
static ssize_t pwm1_enable_show(struct device *dev,
                struct device_attribute *dattr,
                char *buf)
{
    return sprintf(buf, "1\n");
}

static ssize_t pwm1_enable_store(struct device *dev,
                 struct device_attribute *dattr,
                 const char *buf, size_t count)
{
    if (count < 1) {
        return -EINVAL;
    }

    if (strcmp(buf, "1\n") != 0 && strcmp(buf, "1") != 0) {
        return -EINVAL;
    }

    return count;
}
static SYSFS_ATTR_RW(pwm1_enable, pwm1_enable_show, pwm1_enable_store);
static SYSFS_ATTR_RW(pwm2_enable, pwm1_enable_show, pwm1_enable_store);

/*------------------------------------------------------------------------------
 *
 * LED, PSU definitions
 *
 */
#define PLATFORM_DEV_STATE_SIZE 20
struct dev_state_info {
    char name[PLATFORM_DEV_STATE_SIZE];
    uint8_t value;
};

struct dev_state {
    char name[cel_pebble_CPLD_STRING_NAME_SIZE];
    uint32_t reg;
    uint8_t mask;
    uint8_t shift;
    int n_state;
    struct dev_state_info state[8];
};

static struct dev_state cpld_devs[] = {
    {
        .name = "led_system_status",
        .reg  = CPLD_MMC_SYS_LED_REG,
        .mask = CPLD_MMC_SYS_LED_MASK,
        .shift = CPLD_MMC_SYS_LED_SHIFT,
        .n_state = 4,
        .state = {
            { PLATFORM_LED_OFF, CPLD_MMC_SYS_LED_OFF },
            { PLATFORM_LED_GREEN, CPLD_MMC_SYS_LED_GREEN },
            { PLATFORM_LED_AMBER_BLINKING, CPLD_MMC_SYS_LED_AMBER_BLINK },
            { PLATFORM_LED_AMBER, CPLD_MMC_SYS_LED_AMBER },
        },
    },
};
static int n_devs = ARRAY_SIZE(cpld_devs);

/*
 * LEDs
 */
static ssize_t dev_show(struct device * dev,
            struct device_attribute * dattr,
            char * buf)
{
    uint8_t tmp;
    int i;
    struct dev_state * target = NULL;

    /* find the target led */
    for (i = 0; i < n_devs; i++) {
        if (strcmp(dattr->attr.name, cpld_devs[i].name) == 0) {
            target = &cpld_devs[i];
            break;
        }
    }
    if (target == NULL) {
        return sprintf(buf, "undefined target\n");
    }

    /* read the register */
    tmp = cpld_reg_read(target->reg);

    /* find the state */
    tmp &= target->mask;
    tmp >>= target->shift;
    for (i = 0; i < target->n_state; i++) {
        if (tmp == target->state[i].value) {
            break;
        }
    }
    if (i == target->n_state) {
        return sprintf(buf, "undefined\n");
    } else {
        return sprintf(buf, "%s\n", target->state[i].name);
    }
}

static ssize_t dev_store(struct device * dev,
             struct device_attribute * dattr,
             const char * buf, size_t count)
{
    uint8_t tmp;
    int i;
    struct dev_state * target = NULL;
    char raw[PLATFORM_DEV_STATE_SIZE];

    /* find the target dev */
    for (i = 0; i < n_devs; i++) {
        if (strcmp(dattr->attr.name, cpld_devs[i].name) == 0) {
            target = &cpld_devs[i];
            break;
        }
    }
    if (target == NULL) {
        return -EINVAL;
    }

    /* find the state */
    if (sscanf(buf, "%19s", raw) <= 0) {
        return -EINVAL;
    }
    for (i = 0; i < target->n_state; i++) {
        if (strcmp(raw, target->state[i].name) == 0) {
            break;
        }
    }
    if (i == target->n_state) {
        return -EINVAL;
    }

    tmp = cpld_reg_read(target->reg);
    tmp &= ~target->mask;
    tmp |= (target->state[i].value << target->shift) & target->mask;
    cpld_reg_write(target->reg, tmp);

    return count;
}
static SYSFS_ATTR_RW(led_system_status, dev_show, dev_store);

int hexToInt(const char *hexStr, uint32_t *valPtr)
{
    char prefix[] = "0x";
    if (strncmp(hexStr, prefix, strlen(prefix)) == 0) {
        hexStr += strlen(prefix);
    }
    return sscanf(hexStr, "%x", valPtr) != 1;
}


/*
 * Help / README
 *
 * The string length must be less than 4K.
 *
 */
#define HELP_STR "Description of the CPLD driver files:\n\
\n\
\n\
cpld_mmc_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for MMC CPLD\n\
  CPLD version register in the following format:\n\
\n\
  cpld_mmc_version\n\
\n\
  Example: 0.2\n\
\n\
\n\
led_system_status\n\
\n\
  Read-Write:\n\
\n\
  System Status LED color.\n\
  The following values are possible: green, blinking green and off\n\
\n\
\n\
pwm1\n\
pwm2\n\
\n\
  Read-Write:\n\
\n\
  PWM Fan Speed Control for fans i.e. fan1 and fan2\n\
  Possible PWM fan speed value is in between 0-255\n\
\n\
\n\
pwm1_enable\n\
pwm2_enable\n\
\n\
  Read-Write:\n\
\n\
  PWM Enable field is required for hwmon which is included for fans i.e. fan1 and fan2\n\
  Possible PWM enable value is in between 0-1\n\
\n\
\n\
fan1_input\n\
fan2_input\n\
\n\
  Read-Only:\n\
n\
  Speed of the fan is in RPM\n\
\n\
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

static struct attribute *cel_pebble_cpld_attrs[] = {
    &dev_attr_cpld_mmc_version.attr,
    &dev_attr_led_system_status.attr,
    &dev_attr_flash_write_protect.attr,
    &dev_attr_pwm1_enable.attr,
    &dev_attr_pwm2_enable.attr,
    &dev_attr_pwm1.attr,
    &dev_attr_pwm2.attr,
    &dev_attr_fan1_input.attr,
    &dev_attr_fan2_input.attr,
    &dev_attr_help.attr,
    &dev_attr_README.attr,
    NULL,
};

static struct attribute_group cel_pebble_cpld_attr_group = {
    .attrs = cel_pebble_cpld_attrs,
};

/*------------------------------------------------------------------------------
 *
 * module interface
 *
 */
static struct platform_device *cel_pebble_cpld_device;

static int cel_pebble_cpld_probe(struct platform_device *dev)
{
    int ret;

    cel_pebble_cpld_regs = ioport_map(CPLD_IO_BASE, CPLD_IO_SIZE);
    if (!cel_pebble_cpld_regs) {
        pr_err("cpld: unabled to map iomem\n");
        ret = -ENODEV;
        goto err_exit;
    }

    ret = sysfs_create_group(&dev->dev.kobj, &cel_pebble_cpld_attr_group);
    if (ret) {
        pr_err("cpld: sysfs_create_group failed for cpld driver");
        goto err_unmap;
    }

err_unmap:
    iounmap(cel_pebble_cpld_regs);

err_exit:
    return ret;
}

static int cel_pebble_cpld_remove(struct platform_device *dev)
{
    iounmap(cel_pebble_cpld_regs);
    return 0;
}

static struct platform_driver cel_pebble_cpld_driver = {
    .driver = {
        .name = "cel_pebble_cpld",
        .owner = THIS_MODULE,
    },
    .probe = cel_pebble_cpld_probe,
    .remove = cel_pebble_cpld_remove,
};

static int __init cel_pebble_cpld_init(void)
{
    int rv;

    rv = platform_driver_register(&cel_pebble_cpld_driver);
    if (rv) {
        goto err_exit;
    }

    cel_pebble_cpld_device = platform_device_alloc("cel_pebble_cpld", 0);
    if (!cel_pebble_cpld_device) {
        pr_err("platform_device_alloc() failed for cpld device\n");
        rv = -ENOMEM;
        goto err_unregister;
    }

    rv = platform_device_add(cel_pebble_cpld_device);
    if (rv) {
        pr_err("platform_device_add() failed for cpld device.\n");
        goto err_dealloc;
    }
    return 0;

err_dealloc:
    platform_device_unregister(cel_pebble_cpld_device);

err_unregister:
    platform_driver_unregister(&cel_pebble_cpld_driver);

err_exit:
    pr_err("%s platform_driver_register failed (%i)\n",
            driver_name, rv);
    return rv;
}

static void __exit cel_pebble_cpld_exit(void)
{
    platform_driver_unregister(&cel_pebble_cpld_driver);
    platform_device_unregister(cel_pebble_cpld_device);
}

MODULE_AUTHOR("Vidya Ravipati <vidya@cumulusnetworks.com>");
MODULE_DESCRIPTION("Platform CPLD driver for Celestica Pebble");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(cel_pebble_cpld_init);
module_exit(cel_pebble_cpld_exit);
