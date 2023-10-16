#include "keyboard.h"
#include "lib.h" // has some input output stuff
#include "types.h" // reading the values

#define KEYB_IRQ_P 0x60 //maybe 21
#define KEYB_STAT 0x64 //maybe 21

char set_2_table[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', 0, 0, 0, ' '
};

void keyb_init(void){
    enable_irq(1);
}
void keyb_main(void){ //interrupt
    cli();
    uint32_t userin, kb_status;
    kb_status = inb(KEYB_STAT);
    if (kb_status & 0x01) {
        userin = inb(KEYB_IRQ_P);
        if(userin <= 0x81 && userin > 0) { // release key
            char ascii_cur = set_2_table[userin];
            if (ascii_cur) {
                putc(ascii_cur);
            }
        }
    }
    
    // data,port
    send_eoi(0x01);
    sti();
}