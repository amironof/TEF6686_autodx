#include "buttons_helper.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "iot_button.h"
#include "button_gpio.h"
#include "esp_log.h"
#include "mode_helper.h"

static const char *TAG = "BUTTONS_HELPER";

void btn_menu_single_click_cb(void *arg,void *usr_data)
	{
	   	btn_event_t evt = BTN_EVT_MENU;
	   	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	    QueueHandle_t queue = (QueueHandle_t) usr_data;
	    xQueueSendFromISR(queue, &evt, &xHigherPriorityTaskWoken);
	}

void btn_ctrl_1_single_click_cb(void *arg,void *usr_data)
	{
	   	btn_event_t evt = BTN_EVT_CTRL_1;
	   	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	    QueueHandle_t queue = (QueueHandle_t) usr_data;
	    xQueueSendFromISR(queue, &evt, &xHigherPriorityTaskWoken);
	}
	
void btn_ctrl_2_single_click_cb(void *arg,void *usr_data)
	{
	   	btn_event_t evt = BTN_EVT_CTRL_2;
	   	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	    QueueHandle_t queue = (QueueHandle_t) usr_data;
	    xQueueSendFromISR(queue, &evt, &xHigherPriorityTaskWoken);
	}	
	
void btn_ctrl_3_single_click_cb(void *arg,void *usr_data)
	{
	   	btn_event_t evt = BTN_EVT_CTRL_3;
	   	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	    QueueHandle_t queue = (QueueHandle_t) usr_data;
	    xQueueSendFromISR(queue, &evt, &xHigherPriorityTaskWoken);
	}	

void btn_search_left_single_click_cb(void *arg,void *usr_data)
	{
	   	btn_event_t evt = BTN_EVT_LEFT_DOWN;
	   	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	    QueueHandle_t queue = (QueueHandle_t) usr_data;
	    xQueueSendFromISR(queue, &evt, &xHigherPriorityTaskWoken);
	}

void btn_search_right_single_click_cb(void *arg,void *usr_data)
	{
	   	btn_event_t evt = BTN_EVT_RIGHT_DOWN;
	   	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	    QueueHandle_t queue = (QueueHandle_t) usr_data;
	    xQueueSendFromISR(queue, &evt, &xHigherPriorityTaskWoken);
	}

	
void btn_search_left_up_cb(void *arg,void *usr_data)
	{
	   	btn_event_t evt = BTN_EVT_LEFT_UP;
	   	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	    QueueHandle_t queue = (QueueHandle_t) usr_data;
	    xQueueSendFromISR(queue, &evt, &xHigherPriorityTaskWoken);
	}

void btn_search_right_up_cb(void *arg,void *usr_data)
	{
	   	btn_event_t evt = BTN_EVT_RIGHT_UP;
	   	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	    QueueHandle_t queue = (QueueHandle_t) usr_data;
	    xQueueSendFromISR(queue, &evt, &xHigherPriorityTaskWoken);
	}	
	
