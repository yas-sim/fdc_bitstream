add_executable(testRaw2d77 raw2d77.cpp)
target_link_libraries(testRaw2d77 disk_image)

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
