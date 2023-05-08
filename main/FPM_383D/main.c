#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "serial.h"
#include "FPM_383D/FPM_383D.h"



static const char *TAG = "UART TEST";
void app_main(void) {
    serial_init(57600);
    uint16_t id=0, score=0;
    while (1) {
        int status = PS_SearchMB(&id, &score);
        if (status)
            ESP_LOGE(TAG, "Failed to recognize finger, status=%d", status);
        else if (id == 0xFFFF)
            ESP_LOGW(TAG, "Macth Failed, score=%d", score);
        else
            ESP_LOGW(TAG, "id=%d, score=%d", id, score);
        delay(2000);
    }
}