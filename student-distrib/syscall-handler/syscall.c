#include "syscall.h"

/*The halt system call terminates a process, returning the specified value to its parent process.*/


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

    for(i = 0; i < 8; i++){
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