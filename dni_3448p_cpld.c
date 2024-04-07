/*
 * dni_3448p_cpld.c - DNI 3448P Platform Support.
 *
 * Author: Alan Liebthal (alanl@cumulusnetworks.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
 *
 */

#include <stddef.h>

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/platform_data/at24.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <asm/io.h>

#include "dni_3448p_cpld.h"
#include "platform_defs.h"

static const char cpld_driver_name[] = "dni,3448p-cpld";
static const char psu_mux_driver_name[] = "dni_3448p_psu_mux";
#define DRIVER_VERSION "1.0"

/*-------------------------------------------------------------------------
 *
 * Driver resident static variables
 *
 */

static struct i2c_client *dni3448p_cpld_client;


/*---------------------------------------------------------------------
 *
 * CPLD
 */
static int cpld_rd(uint32_t reg)
{
	int val;

	val = i2c_smbus_read_byte_data(dni3448p_cpld_client, reg);
	if (val < 0) {
		pr_err("I2C read error - addr: 0x%02X, offset: 0x%02X",
				dni3448p_cpld_client->addr, reg);
	}
	return val;
}

static int cpld_wr(uint32_t reg, uint8_t write_val)
{
	int res;

	res = i2c_smbus_write_byte_data(dni3448p_cpld_client, reg, write_val);
	if (res) {
		pr_err("could not write to i2c device addr: 0x%02X, "
			"reg: 0x%02X, val: 0x%02X",
			dni3448p_cpld_client->addr, reg, write_val);
	}
	return res;
}

#define DNI3448P_CPLD_STRING_NAME_SIZE 20

/*
 * board version
 */
static ssize_t board_revision_show(struct device * dev,
                                   struct device_attribute * dattr,
                                   char * buf)
{
	int data;
	uint8_t rev;

	data = cpld_rd(CPLD_REG_HW_REVISION);
	if (data < 0)
		return -ENXIO;

	rev = (uint8_t)data;
	return sprintf(buf, "0x%X v%u\n", rev >> 4,
			rev & CPLD_HW_REV_MASK);
}
static SYSFS_ATTR_RO(board_revision, board_revision_show);

/*-------------------------------------------------------------------------
 *
 * PSU status definitions
 *
 * All the definition names use "positive" logic and return "1" for OK
 * and "0" for not OK.
 */
struct psu_status {
	char name[DNI3448P_CPLD_STRING_NAME_SIZE];
	uint8_t good_mask;  // set bits for "good behaviour"
	uint8_t bad_mask;   // set bits for "bad behaviour"
};

