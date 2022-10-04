cmake_minimum_required(VERSION 3.12)
project(fdc_bitstream)

#set(CMAKE_CXX_FLAGS "-std=c++14")

include_directories("${CMAKE_SOURCE_DIR}/include")
link_directories("${CMAKE_SOURCE_DIR}/bin/debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_subdirectory(fdc_bitstream)
add_subdirectory(disk_image)

add_subdirectory(fdc_test)
add_subdirectory(create_mfm_image)
add_subdirectory(image_converter)
add_subdirectory(disk_analyzer)

add_subdirectory(externals/lz4)
add_subdirectory(pauline2raw)

add_subdirectory(tests)
