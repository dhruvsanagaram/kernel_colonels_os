#include "fsdriver.h"

#define SUCCESS 0
#define FAILURE 1


//PTR to filesystem's base address

superblock_t *boot_base_addr;
inode_t* inode_start_ptr;
dentry_t* dentry_start_ptr;
uint8_t* data_block_ptr;

/* 
  * init_fs
  * inputs: unsigned int filesys_addr -- address of base of filesystem
  * outputs: n/a
  * return val: status flag (success: 0 if fs mounted properly, -1 else)
  * effects: set up the filesystem's data structure

*/
int32_t init_fs(unsigned int filesys_addr){
  int i;
  boot_base_addr = (superblock_t*)filesys_addr;
  uint32_t num_inodes = boot_base_addr->inode_ct;
  dentry_start_ptr = boot_base_addr->dentry;
  inode_start_ptr = (inode_t*)(boot_base_addr + 1);
  data_block_ptr = (uint8_t*)(inode_start_ptr + num_inodes);

  //initialize file descriptor table for bookkeeping
  for(i=0; i < MAX_NUM_FILES; ++i){
    fd_arr[i].inode_num = -1; //fdarr not in use yet
    fd_arr[i].file_pos = 0; //initial position
  }
  return SUCCESS; 
}
////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////             DIR SYSCALLS            //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int32_t dir_open(const uint8_t *filename){
  dentry_t the_dentry; //the dentry that represents the directory containing the file
  if(read_dentry_by_name(filename, &the_dentry) == -FAILURE){
    return -FAILURE; //file does not exist
  }

  int fd; //find an empty slot in the file descriptor table
  for(fd = 0; fd < MAX_NUM_FILES; ++fd){
    if(fd_arr[fd].inode_num == -1){
      break;
    }
  }

  if(fd == MAX_NUM_FILES){
    return -FAILURE; //no available file descriptors
  }

  //at this point, we have an empty slot in the file descriptor table to use
  //so we populate the file descriptor table entry with the inode number of the directory
  fd_arr[fd].inode_num = the_dentry.inode_num;
  fd_arr[fd].file_pos = 0;
  return fd; //return the file_descriptor index
}

int32_t dir_close(int32_t fd){
  return SUCCESS; //exit out
}



int32_t dir_read(int32_t fd, void *buf, int32_t nbytes){
  int32_t fd_idx;
  int32_t bytes_name_to_cpy;
  dentry_t dentry_read; //the dentry to read based on calced pos
  if(buf == NULL || nbytes == 0 || fd_arr[fd].inode_num == -1){
    return -FAILURE;
  }
  //check for valid fd
  if(fd < 0 || fd >= MAX_NUM_FILES){
    return -FAILURE;
  }

  //note current position of the file descriptor in the bookkeeping tbl
  fd_idx = fd_arr[fd].position;
  if (read_dentry_by_index(fd_idx, &dentry_read) == -FAILURE){
    return SUCCESS; //end of dir reached
  }

  //Copy the filename into the buffer
  bytes_name_to_cpy = nbytes < FILENAME_LEN ? nbytes : FILENAME_LEN;
  strncpy((int8_t*)buf, (int8_t*)&dentry_read.filename, bytes_name_to_cpy);

  //null-terminate the string to cap extra space
  if(bytes_name_to_cpy < nbytes){
    ((char*)buf)[bytes_name_to_cpy] = '\0';
  }

  //go to next position in the dir
  fd_arr[fd].position++;
  return bytes_name_to_cpy;
}

int32_t dir_write(int32_t fd, const void *buf, int32_t nbytes){
  return -FAILURE;
  ////DIRECTORIES ARE READ ONLY////
}



////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////           FILE SYSCALLS             //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

int32_t file_open(const uint8_t *filename){
  dentry_t the_dentry; //the dentry that represents the directory containing the file
  if(read_dentry_by_name(filename, &the_dentry) == -FAILURE){
    return -FAILURE; //file does not exist
  }
  int fd; //find an empty slot in the file descriptor table
  for(fd = 0; fd < MAX_NUM_FILES; ++fd){
    if(fd_arr[fd].inode_num == -1){
      break;
    }
  }

  if(fd == MAX_NUM_FILES){
    return -FAILURE; //no available file descriptors
  }

  //at this point, we have an empty slot in the file descriptor table to use
  fd_arr[fd].inode_num = the_dentry.inode_num;
  fd_arr[fd].file_pos = 0;
  return fd; //return the file_descriptor index
}

int32_t file_close(int32_t fd){
  return SUCCESS;
}

