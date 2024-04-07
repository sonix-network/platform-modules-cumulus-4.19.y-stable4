/*
 * Smartfusion driver for dell_z9100.
 *
 * Copyright (C) 2015,2016,2019 Cumulus Networks, Inc.
 * Author: Puneet Shenoy <puneet@cumulusnetworks.com>
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
#include <linux/mutex.h>
#include <asm/io.h>

#include "platform-defs.h"
#include "dell-z9100-smf.h"

static const char driver_name[] = "dell_z9100_smf";
#define DRIVER_VERSION "1.0"

static uint8_t* z9100_smf_regs;
static DEFINE_MUTEX(z9100_smf_mbox_mutex);

/* SMF Access via LPC registers */
static inline uint8_t z9100_smf_rd(u32 reg)
{
	return ioread8(z9100_smf_regs + reg - Z9100_SMF_IO_BASE);
}

static inline void z9100_smf_wr(u32 reg, uint8_t data)
{
	iowrite8(data, z9100_smf_regs + reg - Z9100_SMF_IO_BASE);
}

/* SMF sysfs access */
static ssize_t smf_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = attr->index;

	return sprintf(buf, "0x%x\n", z9100_smf_rd(reg));
}

static SENSOR_DEVICE_ATTR(smf_ver,           S_IRUGO, smf_show, NULL, Z9100_SMF_VER);
static SENSOR_DEVICE_ATTR(smf_board_type,    S_IRUGO, smf_show, NULL, Z9100_SMF_BOARD_TYPE);
static SENSOR_DEVICE_ATTR(smf_scratch,       S_IRUGO, smf_show, NULL, Z9100_SMF_SW_SCRATCH);
static SENSOR_DEVICE_ATTR(smf_boot_ok,       S_IRUGO, smf_show, NULL, Z9100_SMF_BOOT_OK);
static SENSOR_DEVICE_ATTR(smf_uart_mux_ctrl, S_IRUGO, smf_show, NULL, Z9100_SMF_UART_MUX_CTRL);
static SENSOR_DEVICE_ATTR(smf_wd_timer,      S_IRUGO, smf_show, NULL, Z9100_SMF_WD_WID);
static SENSOR_DEVICE_ATTR(smf_wd_mask,       S_IRUGO, smf_show, NULL, Z9100_SMF_WD_MASK);
static SENSOR_DEVICE_ATTR(smf_rst_source,    S_IRUGO, smf_show, NULL, Z9100_SMF_RST_SOURCE);
static SENSOR_DEVICE_ATTR(smf_sep_rst,       S_IRUGO, smf_show, NULL, Z9100_SMF_SEP_RST);
static SENSOR_DEVICE_ATTR(smf_cpu_rst_ctrl,  S_IRUGO, smf_show, NULL, Z9100_SMF_CPU_RST_CTRL);
static SENSOR_DEVICE_ATTR(smf_ram_addr_h,    S_IRUGO, smf_show, NULL, Z9100_SMF_RAM_ADDR_H);
static SENSOR_DEVICE_ATTR(smf_ram_addr_l,    S_IRUGO, smf_show, NULL, Z9100_SMF_RAM_ADDR_L);
static SENSOR_DEVICE_ATTR(smf_ram_rd,        S_IRUGO, smf_show, NULL, Z9100_SMF_RAM_R_DATA);
static SENSOR_DEVICE_ATTR(smf_ram_wr,        S_IRUGO, smf_show, NULL, Z9100_SMF_RAM_W_DATA);
static SENSOR_DEVICE_ATTR(smf_tpm_status,    S_IRUGO, smf_show, NULL, Z9100_SMF_TPM_STA_ID);
static SENSOR_DEVICE_ATTR(smf_therm_pwr_ctrl,S_IRUGO, smf_show, NULL, Z9100_SMF_THERM_PWR_CTRL);
static SENSOR_DEVICE_ATTR(smf_therm_timesel, S_IRUGO, smf_show, NULL, Z9100_SMF_THERM_TIMESEL);
static SENSOR_DEVICE_ATTR(smf_therm_timeout, S_IRUGO, smf_show, NULL, Z9100_SMF_THERM_TIMEOUT);

/* SMF Mailbox Access */

enum mb_dir {
	MB_FWD,		/* pack bytes in forward order from hi end of array */
	MB_REV		/* pack bytes in reverse order from lo end of array */
};

static void z9100_smf_mb_rd_array(u32 reg, unsigned int n, enum mb_dir dir,
				  char *array)
{
	int i;
	int index;
	u8 hbyte;
	u8 lbyte;

	/*
	 * Read a series of `n' SMF mailbox bytes into `array' starting
	 * with register offset `reg'.  Different data types are read
	 * from the mailbox and then packed in different directions;
	 * 2-byte sensor values are generally packed right-to-left
	 * ("reverse") and strings are packed left-to-right ("forward").
	 *
	 * An SMF mailbox array protection mutex is held for the entire
	 * array read to prevent other threads from concurrently jumping in.
	 *
	 * Caveats:
	 * `array' must be able to hold at least `n' bytes.
	 * Note that `array' may be larger than `n', e.g. to accommodate
	 *   a terminating NULL for a string.
	 */
	hbyte = (reg >> 8) & 0xff;
	lbyte = reg & 0xff;

	mutex_lock(&z9100_smf_mbox_mutex);
	for (i = 0; i < n; i++) {
		if (dir == MB_FWD) {
			index = i;
		} else {
			index = (n - 1) - i;
		}
		z9100_smf_wr(Z9100_SMF_RAM_ADDR_H, hbyte);
		z9100_smf_wr(Z9100_SMF_RAM_ADDR_L, lbyte);

		*(array + index) = z9100_smf_rd(Z9100_SMF_RAM_R_DATA);

		if (lbyte == 0xff) {
			lbyte = 0;	/* address wraps around */
			hbyte++;
		} else
			lbyte++;
	}
	mutex_unlock(&z9100_smf_mbox_mutex);
}

static u8 z9100_smf_mb_rd(u32 reg)
{
	u8 rdata;

	z9100_smf_mb_rd_array(reg, 1, MB_FWD, &rdata);
	return rdata;
}

