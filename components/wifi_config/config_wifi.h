/*
 * Copyright 2023 YuHongli
 *
 * File: config_wifi.h
 * Description: the wifi config header
 * Version: V1.0.0
 * Date: 2023/08/23
 * Author: YuHongli
 * Revision History:
 *   Version       Date          Author         Revision Description
 *  V1.0.0        2023/08/23    YuHongli       Create and initialize
 */
#ifndef _CONFIG_WIFI_H_
#define _CONFIG_WIFI_H_
#include <netdb.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#define CONFIG_EXAMPLE_STATIC_DNS_AUTO 1
#define EXAMPLE_MAXIMUM_RETRY 5
#ifdef CONFIG_EXAMPLE_STATIC_DNS_AUTO
#define EXAMPLE_BACKUP_DNS_SERVER "0.0.0.0"
#else
#define EXAMPLE_MAIN_DNS_SERVER "114.114.114.114"
#define EXAMPLE_BACKUP_DNS_SERVER "115.115.115.115"
#endif
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
void ip_set(const char *ip, const char *mask, const char *gw);
esp_netif_t *wifi_init_sta();
bool wifi_start_and_connect(esp_netif_t *sta_netif, const char *ssid, const char *password);
#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _CONFIG_WIFI_H_