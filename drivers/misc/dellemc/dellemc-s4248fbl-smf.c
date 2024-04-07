/*
 * Copyright (C) 2019 Cumulus Networks, Inc.
 *
 * Smartfusion driver for Dell EMC S4248FBL
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

/* This is modelled after DellEMC-S5048F driver */

#include <stddef.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/sysfs.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include "platform-defs.h"
#include "dellemc-s4248fbl-smf.h"

#define DRIVER_NAME "dellemc_s4248fbl_smf"
#define DRIVER_VERSION "1.0"

static DEFINE_MUTEX(dellemc_s4248fbl_smf_mutex);

static uint8_t *smf_regs;

/* SMF Access via LPC registers */
static inline uint8_t smf_rd(u32 reg)
{
	return ioread8(smf_regs + reg - SMF_IO_BASE);
}

static inline void smf_wr(u32 reg, uint8_t data)
{
	iowrite8(data, smf_regs + reg - SMF_IO_BASE);
}

/* SMF sysfs access */
static ssize_t smf_show(struct device *dev,
			struct device_attribute *dattr,
			char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = attr->index;

	return sprintf(buf, "0x%x\n", smf_rd(reg));
}

static SENSOR_DEVICE_ATTR_RO(smf_ver,           smf_show, SMF_VER);
static SENSOR_DEVICE_ATTR_RO(smf_board_type,    smf_show, SMF_BOARD_TYPE);
static SENSOR_DEVICE_ATTR_RO(smf_scratch,       smf_show, SMF_SW_SCRATCH);
static SENSOR_DEVICE_ATTR_RO(smf_boot_ok,       smf_show, SMF_BOOT_OK);
static SENSOR_DEVICE_ATTR_RO(smf_mss_sta,       smf_show, SMF_MSS_STA);
static SENSOR_DEVICE_ATTR_RO(smf_wd_timer,      smf_show, SMF_WD_WID);
static SENSOR_DEVICE_ATTR_RO(smf_wd_mask,       smf_show, SMF_WD_MASK);
static SENSOR_DEVICE_ATTR_RO(smf_por_source,    smf_show, SMF_POR_SOURCE);
static SENSOR_DEVICE_ATTR_RO(smf_rst_source,    smf_show, SMF_RST_SOURCE);
static SENSOR_DEVICE_ATTR_RO(smf_sep_rst,       smf_show, SMF_SEP_RST);
static SENSOR_DEVICE_ATTR_RO(smf_cpu_rst_ctrl,  smf_show, SMF_CPU_RST_CTRL);
static SENSOR_DEVICE_ATTR_RO(smf_ram_addr_h,    smf_show, SMF_RAM_ADDR_H);
static SENSOR_DEVICE_ATTR_RO(smf_ram_addr_l,    smf_show, SMF_RAM_ADDR_L);
static SENSOR_DEVICE_ATTR_RO(smf_ram_rd,        smf_show, SMF_RAM_R_DATA);
static SENSOR_DEVICE_ATTR_RO(smf_ram_wr,        smf_show, SMF_RAM_W_DATA);
static SENSOR_DEVICE_ATTR_RO(smf_cpu_eeprom_wp, smf_show, SMF_CPU_EEPROM_WP);
static SENSOR_DEVICE_ATTR_RO(smf_tpm_status,    smf_show, SMF_TPM_STA_ID);

/* SMF Mailbox Access */
static u8 mb_rd(u32 reg)
{
	u8 hbyte, lbyte;
	u8 rdata;

	hbyte = (reg >> 8) & 0xff;
	lbyte = reg & 0xff;
	smf_wr(SMF_RAM_ADDR_H, hbyte);
	smf_wr(SMF_RAM_ADDR_L, lbyte);

	rdata = smf_rd(SMF_RAM_R_DATA);
	return rdata;
}

/* SMF Mailbox sysfs access */
static ssize_t mb_show(struct device *dev,
		       struct device_attribute *dattr,
		       char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = attr->index;
	ssize_t ret;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	ret = sprintf(buf, "0x%x\n", mb_rd(reg));
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	return ret;
}

static SENSOR_DEVICE_ATTR_RO(protocol_ver,     mb_show,	SMF_PROTOCOL_VER);
static SENSOR_DEVICE_ATTR_RO(mss_ver,	       mb_show,	SMF_MSS_VER);
static SENSOR_DEVICE_ATTR_RO(smf_flag,	       mb_show,	SMF_SMF_FLAG);
static SENSOR_DEVICE_ATTR_RO(cpu_flag,	       mb_show,	SMF_CPU_FLAG);
static SENSOR_DEVICE_ATTR_RO(device_status,    mb_show,	SMF_DEVICE_STATUS);
static SENSOR_DEVICE_ATTR_RO(mb_fan_algo,      mb_show,
			     SMF_FAN_CONTROL_ALGO_FLAG);

static SENSOR_DEVICE_ATTR_RO(mb_system_status, mb_show, SMF_MB_SYSTEM_STATUS);

static SENSOR_DEVICE_ATTR_RO(mb_scan_lm75_1,   mb_show,
			     SMF_MB_SCAN_LM75_BASE);
static SENSOR_DEVICE_ATTR_RO(mb_scan_lm75_2,   mb_show,
			     SMF_MB_SCAN_LM75_BASE + 1);
static SENSOR_DEVICE_ATTR_RO(mb_scan_lm75_3,   mb_show,
			     SMF_MB_SCAN_LM75_BASE + 2);
