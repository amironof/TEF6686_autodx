#include "TEF6686_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "Tuner_Patch_Lithio_V102_p224.h"
// #include "Tuner_Patch_Lithio_V102_p209.h"

static i2c_master_dev_handle_t tef_handle_internal;

void tuner_first_patch_load_TEF(i2c_master_dev_handle_t dev_handle, const uint8_t *patch_data, uint32_t size)
	{
	    uint8_t cmd[5];

	    // --- Вроде бы эта команда должна ресетить ---
		
	    cmd[0] = 0x1E;
	    cmd[1] = 0x5A;
	    cmd[2] = 0x01;
	    cmd[3] = 0x5A;
	    cmd[4] = 0x5A;
		
	
	    i2c_master_transmit(dev_handle, cmd, 5, pdMS_TO_TICKS(100));
	    vTaskDelay(pdMS_TO_TICKS(100)); // небольшая пауза после ресета
		
		
	    // --- начальная инициализация патча ---
	    cmd[0] = 0x1C; cmd[1] = 0x00; cmd[2] = 0x00;
	    i2c_master_transmit(dev_handle, cmd, 3, pdMS_TO_TICKS(100));
	    vTaskDelay(pdMS_TO_TICKS(100)); // пауза после команды
	
	    cmd[0] = 0x1C; cmd[1] = 0x00; cmd[2] = 0x74;
	    i2c_master_transmit(dev_handle, cmd, 3, pdMS_TO_TICKS(100));
	    vTaskDelay(pdMS_TO_TICKS(100)); // пауза после команды
	
	    // --- загрузка патча ---
	    const uint16_t LINE_SIZE = 24;   // размер строки данных (как в доке)
	    uint8_t buf[LINE_SIZE + 1];      // +1 байт под 0x1B
	    buf[0] = 0x1B;
	
	    printf("First patch load start, size: %lu bytes\n", (unsigned long)size);
	
	    while (size > 0)
	    {
	        uint16_t len = (size > LINE_SIZE) ? LINE_SIZE : size;
	
	        // копируем одну строку
	        for (uint16_t i = 0; i < len; i++)
	        {
	            buf[1 + i] = patch_data[i];
	        }
	
	        printf("Send line (%d bytes): ", len);
	        for (int i = 0; i < len + 1; i++)
	        {
	            printf("%02X ", buf[i]);
	        }
	        printf("\n");
	
	        esp_err_t ret = i2c_master_transmit(dev_handle,
	                                            buf,
	                                            len + 1,
	                                            pdMS_TO_TICKS(1000));
	
	        if (ret != ESP_OK)
	        {
	            printf("Patch load error: %d\n", ret);
	            return;
	        }
	
	        patch_data += len;
	        size -= len;
	
	        vTaskDelay(pdMS_TO_TICKS(1));  // маленькая пауза между строками
	    }
	
	    // --- завершение патча ---
	    /*
	    cmd[0] = 0x1C; cmd[1] = 0x00; cmd[2] = 0x00;
	    i2c_master_transmit(dev_handle, cmd, 3, pdMS_TO_TICKS(100));
	    */
	    vTaskDelay(pdMS_TO_TICKS(100));
	
	    printf("First patch load done\n");

	}
	
	
