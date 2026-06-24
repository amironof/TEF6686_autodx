#include "mode_helper.h"
#include "common_types.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lcd1602/lcd1602.h"
#include "service_mode.h"
#include "wifi_helper.h"
#include "monitoring_mode.h"
#include "radio_mode.h"

static TaskHandle_t panorama_task_handle = NULL;
static TaskHandle_t single_freq_task_handle = NULL;
static TaskHandle_t monitoring_freq_task_handle = NULL;
static TaskHandle_t get_signal_info_task_handle = NULL;

static dev_handles_t dev_handles_internal;

typedef enum {
    SERVICE_SINGLE_FREQ,
    SERVICE_PANORAMA,
    SERVICE_COUNT
} service_submode_t;

static service_submode_t service_submode = SERVICE_SINGLE_FREQ;

void mode_helper_init(dev_handles_t dev_handle)
	{
		dev_handles_internal = dev_handle;
	}

void mode_switch()
	{
		
	switch (current_mode) {
        case MODE_SERVICE: service_mode_exit(); break;
        case MODE_MONITORING: monitoring_mode_exit(); break;
        case MODE_RADIO: radio_mode_exit(); break;
        default: break;
    }	
		
    current_mode++;
    if (current_mode >= MODE_COUNT) {
        current_mode = MODE_MONITORING;
    }
	
	ESP_LOGI("mode_helper", "Switched to mode: %d", current_mode);
	
    switch (current_mode) {
        case MODE_RADIO:    radio_mode_enter();     break;
        case MODE_MONITORING:  monitoring_mode_enter();  break;
        case MODE_SERVICE:  service_mode_enter();  break;
        case MODE_SETTINGS: settings_mode_enter();   break;
        default: break;
    }
	}

void radio_mode_enter()
	{
		lcd1602_set_backlight(dev_handles_internal.display_context, true);
		lcd1602_clear(dev_handles_internal.display_context);
		display_print("Radio mode", 0, 0);
		
		audio_set_mute_TEF();
		
		xTaskCreate(get_signal_info, "get_signal_info_task", 8192, NULL, 1, &get_signal_info_task_handle);
	
	}	
	
void radio_mode_exit()
	{
		vTaskDelete (get_signal_info_task_handle);
	}	
	
void monitoring_mode_enter()
	{	
		lcd1602_set_backlight(dev_handles_internal.display_context, false);
		lcd1602_clear(dev_handles_internal.display_context);
		display_print("Monitoring mode", 0, 0); 
		xTaskCreate(monitoring_freq_task, "monitoring_freq_task", 24576, &dev_handles_internal, 3, &monitoring_freq_task_handle);
	}	

void monitoring_mode_exit() {
    if (monitoring_freq_task_handle != NULL) {
        vTaskDelete(monitoring_freq_task_handle);
        monitoring_freq_task_handle = NULL;
    	}
}
	
void service_mode_enter()
	{
	lcd1602_set_backlight(dev_handles_internal.display_context, false);
	lcd1602_clear(dev_handles_internal.display_context);
	display_print("Service mode", 0, 0);
	display_print("UART: Single freq.", 1, 0);
	xTaskCreate(FM_SingleFreq_Task, "FM_SingleFreq", 4096,&dev_handles_internal, 5, &single_freq_task_handle);
	}

void service_submode_switch() {
    service_mode_exit();

    service_submode++;
    if (service_submode >= SERVICE_COUNT) {
        service_submode = SERVICE_SINGLE_FREQ;
    }

    switch (service_submode) {
        case SERVICE_SINGLE_FREQ:
        	lcd1602_clear(dev_handles_internal.display_context);
        	display_print("Service mode", 0, 0);
            display_print("UART: Single freq.", 1, 0);
            xTaskCreate(FM_SingleFreq_Task, "FM_SingleFreq", 4096, &dev_handles_internal, 5, &single_freq_task_handle);
            break;
        case SERVICE_PANORAMA:
        	lcd1602_clear(dev_handles_internal.display_context);
        	display_print("Service mode", 0, 0);
            display_print("UART: Panorama", 1, 0);
            xTaskCreate(FM_Panorama_Task, "fm_panorama", 4096, &dev_handles_internal, 1, &panorama_task_handle);
            break;
        default:
        	break;    
    }
}		

void service_mode_exit() {
    if (panorama_task_handle != NULL) {
        vTaskDelete(panorama_task_handle);
        panorama_task_handle = NULL;
    	}
    if (single_freq_task_handle != NULL) {
        vTaskDelete(single_freq_task_handle);
        single_freq_task_handle = NULL;    
    	}
}

void settings_mode_enter()
	{
	lcd1602_set_backlight(dev_handles_internal.display_context, false);
	lcd1602_clear(dev_handles_internal.display_context);
	display_print("Settings mode", 0, 0);
	}
	
