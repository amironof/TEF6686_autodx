#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "common_types.h"

typedef enum {
    BTN_EVT_MENU,
    BTN_EVT_CTRL_1,
    BTN_EVT_CTRL_2,
    BTN_EVT_CTRL_3,
    BTN_EVT_LEFT_DOWN,
    BTN_EVT_LEFT_UP,
    BTN_EVT_RIGHT_DOWN,
    BTN_EVT_RIGHT_UP
} btn_event_t;

typedef struct {
    QueueHandle_t buttons_queue;
    QueueHandle_t search_queue;
    dev_handles_t *dev;
} handler_args_t;

void btn_menu_single_click_cb(void *arg,void *usr_data);

void btn_ctrl_1_single_click_cb(void *arg,void *usr_data);

void btn_ctrl_2_single_click_cb(void *arg,void *usr_data);

void btn_ctrl_3_single_click_cb(void *arg,void *usr_data);

void btn_search_left_single_click_cb(void *arg,void *usr_data);

void btn_search_right_single_click_cb(void *arg,void *usr_data);
	
void btn_search_left_short_up_cb(void *arg,void *usr_data);

void btn_search_right_short_up_cb(void *arg,void *usr_data);

void buttons_init(QueueHandle_t buttons_queue, gpio_num_t btn_menu, gpio_num_t btn_ctrl_1, gpio_num_t btn_ctrl_2, gpio_num_t btn_ctrl_3, gpio_num_t btn_left, gpio_num_t btn_right);

void buttons_handler (void *args);