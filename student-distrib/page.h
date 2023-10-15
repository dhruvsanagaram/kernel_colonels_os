#ifndef PAGING_H
#define PAGING_H

//Define the page size
#define FOUR_KB 4096 //2^12 bytes
#define KERNEL_ADDR 0x400000 //start of kmem in physical media
#define USER_ADDR 0x800000 //start of user mem in physical media
#define MAX_SPACES 1024 //number of pages in dir
#define KERNEL_IDX 1
#define USER_IDX 32


void page_init();
void init_dir_entry(page_dir_entry_t *entry, uint32_t user, uint32_t present, uint32_t offset);

typedef struct __attribute__((packed)) page_dir_entry_t {
  uint32_t present: 1;
  uint32_t rw: 1;
  uint32_t user: 1;
  uint32_t write_through: 1;
  uint32_t cache_disable: 1;
  uint32_t accessed: 1;
  uint32_t ignored: 1;
  uint32_t reserved: 1;
  uint32_t size: 1;
  uint32_t avail: 3; //11:9
  uint32_t offset: 20; //31:12
} page_dir_entry_t;

page_dir_entry_t page_directory[MAX_SPACES] __attribute__((aligned(4096)));

#endif