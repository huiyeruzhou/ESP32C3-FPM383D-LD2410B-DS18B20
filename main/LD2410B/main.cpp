/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

#include "LD2410B/LD2410B.h"
#include "ability_conext.hpp"
#include "config_wifi.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
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
  httpd_handle_t *server = reinterpret_cast<httpd_handle_t *>(arg);
  if (*server) {
    if (stop_webserver(*server) == ESP_OK) {
      *server = NULL;
    } else {
      ESP_LOGE(TAG, "Failed to stop http server");
    }
  }
  esp_restart();
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data) {
  httpd_handle_t *server = reinterpret_cast<httpd_handle_t *>(arg);
  if (*server == NULL) {
    ESP_LOGI(TAG, "Starting webserver");
    *server = start_webserver();
  }
}

std::atomic<bool> run = false;
void stub(void *ctx) {
  // 初始化：获取手机IP，Port，创建并打开RPC客户端
  AbilityContext *speakerContext = reinterpret_cast<AbilityContext *>(ctx);
  auto updateClient = new sensor_UpdateService_Client(
      speakerContext->getConnectIP(), speakerContext->getConnectPort());
  updateClient->open();
  TickType_t xLastWakeTime;
  const TickType_t xDelay80ms = pdMS_TO_TICKS(80);
  getDistance(NULL);
  int fail_cnt = 0;
  // 同步变量
  while (run) {
    xLastWakeTime = xTaskGetTickCount();
    // 传感器驱动，获取距离和模式
    sensor_Value value;
    sensor_Empty empty;
    uint16_t distance = 0;
    int mode = getDistance(&distance);
    // originDistance = distance;
    if (mode == 0xFF) {
      ESP_LOGI(TAG, "Error Mode");
      value.status = 2;
      value.value = -1;
    } else {
      ESP_LOGI(TAG, "Mode: %d, Distance: %d", mode, distance);
      value.value = static_cast<float>(distance);
      value.status = 0;
    }

    // 调用RPC
    rpc_status ret = updateClient->update(&value, &empty);
    // 打印日志
    if (ret != rpc_status::Success) {
      ESP_LOGE(TAG, "Failed to update: Mode: %d, Distance: %d, fail_cnt = %d",
               mode, distance, fail_cnt);
      fail_cnt++;
    } else {
      ESP_LOGI(TAG, "Update success: Mode: %d, Distance: %d", mode, distance);
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
static xQueueHandle gpio_evt_queue = NULL;
static void IRAM_ATTR gpio_isr_handler(void *arg) {
  uint32_t gpio_num = (uint32_t)arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num,
                    NULL);  // freertos中断中发送消息队列
}
static void gpio_task_example(void *arg) {
  uint32_t io_num;
  for (;;) {
    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      LOGE(TAG, "GPIO[%" PRIu32 "] intr, val: %d\n", io_num,
           gpio_get_level((gpio_num_t)io_num));
    }
  }
}
int GPIO_Init(void) {
  gpio_config_t io_conf;
  esp_err_t ret;

  io_conf.intr_type = GPIO_INTR_ANYEDGE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pin_bit_mask = (1 << GPIO_NUM_0);
  ret = gpio_config(&io_conf);
  if (ret != ESP_OK) {
    return ret;
  }
  gpio_install_isr_service(0);
  // hook isr handler for specific gpio pin
  gpio_isr_handler_add(GPIO_NUM_0, gpio_isr_handler,
                       reinterpret_cast<void *>(GPIO_NUM_0));

  LOGI(TAG, "GPIO Init done.\r\n");
  return ESP_OK;
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
  serial_init(256000);
  int max_len = 127;
  char *broadcast =
      reinterpret_cast<char *>(malloc((max_len + 1) * sizeof(char)));
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
      rsp->value = static_cast<float>(distance);
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
    rpc_status ping(sensor_Empty *req, sensor_Empty *rsp) override {
      ESP_LOGI(TAG, "ping");
      return rpc_status::Success;
    }
  };
  auto service = new myService();
  rpc_server->addService(service);

  speakerContext = new AbilityContext("LD2410B-DistanceRadar",

                                      "{\"distanceSensors\": "
                                      "[{"
                                      "\"description\" : \"LD2410B 人在传感器\""
                                      "}]}",

                                      rpc_server);

  server = start_webserver();
  // xTaskCreate(test_task, "test_task", 2048, NULL, 24, NULL);
  GPIO_Init();
  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));  // 创建消息队列
  xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10,
              NULL);  // 创建任务
  return;
}
