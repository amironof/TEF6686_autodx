#pragma once

#include "esp_wifi.h"
#include "esp_event.h"
#include "TEF6686_driver.h"

void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_init_sta(void);
void http_send_data(freq_q_info_t data_to_send, int8_t station_id);
void http_send_json(const char *json);