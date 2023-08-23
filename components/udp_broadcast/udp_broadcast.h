/*
 * Copyright 2023 YuHongli
 *
 * File: udp_broadcast.h
 * Description: the udp broadcast header
 * Version: V1.0.0
 * Date: 2023/08/23
 * Author: YuHongli
 * Revision History:
 *   Version       Date          Author         Revision Description
 *  V1.0.0        2023/08/23    YuHongli       Create and initialize
 */
#pragma once

#define HOST_IP_ADDR "255.255.255.255"
#define UDP_PORT 8888

#ifdef __cplusplus
extern "C" {
#endif

void create_broad_task(const char *arg);

#ifdef __cplusplus
}
#endif
