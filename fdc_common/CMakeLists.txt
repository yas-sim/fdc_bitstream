set(TARGET "fdc_common")
file(GLOB CPP_SOURCES *.cpp)
include_directories("${CMAKE_SOURCE_DIR}/include")
add_library(${TARGET} SHARED ${CPP_SOURCES})
