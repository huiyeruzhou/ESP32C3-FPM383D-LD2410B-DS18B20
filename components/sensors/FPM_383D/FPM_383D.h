#pragma once
#include "serial.h"
#include "esp_log.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief 触发一次指纹匹配
 * @param id 用于存储匹配到的ID的指针
 * @param score 用于存储匹配到的分数的指针
 * @return 返回状态码, 0x00代表成功
*/
    uint8_t PS_SearchMB(uint16_t *id, uint16_t *score);
#ifdef __cplusplus
}
#endif