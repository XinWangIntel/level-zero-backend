cmake_minimum_required(VERSION 2.8)
project(lz0_test)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_compile_options(-std=c++11)

#find_package(LevelZero)

include(FetchContent)
include(LevelZero)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils/include)

add_subdirectory(utils)

add_executable(test src/main.cpp)

target_link_libraries(test lz_wrapper)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
set(destination ${CMAKE_INSTALL_PREFIX})

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/kernels/copy_module.spv"
    DESTINATION ${destination}
)
