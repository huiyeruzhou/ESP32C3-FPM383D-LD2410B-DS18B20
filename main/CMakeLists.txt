set(program FPM_383D)
file(GLOB_RECURSE SRCS
        ${program}/*.cpp
        ${program}/*.c
        )
idf_component_register(SRCS ${SRCS}
                    INCLUDE_DIRS "."
                    REQUIRES sensors)
