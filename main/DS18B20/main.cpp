
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "serial.h"
#include "DS18B20/DS18B20.h"
#include "server/simple_server.hpp"
#include "sensor.pb.hpp"
#include <unistd.h>
#include "config_wifi.h"
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <string>
#include <ctime>
#include <cfloat>
#include <atomic>
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
static const char *TAG = "DS18B20 Server";
AbilityContext *speakerContext = NULL;

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
    // if (*server) {
    //     ESP_LOGI(TAG, "Stopping webserver: %p", *server);
    //     if (stop_webserver(*server) == ESP_OK) {
    //         *server = NULL;
    //     }
    //     else {
    //         ESP_LOGE(TAG, "Failed to stop http server");
    //     }
    // }
}

static void connect_handler(void *arg, esp_event_base_t event_base,
    int32_t event_id, void *event_data) {
    httpd_handle_t *server = (httpd_handle_t *) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

std::atomic<float> temperature;
std::atomic <bool> run = false;
void stub(void *ctx) {
    //初始化：获取手机IP，Port，创建并打开RPC客户端
    AbilityContext *speakerContext = (AbilityContext *) ctx;
    auto updateClient = new sensor_UpdateService_Client(
        speakerContext->getConnectIP(),
        speakerContext->getConnectPort()
    );
    updateClient->open();
    //同步变量
    while (run) {
        //传感器驱动，获取温度值
        temperature = get_temperature();

        //调用RPC
        sensor_Value value;
        sensor_Empty empty;
        value.value = temperature;
        value.status = 0;
        rpc_status ret = updateClient->update(&value, &empty);

        //打印日志
        if (ret != rpc_status::Success) {
            ESP_LOGE(TAG, "Failed to update: T=%+06.1fC\n", value.value);
        }
        else {
            ESP_LOGI(TAG, "Update success: T=%+06.1fC\n", value.value);
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    //退出前释放资源
    delete updateClient;
    vTaskDelete(NULL);
}
extern "C" void app_main(void) {
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
    serial_init(9600);
    if (set_hand()) {
        ESP_LOGE(TAG, "Failed to set hand mode");
    }

    char *broadcast = (char *) malloc(128 * sizeof(char));
    snprintf(broadcast, 127, "%s+Idle,Stable", TAG);
    create_broad_task(broadcast);

    //Initialize RPC
    auto rpc_server = new erpc::SimpleServer("localhost", 12345);
    class myService :public sensor_SensorService_Service {
    private:
        bool config_int = false;
    public:
        rpc_status open(sensor_Empty *req, sensor_Empty *rsp) override {
            ESP_LOGI(TAG, "open");
            if (!run) {
                run = true;
                xTaskCreate(stub, "serial", 4096, speakerContext, 3, NULL);
            }
            return rpc_status::Success;
        }
        rpc_status close(sensor_Empty *req, sensor_Empty *rsp) override {
            ESP_LOGI(TAG, "close");
            if (run) {
                run = false;
            }
            return rpc_status::Success;
        }
        rpc_status read(sensor_Empty *req, sensor_Value *rsp) override {
            ESP_LOGI(TAG, "read");
            if (!run) {
                rsp->status = 1;
                rsp->value = -1;
                ESP_LOGE(TAG, "Cannot read before open");
                return rpc_status::Success;
            }
            rsp->status = 0;
            // rsp->value = get_temperature();
            rsp->value = temperature;
            if (config_int) {
                rsp->value = (int)rsp->value;
            }
            ESP_LOGW(TAG, "T=%+06.1fC\n", rsp->value);
            return rpc_status::Success;
        }
        rpc_status configure(sensor_Value *req, sensor_Empty *rsp) override {
            //比较浮点数相等
            if (req->value == 0) {
                config_int = false;
            }
            else if (req->value == 1) {
                config_int = true;
            }
            else {
                ESP_LOGE(TAG, "Invalid configure");
                return rpc_status::InvalidArgument;
            }
            /*some configure*/
            return rpc_status::Success;
        }
    };
    auto service = new myService();
    rpc_server->addService(service);
    
    speakerContext = new AbilityContext(
        "DS18B20-Temperature",

        "{\"temperatureSensors\": "
        "[{"
        "\"format\": \"Centigrade\","
        "\"description\" : \"温度传感器\","
        "\"minimum\" : \"-55C\","
        "\"maximum\" : \"125C\""
        "}]}",

        rpc_server

    );

    server = start_webserver();
}


