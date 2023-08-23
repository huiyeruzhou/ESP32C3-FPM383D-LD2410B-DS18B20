/*
 * Copyright 2023 YuHongli
 *
 * File: udp_broadcast.c
 * Description: udp_broadcast task
 * Version: V1.0.0
 * Date: 2023/08/23
 * Author: YuHongli
 * Revision History:
 *   Version       Date          Author         Revision Description
 *  V1.0.0        2023/08/23    YuHongli       Create and initialize
 */
#include "udp_broadcast.h"

#include <lwip/netdb.h>
#include <string.h>
#include <sys/param.h>

#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

__attribute__((unused)) static const char *TAG = "UDP Broadcast";

static void udp_broad_task(void *pvParameters)
{
    char *format_payload = "%s+%lld";
    // require pvParameters to be a dynamically allocated space
    const char *message = (const char *)pvParameters;
    char payload[128];
    while (1) {
        // addr_family = AF_INET;
        // ip_protocol = IPPROTO_IP;

        /*create udp socket*/
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(UDP_PORT);
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }

        /*set 1s timeout*/
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, UDP_PORT);
        /*start broadcast*/
        while (1) {
            int payload_length = snprintf(payload, sizeof(payload), format_payload, message, time(NULL));
            int err = sendto(sock, payload, payload_length, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent: %s", (char *)payload);
            /*4s*/
            vTaskDelay(4000 / portTICK_PERIOD_MS);
        }

        /*close socket*/
        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
        /*every 4s try to reconnect*/
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
    free(pvParameters);
    vTaskDelete(NULL);
}
/**
 * @brief create udp broadcast task
 * @param arg message to broadcast
 */
void create_broad_task(const char *arg)
{
    ESP_LOGI(TAG, "create_broad_task: %s", arg);
    xTaskCreate(udp_broad_task, "udp_broadcast", 4096, (void *const)arg, 0, NULL);
}