void tuner_second_patch_load_TEF(i2c_master_dev_handle_t dev_handle, const uint8_t *patch_data, uint32_t size)
	{
	    uint8_t cmd[3];
		
	    // --- начальная инициализация патча ---
	    cmd[0] = 0x1C; cmd[1] = 0x00; cmd[2] = 0x00;
	    i2c_master_transmit(dev_handle, cmd, 3, pdMS_TO_TICKS(100));
	    vTaskDelay(pdMS_TO_TICKS(100)); // пауза после команды
	
	    cmd[0] = 0x1C; cmd[1] = 0x00; cmd[2] = 0x75;
	    i2c_master_transmit(dev_handle, cmd, 3, pdMS_TO_TICKS(100));
	    vTaskDelay(pdMS_TO_TICKS(100)); // пауза после команды
	
	    // --- загрузка патча ---
	    const uint16_t LINE_SIZE = 24;   // размер строки данных (как в доке)
	    uint8_t buf[LINE_SIZE + 1];      // +1 байт под 0x1B
	    buf[0] = 0x1B;
	
	    printf("Second patch load start, size: %lu bytes\n", (unsigned long)size);
	
	    while (size > 0)
	    {
	        uint16_t len = (size > LINE_SIZE) ? LINE_SIZE : size;
	
	        // копируем одну строку
	        for (uint16_t i = 0; i < len; i++)
	        {
	            buf[1 + i] = patch_data[i];
	        }
	
	        printf("Send line (%d bytes): ", len);
	        for (int i = 0; i < len + 1; i++)
	        {
	            printf("%02X ", buf[i]);
	        }
	        printf("\n");
	
	        esp_err_t ret = i2c_master_transmit(dev_handle,
	                                            buf,
	                                            len + 1,
	                                            pdMS_TO_TICKS(1000));
	
	        if (ret != ESP_OK)
	        {
	            printf("Patch load error: %d\n", ret);
	            return;
	        }
	
	        patch_data += len;
	        size -= len;
	
	        vTaskDelay(pdMS_TO_TICKS(1));  // маленькая пауза между строками
	    }
	
	    // --- завершение патча ---
	    cmd[0] = 0x1C; cmd[1] = 0x00; cmd[2] = 0x00;
	    i2c_master_transmit(dev_handle, cmd, 3, pdMS_TO_TICKS(100));
	    vTaskDelay(pdMS_TO_TICKS(100));
	
	    printf("Second patch load done\n");
	}	

void get_operation_status_TEF(i2c_master_dev_handle_t dev_handle)
	{
		uint8_t get_operation_status_cmd[3] = {0x40, 0x80, 0x01};
		uint8_t response[2];
		bool is_boot_state_received = 0;
		
		while (is_boot_state_received == 0)
		{
		    esp_err_t ret = i2c_master_transmit_receive(
		        dev_handle,
		        get_operation_status_cmd, sizeof(get_operation_status_cmd),
		        response, sizeof(response),
		        pdMS_TO_TICKS(100)
		    );
		
		    if (ret == ESP_OK)
		    {
		        if ((response[1] == 0x00) || (response[1] == 0x01) || (response[1] == 0x02) || (response[1] == 0x03))
		        {
		            printf("Get_Operation_Status: %02X %02X \n",
		            response[0], response[1]);
		            is_boot_state_received = 1;
		        }
		    }
		
		    vTaskDelay(pdMS_TO_TICKS(10));
		}
	}
	
void start_TEF(i2c_master_dev_handle_t dev_handle)
	{
		esp_err_t response;
		uint8_t start_cmd[3] = {0x14, 0x00, 0x01};
	
		response = i2c_master_transmit(
			dev_handle, 
			start_cmd, 
			sizeof(start_cmd), 
			pdMS_TO_TICKS(1000));
			
		if (response != ESP_OK) {
		    printf("Start command failed: %d\n", response);
		    return;
		}
		
		printf("Start command was sent\n");
		
		// TEF любит небольшую задержку
		vTaskDelay(pdMS_TO_TICKS(100));
	}
	
void get_id_TEF(i2c_master_dev_handle_t dev_handle)
	{
		uint8_t get_identification_cmd[3] = {0x40, 0x82, 0x01};
		uint8_t identification_resp[6];
	
	    esp_err_t ident_return = i2c_master_transmit_receive(
	        dev_handle,
	        get_identification_cmd, sizeof(get_identification_cmd),
	        identification_resp, sizeof(identification_resp),
	        pdMS_TO_TICKS(100)
	    );
	
	    if (ident_return == ESP_OK) {
        	printf("Get_Identification: %02X %02X %02X %02X %02X %02X \n",
            identification_resp[0], identification_resp[1],
            identification_resp[2], identification_resp[3],
            identification_resp[4], identification_resp[5]);
    	}
	}
	
void APPL_activate_TEF(i2c_master_dev_handle_t dev_handle)
	{
		esp_err_t response;
		uint8_t get_APPL_Activate_cmd[5] = {0x40, 0x05, 0x01, 0x00, 0x01};
	
		response = i2c_master_transmit(dev_handle, get_APPL_Activate_cmd, sizeof(get_APPL_Activate_cmd), pdMS_TO_TICKS(1000));
		if (response != ESP_OK) {
		    printf("APPL_Activate_cmd command failed: %d\n", response);
		    return;
		}
		printf("APPL_Activate_cmd was sent\n");
	}
	
