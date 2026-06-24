#include "wifi_helper.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_crt_bundle.h"
#include "TEF6686_driver.h"
#include "display_helper.h"
// See wifi_config.h.example
#include "wifi_config.h"

static const char *TAG = "WIFI_HELPER";

// Event handler
void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "AutoDX connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect(); // Reconnect on disconnect
    }
}

void wifi_init_sta(void)
{
    // 1. Initialize TCP/IP stack and event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // 2. Initialize Wi-Fi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    vTaskDelay(pdMS_TO_TICKS(100)); 
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 3. Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // 4. Set mode and config
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // 5. Start Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_start());
}

void http_send_data(freq_q_info_t data_to_send, int8_t station_id)
{
    char url[256];
    snprintf(url, sizeof(url), "get_monitoring_info.php%s?station_id=%d&level=%d&usn=%d&wam=%d&offset=%d&bandwidth=%d&modulation=%d",
        SERVER_URL,
        station_id,
        data_to_send.level,
        data_to_send.usn,
        data_to_send.wam,
        data_to_send.offset,
        data_to_send.bandwidth,
        data_to_send.modulation);

    esp_http_client_config_t config = {
        .url = url,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI("HTTP", "Sent OK, status: %d", esp_http_client_get_status_code(client));

        char response[256] = {0};
        int len = esp_http_client_read(client, response, sizeof(response) - 1);
        if (len > 0) {
            ESP_LOGI("HTTP", "Response: %s", response);
        }
    } else {
        ESP_LOGE("HTTP", "Request failed: %s", esp_err_to_name(err));
    }
    
    esp_http_client_cleanup(client);
}

void http_send_json(const char *json)
{
    char url[128];
    char str[64];
    
    display_print("                    ", 1, 0);
    display_print("                    ", 2, 0);
    display_print("                    ", 3, 0);
        
    snprintf(url, sizeof(url), "%sget_monitoring_json.php", SERVER_URL);
    ESP_LOGI("HTTP", "URL: %s", url);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json, strlen(json));

    esp_err_t err;
    int retries = 3;
    int attempt = 0;

    do {
        attempt++;
        ESP_LOGI("HTTP", "Attempt %d/%d", attempt, 3);

        err = esp_http_client_perform(client);

        if (err == ESP_ERR_HTTP_EAGAIN || err == ESP_ERR_HTTP_CONNECT) {
            ESP_LOGW("HTTP", "Retry in 2s... (%d left)", retries - 1);
            esp_http_client_close(client);
            esp_http_client_set_post_field(client, json, strlen(json));
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    } while ((err == ESP_ERR_HTTP_EAGAIN || err == ESP_ERR_HTTP_CONNECT) && --retries > 0);

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
		
		if (attempt > 1) {
	        ESP_LOGW("HTTP", "Sent after %d attempts!", attempt);
	    }
		
        if (status == 200) {
            ESP_LOGI("HTTP", "Was sent, status: %d", status);
            display_print("HTTP OK: 200 ", 1, 0);

            char response[256] = {0};
            int len = esp_http_client_read(client, response, sizeof(response) - 1);
            if (len > 0) {
                ESP_LOGI("HTTP", "Response: %s", response);
            }
        } else {
            ESP_LOGE("HTTP", "Server error, status: %d", status);
        }
    } else {
        ESP_LOGE("HTTP", "Request failed after %d attempts: %s", attempt, esp_err_to_name(err));
        display_print("ERROR (3 attempts)", 1, 0);

        wifi_ap_record_t ap_info;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            ESP_LOGW("HTTP", "WiFi RSSI: %d dBm", ap_info.rssi);
        }
    }

    esp_http_client_cleanup(client);
    ESP_LOGI("MEM", "Free heap: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI("MEM", "Stack: %d bytes", uxTaskGetStackHighWaterMark(NULL));
    
    snprintf(str, sizeof(str), "Free heap: %" PRIu32, esp_get_free_heap_size());
    display_print(str, 2, 0);
    
    snprintf(str, sizeof(str), "Stack: %d", uxTaskGetStackHighWaterMark(NULL));
    display_print(str, 3, 0);
    
}