#ifndef PAGING_H
#define PAGING_H

//Define the page size
#define FOUR_KB 4096 //2^12 bytes

#define MAX_SPACES 1024 //number of pages in dir

void page_init();


uint32_t page_directory[MAX_SPACES] __attribute__((aligned(4096)));

#endif