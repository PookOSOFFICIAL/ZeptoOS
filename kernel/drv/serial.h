#ifndef SERIAL_H
#define SERIAL_H

#include "../lib/types.h"

void serial_init();
void serial_write(char c);
void serial_printf(char* str);

#endif
