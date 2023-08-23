/*
 * Copyright 2023 YuHongli
 *
 * File: FPM_383D.h
 * Description: FPM_383D driver
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

uint8_t PS_SearchMB(uint16_t *id, uint16_t *score);

#ifdef __cplusplus
}
#endif
