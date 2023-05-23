#include "sensor.pb.hpp"
#include <stdio.h>
#include <unistd.h>
#include <client/rpc_client.hpp>

int main(int argc, char **argv) {
    // system("sudo ifconfig eth0 192.168.0.100 netmask 255.255.255.0");
    /* eRPC client side initialization */
    auto client = new sensor_SensorService_Client("192.168.93.194", 12345);
    /* code */
    int32_t ret = 0;
    if (rpc_status::Success != client->open()) return -1;
    sensor_Empty empty;
    sensor_Status status;
    sensor_Value value;
    client->open(&empty, &empty);
    printf("response: %" PRId16 "\n", status.status);
    for (;;) {
        client->read(&empty, &value);
        printf("distance: %f\n", value.value);
        sleep(1);
    }
    return 0;
}