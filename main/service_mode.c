#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "TEF6686_driver.h"
#include "i2c_helpers.h"
#include "common_types.h"

extern uint16_t current_freq;

void FM_Panorama_Task(void *args)
{
    dev_handles_t *dev = (dev_handles_t *) args;
    i2c_master_dev_handle_t tef = dev->tef_handle;

    int16_t freq = 8750;

    printf("PANORAMA START\n");

    while (1)
    {
        turn_on_FM_tune_to_TEF( freq);
        vTaskDelay(pdMS_TO_TICKS(30));

        uint8_t cmd[3] = {0x20, 0x81, 0x01};
        uint8_t resp[14];

        if (i2c_master_transmit_receive(tef, cmd, sizeof(cmd), resp, sizeof(resp), pdMS_TO_TICKS(100)) == ESP_OK)
        {
            uint16_t status = (resp[0] << 8) | resp[1];
            uint16_t timestamp = status & 0x03FF;

            if (timestamp != 0)
            {
                int16_t  level      = (resp[2]  << 8) | resp[3];
                uint16_t usn        = (resp[4]  << 8) | resp[5];
                uint16_t wam        = (resp[6]  << 8) | resp[7];
                int16_t  offset     = (resp[8]  << 8) | resp[9];
                uint16_t bandwidth  = (resp[10] << 8) | resp[11];
                uint16_t modulation = (resp[12] << 8) | resp[13];

                processing_status_data_t processing_statuses = get_Processing_Status_TEF();

                printf("%d.%02d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
                       freq / 100,
                       freq % 100,
                       level / 10.0f,
                       usn / 10.0f,
                       wam / 10.0f,
                       offset / 10.0f,
                       bandwidth / 10.0f,
                       modulation / 10.0f,
                       processing_statuses.softmute_status * 0.1f,
                       processing_statuses.highcut_status  * 0.1f,
                       processing_statuses.stereo_status   * 0.1f,
                       processing_statuses.sthiblend_status * 0.1f
                );
            }  // if (timestamp != 0)
        }  // if (ESP_OK)

        freq += 5;
        if (freq > 10800) freq = 8750;

    }  // while(1)

    vTaskDelete(NULL);
}

void FM_SingleFreq_Task(void *args)
{
    dev_handles_t *dev = (dev_handles_t *) args;
    i2c_master_dev_handle_t tef = dev->tef_handle;

    //const int16_t freq = 10070; // 93.4 МГц (в твоих единицах: 93.4*100)
    
    const int num_samples = 10;
    //const TickType_t sample_delay = pdMS_TO_TICKS(100);
    const TickType_t pause_delay = pdMS_TO_TICKS(100);
	
	turn_on_FM_tune_to_TEF(current_freq);
	
    // printf("SINGLE FREQ MEASUREMENT START\n");

    while (1)
    {
        int32_t sum_level = 0;
        int32_t sum_usn   = 0;
        int32_t sum_wam   = 0;
        int32_t sum_offset = 0;
        int32_t sum_bandwidth = 0;
        int32_t sum_modulation = 0;

        for (int i = 0; i < num_samples; i++)
        {
            
            vTaskDelay(pdMS_TO_TICKS(30)); // ждём стабилизации

            uint8_t cmd[3] = {0x20, 0x81, 0x01};
            uint8_t resp[14];

            if (i2c_master_transmit_receive(tef, cmd, sizeof(cmd), resp, sizeof(resp), pdMS_TO_TICKS(100)) == ESP_OK)
            {
                uint16_t status = (resp[0] << 8) | resp[1];
                uint16_t timestamp = status & 0x03FF;

                if (timestamp != 0)
                {
                    int16_t  level      = (resp[2]  << 8) | resp[3];
                    uint16_t usn        = (resp[4]  << 8) | resp[5];
                    uint16_t wam        = (resp[6]  << 8) | resp[7];
                    int16_t  offset     = (resp[8]  << 8) | resp[9];
                    uint16_t bandwidth  = (resp[10] << 8) | resp[11];
                    uint16_t modulation = (resp[12] << 8) | resp[13];

                    sum_level      += level;
                    sum_usn        += usn;
                    sum_wam        += wam;
                    sum_offset     += offset;
                    sum_bandwidth  += bandwidth;
                    sum_modulation += modulation;
                }
            }

            //vTaskDelay(sample_delay); // интервал между замерами
        }
		
		//vTaskDelay(pdMS_TO_TICKS(100));
		//agc_data_t agc_info = get_Get_AGC_TEF(tef);
		
		//processing_status_data_t processing_statuses = get_Processing_Status_TEF(tef);
		
        // усредняем и выводим в формате "для человека"
        /*
        printf("FREQ %.2f MHz, LEVEL=%.1f, USN=%.1f, WAM=%.1f, OFFSET=%.1f, BW=%.1f, MOD=%.1f, INPUT_ATT=%.1f, FEEDBACK_ATT=%.1f\n",
	       current_freq / 100.0f,
	       sum_level / (float)num_samples / 10.0f,
	       sum_usn   / (float)num_samples / 10.0f,
	       sum_wam   / (float)num_samples / 10.0f,
	       sum_offset/ (float)num_samples / 10.0f,
	       sum_bandwidth / (float)num_samples / 10.0f,
	       sum_modulation / (float)num_samples / 10.0f,
	       agc_info.input_att / 10.0f,
	       agc_info.feedback_att / 10.0f);
		*/
		
		// это вывод для питона
		
		printf("%.2f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
		       current_freq / 100.0f,
		       sum_level / (float)num_samples / 10.0f,
		       sum_usn   / (float)num_samples / 10.0f,
		       sum_wam   / (float)num_samples / 10.0f,
		       sum_offset/ (float)num_samples / 10.0f,
		       sum_bandwidth / (float)num_samples / 10.0f,
		       sum_modulation / (float)num_samples / 10.0f);
		       
		       //processing_statuses.softmute_status * 0.1f,
		       //processing_statuses.highcut_status  * 0.1f,
		       //processing_statuses.stereo_status   * 0.1f,
		       //processing_statuses.sthiblend_status * 0.1f);
		       //agc_info.input_att / 10.0f,      // 0..42 dB
		       //agc_info.feedback_att / 10.0f);  // 0..6 dB

        vTaskDelay(pause_delay); // пауза 5 секунд перед следующим циклом
    }
    


    vTaskDelete(NULL); // формально никогда сюда не дойдём
}	