/*
 * sysfs driver for Accton AS670x_32x System CPLD (SYSPLD)
 *
 * Copyright (C) 2014 Cumulus Networks, Inc.
 * Author: Dustin Byford <dustin@cumulusnetworks.com>
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
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/of_platform.h>
#include <asm/io.h>

#include "accton_as670x_32x_syspld.h"
#include "platform_defs.h"

static const char driver_name[] = "accton_as670x_32x_syspld";
#define DRIVER_VERSION "1.0"

static const struct i2c_device_id syspld_id[] = {
        { "as670x_32x_syspld", 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, syspld_id);

struct syspld {
	struct i2c_adapter *virt_adaps[CPLD_I2C_SWITCH_NUM_CHANNELS];
	u8 last_switch_control;
};

static ssize_t code_revision_show(struct device *dev,
                                  struct device_attribute *dattr,
                                  char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int reg;
	u8 code_ver;
	u8 code_rel;

	reg = i2c_smbus_read_byte_data(client, CPLD_REG_VERSION);
	code_ver = (reg & CPLD_VERSION_CODE_MASK) >> CPLD_VERSION_CODE_SHIFT;
	code_rel = (reg & CPLD_VERSION_CODE_REL_MASK) != 0;

	return sprintf(buf, "%u.0%u\n", code_rel, code_ver);
}
static SYSFS_ATTR_RO(code_revision, code_revision_show);
static ssize_t hw_revision_show(struct device *dev,
                                struct device_attribute *dattr,
                                char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int reg;
	u8 hw_ver;
	u8 hw_rel;

	reg = i2c_smbus_read_byte_data(client, CPLD_REG_VERSION);
	hw_ver = (reg & CPLD_VERSION_HW_MASK) >> CPLD_VERSION_HW_SHIFT;
	hw_rel = (reg & CPLD_VERSION_HW_REL_MASK) != 0;

	if (hw_rel) {
	        return sprintf(buf, "0%u\n", hw_ver);
	} else {
	        return sprintf(buf, "0%c\n", hw_ver + 'A');
	}
}
static SYSFS_ATTR_RO(hw_revision, hw_revision_show);

/*
 * PSU status bits
 */
static const uint8_t *psu1_name = xstr(PLATFORM_PS_NAME_0);
static const uint8_t *psu2_name = xstr(PLATFORM_PS_NAME_1);
static ssize_t psu_power_show(struct device *dev,
			      struct device_attribute *dattr,
			      char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	bool rv = 0;
	bool present, dc_pg;
	const uint8_t *attr_name;
	uint8_t reg;

	reg = i2c_smbus_read_byte_data(client, CPLD_REG_PSU_STATUS);

	if (strncmp(dattr->attr.name, psu1_name, strlen(psu1_name)) == 0) {
		present = (reg & CPLD_PSU_STATUS_PS1_PRESENT_L) == 0;
		dc_pg = (reg & CPLD_PSU_STATUS_PS1_12V_PG) != 0;
		attr_name = dattr->attr.name + strlen(psu1_name) + 1;
	} else {
		present = (reg & CPLD_PSU_STATUS_PS2_PRESENT_L) == 0;
		dc_pg = (reg & CPLD_PSU_STATUS_PS2_12V_PG) != 0;
		attr_name = dattr->attr.name + strlen(psu2_name) + 1;
	}
	if (strcmp(attr_name, "all_ok") == 0) {
		rv = present && dc_pg;
	} else if (strcmp(attr_name, "present") == 0) {
		rv = present;
	} else if (strcmp(attr_name, "dc_ok") == 0) {
		rv = dc_pg;
	}

	return sprintf(buf, "%u\n", rv);
}

static SYSFS_ATTR_RO(psu_pwr1_all_ok,  psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_present, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr1_dc_ok,   psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_all_ok,  psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_present, psu_power_show);
static SYSFS_ATTR_RO(psu_pwr2_dc_ok,   psu_power_show);

/*
 * Module Status Bits
 */
