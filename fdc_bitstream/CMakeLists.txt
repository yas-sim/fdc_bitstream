set(TARGET "fdc_bitstream")
file(GLOB CPP_SOURCES *.cpp)
include_directories("${CMAKE_SOURCE_DIR}/include")
add_library(${TARGET} STATIC ${CPP_SOURCES})
