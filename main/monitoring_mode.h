#pragma once
#include "driver/i2c_master.h"
#include "TEF6686_driver.h"

typedef struct {
    uint16_t rds_pi;
    char rds_ps[9];
} mon_rds_info_t;

bool is_station_local(uint16_t freq);

void monitoring_parse_rds_PS(RDS_data_t *rds, char rds_ps[9]);

mon_rds_info_t monitoring_rds_info();

void monitoring_freq_task(void *args);