static const uint8_t *m1_type = "module_type_m1";
static const uint8_t *m2_type = "module_type_m2";
static const uint8_t *m1_power_ok = "module_power_ok_m1";
static const uint8_t *m2_power_ok = "module_power_ok_m2";
static const uint8_t *m1_power_en = "module_power_en_m1";
static const uint8_t *m2_power_en = "module_power_en_m2";
static ssize_t module_type_show(struct device *dev,
			        struct device_attribute *dattr,
			        char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	uint8_t reg;
	uint8_t status;
	uint8_t *rv;

	reg = i2c_smbus_read_byte_data(client, CPLD_REG_MODULE_STATUS);

	if (strncmp(dattr->attr.name, m1_type, strlen(m1_type)) == 0) {
		status = (reg & CPLD_MODULE_STATUS_M1_MASK) >> CPLD_MODULE_STATUS_M1_SHIFT;
	} else if (strncmp(dattr->attr.name, m2_type, strlen(m2_type)) == 0) {
		status = (reg & CPLD_MODULE_STATUS_M2_MASK) >> CPLD_MODULE_STATUS_M2_SHIFT;
	} else {
		status = -1;
	}

	switch (status) {
		case CPLD_MODULE_STATUS_2CXP:
			rv = "2xCXP";
			break;
		case CPLD_MODULE_STATUS_2CFP:
			rv = "2xCFP";
			break;
		case CPLD_MODULE_STATUS_4TENGBASET:
			rv = "4x10GBaseT";
			break;
		case CPLD_MODULE_STATUS_6QSFP:
			rv = "6xQSFP";
			break;
		default:
			rv = "Unknown";
			BUG_ON(false);
			break;
	}

	return sprintf(buf, "%s\n", rv);
}

static ssize_t module_power_ok_show(struct device *dev,
			          struct device_attribute *dattr,
			          char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	uint8_t reg;
	bool power_ok;

	reg = i2c_smbus_read_byte_data(client, CPLD_REG_MODULE_STATUS);

	if (strncmp(dattr->attr.name, m1_power_ok, strlen(m1_power_ok)) == 0) {
		power_ok = (reg & CPLD_MODULE_STATUS_M1_PWR_OK) != 0;
	} else if (strncmp(dattr->attr.name, m2_power_ok, strlen(m2_power_ok)) == 0) {
		power_ok = (reg & CPLD_MODULE_STATUS_M1_PWR_OK) != 0;
	} else {
		power_ok = 0;
		BUG_ON(false);
	}

	return sprintf(buf, "%u\n", power_ok);
}

static ssize_t module_power_en_show(struct device *dev,
				    struct device_attribute *dattr,
				    char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	uint8_t reg;
	bool power_en;

	if (strncmp(dattr->attr.name, m1_power_en, strlen(m1_power_en)) == 0) {
		reg = i2c_smbus_read_byte_data(client, CPLD_REG_MODULE_PWR_CONTROL_M1);
		power_en = (reg & CPLD_MODULE_PWR_CONTROL_M1_EN) != 0;
	} else if (strncmp(dattr->attr.name, m2_power_en, strlen(m2_power_en)) == 0) {
		reg = i2c_smbus_read_byte_data(client, CPLD_REG_MODULE_PWR_CONTROL_M2);
		power_en = (reg & CPLD_MODULE_PWR_CONTROL_M2_EN) != 0;
	} else {
		power_en = 0;
		BUG_ON(false);
	}

	return sprintf(buf, "%u\n", power_en);
}

static ssize_t module_power_en_store(struct device *dev,
				     struct device_attribute *dattr,
				     const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	uint8_t reg;
	bool power_en;

	if (count < 1) {
	    return -EINVAL;
	}

	if (buf[0] == '0') {
		power_en = 0;
	} else {
		power_en = 1;
	}

	if (strncmp(dattr->attr.name, m1_power_en, strlen(m1_power_en)) == 0) {
		reg = power_en ? CPLD_MODULE_PWR_CONTROL_M1_EN : 0;
		i2c_smbus_write_byte_data(client, CPLD_REG_MODULE_PWR_CONTROL_M1, reg);
	} else if (strncmp(dattr->attr.name, m2_power_en, strlen(m2_power_en)) == 0) {
		reg = power_en ? CPLD_MODULE_PWR_CONTROL_M2_EN : 0;
		i2c_smbus_write_byte_data(client, CPLD_REG_MODULE_PWR_CONTROL_M2, reg);
	}

	return count;
}

