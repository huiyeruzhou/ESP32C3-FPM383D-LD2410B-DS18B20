#pragma once

#define HOST_IP_ADDR "255.255.255.255"
#define UDP_PORT 8888

#ifdef __cplusplus
extern "C"{
#endif

void create_broad_task(const char *arg);

#ifdef __cplusplus
}
#endif