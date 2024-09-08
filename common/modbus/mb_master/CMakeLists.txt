cmake_minimum_required(VERSION 3.22)

project(mb_master)

set(CMAKE_C_STANDARD 99)

add_library(mb_master STATIC
        sm_mb_master_if.h
        sm_mb_master_impl.c
        sm_mb_master_impl.h
        ../../../libs/nanoModbus/nanomodbus.h
        ../../../libs/nanoModbus/nanomodbus.c)
