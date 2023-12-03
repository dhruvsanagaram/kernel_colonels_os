#include "page.h"

/* this changes the CPU registers to enable paging */
extern void enable(int directory);

// void init_dir_entry(page_dir_entry_t *entry, uint32_t user, uint32_t present, uint32_t offset) {
    // page_direcctory[i].user = user;
    // page_direcctory[i].present = present;
    // page_direcctory[i].base_addr = offset;
    // page_direcctory[i].rw = 1;
    // page_direcctory[i].write_through = 0;
    // page_direcctory[i].cache_disable = 0;
    // page_direcctory[i].accessed = 0;
    // page_direcctory[i].ps_bit = 1;
    // page_direcctory[i].reserved = 0;
// }

/* void page_init();
 * Inputs: N/A
 * Return Value: void
 *  Function: Sets up the page directory and page tables with empty pages */
void page_init() {
    int i; //index
    //init first 4MB of the dir with 4kb pages
    page_directory[0].user = 0;  //set this page to supervisor mode
    page_directory[0].present = 1;
    page_directory[0].base_addr = ((uint32_t)page_tables) / FOUR_KB;  //read from the start of the page table, and align it in 4KB incs
    page_directory[0].rw = 1;
    page_directory[0].write_through = 0;
    page_directory[0].cache_disable = 0;
    page_directory[0].accessed = 0;
    page_directory[0].ps_bit = 0;   //This makes every page within this 4MB hold 4KB data
    page_directory[0].reserved = 0;

    for (i = 1; i < PAGE_SIZE; i++) {
        if(i == KERNEL_IDX){
          //set up kernel memory
          //init_dir_entry(&page_directory[i], 0, 1, ((uint32_t)KERNEL_ADDR / FOUR_KB));
          page_directory[i].user = 0;  //Kernel pages are not accessable in userspace
          page_directory[i].present = 1;
          page_directory[i].base_addr = KERNEL_ADDR / FOUR_KB;  //align kernel address
          page_directory[i].rw = 1;
          page_directory[i].write_through = 0;
          page_directory[i].cache_disable = 0;
          page_directory[i].accessed = 0;
          page_directory[i].ps_bit = 1;  //each page here is 4MB
          page_directory[i].reserved = 0;
          page_directory[i].global = 1;
        }
        else if (i == USER_IDX) {
          //set up user virtual memory
          // init_dir_entry(&page_directory[i], 1, 1, ((uint32_t)USER_ADDR / FOUR_KB));
          page_directory[i].user = 1;
          page_directory[i].present = 1;
          page_directory[i].base_addr = USER_ADDR / FOUR_KB;     
          page_directory[i].ps_bit = 1;
          page_directory[i].rw = 1;
          page_directory[i].reserved = 0;
          page_directory[i].write_through = 0;
          page_directory[i].cache_disable = 0;
          page_directory[i].accessed = 0;
        } 
        else {
          //set up remaining memory
          page_directory[i].user = 0;
          page_directory[i].present = 0;
          page_directory[i].rw = 1;
          page_directory[i].write_through = 0;
          page_directory[i].cache_disable = 0;
          page_directory[i].accessed = 0;
          page_directory[i].ps_bit = 1;
          page_directory[i].reserved = 0;
        }
        
    }
    
    //iterate through all pages
    for(i = 0; i < PAGE_SIZE; i++){

      //if the curr page is in video mem, present it
      if(i * FOUR_KB != VIDEO_ADDR ){
        page_tables[i].present = 0;
      }
      else{
        page_tables[i].present= 1;
      }
      if ((i-1) * FOUR_KB == VIDEO_ADDR || (i-2) * FOUR_KB == VIDEO_ADDR || (i-3) * FOUR_KB == VIDEO_ADDR) {
        page_tables[i].present= 1;
      } 

      page_tables[i].base_addr = i;
      page_tables[i].rw = 1;
      page_tables[i].user = 0;
      page_tables[i].write_through = 0; 
      page_tables[i].cache_disable = 0; 
      page_tables[i].accessed = 0; 
      page_tables[i].dirty = 0; 
      page_tables[i].global = 0; 
      page_tables[i].avail = 0;    
    }


    //set up the video memory paging
    for(i = 0; i < PAGE_SIZE; i++){
      page_video_map[i].present = 0;
      page_video_map[i].rw = 1;
      page_video_map[i].user = 0;
      page_video_map[i].write_through = 0;
      page_video_map[i].cache_disable = 1;
      page_video_map[i].accessed = 0;
      page_video_map[i].dirty = 0;
      page_video_map[i].reserved = 0;
      page_video_map[i].global = 0;
      page_video_map[i].base_addr = i;
    }  
    
    enable((int)page_directory);
} 

//Helper to set USER_IDX page tables
void user_page_setup(int32_t cur_PID){
  //page_directory[USER_IDX].base_addr = USER_ADDR / FOUR_KB;
  // page_directory[USER_IDX].base_addr = (USER_ADDR - EIGHT_KB*(cur_PID+1)) / FOUR_KB;
  page_directory[USER_IDX].base_addr = (0x800000 + cur_PID*KERNEL_ADDR) / FOUR_KB;
  page_directory[USER_IDX].ps_bit = 1;

}

//Changes the status of a given page within the video map table based on a given 20 bit index and whether or not
//the page at the index is present or not
void vidmap_page_change(int idx_20, unsigned int present){
  page_table_entry_t page;
  //set the page to present depending on the status
  page.present = present;
  page.rw = 1; //we can read/write to vidmap this way
  page.user = 1; //page accessible in userspace
  page.global = present; //globally accessible depending on if the page is present
  page.base_addr = (uint32_t)idx_20; //ensure that this is 20 bit aligned

  page_video_map[VIDMAP_IDX] = page;
}
