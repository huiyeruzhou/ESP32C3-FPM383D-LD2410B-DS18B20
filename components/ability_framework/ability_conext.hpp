#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <string>
#include <ctime>
#include <cinttypes>
#include "server/simple_server.hpp"

#define RPC_PORT 12345
enum Status {
    INIT = 0,
    STANDBY = 1,
    RUNNING = 2,
    SUSPEND = 3,
    TERMINATE = 4,
    UNKNOWN = 0x80000000
};
enum Cmd {
    START = 0,
    CONNECT = 1,
    DISCONNECT = 2,
    TERMINATE_CMD = 3
};
class AbilityContext {
private:
    erpc::SimpleServer *rpc_server;
    char connectIp[16];
    uint16_t connectPort;
    constexpr static const char *TAG = "ABILITY_CONTEXT";
    
    const char *abilityName = "";
    const char *devicesList = "";

    constexpr static const char *abilityRunningStateFormat = 
        "[{\"abilityName\": %s,"
        "\"abilityPort\" : %d,"
        "\"last_update\" : %" PRId64 ","
        "\"port\" : %d,"
        "\"status\" : \"%s\"}]";

    constexpr static const char *abilitySupportFormat = "[{\"depends\":"
        " {\"abilities\": [\"none\"],"
        "\"devices\" : [%s]},"
        "\"level\": 0,"
        "\"name\" : %s}]";

    int abilityPort = 0;
    unsigned status = INIT;
    //每行一种状态,每列一个动作
    unsigned status_transfer[5][4] = {
        //start connect  disconnect  terminate
        //init can start, terminate
        {STANDBY, UNKNOWN, UNKNOWN, TERMINATE},
        //standby can start, terminate
        {UNKNOWN, RUNNING, UNKNOWN, TERMINATE},
        //running can suspend, terminate
        {UNKNOWN, UNKNOWN, SUSPEND, TERMINATE},
        //suspend can restart, reconnect
        {STANDBY, RUNNING, UNKNOWN, TERMINATE},
        //nothing can be done to terminate
        {STANDBY, UNKNOWN, UNKNOWN, UNKNOWN}
    };
    int lifecyclePort = 0;
    std::string ip = "";
    struct timeval last_update;
    TaskHandle_t microphone_server_handle = NULL;

public:
    AbilityContext(const char *abilityNameArg, const char *devicesListArg, erpc::SimpleServer *server) {
        abilityName = abilityNameArg;
        devicesList = devicesListArg;
        gettimeofday((struct timeval *) &last_update, NULL);
        rpc_server = server;
        //convert to UNIX time_t
        
    }
    void setConnectIP(char* ipArg) {
        if(strncpy(connectIp, ipArg, 16) != connectIp) {
            ESP_LOGE(TAG, "setConnectIp failed");
        }
    }
    const char* getConnectIP() {
        return connectIp;
    }
    void setConnectPort(uint16_t portArg) {
        connectPort = portArg;
    }
    uint16_t getConnectPort() {
        return connectPort;
    }
    int getLifecyclePort() {
        return lifecyclePort;
    }
    const char *getAbilityName() {
        return abilityName;
    }
    bool check_cmd_legal(unsigned cmd) {
        assert(status < 5);
        assert(cmd < 4);
        return status_transfer[status][cmd] != UNKNOWN;
    }
    void do_cmd(unsigned cmd) {
        switch (cmd) {
        case START:
            start();
            break;
        case CONNECT:
            connect();
            break;
        case DISCONNECT:
            disconnect();
            break;
        case TERMINATE_CMD:
            terminate();
            break;
        default:
            ESP_LOGE(TAG, "cmd is illegal: %d", cmd);
            break;
        }
    }
    void start() {
        assert(status < 5);
        status = status_transfer[status][0];
        lifecyclePort = 1;
        gettimeofday((struct timeval *) &last_update, NULL);
    }
    void connect() {
        assert(status < 5);
        status = status_transfer[status][1];
        gettimeofday((struct timeval *) &last_update, NULL);
        // xTaskCreate(micronphone_task, "micronphone", 32768, NULL, 5, &microphone_server_handle);
        abilityPort = RPC_PORT;
        if (rpc_status::Success != rpc_server->open()) {
            // 任务创建失败
            ESP_LOGE(TAG, "RPC打开失败");
        }
        else {
            // 任务创建成功
            ESP_LOGI(TAG, "RPC打开成功");
        }
    }
    void disconnect() {
        assert(status < 5);
        status = status_transfer[status][2];

        if (rpc_status::Success != rpc_server->close()) {
            // 任务创建失败
            ESP_LOGE(TAG, "RPC关闭失败");
        }
        else {
            // 任务创建成功
            ESP_LOGI(TAG, "RPC关闭成功");
        }

        gettimeofday((struct timeval *) &last_update, NULL);
    }
    void terminate() {
        assert(status < 5);
        status = status_transfer[status][3];
        gettimeofday((struct timeval *) &last_update, NULL);
    }
    const char *getDevicesList() {
        return devicesList;
    }
    const char *getStatusString() {
        switch (status) {
        case 0:
            return "INIT";
        case 1:
            return "STANDBY";
        case 2:
            return "RUNNING";
        case 3:
            return "SUSPEND";
        case 4:
            return "TERMINATE";
        default:
            return "UNKNOWN";
        }
    }
    const char *getAbilityRunningState() {
        char *buf = (char *) malloc(1024);
        snprintf(buf, 1024, abilityRunningStateFormat, abilityName, abilityPort, last_update.tv_sec, lifecyclePort, getStatusString());
        return buf;
    }
    const char *getAbilitySupport() {
        char *buf = (char *) malloc(1024);
        snprintf(buf, 1024, abilitySupportFormat, devicesList, abilityName);
        return buf;
    }

};
extern AbilityContext *speakerContext;