/* SMF Mailbox sysfs access */
static ssize_t z9100_smf_mb_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = attr->index;

	return sprintf(buf, "0x%x\n", z9100_smf_mb_rd(reg));
}

static SENSOR_DEVICE_ATTR(mb_proto_ver,    S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_PROTO_VER);
static SENSOR_DEVICE_ATTR(mb_firewall_ver, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_POWER_MAX);
static SENSOR_DEVICE_ATTR(mb_system_status, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_SYSTEM_STATUS);
static SENSOR_DEVICE_ATTR(mb_smf_flag, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_SMF_FLAG);
static SENSOR_DEVICE_ATTR(mb_cpu_flag, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_CPU_FLAG);
static SENSOR_DEVICE_ATTR(mb_dev_status, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_DEV_STATUS);
static SENSOR_DEVICE_ATTR(mb_fan_algo, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_FAN_ALGO);
static SENSOR_DEVICE_ATTR(mb_scan_lm75_1, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_SCAN_LM75_1);
static SENSOR_DEVICE_ATTR(mb_scan_lm75_2, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_SCAN_LM75_1+1);
static SENSOR_DEVICE_ATTR(mb_scan_lm75_3, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_SCAN_LM75_1+2);
static SENSOR_DEVICE_ATTR(mb_scan_lm75_4, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_SCAN_LM75_1+3);
static SENSOR_DEVICE_ATTR(mb_scan_lm75_5, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_SCAN_LM75_1+4);
static SENSOR_DEVICE_ATTR(mb_scan_lm75_6, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_SCAN_LM75_1+5);
static SENSOR_DEVICE_ATTR(mb_scan_lm75_7, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_SCAN_LM75_1+6);
static SENSOR_DEVICE_ATTR(mb_scan_lm75_8, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_SCAN_LM75_1+7);
static SENSOR_DEVICE_ATTR(mb_scan_lm75_9, S_IRUGO, z9100_smf_mb_show, NULL, Z9100_SMF_MB_SCAN_LM75_1+8);


static ssize_t mb_power_show(struct device * dev,
			    struct device_attribute * dattr,
			    char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = attr->index;
	s16 val;

	z9100_smf_mb_rd_array(reg, sizeof(val), MB_REV, (char *)&val);
	if (val == 0xffff)
		return -EIO;

	return sprintf(buf, "%d\n", val/10);
}

static SENSOR_DEVICE_ATTR(power_max,    S_IRUGO, mb_power_show, NULL, Z9100_SMF_MB_FIREWALL_VER);

/* Temp Sensors */
static ssize_t mb_temp_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_TEMP1_SENSOR + ((attr->index - 1) * 2);
	s16 val;

	z9100_smf_mb_rd_array(reg, sizeof(val), MB_REV, (char *)&val);
	if (val == 0xffff)
		return -EINVAL;

	return sprintf(buf, "%d\n", val*100);
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, mb_temp_show, NULL, 1);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO, mb_temp_show, NULL, 2);
static SENSOR_DEVICE_ATTR(temp3_input, S_IRUGO, mb_temp_show, NULL, 3);
static SENSOR_DEVICE_ATTR(temp4_input, S_IRUGO, mb_temp_show, NULL, 4);
static SENSOR_DEVICE_ATTR(temp5_input, S_IRUGO, mb_temp_show, NULL, 5);
static SENSOR_DEVICE_ATTR(temp6_input, S_IRUGO, mb_temp_show, NULL, 6);
static SENSOR_DEVICE_ATTR(temp7_input, S_IRUGO, mb_temp_show, NULL, 7);
static SENSOR_DEVICE_ATTR(temp8_input, S_IRUGO, mb_temp_show, NULL, 8);
static SENSOR_DEVICE_ATTR(temp9_input, S_IRUGO, mb_temp_show, NULL, 9);

static ssize_t mb_temp_limit_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	u32 reg = attr->nr + ((attr->index - 1) * 8);
	s16 val;

	z9100_smf_mb_rd_array(reg, sizeof(val), MB_REV, (char *)&val);
	if (val == 0xffff)
		return -EINVAL;

	return sprintf(buf, "%d\n", val*100);
}

static SENSOR_DEVICE_ATTR_2(temp1_crit,   S_IRUGO, mb_temp_limit_show, NULL, Z9100_SMF_MB_TEMP1_HW_SHUT_LIMIT, 1);
static SENSOR_DEVICE_ATTR_2(temp1_max,       S_IRUGO, mb_temp_limit_show, NULL, Z9100_SMF_MB_TEMP1_MJR_ALARM_LIMIT, 1);

static SENSOR_DEVICE_ATTR_2(temp2_crit,      S_IRUGO, mb_temp_limit_show, NULL, Z9100_SMF_MB_TEMP1_SW_SHUT_LIMIT, 2);
static SENSOR_DEVICE_ATTR_2(temp2_max,       S_IRUGO, mb_temp_limit_show, NULL, Z9100_SMF_MB_TEMP1_MJR_ALARM_LIMIT, 2);

static SENSOR_DEVICE_ATTR_2(temp3_max,       S_IRUGO, mb_temp_limit_show, NULL, Z9100_SMF_MB_TEMP1_MJR_ALARM_LIMIT, 3);

static SENSOR_DEVICE_ATTR_2(temp4_max,       S_IRUGO, mb_temp_limit_show, NULL, Z9100_SMF_MB_TEMP1_MJR_ALARM_LIMIT, 4);

static SENSOR_DEVICE_ATTR_2(temp6_crit,      S_IRUGO, mb_temp_limit_show, NULL, Z9100_SMF_MB_TEMP1_SW_SHUT_LIMIT, 6);
static SENSOR_DEVICE_ATTR_2(temp6_max,       S_IRUGO, mb_temp_limit_show, NULL, Z9100_SMF_MB_TEMP1_MJR_ALARM_LIMIT, 6);

static SENSOR_DEVICE_ATTR_2(temp9_max,       S_IRUGO, mb_temp_limit_show, NULL, Z9100_SMF_MB_TEMP1_MJR_ALARM_LIMIT, 9);

