#ifndef PIT_H
#define PIT_H
#include "i8259.h"


int32_t pit_init(int hz);
int32_t pit_handler();

#endif