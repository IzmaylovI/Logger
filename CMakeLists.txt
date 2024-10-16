cmake_minimum_required(VERSION 3.10)
project(ExampleProject)


set(sources ${CMAKE_CURRENT_SOURCE_DIR}/src/close_lib.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/open_lib.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/lseek_lib.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/realloc_lib.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/free_lib.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/malloc_lib.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/read_lib.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/write_lib.c
	)
add_library(log SHARED ${sources})