static ssize_t mb_temp_fault_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_TEMP1_FAULT + (attr->index - 1);

	return sprintf(buf, "%d\n", z9100_smf_mb_rd(reg));
}

static SENSOR_DEVICE_ATTR(temp1_fault, S_IRUGO, mb_temp_fault_show, NULL, 1);
static SENSOR_DEVICE_ATTR(temp2_fault, S_IRUGO, mb_temp_fault_show, NULL, 2);
static SENSOR_DEVICE_ATTR(temp3_fault, S_IRUGO, mb_temp_fault_show, NULL, 3);
static SENSOR_DEVICE_ATTR(temp4_fault, S_IRUGO, mb_temp_fault_show, NULL, 4);
static SENSOR_DEVICE_ATTR(temp5_fault, S_IRUGO, mb_temp_fault_show, NULL, 5);
static SENSOR_DEVICE_ATTR(temp6_fault, S_IRUGO, mb_temp_fault_show, NULL, 6);
static SENSOR_DEVICE_ATTR(temp7_fault, S_IRUGO, mb_temp_fault_show, NULL, 7);
static SENSOR_DEVICE_ATTR(temp8_fault, S_IRUGO, mb_temp_fault_show, NULL, 8);
static SENSOR_DEVICE_ATTR(temp9_fault, S_IRUGO, mb_temp_fault_show, NULL, 9);

/* Fan Sensors */
static ssize_t mb_fan_show(struct device * dev,
			   struct device_attribute * dattr,
			   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = attr->index;

	return sprintf(buf, "%d\n", z9100_smf_mb_rd(reg));
}

static SENSOR_DEVICE_ATTR(fan_tray_cnt, S_IRUGO, mb_fan_show, NULL, Z9100_SMF_MB_FANS_TRAYS_CNT);
static SENSOR_DEVICE_ATTR(fans_per_tray, S_IRUGO, mb_fan_show, NULL, Z9100_SMF_MB_FANS_PER_TRAY);
static SENSOR_DEVICE_ATTR(fan_max_speed, S_IRUGO, mb_fan_show, NULL, Z9100_SMF_MB_FANS_MAX_SPEED);

static ssize_t mb_fan_speed_show(struct device * dev,
				 struct device_attribute * dattr,
				 char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_FAN_FAN1_SPEED + ((attr->index - 1) * 2);
	u16 val;

	z9100_smf_mb_rd_array(reg, sizeof(val), MB_REV, (char *)&val);
	if (val == 0xffff)
		return -EINVAL;

	return sprintf(buf, "%d\n", val);
}

static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, mb_fan_speed_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO, mb_fan_speed_show, NULL, 2);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO, mb_fan_speed_show, NULL, 3);
static SENSOR_DEVICE_ATTR(fan4_input, S_IRUGO, mb_fan_speed_show, NULL, 4);
static SENSOR_DEVICE_ATTR(fan5_input, S_IRUGO, mb_fan_speed_show, NULL, 5);
static SENSOR_DEVICE_ATTR(fan6_input, S_IRUGO, mb_fan_speed_show, NULL, 6);
static SENSOR_DEVICE_ATTR(fan7_input, S_IRUGO, mb_fan_speed_show, NULL, 7);
static SENSOR_DEVICE_ATTR(fan8_input, S_IRUGO, mb_fan_speed_show, NULL, 8);
static SENSOR_DEVICE_ATTR(fan9_input, S_IRUGO, mb_fan_speed_show, NULL, 9);
static SENSOR_DEVICE_ATTR(fan10_input, S_IRUGO, mb_fan_speed_show, NULL, 10);

static ssize_t mb_fan_present_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_FAN_TRAYS_PRESENT;
	u8 idx = (attr->index - 1);
	u8 val;

	val = z9100_smf_mb_rd(reg);
	val &= (1 << (idx/2));
	if (val == 0)
		return sprintf(buf, "1\n");

	return sprintf(buf, "0\n");
}

static SENSOR_DEVICE_ATTR(fan1_present, S_IRUGO, mb_fan_present_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan2_present, S_IRUGO, mb_fan_present_show, NULL, 2);
static SENSOR_DEVICE_ATTR(fan3_present, S_IRUGO, mb_fan_present_show, NULL, 3);
static SENSOR_DEVICE_ATTR(fan4_present, S_IRUGO, mb_fan_present_show, NULL, 4);
static SENSOR_DEVICE_ATTR(fan5_present, S_IRUGO, mb_fan_present_show, NULL, 5);
static SENSOR_DEVICE_ATTR(fan6_present, S_IRUGO, mb_fan_present_show, NULL, 6);
static SENSOR_DEVICE_ATTR(fan7_present, S_IRUGO, mb_fan_present_show, NULL, 7);
static SENSOR_DEVICE_ATTR(fan8_present, S_IRUGO, mb_fan_present_show, NULL, 8);
static SENSOR_DEVICE_ATTR(fan9_present, S_IRUGO, mb_fan_present_show, NULL, 9);
static SENSOR_DEVICE_ATTR(fan10_present, S_IRUGO, mb_fan_present_show, NULL, 10);

static ssize_t mb_fan_ok_show(struct device * dev,
				 struct device_attribute * dattr,
				 char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_FAN_STATUS;
	u16 val;
	int idx = attr->index - 1;

	z9100_smf_mb_rd_array(reg, sizeof(val), MB_REV, (char *)&val);
	val &= (1 << idx);
	if (val == 0)
		return sprintf(buf, "1\n");

	return sprintf(buf, "0\n");
}

