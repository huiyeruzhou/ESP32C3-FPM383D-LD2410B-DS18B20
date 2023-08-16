#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

#include "FPM_383D/FPM_383D.h"
#include "ability_conext.hpp"
#include "config_wifi.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "handles.hpp"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "sensor.pb.hpp"
#include "serial.h"
#include "server/simple_server.hpp"
#include "udp_broadcast.h"
#include "wifi_info.hpp"
static const char *TAG = "FPM383D Server";
AbilityContext *speakerContext = NULL;
std::atomic<bool> run = false;
std::atomic<float> limit = -1;

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
  httpd_handle_t *server = reinterpret_cast<httpd_handle_t *>(arg);
  if (*server) {
    ESP_LOGI(TAG, "Stopping webserver");
    if (stop_webserver(*server) == ESP_OK) {
      *server = NULL;
    } else {
      ESP_LOGE(TAG, "Failed to stop http server");
    }
  }
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data) {
  httpd_handle_t *server = reinterpret_cast<httpd_handle_t *>(arg);
  if (*server == NULL) {
    ESP_LOGI(TAG, "Starting webserver");
    *server = start_webserver();
  }
}

void stub(void *ctx) {
  // 初始化：获取手机IP，Port，创建并打开RPC客户端
  AbilityContext *speakerContext = reinterpret_cast<AbilityContext *>(ctx);
  auto updateClient = new sensor_UpdateService_Client(
      speakerContext->getConnectIP(), speakerContext->getConnectPort());
  updateClient->open();
  TickType_t xLastWakeTime;
  const TickType_t xDelay80ms = pdMS_TO_TICKS(80);
  int fail_cnt = 0;
  // 同步变量
  while (run) {
    xLastWakeTime = xTaskGetTickCount();
    // 传感器驱动
    sensor_Value value;
    sensor_Empty empty;
    uint16_t id = 0, score = 0;
    int status = PS_SearchMB(&id, &score);
    float local_limit = limit;
    if (status) {
      value.status = 2;
      value.value = static_cast<float>(id);
      ESP_LOGE(TAG, "Failed to recognize finger, status=%d", status);
      if (status == 0x00000008 || 0xFF) {
        ESP_LOGE(TAG, "Finger not found, timeout.");
        value.value = 0.0 / 0.0;
      }
    } else if (id == 0xFFFF) {
      value.status = 3;
      ESP_LOGW(TAG, "Macth Failed, score=%d", score);
      value.value = static_cast<float>(id);
    } else {
      ESP_LOGW(TAG, "id=%d, score=%d", id, score);
      if (local_limit < 0 || score > local_limit) {
        value.status = 0;
        value.value = static_cast<float>(id);
      } else {
        value.status = 4;
        value.value = static_cast<float>(id);
        ESP_LOGE(TAG, "Score too low, limit=%f", local_limit);
      }
    }

    // 调用RPC
    rpc_status ret = updateClient->update(&value, &empty);

    // 打印日志
    if (ret != rpc_status::Success) {
      ESP_LOGE(TAG, "Failed to update: id = %d, score = %d, limit=%f", id,
               score, local_limit);
      fail_cnt++;
    } else {
      ESP_LOGI(TAG, "Update success: id = %d, score = %d, limit=%f", id, score,
               local_limit);
      fail_cnt = 0;
    }
    if (fail_cnt > 5) {
      ESP_LOGE(TAG, "Failed more than 5 times, exit.");
      run = false;
      break;
    }
    vTaskDelayUntil(&xLastWakeTime, xDelay80ms);
  }
  // 退出前释放资源
  delete updateClient;
  vTaskDelete(NULL);
}

extern "C" void app_main(void) {
  static httpd_handle_t server = NULL;
  printf("Hello world!\n");
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  esp_netif_t *sta_netif = wifi_init_sta();

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server,
      NULL));
  ESP_LOGI(TAG, "wifi start");
  wifi_start_and_connect(sta_netif, ESP_WIFI_SSID, ESP_WIFI_PASS);

  // Initialize serial
  serial_init(57600);

  int max_len = 127;
  char *broadcast = static_cast<char *>(malloc((max_len + 1) * sizeof(char)));
  snprintf(broadcast, max_len, "%s+Idle,Stable", TAG);
  create_broad_task(broadcast);

  // Initialize RPC
  auto rpc_server = new erpc::SimpleServer("localhost", 12345);
  class myService : public sensor_SensorService_Service {
   public:
    rpc_status open(sensor_Empty *req, sensor_Empty *rsp) override {
      ESP_LOGI(TAG, "open");
      if (!run) {
        run = true;
        xTaskCreate(stub, "serial", 4096, speakerContext, 3, NULL);
        // xTaskCreate(getDistanceStub, "LD2410B serial", 4096, NULL, 1, NULL);
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
      uint16_t id = 0, score = 0;
      float local_limit = limit;
      int status = PS_SearchMB(&id, &score);
      if (status) {
        rsp->status = 2;
        rsp->value = static_cast<float>(id);
        ESP_LOGE(TAG, "Failed to recognize finger, status=%d", status);
        if (status == 0x00000008 || 0xFF) {
          ESP_LOGE(TAG, "Finger not found, timeout.");
          rsp->value = 0.0 / 0.0;
        }
      } else if (id == 0xFFFF) {
        rsp->status = 3;
        ESP_LOGW(TAG, "Macth Failed, score=%d", score);
        rsp->value = static_cast<float>(id);
      } else {
        ESP_LOGW(TAG, "id=%d, score=%d", id, score);
        if (local_limit < 0 || score > local_limit) {
          rsp->status = 0;
          rsp->value = static_cast<float>(id);
        } else {
          rsp->status = 4;
          rsp->value = static_cast<float>(id);
          ESP_LOGE(TAG, "Score too low, limit=%f", local_limit);
        }
      }
      return rpc_status::Success;
    }
    rpc_status configure(sensor_Value *req, sensor_Empty *rsp) override {
      ESP_LOGI(TAG, "configure");
      limit = req->value;
      /*some configure*/
      return rpc_status::Success;
    }
    rpc_status ping(sensor_Empty *req, sensor_Empty *rsp) override {
      ESP_LOGI(TAG, "ping");
      return rpc_status::Success;
    }
  };
  auto service = new myService();
  rpc_server->addService(service);

  speakerContext = new AbilityContext("FPM383D-Fingerprint",

                                      "{\"fingerprintSensors\": "
                                      "[{"
                                      "\"description\" : \"海陵科 指纹传感器\""
                                      "}]}",

                                      rpc_server);

  server = start_webserver();

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