static struct psu_status cpld_psu_status[] = {
	{
		.name = "psu_pwr1_all_ok",
// alert bit does not seem to work, need to check with DNI
//		.good_mask = CPLD_PS1_ALERT_L | CPLD_PS1_GOOD,
		.good_mask = CPLD_PS1_GOOD,
		.bad_mask = CPLD_PS1_PRESENT_L,
	},
	{
		.name = "psu_pwr1_present",
		.good_mask = 0,
		.bad_mask = CPLD_PS1_PRESENT_L,
	},
	{
		.name = "psu_pwr2_all_ok",
// alert bit does not seem to work, need to check with DNI
//		.good_mask = CPLD_PS2_ALERT_L | CPLD_PS2_GOOD,
		.good_mask = CPLD_PS2_GOOD,
		.bad_mask = CPLD_PS2_PRESENT_L,
	},
	{
		.name = "psu_pwr2_present",
		.good_mask = 0,
		.bad_mask = CPLD_PS2_PRESENT_L,
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
	int data;
	uint8_t tmp, i;
	uint8_t bad = 0;
	struct psu_status* target = NULL;

	for (i = 0; i < n_psu_states; i++) {
		if (strcmp(dattr->attr.name, cpld_psu_status[i].name) == 0) {
			target = &cpld_psu_status[i];
			break;
		}
	}
	if (target == NULL) {
		return sprintf(buf, "undefined\n");
	}
	data = cpld_rd(CPLD_REG_N30XX_PSU_STATUS);
	if (data < 0)
		return -ENXIO;
	tmp = (uint8_t)data;

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
static SYSFS_ATTR_RO(psu_pwr1_all_ok,  psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_present, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_all_ok,  psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_present, psu_power_show);

/*
 * fan status
 */
static ssize_t fan_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	int data;
	uint8_t tmp;

	data = cpld_rd( CPLD_REG_FAN_STATUS);
	if (data < 0)
		return -ENXIO;
	tmp = (uint8_t)data;

	if (tmp & CPLD_FAN1_TRAY_PRESENT_L) {
		return sprintf(buf, PLATFORM_NOT_INSTALLED "\n");
	}
	else {
		return sprintf(buf, PLATFORM_INSTALLED "," PLATFORM_OK "\n");
	}

}
static SYSFS_ATTR_RO(fan_tray_present, fan_show);


/*-------------------------------------------------------------------------
 *
 * LED definitions
 *
 */

struct led {
	char name[DNI3448P_CPLD_STRING_NAME_SIZE];
	uint8_t reg;
	uint8_t mask;
	int n_colors;
	struct led_color colors[8];
};

static struct led cpld_leds[] = {
	{
		.name = "led_system",
		.reg = CPLD_REG_LED_CTL_1,
		.mask = CPLD_SYSTEM_LED_MASK,
		.n_colors = 4,
		.colors = {
			{ PLATFORM_LED_GREEN,          CPLD_SYSTEM_LED_GREEN},
			{ PLATFORM_LED_GREEN_BLINKING, CPLD_SYSTEM_LED_GREEN_BLINK},
			{ PLATFORM_LED_RED,            CPLD_SYSTEM_LED_RED},
			{ PLATFORM_LED_RED_BLINKING,   CPLD_SYSTEM_LED_RED_BLINK},
		},
	},
	{
		.name = "led_fan",
		.reg = CPLD_REG_LED_CTL_1,
		.mask = CPLD_FAN_LED_MASK,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_GREEN, CPLD_FAN_LED_GREEN},
			{ PLATFORM_LED_RED,   CPLD_FAN_LED_RED},
		},
	},
	{
		.name = "led_psu1",
		.reg = CPLD_REG_LED_CTL_1,
		.mask = CPLD_PWR1_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_OFF,              CPLD_PWR1_LED_OFF},
			{ PLATFORM_LED_GREEN,            CPLD_PWR1_LED_GREEN},
			{ PLATFORM_LED_GREEN_BLINKING,   CPLD_PWR1_LED_GREEN_BLINK},
		},
	},
	{
		.name = "led_psu2",
		.reg = CPLD_REG_LED_CTL_1,
		.mask = CPLD_PWR2_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_OFF,              CPLD_PWR2_LED_OFF},
			{ PLATFORM_LED_GREEN,            CPLD_PWR2_LED_GREEN},
			{ PLATFORM_LED_GREEN_BLINKING,   CPLD_PWR2_LED_GREEN_BLINK},
		},
	},
	{
		.name = "led_master",
		.reg = CPLD_REG_LED_CTL_2,
		.mask = CPLD_MASTER_LED_MASK,
		.n_colors = 2,
		.colors = {
			{ PLATFORM_LED_OFF,              CPLD_MASTER_LED_OFF},
			{ PLATFORM_LED_GREEN,            CPLD_MASTER_LED_GREEN},
		},
	},
	{
		.name = "led_temp",
		.reg = CPLD_REG_LED_CTL_2,
		.mask = CPLD_TEMP_LED_MASK,
		.n_colors = 3,
		.colors = {
			{ PLATFORM_LED_OFF,              CPLD_TEMP_LED_OFF},
			{ PLATFORM_LED_RED,              CPLD_TEMP_LED_RED},
			{ PLATFORM_LED_GREEN,            CPLD_TEMP_LED_GREEN},
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
	int data;
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
	data = cpld_rd(target->reg);
	if (data < 0)
		return -ENXIO;
	tmp = (uint8_t)data;

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
	int data;
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
	data = cpld_rd(target->reg);
	tmp = (uint8_t)data;
	tmp &= ~target->mask;
	tmp |= target->colors[i].value;
	data = cpld_wr(target->reg, tmp);
	if (data < 0)
		return -ENXIO;

	return count;
}
static SYSFS_ATTR_RW(led_system, led_show, led_store);
static SYSFS_ATTR_RW(led_fan,    led_show, led_store);
static SYSFS_ATTR_RW(led_psu1,   led_show, led_store);
static SYSFS_ATTR_RW(led_psu2,   led_show, led_store);
static SYSFS_ATTR_RW(led_master, led_show, led_store);
static SYSFS_ATTR_RW(led_temp,   led_show, led_store);

/*
 * 7-segment LEDs
 */

static ssize_t seg7_value_store(struct device * dev,
				struct device_attribute * dattr,
				const char * buf, size_t count)
{
	unsigned int val;

	/*
	 * Assume input is a two byte string containing two hex
	 * digits.
	 */
	if ( strncmp(buf, "0x", 2) == 0 )
		buf += 2;
	if (sscanf(buf, "%X", &val) != 1)
		return -EINVAL;
	if (val > 0xff)
		return -EINVAL;

	cpld_wr( CPLD_REG_7DIGIT_LED_CTL, val);

	return count;
}
static SYSFS_ATTR_WO(led_display_value, seg7_value_store);

static ssize_t rtc_reset_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	int data;

	/* read the register */
	data = cpld_rd(CPLD_REG_WATCHDOG);
	if (data < 0)
		return -ENXIO;

	if (data & CPLD_WATCHDOG_RESET_RTC) {
		return sprintf(buf, "1\n");
	}

	return sprintf(buf, "0\n");
}


