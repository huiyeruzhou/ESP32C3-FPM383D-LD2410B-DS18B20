#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "serial.h"
#include "DS18B20/DS18B20.h"



static const char *TAG = "UART TEST";
void app_main(void) {
    serial_init(9600);
    float temperature;
    if (set_hand()) {
        ESP_LOGE(TAG, "Failed to set hand mode");
    }
    while (1) {
        temperature = get_temperature();
        ESP_LOGW(TAG, "T=%+06.1fC\n", temperature);
        delay(1000);
    }
}