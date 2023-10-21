#ifndef FILESYS_H
#define FILESYS_H

#include "lib.h"
#include "types.h"
////////////// FS_TYPE EXT2 //////////////
///////////// USABLE MACROS //////////////////
#define EXT2_DATA_BLOCK_SIZE 4096 //total block size is 4KB
#define SUPERBLOCK_SIZE 64 //boot block is 64B
#define MAX_FILE_SIZE 4190208 //max file size is 4MB
#define FILENAME_LEN 32 //file names are 32B
#define SUPERBLOCK_RESERVED 52 //52 bytes reserved by intel
#define DENTRY_RESERVED 24 //24 bytes reserved by intel
#define NUM_DENTRIES_BOOT 63 //there are 63 dentries in total
#define NUM_DATA_BLOCKS 1023 //2^10 - 1 datablocks avail
#define DENTRY_OFFSET 64 //each dentry occupies 64B
#define MAX_NUM_FILES 62
#define EOF 0x0 //eof code for ending read
//////////// FILESYS STRUCTS /////////////



typedef struct {
  int8_t filename[FILENAME_LEN];
  int32_t filetype;
  int32_t inode_num;
  int8_t reserved[DENTRY_RESERVED]; ///24
} dentry_t;

typedef struct {
  int32_t len;
  int32_t data_block_num[1023];
} inode_t;

typedef struct {
  int32_t dentry_ct;
  int32_t inode_ct;
  int32_t data_block_ct;
  int8_t reserved[SUPERBLOCK_RESERVED]; //52B reserved
  dentry_t dentries[NUM_DENTRIES_BOOT];
} superblock_t;

typedef struct {
    int32_t inode_num; // The inode number of the file
    int32_t position;  // The current position within the file
} file_d_t;

//array of file descriptors for each file we need to read
file_d_t fd_arr[MAX_NUM_FILES];


/*-- MOUNT INIT FS --*/
int32_t init_fs(unsigned int fs_base);

// Directory syscalls
int32_t dir_open(const uint8_t *filename);
int32_t dir_close(int32_t fd);
int32_t dir_read(int32_t fd, void *buf, int32_t nbytes);
int32_t dir_write(int32_t fd, const void *buf, int32_t nbytes);


// File syscalls
int32_t file_open(const uint8_t *filename);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void *buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes);


/////////// FS UTILS (for the kernel) ///////////
//// table of functs
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);

// read data from the file given inode num //
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif /* FILESYS_H */