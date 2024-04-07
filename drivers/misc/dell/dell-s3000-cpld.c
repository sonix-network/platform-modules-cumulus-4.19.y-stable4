/*
 * CPLD sysfs driver for dell_s3000.
 *
 * Copyright (C) 2015 Cumulus Networks, Inc.
 * Author: Samer Nubani <samer@cumulusnetworks.com>
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

#include "dell-s3000-cpld.h"
#include "platform-defs.h"

static const char driver_name[] = "dell_s3000_cpld";
#define DRIVER_VERSION "1.0"

#define CPLDNAMSZ 20

static uint8_t* dell_s3000_cpld_regs;

/********************************************************************************
 *
 * CPLD I/O
 *
 */
static uint8_t cpld_reg_read(uint32_t reg)
{
    uint8_t data;

    data = ioread8(dell_s3000_cpld_regs + reg - CPLD_IO_BASE);
    return data;
}

static void cpld_reg_write(uint32_t reg, uint8_t data)
{
    iowrite8(data, dell_s3000_cpld_regs + reg - CPLD_IO_BASE);
}

#define dell_s3000_CPLD_STRING_NAME_SIZE 30

/*
 * mmc write protect
 */
static ssize_t mmc_write_protect_show(struct device * dev,
                 struct device_attribute * dattr,
                 char * buf)
{
    uint8_t data;

    data = cpld_reg_read(CPLD_MMC_EEPROM_WP_REG);
    return sprintf(buf, "0x%02X\n", data);
}

static ssize_t mmc_write_protect_store(struct device * dev,
                   struct device_attribute * dattr,
                   const char * buf, size_t count)
{
    int tmp;

    if (sscanf(buf, "%x", &tmp) != 1) {
        return -EINVAL;
    }
    cpld_reg_write(CPLD_MMC_EEPROM_WP_REG, tmp);

    return count;
}
static SYSFS_ATTR_RW(mmc_write_protect, mmc_write_protect_show, mmc_write_protect_store);

/*
 * smc write protect
 */
static ssize_t smc_write_protect_show(struct device * dev,
                 struct device_attribute * dattr,
                 char * buf)
{
    uint8_t data;

    data = cpld_reg_read(CPLD_SMC_FAN_EEPROM_WP_REG);
    return sprintf(buf, "0x%02X\n", data);
}

static ssize_t smc_write_protect_store(struct device * dev,
                   struct device_attribute * dattr,
                   const char * buf, size_t count)
{
    int tmp;

    if (sscanf(buf, "%x", &tmp) != 1) {
        return -EINVAL;
    }
    cpld_reg_write(CPLD_SMC_FAN_EEPROM_WP_REG, tmp);

    return count;
}
static SYSFS_ATTR_RW(smc_write_protect, smc_write_protect_show, smc_write_protect_store);

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
 * SMC (System Management) cpld version
 */
static ssize_t cpld_smc_version_show(struct device * dev,
                 struct device_attribute * dattr,
                 char * buf)
{
    uint8_t tmp;
    uint8_t type;
    uint8_t rev;

    tmp = cpld_reg_read(CPLD_SMC_VERSION_REG);
    type = (tmp & CPLD_SMC_VERSION_H_MASK) >> CPLD_SMC_VERSION_H_SHIFT;
    rev  = (tmp & CPLD_SMC_VERSION_L_MASK) >> CPLD_SMC_VERSION_L_SHIFT;

    return sprintf(buf, "%d.%d\n", type, rev);
}
static SYSFS_ATTR_RO(cpld_smc_version, cpld_smc_version_show);


static ssize_t fan_dir_show(struct device * dev,
                 struct device_attribute * dattr,
                 char * buf)
{
    uint8_t tmp;

    tmp = cpld_reg_read(CPLD_SMC_DEV_STA_REG);
    tmp  = (tmp & CPLD_SMC_DEV_STA_FAN_DIR_MASK) >> CPLD_SMC_DEV_STA_FAN_DIR_SHIFT;

    return sprintf(buf, "%02X\n", tmp);
}
static SYSFS_ATTR_RO(fan_dir, fan_dir_show);

