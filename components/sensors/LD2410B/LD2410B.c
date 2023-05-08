#include "serial.h"
#include "esp_log.h"

__attribute__((unused)) static const char *TAG = "LD2410B";
uint8_t PS_ReceiveBuffer[128];   //串口接收数据的临时缓冲数组
#define size(a) (sizeof(a) / sizeof(a[0]))    //计算数组长度的宏定义


//F4 F3 F2 F1 0D 00 02 AA /*8头部*/02 /*9类型*/ 51  00 /*11运动物体距离*/ 00/*12活力*/ 00 00 /*14静止物体距离*/3B/*15静止活力*/ 00 00/*17探测距离*/  55 00 F8 F7 F6 F5/*23尾部*/
/**
 * @brief 获取运动物体的距离
 * @param distance 用于存储距离值的指针
 * @return 返回运动类型, 0xFF代表失败
 */
uint8_t getDistance(uint16_t *distance) {
    ESP_LOGI(TAG, "Searching");
    /*清空缓冲区*/
    serial_fulsh();
    /*接收相当于两个帧长的数据*/
    int len = serial_receive(46, PS_ReceiveBuffer, 5000);
    ESP_LOGI(TAG, "Received %d", len);
    if(len < 46) {
        *distance = 0;
        return 0xFF;
    }
    /*查找帧头*/
    int head = -1;
    for (int i = 0; i < len; i++) {
        if (PS_ReceiveBuffer[i] == 0xF4 && PS_ReceiveBuffer[i + 1] == 0xF3 && PS_ReceiveBuffer[i + 2] == 0xF2 && PS_ReceiveBuffer[i + 3] == 0xF1) {
            head = i;
            printf("帧头位于第%d个字节\n", i);
            break;
        }
    }
    /*没有找到帧头,报错*/
    if (head == -1) {
        *distance = 0;
        return 0xFF;
    }
    /*打印帧数据*/
    for (int i = head; i < 23 + head; i++) {
        printf("%02X ", PS_ReceiveBuffer[i]);
    }
    printf("\n");
    /*提取距离值*/
    *distance = (PS_ReceiveBuffer[head + 15] << 8) | PS_ReceiveBuffer[head + 16];
    /*返回运动状态, 1代表静物,2代表动物,3代表既有静物又有动物*/
    return PS_ReceiveBuffer[head + 8];
}