static SENSOR_DEVICE_ATTR(fan1_all_ok, S_IRUGO, mb_fan_ok_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan2_all_ok, S_IRUGO, mb_fan_ok_show, NULL, 2);
static SENSOR_DEVICE_ATTR(fan3_all_ok, S_IRUGO, mb_fan_ok_show, NULL, 3);
static SENSOR_DEVICE_ATTR(fan4_all_ok, S_IRUGO, mb_fan_ok_show, NULL, 4);
static SENSOR_DEVICE_ATTR(fan5_all_ok, S_IRUGO, mb_fan_ok_show, NULL, 5);
static SENSOR_DEVICE_ATTR(fan6_all_ok, S_IRUGO, mb_fan_ok_show, NULL, 6);
static SENSOR_DEVICE_ATTR(fan7_all_ok, S_IRUGO, mb_fan_ok_show, NULL, 7);
static SENSOR_DEVICE_ATTR(fan8_all_ok, S_IRUGO, mb_fan_ok_show, NULL, 8);
static SENSOR_DEVICE_ATTR(fan9_all_ok, S_IRUGO, mb_fan_ok_show, NULL, 9);
static SENSOR_DEVICE_ATTR(fan10_all_ok, S_IRUGO, mb_fan_ok_show, NULL, 10);

static ssize_t mb_fan_f2b_show(struct device * dev,
			       struct device_attribute * dattr,
			       char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_FAN_TRAYS_F2B;
	u8 idx = (attr->index - 1);
	u8 val;

	val = z9100_smf_mb_rd(reg);
	val &= (1 << (idx/2));
	if (val == 0)
		return sprintf(buf, "1\n");

	return sprintf(buf, "0\n");
}

static SENSOR_DEVICE_ATTR(fan1_f2b, S_IRUGO, mb_fan_f2b_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan2_f2b, S_IRUGO, mb_fan_f2b_show, NULL, 2);
static SENSOR_DEVICE_ATTR(fan3_f2b, S_IRUGO, mb_fan_f2b_show, NULL, 3);
static SENSOR_DEVICE_ATTR(fan4_f2b, S_IRUGO, mb_fan_f2b_show, NULL, 4);
static SENSOR_DEVICE_ATTR(fan5_f2b, S_IRUGO, mb_fan_f2b_show, NULL, 5);
static SENSOR_DEVICE_ATTR(fan6_f2b, S_IRUGO, mb_fan_f2b_show, NULL, 6);
static SENSOR_DEVICE_ATTR(fan7_f2b, S_IRUGO, mb_fan_f2b_show, NULL, 7);
static SENSOR_DEVICE_ATTR(fan8_f2b, S_IRUGO, mb_fan_f2b_show, NULL, 8);
static SENSOR_DEVICE_ATTR(fan9_f2b, S_IRUGO, mb_fan_f2b_show, NULL, 9);
static SENSOR_DEVICE_ATTR(fan10_f2b, S_IRUGO, mb_fan_f2b_show, NULL, 10);

static ssize_t mb_fan_serial_num_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int idx = (attr->index - 1)/2;
	u32 reg = Z9100_SMF_MB_FAN1_SERIAL_NUM + (35 * idx);
	char val[Z9100_SMF_MB_FAN1_SERIAL_NUM_SIZE + 1];

	z9100_smf_mb_rd_array(reg, Z9100_SMF_MB_FAN1_SERIAL_NUM_SIZE, MB_FWD,
			      val);
	val[Z9100_SMF_MB_FAN1_SERIAL_NUM_SIZE] = '\0';

	return sprintf(buf, "%s\n", val);
}

static SENSOR_DEVICE_ATTR(fan1_serial_num, S_IRUGO, mb_fan_serial_num_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan2_serial_num, S_IRUGO, mb_fan_serial_num_show, NULL, 2);
static SENSOR_DEVICE_ATTR(fan3_serial_num, S_IRUGO, mb_fan_serial_num_show, NULL, 3);
static SENSOR_DEVICE_ATTR(fan4_serial_num, S_IRUGO, mb_fan_serial_num_show, NULL, 4);
static SENSOR_DEVICE_ATTR(fan5_serial_num, S_IRUGO, mb_fan_serial_num_show, NULL, 5);
static SENSOR_DEVICE_ATTR(fan6_serial_num, S_IRUGO, mb_fan_serial_num_show, NULL, 6);
static SENSOR_DEVICE_ATTR(fan7_serial_num, S_IRUGO, mb_fan_serial_num_show, NULL, 7);
static SENSOR_DEVICE_ATTR(fan8_serial_num, S_IRUGO, mb_fan_serial_num_show, NULL, 8);
static SENSOR_DEVICE_ATTR(fan9_serial_num, S_IRUGO, mb_fan_serial_num_show, NULL, 9);
static SENSOR_DEVICE_ATTR(fan10_serial_num, S_IRUGO, mb_fan_serial_num_show, NULL, 10);

static ssize_t mb_fan_part_num_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int idx = (attr->index - 1)/2;
	u32 reg = Z9100_SMF_MB_FAN1_PART_NUM + (35 * idx);
	char val[Z9100_SMF_MB_FAN1_PART_NUM_SIZE + 1];

	z9100_smf_mb_rd_array(reg, Z9100_SMF_MB_FAN1_PART_NUM_SIZE, MB_FWD,
			      val);
	val[Z9100_SMF_MB_FAN1_PART_NUM_SIZE] = '\0';

	return sprintf(buf, "%s\n", val);
}
static SENSOR_DEVICE_ATTR(fan1_part_num, S_IRUGO, mb_fan_part_num_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan2_part_num, S_IRUGO, mb_fan_part_num_show, NULL, 2);
static SENSOR_DEVICE_ATTR(fan3_part_num, S_IRUGO, mb_fan_part_num_show, NULL, 3);
static SENSOR_DEVICE_ATTR(fan4_part_num, S_IRUGO, mb_fan_part_num_show, NULL, 4);
static SENSOR_DEVICE_ATTR(fan5_part_num, S_IRUGO, mb_fan_part_num_show, NULL, 5);
static SENSOR_DEVICE_ATTR(fan6_part_num, S_IRUGO, mb_fan_part_num_show, NULL, 6);
static SENSOR_DEVICE_ATTR(fan7_part_num, S_IRUGO, mb_fan_part_num_show, NULL, 7);
static SENSOR_DEVICE_ATTR(fan8_part_num, S_IRUGO, mb_fan_part_num_show, NULL, 8);
static SENSOR_DEVICE_ATTR(fan9_part_num, S_IRUGO, mb_fan_part_num_show, NULL, 9);
static SENSOR_DEVICE_ATTR(fan10_part_num, S_IRUGO, mb_fan_part_num_show, NULL, 10);

