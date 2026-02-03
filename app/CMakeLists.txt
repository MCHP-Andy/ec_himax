# Copyright (c) 2025 Microchip Technology Inc.
# SPDX-License-Identifier: Apache-2.0
 
cmake_minimum_required(VERSION 3.20.0)

# list(APPEND EXTRA_ZEPHYR_MODULES
#   ${CMAKE_CURRENT_SOURCE_DIR}/../../modules
# )

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(himax_demo)

# FILE(GLOB app_sources src/*.c)
# target_sources(app PRIVATE ${app_sources})

include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(app PRIVATE src/main.c)
# target_sources(app PRIVATE src/blinkly.c)
# target_sources(app PRIVATE src/i3c_host.c)
# target_sources(app PRIVATE src/i3c_target.c)
target_sources(app PRIVATE src/i3c_fake_target.c)

add_subdirectory(driver)
add_subdirectory(service)
