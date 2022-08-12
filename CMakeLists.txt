cmake_minimum_required(VERSION 3.12)
project(fdc_bitstream)

include_directories("${CMAKE_SOURCE_DIR}/include")
link_directories("${CMAKE_SOURCE_DIR}/bin/debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_subdirectory(fdc_bitstream)
add_subdirectory(fdc_test)
add_subdirectory(hfe2mfm)
