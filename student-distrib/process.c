#include "process.h"
#include "lib.h"
#include "page.h"
#include "x86_desc.h"
#include "terminal.h"

#define SUCCESS 0
#define FAILURE 1

int32_t process_slots[6] = {0, 0, 0, 0, 0, 0};
uint32_t cur_PID = -1;    //watcher for current pid being executed
int32_t cur_terminal = 0; //index out of 3 terminals

/**
* getRunningPCB
* inputs: none
* output: pointer to the recently running PCB calculated via runtime stack
* side effects: none
*/
pcb_t* getRunningPCB() {
    pcb_t* pcb;
    pcb = (pcb_t*)(0x800000 - 0x2000*(schedule_term->pid+1)); //Current PCB location: 8MB - 8kB * (pid # + 1)
    return pcb;
}


pcb_t* getPCBByPid(int pid) {
    pcb_t* pcb;
    pcb = (pcb_t*)(0x800000 - 0x2000*(pid+1)); //Current PCB location: 8MB - 8kB * (pid # + 1)
    return pcb;
}


//Perform a switch to the next appropriate process for the terminal
void next_process(){
    /*
        * TODO: Video memory switching/paging
        * TODO: Scheduling
        1. save the info for the current running process 
        2. update the info for the curr pcb ==> set curr_term_pcb to new_term_pcb
        3. set the ebp, esp to the new_term_pcbs's values
        4. setup correct TSS of target process
        5. Paging
    */


    //Step 1
    
    if(schedule_term->pid >= 0){
        pcb_t* pcb = getRunningPCB();
        pcb->tss_kernel_stack_ptr = tss.esp0;
        asm volatile(
            "movl %%esp, %0 \n\t"
            "movl %%ebp, %1 \n\t"
            : "=r" (pcb->process_esp), "=r" (pcb->process_ebp)
        );
    }

    
    //Step 2
    terminal_t* new_terminal = &terminals[schedule_term->tid + 1 % 3]; //default as 0 not -1
    int new_pid = new_terminal->pid;
    // new_terminal->pid = cur_PID;
    schedule_term = new_terminal;



    // global cur diff as new process

     if (new_pid == -1) {
         system_execute((uint8_t*)"shell");
     }

    //Step 3
    // left to right

    pcb_t* new_term_PCB = getPCBByPid(new_pid);
    asm volatile(
            "movl %0, %%esp \n\t" 
            "movl %1, %%ebp \n\t"
            : : "r" (new_term_PCB->process_esp), "r" (new_term_PCB->process_ebp)
    );

    //Step 4
    tss.ss0 = KERNEL_DS;
    tss.esp0 = new_term_PCB->tss_kernel_stack_ptr;
    // tss.esp0 = 0x800000 - 0x2000*(new_terminal->running_PID)

    //Step 5
    user_page_setup(new_pid); // done by pid thanks to earlier coding

    //flush TLB
    asm volatile(
        "movl %%cr3, %%eax \n\t"
        "movl %%eax, %%cr3 \n\t"
        : : : "memory"
    );
}


