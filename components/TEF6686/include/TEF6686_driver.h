#pragma once


#include "driver/i2c_master.h"

typedef struct {
    uint16_t input_att;
    int16_t  feedback_att;
} agc_data_t;

typedef struct {
	uint16_t softmute_status;
	uint16_t highcut_status;
	uint16_t stereo_status;
	uint16_t sthiblend_status;
} processing_status_data_t;

typedef struct {
    int16_t  level;
    uint16_t usn;
    uint16_t wam;
    int16_t  offset;
    uint16_t bandwidth;
    uint16_t modulation;
} freq_q_info_t;

typedef struct {
    uint8_t data_available;
    uint8_t data_lost;
    uint8_t data_type;
    uint8_t group_version;
    uint8_t group_number; 
    uint8_t sync;
    uint16_t a_block;
    uint16_t b_block;
    uint16_t c_block;
    uint16_t d_block;
    uint8_t err_a;
    uint8_t err_b;
    uint8_t err_c;
    uint8_t err_d;
} RDS_data_t;

void tuner_first_patch_load_TEF(i2c_master_dev_handle_t dev_handle, const uint8_t *patch_data, uint32_t size);

void tuner_second_patch_load_TEF(i2c_master_dev_handle_t dev_handle, const uint8_t *patch_data, uint32_t size);

void get_operation_status_TEF(i2c_master_dev_handle_t dev_handle);

void start_TEF(i2c_master_dev_handle_t dev_handle);

void get_id_TEF(i2c_master_dev_handle_t dev_handle);

void APPL_activate_TEF(i2c_master_dev_handle_t dev_handle);

void turn_on_FM_tune_to_TEF(int16_t first_freq);

void FM_tune_to_TEF(i2c_master_dev_handle_t dev_handle, int16_t freq_to_tune);

void audio_set_mute_TEF();

void init_TEF(i2c_master_dev_handle_t dev_handle);

int16_t single_press_FM_tune_to_TEF(int16_t current_freq, int8_t direction, i2c_master_dev_handle_t dev_handle);

void TEF_Get_Quality_Data();

freq_q_info_t TEF_Get_Q_Data_Struct();

void set_bandwidth_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id);

void test_set_bandwidth_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id);

void Set_RFAGC_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id);

void set_MphSuppression_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id);

void set_channelEqualizer_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id);

void set_LevelStep_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id);

void set_LevelOffset_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id);

void set_Bandwidth_Options_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id);

agc_data_t get_AGC_TEF();

processing_status_data_t get_Processing_Status_TEF();

RDS_data_t get_RDS_status_TEF();
