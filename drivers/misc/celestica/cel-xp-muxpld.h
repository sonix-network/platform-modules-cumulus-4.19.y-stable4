


struct cel_xp_bus_mux_info {
	int bus;
	u32 io_base;
	int first_port_num;
	int num_ports;
};

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
};
