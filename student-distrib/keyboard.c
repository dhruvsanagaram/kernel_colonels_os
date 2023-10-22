#include "keyboard.h"
#include "terminal.h"
#include "lib.h"   // has some input output stuff
#include "types.h" // reading the values
#include "i8259.h"
#include "idt.h"
#include "x86_desc.h"

#define KEYB_IRQ_P 0x60
#define KEYB_STAT 0x64
#define CAPS_CHAR 0x3A
#define BCKSPC_CHAR 0x0E

#define LOWER_TBL 0
#define UPPER_TBL 1
#define LOCK_TBL 2 //capslock table

#define LEFT_SHIFT 0x2A
#define LEFT_SHIFT_RELEASE 0xAA
#define RIGHT_SHIFT_RELEASE 0xB6

/*
  general idea:
  1. keep track of num chars read from keyb
  2. we have 2 tables: regular table, caps table
  3. keep track of key currently pressed via a userin var
  4. decoder for special characters -- will take in the keypress, and if it is
                                       any character that needs us to modify the buffer
                                       (e.g. backspace, enter, capslock, shift, etc.)
                                       we hold status of which one is pressed via a collection of flags
                                       and we return a status flag if this was pressed
  6. If shift pressed (use a flag to check this), we read from second table, and if caps lock is pressed,
     we read from second table. If caps lock is not pressed and shift flag is set while userin gains further input,
     we read from second table. Every other case reads from the first table.
  5. we check if the userin was a special character via the decoder:
     if so, we modify the buffer accordingly
  6. Default case: putc the userin to output


*/

// global counter for number of characters
uint32_t keyb_char_count = 0;
uint32_t bufLimit = 0;
uint32_t tabLimit = 0;

char key_buf[MAX_BUF_SIZE];

// bool flags to check which important keys are pressed
// unsigned int caps_pressed;
unsigned int caps_was_active = 0;
unsigned int shift_pressed = 0;
unsigned int del_pressed = 0;
unsigned int enter_pressed = 0;
unsigned int table_to_read_from = 0;

unsigned int ctrl_pressed = 0;

// table comes from os dev
char lower_table[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', ' ',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' '
    };


char caps_table[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', ' ',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '
};

char lock_table[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', ' ',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', 0, 0, 0, ' '
};

    
// char caps_table[] = {
//     0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
//     'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', 0, 'A', 'S',
//     'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V',
//     'B', 'N', 'M', ',', '.', '/', 0, 0, 0, ' '
// };

// char Sh_table[] = {
//     0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
//     'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S',
//     'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V',
//     'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '
//     };

/* void keyb_init(void);
 * Inputs: N/A
 * Return Value: void
 *  Function: init call for keyboard to be called from kernel.c
    SHIFT, CAPS_LOCK, BACKSPACE, CTRL_L*/
void keyb_init(void)
{
    enable_irq(1);
}

/* void keyb_main(void);
 * Inputs: N/A
 * Return Value: void
 *  Function: interrupt handler for keyboard, prints char to screen */