static SYSFS_ATTR_RO(module_type_m1, module_type_show);
static SYSFS_ATTR_RO(module_type_m2, module_type_show);
static SYSFS_ATTR_RO(module_power_ok_m1, module_power_ok_show);
static SYSFS_ATTR_RO(module_power_ok_m2, module_power_ok_show);
static SYSFS_ATTR_RW(module_power_en_m1, module_power_en_show, module_power_en_store);
static SYSFS_ATTR_RW(module_power_en_m2, module_power_en_show, module_power_en_store);

static ssize_t qsfp_present_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	uint32_t present;
	uint8_t reg;

	reg = i2c_smbus_read_byte_data(client, CPLD_REG_QSFP_PRESENT_L_1);
	present = reg;
	reg = i2c_smbus_read_byte_data(client, CPLD_REG_QSFP_PRESENT_L_2);
	present |= reg << 8;
	reg = i2c_smbus_read_byte_data(client, CPLD_REG_QSFP_PRESENT_L_3);
	present |= reg << 16;

	/* only th efirst 20 bits are valid */
	present &= 0xfffff;

	return sprintf(buf, "0x%x\n", present);
}

static ssize_t qsfp_reset_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	uint32_t reset;
	uint8_t reg;

	reg = i2c_smbus_read_byte_data(client, CPLD_REG_QSFP_RST_L_1);
	reset = reg;
	reg = i2c_smbus_read_byte_data(client, CPLD_REG_QSFP_RST_L_2);
	reset |= reg << 8;
	reg = i2c_smbus_read_byte_data(client, CPLD_REG_QSFP_RST_L_3);
	reset |= reg << 16;

	/* only the first 20 bits are valid */
	reset &= 0xfffff;

	return sprintf(buf, "0x%x\n", reset);
}

static ssize_t qsfp_reset_store(struct device *dev,
				struct device_attribute *dattr,
				const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long reset;
	uint8_t reg;
	int rv;

	if (strict_strtoul(buf, 0, &reset) < 0)
		return -EINVAL;

	rv = i2c_smbus_write_byte_data(client, CPLD_REG_QSFP_RST_L_1,
				       (reset >> 0) & 0xff);
	if (rv != 0)
		return rv;
	rv = i2c_smbus_write_byte_data(client, CPLD_REG_QSFP_RST_L_2,
				       (reset >> 8) & 0xff);
	if (rv != 0)
		return rv;

	/*
	 * read-modify-write required because the top 4 bits control a voltage
	 * regulator.  Thanks Accton.
	 */
	reg = i2c_smbus_read_byte_data(client, CPLD_REG_QSFP_RST_L_3);
	if (reg < 0)
		return rv;

	reg &= 0xf0;
	reg |= (reset >> 16) & 0x0f;
	rv = i2c_smbus_write_byte_data(client, CPLD_REG_QSFP_RST_L_3, reg);
	if (rv != 0)
		return rv;
        
	return count;
}

static SYSFS_ATTR_RO(qsfp_present, qsfp_present_show);
static SYSFS_ATTR_RW(qsfp_reset, qsfp_reset_show, qsfp_reset_store);

static const uint8_t *led_alarm = "led_alarm";
static const uint8_t *led_sys = "led_sys";

static ssize_t led_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	const uint8_t *color;
	uint8_t reg;

	reg = i2c_smbus_read_byte_data(client, CPLD_REG_SYS_ALARM_LED);

	if (strncmp(dattr->attr.name, led_alarm, strlen(led_alarm)) == 0) {
		if (reg & CPLD_SYS_ALARM_LED_ALRM_GREEN) {
			color = PLATFORM_LED_GREEN;
		} else if (reg & CPLD_SYS_ALARM_LED_ALRM_RED_BLINK) {
			color = PLATFORM_LED_RED_BLINKING;
		} else {
			color = PLATFORM_LED_OFF;
		}
	} else if (strncmp(dattr->attr.name, led_sys, strlen(led_sys)) == 0) {
		if (reg & CPLD_SYS_ALARM_LED_SYS_GREEN) {
			color = PLATFORM_LED_GREEN;
		} else if (reg & CPLD_SYS_ALARM_LED_SYS_GREEN_BLINK) {
			color = PLATFORM_LED_GREEN_BLINKING;
		} else {
			color = PLATFORM_LED_OFF;
		}
	} else {
		color = "unknown";
		BUG_ON(false);
	}

	return sprintf(buf, "%s\n", color);
}

