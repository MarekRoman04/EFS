#ifndef FILE_H
#define FILE_H

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "arg_parser.h"
#include "log.h"

size_t map_files(cli_args *args);

#endif