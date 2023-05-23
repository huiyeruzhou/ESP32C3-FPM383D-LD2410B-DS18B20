#include "config_wifi.h"
extern "C"
void app_main(void) {
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ip_set("192.168.1.120", "255.255.255.0", "192.168.1.1");
    wifi_init_sta("yu", "esp32c3.");
}
