#include "sensor.pb.hpp"
#include <stdio.h>
#include <unistd.h>
#include <client/rpc_client.hpp>

int main(int argc, char **argv) {
    /* eRPC client side initialization */
    auto client = new sensor_SensorService_Client("192.168.120.1", 12345);
    auto Client = new erpc::Client("192.168.120.1", 12345);
    /* code */
    int32_t ret = 0;
    if (rpc_status::Success != client->open()) return -1;
    for (;;) {
        sensor_Empty empty;
        sensor_Status status;
        client->open(&empty, &status);
        printf("response: %" PRId16 "\n", status.status);
    }
    return 0;
}