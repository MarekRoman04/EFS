#ifndef DATA_H
#define DATA_H

#include <stdlib.h>
#include <stdio.h>

typedef struct string
{
    const char *data;
    size_t length;
} string;

typedef struct search_buffer
{
    char *buffer;
    size_t buffer_size;
    char *f_path;
    FILE *fp;
    int flags;
} search_buffer;

#endif