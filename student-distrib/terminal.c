#include "keyboard.h" 
#include "terminal.h"
#include "lib.h"
#include "i8259.h"

int numChars = 0;
int enterKeyPressed = 0;

/* terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Outputs: int32_t read_bytes
 * Side Effects: Copies keyboard buffer into terminal buffer
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    // printf("Entering terminal read...");
    int32_t read_bytes;
    uint32_t flags;
    int i;
    while(!enterKeyPressed);                      //flush buffer upon enter
    cli_and_save(flags);

    read_bytes = 0;
    
    /*
    if (key_buf[i] == '\n') {

    }
    */

    for (i = 0; i < nbytes - 1; i++) {

        if (key_buf[i] == '\n') { 
            key_buf[i] = ' ';
            break;   
        }
        ((char*)buf)[i] = key_buf[i];            //copy keyboard buffer to terminal buffer
        key_buf[i] = '\0';
        read_bytes++;

    }

    ((char*)buf)[i] = '\n';  
    read_bytes++;                               //Add newline to the end of the buffer 
    i++;
    for (;i < nbytes;i++) {
        ((char*)buf)[i] = '\0';                 //fill rest of term buffer with 0s - for when user gives nbytes < BUFF_SIZE
    }

    keyb_char_count = 0;


    enterKeyPressed = 0;
    restore_flags(flags);
    return read_bytes;
}

/* terminal_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: int32_t fd, const void* buf, int32_t nbytes
 * Outputs: int32_t nbytes
 * Side Effects: Writes terminal buffer to screen
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    const char* ptr = (const char*) buf;
    const char* end = ptr +  nbytes;
    while(ptr < end){
        if(*ptr != '\0'){
            putc(*ptr);                 //putc each char in terminal buff
        }
        ptr++;
    }
    return nbytes;
}

/* terminal_open(const uint8_t* filename)
 * Inputs: const uint8_t* filename
 * Outputs: int32_t
 * Side Effects: Open Terminal 
 */
int32_t terminal_open(const uint8_t* filename){
    return 0;
}

/* terminal_close(int32_t fd)
 * Inputs: int32_t fd
 * Outputs: int32_t, -1
 * Side Effects: Close Terminal
 */
int32_t terminal_close(int32_t fd){
    return -1;
}







