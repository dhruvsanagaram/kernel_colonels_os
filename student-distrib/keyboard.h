#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

void keyb_main(void); //interrupt handler
void keyb_init(void); 

extern uint32_t keyb_char_count;

#endif // KEYBOARD_H