static SENSOR_DEVICE_ATTR_RO(mb_scan_lm75_4,   mb_show,
			     SMF_MB_SCAN_LM75_BASE + 3);
static SENSOR_DEVICE_ATTR_RO(mb_scan_lm75_5,   mb_show,
			     SMF_MB_SCAN_LM75_BASE + 4);
static SENSOR_DEVICE_ATTR_RO(mb_scan_lm75_6,   mb_show,
			     SMF_MB_SCAN_LM75_BASE + 5);
static SENSOR_DEVICE_ATTR_RO(mb_scan_lm75_7,   mb_show,
			     SMF_MB_SCAN_LM75_BASE + 6);
static SENSOR_DEVICE_ATTR_RO(mb_scan_lm75_8,   mb_show,
			     SMF_MB_SCAN_LM75_BASE + 7);
static SENSOR_DEVICE_ATTR_RO(mb_scan_lm75_9,   mb_show,
			     SMF_MB_SCAN_LM75_BASE + 8);

 /* Temp Sensors */
static ssize_t mb_temp_show(struct device *dev,
			    struct device_attribute *dattr,
			    char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = attr->index;
	int val_h, val_l;
	int val;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val_h = mb_rd(reg);
	val_l = mb_rd(reg + 1);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = (val_h << 8) | val_l;

	if (val == 0xffff)
		return -EINVAL;
	return sprintf(buf, "%d\n", val * 100);
}

static SENSOR_DEVICE_ATTR_RO(temp1_input, mb_temp_show,
			     SMF_TEMP_SENSOR_BASE + 0x00);
static SENSOR_DEVICE_ATTR_RO(temp2_input, mb_temp_show,
			     SMF_TEMP_SENSOR_BASE + 0x02);
static SENSOR_DEVICE_ATTR_RO(temp3_input, mb_temp_show,
			     SMF_TEMP_SENSOR_BASE + 0x04);
static SENSOR_DEVICE_ATTR_RO(temp4_input, mb_temp_show,
			     SMF_TEMP_SENSOR_BASE + 0x06);
static SENSOR_DEVICE_ATTR_RO(temp6_input, mb_temp_show,
			     SMF_TEMP_SENSOR_BASE + 0x0a);
static SENSOR_DEVICE_ATTR_RO(temp9_input, mb_temp_show,
			     SMF_TEMP_SENSOR_BASE + 0x10);

static SENSOR_DEVICE_ATTR_RO(temp1_crit,  mb_temp_show,
			     SMF_TEMP_HW_SHUT_BASE + 0x00);
static SENSOR_DEVICE_ATTR_RO(temp8_crit,  mb_temp_show,
			     SMF_TEMP_SW_SHUT_BASE + 0x38);

static SENSOR_DEVICE_ATTR_RO(temp1_max,	  mb_temp_show,
			     SMF_TEMP_MAJOR_ALARM_BASE + 0x00);
static SENSOR_DEVICE_ATTR_RO(temp3_max,	  mb_temp_show,
			     SMF_TEMP_MAJOR_ALARM_BASE + 0x10);
static SENSOR_DEVICE_ATTR_RO(temp8_max,	  mb_temp_show,
			     SMF_TEMP_MAJOR_ALARM_BASE + 0x38);

static ssize_t mb_temp_fault_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = attr->index;
	int val;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val = mb_rd(reg);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	if (val != 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}

static SENSOR_DEVICE_ATTR_RO(temp1_fault, mb_temp_fault_show,
			     SMF_TEMP_STATUS_BASE + 0x00);
static SENSOR_DEVICE_ATTR_RO(temp2_fault, mb_temp_fault_show,
			     SMF_TEMP_STATUS_BASE + 0x01);
static SENSOR_DEVICE_ATTR_RO(temp3_fault, mb_temp_fault_show,
			     SMF_TEMP_STATUS_BASE + 0x02);
static SENSOR_DEVICE_ATTR_RO(temp4_fault, mb_temp_fault_show,
			     SMF_TEMP_STATUS_BASE + 0x03);
static SENSOR_DEVICE_ATTR_RO(temp5_fault, mb_temp_fault_show,
			     SMF_TEMP_STATUS_BASE + 0x04);
static SENSOR_DEVICE_ATTR_RO(temp6_fault, mb_temp_fault_show,
			     SMF_TEMP_STATUS_BASE + 0x05);
static SENSOR_DEVICE_ATTR_RO(temp7_fault, mb_temp_fault_show,
			     SMF_TEMP_STATUS_BASE + 0x06);
static SENSOR_DEVICE_ATTR_RO(temp8_fault, mb_temp_fault_show,
			     SMF_TEMP_STATUS_BASE + 0x07);
static SENSOR_DEVICE_ATTR_RO(temp9_fault, mb_temp_fault_show,
			     SMF_TEMP_STATUS_BASE + 0x08);

/* Fan Sensors
 *
 * There are four fan trays, numbered 1 to 4 on the silkscreen.	 Internally,
 * the fan trays are numbered 1, 2, 4, and 5.  Each fan tray has two fans,
 * numbered 1 to 8.  Internally these fans are numbered 1, 2, 4, 5, 6, 7, 9,
 * and 10.
 *
 * The following table shows the mapping:
 *
 *   ------------------------------------------------------------
 *   | Fan Tray | Internal Number ||  Fans   | Internal Numbers |
 *   |----------|-----------------||---------|------------------|
 *   |	  1	|	 1	  || 1 and 2 |	   1 and 2	|
 *   |	  2	|	 2	  || 3 and 4 |	   4 and 5	|
 *   |	  3	|	 4	  || 5 and 6 |	   6 and 7	|
 *   |	  4	|	 5	  || 7 and 8 |	   9 and 10	|
 *   ------------------------------------------------------------
 */

