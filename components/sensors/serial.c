/*
 * Copyright (C), 2022-2023, Soochow University & OPPO Mobile Comm Corp., Ltd.
 *
 * File: serial.c
 * Description: serial driver
 * Version: V1.0.0
 * Date: 2023/08/23
 * Author: Soochow University
 * Revision History:
 *   Version       Date          Author         Revision Description
 *  V1.0.0        2023/08/23    Soochow University       Create and initialize
 */
#include "serial.h"

void serial_init(int baud_rate)
{
    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;  // if set, this interrupt will be serviced from IRAM
#endif

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    // GPIO4 TXD, GPIO5 RXD, GPIO18 RTS, GPIO19 CTS
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, GPIO_TXD, GPIO_RXD, GPIO_RTS, GPIO_CTS));
}

int serial_available()
{
    size_t bytes_available = 0;
    uart_get_buffered_data_len(UART_PORT_NUM, &bytes_available);
    return bytes_available;
}

void serial_send(int len, uint8_t PS_Databuffer[])
{
    uart_write_bytes(UART_PORT_NUM, (const char *)PS_Databuffer, len);
    ESP_ERROR_CHECK(uart_wait_tx_done(UART_PORT_NUM, portMAX_DELAY));
}

int serial_receive(int len, uint8_t PS_Databuffer[], uint16_t Timeout)
{
    return uart_read_bytes(UART_PORT_NUM, PS_Databuffer, len, pdMS_TO_TICKS(Timeout));
}

void serial_fulsh() { ESP_ERROR_CHECK(uart_flush(UART_PORT_NUM)); }
