#include "config_wifi.h"
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "WIFI";
static int ip_static = 0;
static const char *ip_ip, *ip_netmask, *ip_gw;
static bool is_ip_connected = false;
static char ip_connected[20];

static int s_retry_num = 0;

static esp_err_t example_set_dns_server(esp_netif_t *netif, uint32_t addr, esp_netif_dns_type_t type) {
    if (addr && (addr != IPADDR_NONE)) {
        esp_netif_dns_info_t dns;
        dns.ip.u_addr.ip4.addr = addr;
        dns.ip.type = IPADDR_TYPE_V4;
        ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, type, &dns));
    }
    return ESP_OK;
}

static void example_set_static_ip(esp_netif_t *netif) {
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop dhcp client");
        return;
    }
    esp_netif_ip_info_t ip;
    memset(&ip, 0, sizeof(esp_netif_ip_info_t));
    ip.ip.addr = ipaddr_addr(ip_ip);
    ip.netmask.addr = ipaddr_addr(ip_netmask);
    ip.gw.addr = ipaddr_addr(ip_gw);
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ip info");
        return;
    }
    ESP_LOGD(TAG, "Success to set static ip: %s, netmask: %s, gw: %s", ip_ip, ip_netmask, ip_gw);
    ESP_ERROR_CHECK(example_set_dns_server(netif, ipaddr_addr(ip_gw), ESP_NETIF_DNS_MAIN));
    ESP_ERROR_CHECK(example_set_dns_server(netif, ipaddr_addr(EXAMPLE_BACKUP_DNS_SERVER), ESP_NETIF_DNS_BACKUP));
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (ip_static && event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        example_set_static_ip(arg);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        is_ip_connected = false;
        if (s_retry_num < EXAMPLE_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "static ip:" IPSTR, IP2STR(&event->ip_info.ip));
        int len = snprintf(ip_connected, 20, IPSTR, IP2STR(&event->ip_info.ip));
        is_ip_connected = true;
        ip_connected[len] = 0;
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void ip_set(const char *ip, const char *mask, const char *gw) {
    ip_static = 1;
    ip_ip = ip;
    ip_netmask = mask;
    ip_gw = gw;
}

esp_netif_t *wifi_init_sta() {
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    return sta_netif;

}
void wifi_start_and_connect(esp_netif_t *sta_netif, const char *ssid, const char *password) {
    wifi_config_t wifi_config = {
    .sta = {
            // .ssid = CONFIG_EXAMPLE_WIFI_SSID,
            // .password = CONFIG_EXAMPLE_WIFI_PASSWORD,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
         .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    
    static esp_event_handler_instance_t instance_any_id;
    static esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, sta_netif, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, sta_netif, &instance_got_ip));
    
    ESP_ERROR_CHECK(esp_wifi_start());
    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", ssid, password);
    }
    else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid, password);
    }
    else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);

}

const char *get_ip_connected() {
    return is_ip_connected ? ip_connected : "";
}