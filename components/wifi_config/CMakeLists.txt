file(GLOB_RECURSE RPC_SRC
        *.c
        *.cpp
        )
set(RPC_INCLUDE .)

idf_component_register(SRCS  "${RPC_SRC}"
        INCLUDE_DIRS "${RPC_INCLUDE}"
        REQUIRES esp_wifi nvs_flash)
