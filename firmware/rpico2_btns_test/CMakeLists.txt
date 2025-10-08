# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Asegurate de compilar en modo Debug
set(CMAKE_BUILD_TYPE Debug)

# Para forzar flags (opcional, por si usás otros toolchains):
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(rpico2_btns_test C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(rpico2_btns_test rpico2_btns_test.c )

pico_set_program_name(rpico2_btns_test "rpico2_btns_test")
pico_set_program_version(rpico2_btns_test "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(rpico2_btns_test 0)
pico_enable_stdio_usb(rpico2_btns_test 1)

# Add the standard library to the build
target_link_libraries(rpico2_btns_test
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(rpico2_btns_test PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(rpico2_btns_test)