/**
* system_halt
* inputs: uint8_t status
* output: int32_t
* side effects: Restore parent data & paging & clear relevant FDs
*/
int32_t system_halt(uint16_t status){
    int i;                                      //for iterating across every FDs that are running
    cli();                                      //mask interrupts

    pcb_t* pcb = getRunningPCB();               //get the running PCB
    if(pcb == NULL){
        sti();
        return -FAILURE;
    }

    if (pcb->parent_pid == -1) {
        process_slots[pcb->pid] = 0;       
        for(i = 0; i < 8; i++){
            pcb->fd_arr[i].inode_num = 0;
            pcb->fd_arr[i].flags = 0;                           //Process halted so flags are set to 0 to signify file out of use
            pcb->fd_arr[i].fpos = 0;
            pcb->fd_arr[i].fops = &nul_fops;                     //no more file ops for FDs >:)
        }

        terminals[pcb->tid].pid = -1;
        system_execute((uint8_t*)"shell");
    }

    terminals[pcb->tid].pid = pcb->parent_pid; // most recent process pid

    //Restore parent data
    // pcb_t* parent_pcb = (pcb_t*)(0x800000 - 0x2000 * (pcb->parent_pid+1));  //retrieve parent_pcb start address
    cur_PID = pcb->parent_pid;
    tss.ss0 = KERNEL_DS;
    tss.esp0 = 0x800000 - 0x2000 * (pcb->parent_pid); //getting start pointer of kernel stack for process

    //Restore parent paging(& flush TLB)
    user_page_setup(cur_PID);
    asm volatile(
        "movl %%cr3, %%eax \n\
         movl %%eax, %%cr3 \n\
        "
        :
        :
        : "memory"
    );


    //Close all relevant FDs
    process_slots[pcb->pid] = 0;                            //free up the slot for the recently running process
    for(i = 0; i < 8; i++){
        pcb->fd_arr[i].inode_num = 0;
        pcb->fd_arr[i].flags = 0;                           //Process halted so flags are set to 0 to signify file out of use
        pcb->fd_arr[i].fpos = 0;
        pcb->fd_arr[i].fops = &nul_fops;                     //no more file ops for FDs >:)
    }


    
    if (pcb->vidmap_present == 1) { //Vidmem in use for this process
        if (schedule_term->tid == view_term->tid) {
            vidmap_page_change(VIDEO_ADDR / 0x1000, 0);//Change vidmem mapping
        }
        else {
            vidmap_page_change(schedule_term->vidmem_data / 0x1000, 0);
        }
    }

    pcb->vidmap_present = 0;


    // if(process_slots[0] == 0 && process_slots[1] == 0 && process_slots[2] == 0){
    //     system_execute((uint8_t*)("shell")); //relaunch the shell
    //     return SUCCESS;
    // } <-- we need to change this logic


    //Jump to execute return
    uint32_t esp, ebp;
    uint16_t retStatus = (uint16_t) status;
    ebp = pcb->process_ebp;
    esp = pcb->process_esp; //set it to stack ptr in kernel space   


    //Sagnik: why this line again? vidmap_present is the paging; why would you just invalidate that page?
    // if (schedule_term.vidmap_present ==1){
    //     schedule_term.vidmap_present = 0;
    // }



    //////////// we need to incorporate the schedule terminal logic here /////////////
    /////////////////////////switch user mapping in video/////////////////////////////
    

    //restore ebp and esp for that of parent process
    sti();
    asm volatile(
        "movl %0, %%esp\n\t"
        "movl %1, %%ebp\n\t"
        "xorl %%eax, %%eax\n\t"
        "movw %2, %%ax\n\t"
        : : "r"(esp), "r"(ebp), "r"(retStatus)
        : "ebp", "ebp", "memory", "cc"
    );

    asm volatile(
        "jmp EXECUTE_RETURN"
    );

    return SUCCESS;
}


/**
* parse_command
* inputs: const uint8_t* command, uint8_t* cmd, uint8_t* arg1
* output: void
* side effects: Parse command into cmd, args
*/
void parse_command(const uint8_t* command, uint8_t* cmd, uint8_t *arg1){
    int i, j;
    //cmd and arg1 initialized
    for (i = 0; i < MAX_BUF; i++) {
        if (command[i] == ' ' || command[i] == '\0' || command[i] == '\n') {
            cmd[i] = '\0';
            break;
        }
        else {
            cmd[i] = command[i];
        }
    }
    if (command[i] != '\0') {
        i++;
        j = 0;
        for (;i < MAX_BUF; i++) {
            if (command[i] == ' ' || command[i] == '\0' || command[i] == '\n') {
                arg1[j] = '\0';
                break;
            }
            else {
                arg1[j] = command[i];
            }
            j++;
        }       
    }
}

