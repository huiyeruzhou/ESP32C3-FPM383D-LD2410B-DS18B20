#pragma once
/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */
#include "driver/uart.h"
#include "driver/gpio.h"
#define GPIO_TXD (GPIO_NUM_4)
#define GPIO_RXD (GPIO_NUM_5)
#define GPIO_RTS (UART_PIN_NO_CHANGE)
#define GPIO_CTS (UART_PIN_NO_CHANGE)

#define UART_PORT_NUM (UART_NUM_1)
#define BUF_SIZE (1024)

 //delay函数，单位ms
#define delay(ms) vTaskDelay(pdMS_TO_TICKS((ms)))    

#ifdef __cplusplus
extern "C" {
#endif
 /**
  * @brief  串口初始化函数
  * @param  buard_rate: 波特率
  * @return None
  */
void serial_init(int buard_rate);

/**
 *  @brief  获取串口可用字节数
 *  @return 可用字节数
 */
int serial_available();

/**
  * @brief   串口发送函数
  * @param   len: 发送数组长度
  * @param   PS_Databuffer[]: 需要发送的功能数组
  * @return  None
  */
void serial_send(int len, uint8_t PS_Databuffer[]);

/**
  * @brief   串口接收函数,尝试接收len个数据到PS_Databuffer中，最多阻塞Timeout毫秒
  * @param   len: 接收数据长度, 注意不是接收数组的长度
  * @param   PS_Databuffer[]: 接收数组
  * @param   Timeout：接收超时时间,单位毫秒
  * @return  实际接收的字节数
  */
int serial_receive(int len, uint8_t PS_Databuffer[], uint16_t Timeout);

/**
 * @brief   清空Rx缓冲区
 * @param   None
 * @return  None
 */
void serial_fulsh();

#ifdef __cplusplus
}
#endif
