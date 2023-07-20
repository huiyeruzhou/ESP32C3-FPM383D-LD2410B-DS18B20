#include "handler_common.hpp"
#include "handler_common.hpp"
#include "ability_conext.hpp"
const static char *TAG = "API_ABILITYREQUEST_HANDLER";


/* An HTTP GET handler */
esp_err_t api_AbilityRequest_handler(httpd_req_t *req) {
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

    /* 读取URL参数 */
    int status = 200;
    std::string resp_body = "";
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char *) malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "abilityName", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => abilityName=%s", param);
                ESP_LOGI(TAG, "abilityContext = %p", speakerContext);
                if (strcmp(param, speakerContext->getAbilityName()) != 0) {
                    ESP_LOGE(TAG, "abilityName not match");
                    resp_body += "abilityName not match\n";
                    httpd_resp_set_status(req, "404 Not Found");
                    status = 404;
                }
            }
            if (status == 200 && httpd_query_key_value(buf, "port", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => port=%s", param);
                int port = atoi(param);
                if (port != speakerContext->getLifecyclePort()) {
                    ESP_LOGE(TAG, "port not match");
                    resp_body += "port not match\n";
                    httpd_resp_set_status(req, "404 Not Found");
                    status = 404;
                }
            }
            if (status == 200 && httpd_query_key_value(buf, "cmd", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => cmd=%s", param);
                //compare with cmd 
                const char *cmd[4] = { "start", "connect", "disconnect", "terminate" };
                int i;
                for (i = 0; i < 4; i++) {
                    if (strcmp(param, cmd[i]) == 0) {
                        if (!speakerContext->check_cmd_legal(i)) {
                            ESP_LOGE(TAG, "cannot perform `%s` on `%s` status",
                                param, speakerContext->getStatusString());
                            resp_body += "cannot perform `" + std::string(param) + "` on `" + std::string(speakerContext->getStatusString()) + "` status\n";
                            status = 404;
                        }
                        else {
                            ESP_LOGW(TAG, "performing `%s`", param);
                            ESP_LOGW(TAG, "status = %s", speakerContext->getStatusString());
                            speakerContext->do_cmd(i);
                            ESP_LOGW(TAG, "status = %s", speakerContext->getStatusString());
                        }
                        break;
                    }
                }
                //if not match
                if (i >= 4) {
                    ESP_LOGE(TAG, "cmd not match");
                    resp_body += "cmd not match\n";
                    httpd_resp_set_status(req, "404 Not Found");
                    status = 404;
                }
            }
            if (status == 200 && httpd_query_key_value(buf, "connectIP", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => connectIP=%s", param);
                speakerContext->setConnectIP(param);
            }
            if (status == 200 && httpd_query_key_value(buf, "connectPort", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => connectPort=%s", param);
                speakerContext->setConnectPort(atoi(param));
            }
        }
        else {
            status = 404;
            ESP_LOGE(TAG, "httpd_req_get_url_query_str error");
            resp_body += "httpd_req_get_url_query_str error\n";
        }
        free(buf);
    }
    else {
        status = 404;
        httpd_resp_set_status(req, "404 Not Found");
        resp_body += "no argument set!\n";
    }


    /* 设置响应类型为JSON */
    esp_err_t err = httpd_resp_set_type(req, "application/json");
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "httpd_resp_set_type failed with err=%d", err);
        return err;
    }

    /* 设置响应体*/
    /* 将响应头和响应体拼装成响应信息, 宏指明buf长度用按照字符串而非缓冲区长度计算*/
    if (!resp_body.size()) {
        resp_body += "ability requested";
    }
    err = httpd_resp_send(req, resp_body.c_str(), HTTPD_RESP_USE_STRLEN);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "httpd_resp_send failed with err=%d", err);
        return err;
    }
    return ESP_OK;
}
extern const httpd_uri_t api_AbilityRequest = {
    .uri = "/api/AbilityRequest",
    .method = HTTP_POST,
    .handler = api_AbilityRequest_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx = &speakerContext
};