static ssize_t rtc_reset_set(struct device * dev,
                         struct device_attribute * dattr,
                         const char * buf, size_t count)
{
	int data;
	int val;

	/* find the color */
	if (sscanf(buf, "%d", &val) <= 0) {
		return -EINVAL;
	}
	/* set the new value */
	data = cpld_rd(CPLD_REG_WATCHDOG);
	if (val > 0)
		data |= CPLD_WATCHDOG_RESET_RTC;
	else
		data &= ~CPLD_WATCHDOG_RESET_RTC;

	cpld_wr(CPLD_REG_WATCHDOG, (u8)data);
	return count;
}
static SYSFS_ATTR_RW(rtc_reset, rtc_reset_show, rtc_reset_set);

/*-------------------------------------------------------------------------
 *
 * POE+ Reset and Control Register Show.
 *
 */
static ssize_t poe_plus_power_show(struct device * dev,
                                   struct device_attribute * dattr,
                                   char * buf)
{
	int data;
	uint8_t poe_status;

	data = cpld_rd(CPLD_REG_DEVICE_SHUTDOWN);
	if (data < 0)
		return -ENXIO;
	poe_status = (uint8_t)data;

	return sprintf(buf, "%d\n", poe_status & CPLD_SHUTDOWN_POE_L ? 0 : 1);
}

static ssize_t poe_plus_power_store(struct device * dev,
                                    struct device_attribute * dattr,
                                    const char * buf, size_t count)
{
	int data;
	uint8_t tmp;
	int enable = 0;

	if (sscanf(buf, "%d", &enable) <= 0) {
		return -EINVAL;
	}

	data = cpld_rd(CPLD_REG_DEVICE_SHUTDOWN);
	if (data < 0)
		return -ENXIO;
	tmp = (uint8_t)data;

	if (enable) {
		tmp |= (CPLD_SHUTDOWN_POE_L);
	} else {
		tmp &= ~(CPLD_SHUTDOWN_POE_L);
	}
	data = cpld_wr(CPLD_REG_DEVICE_SHUTDOWN, tmp);
	if (data < 0)
		return -ENXIO;

	return count;
}

static ssize_t poe_plus_reset_show(struct device * dev,
                                   struct device_attribute * dattr,
                                   char * buf)
{
	return strlen("0\n");
}

