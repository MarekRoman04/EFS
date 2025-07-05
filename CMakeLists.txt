cmake_minimum_required(VERSION 3.10.0)
project(EFS VERSION 0.2.0 LANGUAGES C)

# Header file directory
include_directories(include)

# Source files
set(SOURCES
    main.c
    src/algos/bmh.c
    src/arg_parser.c
    src/file.c
    src/log.c
    src/string_search.c
)

# Header files
set(HEADERS
    include/algo.h
    include/arg_parser.h
    include/data.h
    include/file.h
    include/log.h
)

add_compile_definitions(DEBUG)
add_executable(efs ${SOURCES} ${HEADERS})