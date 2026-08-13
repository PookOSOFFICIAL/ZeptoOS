#pragma once

#include "../lib/types.h"

void keyboard_init(void);
void keyboard_handler(void);
int keyboard_has_char(void);
char keyboard_get_char(void);
