#include "keyboard.h"
#include "lib.h" // has some input output stuff
#include "types.h" // reading the values
#include "i8259.h"

#define KEYB_IRQ_P 0x60  
#define KEYB_STAT 0x64 

char set_2_table[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', 0, 0, 0, ' '
};

/* void keyb_init(void);
 * Inputs: N/A
 * Return Value: void
 *  Function: init call for keyboard to be called from kernel.c */
void keyb_init(void){
    enable_irq(1);
}

/* void keyb_main(void);
 * Inputs: N/A
 * Return Value: void
 *  Function: interrupt handler for keyboard, prints char to screen */
void keyb_main(void){                                       //interrupt
    uint32_t flags;
    cli_and_save(flags);                                    // store the flags
    uint32_t userin, kb_status;
    kb_status = inb(KEYB_STAT);
    if (kb_status & 0x01) {
        userin = inb(KEYB_IRQ_P);

        //0x02 = 1 , 0x3A = SPC
        if(userin <= 0x3A && userin >= 0x02) {              // release key
            char ascii_cur = set_2_table[userin];           //map the current ascii symbol from the table
            if (ascii_cur) {
                putc(ascii_cur);                            //Print the character from stdin onto the screen
            }
        }
    }
    
    // data,port
    send_eoi(0x01);                                        //send End of Interrupt on the master pic(1 corresponds to IRQ1 on master PIC)
    sti();
    restore_flags(flags);                                  // bring back the flags 
}
