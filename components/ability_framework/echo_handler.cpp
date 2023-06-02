#include "handler_common.hpp"
/* An HTTP POST handler */
const char *TAG = "ECHO_HANDLER";
esp_err_t echo_post_handler(httpd_req_t *req) {
    char buf[100];
    int ret, remaining = req->content_len;

    /*接收所有发来的信息, 直到读取完毕.一次至多接收缓冲区长度大小的信息*/
    while (remaining > 0) {
        /* 读取请求信息 */
        if ((ret = httpd_req_recv(req, buf,
            MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /*超时重试*/
                continue;
            }
            return ESP_FAIL;
        }

        /* 用增量块形式发送 */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");
    }

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

extern const httpd_uri_t echo = {
    .uri = "/echo",
    .method = HTTP_POST,
    .handler = echo_post_handler,
    .user_ctx = NULL
};
