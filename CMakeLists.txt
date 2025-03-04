cmake_minimum_required(VERSION 3.21)

# Pull in SDK (must be before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pce-adapter CXX C ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin/${PICO_PLATFORM}/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

# Initialize the SDK
pico_sdk_init()

add_executable(${PROJECT_NAME} src/main.c)

add_compile_options(-flto -fwhole-program -ffunction-sections -fdata-sections -O3)

add_subdirectory(drivers/nespad)
add_subdirectory(drivers/genesispad)
add_subdirectory(drivers/ws2812b)

target_link_libraries(${PROJECT_NAME} PRIVATE
        nespad
        genesispad
        ws2812b

        pico_multicore
        pico_runtime
        pico_stdio
)



target_compile_definitions(${PROJECT_NAME} PRIVATE PICO_PANIC_FUNCTION=)

pico_set_binary_type(${PROJECT_NAME} copy_to_ram)
set(PICO_COPY_TO_RAM 1)
pico_enable_stdio_usb(${PROJECT_NAME} 1)
target_link_options(${PROJECT_NAME} PRIVATE -Xlinker --print-memory-usage --data-sections --function-sections)
pico_add_extra_outputs(${PROJECT_NAME})