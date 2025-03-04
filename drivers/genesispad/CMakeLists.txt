add_library(genesispad INTERFACE)

target_link_libraries(genesispad INTERFACE hardware_gpio)

target_include_directories(genesispad INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
