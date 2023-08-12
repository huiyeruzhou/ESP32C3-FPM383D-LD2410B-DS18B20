#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

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

#define CONFIG_GPIO_NUM GPIO_NUM_0
static const char *TAG = "Fan Server";
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
  httpd_handle_t *server = reinterpret_cast<httpd_handle_t *>(arg);
  if (*server) {
    ESP_LOGI(TAG, "Stopping webserver: %p", *server);
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
  io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
  io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pin_bit_mask = (1 << CONFIG_GPIO_NUM);
  ret = gpio_config(&io_conf);
  if (ret != ESP_OK) {
    return ret;
  }
  gpio_install_isr_service(0);
  // hook isr handler for specific gpio pin
  gpio_isr_handler_add(CONFIG_GPIO_NUM, gpio_isr_handler,
                       reinterpret_cast<void *>(CONFIG_GPIO_NUM));

  LOGI(TAG, "GPIO Init done.\r\n");
  return ESP_OK;
}
bool run = false;
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
      IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, sta_netif, &server));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, sta_netif,
      &server));
  ESP_LOGI(TAG, "wifi start");
  wifi_start_and_connect(sta_netif, ESP_WIFI_SSID, ESP_WIFI_PASS);

  // Initialize GPIO
  GPIO_Init();
  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));  // 创建消息队列
  xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10,
              NULL);  // 创建任务

  // Initialize Broadcast
  int max_len = 127;
  char *broadcast =
      reinterpret_cast<char *>(malloc((max_len + 1) * sizeof(char)));
  snprintf(broadcast, max_len, "%s+Idle,Stable", TAG);
  create_broad_task(broadcast);

  // Initialize RPC
  auto rpc_server = new erpc::SimpleServer("localhost", 12345);
  class myService : public sensor_ControlDeviceService_Service {
   public:
    rpc_status open(sensor_Empty *req, sensor_Empty *rsp) override {
      ESP_LOGI(TAG, "open");
      if (!run) {
        run = true;
        gpio_set_level(CONFIG_GPIO_NUM, 1);
      }
      return rpc_status::Success;
    }
    rpc_status close(sensor_Empty *req, sensor_Empty *rsp) override {
      ESP_LOGI(TAG, "close");
      if (run) {
        run = false;
        gpio_set_level(CONFIG_GPIO_NUM, 0);
      }
      return rpc_status::Success;
    }
    rpc_status ping(sensor_Empty *req, sensor_Empty *rsp) override {
      ESP_LOGI(TAG, "ping");
      return rpc_status::Success;
    }
  };
  auto service = new myService();
  rpc_server->addService(service);

  speakerContext = new AbilityContext("Fan-USB",

                                      "{\"FanDevices\": "
                                      "[{"
                                      "\"description\" : \"USB风扇\","
                                      "\"size\" : \"4寸\","
                                      "\"color\" : \"black\""
                                      "}]}",
                                      rpc_server);

  server = start_webserver();
}
