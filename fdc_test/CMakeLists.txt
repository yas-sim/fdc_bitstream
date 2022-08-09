set(TARGET "fdc_test")
project(${TARGET})

file(GLOB CPP_SOURCES *.cpp)
add_executable(${TARGET} ${CPP_SOURCES})
target_link_libraries(${TARGET} fdc_bitstream)
