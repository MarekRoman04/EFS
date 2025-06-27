cmake_minimum_required(VERSION 3.10.0)
project(EFS VERSION 0.1.0 LANGUAGES C)

# Header file directory
include_directories(include)

# Source files
set(SOURCES
    main.c
    src/algos/bmh.c
    src/string_search.c
    src/utils.c
)

# Header files
set(HEADERS
    include/log.h
    include/algo.h
    include/utils.h
)

add_executable(efs ${SOURCES} ${HEADERS})