# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c "i2c_helpers.c" "display_helper.c" "buttons_helper.c" "radio_mode.c" "mode_helper.c" "service_mode.c" "monitoring_mode.c" "wifi_helper.c" 
    INCLUDE_DIRS "."
    REQUIRES lcd1602 TEF6686 button
    PRIV_REQUIRES esp_wifi esp_http_client esp_event nvs_flash driver mbedtls
)
