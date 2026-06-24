#pragma once

#include "driver/i2c_master.h"
#include "display_helper.h"

typedef struct {
    i2c_master_dev_handle_t tef_handle;
    lcd1602_context *display_context;
} dev_handles_t;

typedef enum {
    MODE_MONITORING,
    MODE_RADIO,
    MODE_SERVICE,
    MODE_SETTINGS,
    MODE_COUNT
} work_mode_t;

extern work_mode_t current_mode;