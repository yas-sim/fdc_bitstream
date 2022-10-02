add_library(lz4
lz4.c
lz4.h
lz4file.c
lz4file.h
lz4frame.c
lz4frame.h
lz4frame_static.h
lz4hc.c
lz4hc.h
Makefile
README.md
xxhash.c
xxhash.h)
target_include_directories(lz4 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