void keyb_main(void)
{ // interrupt
    uint32_t flags;
    uint32_t userin, kb_status;
    cli_and_save(flags); // store the flags

    //Reached character limit in keyboard buffer
    if (keyb_char_count == 127)
    {
        bufLimit = 1;
    }
    else
    {
        bufLimit = 0;
    }

    //Reached tab limit in keyboard buffer
    if (keyb_char_count >= 124) {
        tabLimit = 1;
    }
    else {
        tabLimit = 0;
    }

    kb_status = inb(KEYB_STAT);
    if (kb_status & 0x01)
    {
        userin = inb(KEYB_IRQ_P);

        // CTRL-L
        if (userin == 0x1D)
        {
            ctrl_pressed = 1; // CTRL pressed
            send_eoi(1);
            return;
        }
        if (ctrl_pressed == 1 && userin == 0x26)
        {
            // call function to clear screen and update cursor
            keyb_char_count = 0; // CTRL Pressed & L Pressed
            clear();
            send_eoi(1);
            restore_flags(flags);
            return;
        }
        if (userin == 0x9D)
        {
            ctrl_pressed = 0; // CTRL Released
            send_eoi(1);
            return;
        }

        // SHIFT
        // Enable shift capability
        if ((userin == 0x2A || userin == 0x36))
        {                      // Left shift pressed or right shifted
            shift_pressed = 1; // make this non sticky. Find a way to make sure we know if shift released?
            send_eoi(1);
            restore_flags(flags);
            return;
        }
        if ((userin == LEFT_SHIFT_RELEASE || userin == RIGHT_SHIFT_RELEASE))
        {
            shift_pressed = 0;
            send_eoi(1);
            restore_flags(flags);
            return;
        }

        // CAPSLOCK
        // caps pressed once...
        // you need to set CAPS CHAR somewhere above
        if (userin == CAPS_CHAR)
        {
            // caps_pressed = !caps_pressed // by default 0, 0.>1
            //  ^^^ not sure if this is needed
            if (caps_was_active)
            {
                caps_was_active = 0; // caps deactivated
                                     // start reading from first table because caps was clicked twice
            }
            else
            {
                caps_was_active = 1; // set caps to active
                                     // read from second table
            }
            send_eoi(1);
            restore_flags(flags);
            return;
        }

        // Don't return since we want to process valid character with shift/caps presets
        // if ((shift_pressed == 1))
        // {
        //     table_to_read_from = SH_TBL; // set tables to read from accordingly
        // }

        //Table read decoder
        if(shift_pressed == 1){
            table_to_read_from = UPPER_TBL;
        }
        if(caps_was_active == 1 && shift_pressed == 0)
        {
            table_to_read_from = LOCK_TBL;
        }
        if(shift_pressed == 0 && caps_was_active == 0){

            table_to_read_from = LOWER_TBL;
        }

        // BACKSPACE
        if (userin == BCKSPC_CHAR)
        {
            // check if there is space to delete a character, and if not, just exit the if
            if (keyb_char_count > 0)
            {
                // key_buf[keyb_char_count] = ' '; 
                if(keyb_char_count <= 128){
                    key_buf[keyb_char_count] = ' ';
                }
                keyb_char_count--;
            }
            else {
                send_eoi(1);
                restore_flags(flags);
                return;
            }
        }

        // ENTER
        if (userin == 0x1C)
        {
            // putc('\n');
            enterKeyPressed = 1;
            key_buf[keyb_char_count] = '\n';
        }

        // 0x02 = 1 , 0x3A = SPC
        if (userin < 0x3A && userin >= 0x02 && !bufLimit && userin != 0x8F)
        {
            // map the current ascii symbol from the table
            // char ascii_cur = (!!table_to_read_from) ? caps_table[userin] : set_2_table[userin];
            char ascii_cur;
            switch(table_to_read_from){
                case LOWER_TBL: ascii_cur = lower_table[userin]; break;
                case UPPER_TBL: ascii_cur = caps_table[userin]; break;
                case LOCK_TBL: ascii_cur = lock_table[userin]; break;
            }
            //char ascii_cur;
            // switch (!!table_to_read_from){
            //     case 1:
            //         ascii_cur = caps_table[userin];
            //         break;
            //     case 2:
            //         ascii_cur = Sh_table[userin];
            //         break;
            //     case 0:
            //         ascii_cur = set_2_table[userin];
            //         break;
            //}
            if (ascii_cur)
            {
                putc(ascii_cur); // Print the character from stdin onto the screen

                //perhaps we should dbg the val of keyb_char_count + 1
                //to verify if we need this if
                //if (keyb_char_count + 1 < BUFF_SIZE)
               // {
                key_buf[keyb_char_count] = ascii_cur;
                keyb_char_count++;
              //  }
                // else
                // {
                //     keyb_char_count++;
                // }
            }
        }
    }
    //Print info about the buffer
    //printf("Keyb count:%d\n", keyb_char_count);

    send_eoi(1); // send End of Interrupt on the master pic(1 corresponds to IRQ1 on master PIC)
    // sti();
    restore_flags(flags); // bring back the flags
}

// void keyb_main(void){                                       //interrupt
//     uint32_t flags;
//     cli_and_save(flags);                                    // store the flags
//     uint32_t userin, kb_status;
//     kb_status = inb(KEYB_STAT);
//     if (kb_status & 0x01) {
//         userin = inb(KEYB_IRQ_P);

//         //0x02 = 1 , 0x3A = SPC
//         if(userin <= 0x3A && userin >= 0x02) {              // release key
//             char ascii_cur = set_2_table[userin];           //map the current ascii symbol from the table
//             if (ascii_cur) {
//                 putc(ascii_cur);                            //Print the character from stdin onto the screen
//             }
//         }
//     }
    
//     // data,port
//     send_eoi(0x01);                                        //send End of Interrupt on the master pic(1 corresponds to IRQ1 on master PIC)
//     sti();
//     restore_flags(flags);                                  // bring back the flags 
// }
