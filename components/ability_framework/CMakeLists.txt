idf_component_register(SRCS 
                    "api_Devices_handler.cpp"
                    "api_AbilityRunning_handler.cpp"
                    "api_AbilitySupport_handler.cpp"
                    "api_AbilityRequest_handler.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES wifi_config esp_http_server udp_broadcast myrpc)
