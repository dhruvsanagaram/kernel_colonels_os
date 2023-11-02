#include "process.h"
#include "lib.h"
#include "page.h"
#include "x86_desc.h"

#define SUCCESS 0
#define FAILURE 1

int32_t process_slots[2] = {0, 0};
uint32_t cur_PID = -1;    //watcher for current pid being executed
// uint32_t backlog_pid = -1; //watcher for the backlog (parent pid) for subsequently spawned processes

/**
* getRunningPCB
* inputs: none
* output: pointer to the recently running PCB calculated via runtime stack
* side effects: none
*/
pcb_t* getRunningPCB() {
    pcb_t* pcb;
    asm volatile("andl %%esp, %0" 
                 : "=r"(pcb) :"r"(NEAREST_8KB_BOUND));
    return pcb;
}

/**
* setupIRET
* inputs: none
* output: void
* side effects: Setup the IRET context for kernel->user transition
*/
// void setupIRET(){
//     asm volatile(
//         "movw %%ax, %ds\n\t" // Move USER_DS from eax to data segment
//         "pushl %%eax\n\t" //push user data segment to the stack
//         "pushl %%ebx\n\t" //push esp argument from pcb into stack
//         "pushfl\n\t" //push flags to the stack
//         "pushl %%ecx\n\t" //push user context to stack
//         "pushl %%edx\n\t" //push eip argument from pcb into stack
//         "iret\n\t" //interrupt ret
//         "EXECUTE_RETURN: " //interrupt ret
//         : : "a"(USER_DS), "b"(esp), "c"(USER_CS), "d"(eip)
//         : "cc", "memory"
//     );
// }