static ssize_t mb_fan_label_rev_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	int idx = (attr->index - 1)/2;
	u32 reg = Z9100_SMF_MB_FAN1_LABEL_REV + (35 * idx);
	char val[Z9100_SMF_MB_FAN1_LABEL_REV_SIZE + 1];

	z9100_smf_mb_rd_array(reg, Z9100_SMF_MB_FAN1_LABEL_REV_SIZE, MB_FWD,
			      val);
	val[Z9100_SMF_MB_FAN1_LABEL_REV_SIZE] = '\0';

	return sprintf(buf, "%s\n", val);
}

static SENSOR_DEVICE_ATTR(fan1_label_rev, S_IRUGO, mb_fan_label_rev_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan2_label_rev, S_IRUGO, mb_fan_label_rev_show, NULL, 2);
static SENSOR_DEVICE_ATTR(fan3_label_rev, S_IRUGO, mb_fan_label_rev_show, NULL, 3);
static SENSOR_DEVICE_ATTR(fan4_label_rev, S_IRUGO, mb_fan_label_rev_show, NULL, 4);
static SENSOR_DEVICE_ATTR(fan5_label_rev, S_IRUGO, mb_fan_label_rev_show, NULL, 5);
static SENSOR_DEVICE_ATTR(fan6_label_rev, S_IRUGO, mb_fan_label_rev_show, NULL, 6);
static SENSOR_DEVICE_ATTR(fan7_label_rev, S_IRUGO, mb_fan_label_rev_show, NULL, 7);
static SENSOR_DEVICE_ATTR(fan8_label_rev, S_IRUGO, mb_fan_label_rev_show, NULL, 8);
static SENSOR_DEVICE_ATTR(fan9_label_rev, S_IRUGO, mb_fan_label_rev_show, NULL, 9);
static SENSOR_DEVICE_ATTR(fan10_label_rev, S_IRUGO, mb_fan_label_rev_show, NULL, 10);

/* PSU */
static ssize_t mb_psu_power_show(struct device * dev,
			   struct device_attribute * dattr,
			   char * buf)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_BASE + attr->nr + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	s16 val;

	z9100_smf_mb_rd_array(reg, sizeof(val), MB_REV, (char *)&val);
	if (val == 0xffff)
		return -EINVAL;

	return sprintf(buf, "%d\n", val*100*1000);
}

static SENSOR_DEVICE_ATTR_2(power1_max, S_IRUGO, mb_psu_power_show, NULL, \
			    Z9100_SMF_MB_PSU1_MAX - Z9100_SMF_MB_PSU1_BASE, 1);
static SENSOR_DEVICE_ATTR_2(power2_max, S_IRUGO, mb_psu_power_show, NULL, \
			    Z9100_SMF_MB_PSU1_MAX - Z9100_SMF_MB_PSU1_BASE, 2);
static SENSOR_DEVICE_ATTR_2(power1_input, S_IRUGO, mb_psu_power_show, NULL, \
			    Z9100_SMF_MB_PSU1_INP_POWER - Z9100_SMF_MB_PSU1_BASE, 1);
static SENSOR_DEVICE_ATTR_2(power2_input, S_IRUGO, mb_psu_power_show, NULL, \
			    Z9100_SMF_MB_PSU1_INP_POWER - Z9100_SMF_MB_PSU1_BASE, 2);
static SENSOR_DEVICE_ATTR_2(power1_output, S_IRUGO, mb_psu_power_show, NULL, \
			    Z9100_SMF_MB_PSU1_OUT_POWER - Z9100_SMF_MB_PSU1_BASE, 1);
static SENSOR_DEVICE_ATTR_2(power2_output, S_IRUGO, mb_psu_power_show, NULL, \
			    Z9100_SMF_MB_PSU1_OUT_POWER - Z9100_SMF_MB_PSU1_BASE, 2);

static ssize_t mb_psu_present_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_STATUS + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	u8 val;

	val = z9100_smf_mb_rd(reg);
	val &= Z9100_SMF_MB_PSU_PRESENT_FLAG;
	if (val == 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}

static SENSOR_DEVICE_ATTR(psu_pwr1_present, S_IRUGO, mb_psu_present_show, NULL, 1);
static SENSOR_DEVICE_ATTR(psu_pwr2_present, S_IRUGO, mb_psu_present_show, NULL, 2);

static ssize_t mb_psu_ok_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_STATUS + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	u8 val;

	val = z9100_smf_mb_rd(reg);
	val &= Z9100_SMF_MB_PSU_OK_FLAG;
	if (val == 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}

static SENSOR_DEVICE_ATTR(psu_pwr1_all_ok, S_IRUGO, mb_psu_ok_show, NULL, 1);
static SENSOR_DEVICE_ATTR(psu_pwr2_all_ok, S_IRUGO, mb_psu_ok_show, NULL, 2);

static ssize_t mb_psu_status_show(struct device * dev,
				  struct device_attribute * dattr,
				  char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_STATUS + \
		  ((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	u8 val;

	val = z9100_smf_mb_rd(reg);
	return sprintf(buf, "0x%x\n", val);
}

static SENSOR_DEVICE_ATTR(psu_pwr1_status, S_IRUGO, mb_psu_status_show, NULL, 1);
static SENSOR_DEVICE_ATTR(psu_pwr2_status, S_IRUGO, mb_psu_status_show, NULL, 2);

static ssize_t mb_psu_temp_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_TEMP + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	s16 val;

	z9100_smf_mb_rd_array(reg, sizeof(val), MB_REV, (char *)&val);
	if (val == 0xffff)
		return -EINVAL;

	return sprintf(buf, "%d\n", val*100);
}

static SENSOR_DEVICE_ATTR(temp11_input, S_IRUGO, mb_psu_temp_show, NULL, 1);
static SENSOR_DEVICE_ATTR(temp12_input, S_IRUGO, mb_psu_temp_show, NULL, 2);

static ssize_t mb_psu_fan_show(struct device * dev,
                        struct device_attribute * dattr,
                        char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_FAN_SPEED + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	s16 val;

	z9100_smf_mb_rd_array(reg, sizeof(val), MB_REV, (char *)&val);
	if (val == 0xffff)
		return -EINVAL;

	return sprintf(buf, "%d\n", val);
}

static SENSOR_DEVICE_ATTR(fan11_input, S_IRUGO, mb_psu_fan_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan12_input, S_IRUGO, mb_psu_fan_show, NULL, 2);

static ssize_t mb_psu_fan_present_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_FAN_STATUS + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	u8 val;

	val = z9100_smf_mb_rd(reg);
	val &= Z9100_SMF_MB_PSU_FAN_PRESENT_FLAG;
	if (val == 0)
		return sprintf(buf, "1\n");
	return sprintf(buf, "0\n");
}

