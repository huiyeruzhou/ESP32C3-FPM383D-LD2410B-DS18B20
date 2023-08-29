/*
 * Copyright (C), 2022-2023, Soochow University & OPPO Mobile Comm Corp., Ltd.
 *
 * File: DS18B20.c
 * Description: DS18B20 driver
 * Version: V1.0.0
 * Date: 2023/08/23
 * Author: Soochow University
 * Revision History:
 *   Version       Date          Author         Revision Description
 *  V1.0.0        2023/08/23    Soochow University       Create and initialize
 */
#include "DS18B20.h"

__attribute__((unused)) static const char *TAG = "DS18B20";
// receive buffer
uint8_t PS_ReceiveBuffer[16];
// set hand mode
uint8_t HandBuffer[] = {'H', 'a', 'n', 'd', '\r', '\n'};
// read temperature
uint8_t ReadBuffer[] = {'R', 'e', 'a', 'd', '\r', '\n'};
// return value of read success
uint8_t OkBuffer[] = {'O', 'K', '\r', '\n'};

#define size(a) (sizeof(a) / sizeof(a[0]))

/**
 * @brief set hand mode
 * @return 0x00: success. 0xFF: failed
 */
uint8_t set_hand(void)
{
    ESP_LOGI(TAG, "Setting to hand mode");
    serial_send(size(HandBuffer), HandBuffer);
    serial_receive(size(OkBuffer), PS_ReceiveBuffer, 1000);
    for (int i = 0; i < size(OkBuffer); i++) {
        if (PS_ReceiveBuffer[i] != OkBuffer[i]) {
            ESP_LOGE(TAG, "%s", PS_ReceiveBuffer);
            return 0xFF;
        }
    }
    return 0x00;
}

/**
 * @brief get temperature
 * @return temperature, NaN if failed
 */
float get_temperature()
{
    ESP_LOGI(TAG, "Reading temperature");
    serial_send(size(ReadBuffer), ReadBuffer);
    /* format:
     *  T=+025.0C
     */
    serial_receive(11, PS_ReceiveBuffer, 1000);
    float temperature;
    if (sscanf((char *)(PS_ReceiveBuffer), "T=%fC\r\n", &temperature) != 1) {
        return 0.0 / 0.0;
    }
    return temperature;
}