static ssize_t thermal_show(struct device * dev,
                 struct device_attribute * dattr,
                 char * buf)
{
    uint8_t tmp;

    tmp = cpld_reg_read(CPLD_MMC_SUS0_STA_REG);
    
    tmp = (~tmp & CPLD_MMC_SUS0_STA_THERMAL_MASK) >> CPLD_MMC_SUS0_STA_THERMAL_SHIFT;
    /* alerts are active low */
    return sprintf(buf, "%02X\n", tmp);
}
static SYSFS_ATTR_RO(thermal, thermal_show);

static ssize_t sfp_show(struct device * dev,
                 struct device_attribute * dattr,
                 char * buf)
{
    uint8_t tmp;

    tmp = cpld_reg_read(CPLD_SMC_SFPTX_CTRL_REG);

    if (strcmp(dattr->attr.name, "sfp_tx_enable") == 0) {
	    /* invert Enable to make it active high */
        tmp = (~tmp & CPLD_SMC_SFPTX_TXEN_MASK) >> CPLD_SMC_SFPTX_TXEN_SHIFT;
    } else if (strcmp(dattr->attr.name, "sfp_rate") == 0) {
	    /* Speed is 1 for 10G */
        tmp = (tmp & CPLD_SMC_SFPTX_RATE_SEL_MASK) >> CPLD_SMC_SFPTX_RATE_SEL_SHIFT;
    } else {
        return sprintf(buf, "undefined\n");
    }
    return sprintf(buf, "0x%02X\n", tmp);
}

static ssize_t sfp_store(struct device * dev,
                 struct device_attribute * dattr,
                 const char * buf, size_t count)
{
    int val;
    uint8_t tmp;

    if (sscanf(buf, "%x", &val) != 1) {
        return -EINVAL;
    }
    tmp = cpld_reg_read(CPLD_SMC_SFPTX_CTRL_REG);

    if (strcmp(dattr->attr.name, "sfp_tx_enable") == 0) {
	    /* Enable is active LOW */
        tmp &= ~(CPLD_SMC_SFPTX_TXEN_MASK);
        tmp |= (~val << CPLD_SMC_SFPTX_TXEN_SHIFT) & CPLD_SMC_SFPTX_TXEN_MASK;
    } else if (strcmp(dattr->attr.name, "sfp_rate") ==0) {
        tmp &= ~(CPLD_SMC_SFPTX_RATE_SEL_MASK);
        tmp |= (val << CPLD_SMC_SFPTX_RATE_SEL_SHIFT) & CPLD_SMC_SFPTX_RATE_SEL_MASK;
    } else {
        return -EINVAL;
    }
    cpld_reg_write(CPLD_SMC_SFPTX_CTRL_REG, tmp);

    return count;
}
static SYSFS_ATTR_RW(sfp_tx_enable, sfp_show, sfp_store);
static SYSFS_ATTR_RW(sfp_rate, sfp_show, sfp_store);

/*------------------------------------------------------------------------------
 *
 * Reset definitions
 *
 */

struct reset_info {
    char name[dell_s3000_CPLD_STRING_NAME_SIZE];
    u_int8_t shift;
};
static struct reset_info r_info[] = {
    {
        .name = "reset_phy_mgmt",
        .shift = CPLD_SMC_SEP_RST_54616,
    },
    {
        .name = "reset_mux_pca9548",
        .shift = CPLD_SMC_SEP_RST_9548A,
    },
    {
        .name = "reset_phy",
        .shift = CPLD_SMC_SEP_RST_50282,
    },
    {
        .name = "reset_usb",
        .shift = CPLD_SMC_SEP_RST_USB,
    },
    {
        .name = "reset_pcie",
        .shift = CPLD_SMC_SEP_RST_PCIE,
    },

};
static int n_resets = ARRAY_SIZE(r_info);

