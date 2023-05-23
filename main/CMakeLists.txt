set(program wifi_static_ip)
file(GLOB_RECURSE SRCS
        ${program}/*.cpp
        ${program}/*.c
        *.cpp
        )
idf_component_register(SRCS ${SRCS}
                    INCLUDE_DIRS "."
                    REQUIRES sensors wifi_config myrpc)
