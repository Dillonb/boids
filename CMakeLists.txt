cmake_minimum_required (VERSION 3.10)

ADD_COMPILE_OPTIONS(-Wall -march=native)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DCHIP8_DEBUG_MODE")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCHIP8_DEBUG_MODE")

project (boids)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")
find_package(SDL2 REQUIRED)
add_executable(boids main.c)
include_directories(${SDL2_INCLUDE_DIR})
target_link_libraries(boids ${SDL2_LIBRARY})