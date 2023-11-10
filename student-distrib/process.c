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
    // asm volatile("andl %%esp, %0" 
    //              : "=r"(pcb) :"r"(NEAREST_8KB_BOUND));
    pcb = (pcb_t*)(0x800000 - 0x2000*(cur_PID+1));
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

    cli();                                      //mask interrupts

    printf("Start halt\n");


    pcb_t* pcb = getRunningPCB();               //get the running PCB


    printf("%d\n",pcb->pid);

    printf("%d\n",pcb->parent_pid);
    
    printf("%d\n",pcb->tss_kernel_stack_ptr);
    
    printf("%d\n",pcb->process_eip);
    
    printf("%d\n",pcb->process_esp);
    
    printf("%d\n",pcb->process_ebp);

    if(pcb == NULL){
        printf("HALT: No running process ATM\n");  
        return -FAILURE;
    }
    if(pcb->pid == 0){
        sti();
        return SUCCESS;
    }

    printf("PCB found");

    //Restore parent data
    pcb_t* parent_pcb = (pcb_t*)(0x800000 - 0x2000 * (pcb->parent_pid+1));  //retrieve parent_pcb start address
    //cur_PID = parent_pcb->pid;
    cur_PID = pcb->parent_pid;
    tss.ss0 = KERNEL_DS;
    tss.esp0 = 0x800000 - 0x2000 * (pcb->parent_pid+1);

    //Restore parent paging(& flush TLB)
    user_page_setup(cur_PID);

        asm volatile(
            "mov %%cr3, %%eax \n\
             mov %%eax, %%cr3 \n\
            "
            :
            :
            : "memory"
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
    ebp = parent_pcb->process_esp;
    esp = parent_pcb->process_esp; //set it to stack ptr in kernel space   
    
    
    sti();
    asm volatile(
        "movl %0, %%esp\n\t"
        "movl %1, %%ebp\n\t"
        // "xorl %%eax, %%eax\n\t"
        // "movb %2, %%al\n\t"
        // "pushl %%eax\n\t"
        "jmp EXECUTE_RETURN"
        : : "r"(esp), "r"(ebp)
        : "eax", "memory", "cc"
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
    cli();      

    ////PARSE COMMANDS AND ARGS
    int i;
    uint8_t cmd[CMD_SIZE];
    uint8_t arg1[ARG_SIZE];
    parse_command(command, cmd, arg1);
    int32_t ret;

    //Check if command exists and executable
    dentry_t dentry;
    uint8_t buffer[40];
    printf("Checking exec...");
    printf(command);
    if(!check_exec(&dentry, buffer, cmd)) return -FAILURE;
    
    //Get a free PID
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
            "mov %%cr3, %%eax \n\
             mov %%eax, %%cr3 \n\
            "
            :
            :
            : "memory"
    );

    //Load file
    inode_t* prog_img_inode = &inode_start_ptr[dentry.inode_num];
    // uint8_t prog_img_buf[10000];
    printf("Reading data...");
    if (read_data(dentry.inode_num, 0, (uint8_t*)0x08048000, prog_img_inode->len) == -FAILURE) {
        return -FAILURE;
    }
    
    // if (read_data(dentry.inode_num, 0, (uint8_t*)0x08048000, prog_img_inode->len) == -FAILURE) {
    //     return -FAILURE;
    // }
    printf("Copying to program image...");
    // memcpy((uint8_t*)0x08048000,prog_img_buf,prog_img_inode->len);
    printf("Image size: %d\n", prog_img_inode->len);

    // memcpy((uint8_t*)0x08048000,prog_img_buf,prog_img_inode->len);

    pcb_t* pcb = (pcb_t*)(0x0800000 - (0x2000 * (cur_PID + 1)));
    pcb->pid = cur_PID;
    if(cur_PID == 0){
        //pcb->parent_pid = cur_PID; //this was spawned by the current watcher
        pcb->parent_pid = pcb->pid; 
    }
    else {
        pcb->parent_pid = cur_PID - 1;
        //pcb->parent_pid = backlog_pid;
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
    uint32_t eip, esp;
    if(read_data(dentry.inode_num, 24, eip_buffer, sizeof(int32_t)) == -FAILURE){
        return -FAILURE;
    }
    eip = *((int*)(eip_buffer));
    //esp = USER_ADDR - EIGHT_KB*(cur_PID+1);                          // USER MEMORY ADDRESS + 4 MEGABYTE PAGE FOR START (and int32_t align)= 
    esp = 0x8400000 - sizeof(int32_t);
    pcb->process_eip = eip;
    pcb->process_esp = esp; 
    tss.ss0 = KERNEL_DS; // line right
    //tss.esp0 = 0x800000 - (0x2000*(cur_PID+1));   //might be wrong and we have to 4Byte align
    // tss.esp0 = 0x800000 - 0x2000*(cur_PID+1) - 4;
    // tss.esp0 = 0x800000 - 0x2000*(cur_PID + 1) - 4;
    // tss.esp0 = 0x800000 - 0x2000*cur_PID - sizeof(int32_t);
    tss.esp0 = 0x800000 - 0x2000*(cur_PID);
    pcb->tss_kernel_stack_ptr = tss.esp0;
    // setupIRET();
    //Push IRET Context to Stack
    // sti();

    printf("\n%x",eip);
    printf("\n%x",esp);
    printf("\n%x",tss.esp0);
    printf("\n");

    sti();

    asm volatile(
        "pushl $0x002B\n\t" // Move USER_DS from eax to data segment
        "pushl %1\n\t" //push user data segment to the stack
        "pushfl\n\t" //push flags to the stack
        "pushl $0x0023\n\t" //push esp argument from pcb into stack
        "pushl %3\n\t" //push user context to stack
        : : "r"(USER_DS), "r"(esp), "r"(USER_CS), "r"(eip)
        : "cc", "memory"
    );

    
    asm volatile(
        "iret\n\t" //interrupt ret
        "EXECUTE_RETURN: \n\t" //interrupt ret
    );

    printf("Bottom of execute\n");
    while(1);
    // sti();
    return SUCCESS;
}