/**
* check_exec
* inputs: dentry_t dentry, uint8_t* buffer, uint8_t* cmd
* output: bool true/false
* side effects: check executable
*/
int32_t check_exec(dentry_t* dentry, uint8_t* buffer, uint8_t* cmd){

    //retrieves the dentry given the filename and reads 40 bytes of data and checks executable
    if(read_dentry_by_name(&cmd[0], dentry) == -FAILURE 
        || (read_data(dentry->inode_num, 0, buffer, 40) != 40)
        || (buffer[0] != 0x7F || buffer[1] != 0x45 || buffer[2] != 0x4C || buffer[3] != 0x46)){     //executable magic numbers
        return 0; //not executable
    }
    return 1; //is executable
}

/**
* system_execute
* inputs: const uint8_t* command
* output: int32_t
* side effects: Parse arguments, check if executable, setup paging, load file, context switch, setup IRET
*/
int32_t system_execute(const uint8_t* command) {
    cli();      

    //PARSE COMMANDS AND ARGS
    int i;
    uint8_t cmd[CMD_SIZE];
    uint8_t arg1[ARG_SIZE];
    parse_command(command, cmd, arg1);

    //Check if command exists and executable
    dentry_t dentry;
    uint8_t buffer[40];
    if(!check_exec(&dentry, buffer, cmd)) return -FAILURE;
    
    //Get a free PID
    int32_t check_PID = -1;
    for (i = 0; i < 6; i++) {
        if (process_slots[i] == 0){
            process_slots[i] = 1;
            check_PID = i;
            break;
        }
    }
    if(check_PID == -1){
        return SUCCESS;
    }
    cur_PID = check_PID;

    //Set up paging & flush TLB
    user_page_setup(cur_PID);
    asm volatile(
            "movl %%cr3, %%eax \n\
             movl %%eax, %%cr3 \n\
            "
            :
            :
            : "memory"
    );

    //Load file
    inode_t* prog_img_inode = &inode_start_ptr[dentry.inode_num];
    if (read_data(dentry.inode_num, 0, (uint8_t*)0x08048000, prog_img_inode->len) == -FAILURE) {    //program image start addr
        return -FAILURE;
    }
    pcb_t* pcb = (pcb_t*)(0x0800000 - (0x2000 * (cur_PID + 1)));
    //Fix PID system. If its first process in the terminal, set parent PID to -1.
    pcb->pid = cur_PID;
    // if(cur_PID == 0){
    //     pcb->parent_pid = pcb->pid; 
    // }
    // else {
    //     pcb->parent_pid = cur_PID - 1;
    // }

    // if (view_term->pid == -1) {
    //     pcb->parent_pid = -1;
    // }
    // else {
        
    // }
    
    pcb->parent_pid = view_term->pid; //-1 default
    view_term->pid = cur_PID;
    pcb->tid = view_term->tid;

    // initialize fd
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
    pcb->arg1 = arg1;

    //Context switch
    uint8_t eip_buffer[4];
    uint32_t eip, esp, ebp;
    if(read_data(dentry.inode_num, 24, eip_buffer, sizeof(int32_t)) == -FAILURE){       //read bytes 24-27 of executable
        return -FAILURE;
    }
    eip = *((int*)(eip_buffer));
    asm("\t movl %%esp, %0" : "=r"(esp)); // copying kernel esp to a local var
    pcb->process_eip = eip;
    asm("\t movl %%ebp, %0" : "=r"(ebp)); //fill current program ebp
    pcb->process_ebp = ebp;
    pcb->process_esp = esp; //this should be the kernel esp
    tss.ss0 = KERNEL_DS;
    tss.esp0 = 0x800000 - 0x2000*(cur_PID);

 // pushing user context
    sti();
    asm volatile(
        "pushl %0\n\t" // Move USER_DS from eax to data segment
        "pushl %1\n\t" //push user data segment to the stack
        "pushfl\n\t" //push flags to the stack
        "pushl %2\n\t" //push esp argument from pcb into stack
        "pushl %3\n\t" //push user context to stack
        : : "r"(USER_DS), "r"(USER_ESP), "r"(USER_CS), "r"(eip)
    );

    //pushing to kernel stack for iret to set segment registers for syscalls
    asm volatile(
        "iret\n\t"
        "EXECUTE_RETURN: "
    );

    return SUCCESS;
}
