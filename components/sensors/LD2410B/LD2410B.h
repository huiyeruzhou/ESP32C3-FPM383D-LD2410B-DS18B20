#pragma once
#include "esp_log.h"
#include "serial.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief 获取运动物体的距离
 * @param distance 用于存储距离值的指针
 * @return 返回运动类型, 0xFF代表失败
 */
uint8_t getDistance(uint16_t *distance);
uint8_t startConfigure();
uint8_t stopConfigure();
uint8_t configSensity(uint32_t sensity);
#ifdef __cplusplus
}
#endif
