set(TARGET "image_converter")
file(GLOB CPP_SOURCES *.cpp)

if(USE_OCV)
  find_package(OpenCV REQUIRED)
  include_directories(${OpenCV_INCLUDE_DIRS})
endif(USE_OCV)

add_executable(${TARGET} ${CPP_SOURCES})
target_link_libraries(${TARGET} PRIVATE fdc_bitstream disk_image ${OpenCV_LIBS})
