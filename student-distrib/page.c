#include "page.h"


void init_dir_entry(page_dir_entry_t *entry, uint32_t user, uint32_t present, uint32_t offset) {
    entry->user = user;
    entry->present = present;
    entry->offset = offset;
    entry->rw = 1;
    entry->write_through = 0;
    entry->cache_disable = 0;
    entry->accessed = 0;
    entry->size = 1;
    entry->reserved = 0;
}

//initialize the first 4MB of memory with 4KB tables


void page_init() {
    for (i = 0; i < MAX_SPACES; i++) {
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
} 
