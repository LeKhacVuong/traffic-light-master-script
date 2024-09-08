cmake_minimum_required(VERSION 3.22)
project(lib-nanomodbus)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

add_library(nanomodbus STATIC
        nanomodbus.h
        nanomodbus.c)