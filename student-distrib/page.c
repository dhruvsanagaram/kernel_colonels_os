#include "page.h"

void init_dir_entry(page_dir_entry_t *entry, uint32_t user, uint32_t present, uint32_t offset) {
    entry->user = user;
    entry->present = present;
    entry->base_addr = offset;
    entry->rw = 1;
    entry->write_through = 0;
    entry->cache_disable = 0;
    entry->accessed = 0;
    entry->ps_bit = 1;
    entry->reserved = 0;
}




void page_init() {
    int i; //index
    //init first 4MB of the dir with 4kb pages
    page_directory[0].user = 0;
    page_directory[0].present = 1;
    page_directory[0].base_addr = ((uint32_t)page_tables/FOUR_KB);
    page_directory[0].rw = 1;
    page_directory[0].write_through = 0;
    page_directory[0].cache_disable = 0;
    page_directory[0].accessed = 0;
    page_directory[0].ps_bit = 0;
    page_directory[0].reserved = 0;


    for (i = 1; i < PAGE_SIZE; i++) {
        if(i == KERNEL_IDX){
          //set up kernel memory
          init_dir_entry(&page_directory[i], 0, 1, ((uint32_t)KERNEL_ADDR / FOUR_KB));
        }
        else if (i == USER_IDX) {
          //set up user virtual memory
          init_dir_entry(&page_directory[i], 1, 1, ((uint32_t)USER_ADDR / FOUR_KB));
        } else {
          //set up remaining memory
          init_dir_entry(&page_directory[i], 0, 0, 0);
        }
    }
    
    //iterate through all pages
    for(i = 0; i < PAGE_SIZE; i++){

      //if the curr page is in video mem, present it
      if(i * FOUR_KB != VIDEO_ADDR){
        page_tables[i].present = 0;
      }
      else{
        page_tables[i].present= 1;
      }
      page_tables[i].base_addr = i; 
      page_tables[i].rw = 1;  
      page_tables[i].user = 0;
      page_tables[i].write_through = 0; 
      page_tables[i].cache_disable = 0; 
      page_tables[i].accessed = 0; 
      page_tables[i].dirty = 0; 
      page_tables[i].ps_bit = 0; 
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
      page_video_map[i].base_addr = ((uint32_t)VIDEO_ADDR/FOUR_KB);
    }  
    
} 
