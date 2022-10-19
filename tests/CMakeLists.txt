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
if(EXISTS ${TEST_IMAGE_DIR}/000025.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000025.d77)
	add_test(
		NAME COMPARE_000025
		COMMAND testCompare ${TEST_IMAGE_DIR}/000025.d77 ${TEST_IMAGE_DIR}/000025.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000026.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000026.d77)
	add_test(
		NAME COMPARE_000026
		COMMAND testCompare ${TEST_IMAGE_DIR}/000026.d77 ${TEST_IMAGE_DIR}/000026.raw -ignore_crc -exclude_track 1
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
	  # Temporarily exclude track 3.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000027.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000027.d77)
	add_test(
		NAME COMPARE_000027
		COMMAND testCompare ${TEST_IMAGE_DIR}/000027.d77 ${TEST_IMAGE_DIR}/000027.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000028.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000028.d77)
	add_test(
		NAME COMPARE_000028
		COMMAND testCompare ${TEST_IMAGE_DIR}/000028.d77 ${TEST_IMAGE_DIR}/000028.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000029.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000029.d77)
	add_test(
		NAME COMPARE_000029
		COMMAND testCompare ${TEST_IMAGE_DIR}/000029.d77 ${TEST_IMAGE_DIR}/000029.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000030.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000030.d77)
	add_test(
		NAME COMPARE_000030
		COMMAND testCompare ${TEST_IMAGE_DIR}/000030.d77 ${TEST_IMAGE_DIR}/000030.raw -ignore_crc
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000031.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000031.d77)
	add_test(
		NAME COMPARE_000031
		COMMAND testCompare ${TEST_IMAGE_DIR}/000031.d77 ${TEST_IMAGE_DIR}/000031.raw -ignore_crc -track_limit 80
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000032.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000032.d77)
	add_test(
		NAME COMPARE_000032
		COMMAND testCompare ${TEST_IMAGE_DIR}/000032.d77 ${TEST_IMAGE_DIR}/000032.raw -ignore_crc -track_limit 80
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000033.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000033.d77)
	add_test(
		NAME COMPARE_000033
		COMMAND testCompare ${TEST_IMAGE_DIR}/000033.d77 ${TEST_IMAGE_DIR}/000033.raw -ignore_crc -track_limit 80
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000034.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000034.d77)
	add_test(
		NAME COMPARE_000034
		COMMAND testCompare ${TEST_IMAGE_DIR}/000034.d77 ${TEST_IMAGE_DIR}/000034.raw -ignore_crc -track_limit 80
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000035.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000035.d77)
	add_test(
		NAME COMPARE_000035
		COMMAND testCompare ${TEST_IMAGE_DIR}/000035.d77 ${TEST_IMAGE_DIR}/000035.raw -ignore_crc -track_limit 80
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000036.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000036.d77)
	add_test(
		NAME COMPARE_000036
		COMMAND testCompare ${TEST_IMAGE_DIR}/000036.d77 ${TEST_IMAGE_DIR}/000036.raw -ignore_crc -track_limit 80
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000037.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000037.d77)
	add_test(
		NAME COMPARE_000037
		COMMAND testCompare ${TEST_IMAGE_DIR}/000037.d77 ${TEST_IMAGE_DIR}/000037.raw -ignore_crc -track_limit 80
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000038.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000038.d77)
	add_test(
		NAME COMPARE_000038
		COMMAND testCompare ${TEST_IMAGE_DIR}/000038.d77 ${TEST_IMAGE_DIR}/000038.raw -ignore_crc -track_limit 80
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000039.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000039.d77)
	add_test(
		NAME COMPARE_000039
		COMMAND testCompare ${TEST_IMAGE_DIR}/000039.d77 ${TEST_IMAGE_DIR}/000039.raw -ignore_crc -track_limit 80
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000040.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000040.d77)
	add_test(
		NAME COMPARE_000040
		COMMAND testCompare ${TEST_IMAGE_DIR}/000040.d77 ${TEST_IMAGE_DIR}/000040.raw -ignore_crc -track_limit 80
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000041.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000041.d77)
	add_test(
		NAME COMPARE_000041
		COMMAND testCompare ${TEST_IMAGE_DIR}/000041.d77 ${TEST_IMAGE_DIR}/000041.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000042.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000042.d77)
	add_test(
		NAME COMPARE_000042
		COMMAND testCompare ${TEST_IMAGE_DIR}/000042.d77 ${TEST_IMAGE_DIR}/000042.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000043.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000043.d77)
	add_test(
		NAME COMPARE_000043
		COMMAND testCompare ${TEST_IMAGE_DIR}/000043.d77 ${TEST_IMAGE_DIR}/000043.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000044.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000044.d77)
	add_test(
		NAME COMPARE_000044
		COMMAND testCompare ${TEST_IMAGE_DIR}/000044.d77 ${TEST_IMAGE_DIR}/000044.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000045.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000045.d77)
	add_test(
		NAME COMPARE_000045
		COMMAND testCompare ${TEST_IMAGE_DIR}/000045.d77 ${TEST_IMAGE_DIR}/000045.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000046.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000046.d77)
	add_test(
		NAME COMPARE_000046
		COMMAND testCompare ${TEST_IMAGE_DIR}/000046.d77 ${TEST_IMAGE_DIR}/000046.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000047.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000047.d77)
	add_test(
		NAME COMPARE_000047
		COMMAND testCompare ${TEST_IMAGE_DIR}/000047.d77 ${TEST_IMAGE_DIR}/000047.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000048.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000048.d77)
	add_test(
		NAME COMPARE_000048
		COMMAND testCompare ${TEST_IMAGE_DIR}/000048.d77 ${TEST_IMAGE_DIR}/000048.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000049.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000049.d77)
	add_test(
		NAME COMPARE_000049
		COMMAND testCompare ${TEST_IMAGE_DIR}/000049.d77 ${TEST_IMAGE_DIR}/000049.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000050.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000050.d77)
	add_test(
		NAME COMPARE_000050
		COMMAND testCompare ${TEST_IMAGE_DIR}/000050.d77 ${TEST_IMAGE_DIR}/000050.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000051.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000051.d77)
	add_test(
		NAME COMPARE_000051
		COMMAND testCompare ${TEST_IMAGE_DIR}/000051.d77 ${TEST_IMAGE_DIR}/000051.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000053.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000053.d77)
	add_test(
		NAME COMPARE_000053
		COMMAND testCompare ${TEST_IMAGE_DIR}/000053.d77 ${TEST_IMAGE_DIR}/000053.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000054.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000054.d77)
	add_test(
		NAME COMPARE_000054
		COMMAND testCompare ${TEST_IMAGE_DIR}/000054.d77 ${TEST_IMAGE_DIR}/000054.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000055.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000055.d77)
	add_test(
		NAME COMPARE_000055
		COMMAND testCompare ${TEST_IMAGE_DIR}/000055.d77 ${TEST_IMAGE_DIR}/000055.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000056.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000056.d77)
	add_test(
		NAME COMPARE_000056
		COMMAND testCompare ${TEST_IMAGE_DIR}/000056.d77 ${TEST_IMAGE_DIR}/000056.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000057.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000057.d77)
	add_test(
		NAME COMPARE_000057
		COMMAND testCompare ${TEST_IMAGE_DIR}/000057.d77 ${TEST_IMAGE_DIR}/000057.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000058.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000058.d77)
	add_test(
		NAME COMPARE_000058
		COMMAND testCompare ${TEST_IMAGE_DIR}/000058.d77 ${TEST_IMAGE_DIR}/000058.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000059.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000059.d77)
	add_test(
		NAME COMPARE_000059
		COMMAND testCompare ${TEST_IMAGE_DIR}/000059.d77 ${TEST_IMAGE_DIR}/000059.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000060.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000060.d77)
	add_test(
		NAME COMPARE_000060
		COMMAND testCompare ${TEST_IMAGE_DIR}/000060.d77 ${TEST_IMAGE_DIR}/000060.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
if(EXISTS ${TEST_IMAGE_DIR}/000061.raw AND
   EXISTS ${TEST_IMAGE_DIR}/000061.d77)
	add_test(
		NAME COMPARE_000061
		COMMAND testCompare ${TEST_IMAGE_DIR}/000061.d77 ${TEST_IMAGE_DIR}/000061.raw -track_limit 154
	) # Memo-to-myself:  Put D77 first.  RAW may have extra broken sector from overlap read.
endif()
