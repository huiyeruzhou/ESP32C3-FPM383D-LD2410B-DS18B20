#include "serial.h"

/**
 * @brief  串口初始化函数
 * @param  buard_rate: 波特率
 * @return None
 */
void serial_init(int baud_rate) {
  // 默认配置
  uart_config_t uart_config = {
      .baud_rate = baud_rate,                 // 波特率
      .data_bits = UART_DATA_8_BITS,          // 8位数据位
      .parity = UART_PARITY_DISABLE,          // 无奇偶校验
      .stop_bits = UART_STOP_BITS_1,          // 1位停止位
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,  // 无硬件流控
      .rx_flow_ctrl_thresh = 122,             // 硬件流控阈值
      .source_clk = UART_SCLK_DEFAULT,        // 默认时钟源
  };
  int intr_alloc_flags = 0;  // 不分配中断标志

#if CONFIG_UART_ISR_IN_IRAM
  intr_alloc_flags =
      ESP_INTR_FLAG_IRAM;  // 如果要求中断向量放在iram中，设置这个标志
#endif

  // 安装驱动程序, UART串口号为UART_PORT_NUM, 接收缓冲区大小为BUF_SIZE,
  // 发送缓冲区大小为0, 不使用事件队列
  ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE, 0, 0, NULL,
                                      intr_alloc_flags));
  // 设置串口参数
  ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
  // 设置串口引脚, 默认GPIO4为TXD, GPIO5为RXD, GPIO18为RTS, GPIO19为CTS
  ESP_ERROR_CHECK(
      uart_set_pin(UART_PORT_NUM, GPIO_TXD, GPIO_RXD, GPIO_RTS, GPIO_CTS));
}

int serial_available() {
  size_t bytes_available = 0;
  uart_get_buffered_data_len(UART_PORT_NUM, &bytes_available);
  return bytes_available;
}

void serial_send(int len, uint8_t PS_Databuffer[]) {
  uart_write_bytes(UART_PORT_NUM, (const char *)PS_Databuffer, len);
  ESP_ERROR_CHECK(uart_wait_tx_done(UART_PORT_NUM, portMAX_DELAY));
}

int serial_receive(int len, uint8_t PS_Databuffer[], uint16_t Timeout) {
  return uart_read_bytes(UART_PORT_NUM, PS_Databuffer, len,
                         pdMS_TO_TICKS(Timeout));
}

void serial_fulsh() { ESP_ERROR_CHECK(uart_flush(UART_PORT_NUM)); }

void serial_get_buffered_data_len(size_t *len) {
  uart_get_buffered_data_len(UART_PORT_NUM, len);
}
