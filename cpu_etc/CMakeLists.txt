cmake_minimum_required (VERSION 2.8)
include_directories(../src/)

add_executable (compare_ppm ../src/compare_ppm.cpp)

SET(CMAKE_CXX_FLAGS "-std=c++0x")
link_directories(/usr/local/cuda/lib64)
add_executable (compress_cpu ../src/compress.cpp texture_compress.cpp)
target_link_libraries (compress_cpu OpenCL)
