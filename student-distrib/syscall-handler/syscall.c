#include "syscall.h"

/*The halt system call terminates a process, returning the specified value to its parent process.*/


/* int32_t halt;
 * Inputs: uint_8 status
 * Return Value: 
 *  Function: interrupt handler for keyboard, prints char to screen */
int32_t halt (uint8_t status) {
    //Kill me

}

int32_t execute (const uint8_t* command) {
    //Fuck
}

int32_t read (int32_t fd, void* buf, int32_t nbytes) {
    //Return bytes read
    //If initial file position is at or beyond end of file, return 0
}

int32_t write (int32 t fd, const void* buf, int32 t nbytes) {

}

int32_t open (const uint8 t* filename) {

}

int32_t close (int32 t fd) {

}

int32_t getargs (uint8 t* buf, int32 t nbytes) {

}

int32_t vidmap (uint8 t** screen start) {

}

int32_t set_handler (int32 t signum, void* handler address) {

}

int32_t sigreturn (void) {

}