static ssize_t mb_dec_byte_show(struct device *dev,
				struct device_attribute *dattr,
				char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = attr->index;
	ssize_t ret;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	ret = sprintf(buf, "%d\n", mb_rd(reg));
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	return ret;
}

static SENSOR_DEVICE_ATTR_RO(num_fan_trays, mb_dec_byte_show,
			     SMF_MAX_FAN_TRAYS);
static SENSOR_DEVICE_ATTR_RO(fans_per_tray, mb_dec_byte_show,
			     SMF_FANS_PER_TRAY);
static SENSOR_DEVICE_ATTR_RO(fan_max_speed, mb_dec_byte_show,
			     SMF_MAX_FAN_SET_SPEED);

static ssize_t mb_dec_word_show(struct device *dev,
				struct device_attribute *dattr,
				char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = attr->index;
	u32 val;
	u8 val_h, val_l;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val_h = mb_rd(reg);
	val_l = mb_rd(reg + 1);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);
	
	val = (val_h << 8) | val_l;

	if (val == 0xffff)
		return -EINVAL;

	return sprintf(buf, "%d\n", val);
}

static SENSOR_DEVICE_ATTR_RO(fan1_input, mb_dec_word_show,
			     SMF_FAN_TRAY_FAN_SPEED_BASE + 0x00);
static SENSOR_DEVICE_ATTR_RO(fan2_input, mb_dec_word_show,
			     SMF_FAN_TRAY_FAN_SPEED_BASE + 0x02);
static SENSOR_DEVICE_ATTR_RO(fan3_input, mb_dec_word_show,
			     SMF_FAN_TRAY_FAN_SPEED_BASE + 0x04);
static SENSOR_DEVICE_ATTR_RO(fan4_input, mb_dec_word_show,
			     SMF_FAN_TRAY_FAN_SPEED_BASE + 0x06);
static SENSOR_DEVICE_ATTR_RO(fan5_input, mb_dec_word_show,
			     SMF_FAN_TRAY_FAN_SPEED_BASE + 0x0c);
static SENSOR_DEVICE_ATTR_RO(fan6_input, mb_dec_word_show,
			     SMF_FAN_TRAY_FAN_SPEED_BASE + 0x0e);
static SENSOR_DEVICE_ATTR_RO(fan7_input, mb_dec_word_show,
			     SMF_FAN_TRAY_FAN_SPEED_BASE + 0x10);
static SENSOR_DEVICE_ATTR_RO(fan8_input, mb_dec_word_show,
			     SMF_FAN_TRAY_FAN_SPEED_BASE + 0x12);

