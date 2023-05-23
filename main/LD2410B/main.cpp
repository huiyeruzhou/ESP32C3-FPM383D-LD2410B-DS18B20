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



static const char *TAG = "LD2410B Server";
extern "C" void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //Initialize wifi
    // ip_set("192.168.1.120", "255.255.255.0", "192.168.1.1");
    wifi_init_sta("yu", "esp32c3.");

    //Initialize serial
    serial_init(256000);

    //Initialize RPC
    auto server = new erpc::SimpleServer("localhost", 12345);
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
            if(!start) {
                rsp->value = -1;
                return rpc_status::Fail;
            }
            uint16_t distance = 0;
            int mode = getDistance(&distance);
            if (mode == 0xFF) {
                ESP_LOGI(TAG, "Error Mode");
                rsp->value = -1;
                return rpc_status::Fail;
            }
            ESP_LOGI(TAG, "Mode: %d, Distance: %d", mode, distance);
            rsp->value = (float) distance;
            return rpc_status::Success;
        }
        rpc_status configure(sensor_Value *req, sensor_Empty *rsp) override {
            ESP_LOGI(TAG, "configure");
            /*some configure*/
            return rpc_status::Success;
        }
    };
    auto service = new myService();
    server->addService(service);
    server->open();

    /* run server */
    server->run();
}
