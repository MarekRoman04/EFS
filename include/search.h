#ifndef SEARCH_H
#define SEARCH_H

#include <stdio.h>
#include <stdlib.h>

#include "algo.h"
#include "arg_parser.h"
#include "data.h"
#include "file.h"
#include "log.h"

void process_data(cli_args *args);
void start_search(const string *pattern, const search_buffer *buffer);

#endif