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
terminal_t terminals[MAX_TERMS];


/**
* init_terms
* inputs: noone
* output: int32_t
* side effects: initialize all the terminal structs for each term
*/
int32_t init_terms() {
    //populate the terminal structs in the terminals array
    // schedule_term = terminals[0];
    // view_term = terminals[0];
    int32_t i;
    for (i = 0; i < MAX_TERMS; i++) {
        terminal_t add_term;
        add_term.tid = i;
        add_term.pid = -1;
        add_term.enterKeyPressed = 0;
        add_term.keyb_char_count = 0;
        add_term.cursor_x = 0;
        add_term.cursor_y = 0;
        add_term.vidmem_data = VIDEO_ADDR + (i+1) * FOUR_KB;
        terminals[i] = add_term;
    }
    //flush the TLB
    asm volatile(
        "movl %%cr3, %%eax \n\t"
        "movl %%eax, %%cr3 \n\t"
        : : : "memory"
    );
    //update global viewing buf_pos and global keyb_buf as needed
    // screen_x = terminals[0].cursor_x;
    // screen_y = terminals[0].cursor_y;
    //update_cursor(screen_x, screen_y);

    keyb_char_count = terminals[0].keyb_char_count;
    memcpy(key_buf, terminals[0].key_buf, MAX_BUF_SIZE);

    schedule_term = &terminals[0];
    view_term = &terminals[0];

    system_execute((uint8_t*)"shell");
    return 0;
}


/**
* terminal_switch
* inputs: target_tid
* output: n/a
* side effects: initialize the switch for the next terminal, handles paging and keyboard interrupt
*/
void terminal_switch(int32_t target_tid){ // TO-DO: If pid = -1, run shell
    if(target_tid == view_term->tid){
        return;
    }

    // terminal_t* curr_term = &terminals[view_term->tid];
    terminal_t* target_term = &terminals[target_tid];

    //save info about current terminal to curr_term
    view_term->cursor_x = screen_x;
    view_term->cursor_y = screen_y;

    // view_term->key_buf = key_buf;
    memcpy(view_term->key_buf,key_buf,sizeof(key_buf));
    // key_buf = target_term->key_buf;
    memcpy(key_buf,target_term->key_buf,sizeof(target_term->key_buf));
    view_term->keyb_char_count = keyb_char_count; 
    keyb_char_count = target_term->keyb_char_count;
    view_term->enterKeyPressed = enterKeyPressed;
    enterKeyPressed = target_term->enterKeyPressed;

    
    update_video_memory_paging(view_term->tid);
    
    memcpy((void*)(view_term->vidmem_data), (void*)VIDEO_ADDR, FOUR_KB);
    memcpy((void*)(VIDEO_ADDR), (void*)(target_term->vidmem_data), FOUR_KB);
    view_term = &terminals[target_tid];
    update_video_memory_paging(getRunningPCB()->tid);

                                        
    screen_x = view_term->cursor_x;
    screen_y = view_term->cursor_y;
    update_cursor(screen_x,screen_y);
    send_eoi(1);

    if (view_term->pid == -1) {
        system_execute((uint8_t*)"shell");
    }
}



/**
* update_video_memory_paging
* inputs: term_id
* output: n/a
* side effects: takes the call for terminal paging and takes care of mapping
*/
void update_video_memory_paging(int term_id){
    /*
        1. update (save) cursor positions for switching to term_id
        2. update the vidmem mapping by switching based on the vidmem page for the terminal specified by term_id
        3. flush TLB
    */
    //the cursors below initialized in terminal_init()
    // terminals[]

    // screen_x = terminals[term_id].cursor_x;
    // screen_y = terminals[term_id].cursor_y;
    // update_cursor(screen_x,screen_y);
    
    if(view_term->tid == term_id){          //backing store
        //do the paging for same terminal
        page_tables[terminal_index_p].base_addr = VIDEO_ADDR / FOUR_KB;
        //change user video memory mapping
        page_video_map[terminal_index_p].base_addr = VIDEO_ADDR / FOUR_KB;
        int tar_pid = terminals[term_id].pid;
        page_video_map[terminal_index_p].present = getPCBByPid(tar_pid)->vidmap_present;
        // page_video_map[184].present = getRunningPCB()->vidmap_present;
        // page_video_map[VIDMAP_IDX].present = terminals[term_id]->vidmap_present;

    } else {                                //active store  
        //do paging for a diff terminal
        page_tables[terminal_index_p].base_addr = terminals[term_id].vidmem_data / FOUR_KB;
        page_video_map[terminal_index_p].base_addr = terminals[term_id].vidmem_data / FOUR_KB;
        // page_video_map[VIDMAP_IDX].present = terminals[term_id]->vidmap_present;
        // page_video_map[VIDMAP_IDX].base_addr = VIDEO_ADDR / FOUR_KB;
        // int tar_pid = terminals[term_id].pid;
        // page_video_map[184].present = getRunningPCB()->vidmap_present;
        // page_video_map[184].present = getPCBByPid(tar_pid)->vidmap_present;
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







