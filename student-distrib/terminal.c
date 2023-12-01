#include "keyboard.h" 
#include "terminal.h"
#include "page.h"
#include "lib.h"
#include "i8259.h"
#include "process.h"


int numChars = 0;
int enterKeyPressed = 0;
terminal_t* schedule_term;
terminal_t* view_term;
terminal_t* terminals[3];



int32_t init_terms() {
    //populate the terminal structs in the terminals array
    // schedule_term = terminals[0];
    // view_term = terminals[0];
    int i;
    for (i = 0; i < MAX_TERMS; i++) {
        terminals[i]->tid = i;
        terminals[i]->pid = -1;
        terminals[i]->enterKeyPressed = 0;
        terminals[i]->keyb_char_count = 0;
        terminals[i]->cursor_x = 0;
        terminals[i]->cursor_y = 0;
        terminals[i]->vidmem_data = VIDEO_ADDR + (i+1) * FOUR_KB;
    
    }
    //flush the TLB
    asm volatile(
        "movl %%cr3, %%eax \n\t"
        "movl %%eax, %%cr3 \n\t"
        : : : "memory"
    );
    //update global viewing buf_pos and global keyb_buf as needed
    screen_x = terminals[0]->cursor_x;
    screen_y = terminals[0]->cursor_y;
    update_cursor(screen_x, screen_y);
    schedule_term = terminals[0];
    view_term = terminals[0];

    system_execute("shell");
    
    return 0;
}

void terminal_switch(int32_t target_tid){ // TO-DO: If pid = -1, run shell
    if(target_tid == view_term->tid){
        return;
    }

    // terminal_t* curr_term = &terminals[view_term->tid];
    terminal_t* target_term = terminals[target_tid];

    //save info about current terminal to curr_term
    // view_term->key_buf = key_buf;
    memcpy(view_term->key_buf,key_buf,sizeof(key_buf));
    // key_buf = target_term->key_buf;
    memcpy(key_buf,target_term->key_buf,sizeof(target_term->key_buf));
    view_term->keyb_char_count = keyb_char_count;  
    keyb_char_count = target_term->keyb_char_count;
    view_term->enterKeyPressed = enterKeyPressed;
    enterKeyPressed = target_term->enterKeyPressed;
    
    update_video_memory_paging(view_term->tid);
    view_term = terminals[target_tid];
    memcpy((void*)(view_term->vidmem_data), (void*)VIDEO_ADDR, FOUR_KB);
    memcpy((void*)(VIDEO_ADDR), (void*)(target_term->vidmem_data), FOUR_KB);
    update_video_memory_paging(schedule_term->tid);  //Should it be target_tid? Since current_pid 
                                                //denotes the process currently being executed as determined
                                                //by the scheduler, update_video_memory_paging(get_owner_terminal(current_pid))
                                                //denotes the terminal corresponding to current_pid, which may 
                                                //not be a process in the target terminal. 
                                                //So update_video_memory_paging(get_owner_terminal(current_pid)) would be wrong then

    if (view_term->pid == -1) {
        system_execute("shell");
    }

}

void update_video_memory_paging(int term_id){
    /*
        1. update (save) cursor positions for switching to term_id
        2. update the vidmem mapping by switching based on the vidmem page for the terminal specified by term_id
        3. flush TLB
    */
    //the cursors below initialized in terminal_init()
    screen_x = terminals[term_id]->cursor_x;
    screen_y = terminals[term_id]->cursor_y;
    update_cursor(screen_x,screen_y);
    
    if(view_term->tid == term_id){          //backing store
        //do the paging for same terminal
        page_tables[VIDMAP_IDX].base_addr = VIDEO_ADDR / FOUR_KB;
        //change user video memory mapping
        page_video_map[VIDMAP_IDX].base_addr = VIDEO_ADDR / FOUR_KB;
        int tar_pid = terminals[term_id]->pid;
        page_video_map[VIDMAP_IDX].present = getPCBByPid(tar_pid)->vidmap_present;
        // page_video_map[VIDMAP_IDX].present = terminals[term_id]->vidmap_present;

    } else {                                //active store  
        //do paging for a diff terminal
        page_tables[VIDMAP_IDX].base_addr = terminals[term_id]->vidmem_data / FOUR_KB;
        page_video_map[VIDMAP_IDX].base_addr = terminals[term_id]->vidmem_data / FOUR_KB;
        // page_video_map[VIDMAP_IDX].present = terminals[term_id]->vidmap_present;
        page_video_map[VIDMAP_IDX].base_addr = VIDEO_ADDR / FOUR_KB;
        int tar_pid = terminals[term_id]->pid;
        page_video_map[VIDMAP_IDX].present = getPCBByPid(tar_pid)->vidmap_present;
    }

    //flush TLB
    asm volatile(
        "movl %%cr3, %%eax\n\t"
        "movl %%eax, %%cr3\n\t"
        : : : "memory"
    );
}


///////////////////////////////////////////////////// TERM_SYSCALLS /////////////////////////////////////////////////////



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







