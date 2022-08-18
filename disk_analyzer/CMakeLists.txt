set(TARGET "analyzer")
file(GLOB CPP_SOURCES *.cpp)
add_executable(${TARGET} ${CPP_SOURCES})
target_link_libraries(${TARGET} PUBLIC fdc_bitstream fdc_common disk_image)
