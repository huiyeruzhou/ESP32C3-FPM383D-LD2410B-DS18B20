#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "serial.h"
#include "DS18B20/DS18B20.h"
#include "server/simple_server.hpp"
#include "sensor.pb.hpp"
#include <unistd.h>
#include "config_wifi.h"


static const char *TAG = "DS18B20 Server";
extern "C" void app_main(void) {
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
    serial_init(9600);
        if (set_hand()) {
        ESP_LOGE(TAG, "Failed to set hand mode");
    }

    //Initialize RPC
    auto server = new erpc::SimpleServer("localhost", 12345);
    class myService :public sensor_SensorService_Service {
    private:
        bool start;
    public:
        rpc_status open(sensor_Empty *req, sensor_Empty *rsp) override {
            ESP_LOGI(TAG, "open");
            start = true;
            return rpc_status::Success;
        }
        rpc_status close(sensor_Empty *req, sensor_Empty *rsp) override {
            ESP_LOGI(TAG, "close");
            if (start) {
                start = false;
            }
            return rpc_status::Success;
        }
        rpc_status read(sensor_Empty *req, sensor_Value *rsp) override {
            ESP_LOGI(TAG, "read");
            if (!start) {
                rsp->status = 1;
                rsp->value = -1;
                return rpc_status::Success;
            }
            rsp->status = 0;
            rsp->value = get_temperature();
            ESP_LOGW(TAG, "T=%+06.1fC\n", rsp->value);
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