void turn_on_FM_tune_to_TEF(int16_t first_freq)
	{
		esp_err_t response;
		
		uint8_t freq_hi_byte = first_freq >> 8;    // старший байт
		uint8_t freq_lo_byte = first_freq & 0xFF;  // младший байт
		
		uint8_t FM_Tune_To_cmd[7] = {0x20, 0x01, 0x01, 0x00, 0x01, freq_hi_byte, freq_lo_byte};
	
		response = i2c_master_transmit(tef_handle_internal, FM_Tune_To_cmd, sizeof(FM_Tune_To_cmd), pdMS_TO_TICKS(1000));
		if (response != ESP_OK) {
		    printf("FM_Tune_To_cmd command failed: %d\n", response);
		    return;
		}
	
		// printf("FM_Tune_To_cmd was sent\n");
	}
	
void FM_tune_to_TEF(i2c_master_dev_handle_t dev_handle, int16_t freq_to_tune)
	{
		esp_err_t response;
		
		uint8_t freq_hi_byte = freq_to_tune >> 8;    // старший байт
		uint8_t freq_lo_byte = freq_to_tune & 0xFF;  // младший байт
		
		uint8_t FM_Tune_To_cmd[7] = {0x20, 0x01, 0x01, 0x00, 0x01, freq_hi_byte, freq_lo_byte};
	
		response = i2c_master_transmit(dev_handle, FM_Tune_To_cmd, sizeof(FM_Tune_To_cmd), pdMS_TO_TICKS(1000));
		if (response != ESP_OK) {
		    printf("FM_Tune_To_cmd command failed: %d\n", response);
		    return;
		}
	}
	
void audio_set_mute_TEF()
	{
		esp_err_t response;
		
		uint8_t AUDIO_Set_Mute_cmd[5] = {0x30, 0x0B, 0x01, 0x00, 0x00};
	
		response = i2c_master_transmit(tef_handle_internal, AUDIO_Set_Mute_cmd, sizeof(AUDIO_Set_Mute_cmd), pdMS_TO_TICKS(1000));
		if (response != ESP_OK) {
		    printf("AUDIO_Set_Mute_cmd command failed: %d\n", response);
		    return;
		}
	
		printf("Audio_Set_Mute_cmd was send. Radio is working... \n");
	}
	
int16_t single_press_FM_tune_to_TEF(int16_t current_freq, int8_t direction, i2c_master_dev_handle_t dev_handle)
	{
		esp_err_t response;
		
		if (direction == -1) {
			if (current_freq == 8750) {
				current_freq = 10800;
			} else {
			current_freq = current_freq - 5;
			}
		} else if (direction == 1) {
			if (current_freq == 10800) {
				current_freq = 8750;
			} else {
			current_freq = current_freq + 5;
			}
		}
		
		uint8_t freq_hi_byte = current_freq >> 8;    // старший байт
		uint8_t freq_lo_byte = current_freq & 0xFF;  // младший байт
		
		uint8_t FM_Tune_To_cmd[7] = {0x20, 0x01, 0x01, 0x00, 0x01, freq_hi_byte, freq_lo_byte};
	
		response = i2c_master_transmit(dev_handle, FM_Tune_To_cmd, sizeof(FM_Tune_To_cmd), pdMS_TO_TICKS(1000));
		if (response != ESP_OK) {
		    printf("FM_Tune_To_cmd command failed: %d\n", response);
		    return 0;
		} else {
			printf("FM_Tune_To_cmd, current_freq: %d\n", current_freq);
		}
		return current_freq;
	}
	
