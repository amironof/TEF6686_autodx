#include "i2c_helpers.h"  
#include "driver/i2c_master.h"

i2c_master_bus_handle_t create_i2c_bus(i2c_port_t port, gpio_num_t sda, gpio_num_t scl)
	{
		esp_err_t ret;

	    i2c_master_bus_config_t bus_cfg = {
	        .i2c_port = port,
	        .sda_io_num = sda,
	        .scl_io_num = scl,
	        .clk_source = I2C_CLK_SRC_DEFAULT,
	        .glitch_ignore_cnt = 7,
	        .flags.enable_internal_pullup = true,
    	};

	    i2c_master_bus_handle_t bus_handle;
	    
	    ret = i2c_new_master_bus(&bus_cfg, &bus_handle);
	    if (ret != ESP_OK) {
	        printf("Failed to create I2C bus: %d\n", ret);
	        return NULL;
	    }
	    return bus_handle;
	}
	
i2c_master_dev_handle_t add_i2c_device(i2c_master_bus_handle_t bus_handle, uint16_t dev_addr, uint32_t i2c_freq)
	{
		esp_err_t ret;

	    i2c_device_config_t dev_cfg = {
	        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
	        .device_address = dev_addr,
	        .scl_speed_hz = i2c_freq,
	    };
	
	    i2c_master_dev_handle_t dev_handle;
	    ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
	    if (ret != ESP_OK) {
	        printf("Failed to add I2C device: %d\n", ret);
	        return NULL;
	        
	    }
	    return dev_handle;
	}	