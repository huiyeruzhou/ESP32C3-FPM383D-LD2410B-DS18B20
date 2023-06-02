idf_component_register(SRCS 
                    "hello_handler.cpp"
                    "echo_handler.cpp"
                    #"ctrl_handler.cpp"
                    "api_Devices_handler.cpp"
                    "api_AbilityRunning_handler.cpp"
                    "api_AbilitySupport_handler.cpp"
                    "api_AbilityRequest_handler.cpp"
                    ##############"ability_context.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES wifi_config esp_http_server udp_broadcast myrpc)