void TEF_Get_Quality_Data()
		{
		    // Module 32 (0x20), cmd 129 (0x81), index 1
		    uint8_t cmd[3] = {0x20, 0x81, 0x01};
		    uint8_t resp[14];
		
		    esp_err_t ret = i2c_master_transmit_receive(
		        tef_handle_internal,
		        cmd, sizeof(cmd),
		        resp, sizeof(resp),
		        pdMS_TO_TICKS(100)
		    );
		
		    if (ret != ESP_OK) {
		        printf("I2C error: %d\n", ret);
		        return;
		    }
		
		    // --- Разбор 7 слов ---
		    uint16_t status     = (resp[0]  << 8) | resp[1];
		    int16_t  level      = (resp[2]  << 8) | resp[3];
		    uint16_t usn        = (resp[4]  << 8) | resp[5];
		    uint16_t wam        = (resp[6]  << 8) | resp[7];
		    int16_t  offset     = (resp[8]  << 8) | resp[9];
		    uint16_t bandwidth  = (resp[10] << 8) | resp[11];
		    uint16_t modulation = (resp[12] << 8) | resp[13];
		
		    // --- Разбор status ---
		    uint8_t  af_update  = (status >> 15) & 0x01;
		    uint16_t timestamp  = status & 0x03FF;
		
		    printf("=== FM Quality Data ===\n");
		
		    printf("AF_Update: %u\n", af_update);
		    printf("Timestamp: %u\n", timestamp);
		
		    if (timestamp == 0) {
		        printf("Tuning in progress, data not valid yet\n");
		        return;
		    }
		
		    float level_dBuV = level / 10.0f;
		    float level_dBm  = level_dBuV - 107.0f; // преобразуем в дБм для 50Ω
		    printf("Level: %.1f dBuV (%.1f dBm)\n", level_dBuV, level_dBm);
		
		    printf("USN: %.1f %%\n", usn / 10.0f);
		    printf("Multipath (WAM): %.1f %%\n", wam / 10.0f);
		    printf("Offset: %.1f kHz\n", offset / 10.0f);
		    printf("IF Bandwidth: %.1f kHz\n", bandwidth / 10.0f);
		    printf("Modulation: %.1f %%\n", modulation / 10.0f);
		
		    printf("=======================\n\n");
		}

freq_q_info_t TEF_Get_Q_Data_Struct()
		{
		    uint8_t cmd[3] = {0x20, 0x81, 0x01};
		    uint8_t resp[14];
			
			freq_q_info_t out_data;
			
		    esp_err_t ret = i2c_master_transmit_receive(
		        tef_handle_internal,
		        cmd, sizeof(cmd),
		        resp, sizeof(resp),
		        pdMS_TO_TICKS(100)
		    );
		    
		    if (ret != ESP_OK) {
			    ESP_LOGE("TEF", "I2C error: %s", esp_err_to_name(ret));
			}
		    
		    //uint16_t status     = (resp[0]  << 8) | resp[1];
		    int16_t  level      = (resp[2]  << 8) | resp[3];
		    uint16_t usn        = (resp[4]  << 8) | resp[5];
		    uint16_t wam        = (resp[6]  << 8) | resp[7];
		    int16_t  offset     = (resp[8]  << 8) | resp[9];
		    uint16_t bandwidth  = (resp[10] << 8) | resp[11];
		    uint16_t modulation = (resp[12] << 8) | resp[13];
		
			out_data.level = level / 10.0f;
			out_data.usn = usn / 10.0f;
			out_data.wam = wam / 10.0f;
			out_data.offset = offset / 10.0f;
			out_data.bandwidth = bandwidth / 10.0f;
			out_data.modulation = modulation / 10.0f;
			
			return out_data;
		}
		
void set_bandwidth_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id)
	{
		static uint8_t current_bw_index = 7;
		const uint16_t fm_bandwidths[] = {56, 64, 72, 84, 97, 114, 133, 151, 168, 184, 200, 217, 236, 254, 287, 311};
		
		if ((button_id == 1) && (current_bw_index != 0)){
			current_bw_index = current_bw_index - 1;
		//	printf("IF bandwidth: %d\n", fm_bandwidths[current_bw_index]);
			
		}
		
		if ((button_id == 2) && (current_bw_index != 15)){
			current_bw_index = current_bw_index + 1;
		//	printf("IF bandwidth: %d\n", fm_bandwidths[current_bw_index]);
		}
		
		uint16_t bandwidth = fm_bandwidths[current_bw_index] * 10;
		
		uint8_t hi_byte = (bandwidth >> 8) & 0xFF; // старший байт
		uint8_t lo_byte = bandwidth & 0xFF;        // младший байт
		
		//uint8_t cmd[11] = {0x20, 0x0A, 0x01, 0x00, 0x00, hi_byte, lo_byte, 0x03, 0xE8, 0x03, 0xE8}; // 4-й байт - mode
		uint8_t cmd[] = {0x20, 0x0A, 0x01, 0x00, 0x00, hi_byte, lo_byte, 0x03, 0xE8, 0x03, 0xE8}; // 4-й байт - mode
		
		/*
		printf("I2C cmd: ");
		for (int i = 0; i < sizeof(cmd); i++) {
		    printf("%02X ", cmd[i]);
		}
		printf("\n");
		*/
		
		i2c_master_transmit(dev_handle, cmd, sizeof(cmd), pdMS_TO_TICKS(1000));
		
	}
	
