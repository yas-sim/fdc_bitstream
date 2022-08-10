cmake_minimum_required(VERSION 3.21)
project(fdc_bitstream)

include_directories("${CMAKE_SOURCE_DIR}/include")

add_subdirectory(fdc_bitstream)
add_subdirectory(fdc_test)
add_subdirectory(hfe2mfm)
