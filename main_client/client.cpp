#include "sensor.pb.hpp"
#include <stdio.h>
#include <unistd.h>
#include <client/rpc_client.hpp>

int main(int argc, char **argv) {
    // system("sudo ifconfig eth0 192.168.0.100 netmask 255.255.255.0");
    /* eRPC client side initialization */
    auto client = new sensor_SensorService_Client("192.168.93.194", 12345);
    /* code */
    int32_t ret = 1;
    if (rpc_status::Success != client->open()) return -1;
    sensor_Empty empty;
    sensor_Status status;
    sensor_Value value;
    int sensor = 1;
    client->open(&empty, &empty);
    printf("response: %" PRId16 "\n", status.status);
    for (;;) {
        client->read(&empty, &value);
        switch (sensor) {
        case 0:/*DS18B20*/
            printf("temperature: %f\n", value.value);
            break;
        case 1:/*FPM_383D*/
            printf("finger id: %f\n", value.value);
            break;
        case 2:/*LD2410B*/
            printf("distance: %f\n", value.value);
            break;
        }
        sleep(1);
    }
    return 0;
}