file(GLOB CPP_SOURCES *.cpp)
include_directories("${CMAKE_SOURCE_DIR}/include")
add_library(fdc_bitstream SHARED ${CPP_SOURCES})
