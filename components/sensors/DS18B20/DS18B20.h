#pragma once
#include "serial.h"
#include "esp_log.h"

/**
 * @brief 设置为手动模式
 * @return 返回状态码, 0x00代表成功, 其余表示失败
*/
uint8_t set_hand(void);

/**
 * @brief 读取一次温度并返回温度值
 * @return 温度值,读取错误返回NaN
*/
float get_temperature(void);