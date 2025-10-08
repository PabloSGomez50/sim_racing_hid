add_library(display
    display.c
    assets/Logo_Williams_F1_transparent.c
)

target_include_directories(display PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/..
)

target_link_libraries(display
    pico_stdlib
    hardware_spi
    ili9341
    lvgl
)
