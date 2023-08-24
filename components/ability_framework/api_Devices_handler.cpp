/*
 * Copyright (C), 2022-2023, Soochow University & OPPO Mobile Comm Corp., Ltd.
 *
 * File: api_Devices_handler.cpp
 * Description: the devices http-handler
 * Version: V1.0.0
 * Date: 2023/08/23
 * Author: Soochow University
 * Revision History:
 *   Version       Date          Author         Revision Description
 *  V1.0.0        2023/08/23    Soochow University       Create and initialize
 */
#include "ability_conext.hpp"
#include "handler_common.hpp"

#define TAG "API_DEVICES_HANDLER"

/* An HTTP GET handler */
esp_err_t api_Devices_handler(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = reinterpret_cast<char *>(malloc(buf_len));
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    esp_err_t err = httpd_resp_set_type(req, "application/json");
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "httpd_resp_set_type failed with err=%d", err);
        return err;
    }

    ESP_LOGW(TAG, "AbilityContext = %p", req->user_ctx);
    const char *resp_str = (const char *)(speakerContext->getDevicesList());
    ESP_LOGI(TAG, "response body: %s", resp_str);

    err = httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "httpd_resp_send failed with err=%d", err);
        return err;
    }
    return ESP_OK;
}
extern const httpd_uri_t api_Devices = {.uri = "/api/Devices",
                                        .method = HTTP_GET,
                                        .handler = api_Devices_handler,
                                        /* Let's pass response string in user
                                         * context to demonstrate it's usage */
                                        .user_ctx = &speakerContext};
