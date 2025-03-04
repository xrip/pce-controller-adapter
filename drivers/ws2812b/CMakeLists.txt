add_library(ws2812b INTERFACE)

target_link_libraries(ws2812b INTERFACE hardware_pio hardware_dma)

target_include_directories(ws2812b INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
