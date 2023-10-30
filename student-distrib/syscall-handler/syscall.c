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

    //ENUMERATE STDIN AND STDOUT FOR TERMINAL
    stdin_fops.open = terminal_open;
    stdin_fops.close = terminal_close;
    stdin_fops.read = nul_read;
    stdin_fops.write = terminal_write;


    stdout_fops.open = terminal_open;
    stdout_fops.close = terminal_close;
    stdout_fops.read = nul_read;
    stdout_fops.write = terminal_write;

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

}



/*--------------------------------------                 MAIN SYSCALLS TO IMPLEMENT                ----------------------------------------------*/

/* int32_t halt;
 * Inputs: uint_8 status
 * Return Value: 
 *  Function:  */
int32_t halt (uint8_t status) {
    //Kill me

}

/* int32_t execute;
 * Inputs: const uint_8* command
 * Return Value: 
 *  Function:  */
int32_t execute (const uint8_t* command) {
    //Fuck
}

/* int32_t read;
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: 
 *  Function:  */
int32_t read (int32_t fd, void* buf, int32_t nbytes) {
    //Return bytes read
    //If initial file position is at or beyond end of file, return 0
    //Get function for file type
    return (fd_arr[fd].fops->read)(fd,buf,nbytes); //Handle error checking within each individual function type

}

/* int32_t write;
 * Inputs: int32_t fd, const void* buf, int32_t nbytes
 * Return Value: 
 *  Function:  */
int32_t write (int32_t fd, const void* buf, int32_t nbytes) {
    return (fd_arr[fd].fops->write)(fd,buf,nbytes);
}

int32_t open (const uint8_t* filename) {
    dentry_t dentry;
    int i;
    int openFd = -1;

    if(filename == NULL) 
        return -FAILURE;

    if(read_dentry_by_name(filename, &dentry) == -FAILURE) 
        return -FAILURE;

    for(i = 0; i < 8; i++){ // max of 8 files
        if(!fd_arr[i].flags){
            openFd = i;
            break;
        }
    }

    if(openFd == -1) return -FAILURE;

    switch(dentry.filetype){
        case 0:
        fd_arr[openFd].fops = &rtc_fops;
        break;

        case 1:
        fd_arr[openFd].fops = &dir_fops;
        break;

        case 2:
        fd_arr[openFd].fops = &file_fops;
        break;
    }

    if(dentry.filetype == 0 || dentry.filetype == 1){
        fd_arr[openFd].inode_num = 0;
    }
    else{
        fd_arr[openFd].inode_num = dentry.inode_num;
    }

    fd_arr[openFd].fpos = 0;
    fd_arr[openFd].flags = 1;

    if(fd_arr[openFd].fops->open(filename) == -FAILURE){
        return -FAILURE;
    }
    //return openFd;
    return SUCCESS;
}

int32_t close (int32_t fd) {
    if(!fd_arr.flags || fd == 0 || fd == 1 || fd >= 8)
        return -FAILURE;
    fd_arr[fd].flags = 0;
    
    if(fd_arr[openFd].fops->close(filename) == -FAILURE){
        return -FAILURE;
    }
    else{
        return SUCCESS;
    }
}

int32_t getargs (uint8_t* buf, int32_t nbytes) {

}

int32_t vidmap (uint8_t** screen start) {

}

int32_t set_handler (int32_t signum, void* handler address) {

}

int32_t sigreturn (void) {

}

//////////// NULL SYSCALLS (all return -1 as they are null) ////////////
int32_t nul_read (int32_t fd, void* buf, int32_t nbytes) { return -1; }
int32_t nul_write (int32_t fd, const void* buf, int32_t nbytes) { return -1; }
int32_t nul_open (const uint8_t* filename) { return -1; }
int32_t nul_close (int32_t fd) { return -1; }

