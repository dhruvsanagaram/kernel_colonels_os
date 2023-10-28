#include "syscall.h"
#include "../x86_desc.h"
#include "../rtc.h"
#include "../terminal.h"
#include "../filesys.h"


uint32_t curr_process = 0; //watcher for the current process being spawned
uint32_t par_process = 0;  //watcher for the parent process being spawned

/*The halt system call terminates a process, returning the specified value to its parent process.*/

/*--------------------------------------                 HELPERS FOR FOPS AND PCB                ----------------------------------------------*/

/* int32_t populate_fops;
 * Inputs: none
 * Return Value: status
 *  Function: populates the table of driver file ops */
int32_t populate_fops(){
    //ENUMERATE NULL_FOPS
    nul_fops.open = nul_open;
    nul_fops.close = nul_close;
    nul_fops.read = nul_read;
    nul_fops.write = nul_write;

    //ENUMERATE RTC_FOPS
    rtc_fops.open = rtc_open;
    rtc_fops.close = rtc_close;
    rtc_fops.read = rtc_read;
    rtc_fops.write = rtc_write;

    //ENUMERATE DIR_FOPS
    dir_fops.open = directory_open;
    dir_fops.close = directory_close;
    dir_fops.read = directory_read;
    dir_fops.write = directory_write;

    //ENUMERATE FILE_OPS
    file_fops.open = file_open;
    file_fops.close = file_close;
    file_fops.read = file_read;
    file_fops.write = file_write;

    //ENUMERATE STDIN AND STDOUT FOR TERMINAL
    stdin_fops.open = terminal_open;
    stdin_fops.close = terminal_close;
    stdin_fops.read = nul_read;
    stdin_fops.write = terminal_write;


    stdout_fops.open = terminal_open;
    stdout_fops.close = terminal_close;
    stdout_fops.read = nul_read;
    stdout_fops.write = terminal_write;
}



/*--------------------------------------                 MAIN SYSCALLS TO IMPLEMENT                ----------------------------------------------*/

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

//////////// NULL SYSCALLS (all return -1 as they are null) ////////////
int32_t nul_read (int32_t fd, void* buf, int32_t nbytes) { return -1; }
int32_t nul_write (int32_t fd, const void* buf, int32_t nbytes) { return -1; }
int32_t nul_open (const uint8_t* filename) { return -1; }
int32_t nul_close (int32_t fd) { return -1; }