static SENSOR_DEVICE_ATTR(fan11_present, S_IRUGO, mb_psu_fan_present_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan12_present, S_IRUGO, mb_psu_fan_present_show, NULL, 2);

static ssize_t mb_psu_fan_ok_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_FAN_STATUS + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	u8 val;

	val = z9100_smf_mb_rd(reg);
	val &= Z9100_SMF_MB_PSU_FAN_OK_FLAG;
	if (val == 0)
		return sprintf(buf, "1\n");

	return sprintf(buf, "0\n");
}

static SENSOR_DEVICE_ATTR(fan11_all_ok, S_IRUGO, mb_psu_fan_ok_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan12_all_ok, S_IRUGO, mb_psu_fan_ok_show, NULL, 2);

static ssize_t mb_psu_fan_f2b_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_FAN_STATUS + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	u8 val;

	val = z9100_smf_mb_rd(reg);
	val &= Z9100_SMF_MB_PSU_FAN_F2B_FLAG;

	return sprintf(buf, "%d\n", val);
}

static SENSOR_DEVICE_ATTR(fan11_f2b, S_IRUGO, mb_psu_fan_f2b_show, NULL, 1);
static SENSOR_DEVICE_ATTR(fan12_f2b, S_IRUGO, mb_psu_fan_f2b_show, NULL, 2);

static ssize_t mb_psu_county_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_COUNTY + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	char val[Z9100_SMF_MB_PSU1_COUNTY_SIZE + 1];

	z9100_smf_mb_rd_array(reg, Z9100_SMF_MB_PSU1_COUNTY_SIZE, MB_FWD, val);
	val[Z9100_SMF_MB_PSU1_COUNTY_SIZE] = '\0';

	return sprintf(buf, "%s\n", val);
}

static SENSOR_DEVICE_ATTR(psu1_county, S_IRUGO, mb_psu_county_show, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_county, S_IRUGO, mb_psu_county_show, NULL, 2);

static ssize_t mb_psu_part_num_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_PART_NUM + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	char val[Z9100_SMF_MB_PSU1_PART_NUM_SIZE + 1];

	z9100_smf_mb_rd_array(reg, Z9100_SMF_MB_PSU1_PART_NUM_SIZE, MB_FWD,
			      val);
	val[Z9100_SMF_MB_PSU1_PART_NUM_SIZE] = '\0';

	return sprintf(buf, "%s\n", val);
}

static SENSOR_DEVICE_ATTR(psu1_part_num, S_IRUGO, mb_psu_part_num_show, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_part_num, S_IRUGO, mb_psu_part_num_show, NULL, 2);

static ssize_t mb_psu_mfg_id_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_MFG_ID + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	char val[Z9100_SMF_MB_PSU1_MFG_ID_SIZE + 1];

	z9100_smf_mb_rd_array(reg, Z9100_SMF_MB_PSU1_MFG_ID_SIZE, MB_FWD, val);
	val[Z9100_SMF_MB_PSU1_MFG_ID_SIZE] = '\0';

	return sprintf(buf, "%s\n", val);
}

static SENSOR_DEVICE_ATTR(psu1_mfg_id, S_IRUGO, mb_psu_mfg_id_show, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_mfg_id, S_IRUGO, mb_psu_mfg_id_show, NULL, 2);

static ssize_t mb_psu_mfg_date_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_MFG_DATE + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	char val[Z9100_SMF_MB_PSU1_MFG_DATE_SIZE + 1];

	z9100_smf_mb_rd_array(reg, Z9100_SMF_MB_PSU1_MFG_DATE_SIZE, MB_FWD,
			      val);
	val[Z9100_SMF_MB_PSU1_MFG_DATE_SIZE] = '\0';

	return sprintf(buf, "%s\n", val);
}

static SENSOR_DEVICE_ATTR(psu1_mfg_date, S_IRUGO, mb_psu_mfg_date_show, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_mfg_date, S_IRUGO, mb_psu_mfg_date_show, NULL, 2);

static ssize_t mb_psu_serial_num_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_SERIAL_NUM + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	char val[Z9100_SMF_MB_PSU1_SERIAL_NUM_SIZE + 1];

	z9100_smf_mb_rd_array(reg, Z9100_SMF_MB_PSU1_SERIAL_NUM_SIZE, MB_FWD,
			      val);
	val[Z9100_SMF_MB_PSU1_SERIAL_NUM_SIZE] = '\0';

	return sprintf(buf, "%s\n", val);
}

static SENSOR_DEVICE_ATTR(psu1_serial_num, S_IRUGO, mb_psu_serial_num_show, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_serial_num, S_IRUGO, mb_psu_serial_num_show, NULL, 2);

static ssize_t mb_psu_service_tag_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_SERVICE_TAG + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	char val[Z9100_SMF_MB_PSU1_SERVICE_TAG_SIZE + 1];

	z9100_smf_mb_rd_array(reg, Z9100_SMF_MB_PSU1_SERVICE_TAG_SIZE, MB_FWD,
			      val);
	val[Z9100_SMF_MB_PSU1_SERVICE_TAG_SIZE] = '\0';

	return sprintf(buf, "%s\n", val);
}

