#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "serial.h"
#include "FPM_383D/FPM_383D.h"
#include "server/simple_server.hpp"
#include "sensor.pb.hpp"
#include <unistd.h>
#include "config_wifi.h"


static const char *TAG = "FPM_383D Server";
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
    serial_init(57600);

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
                rsp->value = -1;
                return rpc_status::Fail;
            }
            
            int status = PS_SearchMB(&id, &score);
            if (status) {
                ESP_LOGE(TAG, "Failed to recognize finger, status=%d", status);
                return rpc_status::Fail;
            }
            else if (id == 0xFFFF) {
                ESP_LOGW(TAG, "Macth Failed, score=%d", score);
                rsp->value = (float) id;
                return rpc_status::Fail;
            }
            else {
                ESP_LOGW(TAG, "id=%d, score=%d", id, score);
                rsp->value = (float) id;
                return rpc_status::Success;
            }
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