void test_set_bandwidth_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id)
	{
		static uint8_t current_test_index = 7;
		const uint16_t fm_tests_array[] = {500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500};
		
		if ((button_id == 1) && (current_test_index != 0)){
			current_test_index = current_test_index - 1;
		printf("IF bandwidth: %d\n", fm_tests_array[current_test_index]);
			
		}
		
		if ((button_id == 2) && (current_test_index != 10)){
			current_test_index = current_test_index + 1;
		printf("IF bandwidth: %d\n", fm_tests_array[current_test_index]);
		}
		
		//uint16_t test_parameter = fm_tests_array[current_test_index];
		
		//uint8_t hi_byte = (test_parameter >> 8) & 0xFF; // старший байт
		//uint8_t lo_byte = test_parameter & 0xFF;        // младший байт
		
		//uint8_t cmd[11] = {0x20, 0x0A, 0x01, 0x00, 0x00, hi_byte, lo_byte, 0x03, 0xE8, 0x03, 0xE8}; // 4-й байт - mode
		// uint8_t cmd[] = {0x20, 0x0A, 0x01, 0x00, 0x01, 0x09, 0x38, 0x03, 0xE8,hi_byte, lo_byte}; // 4-й байт - mode
		
		//Тестируем, работает ли установка nominal_bandwidth в разных версиях прошивки

		
		uint8_t cmd[] = {
		    0x20,       // module 32 - FM
		    0x0A,       // cmd 10 - Set_Bandwidth
		    0x01,       // тут вроде всегда 1
		    0x00,0x01,  // mode = auto
		    0x09, 0x38, // bandwidth = 2360 → 236 kHz
		    0x03, 0xE8, // control_sensitivity = 1000 → 100%
		    0x03, 0xE8, // low_level_sensitivity = 1000 → 100%
		    0x02, 0x30, // min_bandwidth = 
		    0x05, 0xE6, // nominal_bandwidth = 2360 → 236 kHz
		    0x01, 0x2C  // control_attack = 300 µs
		};
		
		printf("I2C cmd: ");
		for (int i = 0; i < sizeof(cmd); i++) {
		    printf("%02X ", cmd[i]);
		}
		printf("\n");
		
		
		i2c_master_transmit(dev_handle, cmd, sizeof(cmd), pdMS_TO_TICKS(1000));
	
	}
	
void  Set_RFAGC_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id)
	{
		static uint8_t current_test_index = 0;
		const uint16_t fm_tests_array[] = {840, 850, 860, 870, 880, 890, 900,910, 920};
		size_t array_last_index = (sizeof(fm_tests_array) / sizeof(fm_tests_array[0])) - 1;
		
		if ((button_id == 1) && (current_test_index != 0)){
			current_test_index = current_test_index - 1;
		printf("RFAGC: %d\n", fm_tests_array[current_test_index]);
			
		}
		
		if ((button_id == 2) && (current_test_index != array_last_index)){
			current_test_index = current_test_index + 1;
		printf("RFAGC: %d\n", fm_tests_array[current_test_index]);
		}
		
		uint16_t test_parameter = fm_tests_array[current_test_index];
		
		uint8_t hi_byte = (test_parameter >> 8) & 0xFF; // старший байт
		uint8_t lo_byte = test_parameter & 0xFF;        // младший байт
		
		uint8_t cmd[] = {0x20, 0x0B, 0x01, hi_byte, lo_byte, 0x00, 0x00}; 
		
		printf("RFAGC cmd: ");
		for (int i = 0; i < sizeof(cmd); i++) {
		    printf("%02X ", cmd[i]);
		}
		printf("\n");
		
		i2c_master_transmit(dev_handle, cmd, sizeof(cmd), pdMS_TO_TICKS(1000));
	}

void set_MphSuppression_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id)
	{
		static uint8_t current_mode = 0;
		uint8_t cmd[5] = {0};
		
		if ((button_id == 1) && (current_mode != 0)){
			current_mode = 0;
			cmd[0]=0x20; cmd[1]=0x14; cmd[2]=0x01; cmd[3]=0x00; cmd[4]=0x00;
			i2c_master_transmit(dev_handle, cmd, sizeof(cmd), pdMS_TO_TICKS(1000));
			printf("MphSuppression: %d\n", current_mode);
			
		}
		
		if ((button_id == 2) && (current_mode != 1)){
			current_mode = 1;
			cmd[0]=0x20; cmd[1]=0x14; cmd[2]=0x01; cmd[3]=0x00; cmd[4]=0x01;
			i2c_master_transmit(dev_handle, cmd, sizeof(cmd), pdMS_TO_TICKS(1000));
			printf("MphSuppression: %d\n", current_mode);
		}
		
		printf("MphSuppression cmd: ");
		for (int i = 0; i < sizeof(cmd); i++) {
		    printf("%02X ", cmd[i]);
		}
		printf("\n");
		
	}
	