static ssize_t poe_plus_reset_store(struct device * dev,
                                    struct device_attribute * dattr,
                                    const char * buf, size_t count)
{
	int data;
	uint8_t tmp;
	int reset = 0;

	if (sscanf(buf, "%d", &reset) <= 0) {
		return -EINVAL;
	}

	data = cpld_rd(CPLD_REG_SW_RESET);
	if (data < 0)
		return -ENXIO;
	tmp = (uint8_t)data;

	if (reset) {
		tmp &= ~(CPLD_RESET_POE_L);
	} else {
		tmp |= (CPLD_RESET_POE_L);
	}

	data = cpld_wr(CPLD_REG_SW_RESET, tmp);
	if (data < 0)
		return -ENXIO;

	return count;
}
static SYSFS_ATTR_RW(poe_plus_power, poe_plus_power_show,
				poe_plus_power_store);
static SYSFS_ATTR_RW(poe_plus_reset, poe_plus_reset_show,
				poe_plus_reset_store);

/*-------------------------------------------------------------------------
 *
 * sysfs registration
 *
 */

static struct attribute *dni3448p_cpld_attrs[] = {
	&dev_attr_board_revision.attr,
	&dev_attr_psu_pwr1_all_ok.attr,
	&dev_attr_psu_pwr1_present.attr,
	&dev_attr_psu_pwr2_all_ok.attr,
	&dev_attr_psu_pwr2_present.attr,
	&dev_attr_fan_tray_present.attr,
	&dev_attr_led_system.attr,
	&dev_attr_led_fan.attr,
	&dev_attr_led_psu1.attr,
	&dev_attr_led_psu2.attr,
	&dev_attr_led_master.attr,
	&dev_attr_led_temp.attr,
	&dev_attr_led_display_value.attr,
	&dev_attr_poe_plus_power.attr,
	&dev_attr_poe_plus_reset.attr,
	&dev_attr_rtc_reset.attr,
	NULL,
};

/*---------------------------------------------------------------------
 *
 * psu mux
 *
 *  The two PSUs are located at the same addresses behind a mux
 *  controlled by the CPLD. This mux provides access to the two
 *  PSUs.
 */
#define EEPROM_LABEL_SIZE       16
static struct i2c_board_info *alloc_psu_eeprom_board_info(int psu_num) {
	char *label;
	struct eeprom_platform_data *eeprom_data;
	struct at24_platform_data *at24_data;
	struct i2c_board_info *board_info;

	label = kzalloc(EEPROM_LABEL_SIZE, GFP_KERNEL);
	if (!label) {
		goto err_exit;
	}
	eeprom_data = kzalloc(sizeof(struct eeprom_platform_data), GFP_KERNEL);
	if (!eeprom_data) {
		goto err_exit_eeprom;
	}
	at24_data = kzalloc(sizeof(struct at24_platform_data), GFP_KERNEL);
	if (!at24_data) {
		goto err_exit_at24;
	}
	board_info = kzalloc(sizeof(struct i2c_board_info), GFP_KERNEL);
	if (!board_info) {
		goto err_exit_board;
	}

	snprintf(label, EEPROM_LABEL_SIZE, "psu%u_eeprom", psu_num);
	eeprom_data->label = label;
	at24_data->byte_len = 256;
	at24_data->flags = AT24_FLAG_IRUGO;
	at24_data->page_size = 1;
	at24_data->eeprom_data = eeprom_data;

	strcpy(board_info->type, "24c02");
	board_info->addr = 0x52;
	board_info->platform_data = at24_data;

	return board_info;

err_exit_board:
	kfree(at24_data);
err_exit_at24:
	kfree(eeprom_data);
err_exit_eeprom:
	kfree(label);
err_exit:
	return NULL;
}

static int psu_mux_channel_to_val[] = {
	CPLD_I2C_PSU1_SELECT, CPLD_I2C_PSU2_SELECT
};
#define DNI_3448P_PSU_MUX_NUM_CHANNELS  ARRAY_SIZE(psu_mux_channel_to_val)
#define DNI_3448P_BASE_MUX_NUM             (5)
#define DNI_3448P_MUX_INVALID_CHANNEL      (0xDEADBEEF)