int32_t file_read(int32_t fd, void *buf, int32_t nbytes){
  //do something with read_data for file
  if (fd < 0 || fd >= MAX_NUM_FILES || nbytes < 0 || fd_arr[fd].inode_num == -1){
      return -FAILURE //Invalid 
  }
  file_d_t *file_d = &fd_arr[fd];
  int32_t byte_r = read_data(file_d->inode_num, file_d->position, buf, nbytes);
  if(byte_r <= 0){
    return -FAILURE;
  }
  file_d->position += byte_r; //Update
  return byte_r;
}

int32_t file_write(int32_t fd, const void *buf, int32_t nbytes){
  return -FAILURE; //fails as this is a read only file system
}


/////////////////////// API FUNCS ///////////////////////////////////////////////////////////////// 

/* 
  * read_dentry_by_name
  * Description: searches the system from dentry fname
  * inputs:  fname -- the directory filename
             dentry -- pointer to the dentry object
  * outputs: n/a
  * return val: status flag (success: 0 if fs mounted properly, -1 else)
  * effects: set up the filesystem's data structure

*/
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry){
  if(fname == NULL){
    return -FAILURE;
  }
  int len_filename = strlen((int8_t*)fname);
  //check if filename is between 32 and 0 chars
  if(len_filename > 32 || len_filename <= 0){
    return -FAILURE;
  }
  int i;
  for(i = 0; i < SUPERBLOCK_SIZE; ++i){
    uint8_t dentry_len = strlen((int8_t*)(dentry_start_ptr + i)->filename);
    if(dentry_len >= FILENAME_LEN){
      //clip down to 32 chars for security
      dentry_len = FILENAME_LEN;
    }
    if(dentry_len == len_filename){
      if(strncmp((int8_t*)fname, (int8_t*)(dentry_start_ptr + i)->filename, FILENAME_LEN) == 0){
        read_dentry_by_index(i, dentry);
        return SUCCESS; //we finished reading
      }
    }
  }
  return -FAILURE; //filename not found
}



int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry){
  if (index >= NUM_DENTRIES_BOOT) {
    return -FAILURE;// out of bounds or dentry null
  }
  *dentry = boot_base_addr->dentries[index]; //set eq
  return SUCCESS; // works
}


/* 
  * read_data
  * Description: reads given data from a file's inode indx
  * inputs: inode -- index/inode number to read data from
            offset -- offset from beginning marker to read to EOF
            buf -- pointer to buffer to read data into
            length -- number of bytes to read
  * outputs: n/a
  * return val: status flag (success: 0 if fs mounted properly, -1 else)
  * effects: set up the filesystem's data structure

*/

// read data from the file given inode num //
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    uint32_t num_bytes_read = 0; //counter for number of bytes read
    uint32_t datablock_curr = offset / EXT2_DATA_BLOCK_SIZE;
    uint32_t byte_curr = offset % EXT2_DATA_BLOCK_SIZE; //the current byte to read
    uint32_t buf_idx = 0; //current index in buf we are writing to after read
    uint32_t NUM_INODES = boot_base_addr->inode_ct; //number of inodes in the system
    uint32_t NUM_DATABLOCKS = boot_base_addr->data_block_ct; //number of data blocks in the system
    
    int i;
    if (inode >= NUM_INODES || inode < 0) {
        return -FAILURE; // invalid
    }
    //inode_t* curr_inode = (inode_t*)(inode_start_ptr + inode);
    inode_t* curr_inode = &inode_start_ptr[inode];
    if( offset >= curr_inode->len){
      return -FAILURE; // invalid
    }
    if (offset + length > curr_inode->len) {
        length = curr_inode->len - offset; // bytes read
    }


    // uint32_t num_bytes_read; //counter for number of bytes read
    // uint32_t byte_curr; //the current byte to read
    // int i;
    for(i=0; i < length; ++i){
      
      if(i + offest >= curr_inode->len){
        return num_bytes_read; //reached EOF
      }
      if(byte_curr >= EXT2_DATA_BLOCK_SIZE){
        byte_curr = 0; // reset cur byte in that file, reset
        datablock_curr++; //go to the next datablock
      }
      uint32_t data_block_num = curr_inode->data_block_num[datablock_curr];
      if(data_block_num >= NUM_DATABLOCKS){
        return -FAILURE; // invalid
      }

      uint8_t* block = (uint8_t*)(data_block_ptr + data_block_num * EXT2_DATA_BLOCK_SIZE);
      // buf[buf_idx] = block[block_offset]; 
      memcpy(&buf[buf_idx], &block[byte_curr], 1);
      buf_idx++; byte_curr++; num_bytes_read++;
    }

    return num_bytes_read;
}

////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////             TESTS                ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

