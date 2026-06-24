#pragma once
#include "driver/i2c_types.h"
#include "driver/gpio.h"

//#include "driver/i2c_master.h"
//#include "driver/gpio.h"

i2c_master_bus_handle_t create_i2c_bus(i2c_port_t port, gpio_num_t sda, gpio_num_t scl);

i2c_master_dev_handle_t add_i2c_device(i2c_master_bus_handle_t bus_handle, uint16_t dev_addr, uint32_t i2c_freq);