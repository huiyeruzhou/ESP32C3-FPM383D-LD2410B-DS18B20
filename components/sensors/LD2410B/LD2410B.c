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
    /*接收一帧的数据*/
    int len = serial_receive(23, PS_ReceiveBuffer, 5000);
    ESP_LOGI(TAG, "Received %d", len);
    if(len < 23) {
        *distance = 0;
        return 0xFF;
    }
    /*查找帧头*/
    // int head = -1;
    // for (int i = 0; i < len; i++) {
    //     if (PS_ReceiveBuffer[i] == 0xF4 && PS_ReceiveBuffer[i + 1] == 0xF3 && PS_ReceiveBuffer[i + 2] == 0xF2 && PS_ReceiveBuffer[i + 3] == 0xF1) {
    //         head = i;
    //         printf("帧头位于第%d个字节\n", i);
    //         break;
    //     }
    // }
    // /*没有找到帧头,报错*/
    // if (head == -1) {
    //     *distance = 0;
    //     return 0xFF;
    // }
    /*打印帧数据*/
    int head = 0;
    for (int i = head; i < 23 + head; i++) {
        printf("%02X ", PS_ReceiveBuffer[i]);
    }
    printf("\n");
    /*提取距离值*/
    *distance = (PS_ReceiveBuffer[head + 16] << 8) | PS_ReceiveBuffer[head + 15];
    /*返回运动状态, 1代表静物,2代表动物,3代表既有静物又有动物*/
    return PS_ReceiveBuffer[head + 8];
}
//FD FC FB FA 04 00 FF 00 01 00 04 03 02 01:命令
//FD FC FB FA 08 00 FF 01 00 00 01 00 40 00 04 03 02 01 :回复
uint8_t startConfigure() {
    uint8_t data[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xFF, 0x00, 0x01, 0x00, 0x04, 0x03, 0x02, 0x01};
    ESP_LOGI(TAG, "Start configure");
    //判断是否成功
    serial_fulsh();
    serial_send(size(data), data);
    int len = serial_receive(18, PS_ReceiveBuffer, 5000);
    uint8_t ack[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x08, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x01, 0x00, 0x40, 0x00, 0x04, 0x03, 0x02, 0x01};
    for (int i = 0; i < len; i++) {
        //找到帧头
        if (PS_ReceiveBuffer[i] == ack[0]) {
            int j = 1;
            //判断是否完全相等
            for (; j < size(ack) && i + j < len; j++) {
                if (PS_ReceiveBuffer[i + j] != ack[j]) {
                    break;
                }
            }
            if (j == size(ack)) {
                ESP_LOGI(TAG, "Start configure Success");
                return 0;
            }
        }
    }
    //打印错误信息
    ESP_LOGE(TAG, "Start configure Failed\n");
    for (int i = 0; i < len; i++) {
        printf("%02X ", PS_ReceiveBuffer[i]);
    }
    printf("\n");
    return 1;
}
//FD FC FB FA 02 00 FE 00 04 03 02 01
//FD FC FB FA 04 00 FE 01 00 00 04 03 02 01
uint8_t stopConfigure() {
    uint8_t data[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xFE, 0x00, 0x04, 0x03, 0x02, 0x01};
    ESP_LOGI(TAG, "End configure");
    //判断是否成功
    serial_fulsh();
    serial_send(size(data), data);
    int len = serial_receive(14, PS_ReceiveBuffer, 5000);
    uint8_t ack[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xFE, 0x01, 0x00, 0x00, 0x04, 0x03, 0x02, 0x01};
    for (int i = 0; i < len; i++) {
        //找到帧头
        if (PS_ReceiveBuffer[i] == ack[0]) {
            int j = 1;
            //判断是否完全相等
            for (; j < size(ack) && i + j < len; j++) {
                if (PS_ReceiveBuffer[i + j] != ack[j]) {
                    break;
                }
            }
            if (j == size(ack)) {
                ESP_LOGI(TAG, "End configure success\n");
                return 0;
            }
        }
    }
    //打印错误信息
    ESP_LOGE(TAG, "End configure failed");
    for (int i = 0; i < len; i++) {
        printf("%02X ", PS_ReceiveBuffer[i]);
    }
    printf("\n");
    return 1;
}
//40 -> 28 00
//FD FC FB FA 14 00 64 00 00 00 FF FF 00 00 01 00 /*28 00 00 00*/ 02 00 /*28 00 00 00*/ 04 03 02 01
//FD FC FB FA 04 00 64 01 00 00 04 03 02 01
uint8_t configSensity(uint32_t sensity) {
    uint8_t data[] = { 0xFD, 0xFC, 0xFB, 0xFA,
    0x14, 0x00, 0x64, 0x00,
    0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
    0x01, 0x00, sensity & 0xFF, (sensity >> 8) & 0xFF, (sensity >> 16) & 0xFF, (sensity >> 24) & 0xFF,
    0x02, 0x00, sensity & 0xFF, (sensity >> 8) & 0xFF, (sensity >> 16) & 0xFF, (sensity >> 24) & 0xFF,
    0x04, 0x03, 0x02, 0x01 };
    ESP_LOGI(TAG, "Config sensity");
    serial_fulsh();
    serial_send(size(data), data);

    //判断是否成功
    int len = serial_receive(14, PS_ReceiveBuffer, 5000);
    uint8_t ack[] = { 0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0x64, 0x01, 0x00, 0x00, 0x04, 0x03, 0x02, 0x01 };
    for (int i = 0; i < len; i++) {
        //找到帧头
        if (PS_ReceiveBuffer[i] == ack[0]) {
            int j = 1;
            //判断是否完全相等
            for (; j < size(ack) && i + j < len; j++) {
                if (PS_ReceiveBuffer[i + j] != ack[j]) {
                    break;
                }
            }
            if (j == size(ack)) {
                ESP_LOGI(TAG, "Config sensity success: %" PRId32 , sensity);
                return 0;
            }
        }
    }
    //打印错误信息
    ESP_LOGE(TAG, "Config sensity failed");
    for (int i = 0; i < len; i++) {
        printf("%02X ", PS_ReceiveBuffer[i]);
    }
    printf("\n");
    return 1;
}