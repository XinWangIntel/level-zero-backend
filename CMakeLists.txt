cmake_minimum_required(VERSION 3.14)
project(lz_test)

#set(CMAKE_C_COMPILER "/usr/bin/clang-10")
#set(CMAKE_CXX_COMPILER "/usr/bin/clang++-10")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_compile_options(-std=c++17)

option(ENABLE_LOCAL_LEVELZERO "Enabel local installed LevelZero" ON)

if(ENABLE_LOCAL_LEVELZERO)
    find_package(LevelZero)
    add_compile_definitions(USE_LOCAL_LEVEL_ZERO)
else()
    # In this case, change <level_zero/ze_api.h> to <ze_api.h>
    include(FetchContent)
    include(LevelZero)
    include_directories(SYSTEM ${levelzero_SOURCE_DIR}/include)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils/include)

add_subdirectory(utils)

add_executable(test src/main.cpp)

target_link_libraries(test lz_wrapper)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kernels/spirv_0 ${CMAKE_CURRENT_BINARY_DIR}/spirv_0 COPYONLY)

#set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
#set(destination ${CMAKE_INSTALL_PREFIX})

#install(
#    FILES "${CMAKE_CURRENT_SOURCE_DIR}/kernels/spirv_0"
#    DESTINATION ${destination}
#)
