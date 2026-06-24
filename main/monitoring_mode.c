#include "monitoring_mode.h"
#include "TEF6686_driver.h"
#include "common_types.h"
#include "driver/i2c_master.h"
#include "i2c_helpers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_helper.h"
#include <string.h>
#include "esp_log.h"
#include "display_helper.h"

bool is_station_local(uint16_t freq)
	{
	    switch(freq) {
	        case 9500:
	        case 9590:
	        case 9670:
	        case 9800:
	        case 9840:
	        case 9880:
	        case 9920:
	        case 9960:
	        case 10000:
	        case 10040:
	        case 10100:
	        case 10140:
	        case 10180:
	        case 10220:
	        case 10310:
	        case 10350:
	        case 10430:
	        case 10470:
	        case 10520:
	        case 10570:
	        case 10680:
	        case 10720:
	        case 10790:
	            return true;
	        default:
	            return false;
	    }
	}

void monitoring_parse_rds_PS(RDS_data_t *rds, char rds_ps[9])
	{
	    if (rds->group_number != 0) return;
	    if (rds->err_d > 1) return;
	
	    uint8_t rdsBLow  = (uint8_t)(rds->b_block);
	    uint8_t rdsDHigh = (uint8_t)(rds->d_block >> 8);
	    uint8_t rdsDLow  = (uint8_t)(rds->d_block);
	
	    uint8_t address = rdsBLow & 3;
	    rds_ps[address * 2]     = rdsDHigh;
	    rds_ps[address * 2 + 1] = rdsDLow;
	    rds_ps[8] = '\0';
	}

mon_rds_info_t monitoring_rds_info()
	{
	    char rds_ps[9] = {0};
	    RDS_data_t rds_data = {0};
		mon_rds_info_t monitoring_rds_info = {0};
		uint16_t pi = 0;
		
		for (int16_t i = 1000; i > 0; i--) {
			rds_data = get_RDS_status_TEF();
			
		    if (rds_data.a_block != 0 && rds_data.err_a <= 1) {
		        pi = rds_data.a_block;
		    }
			
	    	monitoring_parse_rds_PS(&rds_data, rds_ps);
	    	vTaskDelay(pdMS_TO_TICKS(20));
		}	        
		
		monitoring_rds_info.rds_pi = pi;
		
		strcpy(monitoring_rds_info.rds_ps, rds_ps);
		
		return monitoring_rds_info;
		
	}

void monitoring_freq_task(void *args)
	{
		dev_handles_t *dev = (dev_handles_t *) args;
    	i2c_master_dev_handle_t tef = dev->tef_handle;
		
		char json[10000];
		
		freq_q_info_t freq_quality_data;
		processing_status_data_t freq_processing_data;
		agc_data_t agc_data;
		
		mon_rds_info_t rds_info = {0};
				
		while (1){
		
			int16_t pos = 0;
			memset(json, 0, sizeof(json));
			agc_data = get_AGC_TEF();
			pos += snprintf(json + pos, sizeof(json) - pos,
		        "{\"input_att\":%d,\"feedback_att\":%d,\"data\":[",
		        agc_data.input_att,
		        agc_data.feedback_att
		    );
			for (uint16_t testing_freq = 8750; testing_freq <= 10800; testing_freq+=10)
				{
					vTaskDelay(pdMS_TO_TICKS(50));
					FM_tune_to_TEF(tef, testing_freq);
					vTaskDelay(pdMS_TO_TICKS(50));
					freq_quality_data = TEF_Get_Q_Data_Struct();
					
					memset(&rds_info, 0, sizeof(rds_info));
					
					if ((!is_station_local(testing_freq)) && (freq_quality_data.level > 12) && (freq_quality_data.bandwidth > 100))
						{
							rds_info = monitoring_rds_info();
							ESP_LOGI("STATION", "FREQ: %d", testing_freq);
							ESP_LOGI("RDS_INFO", "RDS_PI: %04X", rds_info.rds_pi);
							ESP_LOGI("RDS_INFO", "RDS_PS: %s", rds_info.rds_ps);
						}
					
					freq_processing_data = get_Processing_Status_TEF();
					pos += snprintf(json + pos, sizeof(json) - pos,
			        "[%d,%d,%d,%d,%d,%d,%d,%d,\"%04X\",\"%s\"]%s",
			        testing_freq,
			        freq_quality_data.level,
			        freq_quality_data.usn,
			        freq_quality_data.wam,
			        freq_quality_data.offset,
			        freq_quality_data.bandwidth,
			        freq_quality_data.modulation,
			        freq_processing_data.stereo_status,
			        rds_info.rds_pi,
			        rds_info.rds_ps,
			        testing_freq == 10800 ? "" : ","
			    );
					
				}
			
			pos += snprintf(json + pos, sizeof(json) - pos, "]}");
			
			//printf("%s\n", json);
			http_send_json(json);
			
			vTaskDelay(pdMS_TO_TICKS(60000)); 
		}
		
	}
	
	