static ssize_t mb_fan_tray_present_show(struct device *dev,
					struct device_attribute *dattr,
					char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_FAN_PRESENT;
	u8 idx = attr->index;
	u8 val;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val = mb_rd(reg);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = val & (1 << idx);
	if (val == 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}

static SENSOR_DEVICE_ATTR_RO(fan1_present, mb_fan_tray_present_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan2_present, mb_fan_tray_present_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan3_present, mb_fan_tray_present_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan4_present, mb_fan_tray_present_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan5_present, mb_fan_tray_present_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan6_present, mb_fan_tray_present_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan7_present, mb_fan_tray_present_show, 4);
static SENSOR_DEVICE_ATTR_RO(fan8_present, mb_fan_tray_present_show, 4);

static ssize_t mb_fan_ok_show(struct device *dev,
			      struct device_attribute *dattr,
			      char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_FAN_STATUS;
	u32 idx = attr->index;
	u32 val;
	u8 val_h, val_l;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val_h = mb_rd(reg);
	val_l = mb_rd(reg + 1);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = (val_h << 8) | val_l;
	val = (val & (1 << idx));
	if (val == 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}
static SENSOR_DEVICE_ATTR_RO(fan1_all_ok, mb_fan_ok_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan2_all_ok, mb_fan_ok_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan3_all_ok, mb_fan_ok_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan4_all_ok, mb_fan_ok_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan5_all_ok, mb_fan_ok_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan6_all_ok, mb_fan_ok_show, 6);
static SENSOR_DEVICE_ATTR_RO(fan7_all_ok, mb_fan_ok_show, 8);
static SENSOR_DEVICE_ATTR_RO(fan8_all_ok, mb_fan_ok_show, 9);

static ssize_t mb_fan_tray_ok_show(struct device *dev,
				   struct device_attribute *dattr,
				   char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_FAN_STATUS;
	u32 idx = attr->index;
	u32 val;
	u8 val_h, val_l;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val_h = mb_rd(reg);
	val_l = mb_rd(reg + 1);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = (val_h << 8) | val_l;
	val = (val & (1 << idx));
	if (val == 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}
static SENSOR_DEVICE_ATTR_RO(fan_tray1_all_ok, mb_fan_tray_ok_show,
			     SMF_FAN_TRAY1_ALL_OK_MASK);
static SENSOR_DEVICE_ATTR_RO(fan_tray2_all_ok, mb_fan_tray_ok_show,
			     SMF_FAN_TRAY2_ALL_OK_MASK);
static SENSOR_DEVICE_ATTR_RO(fan_tray3_all_ok, mb_fan_tray_ok_show,
			     SMF_FAN_TRAY3_ALL_OK_MASK);
static SENSOR_DEVICE_ATTR_RO(fan_tray4_all_ok, mb_fan_tray_ok_show,
			     SMF_FAN_TRAY4_ALL_OK_MASK);

static ssize_t mb_fan_tray_f2b_show(struct device *dev,
				    struct device_attribute *dattr,
				    char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_FAN_F2B;
	u32 idx = attr->index;
	u8 val;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val = mb_rd(reg);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = val & (1 << idx);
	if (val == 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}
static SENSOR_DEVICE_ATTR_RO(fan1_f2b, mb_fan_tray_f2b_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan2_f2b, mb_fan_tray_f2b_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan3_f2b, mb_fan_tray_f2b_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan4_f2b, mb_fan_tray_f2b_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan5_f2b, mb_fan_tray_f2b_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan6_f2b, mb_fan_tray_f2b_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan7_f2b, mb_fan_tray_f2b_show, 4);
static SENSOR_DEVICE_ATTR_RO(fan8_f2b, mb_fan_tray_f2b_show, 4);

static ssize_t mb_fan_tray_serial_num_show(struct device *dev,
					   struct device_attribute *dattr,
					   char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_FAN_TRAY_SERIAL_NUM_BASE +
		(attr->index * SMF_FAN_BLOCK_SIZE);
	char val[SMF_FAN_TRAY_SERIAL_NUM_SIZE + 1];
	int i;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	for (i = 0; i < SMF_FAN_TRAY_SERIAL_NUM_SIZE; i++)
		val[i] = mb_rd(reg + i);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val[SMF_FAN_TRAY_SERIAL_NUM_SIZE] = '\0';
	return sprintf(buf, "%s\n", val);
}

static SENSOR_DEVICE_ATTR_RO(fan1_serial_num,
			     mb_fan_tray_serial_num_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan2_serial_num,
			     mb_fan_tray_serial_num_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan3_serial_num,
			     mb_fan_tray_serial_num_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan4_serial_num,
			     mb_fan_tray_serial_num_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan5_serial_num,
			     mb_fan_tray_serial_num_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan6_serial_num,
			     mb_fan_tray_serial_num_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan7_serial_num,
			     mb_fan_tray_serial_num_show, 4);
static SENSOR_DEVICE_ATTR_RO(fan8_serial_num,
			     mb_fan_tray_serial_num_show, 4);

static ssize_t mb_fan_tray_part_num_show(struct device *dev,
					 struct device_attribute *dattr,
					 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_FAN_TRAY_PART_NUM_BASE +
		(attr->index * SMF_FAN_BLOCK_SIZE);
	char val[SMF_FAN_TRAY_PART_NUM_SIZE + 1];
	int i;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	for (i = 0; i < SMF_FAN_TRAY_PART_NUM_SIZE; i++)
		val[i] = mb_rd(reg + i);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val[SMF_FAN_TRAY_PART_NUM_SIZE] = '\0';
	return sprintf(buf, "%s\n", val);
}
static SENSOR_DEVICE_ATTR_RO(fan1_part_num, mb_fan_tray_part_num_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan2_part_num, mb_fan_tray_part_num_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan3_part_num, mb_fan_tray_part_num_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan4_part_num, mb_fan_tray_part_num_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan5_part_num, mb_fan_tray_part_num_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan6_part_num, mb_fan_tray_part_num_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan7_part_num, mb_fan_tray_part_num_show, 4);
static SENSOR_DEVICE_ATTR_RO(fan8_part_num, mb_fan_tray_part_num_show, 4);

static ssize_t mb_fan_tray_label_rev_show(struct device *dev,
					  struct device_attribute *dattr,
					  char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_FAN_TRAY_LABEL_REV_BASE +
		(attr->index * SMF_FAN_BLOCK_SIZE);
	char val[SMF_FAN_TRAY_LABEL_REV_SIZE + 1];
	int i;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	for (i = 0; i < SMF_FAN_TRAY_LABEL_REV_SIZE; i++)
		val[i] = mb_rd(reg + i);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val[SMF_FAN_TRAY_LABEL_REV_SIZE] = '\0';
	return sprintf(buf, "%s\n", val);
}
static SENSOR_DEVICE_ATTR_RO(fan1_label_rev,
			     mb_fan_tray_label_rev_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan2_label_rev,
			     mb_fan_tray_label_rev_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan3_label_rev,
			     mb_fan_tray_label_rev_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan4_label_rev,
			     mb_fan_tray_label_rev_show, 1);
static SENSOR_DEVICE_ATTR_RO(fan5_label_rev,
			     mb_fan_tray_label_rev_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan6_label_rev,
			     mb_fan_tray_label_rev_show, 3);
static SENSOR_DEVICE_ATTR_RO(fan7_label_rev,
			     mb_fan_tray_label_rev_show, 4);
static SENSOR_DEVICE_ATTR_RO(fan8_label_rev,
			     mb_fan_tray_label_rev_show, 4);

/* PSU */
static ssize_t mb_psu_power_show(struct device *dev,
				 struct device_attribute *dattr,
				 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = attr->index;
	int val_h, val_l;
	int val;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val_h = mb_rd(reg);
	val_l = mb_rd(reg + 1);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = (val_h << 8) | val_l;
	if (val == 0xffff)
		return -EINVAL;
	return sprintf(buf, "%d\n", val / 10);
}
static SENSOR_DEVICE_ATTR_RO(psu1_max, mb_psu_power_show,
			     SMF_PSU_MAX_POWER_BASE + (0 * SMF_PSU_BLOCK_SIZE));
static SENSOR_DEVICE_ATTR_RO(psu2_max, mb_psu_power_show,
			     SMF_PSU_MAX_POWER_BASE + (1 * SMF_PSU_BLOCK_SIZE));
static SENSOR_DEVICE_ATTR_RO(psu1_input, mb_psu_power_show,
			     SMF_PSU_INPUT_POWER_BASE +
			     (0 * SMF_PSU_BLOCK_SIZE));
static SENSOR_DEVICE_ATTR_RO(psu2_input, mb_psu_power_show,
			     SMF_PSU_INPUT_POWER_BASE +
			     (1 * SMF_PSU_BLOCK_SIZE));
static SENSOR_DEVICE_ATTR_RO(psu1_output, mb_psu_power_show,
			     SMF_PSU_OUTPUT_POWER_BASE +
			     (0 * SMF_PSU_BLOCK_SIZE));
static SENSOR_DEVICE_ATTR_RO(psu2_output, mb_psu_power_show,
			     SMF_PSU_OUTPUT_POWER_BASE +
			     (1 * SMF_PSU_BLOCK_SIZE));

static ssize_t mb_psu_present_show(struct device *dev,
				   struct device_attribute *dattr,
				   char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_STATUS_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);
	u8 val;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val = mb_rd(reg);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = val & SMF_PSU_PRESENT_N_MASK;
	if (val == 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}

static SENSOR_DEVICE_ATTR_RO(psu1_present, mb_psu_present_show, 0);
static SENSOR_DEVICE_ATTR_RO(psu2_present, mb_psu_present_show, 1);

static ssize_t mb_psu_ok_show(struct device *dev,
			      struct device_attribute *dattr,
			      char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_STATUS_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);
	u8 val;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val = mb_rd(reg);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = val & SMF_PSU_ALL_OK_N_MASK;
	if (val == 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}

static SENSOR_DEVICE_ATTR_RO(psu_pwr1_all_ok, mb_psu_ok_show, 0);
static SENSOR_DEVICE_ATTR_RO(psu_pwr2_all_ok, mb_psu_ok_show, 1);

/*
 * When a PSU is not installed or not powered on, the spec says reading the PSU
 * temp sensor should return 0xFFFF, but we are seeing 0xFFFB instead.
 *
 * Rather than looking at the return value for an invalid read, we will first
 * check the present bit.  If it shows the PSU is not present, then don't
 * bother reading the temp sensor.
 *
 */
static ssize_t mb_psu_temp_show(struct device *dev,
				struct device_attribute *dattr,
				char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg;
	u8 val_h, val_l;
	int val;

	reg = SMF_PSU_STATUS_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val = mb_rd(reg);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = val & SMF_PSU_PRESENT_N_MASK;
	if (val != 0)
		return -EINVAL;

	reg = SMF_PSU_TEMP_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val_h = mb_rd(reg);
	val_l = mb_rd(reg + 1);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = (val_h << 8) | val_l;
	return sprintf(buf, "%d\n", val * 100);
}
static SENSOR_DEVICE_ATTR_RO(temp11_input, mb_psu_temp_show, 0);
static SENSOR_DEVICE_ATTR_RO(temp12_input, mb_psu_temp_show, 1);

static ssize_t mb_psu_fan_show(struct device *dev,
			       struct device_attribute *dattr,
			       char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg;
	u8 val_h, val_l;
	int val;

	reg = SMF_PSU_FAN_STATUS_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val = mb_rd(reg);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = val & SMF_PSU_FAN_PRESENT_N_MASK;
	if (val != 0)
		return -EINVAL;

	reg = SMF_PSU_FAN_SPEED_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val_h = mb_rd(reg);
	val_l = mb_rd(reg + 1);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = (val_h << 8) | val_l;
	if (val == 0xffff)
		return -EINVAL;
	return sprintf(buf, "%d\n", val);
}
static SENSOR_DEVICE_ATTR_RO(fan9_input,  mb_psu_fan_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan10_input, mb_psu_fan_show, 1);

static ssize_t mb_psu_fan_present_show(struct device *dev,
				       struct device_attribute *dattr,
				       char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_FAN_STATUS_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);
	u8 val;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val = mb_rd(reg);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = val & SMF_PSU_FAN_PRESENT_N_MASK;
	if (val == 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}
static SENSOR_DEVICE_ATTR_RO(fan9_present,  mb_psu_fan_present_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan10_present, mb_psu_fan_present_show, 1);

static ssize_t mb_psu_fan_ok_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_FAN_STATUS_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);
	u8 val;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val = mb_rd(reg);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = val & SMF_PSU_FAN_ALL_OK_N_MASK;
	if (val == 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}
static SENSOR_DEVICE_ATTR_RO(fan9_all_ok,  mb_psu_fan_ok_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan10_all_ok, mb_psu_fan_ok_show, 1);

static ssize_t mb_psu_fan_f2b_show(struct device *dev,
				   struct device_attribute *dattr,
				   char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_FAN_STATUS_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);
	u8 val;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	val = mb_rd(reg);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val = val & SMF_PSU_FAN_F2B_MASK;
	return sprintf(buf, "%d\n", val);
}
static SENSOR_DEVICE_ATTR_RO(fan9_f2b,	mb_psu_fan_f2b_show, 0);
static SENSOR_DEVICE_ATTR_RO(fan10_f2b, mb_psu_fan_f2b_show, 1);

static ssize_t mb_psu_country_code_show(struct device *dev,
					struct device_attribute *dattr,
					char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_COUNTRY_CODE_BASE +
		(attr->index * SMF_PSU_BLOCK_SIZE);
	char val[SMF_PSU_COUNTRY_CODE_SIZE + 1];
	int i;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	for (i = 0; i < SMF_PSU_COUNTRY_CODE_SIZE; i++)
		val[i] = mb_rd(reg + i);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val[SMF_PSU_COUNTRY_CODE_SIZE] = '\0';
	return sprintf(buf, "%s\n", val);
}
static SENSOR_DEVICE_ATTR_RO(psu1_country_code, mb_psu_country_code_show, 0);
static SENSOR_DEVICE_ATTR_RO(psu2_country_code, mb_psu_country_code_show, 1);

static ssize_t mb_psu_part_num_show(struct device *dev,
				    struct device_attribute *dattr,
				    char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_PART_NUM_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);
	char val[SMF_PSU_PART_NUM_SIZE + 1];
	int i;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	for (i = 0; i < SMF_PSU_PART_NUM_SIZE; i++)
		val[i] = mb_rd(reg + i);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val[SMF_PSU_PART_NUM_SIZE] = '\0';
	return sprintf(buf, "%s\n", val);
}
static SENSOR_DEVICE_ATTR_RO(psu1_part_num, mb_psu_part_num_show, 0);
static SENSOR_DEVICE_ATTR_RO(psu2_part_num, mb_psu_part_num_show, 1);

static ssize_t mb_psu_mfg_id_show(struct device *dev,
				  struct device_attribute *dattr,
				  char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_MFG_ID_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);
	char val[SMF_PSU_MFG_ID_SIZE + 1];
	int i;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	for (i = 0; i < SMF_PSU_MFG_ID_SIZE; i++)
		val[i] = mb_rd(reg + i);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val[SMF_PSU_MFG_ID_SIZE] = '\0';
	return sprintf(buf, "%s\n", val);
}
static SENSOR_DEVICE_ATTR_RO(psu1_mfg_id, mb_psu_mfg_id_show, 0);
static SENSOR_DEVICE_ATTR_RO(psu2_mfg_id, mb_psu_mfg_id_show, 1);

