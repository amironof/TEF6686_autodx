#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>
#include "common_types.h"
#include "TEF6686_driver.h"

void searching_task(void *args);
void get_signal_info(void *pvParameters);

typedef struct {
    QueueHandle_t search_queue;
    dev_handles_t *dev;
    uint16_t *current_freq;
} radio_mode_args_t;