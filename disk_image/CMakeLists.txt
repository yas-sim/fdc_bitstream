set(TARGET "disk_image")
file(GLOB CPP_SOURCES *.cpp)
include_directories("${CMAKE_SOURCE_DIR}/include")
add_library(${TARGET} STATIC ${CPP_SOURCES})
target_link_libraries(${TARGET} PUBLIC fdc_bitstream)
