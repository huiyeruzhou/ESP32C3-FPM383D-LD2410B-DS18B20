#include "DS18B20.h"

__attribute__((unused)) static const char *TAG = "DS18B20";
uint8_t PS_ReceiveBuffer[16];   //串口接收数据的临时缓冲数组
uint8_t HandBuffer[] = { 'H','a','n','d','\r','\n'}; //设置为手动模式的命令
uint8_t ReadBuffer[] = { 'R','e','a','d','\r','\n'}; //读取一次温度的命令
uint8_t OkBuffer[] = { 'O','K','\r','\n' };     //读取成功的返回值
#define size(a) (sizeof(a) / sizeof(a[0]))    //计算数组长度的宏定义

/**
 * @brief 设置为手动模式
 * @return 返回状态码, 0x00代表成功, 其余表示失败
*/
uint8_t set_hand(void) {
    ESP_LOGI(TAG, "Setting to hand mode");
    /*发送设置手动模式命令*/
    serial_send(size(HandBuffer), HandBuffer);
    /*接收设置结果*/
    serial_receive(size(OkBuffer), PS_ReceiveBuffer, 1000);
    /*检查返回值*/
    for(int i = 0; i < size(OkBuffer); i++) {
        if (PS_ReceiveBuffer[i] != OkBuffer[i]) {

            ESP_LOGE(TAG, "%s", PS_ReceiveBuffer);
            return 0xFF;
        }
    }
    return 0x00;
}

/**
 * @brief 读取一次温度并返回温度值
 * @return 温度值
*/
float get_temperature() {
    ESP_LOGI(TAG, "Reading temperature");
    /*发送读取温度命令*/
    serial_send(size(ReadBuffer), ReadBuffer);
    /*接收温度值
     * 格式: T=+025.0C
    */
    serial_receive(11, PS_ReceiveBuffer, 1000);
    /*检查返回值*/
    float temperature;
    if(sscanf((char *)PS_ReceiveBuffer, "T=%fC\r\n", &temperature) != 1) {
        return 0.0/0.0;
    }
    return temperature;
}