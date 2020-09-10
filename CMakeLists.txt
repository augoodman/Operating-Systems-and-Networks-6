cmake_minimum_required(VERSION 3.17)
project(Module6 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Module6
        BaseFilters.c
        BmpProcessor.h
        PixelProcessor.h
        GoodmanFilters.c)
