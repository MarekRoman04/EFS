cmake_minimum_required(VERSION 3.10.0)
project(EFS VERSION 0.0.1 LANGUAGES C)

# Header file directory
include_directories(include)

# Source files
set(SOURCES
    main.c
)

# Header files
set(HEADERS
)

add_executable(efs ${SOURCES} ${HEADERS})