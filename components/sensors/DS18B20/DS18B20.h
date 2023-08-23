/*
 * Copyright 2023 YuHongli
 *
 * File: DS18B20.h
 * Description: DS18B20 driver
 * Version: V1.0.0
 * Date: 2023/08/23
 * Author: YuHongli
 * Revision History:
 *   Version       Date          Author         Revision Description
 *  V1.0.0        2023/08/23    YuHongli       Create and initialize
 */
#pragma once
#include "esp_log.h"
#include "serial.h"
#ifdef __cplusplus
extern "C" {
#endif

uint8_t set_hand(void);

float get_temperature(void);
#ifdef __cplusplus
}
#endif
