/*
 * cel_fpga_i2c.h - Header file for Celestica FPGA I2C bus
 *
 * Copyright (C) 2019 Cumulus Networks, Inc.  All rights reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 */

#define PCI_DEVICE_ID_XILINX_FPGA    0x7021
#define FPGA_BLOCK_MAX		     32
#define FPGA_MAX_RETRIES	     5
#define FPGA_BAR		     0
#define FPGA_DESC_ENTRIES	     2
#define FPGA_FAILED_READ_REG	     0xffffffffU

#define CEL_FPGA_I2C_DRIVER_NAME    "cel_fpga_i2c"
#define FPGA_I2C_50KHZ              (63)
#define FPGA_I2C_100KHZ             (31)
#define FPGA_I2C_200KHZ             (15)
#define FPGA_I2C_400KHZ              (7)

struct fpga_i2c_device_info {
	int bus;
	struct i2c_board_info *info;
};

struct cel_fpga_i2c_bus_data {
        int io_base;
        uint32_t clock;
        uint32_t timeout;
        int bus;
};

struct fpga_mux_data {
        struct i2c_adapter *parent_adapter;
        int mux_base_port_num;
        int mux_num_ports;
        int mux_base_id;
        int mux_ports_base_bus;
};

struct fpga_i2c_platform_data {
        u32 reg_shift; /* register offset shift value */
        u32 reg_io_width; /* register io read/write width */
        u32 clock_khz; /* input clock in kHz */
        u8 num_devices; /* number of devices in the devices list */
        struct fpga_i2c_device_info *devices; /* devs connected to the bus */
};