struct dni3448p_psu_mux_info {
	int last_channel;
	struct i2c_adapter *m_virt_adaps[2];
};

static int dni3448p_psu_mux_select_chan(struct i2c_adapter *adap,
                                       void *client, u32 chan)
{
	struct dni3448p_psu_mux_info* dev = i2c_get_clientdata(client);
	int data;
	uint8_t tmp;

	if (chan == dev->last_channel)
		return 0;

	dev->last_channel = chan;
	if (chan >= DNI_3448P_PSU_MUX_NUM_CHANNELS)
		return -EINVAL;

	data = cpld_rd(CPLD_REG_N30XX_I2C_MUX_CTL);
	if (data < 0)
		return -ENXIO;
	tmp = (uint8_t)data;
	tmp &= ~CPLD_I2C_SELECT_MASK;
	tmp |= psu_mux_channel_to_val[chan];
	data = cpld_wr(CPLD_REG_N30XX_I2C_MUX_CTL, tmp);
	if (data < 0)
		return -ENXIO;

	return 0;
}

static int dni3448p_psu_mux_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{
	struct i2c_adapter* adap = to_i2c_adapter(client->dev.parent);
	struct i2c_board_info *board_info;
	struct i2c_client *eeprom_client;
	struct dni3448p_psu_mux_info* dev;
	int num;

	int ret = -ENODEV;

	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_BYTE))
		goto exit;

	dev = kzalloc(sizeof(struct dni3448p_psu_mux_info), GFP_KERNEL);
	if (!dev) {
		ret = -ENOMEM;
		goto exit;
	}
	dev->last_channel = DNI_3448P_MUX_INVALID_CHANNEL;
	i2c_set_clientdata(client, dev);

	/* Now create an adapter for each channel */
	for (num = 0; num < DNI_3448P_PSU_MUX_NUM_CHANNELS; num++) {
		dev->m_virt_adaps[num] =
			i2c_add_mux_adapter(adap, &client->dev, client,
				DNI_3448P_BASE_MUX_NUM+num, num,
				0, dni3448p_psu_mux_select_chan,
				NULL);

		if (dev->m_virt_adaps[num] == NULL) {
			ret = -ENODEV;
			dev_err(&client->dev,
			        "failed to register multiplexed adapter %d\n", num);
			goto virt_reg_failed;
		}
		board_info = alloc_psu_eeprom_board_info(num+1);
		if (!board_info) {
			pr_err("could not allocate board info\n");
			ret = -ENOMEM;
			goto exit;
		}
		eeprom_client = i2c_new_device(dev->m_virt_adaps[num], board_info);
		if (!eeprom_client) {
			pr_err("could not create i2c_client %s\n", board_info->type);
			ret = -ENOMEM;
			goto exit;
		}
	}

	dev_info(&client->dev,
	         "registered %d multiplexed buses for %s\n",
	          num, psu_mux_driver_name);
		return 0;

virt_reg_failed:
	for (num--; num >= 0; num--)
		i2c_del_mux_adapter(dev->m_virt_adaps[num]);

	kfree(dev);
exit:
	return ret;
}

static int dni3448p_psu_mux_remove(struct i2c_client* client)
{
	struct dni3448p_psu_mux_info* dev = i2c_get_clientdata(client);
	int i;

	for (i = 0; i < DNI_3448P_PSU_MUX_NUM_CHANNELS; ++i)
		if (dev->m_virt_adaps[i]) {
			i2c_del_mux_adapter(dev->m_virt_adaps[i]);
			dev->m_virt_adaps[i] = NULL;
		}

	kfree(dev);
	return 0;
}

static struct attribute_group dni3448p_cpld_attr_group = {
	.attrs = dni3448p_cpld_attrs,
};


static struct i2c_device_id dni3448p_psu_mux_ids[] = {
	{
		.name = "3448p-psu-mux",
	},
	{ /* end of list */ },
};
MODULE_DEVICE_TABLE(i2c, dni3448p_psu_mux_ids);

