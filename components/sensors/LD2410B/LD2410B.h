/*
 * Copyright 2023 YuHongli
 *
 * File: LD2410B.h
 * Description: the LD2410B header
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

uint8_t getDistance(uint16_t *distance);
uint8_t startConfigure();
uint8_t stopConfigure();
uint8_t configSensity(uint32_t sensity);

#ifdef __cplusplus
}
#endif
