cmake_minimum_required(VERSION 3.10.0)
project(EFS VERSION 0.7.0 LANGUAGES C)

# Header file directory
include_directories(include)

# Source files
set(SOURCES
    main.c
    src/algos/bm.c
    src/arg_parser.c
    src/file_stream/dir_stream.c
    src/file_stream/file_stream.c
    src/file_stream/line_stream.c
    src/log.c
    src/search/file_search.c
    src/search/search_functions.c
)

# Header files
set(HEADERS
    include/algo.h
    include/arg_parser.h
    include/file_stream.h
    include/log.h
    include/search.h
)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

option(DEBUG_MODE "Enable debug mode" OFF)

if(DEBUG_MODE)
    add_compile_definitions(DEBUG)
endif()

add_executable(efs ${SOURCES} ${HEADERS})