static SENSOR_DEVICE_ATTR(psu1_service_tag, S_IRUGO, mb_psu_service_tag_show, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_service_tag, S_IRUGO, mb_psu_service_tag_show, NULL, 2);

static ssize_t mb_psu_label_rev_show(struct device * dev,
				   struct device_attribute * dattr,
				   char * buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(dattr);
	u32 reg = Z9100_SMF_MB_PSU1_LABEL_REV + \
		((attr->index - 1) * Z9100_SMF_MB_PSU_OFFSET);
	char val[Z9100_SMF_MB_PSU1_LABEL_REV_SIZE + 1];

	z9100_smf_mb_rd_array(reg, Z9100_SMF_MB_PSU1_LABEL_REV_SIZE, MB_FWD,
			      val);
	val[Z9100_SMF_MB_PSU1_LABEL_REV_SIZE] = '\0';

	return sprintf(buf, "%s\n", val);
}

static SENSOR_DEVICE_ATTR(psu1_label_rev, S_IRUGO, mb_psu_label_rev_show, NULL, 1);
static SENSOR_DEVICE_ATTR(psu2_label_rev, S_IRUGO, mb_psu_label_rev_show, NULL, 2);

static struct attribute *dell_z9100_smf_attrs[] = {
       /* &sensor_dev_attr_name.dev_attr.attr, */
	&sensor_dev_attr_smf_ver.dev_attr.attr,
	&sensor_dev_attr_smf_board_type.dev_attr.attr,
	&sensor_dev_attr_smf_scratch.dev_attr.attr,
	&sensor_dev_attr_smf_boot_ok.dev_attr.attr,
	&sensor_dev_attr_smf_uart_mux_ctrl.dev_attr.attr,
	&sensor_dev_attr_smf_wd_timer.dev_attr.attr,
	&sensor_dev_attr_smf_wd_mask.dev_attr.attr,
	&sensor_dev_attr_smf_rst_source.dev_attr.attr,
	&sensor_dev_attr_smf_sep_rst.dev_attr.attr,
	&sensor_dev_attr_smf_cpu_rst_ctrl.dev_attr.attr,
	&sensor_dev_attr_smf_ram_addr_l.dev_attr.attr,
	&sensor_dev_attr_smf_ram_addr_h.dev_attr.attr,
	&sensor_dev_attr_smf_ram_rd.dev_attr.attr,
	&sensor_dev_attr_smf_ram_wr.dev_attr.attr,
	&sensor_dev_attr_smf_tpm_status.dev_attr.attr,
	&sensor_dev_attr_smf_therm_pwr_ctrl.dev_attr.attr,
	&sensor_dev_attr_smf_therm_timesel.dev_attr.attr,
	&sensor_dev_attr_smf_therm_timeout.dev_attr.attr,

	&sensor_dev_attr_mb_proto_ver.dev_attr.attr,
	&sensor_dev_attr_mb_firewall_ver.dev_attr.attr,
	&sensor_dev_attr_mb_system_status.dev_attr.attr,
	&sensor_dev_attr_mb_smf_flag.dev_attr.attr,
	&sensor_dev_attr_mb_cpu_flag.dev_attr.attr,
	&sensor_dev_attr_mb_dev_status.dev_attr.attr,
	&sensor_dev_attr_mb_fan_algo.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_1.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_2.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_3.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_4.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_5.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_6.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_7.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_8.dev_attr.attr,
	&sensor_dev_attr_mb_scan_lm75_9.dev_attr.attr,

	&sensor_dev_attr_power_max.dev_attr.attr,

	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp4_input.dev_attr.attr,
	&sensor_dev_attr_temp5_input.dev_attr.attr,
	&sensor_dev_attr_temp6_input.dev_attr.attr,
	&sensor_dev_attr_temp7_input.dev_attr.attr,
	&sensor_dev_attr_temp8_input.dev_attr.attr,
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
	&sensor_dev_attr_temp1_max.dev_attr.attr,

	&sensor_dev_attr_temp2_crit.dev_attr.attr,
	&sensor_dev_attr_temp2_max.dev_attr.attr,

	&sensor_dev_attr_temp3_max.dev_attr.attr,

	&sensor_dev_attr_temp4_max.dev_attr.attr,

	&sensor_dev_attr_temp6_crit.dev_attr.attr,
	&sensor_dev_attr_temp6_max.dev_attr.attr,

	&sensor_dev_attr_temp9_max.dev_attr.attr,

	&sensor_dev_attr_fan_tray_cnt.dev_attr.attr,
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
	&sensor_dev_attr_fan9_input.dev_attr.attr,
	&sensor_dev_attr_fan10_input.dev_attr.attr,

	&sensor_dev_attr_fan1_present.dev_attr.attr,
	&sensor_dev_attr_fan2_present.dev_attr.attr,
	&sensor_dev_attr_fan3_present.dev_attr.attr,
	&sensor_dev_attr_fan4_present.dev_attr.attr,
	&sensor_dev_attr_fan5_present.dev_attr.attr,
	&sensor_dev_attr_fan6_present.dev_attr.attr,
	&sensor_dev_attr_fan7_present.dev_attr.attr,
	&sensor_dev_attr_fan8_present.dev_attr.attr,
	&sensor_dev_attr_fan9_present.dev_attr.attr,
	&sensor_dev_attr_fan10_present.dev_attr.attr,

	&sensor_dev_attr_fan1_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan2_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan3_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan4_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan5_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan6_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan7_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan8_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan9_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan10_all_ok.dev_attr.attr,

	&sensor_dev_attr_fan1_f2b.dev_attr.attr,
	&sensor_dev_attr_fan2_f2b.dev_attr.attr,
	&sensor_dev_attr_fan3_f2b.dev_attr.attr,
	&sensor_dev_attr_fan4_f2b.dev_attr.attr,
	&sensor_dev_attr_fan5_f2b.dev_attr.attr,
	&sensor_dev_attr_fan6_f2b.dev_attr.attr,
	&sensor_dev_attr_fan7_f2b.dev_attr.attr,
	&sensor_dev_attr_fan8_f2b.dev_attr.attr,
	&sensor_dev_attr_fan9_f2b.dev_attr.attr,
	&sensor_dev_attr_fan10_f2b.dev_attr.attr,

