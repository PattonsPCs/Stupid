cmake_minimum_required(VERSION 4.1)
project(Sutpid C)

set(CMAKE_C_STANDARD 11)

add_executable(Sutpid WIN32 main.c
        include/miniaudio.h
        operations.c
        include/operations.h
        include/gui.h
        gui.c
        include/globals.h)

target_link_libraries(Sutpid winmm)