static ssize_t reset_show(struct device * dev,
                 struct device_attribute * dattr,
                 char * buf)
{
    uint8_t tmp;
    int i;
    struct reset_info *target = NULL;

    for (i = 0; i < n_resets; i++) {
        if (strcmp(dattr->attr.name, r_info[i].name) == 0) {
            target = &r_info[i];
            break;
        }
    }
    if (target == NULL) {
        return sprintf(buf, "undefined target\n");
    }

    tmp = cpld_reg_read(CPLD_SMC_SEP_RST_REG);
    tmp &= target->shift;

    /* reset is active low */
    if (tmp & target->shift)
	    return sprintf(buf, "0\n");
    return sprintf(buf, "1\n");
}

static ssize_t reset_store(struct device * dev,
                 struct device_attribute * dattr,
                 const char * buf, size_t count)
{
    int val;
    int i;
    uint8_t tmp;
    struct reset_info *target = NULL;

    /* find the target dev */
    for (i = 0; i < n_resets; i++) {
        if (strcmp(dattr->attr.name, r_info[i].name) == 0) {
            target = &r_info[i];
            break;
        }
    }
    if (target == NULL) {
        return -EINVAL;
    }

    if (sscanf(buf, "%d", &val) <= 0) {
        return -EINVAL;
    }

    tmp = cpld_reg_read(CPLD_SMC_SEP_RST_REG);
    tmp &= ~target->shift;

    /* reset is active low */
    if (val == 0)
        tmp |= target->shift;

    cpld_reg_write(CPLD_SMC_SEP_RST_REG, tmp);
    return count;

}

static SYSFS_ATTR_RW(reset_phy_mgmt, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_mux_pca9548, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_phy, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_usb, reset_show, reset_store);
static SYSFS_ATTR_RW(reset_pcie, reset_show, reset_store);


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
    char name[dell_s3000_CPLD_STRING_NAME_SIZE];
    uint32_t reg;
    uint8_t mask;
    uint8_t shift;
    int n_state;
    struct dev_state_info state[8];
};

