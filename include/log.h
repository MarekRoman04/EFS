#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>

#define LOG_INFO(msg) \
    fprintf(stderr, "[INFO] %s:%d: %s\n", __FILE__, __LINE__, (msg))

#define LOG_ERROR(msg)                                                     \
    do                                                                     \
    {                                                                      \
        fprintf(stderr, "[ERROR] %s:%d: %s\n", __FILE__, __LINE__, (msg)); \
        exit(EXIT_FAILURE);                                                \
    } while (0)

#endif