static ssize_t mb_psu_mfg_date_show(struct device *dev,
				    struct device_attribute *dattr,
				    char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_MFG_DATE_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);
	char val[SMF_PSU_MFG_DATE_SIZE + 1];
	int i;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	for (i = 0; i < SMF_PSU_MFG_DATE_SIZE; i++)
		val[i] = mb_rd(reg + i);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val[SMF_PSU_MFG_DATE_SIZE] = '\0';
	return sprintf(buf, "%s\n", val);
}
static SENSOR_DEVICE_ATTR_RO(psu1_mfg_date, mb_psu_mfg_date_show, 0);
static SENSOR_DEVICE_ATTR_RO(psu2_mfg_date, mb_psu_mfg_date_show, 1);

static ssize_t mb_psu_serial_num_show(struct device *dev,
				      struct device_attribute *dattr,
				      char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_SERIAL_NUM_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);
	char val[SMF_PSU_SERIAL_NUM_SIZE + 1];
	int i;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	for (i = 0; i < SMF_PSU_SERIAL_NUM_SIZE; i++)
		val[i] = mb_rd(reg + i);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val[SMF_PSU_SERIAL_NUM_SIZE] = '\0';
	return sprintf(buf, "%s\n", val);
}
static SENSOR_DEVICE_ATTR_RO(psu1_serial_num, mb_psu_serial_num_show, 0);
static SENSOR_DEVICE_ATTR_RO(psu2_serial_num, mb_psu_serial_num_show, 1);

