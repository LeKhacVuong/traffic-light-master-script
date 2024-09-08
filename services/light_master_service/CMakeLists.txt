cmake_minimum_required(VERSION 3.22)

project(light_master_sv)

set(CMAKE_C_STANDARD 99)

add_library(light_master_sv STATIC
        light_master_service.c
        light_master_service.h
        light_data.h)

target_link_libraries(light_master_sv mb_master sdk_utils)
