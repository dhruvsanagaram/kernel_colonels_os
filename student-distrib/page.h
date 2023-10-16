#ifndef PAGE_H
#define PAGE_H

#ifndef ASM

#include "types.h"

//Define the page size
#define FOUR_KB 4096 //2^12 bytes
#define KERNEL_ADDR 0x400000 //start of kmem in physical media
#define USER_ADDR 0x800000 //start of virtual user mem 
#define VIDEO_ADDR 0xB8000 //start of video memory
#define PAGE_SIZE 1024 //number of pages in dir
#define KERNEL_IDX 1
#define USER_IDX 32


extern void page_init();
//extern void init_dir_entry(page_dir_entry_t* entry, uint32_t user, uint32_t present, uint32_t offset);



//PAGING struct: taken from https://wiki.osdev.org/Paging
typedef struct page_dir_entry_t {
  struct{
    uint32_t present       :1; //bit 0: present?12
    uint32_t rw            :1;  //bit 1: read/write?12
    uint32_t user          :1; //bit 2: user mode? 1 is user, 0 is supervisor12
    uint32_t write_through :1; //bit 3: write-through?12
    uint32_t cache_disable :1; //bit 4: is cache disabled?12
    uint32_t accessed      :1; //bit 5: PDE or PTE was read during virtual address translation?12
    uint32_t reserved      :1; //bit 6: reserved? (usually 0)12
    uint32_t ps_bit        :1; //bit 7: page size? 0 is 4KB, 1 is 4MB12
    uint32_t global        :1; //bit 8: is this a global page?
    uint32_t avail         :3; //11:9 accounting information/user available info
    uint32_t base_addr     :20; //31:12 base address1
  }__attribute__((packed));
} page_dir_entry_t;

typedef struct page_table_entry_t {
   struct{
    uint32_t present       :1; //bit 0: present?
    uint32_t rw            :1;  //bit 1: read/write?
    uint32_t user          :1; //bit 2: user mode? 1 is user, 0 is supervisor
    uint32_t write_through :1; //bit 3: write-through?
    uint32_t cache_disable :1; //bit 4: is cache disabled?
    uint32_t accessed      :1; //bit 5: PDE or PTE was read during virtual address translation?
    uint32_t dirty         :1; //bit 6: was this page written to?
    uint32_t reserved      :1; //bit 7: reserved for intel
    uint32_t global        :1; //bit 8: is this a global page?
    uint32_t avail         :3; //11:9 accounting information/user available info
    uint32_t base_addr     :20; //31:12 base address
   }__attribute__((packed));
} page_table_entry_t;


//pages aligned via 4kB boundaries
page_dir_entry_t page_directory[PAGE_SIZE] __attribute__((aligned(FOUR_KB)));
page_table_entry_t page_tables[PAGE_SIZE] __attribute__((aligned(FOUR_KB)));
page_table_entry_t page_video_map[PAGE_SIZE] __attribute__((aligned(FOUR_KB)));


#endif
#endif
