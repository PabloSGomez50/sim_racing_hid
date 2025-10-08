add_library(as5600
	as5600.c
)

target_include_directories(as5600 PUBLIC
	.
)

target_link_libraries(as5600 pico_stdlib hardware_i2c)