/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "serial.h"
#include "server/simple_server.hpp"
#include "sensor.pb.hpp"
#include <unistd.h>
#include "config_wifi.h"
#include "LD2410B/LD2410B.h"
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <string>
#include <ctime>
#include "nvs_flash.h"

#include "sdkconfig.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "udp_broadcast.h"
#include "esp_http_server.h"
#include "handles.hpp"
#include "ability_conext.hpp"

#include <sys/time.h>
#define ESP_WIFI_SSID "testap"
#define ESP_WIFI_PASS "testtest"
#define ESP_MAXIMUM_RETRY 5
AbilityContext *speakerContext = NULL;
static const char *TAG = "LD2410B Server";


static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.server_port = 8080;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &echo);
        // httpd_register_uri_handler(server, &ctrl);
        httpd_register_uri_handler(server, &api_Devices);
        httpd_register_uri_handler(server, &api_AbilityRunning);
        httpd_register_uri_handler(server, &api_AbilitySupport);
        httpd_register_uri_handler(server, &api_AbilityRequest);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server) {
    // Stop the httpd server
    return httpd_stop(server);
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
    int32_t event_id, void *event_data) {
    httpd_handle_t *server = (httpd_handle_t *) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver: %p", *server);
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        }
        else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

static void connect_handler(void *arg, esp_event_base_t event_base,
    int32_t event_id, void *event_data) {
    httpd_handle_t *server = (httpd_handle_t *) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

extern "C" void app_main(void)
{
    static httpd_handle_t server = NULL;
    printf("Hello world!\n");
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    esp_netif_t *sta_netif = wifi_init_sta();

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, sta_netif, &server));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, sta_netif, &server));
    ESP_LOGI(TAG, "wifi start");
    wifi_start_and_connect(sta_netif, ESP_WIFI_SSID, ESP_WIFI_PASS);

    //Initialize serial
    serial_init(256000);

    create_broad_task((std::string(TAG) + std::string("+Idle,Stable+") +
        std::to_string(std::time(nullptr))).c_str());


    //Initialize RPC
    auto rpc_server = new erpc::SimpleServer("localhost", 12345);
    class myService :public sensor_SensorService_Service {
    private:
        bool start = false;
    public:
        rpc_status open(sensor_Empty *req, sensor_Empty *rsp) override {
            ESP_LOGI(TAG, "open");
            /*open*/
            start = true;
            return rpc_status::Success;
        }
        rpc_status close(sensor_Empty *req, sensor_Empty *rsp) override {
            ESP_LOGI(TAG, "close");
            /*close*/
            if(start) {
                start = false;
            }
            return rpc_status::Success;
        }
        rpc_status read(sensor_Empty *req, sensor_Value *rsp) override {
            ESP_LOGI(TAG, "read");
            if (!start) {
                rsp->status = 1;
                rsp->value = -1;
                ESP_LOGE(TAG, "Cannot read before open");
                return rpc_status::Success;
            }
            uint16_t distance = 0;
            int mode = getDistance(&distance);
            if (mode == 0xFF) {
                ESP_LOGI(TAG, "Error Mode");
                rsp->status = 2;
                rsp->value = -1;
                return rpc_status::Success;
            }
            ESP_LOGI(TAG, "Mode: %d, Distance: %d", mode, distance);
            rsp->status = 0;
            rsp->value = (float) distance;
            return rpc_status::Success;
        }
        rpc_status configure(sensor_Value *req, sensor_Empty *rsp) override {
            ESP_LOGI(TAG, "configure");
            /*some configure*/
            startConfigure();
            configSensity(req->value);
            stopConfigure();
            return rpc_status::Success;
        }
    };
    auto service = new myService();
    rpc_server->addService(service);


    speakerContext = new AbilityContext(
        "LD18B20-DistanceRadar",
        
        "{\"distanceSensors\": "
        "[{"
        "\"description\" : \"LD2410B 人在传感器\""
        "}]}",
        
        rpc_server
        );

    server = start_webserver();





    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
