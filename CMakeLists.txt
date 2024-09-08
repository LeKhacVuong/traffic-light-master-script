cmake_minimum_required(VERSION 3.29)
project(light_master)

set(CMAKE_CXX_STANDARD 20)

include_directories(utils
        utils/algorithms
        utils/timing
        utils/unix
        common/modbus
        common/modbus/mb_slave
        common/modbus/mb_master
        libs/nanoModbus
        services/light_master_service
)

add_executable(mainapp main.c
        services/light_master_service/light_master_service.h)

target_link_libraries(mainapp sdk_utils light_master_sv)

add_subdirectory(utils)
add_subdirectory(common/modbus/mb_slave)
add_subdirectory(common/modbus/mb_master)
add_subdirectory(libs/nanoModbus)
add_subdirectory(services/light_master_service)
