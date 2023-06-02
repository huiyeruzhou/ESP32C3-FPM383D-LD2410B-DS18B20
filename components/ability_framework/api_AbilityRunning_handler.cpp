#include "handler_common.hpp"
#include "handler_common.hpp"
#include "ability_conext.hpp"

const static char *TAG = "API_ABILITYRUNNING_HANDLER";


/* An HTTP GET handler */
esp_err_t api_AbilityRunning_handler(httpd_req_t *req) {
    char *buf;
    size_t buf_len;

    /*读取HEADER*/
    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = (char *) malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    /* 设置响应类型为JSON */
    esp_err_t err = httpd_resp_set_type(req, "application/json");
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "httpd_resp_set_type failed with err=%d", err);
        return err;
    }

    /* 设置响应体*/
    char *resp_str = (char *) (speakerContext->getAbilityRunningState());
    ESP_LOGI(TAG, "response body: %s", resp_str);

    /* 将响应头和响应体拼装成响应信息, 宏指明buf长度用按照字符串而非缓冲区长度计算*/
    err = httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "httpd_resp_send failed with err=%d", err);
        return err;
    }
    free(resp_str);
    return ESP_OK;
}
extern const httpd_uri_t api_AbilityRunning = {
    .uri = "/api/AbilityRunning",
    .method = HTTP_GET,
    .handler = api_AbilityRunning_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx = &speakerContext
};
