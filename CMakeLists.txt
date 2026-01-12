cmake_minimum_required(VERSION 3.10)
project(Sutpid C)

set(CMAKE_C_STANDARD 11)

# Force 64-bit architecture for compatibility with modern Windows systems
# This ensures the executable runs on 64-bit Windows (most common)
message(STATUS "Configuring for 64-bit Windows build")

# Explicitly set for MinGW/GCC (common on Windows)
if(MINGW OR CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64")
    message(STATUS "MinGW/GCC detected - forcing 64-bit build with -m64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32-bit detected, try to force 64-bit if compiler supports it
    message(STATUS "32-bit compiler detected - attempting 64-bit build")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64")
endif()

# For MSVC, set architecture
if(MSVC)
    set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "" FORCE)
    message(STATUS "MSVC detected - setting x64 platform")
endif()

# For MSVC, set architecture
if(MSVC)
    set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "" FORCE)
    message(STATUS "MSVC detected - setting x64 platform")
endif()

add_executable(Sutpid WIN32 
    main.c
    gui.c
    operations.c
    dependencies.c
)

target_link_libraries(Sutpid shell32 user32 gdi32)
