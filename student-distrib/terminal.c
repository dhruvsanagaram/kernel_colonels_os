#include "keyboard.h" 
#include "terminal.h"
#include "lib.h"
#include "i8259.h"


int numChars = 0;
int enterKeyPressed = 0;

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    int32_t read_bytes;
    int32_t read_lim;
    uint32_t flags;
    int i;
    while(!enterKeyPressed);
    cli_and_save(flags);

    read_bytes = 0;
    // if(nbytes >= 128){
    //     read_lim = 127; //clip the chars to read up to 127 so that the 128th index is automatically \n
    // }
    // else {
    //     read_lim = nbytes;
    // }

    //loop until either n bytes reached or command flow

    //read_lim - 1
    for (i = 0; i < nbytes - 1; i++) {

        //if nbytes > terminal buf size, this causes loop to break
        if (key_buf[i] == '\n') { 
            key_buf[i] = ' ';
            //i--;
            break;   
        }
        ((char*)buf)[i] = key_buf[i];            //copy keyboard buffer to terminal buffer
        key_buf[i] = '\0';
        read_bytes++;

    }
    //i++;
    ((char*)buf)[i] = '\n';  
    read_bytes++;                            //Add newline to the end of the buffer 
    i++;
    for (;i < nbytes;i++) {
        ((char*)buf)[i] = '\0';                             //fill rest of term buffer with 0s - for when user gives nbytes < BUFF_SIZE
    }

    keyb_char_count = 0;


    enterKeyPressed = 0;
    // sti();
    restore_flags(flags);
    return read_bytes;
}

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    const char* ptr = (const char*) buf;
    const char* end = ptr +  nbytes;
    while(ptr < end){
        //we might need to debug this to verify if buf is null terminated
        //if this breaks, we just use a for loop
        if(*ptr != '\0'){
            putc(*ptr);
        }
        ptr++;
    }
    return nbytes;
}

int32_t terminal_open(const uint8_t* filename){
    return 0;
}

int32_t terminal_close(int32_t fd){
    return -1;
}

//Complete scrolling func with putc helper
//Add to keyboard handler to handle enter, backspace, overflow when adding to keyboard buffer(key_buf)