static ssize_t mb_psu_service_tag_show(struct device *dev,
				       struct device_attribute *dattr,
				       char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_SERVICE_TAG_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);
	char val[SMF_PSU_SERVICE_TAG_SIZE + 1];
	int i;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	for (i = 0; i < SMF_PSU_SERVICE_TAG_SIZE; i++)
		val[i] = mb_rd(reg + i);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val[SMF_PSU_SERVICE_TAG_SIZE] = '\0';
	return sprintf(buf, "%s\n", val);
}
static SENSOR_DEVICE_ATTR_RO(psu1_service_tag, mb_psu_service_tag_show, 0);
static SENSOR_DEVICE_ATTR_RO(psu2_service_tag, mb_psu_service_tag_show, 1);

static ssize_t mb_psu_label_rev_show(struct device *dev,
				     struct device_attribute *dattr,
				     char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = SMF_PSU_LABEL_REV_BASE + (attr->index * SMF_PSU_BLOCK_SIZE);
	char val[SMF_PSU_LABEL_REV_SIZE + 1];
	int i;

	mutex_lock(&dellemc_s4248fbl_smf_mutex);
	for (i = 0; i < SMF_PSU_LABEL_REV_SIZE; i++)
		val[i] = mb_rd(reg + i);
	mutex_unlock(&dellemc_s4248fbl_smf_mutex);

	val[SMF_PSU_LABEL_REV_SIZE] = '\0';
	return sprintf(buf, "%s\n", val);
}
static SENSOR_DEVICE_ATTR_RO(psu1_label_rev, mb_psu_label_rev_show, 0);
static SENSOR_DEVICE_ATTR_RO(psu2_label_rev, mb_psu_label_rev_show, 1);

