/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.8-dev */

#ifndef PB_SENSOR_SENSOR_PB_HPP_INCLUDED
#define PB_SENSOR_SENSOR_PB_HPP_INCLUDED
#include <pb.h>
#include <server/service.hpp>
#include <client/rpc_client.hpp>
#include <rpc_status.hpp>
#include <pb_encode.h>
#include <pb_decode.h>
#include <functional>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _sensor_Empty {
    char dummy_field;
} sensor_Empty;

typedef struct _sensor_Status {
    bool status;
} sensor_Status;

typedef struct _sensor_Value {
    uint32_t status;
    float value;
} sensor_Value;

typedef struct _sensor_Configure {
    float configure;
} sensor_Configure;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define sensor_Empty_init_default                {0}
#define sensor_Status_init_default               {0}
#define sensor_Value_init_default                {0, 0}
#define sensor_Configure_init_default            {0}
#define sensor_Empty_init_zero                   {0}
#define sensor_Status_init_zero                  {0}
#define sensor_Value_init_zero                   {0, 0}
#define sensor_Configure_init_zero               {0}

/* Field tags (for use in manual encoding/decoding) */
#define sensor_Status_status_tag                 1
#define sensor_Value_status_tag                  1
#define sensor_Value_value_tag                   2
#define sensor_Configure_configure_tag           1

/* Struct field encoding specification for nanopb */
#define sensor_Empty_FIELDLIST(X, a) \

#define sensor_Empty_CALLBACK NULL
#define sensor_Empty_DEFAULT NULL

#define sensor_Status_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, BOOL,     status,            1)
#define sensor_Status_CALLBACK NULL
#define sensor_Status_DEFAULT NULL

#define sensor_Value_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, UINT32,   status,            1) \
X(a, STATIC,   REQUIRED, FLOAT,    value,             2)
#define sensor_Value_CALLBACK NULL
#define sensor_Value_DEFAULT NULL

#define sensor_Configure_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, FLOAT,    configure,         1)
#define sensor_Configure_CALLBACK NULL
#define sensor_Configure_DEFAULT NULL

extern const pb_msgdesc_t sensor_Empty_msg;
extern const pb_msgdesc_t sensor_Status_msg;
extern const pb_msgdesc_t sensor_Value_msg;
extern const pb_msgdesc_t sensor_Configure_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define sensor_Empty_fields &sensor_Empty_msg
#define sensor_Status_fields &sensor_Status_msg
#define sensor_Value_fields &sensor_Value_msg
#define sensor_Configure_fields &sensor_Configure_msg

/* Maximum encoded size of messages (where known) */
#define sensor_Configure_size                    5
#define sensor_Empty_size                        0
#define sensor_Status_size                       2
#define sensor_Value_size                        11

/* Service Definations */
class sensor_SensorService_Service : public erpc::Service {
public:
    sensor_SensorService_Service();
    virtual ~sensor_SensorService_Service() {}
	using erpc::Service::Service;
	using erpc::Service::addMethod;
    virtual rpc_status open(sensor_Empty *request, sensor_Empty *response);
    virtual rpc_status close(sensor_Empty *request, sensor_Empty *response);
    virtual rpc_status read(sensor_Empty *request, sensor_Value *response);
    virtual rpc_status configure(sensor_Value *request, sensor_Empty *response);
};
class sensor_UpdateService_Service : public erpc::Service {
public:
    sensor_UpdateService_Service();
    virtual ~sensor_UpdateService_Service() {}
	using erpc::Service::Service;
	using erpc::Service::addMethod;
    virtual rpc_status update(sensor_Value *request, sensor_Empty *response);
};
class sensor_ControlDeviceService_Service : public erpc::Service {
public:
    sensor_ControlDeviceService_Service();
    virtual ~sensor_ControlDeviceService_Service() {}
	using erpc::Service::Service;
	using erpc::Service::addMethod;
    virtual rpc_status open(sensor_Empty *request, sensor_Empty *response);
    virtual rpc_status close(sensor_Empty *request, sensor_Empty *response);
};

/* Client Defination */
class sensor_SensorService_Client : public erpc::Client {
public:
    sensor_SensorService_Client(const char *host, uint16_t port): erpc::Client(host, port) {}
    virtual ~sensor_SensorService_Client() {}
	using erpc::Client::open;
	using erpc::Client::Client;
    virtual rpc_status open(sensor_Empty *request, sensor_Empty *response);
    virtual rpc_status close(sensor_Empty *request, sensor_Empty *response);
    virtual rpc_status read(sensor_Empty *request, sensor_Value *response);
    virtual rpc_status configure(sensor_Value *request, sensor_Empty *response);
};
class sensor_UpdateService_Client : public erpc::Client {
public:
    sensor_UpdateService_Client(const char *host, uint16_t port): erpc::Client(host, port) {}
    virtual ~sensor_UpdateService_Client() {}
	using erpc::Client::open;
	using erpc::Client::Client;
    virtual rpc_status update(sensor_Value *request, sensor_Empty *response);
};
class sensor_ControlDeviceService_Client : public erpc::Client {
public:
    sensor_ControlDeviceService_Client(const char *host, uint16_t port): erpc::Client(host, port) {}
    virtual ~sensor_ControlDeviceService_Client() {}
	using erpc::Client::open;
	using erpc::Client::Client;
    virtual rpc_status open(sensor_Empty *request, sensor_Empty *response);
    virtual rpc_status close(sensor_Empty *request, sensor_Empty *response);
};
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
