set(program DS18B20)
file(GLOB_RECURSE SRCS
        ${program}/*.cpp
        ${program}/*.c
        *.pb.cpp
        )
idf_component_register(SRCS ${SRCS}
                    INCLUDE_DIRS "."
                    REQUIRES sensors wifi_config myrpc lwip udp_broadcast ability_framework esp_http_server esp_system driver)
        