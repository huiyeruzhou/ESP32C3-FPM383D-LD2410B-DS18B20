/*
 * Copyright (C), 2022-2023, Soochow University & OPPO Mobile Comm Corp., Ltd.
 *
 * File: serial.h
 * Description: serial driver
 * Version: V1.0.0
 * Date: 2023/08/23
 * Author: Soochow University
 * Revision History:
 *   Version       Date          Author         Revision Description
 *  V1.0.0        2023/08/23    Soochow University       Create and initialize
 */
#pragma once
#include "driver/gpio.h"
#include "driver/uart.h"
#define GPIO_TXD (GPIO_NUM_4)
#define GPIO_RXD (GPIO_NUM_5)
#define GPIO_RTS (UART_PIN_NO_CHANGE)
#define GPIO_CTS (UART_PIN_NO_CHANGE)

#define UART_PORT_NUM (UART_NUM_1)
#define BUF_SIZE (512)

#define delay(ms) vTaskDelay(pdMS_TO_TICKS((ms)))

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief  init serial
 * @param  buard_rate: buard rate
 * @return None
 */
void serial_init(int buard_rate);

/**
 *  @brief  get number of bytes in Rx buffer
 *  @return available bytes
 */
int serial_available();

/**
 * @brief   send data to serial
 * @param   len: length of data
 * @param   PS_Databuffer[]: data to send
 * @return  None
 */
void serial_send(int len, uint8_t PS_Databuffer[]);

/**
 * @brief   receive data from serial
 * @param   len: length of data to receive
 * @param   PS_Databuffer[]: buffer to store data
 * @param   Timeout：timeout in ms
 * @return  number of bytes received
 */
int serial_receive(int len, uint8_t PS_Databuffer[], uint16_t Timeout);

/**
 * @brief   flush Rx buffer
 * @param   None
 * @return  None
 */
void serial_fulsh();

#ifdef __cplusplus
}
#endif
