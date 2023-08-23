/*
 * Copyright 2023 YuHongli
 *
 * File: FPM_383D.c
 * Description: FPM_383D driver
 * Version: V1.0.0
 * Date: 2023/08/23
 * Author: YuHongli
 * Revision History:
 *   Version       Date          Author         Revision Description
 *  V1.0.0        2023/08/23    YuHongli       Create and initialize
 */
#include "FPM_383D.h"

__attribute__((unused)) static const char *TAG = "FPM_383D";
uint8_t PS_ReceiveBuffer[128];
#define size(a) (sizeof(a) / sizeof(a[0]))
uint8_t PS_SearchMBBuffer[] = {
    0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A, 0x00, 0x07, 0x86, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0xDC,
};

/**
 * @brief try to match a fingerprint, timeout after 5 seconds
 * @param id to store the matched ID, set to 0xFFFF if the fingerprint can be recognized but not matched
 * @param score to store the matched score, set to 0 if the fingerprint can be recognized but not matched
 * @return 0x00: success. 0xFF: failed. other: error code
 */
uint8_t PS_SearchMB(uint16_t *id, uint16_t *score)
{
    ESP_LOGI(TAG, "Searching");
    serial_send(size(PS_SearchMBBuffer), PS_SearchMBBuffer);
    ESP_LOGI(TAG, "receiving");

    int recvd = serial_receive(28, PS_ReceiveBuffer, 3000);
    if (recvd == 0) {
        ESP_LOGI(TAG, "failed to receive");
        if (id) {
            *id = 0xFFFF;
        }
        if (score) {
            *score = 0;
        }
        return 0xFF;
    }
    for (int i = 0; i < 28; i++) {
        printf("%02X ", PS_ReceiveBuffer[i]);
    }
    printf("\n");

    /*error*/
    if (PS_ReceiveBuffer[20] != 0x00) {
        if (id) {
            *id = 0xFFFF;
        }
        if (score) {
            *score = 0x0000;
        }
        return PS_ReceiveBuffer[20];
    } else {
        /*failed to match*/
        if (PS_ReceiveBuffer[22] != 0x01) {
            if (id) {
                *id = 0xFFFF;
            }
        } else {
            if (id) {
                *id = (PS_ReceiveBuffer[25] << 8) | PS_ReceiveBuffer[26];
            }
        }
        if (score) {
            *score = (PS_ReceiveBuffer[23] << 8) | PS_ReceiveBuffer[24];
        }
        return 0x00;
    }
}
