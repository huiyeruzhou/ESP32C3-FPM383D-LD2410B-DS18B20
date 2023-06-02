/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "udp_broadcast.h"



static const char *TAG = "UDP Broadcast";


static void udp_broad_task(void *pvParameters)
{
    
    /*广播内容*/
    char *format_payload = "%s";
    const char *message = (const char *) pvParameters;
    char payload[128];
    int payload_length = snprintf(payload, sizeof(payload), format_payload, message);

    while (1) {
        // addr_family = AF_INET;
        // ip_protocol = IPPROTO_IP;

        /*创建UDP套接字*/
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(UDP_PORT);
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }

        /*设置发送超时时间为1s*/
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, UDP_PORT);
        /*开始进行广播*/
        while (1) {
            int err = sendto(sock, payload, payload_length, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent: %s", (char *)payload);
            /*4s一次*/
            vTaskDelay(4000 / portTICK_PERIOD_MS);
        }

        /*关闭套接字*/
        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}
/**
 * @brief 创建广播任务
 * @param arg 广播内容
*/
void create_broad_task(const char * arg){
    xTaskCreate(udp_broad_task, "udp_broadcast", 4096, arg, 5, NULL);
}
