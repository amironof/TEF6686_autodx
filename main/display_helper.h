#pragma once
#include "lcd1602/lcd1602.h"

lcd1602_context *display_init(i2c_master_bus_handle_t i2c_bus_handle, i2c_port_t port, int sda, int scl, uint8_t dev_addr);

void display_print(char *text, uint8_t row, uint8_t column);