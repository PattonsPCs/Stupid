cmake_minimum_required(VERSION 3.10)
project(Sutpid C)

set(CMAKE_C_STANDARD 11)

add_executable(Sutpid WIN32 
    main.c
    gui.c
    operations.c
)

target_link_libraries(Sutpid shell32)
