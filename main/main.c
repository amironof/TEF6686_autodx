#include <stdio.h>
#include "driver/i2c_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "iot_button.h"
#include "button_gpio.h"
#include "esp_log.h"
#include "TEF6686_driver.h"
#include "i2c_helpers.h"
#include "display_helper.h"
#include "service_mode.h"
#include "common_types.h"
#include "buttons_helper.h"
#include "radio_mode.h"
#include "mode_helper.h"
#include "wifi_helper.h"
#include "nvs_flash.h"

#define I2C_MASTER_SDA_IO  21
#define I2C_MASTER_SCL_IO  22
#define I2C_PORT           I2C_NUM_0
#define I2C_FREQ_HZ        180000   
#define I2C_TEF_ADDR       0x64
#define I2C_DISPLAY_ADDR   0x27

#define BTN_MENU			GPIO_NUM_16
#define BTN_CTRL_1			GPIO_NUM_17
#define BTN_CTRL_2			GPIO_NUM_5
#define BTN_CTRL_3			GPIO_NUM_18
#define BTN_SEARCH_LEFT		GPIO_NUM_19
#define BTN_SEARCH_RIGHT	GPIO_NUM_23

uint16_t current_freq = 10070;
work_mode_t current_mode = MODE_MONITORING;
static QueueHandle_t buttons_queue;
static QueueHandle_t search_queue;

static void system_init(dev_handles_t *dev)
	{
	    i2c_master_bus_handle_t i2c_bus_handle = create_i2c_bus(I2C_PORT, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
	    dev->tef_handle = add_i2c_device(i2c_bus_handle, I2C_TEF_ADDR, I2C_FREQ_HZ);
	    dev->display_context = display_init(i2c_bus_handle, I2C_PORT, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_DISPLAY_ADDR);
		
	    init_TEF(dev->tef_handle);
		mode_helper_init(*dev);
	    
	   	esp_err_t ret = nvs_flash_init();
	    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	        nvs_flash_erase();
	        nvs_flash_init();
    	}
    	
    	wifi_init_sta();
	    
	}
	
static void queues_tasks_init(dev_handles_t *dev) {
	    buttons_queue = xQueueCreate(10, sizeof(btn_event_t));
	    search_queue = xQueueCreate(10, sizeof(int8_t));
	
	    buttons_init(buttons_queue, BTN_MENU, BTN_CTRL_1, BTN_CTRL_2, BTN_CTRL_3, BTN_SEARCH_LEFT, BTN_SEARCH_RIGHT);
	
	    static handler_args_t handler_args;
	    handler_args.buttons_queue = buttons_queue;
	    handler_args.search_queue = search_queue;
	    handler_args.dev = dev;
	
	    xTaskCreate(buttons_handler, "btn_handler", 2048, &handler_args, 2, NULL);
	
	    static radio_mode_args_t radio_args;
	    radio_args.search_queue = search_queue;
	    radio_args.dev = dev;
	    radio_args.current_freq = &current_freq;
	
	    xTaskCreate(searching_task, "searching_task", 8192, &radio_args, 1, NULL);
	}

void app_main(void)
	{
		static dev_handles_t dev_handles;
	    system_init(&dev_handles);
	    vTaskDelay(pdMS_TO_TICKS(100));
	   	queues_tasks_init(&dev_handles);		
	    ESP_LOGI("TEST", "Point 1");
	    monitoring_mode_enter();
	    
	    while (1) {
    	vTaskDelay(pdMS_TO_TICKS(1000));
		}
}