#include <stdio.h>
#include <unistd.h>

#include <client/rpc_client.hpp>

#include "sensor.pb.hpp"
__attribute__((unused)) static const char *TAG = "Sensors Client";
static sensor_SensorService_Client *client;
void read(int sensor)
{
    sensor_Empty empty;
    sensor_Value value;
    client->read(&empty, &value);

    if (value.status == 0) {
        LOGI(TAG, "Success");
    } else {
        LOGW(TAG, "Failed, status = %" PRId16, value.status);
    }

    switch (sensor) {
        case 0: /*DS18B20*/
            printf("temperature: %f\n", value.value);
            break;
        case 1: /*FPM_383D*/
            printf("finger id: %f\n", value.value);
            break;
        case 2: /*LD2410B*/
            printf("distance: %f\n", value.value);
            break;
    }
}
void open()
{
    sensor_Empty empty;
    client->open(&empty, &empty);
}
void close()
{
    sensor_Empty empty;
    client->close(&empty, &empty);
}
void configure(uint32_t config)
{
    sensor_Value value;
    sensor_Empty empty;
    value.value = (float)config;
    client->configure(&value, &empty);
}
int main(int argc, char **argv)
{
    /* RPC client side initialization */
    client = new sensor_SensorService_Client("192.168.50.131", 12345);
    int sensor = 1;
    if (rpc_status::Success != client->open()) {
        return -1;
    }
    // LOGE(TAG, "try to read when sensor is closed");
    // read(sensor);
    LOGE(TAG, "open the sensor");
    open();
    for (int i = 0; i < 3; i++) {
        LOGE(TAG, "read the sensor for %d time", i);
        read(sensor);
        LOGE(TAG, "confgiure the sensor");
        configure(100);
        sleep(1);
    }
    LOGE(TAG, "close the sensor");
    close();
    LOGE(TAG, "read the sensor");
    read(sensor);
    return 0;
}
