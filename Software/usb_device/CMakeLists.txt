# Set minimum required version
cmake_minimum_required(VERSION 3.13)

set(FAMILY rp2040)
set(BOARD pico_sdk)

# Import cmake
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

# Project setup
project(usb_device C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise SDK
pico_sdk_init()

# List of executables
add_executable(${PROJECT_NAME}
   main.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

target_sources(${PROJECT_NAME} PUBLIC
   main.c
   usb_descriptors.c)

# Generate programming files
pico_add_extra_outputs(${PROJECT_NAME})

# Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC
   pico_stdlib
   tinyusb_device
   tinyusb_board
)