void buttons_init(QueueHandle_t buttons_queue, gpio_num_t btn_menu, gpio_num_t btn_ctrl_1, gpio_num_t btn_ctrl_2, gpio_num_t btn_ctrl_3, gpio_num_t btn_left, gpio_num_t btn_right)
	{
		const button_config_t btn_cfg = {0};
		
		button_gpio_config_t btn_gpio_cfg_menu = {
		    .gpio_num = btn_menu,
		    .active_level = 0,
		};
		button_handle_t btn_menu_handle = NULL;
		
		ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg_menu, &btn_menu_handle));
		ESP_ERROR_CHECK(iot_button_register_cb(btn_menu_handle, BUTTON_PRESS_DOWN, NULL, btn_menu_single_click_cb, buttons_queue));

		
		button_gpio_config_t btn_gpio_cfg_ctrl_1 = {
		    .gpio_num = btn_ctrl_1,
		    .active_level = 0,
		};
		button_handle_t btn_ctrl_1_handle = NULL;
		
		ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg_ctrl_1, &btn_ctrl_1_handle));
		ESP_ERROR_CHECK(iot_button_register_cb(btn_ctrl_1_handle, BUTTON_PRESS_DOWN, NULL, btn_ctrl_1_single_click_cb, buttons_queue));
		
		
		button_gpio_config_t btn_gpio_cfg_ctrl_2 = {
		    .gpio_num = btn_ctrl_2,
		    .active_level = 0,
		};
		button_handle_t btn_ctrl_2_handle = NULL;
		
		ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg_ctrl_2, &btn_ctrl_2_handle));
		ESP_ERROR_CHECK(iot_button_register_cb(btn_ctrl_2_handle, BUTTON_PRESS_DOWN, NULL, btn_ctrl_2_single_click_cb, buttons_queue));
		
		
		button_gpio_config_t btn_gpio_cfg_ctrl_3 = {
		    .gpio_num = btn_ctrl_3,
		    .active_level = 0,
		};
		button_handle_t btn_ctrl_3_handle = NULL;
		
		ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg_ctrl_3, &btn_ctrl_3_handle));
		ESP_ERROR_CHECK(iot_button_register_cb(btn_ctrl_3_handle, BUTTON_PRESS_DOWN, NULL, btn_ctrl_3_single_click_cb, buttons_queue));
		

		button_gpio_config_t btn_gpio_cfg_search_left = {
		    .gpio_num = btn_left,
		    .active_level = 0,
		};
		button_handle_t btn_search_left_handle = NULL;
		
		ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg_search_left, &btn_search_left_handle));
		ESP_ERROR_CHECK(iot_button_register_cb(btn_search_left_handle, BUTTON_PRESS_DOWN, NULL, btn_search_left_single_click_cb, buttons_queue));
		ESP_ERROR_CHECK(iot_button_register_cb(btn_search_left_handle, BUTTON_PRESS_UP, NULL, btn_search_left_up_cb, buttons_queue));
		

		button_gpio_config_t btn_gpio_cfg_search_right = {
		    .gpio_num = btn_right,
		    .active_level = 0,
		};
		button_handle_t btn_search_right_handle = NULL;
		
		ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg_search_right, &btn_search_right_handle));
		ESP_ERROR_CHECK(iot_button_register_cb(btn_search_right_handle, BUTTON_PRESS_DOWN, NULL, btn_search_right_single_click_cb, buttons_queue));
		ESP_ERROR_CHECK(iot_button_register_cb(btn_search_right_handle, BUTTON_PRESS_UP, NULL, btn_search_right_up_cb, buttons_queue));
		
	
	}
	
void buttons_handler(void *args)
	{
	    handler_args_t *handler_args = (handler_args_t *) args;
	    QueueHandle_t queue = handler_args->buttons_queue;
	    QueueHandle_t search_queue = handler_args->search_queue;
	    btn_event_t evt;
	
	    while (1) {
	        xQueueReceive(queue, &evt, portMAX_DELAY);
	        switch (evt) {
	            case BTN_EVT_MENU: {
					mode_switch();
					break;
				} 
	            case BTN_EVT_CTRL_1:     
	            	    if (current_mode == MODE_SERVICE) {
				        service_submode_switch();
				    }
	                break;
	            case BTN_EVT_CTRL_2:     ESP_LOGI(TAG, "BTN_CTRL_2 pressed");    break;
	            case BTN_EVT_CTRL_3:     ESP_LOGI(TAG, "BTN_CTRL_3 pressed");    break;
	            case BTN_EVT_LEFT_DOWN: {
	                 if (current_mode == MODE_RADIO) {
		                int8_t dir = -1;
		                        BaseType_t res = xQueueSend(search_queue, &dir, 0);
						        ESP_LOGI("BTN", "LEFT_DOWN sent to queue, result: %d, mode: %d", res, current_mode);
						    } else {
						        ESP_LOGI("BTN", "LEFT_DOWN ignored, mode: %d", current_mode); // сюда не должны попадать
	                }
	                break;
	            }
	            case BTN_EVT_LEFT_UP: {
	                 if (current_mode == MODE_RADIO) {
		                int8_t dir = 0;
		                xQueueSend(search_queue, &dir, 0);
	                }
	                break;
	            }
	            case BTN_EVT_RIGHT_DOWN: {
	                 if (current_mode == MODE_RADIO) {
		                int8_t dir = 1;
		                xQueueSend(search_queue, &dir, 0);
	                }
	                break;
	            }
	            case BTN_EVT_RIGHT_UP: {
					 if (current_mode == MODE_RADIO) {
		                int8_t dir = 0;
		                xQueueSend(search_queue, &dir, 0);
	                }
	                break;
	            }
	        }
	    }
	}
	