static struct i2c_driver dni3448p_psu_mux_driver = {
	.probe = dni3448p_psu_mux_probe,
	.remove = dni3448p_psu_mux_remove,
	.id_table = dni3448p_psu_mux_ids,
	.driver = {
		.name  = psu_mux_driver_name,
		.owner = THIS_MODULE,
	},
};

extern void (*cpld_system_reset)(void);

void dni3448p_cpld_reset(void)
{
	uint8_t tmp;
	u8 invert = 0;

	/*  all defined resets except the SoC (switch) and system reset */
	tmp = ~invert & ~(CPLD_RESET_OOB_L |
			CPLD_RESET_10G_SLOT1_L | CPLD_RESET_PHY_L |
			CPLD_RESET_POE_L);
	(void)cpld_wr(CPLD_REG_SW_RESET, tmp); /* reset */
	mdelay(20); /*  wait a bit for the other resets */

	/*  reset the system and SoC (switch) */
	tmp = ~invert & ~(CPLD_FORCE_SW_RESET_L | CPLD_RESET_MAC_L);
	(void)cpld_wr(CPLD_REG_SW_RESET, tmp);
}

/*-------------------------------------------------------------------------
 *
 * CPLD driver interface
 *
 */

static int dni3448p_cpld_setup(void)
{
	// Put some interesting, one-time initializations here.

	cpld_system_reset = dni3448p_cpld_reset;

	return 0;
}

static int dni3448p_cpld_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	int retval = 0;
	struct kobject * kobj = &client->dev.kobj;

	dev_info(&client->dev, "probe 3448p\n");
	dev_info(&client->dev, "addr 0x%02X\n", client->addr);

	if (dev_get_drvdata(&client->dev)) {
		dev_info(&client->dev, "already probed\n");
		return 0;
	}
	dni3448p_cpld_client = client;

	retval = sysfs_create_group(kobj, &dni3448p_cpld_attr_group);
	if (retval) {
		return retval;
	}

	if (dni3448p_cpld_setup()) {
	     return -EIO;
	}

	dev_info(&client->dev, "probed @ 0x%p\n", dni3448p_cpld_setup);

	return 0;
}

static int dni3448p_cpld_remove(struct i2c_client *client)
{
	struct kobject * kobj = &client->dev.kobj;

	cpld_system_reset = NULL;

	sysfs_remove_group(kobj, &dni3448p_cpld_attr_group);

	dev_info(&client->dev, "removed\n");
	return 0;
}

static struct i2c_device_id dni3448p_cpld_ids[] = {
	{
		.name = "3448p-cpld",
	},
	{ /* end of list */ },
};
MODULE_DEVICE_TABLE(i2c, dni3448p_cpld_ids);

static struct i2c_driver dni3448p_cpld_driver = {
	.probe = dni3448p_cpld_probe,
	.remove = dni3448p_cpld_remove,
	.id_table = dni3448p_cpld_ids,
	.driver = {
		.name  = cpld_driver_name,
		.owner = THIS_MODULE,
	},
};


/*-------------------------------------------------------------------------
 *
 * module interface
 *
 */

static int __init dni3448p_cpld_init(void)
{
	int rv;

	rv = i2c_add_driver(&dni3448p_cpld_driver);
	if (rv) {
		printk(KERN_ERR
		       "%s i2c_add_driver() failed (%i)\n",
		       cpld_driver_name, rv);
		goto exit;
	}

	rv = i2c_add_driver(&dni3448p_psu_mux_driver);
	if (rv) {
		printk(KERN_ERR
		       "%s i2c_add_driver() failed (%i)\n",
		       psu_mux_driver_name, rv);
		goto exit;
	}
exit:
	return rv;
}

static void __exit dni3448p_cpld_exit(void)
{
	i2c_del_driver(&dni3448p_psu_mux_driver);
	i2c_del_driver(&dni3448p_cpld_driver);
}

MODULE_AUTHOR("Alan Liebthal <alanl@cumulusnetworks.com>");
MODULE_DESCRIPTION("CPLD driver for Delta Networks Inc. ET-3448P");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(dni3448p_cpld_init);
module_exit(dni3448p_cpld_exit);
