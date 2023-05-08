#pragma once
#include "serial.h"
#include "esp_log.h"
#ifdef __cplusplus
extern "C" {
#endif
    /**
     * @brief 获取运动物体的距离
     * @param distance 用于存储距离值的指针
     * @return 返回运动类型, 0xFF代表失败
     */
    uint8_t getDistance(uint16_t *distance);
#ifdef __cplusplus
}
#endif