


#define CPLD_I2C_50KHZ              (63)
#define CPLD_I2C_100KHZ             (31)
#define CPLD_I2C_200KHZ             (15)
#define CPLD_I2C_400KHZ              (7)

struct cpld_bus_data {
	int io_base;
	uint32_t clock;
	uint32_t timeout;
	int bus;
};

struct cpld_mux_data {
	struct i2c_adapter *parent_adapter;
	int mux_base_port_num;
	int mux_num_ports;
	int mux_base_id;
	int mux_ports_base_bus;
};