static struct dev_state cpld_devs[] = {
    {
        .name = "led_system_power",
        .reg  = CPLD_SMC_FPS_LED1_REG,
        .mask = CPLD_SMC_FPS_LED1_POWER_MASK,
        .shift = CPLD_SMC_FPS_LED1_POWER_SHIFT,
        .n_state = 4,
        .state = {
            { PLATFORM_LED_GREEN,          CPLD_SMC_FPS_LED1_POWER_GREEN},
            { PLATFORM_LED_AMBER,          CPLD_SMC_FPS_LED1_POWER_AMBER},
            { PLATFORM_LED_AMBER_BLINKING, CPLD_SMC_FPS_LED1_POWER_AMBER_BLINK},
            { PLATFORM_LED_OFF,            CPLD_SMC_FPS_LED1_POWER_OFF},
        },
    },
    {
        .name = "led_system_status",
        .reg  = CPLD_SMC_FPS_LED1_REG,
        .mask = CPLD_SMC_FPS_LED1_STA_MASK,
        .shift = CPLD_SMC_FPS_LED1_STA_SHIFT,
        .n_state = 5,
        .state = {
            { PLATFORM_LED_GREEN,          CPLD_SMC_FPS_LED1_STA_GREEN},
            { PLATFORM_LED_AMBER,          CPLD_SMC_FPS_LED1_STA_AMBER},
            { PLATFORM_LED_GREEN_BLINKING, CPLD_SMC_FPS_LED1_STA_GREEN_BLINK},
            { PLATFORM_LED_AMBER_BLINKING, CPLD_SMC_FPS_LED1_STA_AMBER_BLINK},
            { PLATFORM_LED_OFF,            CPLD_SMC_FPS_LED1_STA_OFF},
        },
    },
    {
        .name = "led_system_fan",
        .reg  = CPLD_SMC_FPS_LED1_REG,
        .mask = CPLD_SMC_FPS_LED1_FAN_MASK,
        .shift = CPLD_SMC_FPS_LED1_FAN_SHIFT,
        .n_state = 3,
        .state = {
            { PLATFORM_LED_GREEN, CPLD_SMC_FPS_LED1_FAN_GREEN},
            { PLATFORM_LED_AMBER, CPLD_SMC_FPS_LED1_FAN_AMBER},
            { PLATFORM_LED_OFF,   CPLD_SMC_FPS_LED1_FAN_OFF},
        },
    },
    {
        .name = "led_system_master",
        .reg = CPLD_SMC_FPS_LED1_REG,
        .mask = CPLD_SMC_FPS_LED1_MASTER,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_LED_GREEN, 0},
            { PLATFORM_LED_OFF, CPLD_SMC_FPS_LED1_MASTER},
        }
    },
    {
        .name = "led_fan1",
        .reg  = CPLD_SMC_FAN1_LED_REG,
        .mask = CPLD_SMC_FAN1_LED_MASK,
        .shift = CPLD_SMC_FAN1_LED_SHIFT,
        .n_state = 5,
        .state = {
            { PLATFORM_LED_GREEN, CPLD_SMC_FAN1_LED_GREEN},
            { PLATFORM_LED_GREEN_BLINKING, CPLD_SMC_FAN1_LED_GREEN_SLOW_BLINK},
            { PLATFORM_LED_YELLOW, CPLD_SMC_FAN1_LED_YELLOW},
            { PLATFORM_LED_YELLOW_BLINKING, CPLD_SMC_FAN1_LED_YELLOW_SLOW_BLINK},
            { PLATFORM_LED_OFF, CPLD_SMC_FAN1_LED_OFF},
        },
    },
    {
        .name = "led_fan2",
        .reg  = CPLD_SMC_FAN2_LED_REG,
        .mask = CPLD_SMC_FAN2_LED_MASK,
        .shift = CPLD_SMC_FAN2_LED_SHIFT,
        .n_state = 5,
        .state = {
            { PLATFORM_LED_GREEN, CPLD_SMC_FAN2_LED_GREEN},
            { PLATFORM_LED_GREEN_BLINKING, CPLD_SMC_FAN2_LED_GREEN_SLOW_BLINK},
            { PLATFORM_LED_YELLOW, CPLD_SMC_FAN2_LED_YELLOW},
            { PLATFORM_LED_YELLOW_BLINKING, CPLD_SMC_FAN2_LED_YELLOW_SLOW_BLINK},
            { PLATFORM_LED_OFF, CPLD_SMC_FAN2_LED_OFF},
        },
    },
    {
        .name = "led_fan3",
        .reg  = CPLD_SMC_FAN3_LED_REG,
        .mask = CPLD_SMC_FAN3_LED_MASK,
        .shift = CPLD_SMC_FAN3_LED_SHIFT,
        .n_state = 5,
        .state = {
            { PLATFORM_LED_GREEN, CPLD_SMC_FAN3_LED_GREEN},
            { PLATFORM_LED_GREEN_BLINKING, CPLD_SMC_FAN3_LED_GREEN_SLOW_BLINK},
            { PLATFORM_LED_YELLOW, CPLD_SMC_FAN3_LED_YELLOW},
            { PLATFORM_LED_YELLOW_BLINKING, CPLD_SMC_FAN3_LED_YELLOW_SLOW_BLINK},
            { PLATFORM_LED_OFF, CPLD_SMC_FAN3_LED_OFF},
        },
    },
    {
        .name = "fan_1",
        .reg = CPLD_SMC_SUS6_STA2_REG,
        .mask = CPLD_SMC_SUS6_FAN1_PRESENT_STA,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_NOT_INSTALLED, CPLD_SMC_SUS6_FAN1_PRESENT_STA },
            { PLATFORM_OK, 0 },
        },
    },
    {
        .name = "fan_2",
        .reg = CPLD_SMC_SUS6_STA2_REG,
        .mask = CPLD_SMC_SUS6_FAN2_PRESENT_STA,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_NOT_INSTALLED, CPLD_SMC_SUS6_FAN2_PRESENT_STA },
            { PLATFORM_OK, 0 },
        },
    },
    {
        .name = "fan_3",
        .reg = CPLD_SMC_SUS6_STA2_REG,
        .mask = CPLD_SMC_SUS6_FAN3_PRESENT_STA,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_NOT_INSTALLED, CPLD_SMC_SUS6_FAN3_PRESENT_STA },
            { PLATFORM_OK, 0 },
        },
    },
    {
        .name = "psu_pwr1",
        .reg = CPLD_SMC_SUS6_STA2_REG,
        .mask = CPLD_SMC_SUS6_PSUR_PRESENT_STA,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_NOT_INSTALLED, CPLD_SMC_SUS6_PSUR_PRESENT_STA },
            { PLATFORM_INSTALLED, 0 },
        },
    },
    {
        .name = "psu_pwr2",
        .reg  = CPLD_SMC_SUS6_STA2_REG,
        .mask = CPLD_SMC_SUS6_PSUL_PRESENT_STA,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_NOT_INSTALLED, CPLD_SMC_SUS6_PSUL_PRESENT_STA },
            { PLATFORM_INSTALLED, 0 },
        },
    },
    {
        .name = "psu_pwr1_ac",
        .reg  = CPLD_SMC_PSU_STATUS_REG,
        .mask = CPLD_SMC_PSUR_AC,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_PS_POWER_BAD, 0 },
            { PLATFORM_OK, CPLD_SMC_PSUR_AC },
        },
    },
    {
        .name = "psu_pwr1_power",
        .reg  = CPLD_SMC_PSU_STATUS_REG,
        .mask = CPLD_SMC_PSUR_POWER,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_PS_POWER_BAD, 0 },
            { PLATFORM_OK, CPLD_SMC_PSUR_POWER },
        },
    },
    {
        .name = "psu_pwr1_alert",
        .reg  = CPLD_SMC_PSU_STATUS_REG,
        .mask = CPLD_SMC_PSUR_ALERT,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_PS_POWER_BAD, 0 },
            { PLATFORM_OK, CPLD_SMC_PSUR_ALERT },
        },
    },
    {
        .name = "psu_pwr2_ac",
        .reg  = CPLD_SMC_PSU_STATUS_REG,
        .mask = CPLD_SMC_PSUL_AC,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_PS_POWER_BAD, 0 },
            { PLATFORM_OK, CPLD_SMC_PSUL_AC },
        },
    },
    {
        .name = "psu_pwr2_power",
        .reg  = CPLD_SMC_PSU_STATUS_REG,
        .mask = CPLD_SMC_PSUL_POWER,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_PS_POWER_BAD, 0 },
            { PLATFORM_OK, CPLD_SMC_PSUL_POWER },
        },
    },
    {
        .name = "psu_pwr2_alert",
        .reg  = CPLD_SMC_PSU_STATUS_REG,
        .mask = CPLD_SMC_PSUL_ALERT,
        .shift = 0,
        .n_state = 2,
        .state = {
            { PLATFORM_PS_POWER_BAD, 0 },
            { PLATFORM_OK, CPLD_SMC_PSUL_ALERT },
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

static SYSFS_ATTR_RW(led_system_power, dev_show, dev_store);
static SYSFS_ATTR_RW(led_system_status, dev_show, dev_store);
static SYSFS_ATTR_RW(led_system_fan, dev_show, dev_store);
static SYSFS_ATTR_RW(led_system_master, dev_show, dev_store);
static SYSFS_ATTR_RW(led_fan1, dev_show, dev_store);
static SYSFS_ATTR_RW(led_fan2, dev_show, dev_store);
static SYSFS_ATTR_RW(led_fan3, dev_show, dev_store);
static SYSFS_ATTR_RO(fan_1, dev_show);
static SYSFS_ATTR_RO(fan_2, dev_show);
static SYSFS_ATTR_RO(fan_3, dev_show);
static SYSFS_ATTR_RO(psu_pwr1, dev_show); /* PSUR */
static SYSFS_ATTR_RO(psu_pwr2, dev_show); /* PSUL */
static SYSFS_ATTR_RO(psu_pwr1_ac, dev_show);
static SYSFS_ATTR_RO(psu_pwr1_power, dev_show);
static SYSFS_ATTR_RO(psu_pwr1_alert, dev_show);
static SYSFS_ATTR_RO(psu_pwr2_ac, dev_show);
static SYSFS_ATTR_RO(psu_pwr2_power, dev_show);
static SYSFS_ATTR_RO(psu_pwr2_alert, dev_show);


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
cpld_smc_version\n\
\n\
  Read-Only:\n\
\n\
  CPLD version register for SMC CPLD\n\
  CPLD version register in the following format:\n\
\n\
  cpld_smc_version\n\
\n\
  Example: 0.2\n\
\n\
\n\
led_system_master\n\
\n\
  Read-Write:\n\
\n\
  System Master LED color.\n\
  The following values are possible: green and off\n\
\n\
\n\
led_system_fan\n\
\n\
  Read-Write:\n\
\n\
  System Fan LED color.\n\
  The following values are possible: green, yellow and off\n\
\n\
\n\
led_system_power\n\
\n\
  Read-Write:\n\
\n\
  System Power LED color.\n\
  The following values are possible: green, yellow and off\n\
\n\
\n\
led_system_status\n\
\n\
  Read-Write:\n\
\n\
  System Status LED color.\n\
  The following values are possible: green, yellow and off\n\
\n\
\n\
led_fan1\n\
led_fan2\n\
led_fan3\n\
\n\
  Read-Write:\n\
\n\
  Indicates the operational status of the fans.\n\
   'green' - front to back fan operational\n\
   'green blinking' - front to back fan alarm\n\
   'yellow' - back to front fan operational\n\
   'yellow blinking' - back to front fan alarm\n\
   'off' - fan not inserted correctly\n\
\n\
\n\
sfp_rate\n\
\n\
  Read-Write:\n\
\n\
  Read or set an SFP port's transmit rate\n\
  A bitmask with bits 0:3 for SFP ports 1 - 4\n\
  1 means 10G. 0 means 1G.\n\
\n\
\n\
sfp_tx_enable\n\
\n\
  Read-Write:\n\
\n\
  Read or set an SFP port's transmit enable.\n\
  A bitmask with bits 0:3 for SFP ports 1 - 4\n\
  1 means enable. 0 means disable.\n\
\n\
\n\
reset_phy_mgmt\n\
reset_mux_pca9548\n\
reset_mux_phy\n\
reset_usb\n\
reset_mux_pcie\n\
\n\
  Read-Write:\n\
\n\
  Indicates the operational status of the fans.\n\
   '1' - Puts the device in reset \n\
   '0' - Takes device out of reset \n\
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

static SYSFS_ATTR_RO(README, help_show);

/*------------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */

static struct attribute *dell_s3000_cpld_attrs[] = {
    &dev_attr_cpld_mmc_version.attr,
    &dev_attr_cpld_smc_version.attr,
    &dev_attr_led_system_power.attr,
    &dev_attr_led_system_status.attr,
    &dev_attr_led_system_fan.attr,
    &dev_attr_led_system_master.attr,
    &dev_attr_led_fan1.attr,
    &dev_attr_led_fan2.attr,
    &dev_attr_led_fan3.attr,
    &dev_attr_mmc_write_protect.attr,
    &dev_attr_smc_write_protect.attr,
    &dev_attr_psu_pwr1.attr,
    &dev_attr_psu_pwr1_ac.attr,
    &dev_attr_psu_pwr1_power.attr,
    &dev_attr_psu_pwr1_alert.attr,
    &dev_attr_psu_pwr2.attr,
    &dev_attr_psu_pwr2_ac.attr,
    &dev_attr_psu_pwr2_power.attr,
    &dev_attr_psu_pwr2_alert.attr,
    &dev_attr_fan_dir.attr,
    &dev_attr_thermal.attr,
    &dev_attr_sfp_tx_enable.attr,
    &dev_attr_sfp_rate.attr,
    &dev_attr_fan_1.attr,
    &dev_attr_fan_2.attr,
    &dev_attr_fan_3.attr,
    &dev_attr_reset_phy_mgmt.attr,
    &dev_attr_reset_mux_pca9548.attr,
    &dev_attr_reset_phy.attr,
    &dev_attr_reset_usb.attr,
    &dev_attr_reset_pcie.attr,
    &dev_attr_README.attr,
    NULL,
};

static struct attribute_group dell_s3000_cpld_attr_group = {
    .attrs = dell_s3000_cpld_attrs,
};

/*------------------------------------------------------------------------------
 *
 * module interface
 *
 */
static struct platform_device *dell_s3000_cpld_device;

static int dell_s3000_cpld_probe(struct platform_device *dev)
{
    int ret;

    dell_s3000_cpld_regs = ioport_map(CPLD_IO_BASE, CPLD_IO_SIZE);
    if (!dell_s3000_cpld_regs) {
        pr_err("cpld: unabled to map iomem\n");
        ret = -ENODEV;
        goto err_exit;
    }

    ret = sysfs_create_group(&dev->dev.kobj, &dell_s3000_cpld_attr_group);
    if (ret) {
        pr_err("cpld: sysfs_create_group failed for cpld driver");
        goto err_unmap;
    }

err_unmap:
    iounmap(dell_s3000_cpld_regs);

err_exit:
    return ret;
}

static int dell_s3000_cpld_remove(struct platform_device *dev)
{
    iounmap(dell_s3000_cpld_regs);
    return 0;
}

static struct platform_driver dell_s3000_cpld_driver = {
    .driver = {
        .name = "dell_s3000_cpld",
        .owner = THIS_MODULE,
    },
    .probe = dell_s3000_cpld_probe,
    .remove = dell_s3000_cpld_remove,
};

static int __init dell_s3000_cpld_init(void)
{
    int rv;

    rv = platform_driver_register(&dell_s3000_cpld_driver);
    if (rv) {
        goto err_exit;
    }

    dell_s3000_cpld_device = platform_device_alloc("dell_s3000_cpld", 0);
    if (!dell_s3000_cpld_device) {
        pr_err("platform_device_alloc() failed for cpld device\n");
        rv = -ENOMEM;
        goto err_unregister;
    }

    rv = platform_device_add(dell_s3000_cpld_device);
    if (rv) {
        pr_err("platform_device_add() failed for cpld device.\n");
        goto err_dealloc;
    }
    return 0;

err_dealloc:
    platform_device_unregister(dell_s3000_cpld_device);

err_unregister:
    platform_driver_unregister(&dell_s3000_cpld_driver);

err_exit:
    pr_err("%s platform_driver_register failed (%i)\n",
            driver_name, rv);
    return rv;
}

static void __exit dell_s3000_cpld_exit(void)
{
    platform_driver_unregister(&dell_s3000_cpld_driver);
    platform_device_unregister(dell_s3000_cpld_device);
}

MODULE_AUTHOR("Samer Nubani <samer@cumulusnetworks.com>");
MODULE_DESCRIPTION("Platform CPLD driver for DELL S3000");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(dell_s3000_cpld_init);
module_exit(dell_s3000_cpld_exit);