	&sensor_dev_attr_fan1_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan2_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan3_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan4_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan5_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan6_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan7_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan8_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan9_serial_num.dev_attr.attr,
	&sensor_dev_attr_fan10_serial_num.dev_attr.attr,

	&sensor_dev_attr_fan1_part_num.dev_attr.attr,
	&sensor_dev_attr_fan2_part_num.dev_attr.attr,
	&sensor_dev_attr_fan3_part_num.dev_attr.attr,
	&sensor_dev_attr_fan4_part_num.dev_attr.attr,
	&sensor_dev_attr_fan5_part_num.dev_attr.attr,
	&sensor_dev_attr_fan6_part_num.dev_attr.attr,
	&sensor_dev_attr_fan7_part_num.dev_attr.attr,
	&sensor_dev_attr_fan8_part_num.dev_attr.attr,
	&sensor_dev_attr_fan9_part_num.dev_attr.attr,
	&sensor_dev_attr_fan10_part_num.dev_attr.attr,


	&sensor_dev_attr_fan1_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan2_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan3_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan4_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan5_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan6_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan7_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan8_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan9_label_rev.dev_attr.attr,
	&sensor_dev_attr_fan10_label_rev.dev_attr.attr,

	&sensor_dev_attr_power1_max.dev_attr.attr,
	&sensor_dev_attr_power2_max.dev_attr.attr,
	&sensor_dev_attr_power1_input.dev_attr.attr,
	&sensor_dev_attr_power2_input.dev_attr.attr,
	&sensor_dev_attr_power1_output.dev_attr.attr,
	&sensor_dev_attr_power2_output.dev_attr.attr,

	&sensor_dev_attr_psu_pwr1_present.dev_attr.attr,
	&sensor_dev_attr_psu_pwr2_present.dev_attr.attr,
	&sensor_dev_attr_psu_pwr1_all_ok.dev_attr.attr,
	&sensor_dev_attr_psu_pwr2_all_ok.dev_attr.attr,
	&sensor_dev_attr_psu_pwr1_status.dev_attr.attr,
	&sensor_dev_attr_psu_pwr2_status.dev_attr.attr,

	&sensor_dev_attr_temp11_input.dev_attr.attr,
	&sensor_dev_attr_temp12_input.dev_attr.attr,
	&sensor_dev_attr_fan11_input.dev_attr.attr,
	&sensor_dev_attr_fan12_input.dev_attr.attr,
	&sensor_dev_attr_fan11_present.dev_attr.attr,
	&sensor_dev_attr_fan12_present.dev_attr.attr,
	&sensor_dev_attr_fan11_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan12_all_ok.dev_attr.attr,
	&sensor_dev_attr_fan11_f2b.dev_attr.attr,
	&sensor_dev_attr_fan12_f2b.dev_attr.attr,

	&sensor_dev_attr_psu1_county.dev_attr.attr,
	&sensor_dev_attr_psu2_county.dev_attr.attr,
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

ATTRIBUTE_GROUPS(dell_z9100_smf);

/*------------------------------------------------------------------------------
 *
 * driver interface
 *
 */

static int dell_z9100_smf_probe(struct platform_device * dev)
{
	int ret = 0;
	struct device *hdev;

	z9100_smf_regs = ioport_map(Z9100_SMF_IO_BASE, Z9100_SMF_IO_SIZE);

        if (!z9100_smf_regs) {
	     pr_err("Dell Z9100 SMF: unabled to map iomem\n");
	     ret = -ENODEV;
	     goto err_exit;
	}

	hdev = hwmon_device_register_with_groups(&dev->dev, driver_name,
						 NULL,
						 dell_z9100_smf_groups);
	if (IS_ERR(hdev)) {
		dev_err(&dev->dev, "hwmon registration failed");
		ret = PTR_ERR(hdev);
		goto err_unmap;
	}
	dev_info(&dev->dev, "version "DRIVER_VERSION" successfully loaded\n" );
	return ret;

err_unmap:
        iounmap(z9100_smf_regs);
err_exit:
        return ret;
}

static int dell_z9100_smf_remove(struct platform_device * ofdev)
{
	iounmap(z9100_smf_regs);
	platform_set_drvdata(ofdev, NULL);
	dev_info(&ofdev->dev, "removed\n");
	return 0;
}

static struct platform_driver dell_z9100_smf_driver = {
	.probe = dell_z9100_smf_probe,
	.remove = dell_z9100_smf_remove,
	.driver = {
		.name  = driver_name,
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
static struct platform_device *dell_z9100_smf_device;

static int __init dell_z9100_smf_init(void)
{
	int rv;

	rv = platform_driver_register(&dell_z9100_smf_driver);
	if (rv) {
		goto err_exit;
	}

	dell_z9100_smf_device = platform_device_alloc(driver_name, 0);
        if (!dell_z9100_smf_device) {
	     pr_err("platform_device_alloc() failed for sm device\n");
	     rv = -ENOMEM;
	     goto err_unregister;
        }

	rv = platform_device_add(dell_z9100_smf_device);
        if (rv) {
	     pr_err("platform_device_add() failed for sm device.\n");
	     goto err_dealloc;
        }
        return 0;

err_dealloc:
	platform_device_unregister(dell_z9100_smf_device);

err_unregister:
        platform_driver_unregister(&dell_z9100_smf_driver);

err_exit:
	printk(KERN_ERR "%s platform_driver_register failed (%i)\n",
	       driver_name, rv);

	return rv;
}

static void __exit dell_z9100_smf_exit(void)
{
	platform_device_unregister(dell_z9100_smf_device);
	platform_driver_unregister(&dell_z9100_smf_driver);
}

MODULE_AUTHOR("Puneet Shenoy <puneet@cumulusnetworks.com");
MODULE_DESCRIPTION("Smartfusion Driver for Dell Z9100");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(dell_z9100_smf_init);
module_exit(dell_z9100_smf_exit);
