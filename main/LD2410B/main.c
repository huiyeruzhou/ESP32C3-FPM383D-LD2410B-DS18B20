/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "serial.h"
#include "LD2410B/LD2410B.h"



static const char *TAG = "UART TEST";
void app_main(void)
{
    serial_init(256000);
    uint16_t distance = 0;
    while (1) {
        int mode = getDistance(&distance);
        ESP_LOGI(TAG, "Mode: %d", mode);
        if(mode == 0xFF) {
            ESP_LOGI(TAG, "Error Mode");
            continue;
        }
        ESP_LOGI(TAG, "Distance: %d", distance);
        delay(1000);
    }

}