void set_channelEqualizer_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id)
	{
		static uint8_t current_mode = 0;
		uint8_t cmd[5] = {0};
		
		if ((button_id == 1) && (current_mode != 0)){
			current_mode = 0;
			cmd[0]=0x20; cmd[1]=0x16; cmd[2]=0x01; cmd[3]=0x00; cmd[4]=0x00;
			i2c_master_transmit(dev_handle, cmd, sizeof(cmd), pdMS_TO_TICKS(1000));
			printf("ChannelEqualizer: %d\n", current_mode);
			
		}
		
		if ((button_id == 2) && (current_mode != 1)){
			current_mode = 1;
			cmd[0]=0x20; cmd[1]=0x16; cmd[2]=0x01; cmd[3]=0x00; cmd[4]=0x01;
			i2c_master_transmit(dev_handle, cmd, sizeof(cmd), pdMS_TO_TICKS(1000));
			printf("ChannelEqualizer: %d\n", current_mode);
		}
		
		printf("ChannelEqualizer cmd: ");
		for (int i = 0; i < sizeof(cmd); i++) {
		    printf("%02X ", cmd[i]);
		}
		printf("\n");
		
	}
	
void set_LevelStep_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id)
	{
	    static uint8_t current_test_index = 0;
	
		/*
	    uint8_t cmd_default[] = {0x20, 0x26, 0x01,
	                             0xFF, 0xEC,
	                             0xFF, 0xE2,
	                             0xFF, 0xD8,
	                             0xFF, 0xCE,
	                             0xFF, 0xC4,
	                             0xFF, 0xC4,
	                             0xFF, 0xC4};
		*/
		
	    uint8_t cmd_default[] = {0x20, 0x26, 0x01,
	                             0x00, 0x00,
	                             0x00, 0x00,
	                             0x00, 0x00,
	                             0x00, 0x00,
	                             0x00, 0x00,
	                             0x00, 0x00,
	                             0x00, 0x00};
		
		
		uint8_t cmd_testing[] = {0x20, 0x26, 0x01,
                         0xFF, 0xC4,   // -60 = -6 dB
                         0xFF, 0xC4,   // -60 = -6 dB
                         0xFF, 0xC4,   // -60 = -6 dB
                         0xFF, 0xC4,   // -60 = -6 dB
                         0xFF, 0xC4,   // -60 = -6 dB
                         0xFF, 0xC4,   // -60 = -6 dB
                         0xFF, 0xC4};  // -60 = -6 dB	
	
	
	    if ((button_id == 2) && (current_test_index != 0)) {
	        current_test_index = 0;
	        i2c_master_transmit(dev_handle, cmd_default, sizeof(cmd_default), pdMS_TO_TICKS(1000));
	        printf("LevelStep is default: %d\n", current_test_index);
	    }
	
	    if ((button_id == 1) && (current_test_index != 1)) {
	        current_test_index = 1;
	        i2c_master_transmit(dev_handle, cmd_testing, sizeof(cmd_testing), pdMS_TO_TICKS(1000));
	        printf("LevelStep is testing: %d\n", current_test_index);
	    }
	
	    uint8_t *cmd = (current_test_index == 0) ? cmd_default : cmd_testing;
	    size_t cmd_size = (current_test_index == 0) ? sizeof(cmd_default) : sizeof(cmd_testing);
	
	    printf("LevelStep cmd: ");
	    for (int i = 0; i < cmd_size; i++) {
	        printf("%02X ", cmd[i]);
	    }
	    printf("\n");
	}
	
