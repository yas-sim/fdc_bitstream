add_library(testutil util.cpp)
target_include_directories(testutil PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(testutil disk_image)



add_executable(testRaw2d77 raw2d77.cpp)
target_link_libraries(testRaw2d77 disk_image testutil)

add_test(
	NAME TEST_RAW_4MHz_TO_D77
	COMMAND
		testRaw2d77
		"${CMAKE_SOURCE_DIR}/test_data/2019FM77AVDemo-4MHz.raw"
		"${CMAKE_SOURCE_DIR}/test_data/2019FM77AVDemo.d77"
		"${CMAKE_BINARY_DIR}"
)
add_test(
	NAME TEST_RAW_8MHz_TO_D77
	COMMAND
		testRaw2d77
		"${CMAKE_SOURCE_DIR}/test_data/2019FM77AVDemo-8MHz.raw"
		"${CMAKE_SOURCE_DIR}/test_data/2019FM77AVDemo.d77"
		"${CMAKE_BINARY_DIR}"
)



add_executable(testFormat format.cpp)
target_link_libraries(testFormat disk_image testutil)
add_test(
	NAME TEST_FORMAT
	COMMAND
		testFormat
)



add_executable(testCompare compare.cpp)
target_link_libraries(testCompare disk_image testutil)

