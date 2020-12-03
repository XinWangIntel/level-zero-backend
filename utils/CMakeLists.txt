# Copyright (C) 2019 Intel Corporation
# SPDX-License-Identifier: MIT

file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/harness/*.cpp
)

file(GLOB HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils/.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/test_harness/.hpp
)

add_library(lz_wrapper ${HEADERS} ${SOURCES})

target_link_libraries(lz_wrapper
    PUBLIC
    LevelZero::LevelZero
)
