set(TARGET "fdc_test")
file(GLOB CPP_SOURCES *.cpp)
add_executable(${TARGET} ${CPP_SOURCES})
target_link_libraries(${TARGET} PRIVATE fdc_bitstream)
