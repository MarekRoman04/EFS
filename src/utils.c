#include "utils.h"

char *load_file(const char *f_path)
{
    FILE *fp = fopen(f_path, "r");
    size_t f_size;

    if (!fp)
    log_error("Error openning file!\n");

    FILE_SIZE(fp, f_size);
    char *f_data = (char *) malloc(sizeof(char) * f_size + 1);

    if (!f_data)
        log_error("Error allocating memory!\n");

    fread(f_data, f_size, 1, fp);
    f_data[f_size] = 0;

    fclose(fp);
    return f_data;
}