static ssize_t led_store(struct device *dev,
			 struct device_attribute *dattr,
			 const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	uint8_t reg;
	int rv;

	reg = i2c_smbus_read_byte_data(client, CPLD_REG_SYS_ALARM_LED);

	if (strncmp(dattr->attr.name, led_alarm, strlen(led_alarm)) == 0) {
		reg &= ~CPLD_SYS_ALARM_LED_ALRM_MASK;
		if (strncmp(buf, PLATFORM_LED_GREEN, count) == 0) {
			reg |= CPLD_SYS_ALARM_LED_ALRM_GREEN;
		} else if (strncmp(buf, PLATFORM_LED_RED_BLINKING, count) == 0) {
			reg |= CPLD_SYS_ALARM_LED_ALRM_RED_BLINK;
		} else if (strncmp(buf, PLATFORM_LED_OFF, count) != 0) {
			return -EINVAL;
		}
	} else if (strncmp(dattr->attr.name, led_sys, strlen(led_sys)) == 0) {
		reg &= ~CPLD_SYS_ALARM_LED_SYS_MASK;
		if (strncmp(buf, PLATFORM_LED_GREEN, count) == 0) {
			reg |= CPLD_SYS_ALARM_LED_SYS_GREEN;
		} else if (strncmp(buf, PLATFORM_LED_GREEN_BLINKING, count) == 0) {
			reg |= CPLD_SYS_ALARM_LED_SYS_GREEN_BLINK;
		} else if (strncmp(buf, PLATFORM_LED_OFF, count) != 0) {
			return -EINVAL;
		}
	} else {
		BUG_ON(false);
		return -EINVAL;
	}

	rv = i2c_smbus_write_byte_data(client, CPLD_REG_SYS_ALARM_LED, reg);
	if (rv != 0) {
		return rv;
	}

	return count;
}

static SYSFS_ATTR_RW(led_alarm, led_show, led_store);
static SYSFS_ATTR_RW(led_sys,   led_show, led_store);

static struct attribute *syspld_attrs[] = {
	&dev_attr_code_revision.attr,
	&dev_attr_hw_revision.attr,
	&dev_attr_psu_pwr1_all_ok.attr,
	&dev_attr_psu_pwr1_present.attr,
	&dev_attr_psu_pwr1_dc_ok.attr,
	&dev_attr_psu_pwr2_all_ok.attr,
	&dev_attr_psu_pwr2_present.attr,
	&dev_attr_psu_pwr2_dc_ok.attr,
	&dev_attr_module_type_m1.attr,
	&dev_attr_module_type_m2.attr,
	&dev_attr_module_power_ok_m1.attr,
	&dev_attr_module_power_ok_m2.attr,
	&dev_attr_module_power_en_m1.attr,
	&dev_attr_module_power_en_m2.attr,
	&dev_attr_qsfp_present.attr,
	&dev_attr_qsfp_reset.attr,
	&dev_attr_led_alarm.attr,
	&dev_attr_led_sys.attr,
	NULL
};

static struct attribute_group syspld_attr_group = {
	.attrs = syspld_attrs,
};

static int syspld_remove(struct i2c_client *client)
{
	struct syspld *syspld = i2c_get_clientdata(client);
	int rv = 0;
	int chan;

	sysfs_remove_group(&client->dev.kobj, &syspld_attr_group);

	for (chan = 0; chan < ARRAY_SIZE(syspld->virt_adaps); chan++) {
		if (syspld->virt_adaps[chan]) {
			int removed = i2c_del_mux_adapter(syspld->virt_adaps[chan]);
                        if (removed != 0) {
				dev_err(&client->dev,
				        "failed to remove i2c bus for chan %u\n",
					chan);
				rv = -ENODEV;
			}
                        syspld->virt_adaps[chan] = NULL;
		}
	}

	kfree(syspld);

	return rv;
}

static int syspld_i2c_reg_write(struct i2c_adapter *adap,
                                struct i2c_client *client, u8 val)
{
	static const u8 command = CPLD_REG_I2C_SWITCH_CONTROL;
	int rv;

	if (adap->algo->master_xfer) {
		struct i2c_msg msg = { 0 };
		char buf[2];

		msg.addr = client->addr;
		msg.flags = 0;
		msg.len = 2;
		buf[0] = command;
		buf[1] = val;
		msg.buf = buf;
		rv = adap->algo->master_xfer(adap, &msg, 1);
	} else {
                union i2c_smbus_data data;

                data.byte = val;
                rv = adap->algo->smbus_xfer(adap, client->addr,
					    client->flags,
					    I2C_SMBUS_WRITE,
					    command, I2C_SMBUS_BYTE_DATA, &data);
	}

