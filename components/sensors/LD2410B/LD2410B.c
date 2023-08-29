/*
 * Copyright (C), 2022-2023, Soochow University & OPPO Mobile Comm Corp., Ltd.
 *
 * File: LD2410B.c
 * Description: LD2410B driver
 * Version: V1.0.0
 * Date: 2023/08/23
 * Author: Soochow University
 * Revision History:
 *   Version       Date          Author         Revision Description
 *  V1.0.0        2023/08/23    Soochow University       Create and initialize
 */
#include "LD2410B.h"

#include "esp_log.h"
#include "serial.h"

__attribute__((unused)) static const char *TAG = "LD2410B";
uint8_t PS_ReceiveBuffer[128];
#define size(a) (sizeof(a) / sizeof(a[0]))

// F4 F3 F2 F1 0D 00 02 AA /*8header*/02 /*9type*/ 51  00 /*11distance of moving*/
// 00/*12energy of moving*/ 00 00 /*14distance of static*/3B/*15energy of static*/ 00 00/*17distance*/  55
// 00 F8 F7 F6 F5/*23trailer*/
/**
 * @brief get distance abd motion type
 * @param distance to store the distance
 * @return motion type.0x01:static, 0x02:moving. 0x03:both, 0xFF: failed
 */
uint8_t getDistance(uint16_t *distance)
{
    ESP_LOGI(TAG, "Searching");

    serial_fulsh();

    int len = serial_receive(23, PS_ReceiveBuffer, 5000);
    ESP_LOGI(TAG, "Received %d", len);
    if (len < 23) {
        if (distance) {
            *distance = 0;
        }
        return 0xFF;
    }

    int head = -1;
    for (int i = 0; i < len; i++) {
        if ((PS_ReceiveBuffer[i] == 0xF4) && (PS_ReceiveBuffer[i + 1] == 0xF3) && (PS_ReceiveBuffer[i + 2] == 0xF2) && (
            PS_ReceiveBuffer[i + 3] == 0xF1)) {
            head = i;
            printf("帧头位于第%d个字节\n", i);
            break;
        }
    }

    if (head == -1) {
        if (distance) {
            *distance = 0;
        }
        return 0xFF;
    }

    for (int i = head; i < 23 + head; i++) {
        printf("%02X ", PS_ReceiveBuffer[i]);
    }
    printf("\n");

    if (distance != NULL) {
        *distance = (PS_ReceiveBuffer[head + 16] << 8) | PS_ReceiveBuffer[head + 15];
    } else {
        ESP_LOGE(TAG, "distance is NULL");
    }

    return PS_ReceiveBuffer[head + 8];
}
// FD FC FB FA 04 00 FF 00 01 00 04 03 02 01:cmd
// FD FC FB FA 08 00 FF 01 00 00 01 00 40 00 04 03 02 01 :rsp
uint8_t startConfigure()
{
    uint8_t data[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xFF, 0x00, 0x01, 0x00, 0x04, 0x03, 0x02, 0x01};
    ESP_LOGI(TAG, "Start configure");

    serial_fulsh();
    serial_send(size(data), data);
    int len = serial_receive(18, PS_ReceiveBuffer, 5000);
    uint8_t ack[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x08, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x01, 0x00, 0x40, 0x00, 0x04, 0x03, 0x02, 0x01};
    for (int i = 0; i < len; i++) {
        if (PS_ReceiveBuffer[i] == ack[0]) {
            int j = 1;
            for (; j < size(ack) && i + j < len; j++) {
                if (PS_ReceiveBuffer[i + j] != ack[j]) {
                    break;
                }
            }
            if (j == size(ack)) {
                ESP_LOGI(TAG, "Start configure Success");
                return 0;
            }
        }
    }

    ESP_LOGE(TAG, "Start configure Failed\n");
    for (int i = 0; i < len; i++) {
        printf("%02X ", PS_ReceiveBuffer[i]);
    }
    printf("\n");
    return 1;
}
// FD FC FB FA 02 00 FE 00 04 03 02 01
// FD FC FB FA 04 00 FE 01 00 00 04 03 02 01
uint8_t stopConfigure()
{
    uint8_t data[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xFE, 0x00, 0x04, 0x03, 0x02, 0x01};
    ESP_LOGI(TAG, "End configure");

    serial_fulsh();
    serial_send(size(data), data);
    int len = serial_receive(14, PS_ReceiveBuffer, 5000);
    uint8_t ack[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xFE, 0x01, 0x00, 0x00, 0x04, 0x03, 0x02, 0x01};
    for (int i = 0; i < len; i++) {
        if (PS_ReceiveBuffer[i] == ack[0]) {
            int j = 1;

            for (; j < size(ack) && i + j < len; j++) {
                if (PS_ReceiveBuffer[i + j] != ack[j]) {
                    break;
                }
            }
            if (j == size(ack)) {
                ESP_LOGI(TAG, "End configure success\n");
                return 0;
            }
        }
    }

    ESP_LOGE(TAG, "End configure failed");
    for (int i = 0; i < len; i++) {
        printf("%02X ", PS_ReceiveBuffer[i]);
    }
    printf("\n");
    return 1;
}
// 40 -> 28 00
// FD FC FB FA 14 00 64 00 00 00 FF FF 00 00 01 00 /*28 00 00 00*/ 02 00 /*28 00
// 00 00*/ 04 03 02 01 FD FC FB FA 04 00 64 01 00 00 04 03 02 01
uint8_t configSensity(uint32_t sensity)
{
    uint8_t data[] = {0xFD,
                      0xFC,
                      0xFB,
                      0xFA,
                      0x14,
                      0x00,
                      0x64,
                      0x00,
                      0x00,
                      0x00,
                      0xFF,
                      0xFF,
                      0x00,
                      0x00,
                      0x01,
                      0x00,
                      sensity & 0xFF,
                      (sensity >> 8) & 0xFF,
                      (sensity >> 16) & 0xFF,
                      (sensity >> 24) & 0xFF,
                      0x02,
                      0x00,
                      sensity & 0xFF,
                      (sensity >> 8) & 0xFF,
                      (sensity >> 16) & 0xFF,
                      (sensity >> 24) & 0xFF,
                      0x04,
                      0x03,
                      0x02,
                      0x01};
    ESP_LOGI(TAG, "Config sensity");
    serial_fulsh();
    serial_send(size(data), data);

    int len = serial_receive(14, PS_ReceiveBuffer, 5000);
    uint8_t ack[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0x64, 0x01, 0x00, 0x00, 0x04, 0x03, 0x02, 0x01};
    for (int i = 0; i < len; i++) {
        if (PS_ReceiveBuffer[i] == ack[0]) {
            int j = 1;

            for (; j < size(ack) && i + j < len; j++) {
                if (PS_ReceiveBuffer[i + j] != ack[j]) {
                    break;
                }
            }
            if (j == size(ack)) {
                ESP_LOGI(TAG, "Config sensity success: %" PRId32, sensity);
                return 0;
            }
        }
    }

    ESP_LOGE(TAG, "Config sensity failed");
    for (int i = 0; i < len; i++) {
        printf("%02X ", PS_ReceiveBuffer[i]);
    }
    printf("\n");
    return 1;
}
