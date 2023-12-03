#include "syscall.h"
#include "../x86_desc.h"
#include "../rtc.h"
#include "../terminal.h"
#include "../filesys.h"
#include "../process.h"
#include "../page.h"

#define SUCCESS 0
#define FAILURE 1


// uint32_t curr_process = 0; //watcher for the current process being spawned
// uint32_t par_process = 0;  //watcher for the parent process being spawned

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
    stdin_fops.read = terminal_read;
    stdin_fops.write = terminal_write;


    stdout_fops.open = terminal_open;
    stdout_fops.close = terminal_close;
    stdout_fops.read = terminal_read;
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


    return SUCCESS;
}



/*--------------------------------------                 MAIN SYSCALLS TO IMPLEMENT                ----------------------------------------------*/

/* int32_t halt;
 * Inputs: uint_8 status
 * Return Value: int32_t
 *  Function:  */
int32_t halt (uint8_t status) {
    return system_halt((uint16_t)status);
}

/* int32_t execute;
 * Inputs: const uint_8* command
 * Return Value: int32_t
 *  Function:  */
int32_t execute (const uint8_t* command) {
    return system_execute(command);
}

/* int32_t read;
 * Inputs: int32_t fd, void* buf, int32_t nbytes
 * Return Value: 
 *  Function:  */
int32_t read (int32_t fd, void* buf, int32_t nbytes) {
    if (fd<0 || fd > 7) {
        return -FAILURE;
    }
    if (fd == 1) {
        return -FAILURE;
    }
    if (getRunningPCB()->fd_arr[fd].flags == 0) {
        return -FAILURE;
    }
    return (getRunningPCB()->fd_arr[fd].fops->read)(fd,buf,nbytes); //Handle error checking within each individual function type

}

/* int32_t write;
 * Inputs: int32_t fd, const void* buf, int32_t nbytes
 * Return Value: 
 *  Function:  */
int32_t write (int32_t fd, const void* buf, int32_t nbytes) {
    if (fd<1 || fd > 7) {   //boundaries for fd numbers 
        return -FAILURE;
    }

    return (getRunningPCB()->fd_arr[fd].fops->write)(fd,buf,nbytes);
}

/* int32_t open;
 * Inputs: const uint8_t* filename
 * Return Value: int32_t
 *  Function:  */
int32_t open (const uint8_t* filename) {
    pcb_t* pcb = getRunningPCB();
    dentry_t dentry;
    int i;
    int openFd = -1;

    if(filename == NULL) 
        return -FAILURE;

    if(read_dentry_by_name(filename, &dentry) == -FAILURE) 
        return -FAILURE;

    for(i = 0; i < 8; i++){ // max of 8 files
        if(!pcb->fd_arr[i].flags){
            openFd = i;
            break;
        }
    }

    if(openFd == -1) return -FAILURE;

    switch(dentry.filetype){
        case 0:
        pcb->fd_arr[openFd].fops = &rtc_fops;
        break;

        case 1:
        pcb->fd_arr[openFd].fops = &dir_fops;
        break;

        case 2:
        pcb->fd_arr[openFd].fops = &file_fops;
        break;
    }

    if(dentry.filetype == 0 || dentry.filetype == 1){
        pcb->fd_arr[openFd].inode_num = 0;
    }
    else{
        pcb->fd_arr[openFd].inode_num = dentry.inode_num;
    }

    pcb->fd_arr[openFd].fpos = 0;
    pcb->fd_arr[openFd].flags = 1;

    if(pcb->fd_arr[openFd].fops->open(filename) == -FAILURE){
        return -FAILURE;
    }
    return openFd;
}

/* int32_t close
 * Inputs: int32_t fd
 * Return Value: int32_t
 *  Function:  */
int32_t close (int32_t fd) {
    pcb_t* pcb = getRunningPCB();
    if (fd<2 || fd > 7) {
        return -FAILURE;
    }
    if(!pcb->fd_arr[fd].flags || fd == 0 || fd == 1 || fd >= 8)
        return -FAILURE;
    pcb->fd_arr[fd].flags = 0;
    
    if(pcb->fd_arr[fd].fops->close(fd) == -FAILURE){
        return -FAILURE;
    }
    return SUCCESS;
}

/* int32_t getargs
 * Inputs: uint8_t* buf, int32_t nbytes
 * Return Value: int32_t
 *  Function:  */
int32_t getargs (uint8_t* buf, int32_t nbytes) {
    pcb_t* pcb = getRunningPCB();
    if(pcb->arg1[0] == '\0') return -FAILURE;
    strncpy((int8_t*)buf, (int8_t*)pcb->arg1, nbytes);

    return SUCCESS;
}

/* int32_t vidmap
 * Inputs: uint8_t** screen_start
 * Return Value: int32_t
 *  Function:  */
int32_t vidmap (uint8_t** screen_start) {
    if(screen_start == NULL){
        return -1;
    }
    uint32_t screen_addr = (uint32_t)(screen_start); //get the screen start address as 32bit signextend

    //check if the address is within bounds of the maximum allocated page
    if(screen_addr < USER_ADDR || screen_addr > (USER_ADDR+FOUR_MB)){
        return -1;
    }

    //set up the virtual address at 136 mb which is for vidmap
    page_directory[VIDMAP_IDX].ps_bit = 0;
    page_directory[VIDMAP_IDX].present = 1;
    page_directory[VIDMAP_IDX].user = 1;
    page_directory[VIDMAP_IDX].base_addr = ((int)page_video_map)/FOUR_KB; //align the 20bits to 4KB

    //set the pages in the page table for the video map
    page_video_map[0].present = 1;
    
    // int32_t term_id_cur = view_term->tid;
    // terminals[term_id_cur].vidmap_present = 1;

    pcb_t *cur_pcb = getRunningPCB();
    cur_pcb->vidmap_present = 1;
    
    // if (schedule_term->tid == view_term->tid) {
    //     vidmap_page_change(VIDEO_ADDR / FOUR_KB, 1);
    // }
    // else {
    //     vidmap_page_change(schedule_term->vidmem_data / FOUR_KB, 1);
    // }

    // vidmap_page_change(VIDEO_ADDR / FOUR_KB, 1);
    vidmap_page_change(view_term->vidmem_data / FOUR_KB, 1);




    page_video_map[0].user = 1; //set to user mode
    page_video_map[0].base_addr = VIDEO_ADDR/FOUR_KB;

    //flush tlb
    asm volatile(
        "movl %%cr3, %%eax \n\
         movl %%eax, %%cr3 \n\
        "
        : : : "memory"
    );

    //set the screen start to point to the base of the 4MB page assigned to vidmem at 126MB
    *screen_start = (uint8_t*)(VIDEO_ADDR_VIR);
    return SUCCESS;
}

int32_t set_handler (int32_t signum, void* handler_address) {
    return -FAILURE;
}

int32_t sigreturn (void) {
    return -FAILURE;
}

//////////// NULL SYSCALLS (all return -1 as they are null) ////////////
int32_t nul_read (int32_t fd, void* buf, int32_t nbytes) { return -1; }
int32_t nul_write (int32_t fd, const void* buf, int32_t nbytes) { return -1; }
int32_t nul_open (const uint8_t* filename) { return -1; }
int32_t nul_close (int32_t fd) { return -1; }

