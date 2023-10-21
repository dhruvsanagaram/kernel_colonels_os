#include "keyboard.h" 
#include "terminal.h"
#include "lib.h"
#include "i8259.h"

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    char* read;
    int32_t read_bytes;
    uint32_t flags;
    int i;
    while(!enterKeyPressed);
    cli_and_save(flags);

    read_bytes = 0;
    for (i = 0; i < nbytes; i++) {

        //if nbytes > terminal buf size, this causes loop to break
        if (((char)key_buf)[i] == '\n') { 
            i = i-1;    //Why?
            break;   
        }
        ((char)buf)[i] = key_buf[i];            //copy keyboard buffer to terminal buffer
        read_bytes++;
    }
    term_buf[i] = '\n';                              //Add newline to the end of the buffer 
    i++;
    for (;i < nBytes;i++) {
        term_buf[i] = 0;                             //fill rest of term buffer with 0s - for when user gives nbytes < BUFF_SIZE
    }

    enterKeyPressed = 0;
    sti();
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