	return rv;
}

static const uint8_t switch_control_table[] = {
	CPLD_REG_I2C_SWITCH_CONTROL_PORT1,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT2,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT3,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT4,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT5,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT6,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT7,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT8,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT9,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT10,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT11,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT12,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT13,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT14,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT15,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT16,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT17,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT18,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT19,
	CPLD_REG_I2C_SWITCH_CONTROL_PORT20,
	CPLD_REG_I2C_SWITCH_CONTROL_M1,
	CPLD_REG_I2C_SWITCH_CONTROL_M2,
};

static int syspld_i2c_select_chan(struct i2c_adapter *adap, void *client, u32 chan)
{
	struct syspld *syspld = i2c_get_clientdata(client);
	u8 switch_control;
	int rv = 0;

	if (chan >= ARRAY_SIZE(switch_control_table)) {
		printk(KERN_ERR "invalid i2c switch channel: %u\n", chan);
		return -EINVAL;
	}

	switch_control = switch_control_table[chan];
	if (syspld->last_switch_control != switch_control) {
		rv = syspld_i2c_reg_write(adap, client, switch_control);
		if (rv == 0) {
			syspld->last_switch_control = switch_control;
		}
	}

	return rv;
}

static int syspld_i2c_deselect(struct i2c_adapter *adap, void *client, u32 chan)
{
	struct syspld *syspld = i2c_get_clientdata(client);

        syspld->last_switch_control = CPLD_REG_I2C_SWITCH_CONTROL_DISABLE;
        return syspld_i2c_reg_write(adap, client, CPLD_REG_I2C_SWITCH_CONTROL_DISABLE);
}

static int syspld_probe(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
	struct i2c_adapter *parent = to_i2c_adapter(client->dev.parent);
	struct syspld *syspld;
	int chan;
	int rv;

	rv  = i2c_smbus_write_byte_data(client, CPLD_REG_I2C_SWITCH_CONTROL, 0);
	if (rv < 0) {
		dev_err(&client->dev, "failed to initialize i2c switch\n");
		return rv;
	}

	syspld = kzalloc(sizeof(struct syspld), GFP_KERNEL);
	if (!syspld) {
		return -ENOMEM;
	}

	i2c_set_clientdata(client, syspld);

	for (chan = 0; chan < ARRAY_SIZE(syspld->virt_adaps); chan++) {
		struct i2c_adapter *virt_adap;
		virt_adap = i2c_add_mux_adapter(parent, &client->dev, client, 0, chan,
                                                syspld_i2c_select_chan, syspld_i2c_deselect);
		if (!virt_adap) {
			dev_err(&client->dev,
			        "failed to add mux adapter for channel %u\n",
				chan);
			rv = -ENODEV;
			goto done;
		}
		syspld->virt_adaps[chan] = virt_adap;
	}

	rv = sysfs_create_group(&client->dev.kobj, &syspld_attr_group);
	if (rv < 0) {
		dev_err(&client->dev, "failed to create sysfs group\n");
		goto done;
	}

	dev_info(&client->dev, "registered %u multiplexed busses for I2C %s\n",
		 chan, client->name);

done:
	if (rv < 0) {
		for (chan = 0; chan < ARRAY_SIZE(syspld->virt_adaps); chan++) {
			if (syspld->virt_adaps[chan]) {
				i2c_del_mux_adapter(syspld->virt_adaps[chan]);
			}
		}
		kfree(syspld);
	}
	return rv;
}

static struct i2c_driver syspld_driver = {
        .driver = {
                .name   = "as670x_32x_syspld",
                .owner  = THIS_MODULE,
        },
        .probe          = syspld_probe,
        .remove         = syspld_remove,
        .id_table       = syspld_id,
};

static int __init syspld_init(void)
{
	return i2c_add_driver(&syspld_driver);
}

static void __exit syspld_exit(void)
{
	i2c_del_driver(&syspld_driver);
}

MODULE_AUTHOR("Dustin Byford <dustin@cumulusnetworks.com>");
MODULE_DESCRIPTION("SYSPLD driver for Accton Technology Corporation AS670x-32X");
MODULE_LICENSE("GPL");

module_init(syspld_init);
module_exit(syspld_exit);
