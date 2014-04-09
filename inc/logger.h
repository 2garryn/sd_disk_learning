//Logger header file
#include <stdint.h>

#ifndef _LOGGER_INC
#define _LOGGER_INC 1

void logger_initialize();

void logger_print(uint8_t str_size, char * str);

#endif
