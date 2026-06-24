#include "lcd1602/lcd1602.h"
#include "display_helper.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static lcd1602_context lcd1602_context_internal;

lcd1602_context *display_init(i2c_master_bus_handle_t i2c_bus_handle, i2c_port_t port, int sda, int scl, uint8_t dev_addr)
	{
		bool is_display_inited = 0;
		
		i2c_lowlevel_config lcd_cfg = {0};
		
		lcd_cfg.bus = &i2c_bus_handle;
		lcd_cfg.port = port;
		lcd_cfg.pin_sda = sda;
		lcd_cfg.pin_scl = scl;
		
		lcd1602_context *display_context = NULL;
		
		while (is_display_inited == 0) {
			display_context = lcd1602_init(dev_addr, true, &lcd_cfg);
			
			if (display_context != NULL) {
				lcd1602_set_display(display_context, true, false, false);
				is_display_inited = 1;
			} else {
				ESP_LOGI("DISP", "Display not initialized, retrying...");
				vTaskDelay(pdMS_TO_TICKS(1000));
			}
		}
		
		lcd1602_context_internal = display_context;
		
		return display_context;
	}
	
void display_print(char *text, uint8_t row, uint8_t column)
	{
		lcd1602_set_cursor(lcd1602_context_internal, row, column);
		lcd1602_string(lcd1602_context_internal, text);
	}