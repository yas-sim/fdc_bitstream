set(TEST_IMAGE_DIR ${CMAKE_SOURCE_DIR}/../fdc_bitstream_test)


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

if(EXISTS ${TEST_IMAGE_DIR}/000000.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000000.d77)
	add_test(
		NAME COMPARE_000000
		COMMAND testCompare ${TEST_IMAGE_DIR}/000000.d77 ${TEST_IMAGE_DIR}/000000.raw
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000001.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000001.d77)
	add_test(
		NAME COMPARE_000001
		COMMAND testCompare ${TEST_IMAGE_DIR}/000001.d77 ${TEST_IMAGE_DIR}/000001.raw
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000002.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000002.d77)
	add_test(
		NAME COMPARE_000002
		COMMAND testCompare ${TEST_IMAGE_DIR}/000002.d77 ${TEST_IMAGE_DIR}/000002.raw
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000003.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000003.d77)
	add_test(
		NAME COMPARE_000003
		COMMAND testCompare ${TEST_IMAGE_DIR}/000003.d77 ${TEST_IMAGE_DIR}/000003.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000004.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000004.d77)
	add_test(
		NAME COMPARE_000004
		COMMAND testCompare ${TEST_IMAGE_DIR}/000004.d77 ${TEST_IMAGE_DIR}/000004.raw
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000005.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000005.d77)
	add_test(
		NAME COMPARE_000005
		COMMAND testCompare ${TEST_IMAGE_DIR}/000005.d77 ${TEST_IMAGE_DIR}/000005.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000006.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000006.d77)
	add_test(
		NAME COMPARE_000006
		COMMAND testCompare ${TEST_IMAGE_DIR}/000006.d77 ${TEST_IMAGE_DIR}/000006.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000007.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000007.d77)
	add_test(
		NAME COMPARE_000007
		COMMAND testCompare ${TEST_IMAGE_DIR}/000007.d77 ${TEST_IMAGE_DIR}/000007.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000008.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000008.d77)
	add_test(
		NAME COMPARE_000008
		COMMAND testCompare ${TEST_IMAGE_DIR}/000008.d77 ${TEST_IMAGE_DIR}/000008.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000009.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000009.d77)
	add_test(
		NAME COMPARE_000009
		COMMAND testCompare ${TEST_IMAGE_DIR}/000009.d77 ${TEST_IMAGE_DIR}/000009.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000010.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000010.d77)
	add_test(
		NAME COMPARE_000010
		COMMAND testCompare ${TEST_IMAGE_DIR}/000010.d77 ${TEST_IMAGE_DIR}/000010.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000011.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000011.d77)
	add_test(
		NAME COMPARE_000011
		COMMAND testCompare ${TEST_IMAGE_DIR}/000011.d77 ${TEST_IMAGE_DIR}/000011.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000012.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000012.d77)
	add_test(
		NAME COMPARE_000012
		COMMAND testCompare ${TEST_IMAGE_DIR}/000012.d77 ${TEST_IMAGE_DIR}/000012.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000013.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000013.d77)
	add_test(
		NAME COMPARE_000013
		COMMAND testCompare ${TEST_IMAGE_DIR}/000013.d77 ${TEST_IMAGE_DIR}/000013.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000014.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000014.d77)
	add_test(
		NAME COMPARE_000014
		COMMAND testCompare ${TEST_IMAGE_DIR}/000014.d77 ${TEST_IMAGE_DIR}/000014.raw -ignore_crc -exclude_track 3
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000015.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000015.d77)
	add_test(
		NAME COMPARE_000015
		COMMAND testCompare ${TEST_IMAGE_DIR}/000015.d77 ${TEST_IMAGE_DIR}/000015.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000016.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000016.d77)
	add_test(
		NAME COMPARE_000016
		COMMAND testCompare ${TEST_IMAGE_DIR}/000016.d77 ${TEST_IMAGE_DIR}/000016.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000017.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000017.d77)
	add_test(
		NAME COMPARE_000017
		COMMAND testCompare ${TEST_IMAGE_DIR}/000017.d77 ${TEST_IMAGE_DIR}/000017.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000018.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000018.d77)
	add_test(
		NAME COMPARE_000018
		COMMAND testCompare ${TEST_IMAGE_DIR}/000018.d77 ${TEST_IMAGE_DIR}/000018.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000019.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000019.d77)
	add_test(
		NAME COMPARE_000019
		COMMAND testCompare ${TEST_IMAGE_DIR}/000019.d77 ${TEST_IMAGE_DIR}/000019.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000020.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000020.d77)
	add_test(
		NAME COMPARE_000020
		COMMAND testCompare ${TEST_IMAGE_DIR}/000020.d77 ${TEST_IMAGE_DIR}/000020.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000021.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000021.d77)
	add_test(
		NAME COMPARE_000021
		COMMAND testCompare ${TEST_IMAGE_DIR}/000021.d77 ${TEST_IMAGE_DIR}/000021.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000022.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000022.d77)
	add_test(
		NAME COMPARE_000022
		COMMAND testCompare ${TEST_IMAGE_DIR}/000022.d77 ${TEST_IMAGE_DIR}/000022.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000023.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000023.d77)
	add_test(
		NAME COMPARE_000023
		COMMAND testCompare ${TEST_IMAGE_DIR}/000023.d77 ${TEST_IMAGE_DIR}/000023.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000024.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000024.d77)
	add_test(
		NAME COMPARE_000024
		COMMAND testCompare ${TEST_IMAGE_DIR}/000024.d77 ${TEST_IMAGE_DIR}/000024.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
