add_library(picoRGB
    picoRGB.c
)

pico_generate_pio_header(picoRGB ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

target_include_directories(picoRGB PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(picoRGB
    pico_stdlib
    hardware_pio
)
