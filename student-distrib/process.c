#include "process.h"
#include "lib.h"
#include "filesys.h"
#include "page.h"
#include "syscall.h"


// int32_t process_slots[2] = {0,0};
int32_t process_slots[2];
    process_slots[0] = 0;
    process_slots[1] = 0;
int32_t canhandle_new = 1; // be default they all are open

int32_t system_execute(const uint8_t* command) {
    //uint32_t flags;
    //cli_and_save(flags);
    //Parse args
    /*
    The command is a space-separated sequence of words. The first word is the file name of the
    program to be executed, and the rest of the command—stripped of leading spaces—should be provided to the new
    program on request via the getargs system call.
    */

    uint8_t cmd[CMD_SIZE];
    uint8_t arg1[ARG_SIZE];
    int i;
    for (i = 0; i < MAX_BUF; i++) {
        if (command[i] == ' ' || command[i] == '/0' || command[i] == '/n') {
            cmd[i] = '/0';
            break;
        }
        else {
            cmd[i] = command[i];
        }
    }
    i++;
    for (;i < MAX_BUF; i++) {
        if (command[i] == ' ' || command[i] == '/0' || command[i] == '/n') {
            arg1[i] = '/0';
            break;
        }
        else {
            arg1[i] = command[i];
        }
    }

    //Check if command exists
    //Check for executable
    //MAGIC NUMBER!
    dentry_t dentry;
    if(read_dentry_by_name(cmd[0], &dentry) == -FAILURE){
        return -FAILURE;
    }
    uint8_t* buffer[40];
    if(read_data(dentry.inode_num, 0, buffer, 40) != 40
        || (buffer[0] != 0x7F || buffer[1] != 0x45 || buffer[2] != 0x4C || buffer[3] != 0x46)){
        return -FAILURE;
    }

    //Get a PID
    int cur_PID = -1;
    int i;
    for (int i = 0; i < 2; i++) {
        if (process_slots[i] == 0){
            cur_PID = i;
            process_slots[i] = 1;
            break;
        }
    }
    if(cur_PID == -1){
        //canhandle_new = 0; // false, 1 by default
        printf("Max processes\n");
        return SUCCESS;
    }

    //Set up paging

    page_directory[USER_INDEX].base_addr = 2+cur_PID;
    // flush the tlb
    asm volatile(
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        :
        :
        : "eax", "memory", "cc"
    );

    //Load file
    inode_t* prog_img_inode = &inode_start_ptr(dentry.inode_num);

    if (read_data(dentry.inode_num, 0, (uint8_t*)0x0804800, prog_img_inode->len) == -FAILURE) {
        return -FAILURE;
    }



    //init pcb
    int32_t new_pcb_pids[2];
    for (int i = 0; i < 2; i++) {
        new_pcb_pids[i] = process_slots[i] + 1;
    }

    pcb_t* pointer_top = (pcb_t*)(0x0800000 - (0x2000*(new_pcb_pids[1])));
    pcb_t* pointer_bot = (pcb_t*)(0x0800000 - (0x2000*(new_pcb_pids[0])));

    pcb->fd_arr[0].fops = &stdin_fops;
    pcb->fd_arr[0].inode_num = 0;
    pcb->fd_arr[0].fpos = 0;
    pcb->fd_arr[0].flags = 1;
    pcb->fd_arr[1].fops = &stdout_fops;
    pcb->fd_arr[1].inode_num = 0;
    pcb->fd_arr[1].fpos = 0;
    pcb->fd_arr[1].flags = 1;
    for(i = 2; i < 8; i++){
        pcb->fd_arr[i].fops = &nul_fops;
        pcb->fd_arr[i].inode_num = 0;
        pcb->fd_arr[i].fpos = 0;
        pcb->fd_arr[i].flags = 0;
    }

    //Prepare for context switch

    //Push IRET context to stack
    
    return SUCCESS;
}