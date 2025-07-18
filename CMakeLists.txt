cmake_minimum_required(VERSION 3.10.0)
project(EFS VERSION 0.3.0 LANGUAGES C)

# Header file directory
include_directories(include)

# Source files
set(SOURCES
    main.c
    src/algos/bmh.c
    src/arg_parser.c
    src/file.c
    src/file_stream/file_stream.c
    src/file_stream/line_stream.c
    src/log.c
    src/search/file_search.c
)

# Header files
set(HEADERS
    include/algo.h
    include/arg_parser.h
    include/file.h
    include/file_stream.h
    include/log.h
    include/search.h
)

add_compile_definitions(DEBUG)
add_executable(efs ${SOURCES} ${HEADERS})