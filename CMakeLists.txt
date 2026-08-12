cmake_minimum_required(VERSION 3.10)
project(EFS VERSION 1.1.0 LANGUAGES C)

# Source files
set(SOURCES
    main.c
    src/arg_parser.c
    src/efs_algo.c
    src/hash_set.c
    src/log.c
    src/search.c
    src/file_stream/dir_stream.c
    src/file_stream/file_stream.c
    src/file_stream/line_stream.c
)

# Header files
set(HEADERS
    include/hash_set.h
    include/arg_parser.h
    include/file_stream.h
    include/log.h
    include/efs_search.h
)

add_executable(efs ${SOURCES} ${HEADERS})
target_include_directories(efs PRIVATE include)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(efs PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional memory logging in debug mode
option(LOG_MEMORY "Enable memory allocation/free logging" OFF)

target_compile_definitions(efs PRIVATE
    _POSIX_C_SOURCE=200809L
    $<$<CONFIG:Debug>:DEBUG>
    $<$<AND:$<CONFIG:Debug>,$<BOOL:${LOG_MEMORY}>>:MEM_LOG>
)