/**
* system_halt
* inputs: uint8_t status
* output: int32_t
* side effects: Restore parent data & paging & clear relevant FDs
*/
int32_t system_halt(uint8_t status){
    int i;                                      //for iterating across every FDs that are running
    int32_t lower_status = status & 0x00FF;     //mask lower bits of status for ret

    cli();                                      //mask interrupts

    pcb_t* pcb = getRunningPCB();               //get the running PCB
    if(pcb == NULL){
        printf("HALT: No running process ATM\n");
        return -FAILURE;
    }
    if(pcb->pid == 0){
        return SUCCESS;
    }

    //Restore parent data
    pcb_t* parent_pcb = (pcb_t*)(0x800000 - 0x2000 * (pcb->parent_pid+1));
    cur_PID = parent_pcb->pid;
    tss.ss0 = KERNEL_DS;
    tss.esp0 = 0x800000 - 0x2000 * (parent_pcb->pid+1) - sizeof(int32_t);

    //Restore parent paging(& flush TLB)
    user_page_setup(cur_PID);
    //flush tlb
    asm volatile(
        "movl %%cr3, %%eax\n\t"             
        "movl %%eax, %%cr3 "
        : : : "eax", "memory", "cc"
    );
    
    //Close all relevant FDs
    process_slots[pcb->pid] = 0;                            //free up the slot for the recently running process
    for(i = 0; i < 8; i++){
        pcb->fd_arr[i].inode_num = -1;
        pcb->fd_arr[i].flags = 0;                           //Process halted so flags are set to 0 to signify file out of use
        pcb->fd_arr[i].fpos = 0;
        pcb->fd_arr[i].fops = &nul_fops;                     //no more file ops for FDs >:)
    }

    //Jump to execute return
    uint32_t esp, ebp;
    ebp = parent_pcb->process_ebp;
    esp = parent_pcb->process_esp;

    sti();
    asm volatile(
        "movl %0, %%esp\n\t"
        "movl %1, %%ebp\n\t"
        "movl %2, %%eax\n\t"
        "jmp EXECUTE_RETURN "
        : : "r"(esp), "r"(ebp), "r"(lower_status)
        : "eax", "memory", "cc"
    );
    return status;
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
    //was max_buff
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
    if(read_dentry_by_name(&cmd[0], dentry) == -FAILURE 
        || (read_data(dentry->inode_num, 0, buffer, 40) != 40)
        || (buffer[0] != 0x7F || buffer[1] != 0x45 || buffer[2] != 0x4C || buffer[3] != 0x46)){
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
    //Parse args
    cli();      /* The command is a space-separated sequence of words. The first word is the file name of the
                program to be executed, and the rest of the command—stripped of leading spaces—should be provided to the new
                program on request via the getargs system call.
                */

    ////PARSE COMMANDS AND ARGS
    int i;
    uint8_t cmd[CMD_SIZE];
    uint8_t arg1[ARG_SIZE];
    parse_command(command, cmd, arg1);
    // int i;
    // for (i = 0; i < MAX_BUF; i++) {
    //     if (command[i] == ' ' || command[i] == '\0' || command[i] == '\n') {
    //         cmd[i] = '\0';
    //         break;
    //     }
    //     else {
    //         cmd[i] = command[i];
    //     }
    // }
    // if (command[i] != '\0') {
    //     i++;
    //     for (;i < MAX_BUF; i++) {
    //         if (command[i] == ' ' || command[i] == '\0' || command[i] == '\n') {
    //             arg1[i] = '\0';
    //             break;
    //         }
    //         else {
    //             arg1[i] = command[i];
    //         }
    //     }       
    // }

    //Check if command exists and executable
    dentry_t dentry;
    uint8_t buffer[40];
    // if(read_dentry_by_name(&cmd[0], &dentry) == -FAILURE 
    //     || (read_data(dentry.inode_num, 0, buffer, 40) != 40)
    //     || (buffer[0] != 0x7F || buffer[1] != 0x45 || buffer[2] != 0x4C || buffer[3] != 0x46)){
    //     return -FAILURE;
    // }
    printf("Checking exec...");
    if(!check_exec(&dentry, buffer, cmd)) return -FAILURE;
    printf("Shit is exec...");
    //Get a PID
    int32_t check_PID = -1;
    for (i = 0; i < 2; i++) {
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
        "movl %%cr3, %%eax\n\t"
        "movl %%eax, %%cr3 "
        : : : "eax", "memory", "cc"
    );

    //Load file
    inode_t* prog_img_inode = &inode_start_ptr[dentry.inode_num];
    uint8_t prog_img_buf[10000];
    printf("Reading data...");
    if (read_data(dentry.inode_num, 0, prog_img_buf, prog_img_inode->len) == -FAILURE) {
        printf("cock!");
        return -FAILURE;
    }
    printf("Copying to program image...");
    memcpy((uint8_t*)0x08048000,prog_img_buf,prog_img_inode->len);
    printf("Balls...");
    pcb_t* pcb = (pcb_t*)(0x0800000 - (0x2000 * (cur_PID + 1)));
    pcb->pid = cur_PID;
    if(cur_PID == 0){
        //pcb->parent_pid = cur_PID; //this was spawned by the current watcher
        pcb->parent_pid = pcb->pid;
    }
    else {
        //get the current pcb somehow 
        //and set parent_pid to the pid of the current running pcb
        pcb->parent_pid = getRunningPCB()->pid;
        //pcb->parent_pid = backlog_pid;
        //backlog_pid = cur_PID; //Subsequently spawned processes will be attributed to the current PID executed
    }

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

    //Context switch
    uint8_t eip_buffer[4];
    int32_t eip, esp;
    if(read_data(dentry.inode_num, 24, eip_buffer, sizeof(int32_t)) == -FAILURE){
        return -FAILURE;
    }
    eip = *((int*)(eip_buffer));
    esp = USER_ADDR - EIGHT_KB*(cur_PID+1);                          // USER MEMORY ADDRESS + 4 MEGABYTE PAGE FOR START (and int32_t align)= 
    pcb->process_eip = eip;
    pcb->process_esp = esp; 
    tss.ss0 = KERNEL_DS; // line right
    tss.esp0 = 0x800000 - (0x2000*(cur_PID+1));   //might be wrong and we have to 4Byte align
    pcb->tss_kernel_stack_ptr = tss.esp0;
    // setupIRET();
    //Push IRET Context to Stack
    sti();

    // asm volatile(
    //     "movw %%ax, %%ds " // Move USER_DS from eax to data segment
    //     : : "a"(USER_DS)
    //     : "cc", "memory"
    // );
    // asm volatile(
    //     "pushl %%eax\n\t" //push user data segment to the stack
    //     "pushl %%ebx\n\t" //push esp argument from pcb into stack
    //     "pushfl\n\t" //push flags to the stack
    //     "pushl %%ecx\n\t" //push user context to stack
    //     "pushl %%edx " //push eip argument from pcb into stack
    //     : : "a"(USER_DS), "b"(esp), "c"(USER_CS), "d"(eip)
    //     : "cc", "memory"
    // );
    // asm volatile(
    //     "iret\n\t" //interrupt ret
    //     "EXECUTE_RETURN: " //interrupt ret
    //     : : : "memory"
    // );
    asm volatile(
        "movw %%ax, %%ds\n\t" // Move USER_DS from eax to data segment
        "pushl %%eax\n\t" //push user data segment to the stack
        "pushl %%ebx\n\t" //push esp argument from pcb into stack
        "pushfl\n\t" //push flags to the stack
        "pushl %%ecx\n\t" //push user context to stack
        "pushl %%edx\n\t" //push eip argument from pcb into stack
        "iret\n\t" //interrupt ret
        "EXECUTE_RETURN: " //interrupt ret
        : : "a"(USER_DS), "b"(esp), "c"(USER_CS), "d"(eip)
        : "cc", "memory"
    );

    // iretContext(esp, eip);

    // sti();
    return SUCCESS;
}
