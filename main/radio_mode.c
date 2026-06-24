#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "common_types.h"
#include "display_helper.h"
#include "TEF6686_driver.h"
#include "radio_mode.h"

static char rdsProgramService[9] = {0};

void searching_task(void *args)
{
    radio_mode_args_t *radio_args = (radio_mode_args_t *) args;
    QueueHandle_t search_queue = radio_args->search_queue;
    i2c_master_dev_handle_t tef = radio_args->dev->tef_handle;
    lcd1602_context *display = radio_args->dev->display_context;
    uint16_t *current_freq = radio_args->current_freq;

    int8_t direction;
    char text_to_display[16];

    while (1) {
        xQueueReceive(search_queue, &direction, portMAX_DELAY);
        while (direction != 0) {
			memset(rdsProgramService, 0, sizeof(rdsProgramService));
            *current_freq = single_press_FM_tune_to_TEF(*current_freq, direction, tef);
            lcd1602_clear(display);
            snprintf(text_to_display, sizeof(text_to_display), "%d.%02d MHz", *current_freq / 100, *current_freq % 100);
            display_print(text_to_display, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
            xQueueReceive(search_queue, &direction, 0);
        }
    }
}

static void parse_PS(RDS_data_t *rds) {
    if (rds->group_number != 0) return;
    if (rds->err_d > 1) return;

    uint8_t rdsBLow  = (uint8_t)(rds->b_block);
    uint8_t rdsDHigh = (uint8_t)(rds->d_block >> 8);
    uint8_t rdsDLow  = (uint8_t)(rds->d_block);

    uint8_t address = rdsBLow & 3;
    if (rdsDHigh != '\0') rdsProgramService[address * 2]     = rdsDHigh;
    if (rdsDLow  != '\0') rdsProgramService[address * 2 + 1] = rdsDLow;
    rdsProgramService[8] = '\0';
}

void get_signal_info(void *pvParameters)
{
    RDS_data_t rds_data = {0};
    freq_q_info_t signal_info;
    agc_data_t agc_data;
    char level_to_display[16];
    char agc_to_display[16];
    char bw_to_display[16];
    char rds_pi_display[16];

    while (1) {
		for (int8_t i = 100; i > 0; i--) {
			rds_data = get_RDS_status_TEF();
        	parse_PS(&rds_data);
        	vTaskDelay(pdMS_TO_TICKS(20));
		}

        signal_info = TEF_Get_Q_Data_Struct();
        agc_data = get_AGC_TEF();
        
        snprintf(level_to_display, sizeof(level_to_display), "Level: %d", (int)signal_info.level);
        snprintf(agc_to_display, sizeof(agc_to_display), "AGC: %d", (int)agc_data.input_att);
        snprintf(bw_to_display, sizeof(bw_to_display), "BW: %d", (int)signal_info.bandwidth);
        snprintf(rds_pi_display, sizeof(rds_pi_display), "PI: %04X", (int)rds_data.a_block);
        
        display_print("  ", 1, 7);
        display_print(level_to_display, 1, 0);
        display_print("  ", 1, 13);
        display_print(agc_to_display, 1, 13);
        display_print("   ", 2, 4);
        display_print(bw_to_display, 2, 0);
        display_print(rds_pi_display, 3, 0);
        display_print("        ", 3, 11);
        display_print(rdsProgramService, 3, 11);
        
        ESP_LOGI("RDS", "PS: %s", rdsProgramService);
        ESP_LOGI("Signal", "Level: %.1hd", signal_info.level);
        ESP_LOGI("RDS", "PI: %04X", rds_data.a_block);
        
    }
}