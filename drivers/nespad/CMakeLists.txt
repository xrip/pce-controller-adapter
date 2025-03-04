add_library(nespad INTERFACE)

target_link_libraries(nespad INTERFACE hardware_pio)

target_include_directories(nespad INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