void set_LevelOffset_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id)
	{
		static uint8_t current_test_index = 7;
		const int16_t tests_array[] = {-480, -410, -340, -270, -200, -130, -70, 0, 80, 150};
		size_t array_last_index = (sizeof(tests_array) / sizeof(tests_array[0])) - 1;
		
		if ((button_id == 2) && (current_test_index != 0)){
			current_test_index = current_test_index - 1;
		printf("LevelOffset: %d\n", tests_array[current_test_index]);
			
		}
		
		if ((button_id == 1) && (current_test_index != array_last_index)){
			current_test_index = current_test_index + 1;
		printf("LevelOffset: %d\n", tests_array[current_test_index]);
		}
		
		uint16_t test_parameter = tests_array[current_test_index];
		
		uint8_t hi_byte = (test_parameter >> 8) & 0xFF; // старший байт
		uint8_t lo_byte = test_parameter & 0xFF;        // младший байт
		
		uint8_t cmd[] = {0x20, 0x27, 0x01, hi_byte, lo_byte}; 
		
		printf("LevelOffset cmd: ");
		for (int i = 0; i < sizeof(cmd); i++) {
		    printf("%02X ", cmd[i]);
		}
		printf("\n");
		
		i2c_master_transmit(dev_handle, cmd, sizeof(cmd), pdMS_TO_TICKS(1000));
	}
	
void set_Bandwidth_Options_TEF(i2c_master_dev_handle_t dev_handle, int8_t button_id)
	{
		static uint8_t current_test_index = 3;
		const int16_t tests_array[] = {660, 750, 840, 950, 1020, 1110, 1200, 1280, 1330};
		size_t array_last_index = (sizeof(tests_array) / sizeof(tests_array[0])) - 1;
		
		if ((button_id == 2) && (current_test_index != 0)){
			current_test_index = current_test_index - 1;
		printf("Bandwidth_Options: %d\n", tests_array[current_test_index]);
			
		}
		
		if ((button_id == 1) && (current_test_index != array_last_index)){
			current_test_index = current_test_index + 1;
		printf("Bandwidth_Options: %d\n", tests_array[current_test_index]);
		}
		
		uint16_t test_parameter = tests_array[current_test_index];
		
		uint8_t hi_byte = (test_parameter >> 8) & 0xFF; // старший байт
		uint8_t lo_byte = test_parameter & 0xFF;        // младший байт
		
		uint8_t cmd[] = {0x20, 0x56, 0x01, hi_byte, lo_byte}; 
		
		printf("Bandwidth_Options cmd: ");
		for (int i = 0; i < sizeof(cmd); i++) {
		    printf("%02X ", cmd[i]);
		}
		printf("\n");
		
		i2c_master_transmit(dev_handle, cmd, sizeof(cmd), pdMS_TO_TICKS(1000));
	}
	
agc_data_t get_AGC_TEF()
	{
		agc_data_t result = {0, 0};
		uint8_t get_agc_cmd[3] = {0x20, 0x84, 0x01};
		uint8_t get_agc_resp[4];
		
		vTaskDelay(pdMS_TO_TICKS(1000));
		
	    esp_err_t ident_return = i2c_master_transmit_receive(
	        tef_handle_internal,
	        get_agc_cmd, sizeof(get_agc_cmd),
	        get_agc_resp, sizeof(get_agc_resp),
	        pdMS_TO_TICKS(100)
	    );
		
		if (ident_return == ESP_OK) {
	        result.input_att    = ((get_agc_resp[0] << 8) | get_agc_resp[1]) / 10.0f;
	        result.feedback_att = ((get_agc_resp[2] << 8) | get_agc_resp[3]) / 10.0f;
	        
	        //printf("input_att: %d\n", result.input_att);
			//printf("feedback_att: %d\n", result.feedback_att);
	    }
	    
	    ESP_LOGI("AGC", "AGC level: %d", result.input_att);
	    
	    return result;
	}
	
processing_status_data_t get_Processing_Status_TEF()
	{
		processing_status_data_t result = {0, 0, 0, 0};
		uint8_t get_processing_status_cmd[3] = {0x20, 0x86, 0x01};
		uint8_t get_processing_status_resp[8];
	
	    esp_err_t ident_return = i2c_master_transmit_receive(
	        tef_handle_internal,
	        get_processing_status_cmd, sizeof(get_processing_status_cmd),
	        get_processing_status_resp, sizeof(get_processing_status_resp),
	        pdMS_TO_TICKS(100)
	    );
		
		if (ident_return == ESP_OK) {
	        result.softmute_status    = ((get_processing_status_resp[0] << 8) | get_processing_status_resp[1]) / 10.0f;
	        result.highcut_status = ((get_processing_status_resp[2] << 8) | get_processing_status_resp[3]) / 10.0f;
	        result.stereo_status = ((get_processing_status_resp[4] << 8) | get_processing_status_resp[5]) / 10.0f;
	        result.sthiblend_status = ((get_processing_status_resp[6] << 8) | get_processing_status_resp[7]) / 10.0f;
	        
	        //printf("input_att: %d\n", result.input_att);
			//printf("feedback_att: %d\n", result.feedback_att);
	    }
	    return result;
	}	
	