static struct attribute *smf_attrs[] = {
       /* &sensor_dev_attr_name.dev_attr.attr, */
	&sensor_dev_attr_smf_ver.dev_attr.attr,
	&sensor_dev_attr_smf_board_type.dev_attr.attr,
	&sensor_dev_attr_smf_scratch.dev_attr.attr,
	&sensor_dev_attr_smf_boot_ok.dev_attr.attr,
	&sensor_dev_attr_smf_mss_sta.dev_attr.attr,
	&sensor_dev_attr_smf_wd_timer.dev_attr.attr,
	&sensor_dev_attr_smf_wd_mask.dev_attr.attr,
	&sensor_dev_attr_smf_por_source.dev_attr.attr,
	&sensor_dev_attr_smf_rst_source.dev_attr.attr,
	&sensor_dev_attr_smf_sep_rst.dev_attr.attr,
	&sensor_dev_attr_smf_cpu_rst_ctrl.dev_attr.attr,
	&sensor_dev_attr_smf_ram_addr_h.dev_attr.attr,
	&sensor_dev_attr_smf_ram_addr_l.dev_attr.attr,
	&sensor_dev_attr_smf_ram_rd.dev_attr.attr,
	&sensor_dev_attr_smf_ram_wr.dev_attr.attr,
	&sensor_dev_attr_smf_cpu_eeprom_wp.dev_attr.attr,
	&sensor_dev_attr_smf_tpm_status.dev_attr.attr,

	&sensor_dev_attr_protocol_ver.dev_attr.attr,
	&sensor_dev_attr_mss_ver.dev_attr.attr,

	&sensor_dev_attr_smf_flag.dev_attr.attr,
	&sensor_dev_attr_cpu_flag.dev_attr.attr,
	&sensor_dev_attr_device_status.dev_attr.attr,

	&sensor_dev_attr_mb_fan_algo.dev_attr.attr,
	&sensor_dev_attr_mb_system_status.dev_attr.attr,

	&sensor_dev_attr_mb_scan_lm75_1.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_2.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_3.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_4.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_5.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_6.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_7.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_8.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_9.dev_attr.attr,

	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp4_input.dev_attr.attr,
	&sensor_dev_attr_temp6_input.dev_attr.attr,
	&sensor_dev_attr_temp9_input.dev_attr.attr,

	&sensor_dev_attr_temp1_fault.dev_attr.attr,
	&sensor_dev_attr_temp2_fault.dev_attr.attr,
	&sensor_dev_attr_temp3_fault.dev_attr.attr,
	&sensor_dev_attr_temp4_fault.dev_attr.attr,
	&sensor_dev_attr_temp5_fault.dev_attr.attr,
	&sensor_dev_attr_temp6_fault.dev_attr.attr,
	&sensor_dev_attr_temp7_fault.dev_attr.attr,
	&sensor_dev_attr_temp8_fault.dev_attr.attr,
	&sensor_dev_attr_temp9_fault.dev_attr.attr,

	&sensor_dev_attr_temp1_crit.dev_attr.attr,
	&sensor_dev_attr_temp8_crit.dev_attr.attr,

	&sensor_dev_attr_temp1_max.dev_attr.attr,
	&sensor_dev_attr_temp3_max.dev_attr.attr,
	&sensor_dev_attr_temp8_max.dev_attr.attr,

	&sensor_dev_attr_num_fan_trays.dev_attr.attr,
	&sensor_dev_attr_fans_per_tray.dev_attr.attr,
	&sensor_dev_attr_fan_max_speed.dev_attr.attr,

	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,

	&sensor_dev_attr_fan1_present.dev_attr.attr,
	&sensor_dev_attr_fan2_present.dev_attr.attr,
	&sensor_dev_attr_fan3_present.dev_attr.attr,
	&sensor_dev_attr_fan4_present.dev_attr.attr,
	&sensor_dev_attr_fan5_present.dev_attr.attr,
	&sensor_dev_attr_fan6_present.dev_attr.attr,
	&sensor_dev_attr_fan7_present.dev_attr.attr,
	&sensor_dev_attr_fan8_present.dev_attr.attr,

	&sensor_dev_attr_fan1_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan2_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan3_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan4_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan5_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan6_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan7_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan8_all_ok.dev_attr.attr,

	&sensor_dev_attr_fan_tray1_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan_tray2_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan_tray3_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan_tray4_all_ok.dev_attr.attr,

	&sensor_dev_attr_fan1_f2b.dev_attr.attr,
	&sensor_dev_attr_fan2_f2b.dev_attr.attr,
	&sensor_dev_attr_fan3_f2b.dev_attr.attr,
	&sensor_dev_attr_fan4_f2b.dev_attr.attr,
	&sensor_dev_attr_fan5_f2b.dev_attr.attr,
	&sensor_dev_attr_fan6_f2b.dev_attr.attr,
	&sensor_dev_attr_fan7_f2b.dev_attr.attr,
	&sensor_dev_attr_fan8_f2b.dev_attr.attr,

	&sensor_dev_attr_fan1_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan2_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan3_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan4_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan5_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan6_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan7_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan8_serial_num.dev_attr.attr,

