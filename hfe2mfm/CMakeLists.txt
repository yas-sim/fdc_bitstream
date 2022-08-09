set(TARGET "hfe2mfm")
project(${TARGET})

file(GLOB CPP_SOURCES *.cpp)
add_executable(${TARGET} ${CPP_SOURCES})
target_link_libraries(${TARGET} fdc_bitstream)