RDS_data_t get_RDS_status_TEF()
	{
		RDS_data_t result = {0};
		uint8_t get_RDS_data_cmd[3] = {0x20, 0x83, 0x01};
		uint8_t get_RDS_data_resp[12];
		
		esp_err_t rds_status_return = i2c_master_transmit_receive(
	        tef_handle_internal,
	        get_RDS_data_cmd, sizeof(get_RDS_data_cmd),
	        get_RDS_data_resp, sizeof(get_RDS_data_resp),
	        pdMS_TO_TICKS(100)
	    );
		
		if (rds_status_return == ESP_OK) {
		    uint16_t status = (get_RDS_data_resp[0] << 8) | get_RDS_data_resp[1];
		    uint16_t a_block = (get_RDS_data_resp[2] << 8) | get_RDS_data_resp[3];
		    uint16_t b_block = (get_RDS_data_resp[4] << 8) | get_RDS_data_resp[5];
		    uint16_t c_block = (get_RDS_data_resp[6] << 8) | get_RDS_data_resp[7];
		    uint16_t d_block = (get_RDS_data_resp[8] << 8) | get_RDS_data_resp[9];
			uint16_t dec_error = (get_RDS_data_resp[10] << 8) | get_RDS_data_resp[11];
			
			result.data_available = (status >> 15) & 1;
		    result.data_lost      = (status >> 14) & 1;
		    result.data_type	  = (status >> 13) & 1;
		    result.group_version = (status >> 12) & 1;   
			result.group_number  = (b_block >> 12) & 0x0F;
		    result.sync           = (status >> 9)  & 1;
		    result.a_block        = a_block;
		    result.b_block		  = b_block;
		    result.c_block		  = c_block;
		    result.d_block		  = d_block;
		    result.err_a = (dec_error >> 14) & 3;
			result.err_b = (dec_error >> 12) & 3;
			result.err_c = (dec_error >> 10) & 3;
			result.err_d = (dec_error >> 8)  & 3;
			
			/*
			ESP_LOGI("RDS", "data_available: %d", result.data_available);
			ESP_LOGI("RDS", "data_lost:      %d", result.data_lost);
			ESP_LOGI("RDS", "data_type:      %d", result.data_type);
			ESP_LOGI("RDS", "group_version:  %d", result.group_version);
			ESP_LOGI("RDS", "group_number:   %d", result.group_number);
			ESP_LOGI("RDS", "sync:           %d", result.sync);
			ESP_LOGI("RDS", "a_block:        0x%04X", result.a_block);
			ESP_LOGI("RDS", "b_block:        0x%04X", result.b_block);
			ESP_LOGI("RDS", "c_block:        0x%04X", result.c_block);
			ESP_LOGI("RDS", "d_block:        0x%04X", result.d_block);
			ESP_LOGI("RDS", "err_a: %d  err_b: %d  err_c: %d  err_d: %d", result.err_a, result.err_b, result.err_c, result.err_d);
			*/
		}
		
		return result;
	}	
	
void init_TEF (i2c_master_dev_handle_t dev_handle)
	{
		
		tef_handle_internal = dev_handle;
		// Получаем текущий статус приемника
		get_operation_status_TEF(dev_handle);

		// Загружаем патчи из файла V102_p209.h
		tuner_first_patch_load_TEF(dev_handle, pPatchBytes102, PatchSize102);
		tuner_second_patch_load_TEF(dev_handle, pLutBytes102, LutSize102);

		//get_operation_status_TEF(dev_handle);
		vTaskDelay(pdMS_TO_TICKS(50));
		
		// Стартуем ТЕФ
		start_TEF(dev_handle);
		
		// Снова получаем статус приёмника
		get_operation_status_TEF(dev_handle);
		
		// Читаем идентификатор чипа
		get_id_TEF(dev_handle);
		
		// Переводим чип в активный режим
		APPL_activate_TEF(dev_handle);
		
		// Снова получаем статус приёмника
		get_operation_status_TEF(dev_handle);
		
		// Настраиваемся на частоту
		turn_on_FM_tune_to_TEF(10000);
		
		// Снова получаем статус приёмника
		//get_operation_status_TEF(dev_handle);
		
		// Вырубаем mute
		//audio_set_mute_TEF(dev_handle);
	}