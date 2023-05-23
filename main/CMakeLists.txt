set(program LD2410B)
file(GLOB_RECURSE SRCS
        ${program}/*.cpp
        ${program}/*.c
        *.pb.cpp
        )
idf_component_register(SRCS ${SRCS}
                    INCLUDE_DIRS "."
                    REQUIRES sensors wifi_config myrpc)