	&sensor_dev_attr_fan1_part_num.dev_attr.attr,
	&sensor_dev_attr_fan2_part_num.dev_attr.attr,
	&sensor_dev_attr_fan3_part_num.dev_attr.attr,
	&sensor_dev_attr_fan4_part_num.dev_attr.attr,
	&sensor_dev_attr_fan5_part_num.dev_attr.attr,
	&sensor_dev_attr_fan6_part_num.dev_attr.attr,
	&sensor_dev_attr_fan7_part_num.dev_attr.attr,
	&sensor_dev_attr_fan8_part_num.dev_attr.attr,

	&sensor_dev_attr_fan1_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan2_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan3_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan4_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan5_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan6_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan7_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan8_label_rev.dev_attr.attr,

	&sensor_dev_attr_psu1_max.dev_attr.attr,
	&sensor_dev_attr_psu2_max.dev_attr.attr,
	&sensor_dev_attr_psu1_input.dev_attr.attr,
	&sensor_dev_attr_psu2_input.dev_attr.attr,
	&sensor_dev_attr_psu1_output.dev_attr.attr,
	&sensor_dev_attr_psu2_output.dev_attr.attr,

	&sensor_dev_attr_psu1_present.dev_attr.attr,
	&sensor_dev_attr_psu2_present.dev_attr.attr,
	&sensor_dev_attr_psu_pwr1_all_ok.dev_attr.attr,
	&sensor_dev_attr_psu_pwr2_all_ok.dev_attr.attr,

	&sensor_dev_attr_temp11_input.dev_attr.attr,
	&sensor_dev_attr_temp12_input.dev_attr.attr,
	&sensor_dev_attr_fan9_input.dev_attr.attr,
	&sensor_dev_attr_fan10_input.dev_attr.attr,
	&sensor_dev_attr_fan9_present.dev_attr.attr,
	&sensor_dev_attr_fan10_present.dev_attr.attr,
	&sensor_dev_attr_fan9_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan10_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan9_f2b.dev_attr.attr,
	&sensor_dev_attr_fan10_f2b.dev_attr.attr,

	&sensor_dev_attr_psu1_country_code.dev_attr.attr,
	&sensor_dev_attr_psu2_country_code.dev_attr.attr,
	&sensor_dev_attr_psu1_part_num.dev_attr.attr,
	&sensor_dev_attr_psu2_part_num.dev_attr.attr,
	&sensor_dev_attr_psu1_mfg_id.dev_attr.attr,
	&sensor_dev_attr_psu2_mfg_id.dev_attr.attr,
	&sensor_dev_attr_psu1_mfg_date.dev_attr.attr,
	&sensor_dev_attr_psu2_mfg_date.dev_attr.attr,
	&sensor_dev_attr_psu1_serial_num.dev_attr.attr,
	&sensor_dev_attr_psu2_serial_num.dev_attr.attr,
	&sensor_dev_attr_psu1_service_tag.dev_attr.attr,
	&sensor_dev_attr_psu2_service_tag.dev_attr.attr,
	&sensor_dev_attr_psu1_label_rev.dev_attr.attr,
	&sensor_dev_attr_psu2_label_rev.dev_attr.attr,
	NULL,
};

ATTRIBUTE_GROUPS(smf);

/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static int smf_probe(struct platform_device *dev)
{
	int ret = 0;
	struct device *hdev;

	smf_regs = ioport_map(SMF_IO_BASE, SMF_IO_SIZE);

	if (!smf_regs) {
		pr_err(DRIVER_NAME " unabled to map iomem\n");
		ret = -ENODEV;
		goto err_exit;
	}

	hdev = hwmon_device_register_with_groups(&dev->dev, DRIVER_NAME,
						 NULL,
						 smf_groups);
	if (IS_ERR(hdev)) {
		dev_err(&dev->dev, "hwmon registration failed");
		ret = PTR_ERR(hdev);
		goto err_unmap;
	}
	dev_info(&dev->dev, "version " DRIVER_VERSION " successfully loaded\n");
	return ret;

err_unmap:
	iounmap(smf_regs);
err_exit:
	return ret;
}

static int smf_remove(struct platform_device *ofdev)
{
	iounmap(smf_regs);
	platform_set_drvdata(ofdev, NULL);
	dev_info(&ofdev->dev, "removed\n");
	return 0;
}

static struct platform_driver smf_driver = {
	.probe = smf_probe,
	.remove = smf_remove,
	.driver = {
		.name  = DRIVER_NAME,
		.owner = THIS_MODULE,
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
static struct platform_device *smf_device;

static int __init smf_init(void)
{
	int rv;

	rv = platform_driver_register(&smf_driver);
	if (rv)
		goto err_exit;

	smf_device = platform_device_alloc(DRIVER_NAME, 0);
	if (!smf_device) {
		pr_err(DRIVER_NAME "platform_device_alloc failed for sm device\n");
		rv = -ENOMEM;
		goto err_unregister;
	}

	rv = platform_device_add(smf_device);
	if (rv) {
		pr_err(DRIVER_NAME "platform_device_add failed for sm device.\n");
		goto err_dealloc;
	}
	return 0;

err_dealloc:
	platform_device_unregister(smf_device);

err_unregister:
	platform_driver_unregister(&smf_driver);

err_exit:
	pr_err(DRIVER_NAME " platform_driver_register failed (%i)\n", rv);

	return rv;
}

static void __exit smf_exit(void)
{
	platform_device_unregister(smf_device);
	platform_driver_unregister(&smf_driver);
}

MODULE_AUTHOR("Manohar K C <manu@cumulusnetworks.com");
MODULE_DESCRIPTION("Smartfusion Driver for Dell EMC S4248FBL");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(smf_init);
module_exit(smf_exit);
