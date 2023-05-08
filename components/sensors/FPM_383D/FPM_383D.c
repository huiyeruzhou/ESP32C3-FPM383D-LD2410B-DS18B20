#include "FPM_383D.h"

__attribute__((unused)) static const char *TAG = "FPM_383D";
uint8_t PS_ReceiveBuffer[128];   //串口接收数据的临时缓冲数组
#define size(a) (sizeof(a) / sizeof(a[0]))    //计算数组长度的宏定义
uint8_t PS_SearchMBBuffer[] = { 0xF1, 0x1F, 0xE2, 0x2E, 0xB6, 0x6B, 0xA8, 0x8A, 0x00, 0x07, 0x86, 0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0xDC, };

/**
 * @brief 触发一次指纹匹配,5秒后超时
 * @param id 用于存储匹配到的ID的指针,如果指纹可以识别但不能匹配, 设置为0xFFFF
 * @param score 用于存储匹配到的分数的指针
 * @return 返回识别状态码, 0x00代表成功
*/
uint8_t PS_SearchMB(uint16_t *id, uint16_t *score) {
    ESP_LOGI(TAG, "Searching");
    /*发送同步匹配指纹命令*/
    serial_send(size(PS_SearchMBBuffer), PS_SearchMBBuffer);
    /*接收匹配结果*/
    int recvd = serial_receive(28, PS_ReceiveBuffer, 5000);
    if (recvd == 0) {
        if(id) *id = 0xFFFF;
        if (score) *score = 0;
        return 0xFF;
    }
    for (int i = 0; i < 28; i++)
    {
        printf("%02X ", PS_ReceiveBuffer[i]);
    }
    printf("\n");

    /*出现识别异常,返回异常状态码*/
    if (PS_ReceiveBuffer[20] != 0x00) {
        if (id) *id = 0xFFFF;
        if (score) *score = 0x0000;
        return PS_ReceiveBuffer[20];
    }
    else {
        /*识别成功, 检查是否匹配成功(第22位是否为1,1代表成功)*/
        /*匹配失败*/
        if (PS_ReceiveBuffer[22] != 0x01) {
            if (id) *id = 0xFFFF;
        }
        else {
        /*匹配成功*/
            if (id) *id = (PS_ReceiveBuffer[25] << 8) | PS_ReceiveBuffer[26];
        }
        if (score) *score = (PS_ReceiveBuffer[23] << 8) | PS_ReceiveBuffer[24];
        return 0x00;
    }
}