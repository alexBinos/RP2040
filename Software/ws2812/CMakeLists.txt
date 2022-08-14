# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

# Set project name and C/C++ standards
project(ws2812 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# SDK libraries
pico_sdk_init()

add_executable(${PROJECT_NAME}
   ws2812.c
)

pico_generate_pio_header(${PROJECT_NAME}
   ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
)

# Create program files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib
target_link_libraries(${PROJECT_NAME}
   pico_stdlib
   hardware_pio
)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)